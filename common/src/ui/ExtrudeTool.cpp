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

#include "ExtrudeTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushGeometry.h"
#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "mdl/Polyhedron.h"
#include "ui/MapDocument.h"
#include "ui/TransactionScope.h"

#include "kdl/map_utils.h"
#include "kdl/memory_utils.h"
#include "kdl/overload.h"
#include "kdl/reflection_impl.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/vector_utils.h"

#include "vm/distance.h"
#include "vm/line_io.h"  // IWYU pragma: keep
#include "vm/plane_io.h" // IWYU pragma: keep
#include "vm/polygon.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <map>
#include <vector>

namespace tb::ui
{

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

ExtrudeTool::ExtrudeTool(std::weak_ptr<MapDocument> document)
  : Tool{true}
  , m_document{std::move(document)}
  , m_dragging{false}
{
  connectObservers();
}

bool ExtrudeTool::applies() const
{
  auto document = kdl::mem_lock(m_document);
  return document->selection().hasBrushes();
}

const Grid& ExtrudeTool::grid() const
{
  return kdl::mem_lock(m_document)->grid();
}

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
  const mdl::BrushEdge* edge, mdl::BrushNode* brushNode, const vm::ray3d& pickRay)
{

  const auto segment = edge->segment();
  const auto dist = vm::distance(pickRay, segment);

  const auto leftFaceIndex = edge->firstFace()->payload();
  const auto rightFaceIndex = edge->secondFace()->payload();
  assert(leftFaceIndex && rightFaceIndex);

  const auto& leftFace = brushNode->brush().face(*leftFaceIndex);
  const auto& rightFace = brushNode->brush().face(*rightFaceIndex);

  const auto leftDot = vm::dot(leftFace.boundary().normal, pickRay.direction);
  const auto rightDot = vm::dot(rightFace.boundary().normal, pickRay.direction);

  if ((leftDot < 0.0) == (rightDot < 0.0))
  {
    // either both faces visible or both faces invisible
    return std::nullopt;
  }

  const auto leftFaceHandle = mdl::BrushFaceHandle{brushNode, *leftFaceIndex};
  const auto rightFaceHandle = mdl::BrushFaceHandle{brushNode, *rightFaceIndex};

  return {{leftFaceHandle, rightFaceHandle, leftDot, rightDot, segment, dist}};
}

std::optional<EdgeInfo> findClosestHorizonEdge(
  const std::vector<mdl::Node*>& nodes, const vm::ray3d& pickRay)
{
  auto result = std::optional<EdgeInfo>{};
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [](mdl::GroupNode*) {},
      [](mdl::EntityNode*) {},
      [&](mdl::BrushNode* brushNode) {
        for (const auto* edge : brushNode->brush().edges())
        {
          result = std::min(result, getEdgeInfo(edge, brushNode, pickRay));
        }
      },
      [](mdl::PatchNode*) {}));
  }
  return result;
}
} // namespace

mdl::Hit ExtrudeTool::pick2D(
  const vm::ray3d& pickRay, const mdl::PickResult& pickResult) const
{
  using namespace mdl::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& hit = pickResult.first(type(mdl::BrushNode::BrushHitType) && selected());
  if (hit.isMatch())
  {
    return mdl::Hit::NoHit;
  }

  const auto edgeInfo = findClosestHorizonEdge(document->selection().nodes, pickRay);
  if (!edgeInfo)
  {
    return mdl::Hit::NoHit;
  }

  const auto [leftFaceHandle, rightFaceHandle, leftDot, rightDot, segment, distance] =
    *edgeInfo;
  const auto hitPoint = vm::point_at_distance(pickRay, distance.position1);
  const auto handlePosition = vm::point_at_distance(segment, distance.position2);

  // Select the face that is perpendicular to the view direction or the back facing one.
  if (leftDot >= -vm::Cd::almost_zero() && !vm::is_zero(rightDot, vm::Cd::almost_zero()))
  {
    return {
      ExtrudeHitType,
      distance.position1,
      hitPoint,
      ExtrudeHitData{
        leftFaceHandle, vm::plane3d{handlePosition, pickRay.direction}, handlePosition}};
  }
  return {
    ExtrudeHitType,
    distance.position1,
    hitPoint,
    ExtrudeHitData{
      rightFaceHandle, vm::plane3d{handlePosition, pickRay.direction}, handlePosition}};
}

mdl::Hit ExtrudeTool::pick3D(
  const vm::ray3d& pickRay, const mdl::PickResult& pickResult) const
{
  using namespace mdl::HitFilters;

  auto document = kdl::mem_lock(m_document);

  const auto& hit = pickResult.first(type(mdl::BrushNode::BrushHitType) && selected());
  if (const auto faceHandle = hitToFaceHandle(hit))
  {
    return {
      ExtrudeHitType,
      hit.distance(),
      hit.hitPoint(),
      ExtrudeHitData{
        *faceHandle,
        vm::line3d{hit.hitPoint(), faceHandle->face().normal()},
        hit.hitPoint()}};
  }

  const auto edgeInfo = findClosestHorizonEdge(document->selection().nodes, pickRay);
  if (!edgeInfo)
  {
    return mdl::Hit::NoHit;
  }

  const auto [leftFaceHandle, rightFaceHandle, leftDot, rightDot, segment, distance] =
    *edgeInfo;
  const auto hitPoint = vm::point_at_distance(pickRay, distance.position1);
  const auto handlePosition = vm::point_at_distance(segment, distance.position2);

  // choose the face that we are seeing from behind
  const auto dragFaceHandle = leftDot > rightDot ? leftFaceHandle : rightFaceHandle;
  const auto referenceFaceHandle = leftDot > rightDot ? rightFaceHandle : leftFaceHandle;

  return {
    ExtrudeHitType,
    distance.position1,
    hitPoint,
    ExtrudeHitData{
      dragFaceHandle,
      vm::plane3d{handlePosition, referenceFaceHandle.face().normal()},
      handlePosition}};
}

const std::vector<ExtrudeDragHandle>& ExtrudeTool::proposedDragHandles() const
{
  return m_proposedDragHandles;
}

namespace
{
std::vector<mdl::BrushFaceHandle> collectCoplanarFaces(
  const std::vector<mdl::Node*>& nodes, const mdl::BrushFaceHandle& faceHandle)
{
  auto result = std::vector<mdl::BrushFaceHandle>{};

  const auto& referenceFace = faceHandle.face();
  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](mdl::WorldNode*) {},
      [](mdl::LayerNode*) {},
      [](mdl::GroupNode*) {},
      [](mdl::EntityNode*) {},
      [&](mdl::BrushNode* brushNode) {
        const auto& brush = brushNode->brush();
        for (size_t i = 0; i < brush.faceCount(); ++i)
        {
          const auto& face = brush.face(i);
          if (!face.coplanarWith(referenceFace.boundary()))
          {
            continue;
          }

          result.emplace_back(brushNode, i);
        }
      },
      [](mdl::PatchNode*) {}));
  }

  return result;
}

std::vector<ExtrudeDragHandle> getDragHandles(
  const std::vector<mdl::Node*>& nodes, const mdl::Hit& hit)
{
  if (!hit.isMatch())
  {
    return {};
  }

  assert(hit.hasType(ExtrudeTool::ExtrudeHitType));
  const auto& data = hit.target<const ExtrudeHitData&>();

  return kdl::vec_transform(
    collectCoplanarFaces(nodes, data.face),
    [](const auto& handle) { return ExtrudeDragHandle{handle}; });
}
} // namespace

void ExtrudeTool::updateProposedDragHandles(const mdl::PickResult& pickResult)
{
  using namespace mdl::HitFilters;

  auto document = kdl::mem_lock(m_document);
  if (m_dragging)
  {
    // FIXME: this should be turned into an ensure failure, but it's easy to make it
    // fail currently by spamming drags/modifiers. Indicates a bug in
    // ExtrudeToolController thinking we are not dragging when we actually still are.
    document->error() << "updateProposedDragHandles called during a drag";
    return;
  }

  const auto& hit = pickResult.first(type(ExtrudeHitType));
  const auto& nodes = document->selection().nodes;

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
  ensure(!m_dragging, "may not be called during a drag");
  m_dragging = true;
  kdl::mem_lock(m_document)
    ->startTransaction("Resize Brushes", TransactionScope::LongRunning);
}

namespace
{

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
  MapDocument& document, const vm::vec3d& delta, ExtrudeDragState& dragState)
{
  const auto& worldBounds = document.worldBounds();
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

  return kdl::vec_transform(
           dragState.initialDragHandles,
           [&](const auto& dragHandle) {
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
             document.rollbackTransaction();

             document.deselectAll();
             const auto addedNodes = document.addNodes(newNodes);
             document.selectNodes(addedNodes);
             dragState.currentDragFaces = std::move(newDragFaces);
             dragState.totalDelta = delta;
           })
         | kdl::transform_error([&](auto e) {
             document.error() << "Could not extrude brush: " << e;
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
  MapDocument& document, const vm::vec3d& delta, ExtrudeDragState& dragState)
{
  const auto& worldBounds = document.worldBounds();
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
      document.error() << "Could not extrude inwards: Error transforming face";
      kdl::map_clear_and_delete(newNodes);
      return false;
    }

    auto clipFaceInverted = clipFace;
    clipFaceInverted.invert();

    // Front brush should always be valid
    if (frontBrush.clip(worldBounds, clipFaceInverted).is_error())
    {
      document.error() << "Could not extrude inwards: Front brush is empty";
      kdl::map_clear_and_delete(newNodes);
      return false;
    }

    nodesToUpdate.emplace_back(brushNode, std::move(frontBrush));

    // Back brush
    if (backBrush.clip(worldBounds, clipFace).is_success())
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
  document.rollbackTransaction();

  // FIXME: deal with linked group update failure (needed for #3647)
  const bool success = document.swapNodeContents("Resize Brushes", nodesToUpdate);
  unused(success);

  // Add the newly split off brushes and select them (keeping the original brushes
  // selected).
  // FIXME: deal with linked group update failure (needed for #3647)
  const auto addedNodes = document.addNodes(newNodes);
  document.selectNodes(addedNodes);

  dragState.currentDragFaces = std::move(newDragFaces);
  dragState.totalDelta = delta;

  return true;
}

std::vector<vm::polygon3d> getPolygons(const std::vector<ExtrudeDragHandle>& dragHandles)
{
  return kdl::vec_transform(dragHandles, [](const auto& dragHandle) {
    return dragHandle.brushAtDragStart.face(dragHandle.faceHandle.faceIndex()).polygon();
  });
}
} // namespace

bool ExtrudeTool::extrude(const vm::vec3d& handleDelta, ExtrudeDragState& dragState)
{
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);

  if (dragState.splitBrushes)
  {
    if (
      splitBrushesOutward(*document, handleDelta, dragState)
      || splitBrushesInward(*document, handleDelta, dragState))
    {
      return true;
    }
  }
  else
  {
    document->rollbackTransaction();
    if (document->extrudeBrushes(getPolygons(dragState.initialDragHandles), handleDelta))
    {
      dragState.totalDelta = handleDelta;
    }
    else
    {
      // extrudeBrushes() fails if some brushes were completely clipped away.
      // In that case, restore the last m_totalDelta to be successfully applied.
      document->extrudeBrushes(
        getPolygons(dragState.initialDragHandles), dragState.totalDelta);
    }
  }

  dragState.currentDragFaces = getDragFaces(m_proposedDragHandles);

  return true;
}

void ExtrudeTool::beginMove()
{
  ensure(!m_dragging, "may not be called during a drag");
  m_dragging = true;
  kdl::mem_lock(m_document)
    ->startTransaction("Move Faces", TransactionScope::LongRunning);
}

bool ExtrudeTool::move(const vm::vec3d& delta, ExtrudeDragState& dragState)
{
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);

  document->rollbackTransaction();
  if (document->transformFaces(
        getPolygons(dragState.initialDragHandles), vm::translation_matrix(delta)))
  {
    dragState.totalDelta = delta;
  }
  else
  {
    // restore the last successful position
    document->transformFaces(
      getPolygons(dragState.initialDragHandles),
      vm::translation_matrix(dragState.totalDelta));
  }

  dragState.currentDragFaces = getDragFaces(m_proposedDragHandles);

  return true;
}

void ExtrudeTool::commit(const ExtrudeDragState& dragState)
{
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);
  if (vm::is_zero(dragState.totalDelta, vm::Cd::almost_zero()))
  {
    document->cancelTransaction();
  }
  else
  {
    document->commitTransaction();
  }
  m_proposedDragHandles.clear();
  m_dragging = false;
}

void ExtrudeTool::cancel()
{
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);
  document->cancelTransaction();
  m_proposedDragHandles.clear();
  m_dragging = false;
}

void ExtrudeTool::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &ExtrudeTool::nodesDidChange);
  m_notifierConnection +=
    document->nodesWillChangeNotifier.connect(this, &ExtrudeTool::nodesDidChange);
  m_notifierConnection +=
    document->nodesWillBeRemovedNotifier.connect(this, &ExtrudeTool::nodesDidChange);
  m_notifierConnection +=
    document->selectionDidChangeNotifier.connect(this, &ExtrudeTool::selectionDidChange);
}

void ExtrudeTool::nodesDidChange(const std::vector<mdl::Node*>&)
{
  if (!m_dragging)
  {
    m_proposedDragHandles.clear();
  }
}

void ExtrudeTool::selectionDidChange(const SelectionChange&)
{
  if (!m_dragging)
  {
    m_proposedDragHandles.clear();
  }
}

} // namespace tb::ui
