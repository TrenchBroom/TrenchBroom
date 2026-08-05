/*
 Copyright (C) 2010 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/ExtrudeTool.h"

#include "base/Logger.h"
#include "base/PreferenceManager.h"
#include "gl/Camera.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushGeometry.h"
#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeContents.h"
#include "mdl/PickResult.h"
#include "mdl/Polyhedron.h"
#include "mdl/TransactionScope.h"
#include "prefs/Preferences.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/k.h"
#include "kd/map_utils.h"
#include "kd/overload.h"
#include "kd/ranges/concat_view.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"
#include "kd/result.h"
#include "kd/result_fold.h"

#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/line_io.h"  // IWYU pragma: keep
#include "vm/plane_io.h" // IWYU pragma: keep
#include "vm/polygon.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <algorithm>
#include <map>
#include <ranges>
#include <vector>

namespace tb::ui
{
namespace
{

struct EdgeInfo
{
  mdl::BrushFaceHandle leftFaceHandle;
  mdl::BrushFaceHandle rightFaceHandle;
  double leftDot;
  double rightDot;
  vm::segment3d segment;
  vm::line_distance<double> dist;

  std::partial_ordering operator<=>(const EdgeInfo& other) const
  {
    return dist.distance <=> other.dist.distance;
  }

  bool operator==(const EdgeInfo& other) const { return *this <=> other == 0; }
};

std::partial_ordering operator<=>(
  const std::optional<EdgeInfo>& lhs, const std::optional<EdgeInfo>& rhs)
{
  return !lhs && !rhs ? std::partial_ordering::unordered
         : !lhs       ? std::partial_ordering::greater
         : !rhs       ? std::partial_ordering::less
                      : *lhs <=> *rhs;
}

std::optional<EdgeInfo> getEdgeInfo(
  const mdl::BrushEdge* edge, mdl::BrushNode& brushNode, const vm::ray3d& pickRay)
{

  const auto segment = edge->segment();
  const auto dist = vm::distance(pickRay, segment);

  const auto leftFaceIndex = edge->firstFace()->payload();
  const auto rightFaceIndex = edge->secondFace()->payload();
  contract_assert(leftFaceIndex && rightFaceIndex);

  const auto& leftFace = brushNode.brush().face(*leftFaceIndex);
  const auto& rightFace = brushNode.brush().face(*rightFaceIndex);

  const auto leftDot = vm::dot(leftFace.boundary().normal, pickRay.direction);
  const auto rightDot = vm::dot(rightFace.boundary().normal, pickRay.direction);

  if ((leftDot < 0.0) == (rightDot < 0.0))
  {
    // either both faces visible or both faces invisible
    return std::nullopt;
  }

  const auto leftFaceHandle = mdl::BrushFaceHandle{&brushNode, *leftFaceIndex};
  const auto rightFaceHandle = mdl::BrushFaceHandle{&brushNode, *rightFaceIndex};

  return {{leftFaceHandle, rightFaceHandle, leftDot, rightDot, segment, dist}};
}

std::vector<mdl::BrushFaceHandle> collectCoincidentFaces(
  const std::vector<mdl::Node*>& nodes,
  const mdl::BrushFaceHandle& faceHandle,
  const bool normalSign)
{
  auto result = std::vector<mdl::BrushFaceHandle>{};

  const auto& referenceFace = faceHandle.face();
  const auto referencePlane =
    normalSign ? referenceFace.boundary() : referenceFace.boundary().flip();

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](mdl::WorldNode&) {},
      [](mdl::LayerNode&) {},
      [](mdl::GroupNode&) {},
      [](mdl::EntityNode&) {},
      [&](mdl::BrushNode& brushNode) {
        const auto& brush = brushNode.brush();
        for (size_t i = 0; i < brush.faceCount(); ++i)
        {
          const auto& face = brush.face(i);
          if (!face.coplanarWith(referencePlane))
          {
            continue;
          }

          result.emplace_back(&brushNode, i);
        }
      },
      [](mdl::PatchNode&) {}));
  }

  return result;
}

std::vector<mdl::BrushFaceHandle> collectCoplanarFaces(
  const std::vector<mdl::Node*>& nodes, const mdl::BrushFaceHandle& faceHandle)
{
  return collectCoincidentFaces(nodes, faceHandle, K(normalSign));
}


std::vector<mdl::BrushFaceHandle> collectOpposingFaces(
  const std::vector<mdl::Node*>& nodes, const mdl::BrushFaceHandle& faceHandle)
{
  return collectCoincidentFaces(nodes, faceHandle, !K(normalSign));
}

std::optional<EdgeInfo> findClosestHorizonEdge(
  const std::vector<mdl::Node*>& nodes, const vm::ray3d& pickRay)
{
  auto result = std::optional<EdgeInfo>{};
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](mdl::WorldNode&) {},
      [](mdl::LayerNode&) {},
      [](mdl::GroupNode&) {},
      [](mdl::EntityNode&) {},
      [&](mdl::BrushNode& brushNode) {
        for (const auto* edge : brushNode.brush().edges())
        {
          result = std::min(result, getEdgeInfo(edge, brushNode, pickRay));
        }
      },
      [](mdl::PatchNode&) {}));
  }
  return result;
}

/**
 * Returns every horizon edge of the given nodes (i.e. all candidates that could possibly
 * compete for the edge handle at the current pick ray), unlike findClosestHorizonEdge,
 * which only returns the single closest one.
 */
std::vector<EdgeInfo> findHorizonEdges(
  const std::vector<mdl::Node*>& nodes, const vm::ray3d& pickRay)
{
  auto result = std::vector<EdgeInfo>{};
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](mdl::WorldNode&) {},
      [](mdl::LayerNode&) {},
      [](mdl::GroupNode&) {},
      [](mdl::EntityNode&) {},
      [&](mdl::BrushNode& brushNode) {
        for (const auto* edge : brushNode.brush().edges())
        {
          if (auto edgeInfo = getEdgeInfo(edge, brushNode, pickRay))
          {
            result.push_back(std::move(*edgeInfo));
          }
        }
      },
      [](mdl::PatchNode&) {}));
  }
  return result;
}

/**
 * Returns true if the given edge should take priority over the given direct brush face
 * hit. The edge's own adjacent face never steals the handle; a different, occluding face
 * only wins if it is nearer along the ray than the edge.
 */
bool edgeHandleWinsOverFace(
  const gl::Camera& camera,
  const vm::ray3d& pickRay,
  const EdgeInfo& edgeInfo,
  const mdl::Hit& faceHit)
{
  const auto edgeDistance = camera.pickLineSegmentHandle(
    pickRay, edgeInfo.segment, pref(Preferences::HandleRadius));
  contract_assert(edgeDistance);

  const auto faceHandle = hitToFaceHandle(faceHit);
  const auto hitIsAdjacentFace =
    faceHandle
    && (*faceHandle == edgeInfo.leftFaceHandle || *faceHandle == edgeInfo.rightFaceHandle);
  return hitIsAdjacentFace || *edgeDistance <= faceHit.distance();
}

/**
 * Returns true if some edge of a brush other than excludeBrush is collinear with, and
 * genuinely overlaps (shares more than a single point with), the given segment. Unlike a
 * horizon edge, the partner edge does not itself need to pass the front/back-facing test:
 * at a seam between two flush brushes, only the brush whose own face pair straddles the
 * silhouette (one face visible, one hidden) produces a horizon edge there; the other
 * brush's coincident edge typically has both of its adjacent faces on the same side of
 * that test (their shared visibility ignores that the near brush occludes it), so it is
 * never classified as a horizon edge itself. Its mere existence as a plain, overlapping
 * edge belonging to a different brush is what proves the seam is real.
 */
bool hasOverlappingEdgeInOtherBrush(
  const std::vector<mdl::Node*>& nodes,
  const mdl::BrushNode* excludeBrush,
  const vm::segment3d& segment)
{
  auto eligibleNodes =
    nodes | std::views::filter([&](const auto* node) { return node != excludeBrush; });

  return std::ranges::any_of(eligibleNodes, [&](const auto* node) {
    return node->accept(kdl::overload(
      [](const mdl::WorldNode&) { return false; },
      [](const mdl::LayerNode&) { return false; },
      [](const mdl::GroupNode&) { return false; },
      [](const mdl::EntityNode&) { return false; },
      [&](const mdl::BrushNode& brushNode) {
        return std::ranges::any_of(brushNode.brush().edges(), [&](const auto* edge) {
          return vm::segments_overlap(segment, edge->segment(), vm::Cd::almost_zero());
        });
      },
      [](const mdl::PatchNode&) { return false; }));
  });
}

/**
 * Returns the horizon edge, if any, that should override the given direct brush face hit.
 * A single nearby horizon edge is never enough on its own -- it is only eligible if it is
 * within the full handle radius *and* some other brush has a plain edge that is collinear
 * with, and genuinely overlaps, it. This is the geometric signature of two separate
 * brushes meeting flush at a seam, which is what makes the touching-face feature possible
 * in the first place; a face's own distinct edges merely clustering near the cursor does
 * not qualify, since they all belong to the same brush. Of the eligible edges, the
 * closest one still has to win out over a closer, non-adjacent occluding face.
 */
std::optional<EdgeInfo> selectOverridingEdge(
  const std::vector<mdl::Node*>& nodes,
  const gl::Camera& camera,
  const vm::ray3d& pickRay,
  const std::vector<EdgeInfo>& edgeInfos,
  const mdl::Hit& faceHit)
{
  auto eligible = edgeInfos | std::views::filter([&](const auto& edgeInfo) {
                    return camera.pickLineSegmentHandle(
                             pickRay, edgeInfo.segment, pref(Preferences::HandleRadius))
                           && hasOverlappingEdgeInOtherBrush(
                             nodes, edgeInfo.leftFaceHandle.node(), edgeInfo.segment);
                  });

  if (eligible.empty())
  {
    return std::nullopt;
  }

  const auto& winner = *std::ranges::min_element(eligible);
  return edgeHandleWinsOverFace(camera, pickRay, winner, faceHit) ? std::optional{winner}
                                                                  : std::nullopt;
}

std::vector<ExtrudeDragHandle> getDragHandles(
  const std::vector<mdl::Node*>& nodes, const mdl::Hit& hit)
{
  if (!hit.isMatch())
  {
    return {};
  }

  contract_assert(hit.hasType(ExtrudeTool::ExtrudeHitType));
  const auto& data = hit.target<const ExtrudeHitData&>();

  return kdl::views::concat(
           collectCoplanarFaces(nodes, data.face), collectOpposingFaces(nodes, data.face))
         | std::views::transform(
           [](const auto& faceHandle) { return ExtrudeDragHandle{faceHandle}; })
         | kdl::ranges::to<std::vector>();
}

/**
 * Splits off new brush "outward" from the drag handles.
 *
 * Returns false if the given delta isn't suitable for splitting "outward".
 *
 * Otherwise:
 * - rolls back the transaction
 * - applies a split outward with the given delta
 * - sets m_totalDelta to the given delta
 * - returns true
 */
bool splitBrushesOutward(
  mdl::Map& map, const vm::vec3d& delta, ExtrudeDragState& dragState)
{
  const auto& worldBounds = map.worldBounds();
  const bool lockAlignment = pref(Preferences::AlignmentLock);

  // First ensure that the drag can be applied at all. For this, check whether each drag
  // handle is moved "up" along its normal.
  for (const auto& dragHandle : dragState.initialDragHandles)
  {
    const auto& normal = dragHandle.faceNormal();
    if (vm::dot(normal, delta) <= double{0})
    {
      return false;
    }
  }

  auto newDragFaces = std::vector<mdl::BrushFaceHandle>{};
  auto newNodes = std::map<mdl::Node*, std::vector<mdl::Node*>>{};

  return dragState.initialDragHandles
         | std::views::transform([&](const auto& dragHandle) {
             auto* brushNode = dragHandle.faceHandle.node();

             const auto& oldBrush = dragHandle.brushAtDragStart;
             const auto dragFaceIndex = dragHandle.faceHandle.faceIndex();
             const auto newDragFaceNormal = dragHandle.faceNormal();

             auto newBrush = oldBrush;
             return newBrush.moveBoundary(
                      worldBounds, dragFaceIndex, delta, lockAlignment)
                    | kdl::and_then([&]() {
                        auto clipFace = oldBrush.face(dragFaceIndex);
                        clipFace.invert();
                        return newBrush.clip(worldBounds, std::move(clipFace));
                      })
                    | kdl::transform([&]() {
                        auto* newBrushNode = new mdl::BrushNode(std::move(newBrush));
                        newNodes[brushNode->parent()].push_back(newBrushNode);

                        // Look up the new face index of the new drag handle
                        if (
                          const auto newDragFaceIndex =
                            newBrushNode->brush().findFace(newDragFaceNormal))
                        {
                          newDragFaces.push_back(
                            mdl::BrushFaceHandle(newBrushNode, *newDragFaceIndex));
                        }
                      });
           })
         | kdl::fold | kdl::transform([&]() {
             // Apply the changes calculated above
             map.rollbackTransaction();

             deselectAll(map);
             const auto addedNodes = addNodes(map, newNodes);
             selectNodes(map, addedNodes);
             dragState.currentDragFaces = std::move(newDragFaces);
             dragState.totalDelta = delta;
           })
         | kdl::transform_error([&](auto e) {
             map.logger().error() << "Could not extrude brush: " << e;
             kdl::map_clear_and_delete(newNodes);
           })
         | kdl::is_success();
}

/**
 * Splits brushes "inwards" effectively clipping the selected brushes into two halves.
 *
 * Returns false if the given delta isn't suitable for splitting inward.
 *
 * Otherwise:
 * - rolls back the transaction
 * - applies a split inward with the given delta
 * - sets m_totalDelta to the given delta
 * - returns true
 */
bool splitBrushesInward(
  mdl::Map& map, const vm::vec3d& delta, ExtrudeDragState& dragState)
{
  const auto& worldBounds = map.worldBounds();
  const bool lockAlignment = pref(Preferences::AlignmentLock);

  // First ensure that the drag can be applied at all. For this, check whether each drag
  // handle is moved "down" along its normal.
  for (const auto& dragHandle : dragState.initialDragHandles)
  {
    const auto& normal = dragHandle.faceNormal();
    if (vm::dot(normal, delta) > double{0})
    {
      return false;
    }
  }

  auto newDragFaces = std::vector<mdl::BrushFaceHandle>{};
  // This map is to handle the case when the brushes being
  // extruded have different parents (e.g. different brush entities),
  // so each newly created brush should be made a sibling of the brush it was cloned
  // from.
  auto newNodes = std::map<mdl::Node*, std::vector<mdl::Node*>>{};
  auto nodesToUpdate = std::vector<std::pair<mdl::Node*, mdl::NodeContents>>{};

  for (const auto& dragHandle : dragState.initialDragHandles)
  {
    auto* brushNode = dragHandle.faceHandle.node();

    // "Front" means the part closer to the drag handles at the drag start
    auto frontBrush = dragHandle.brushAtDragStart;
    auto backBrush = dragHandle.brushAtDragStart;

    auto clipFace = frontBrush.face(dragHandle.faceHandle.faceIndex());

    if (clipFace.transform(vm::translation_matrix(delta), lockAlignment).is_error())
    {
      map.logger().error() << "Could not extrude inwards: Error transforming face";
      kdl::map_clear_and_delete(newNodes);
      return false;
    }

    auto clipFaceInverted = clipFace;
    clipFaceInverted.invert();

    // Front brush should always be valid
    if (frontBrush.clip(worldBounds, clipFaceInverted).is_error())
    {
      map.logger().error() << "Could not extrude inwards: Front brush is empty";
      kdl::map_clear_and_delete(newNodes);
      return false;
    }

    nodesToUpdate.emplace_back(brushNode, std::move(frontBrush));

    // Back brush
    if (backBrush.clip(worldBounds, clipFace))
    {
      auto* newBrushNode = new mdl::BrushNode(std::move(backBrush));
      newNodes[brushNode->parent()].push_back(newBrushNode);

      // Look up the new face index of the new drag handle
      if (const auto newDragFaceIndex = newBrushNode->brush().findFace(clipFace.normal()))
      {
        newDragFaces.emplace_back(newBrushNode, *newDragFaceIndex);
      }
    }
  }

  // Apply changes calculated above

  dragState.currentDragFaces.clear();
  map.rollbackTransaction();

  // FIXME: deal with linked group update failure (needed for #3647)
  const bool success = updateNodeContents(map, "Resize Brushes", nodesToUpdate);
  unused(success);

  // Add the newly split off brushes and select them (keeping the original brushes
  // selected).
  // FIXME: deal with linked group update failure (needed for #3647)
  const auto addedNodes = addNodes(map, newNodes);
  selectNodes(map, addedNodes);

  dragState.currentDragFaces = std::move(newDragFaces);
  dragState.totalDelta = delta;

  return true;
}

std::vector<vm::polygon3d> getPolygons(const std::vector<ExtrudeDragHandle>& dragHandles)
{
  return dragHandles | std::views::transform([](const auto& dragHandle) {
           return dragHandle.brushAtDragStart.face(dragHandle.faceHandle.faceIndex())
             .polygon();
         })
         | kdl::ranges::to<std::vector>();
}

} // namespace

// DragHandle

ExtrudeDragHandle::ExtrudeDragHandle(mdl::BrushFaceHandle i_faceHandle)
  : faceHandle{std::move(i_faceHandle)}
  , brushAtDragStart{faceHandle.node()->brush()}
{
}

const mdl::BrushFace& ExtrudeDragHandle::faceAtDragStart() const
{
  return brushAtDragStart.face(faceHandle.faceIndex());
}

vm::vec3d ExtrudeDragHandle::faceNormal() const
{
  return faceAtDragStart().normal();
}

kdl_reflect_impl(ExtrudeDragHandle);

kdl_reflect_impl(ExtrudeDragState);

kdl_reflect_impl(ExtrudeHitData);

// ExtrudeTool

const mdl::HitType::Type ExtrudeTool::ExtrudeHitType = mdl::HitType::freeType();

ExtrudeTool::ExtrudeTool(MapDocument& document)
  : Tool{true}
  , m_document{document}
  , m_dragging{false}
{
  connectObservers();
}

bool ExtrudeTool::applies() const
{
  return m_document.map().selection().hasBrushes();
}

const mdl::Grid& ExtrudeTool::grid() const
{
  return m_document.map().grid();
}

mdl::Hit ExtrudeTool::pick2D(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const mdl::PickResult& pickResult) const
{
  using namespace mdl::HitFilters;

  const auto makeEdgeHit = [&](const EdgeInfo& edgeInfo) -> mdl::Hit {
    const auto& [leftFaceHandle, rightFaceHandle, leftDot, rightDot, segment, distance] =
      edgeInfo;
    const auto hitPoint = vm::point_at_distance(pickRay, distance.position1);
    const auto handlePosition = vm::point_at_distance(segment, distance.position2);

    // Select the face that is perpendicular to the view direction or the back facing
    // one.
    if (
      leftDot >= -vm::Cd::almost_zero() && !vm::is_zero(rightDot, vm::Cd::almost_zero()))
    {
      return {
        ExtrudeHitType,
        distance.position1,
        hitPoint,
        ExtrudeHitData{
          leftFaceHandle,
          vm::plane3d{handlePosition, pickRay.direction},
          handlePosition}};
    }
    return {
      ExtrudeHitType,
      distance.position1,
      hitPoint,
      ExtrudeHitData{
        rightFaceHandle, vm::plane3d{handlePosition, pickRay.direction}, handlePosition}};
  };

  const auto& hit = pickResult.first(type(mdl::BrushNode::BrushHitType) && selected());

  if (hit.isMatch())
  {
    // The cursor is over a brush face. Only engage if a horizon edge handle is grabbed.
    const auto& nodes = m_document.map().selection().nodes;
    const auto edgeInfos = findHorizonEdges(nodes, pickRay);
    if (
      const auto edgeInfo = selectOverridingEdge(nodes, camera, pickRay, edgeInfos, hit))
    {
      return makeEdgeHit(*edgeInfo);
    }
    return mdl::Hit::NoHit;
  }

  const auto edgeInfo =
    findClosestHorizonEdge(m_document.map().selection().nodes, pickRay);
  if (!edgeInfo)
  {
    return mdl::Hit::NoHit;
  }
  return makeEdgeHit(*edgeInfo);
}

mdl::Hit ExtrudeTool::pick3D(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const mdl::PickResult& pickResult) const
{
  using namespace mdl::HitFilters;

  const auto makeEdgeHit = [&](const EdgeInfo& edgeInfo) -> mdl::Hit {
    const auto& [leftFaceHandle, rightFaceHandle, leftDot, rightDot, segment, distance] =
      edgeInfo;
    const auto hitPoint = vm::point_at_distance(pickRay, distance.position1);
    const auto handlePosition = vm::point_at_distance(segment, distance.position2);

    // choose the face that we are seeing from behind
    const auto dragFaceHandle = leftDot > rightDot ? leftFaceHandle : rightFaceHandle;
    const auto referenceFaceHandle =
      leftDot > rightDot ? rightFaceHandle : leftFaceHandle;

    return {
      ExtrudeHitType,
      distance.position1,
      hitPoint,
      ExtrudeHitData{
        dragFaceHandle,
        vm::plane3d{handlePosition, referenceFaceHandle.face().normal()},
        handlePosition}};
  };

  const auto& hit = pickResult.first(type(mdl::BrushNode::BrushHitType) && selected());
  const auto faceHandle = hitToFaceHandle(hit);

  if (faceHandle)
  {
    // The cursor is over a brush face. Prefer it unless a horizon edge handle is grabbed.
    const auto& nodes = m_document.map().selection().nodes;
    const auto edgeInfos = findHorizonEdges(nodes, pickRay);
    if (
      const auto edgeInfo = selectOverridingEdge(nodes, camera, pickRay, edgeInfos, hit))
    {
      return makeEdgeHit(*edgeInfo);
    }
    return {
      ExtrudeHitType,
      hit.distance(),
      hit.hitPoint(),
      ExtrudeHitData{
        *faceHandle,
        vm::line3d{hit.hitPoint(), faceHandle->face().normal()},
        hit.hitPoint()}};
  }

  const auto edgeInfo =
    findClosestHorizonEdge(m_document.map().selection().nodes, pickRay);
  if (!edgeInfo)
  {
    return mdl::Hit::NoHit;
  }
  return makeEdgeHit(*edgeInfo);
}

const std::vector<ExtrudeDragHandle>& ExtrudeTool::proposedDragHandles() const
{
  return m_proposedDragHandles;
}

void ExtrudeTool::updateProposedDragHandles(const mdl::PickResult& pickResult)
{
  using namespace mdl::HitFilters;

  auto& map = m_document.map();
  if (m_dragging)
  {
    // FIXME: this should be turned into an ensure failure, but it's easy to make it
    // fail currently by spamming drags/modifiers. Indicates a bug in
    // ExtrudeToolController thinking we are not dragging when we actually still are.
    map.logger().error() << "updateProposedDragHandles called during a drag";
    return;
  }

  const auto& hit = pickResult.first(type(ExtrudeHitType));
  const auto& nodes = map.selection().nodes;

  auto newDragHandles = getDragHandles(nodes, hit);
  if (newDragHandles != m_proposedDragHandles)
  {
    m_proposedDragHandles = std::move(newDragHandles);
    refreshViews();
  }
}

std::vector<mdl::BrushFaceHandle> ExtrudeTool::getDragFaces(
  const std::vector<ExtrudeDragHandle>& dragHandles)
{
  auto dragFaces = std::vector<mdl::BrushFaceHandle>{};
  dragFaces.reserve(dragHandles.size());

  for (const auto& dragHandle : dragHandles)
  {
    const auto& brush = dragHandle.faceHandle.node()->brush();
    if (const auto faceIndex = brush.findFace(dragHandle.faceNormal()))
    {
      dragFaces.emplace_back(dragHandle.faceHandle.node(), *faceIndex);
    }
  }

  return dragFaces;
}

/**
 * Starts resizing the faces determined by the previous call to
 * updateProposedDragHandles
 */
void ExtrudeTool::beginExtrude()
{
  contract_pre(!m_dragging);

  m_dragging = true;
  m_document.map().startTransaction("Resize Brushes", mdl::TransactionScope::LongRunning);
}

bool ExtrudeTool::extrude(const vm::vec3d& handleDelta, ExtrudeDragState& dragState)
{
  contract_pre(m_dragging);

  auto& map = m_document.map();
  if (dragState.splitBrushes)
  {
    if (
      splitBrushesOutward(map, handleDelta, dragState)
      || splitBrushesInward(map, handleDelta, dragState))
    {
      return true;
    }
  }
  else
  {
    map.rollbackTransaction();
    if (extrudeBrushes(map, getPolygons(dragState.initialDragHandles), handleDelta))
    {
      dragState.totalDelta = handleDelta;
    }
    else
    {
      // extrudeBrushes() fails if some brushes were completely clipped away.
      // In that case, restore the last m_totalDelta to be successfully applied.
      extrudeBrushes(
        map, getPolygons(dragState.initialDragHandles), dragState.totalDelta);
    }
  }

  dragState.currentDragFaces = getDragFaces(m_proposedDragHandles);

  return true;
}

void ExtrudeTool::beginMove()
{
  contract_pre(!m_dragging);

  m_dragging = true;
  m_document.map().startTransaction("Move Faces", mdl::TransactionScope::LongRunning);
}

bool ExtrudeTool::move(const vm::vec3d& delta, ExtrudeDragState& dragState)
{
  contract_pre(m_dragging);

  auto& map = m_document.map();
  map.rollbackTransaction();
  if (transformFaces(
        map, getPolygons(dragState.initialDragHandles), vm::translation_matrix(delta)))
  {
    dragState.totalDelta = delta;
  }
  else
  {
    // restore the last successful position
    transformFaces(
      map,
      getPolygons(dragState.initialDragHandles),
      vm::translation_matrix(dragState.totalDelta));
  }

  dragState.currentDragFaces = getDragFaces(m_proposedDragHandles);

  return true;
}

void ExtrudeTool::commit(const ExtrudeDragState& dragState)
{
  contract_pre(m_dragging);

  auto& map = m_document.map();
  if (vm::is_zero(dragState.totalDelta, vm::Cd::almost_zero()))
  {
    map.cancelTransaction();
  }
  else
  {
    map.commitTransaction();
  }
  m_proposedDragHandles.clear();
  m_dragging = false;
}

void ExtrudeTool::cancel()
{
  contract_pre(m_dragging);

  m_document.map().cancelTransaction();
  m_proposedDragHandles.clear();
  m_dragging = false;
}

void ExtrudeTool::connectObservers()
{
  m_notifierConnection += m_document.documentWasLoadedNotifier.connect(
    [&] { clearDragHandlesIfNotDragging(); });
  m_notifierConnection += m_document.documentDidChangeNotifier.connect(
    [&] { clearDragHandlesIfNotDragging(); });
  m_notifierConnection += m_document.selectionDidChangeNotifier.connect(
    [&](const auto&) { clearDragHandlesIfNotDragging(); });
}

void ExtrudeTool::clearDragHandlesIfNotDragging()
{
  if (!m_dragging)
  {
    m_proposedDragHandles.clear();
  }
}

} // namespace tb::ui
