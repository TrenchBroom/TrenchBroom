/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "ResizeBrushesTool.h"

#include "Exceptions.h"
#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"

#include <kdl/collection_utils.h>
#include <kdl/map_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/overload.h>
#include <kdl/reflection_impl.h>
#include <kdl/result.h>
#include <kdl/result_for_each.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <vecmath/distance.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <limits>
#include <map>
#include <vector>

namespace TrenchBroom::View {

// DragHandle

ResizeDragHandle::ResizeDragHandle(Model::BrushFaceHandle i_faceHandle)
  : faceHandle{std::move(i_faceHandle)}
  , brushAtDragStart{faceHandle.node()->brush()} {}

const Model::BrushFace& ResizeDragHandle::faceAtDragStart() const {
  return brushAtDragStart.face(faceHandle.faceIndex());
}

vm::vec3 ResizeDragHandle::faceNormal() const {
  return faceAtDragStart().normal();
}

kdl_reflect_impl(ResizeDragHandle);

kdl_reflect_impl(ResizeDragState);

// ResizeBrushesTool

const Model::HitType::Type ResizeBrushesTool::Resize2DHitType = Model::HitType::freeType();
const Model::HitType::Type ResizeBrushesTool::Resize3DHitType = Model::HitType::freeType();

ResizeBrushesTool::ResizeBrushesTool(std::weak_ptr<MapDocument> document)
  : Tool{true}
  , m_document{std::move(document)}
  , m_dragging{false} {
  connectObservers();
}

bool ResizeBrushesTool::applies() const {
  auto document = kdl::mem_lock(m_document);
  return document->selectedNodes().hasBrushes();
}

const Grid& ResizeBrushesTool::grid() const {
  return kdl::mem_lock(m_document)->grid();
}

namespace {
struct EdgeInfo {
  Model::BrushFaceHandle leftFaceHandle;
  Model::BrushFaceHandle rightFaceHandle;
  FloatType leftDot;
  FloatType rightDot;
  vm::line_distance<FloatType> dist;
};

bool operator<(const std::optional<EdgeInfo>& lhs, const std::optional<EdgeInfo>& rhs) {
  return lhs == std::nullopt   ? false
         : rhs == std::nullopt ? true
                               : lhs->dist.distance < rhs->dist.distance;
}

std::optional<EdgeInfo> getEdgeInfo(
  const Model::BrushEdge* edge, Model::BrushNode* brushNode, const vm::ray3& pickRay) {

  const auto dist = vm::distance(pickRay, edge->segment());
  if (vm::is_nan(dist.distance)) {
    return std::nullopt;
  }

  const auto leftFaceIndex = edge->firstFace()->payload();
  const auto rightFaceIndex = edge->secondFace()->payload();
  assert(leftFaceIndex && rightFaceIndex);

  const auto& leftFace = brushNode->brush().face(*leftFaceIndex);
  const auto& rightFace = brushNode->brush().face(*rightFaceIndex);

  const auto leftDot = vm::dot(leftFace.boundary().normal, pickRay.direction);
  const auto rightDot = vm::dot(rightFace.boundary().normal, pickRay.direction);

  if ((leftDot < 0.0) == (rightDot < 0.0)) {
    // either both faces visible or both faces invisible
    return std::nullopt;
  }

  const auto leftFaceHandle = Model::BrushFaceHandle{brushNode, *leftFaceIndex};
  const auto rightFaceHandle = Model::BrushFaceHandle{brushNode, *rightFaceIndex};

  return {{leftFaceHandle, rightFaceHandle, leftDot, rightDot, dist}};
}

std::optional<EdgeInfo> findClosestHorizonEdge(
  const std::vector<Model::Node*>& nodes, const vm::ray3& pickRay) {
  auto result = std::optional<EdgeInfo>{};
  for (auto* node : nodes) {
    node->accept(kdl::overload(
      [](Model::WorldNode*) {}, [](Model::LayerNode*) {}, [](Model::GroupNode*) {},
      [](Model::EntityNode*) {},
      [&](Model::BrushNode* brushNode) {
        for (const auto* edge : brushNode->brush().edges()) {
          result = std::min(result, getEdgeInfo(edge, brushNode, pickRay));
        }
      },
      [](Model::PatchNode*) {}));
  }
  return result;
}
} // namespace

Model::Hit ResizeBrushesTool::pick2D(
  const vm::ray3& pickRay, const Model::PickResult& pickResult) const {
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);
  const auto& hit = pickResult.first(type(Model::BrushNode::BrushHitType) && selected());
  if (hit.isMatch()) {
    return Model::Hit::NoHit;
  }

  const auto edgeInfo = findClosestHorizonEdge(document->selectedNodes().nodes(), pickRay);
  if (!edgeInfo) {
    return Model::Hit::NoHit;
  }

  const auto [leftFaceHandle, rightFaceHandle, leftDot, rightDot, distance] = *edgeInfo;
  const auto hitPoint = vm::point_at_distance(pickRay, distance.position1);

  if (vm::is_zero(leftDot, vm::C::almost_zero())) {
    return {
      Resize2DHitType, distance.position1, hitPoint,
      std::vector<Model::BrushFaceHandle>{leftFaceHandle}};
  } else if (vm::is_zero(rightDot, vm::C::almost_zero())) {
    return {
      Resize2DHitType, distance.position1, hitPoint,
      std::vector<Model::BrushFaceHandle>{rightFaceHandle}};
  } else {
    auto data = std::vector<Model::BrushFaceHandle>{};
    data.reserve(2);

    // only include if face isn't perpendicular to view direction
    if (vm::abs(leftDot) < 1.0) {
      data.push_back(leftFaceHandle);
    }
    if (vm::abs(rightDot) < 1.0) {
      data.push_back(rightFaceHandle);
    }
    return {Resize2DHitType, distance.position1, hitPoint, std::move(data)};
  }
}

Model::Hit ResizeBrushesTool::pick3D(
  const vm::ray3& pickRay, const Model::PickResult& pickResult) const {
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);

  const auto& hit = pickResult.first(type(Model::BrushNode::BrushHitType) && selected());
  if (const auto faceHandle = hitToFaceHandle(hit)) {
    return {Resize3DHitType, hit.distance(), hit.hitPoint(), *faceHandle};
  }

  const auto edgeInfo = findClosestHorizonEdge(document->selectedNodes().nodes(), pickRay);
  if (!edgeInfo) {
    return Model::Hit::NoHit;
  }

  const auto [leftFaceHandle, rightFaceHandle, leftDot, rightDot, distance] = *edgeInfo;
  const auto hitPoint = vm::point_at_distance(pickRay, distance.position1);

  // choose the face that we are seeing from behind
  return {
    Resize3DHitType, distance.position1, hitPoint,
    leftDot > rightDot ? leftFaceHandle : rightFaceHandle};
}

const std::vector<ResizeDragHandle>& ResizeBrushesTool::proposedDragHandles() const {
  return m_proposedDragHandles;
}

namespace {
std::vector<Model::BrushFaceHandle> collectDragFaces(
  const std::vector<Model::Node*>& nodes, const Model::BrushFaceHandle& faceHandle) {
  auto result = std::vector<Model::BrushFaceHandle>{};

  const auto& referenceFace = faceHandle.face();
  for (auto* node : nodes) {
    node->accept(kdl::overload(
      [](Model::WorldNode*) {}, [](Model::LayerNode*) {}, [](Model::GroupNode*) {},
      [](Model::EntityNode*) {},
      [&](Model::BrushNode* brushNode) {
        const auto& brush = brushNode->brush();
        for (size_t i = 0; i < brush.faceCount(); ++i) {
          const auto& face = brush.face(i);
          if (&face == &referenceFace) {
            continue;
          }

          if (!face.coplanarWith(referenceFace.boundary())) {
            continue;
          }

          result.emplace_back(brushNode, i);
        }
      },
      [](Model::PatchNode*) {}));
  }

  return result;
}

std::vector<ResizeDragHandle> collectDragHandles(
  const std::vector<Model::Node*>& nodes, const Model::Hit& hit) {
  assert(hit.isMatch());
  assert(
    hit.type() == ResizeBrushesTool::Resize2DHitType ||
    hit.type() == ResizeBrushesTool::Resize3DHitType);

  auto result = std::vector<Model::BrushFaceHandle>{};
  if (hit.type() == ResizeBrushesTool::Resize2DHitType) {
    const auto& data = hit.target<const ResizeBrushesTool::Resize2DHitData&>();
    assert(!data.empty());
    result = kdl::vec_concat(std::move(result), data, collectDragFaces(nodes, data[0]));
    if (data.size() > 1) {
      result = kdl::vec_concat(std::move(result), collectDragFaces(nodes, data[1]));
    }
  } else {
    const auto& data = hit.target<const ResizeBrushesTool::Resize3DHitData&>();
    result.push_back(data);
    result = kdl::vec_concat(std::move(result), collectDragFaces(nodes, data));
  }

  return kdl::vec_transform(result, [](const auto& handle) {
    return ResizeDragHandle{handle};
  });
}

std::vector<ResizeDragHandle> getDragHandles(
  const std::vector<Model::Node*>& nodes, const Model::Hit& hit) {
  if (hit.isMatch()) {
    return collectDragHandles(nodes, hit);
  } else {
    return std::vector<ResizeDragHandle>{};
  }
}
} // namespace

void ResizeBrushesTool::updateProposedDragHandles(const Model::PickResult& pickResult) {
  using namespace Model::HitFilters;

  auto document = kdl::mem_lock(m_document);
  if (m_dragging) {
    // FIXME: this should be turned into an ensure failure, but it's easy to make it fail
    // currently by spamming drags/modifiers.
    // Indicates a bug in ResizeBrushesToolController thinking we are not dragging when we actually
    // still are.
    document->error() << "updateProposedDragHandles called during a drag";
    return;
  }

  const auto& hit = pickResult.first(type(Resize2DHitType | Resize3DHitType));
  const auto& nodes = document->selectedNodes().nodes();

  auto newDragHandles = getDragHandles(nodes, hit);
  if (newDragHandles != m_proposedDragHandles) {
    m_proposedDragHandles = std::move(newDragHandles);
    refreshViews();
  }
}

std::vector<Model::BrushFaceHandle> ResizeBrushesTool::getDragFaces(
  const std::vector<ResizeDragHandle>& dragHandles) {
  auto dragFaces = std::vector<Model::BrushFaceHandle>{};
  dragFaces.reserve(dragHandles.size());

  for (const auto& dragHandle : dragHandles) {
    const auto& brush = dragHandle.faceHandle.node()->brush();
    if (const auto faceIndex = brush.findFace(dragHandle.faceNormal())) {
      dragFaces.emplace_back(dragHandle.faceHandle.node(), *faceIndex);
    }
  }

  return dragFaces;
}

/**
 * Starts resizing the faces determined by the previous call to updateProposedDragHandles
 */
void ResizeBrushesTool::beginResize() {
  ensure(!m_dragging, "may not be called during a drag");
  m_dragging = true;
  kdl::mem_lock(m_document)->startTransaction("Resize Brushes");
}

namespace {

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
bool splitBrushesOutward(MapDocument& document, const vm::vec3& delta, ResizeDragState& dragState) {
  const auto& worldBounds = document.worldBounds();
  const bool lockTextures = pref(Preferences::TextureLock);

  // First ensure that the drag can be applied at all. For this, check whether each drag handle is
  // moved "up" along its normal.
  for (const auto& dragHandle : dragState.initialDragHandles) {
    const auto& normal = dragHandle.faceNormal();
    if (vm::dot(normal, delta) <= FloatType{0}) {
      return false;
    }
  }

  auto newDragFaces = std::vector<Model::BrushFaceHandle>{};
  auto newNodes = std::map<Model::Node*, std::vector<Model::Node*>>{};

  return kdl::for_each_result(
           dragState.initialDragHandles,
           [&](const auto& dragHandle) {
             auto* brushNode = dragHandle.faceHandle.node();

             const auto& oldBrush = dragHandle.brushAtDragStart;
             const auto dragFaceIndex = dragHandle.faceHandle.faceIndex();
             const auto newDragFaceNormal = dragHandle.faceNormal();

             auto newBrush = oldBrush;
             return newBrush.moveBoundary(worldBounds, dragFaceIndex, delta, lockTextures)
               .and_then([&]() {
                 auto clipFace = oldBrush.face(dragFaceIndex);
                 clipFace.invert();
                 return newBrush.clip(worldBounds, std::move(clipFace));
               })
               .and_then([&]() {
                 auto* newBrushNode = new Model::BrushNode(std::move(newBrush));
                 newNodes[brushNode->parent()].push_back(newBrushNode);

                 // Look up the new face index of the new drag handle
                 if (
                   const auto newDragFaceIndex =
                     newBrushNode->brush().findFace(newDragFaceNormal)) {
                   newDragFaces.push_back(Model::BrushFaceHandle(newBrushNode, *newDragFaceIndex));
                 }
               });
           })
    .and_then([&]() {
      // Apply the changes calculated above
      document.rollbackTransaction();

      document.deselectAll();
      const auto addedNodes = document.addNodes(newNodes);
      document.select(addedNodes);
      dragState.currentDragFaces = std::move(newDragFaces);
      dragState.totalDelta = delta;
    })
    .handle_errors([&](const Model::BrushError e) {
      document.error() << "Could not extrude brush: " << e;
      kdl::map_clear_and_delete(newNodes);
    });
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
bool splitBrushesInward(MapDocument& document, const vm::vec3& delta, ResizeDragState& dragState) {
  const auto& worldBounds = document.worldBounds();
  const bool lockTextures = pref(Preferences::TextureLock);

  // First ensure that the drag can be applied at all. For this, check whether each drag handle is
  // moved "down" along its normal.
  for (const auto& dragHandle : dragState.initialDragHandles) {
    const auto& normal = dragHandle.faceNormal();
    if (vm::dot(normal, delta) > FloatType{0}) {
      return false;
    }
  }

  auto newDragFaces = std::vector<Model::BrushFaceHandle>{};
  // This map is to handle the case when the brushes being
  // extruded have different parents (e.g. different brush entities),
  // so each newly created brush should be made a sibling of the brush it was cloned from.
  auto newNodes = std::map<Model::Node*, std::vector<Model::Node*>>{};
  auto nodesToUpdate = std::vector<std::pair<Model::Node*, Model::NodeContents>>{};

  for (const auto& dragHandle : dragState.initialDragHandles) {
    auto* brushNode = dragHandle.faceHandle.node();

    // "Front" means the part closer to the drag handles at the drag start
    auto frontBrush = dragHandle.brushAtDragStart;
    auto backBrush = dragHandle.brushAtDragStart;

    auto clipFace = frontBrush.face(dragHandle.faceHandle.faceIndex());

    if (clipFace.transform(vm::translation_matrix(delta), lockTextures).is_error()) {
      document.error() << "Could not extrude inwards: Error transforming face";
      kdl::map_clear_and_delete(newNodes);
      return false;
    }

    auto clipFaceInverted = clipFace;
    clipFaceInverted.invert();

    // Front brush should always be valid
    if (frontBrush.clip(worldBounds, clipFaceInverted).is_error()) {
      document.error() << "Could not extrude inwards: Front brush is empty";
      kdl::map_clear_and_delete(newNodes);
      return false;
    }

    nodesToUpdate.emplace_back(brushNode, std::move(frontBrush));

    // Back brush
    if (backBrush.clip(worldBounds, clipFace).is_success()) {
      auto* newBrushNode = new Model::BrushNode(std::move(backBrush));
      newNodes[brushNode->parent()].push_back(newBrushNode);

      // Look up the new face index of the new drag handle
      if (const auto newDragFaceIndex = newBrushNode->brush().findFace(clipFace.normal())) {
        newDragFaces.push_back(Model::BrushFaceHandle(newBrushNode, *newDragFaceIndex));
      }
    }
  }

  // Apply changes calculated above

  dragState.currentDragFaces.clear();
  document.rollbackTransaction();

  // FIXME: deal with linked group update failure (needed for #3647)
  const bool success = document.swapNodeContents("Resize Brushes", nodesToUpdate);
  unused(success);

  // Add the newly split off brushes and select them (keeping the original brushes selected).
  // FIXME: deal with linked group update failure (needed for #3647)
  const auto addedNodes = document.addNodes(std::move(newNodes));
  document.select(addedNodes);

  dragState.currentDragFaces = std::move(newDragFaces);
  dragState.totalDelta = delta;

  return true;
}

std::vector<vm::polygon3> getPolygons(const std::vector<ResizeDragHandle>& dragHandles) {
  return kdl::vec_transform(dragHandles, [](const auto& dragHandle) {
    return dragHandle.brushAtDragStart.face(dragHandle.faceHandle.faceIndex()).polygon();
  });
}
} // namespace

bool ResizeBrushesTool::resize(const vm::vec3& faceDelta, ResizeDragState& dragState) {
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);

  if (dragState.splitBrushes) {
    if (
      splitBrushesOutward(*document, faceDelta, dragState) ||
      splitBrushesInward(*document, faceDelta, dragState)) {
      return true;
    }
  } else {
    document->rollbackTransaction();
    if (document->resizeBrushes(getPolygons(dragState.initialDragHandles), faceDelta)) {
      dragState.totalDelta = faceDelta;
    } else {
      // resizeBrushes() fails if some brushes were completely clipped away.
      // In that case, restore the last m_totalDelta to be successfully applied.
      document->resizeBrushes(getPolygons(dragState.initialDragHandles), dragState.totalDelta);
    }
  }

  dragState.currentDragFaces = getDragFaces(m_proposedDragHandles);

  return true;
}

void ResizeBrushesTool::beginMove() {
  ensure(!m_dragging, "may not be called during a drag");
  m_dragging = true;
  kdl::mem_lock(m_document)->startTransaction("Move Faces");
}

bool ResizeBrushesTool::move(const vm::vec3& delta, ResizeDragState& dragState) {
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);

  document->rollbackTransaction();
  if (document->moveFaces(getPolygons(dragState.initialDragHandles), delta)) {
    dragState.totalDelta = delta;
  } else {
    // restore the last successful position
    document->moveFaces(getPolygons(dragState.initialDragHandles), dragState.totalDelta);
  }

  dragState.currentDragFaces = getDragFaces(m_proposedDragHandles);

  return true;
}

void ResizeBrushesTool::commit(const ResizeDragState& dragState) {
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);
  if (vm::is_zero(dragState.totalDelta, vm::C::almost_zero())) {
    document->cancelTransaction();
  } else {
    document->commitTransaction();
  }
  m_proposedDragHandles.clear();
  m_dragging = false;
}

void ResizeBrushesTool::cancel() {
  ensure(m_dragging, "may only be called during a drag");

  auto document = kdl::mem_lock(m_document);
  document->cancelTransaction();
  m_proposedDragHandles.clear();
  m_dragging = false;
}

void ResizeBrushesTool::connectObservers() {
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &ResizeBrushesTool::nodesDidChange);
  m_notifierConnection +=
    document->nodesWillChangeNotifier.connect(this, &ResizeBrushesTool::nodesDidChange);
  m_notifierConnection +=
    document->nodesWillBeRemovedNotifier.connect(this, &ResizeBrushesTool::nodesDidChange);
  m_notifierConnection +=
    document->selectionDidChangeNotifier.connect(this, &ResizeBrushesTool::selectionDidChange);
}

void ResizeBrushesTool::nodesDidChange(const std::vector<Model::Node*>&) {
  if (!m_dragging) {
    m_proposedDragHandles.clear();
  }
}

void ResizeBrushesTool::selectionDidChange(const Selection&) {
  if (!m_dragging) {
    m_proposedDragHandles.clear();
  }
}

} // namespace TrenchBroom::View
