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

#include "ResizeBrushesToolController.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/PickResult.h"
#include "Model/Polyhedron.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"
#include "View/DragTracker.h"
#include "View/Grid.h"
#include "View/HandleDragTracker.h"
#include "View/InputState.h"
#include "View/ResizeBrushesTool.h"

#include <vecmath/distance.h>
#include <vecmath/intersection.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/scalar.h>

namespace TrenchBroom::View {
ResizeBrushesToolController::ResizeBrushesToolController(ResizeBrushesTool& tool)
  : m_tool{tool} {}

ResizeBrushesToolController::~ResizeBrushesToolController() = default;

Tool& ResizeBrushesToolController::tool() {
  return m_tool;
}

const Tool& ResizeBrushesToolController::tool() const {
  return m_tool;
}

void ResizeBrushesToolController::pick(
  const InputState& inputState, Model::PickResult& pickResult) {
  if (handleInput(inputState)) {
    const Model::Hit hit = doPick(inputState.pickRay(), pickResult);
    if (hit.isMatch()) {
      pickResult.addHit(hit);
    }
  }
}

void ResizeBrushesToolController::modifierKeyChange(const InputState& inputState) {
  if (!anyToolDragging(inputState)) {
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }
}

void ResizeBrushesToolController::mouseMove(const InputState& inputState) {
  if (handleInput(inputState) && !anyToolDragging(inputState)) {
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }
}

namespace {
Renderer::DirectEdgeRenderer buildEdgeRenderer(
  const std::vector<Model::BrushFaceHandle>& dragHandles) {
  using Vertex = Renderer::GLVertexTypes::P3::Vertex;
  auto vertices = std::vector<Vertex>{};

  for (const auto& dragHandle : dragHandles) {
    const auto& dragFace = dragHandle.face();
    for (const auto* edge : dragFace.edges()) {
      vertices.emplace_back(vm::vec3f{edge->firstVertex()->position()});
      vertices.emplace_back(vm::vec3f{edge->secondVertex()->position()});
    }
  }

  return Renderer::DirectEdgeRenderer{
    Renderer::VertexArray::move(std::move(vertices)), Renderer::PrimType::Lines};
}

Renderer::DirectEdgeRenderer buildEdgeRenderer(const std::vector<ResizeDragHandle>& dragHandles) {
  return buildEdgeRenderer(kdl::vec_transform(dragHandles, [](const auto& h) {
    return h.faceHandle;
  }));
}

class ResizeToolDragTracker : public DragTracker {
private:
  using DragFunction = std::function<bool(const InputState&, ResizeDragState& dragState)>;

  ResizeBrushesTool& m_tool;
  ResizeDragState m_dragState;
  DragFunction m_drag;

public:
  ResizeToolDragTracker(ResizeBrushesTool& tool, ResizeDragState dragState, DragFunction drag)
    : m_tool{tool}
    , m_dragState{std::move(dragState)}
    , m_drag{std::move(drag)} {}

  bool drag(const InputState& inputState) override { return m_drag(inputState, m_dragState); }

  void end(const InputState& inputState) override {
    m_tool.commit(m_dragState);
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }

  void cancel() override { m_tool.cancel(); }

  void setRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const override {
    renderContext.setForceShowSelectionGuide();
  }

  void render(const InputState&, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch)
    const override {
    auto edgeRenderer = buildEdgeRenderer(m_dragState.currentDragFaces);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
  }
};

struct ResizeDragDelegate : public HandleDragTrackerDelegate {
  ResizeBrushesTool& m_tool;
  ResizeDragState m_resizeDragState;

  ResizeDragDelegate(ResizeBrushesTool& tool, ResizeDragState resizeDragState)
    : m_tool{tool}
    , m_resizeDragState{std::move(resizeDragState)} {}

  HandlePositionProposer start(
    const InputState&, const vm::vec3&, const vm::vec3& handleOffset) override {
    const auto& dragFaceHandle = m_resizeDragState.initialDragHandles.at(0);
    const auto& dragFace = dragFaceHandle.faceAtDragStart();
    const auto& faceNormal = dragFace.boundary().normal;

    auto picker =
      makeLineHandlePicker(vm::line3{m_resizeDragState.dragOrigin, faceNormal}, handleOffset);

    auto snapper =
      [&](const InputState&, const DragState& dragState, const vm::vec3& proposedHandlePosition) {
        auto& grid = m_tool.grid();
        if (!grid.snap()) {
          return proposedHandlePosition;
        }

        const auto totalFaceDelta = proposedHandlePosition - dragState.initialHandlePosition;
        const auto snappedFaceDelta = grid.moveDelta(dragFace, totalFaceDelta);
        return dragState.initialHandlePosition + snappedFaceDelta;
      };

    return makeHandlePositionProposer(std::move(picker), std::move(snapper));
  }

  DragStatus drag(
    const InputState&, const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override {
    const auto faceDelta = proposedHandlePosition - dragState.initialHandlePosition;
    if (m_tool.resize(faceDelta, m_resizeDragState)) {
      return DragStatus::Continue;
    }
    return DragStatus::Deny;
  }

  void end(const InputState& inputState, const DragState&) override {
    m_tool.commit(m_resizeDragState);
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void setRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const override {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState&, const DragState&, Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override {
    auto edgeRenderer = buildEdgeRenderer(m_resizeDragState.currentDragFaces);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
  }
};

struct MoveDragDelegate : public HandleDragTrackerDelegate {
  ResizeBrushesTool& m_tool;
  ResizeDragState m_resizeDragState;

  MoveDragDelegate(ResizeBrushesTool& tool, ResizeDragState resizeDragState)
    : m_tool{tool}
    , m_resizeDragState{std::move(resizeDragState)} {}

  HandlePositionProposer start(
    const InputState& inputState, const vm::vec3&, const vm::vec3& handleOffset) override {
    auto picker = makePlaneHandlePicker(
      vm::plane3{m_resizeDragState.dragOrigin, vm::vec3{inputState.camera().direction()}},
      handleOffset);

    auto snapper =
      [&](const InputState&, const DragState& dragState, const vm::vec3& proposedHandlePosition) {
        auto& grid = m_tool.grid();
        if (!grid.snap()) {
          return proposedHandlePosition;
        }

        const auto totalDelta = proposedHandlePosition - dragState.initialHandlePosition;
        const auto snappedDelta = grid.snap(totalDelta);
        return dragState.initialHandlePosition + snappedDelta;
      };

    return makeHandlePositionProposer(std::move(picker), std::move(snapper));
  }

  DragStatus drag(
    const InputState&, const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override {
    const auto delta = proposedHandlePosition - dragState.initialHandlePosition;
    if (m_tool.move(delta, m_resizeDragState)) {
      return DragStatus::Continue;
    }
    return DragStatus::Deny;
  }

  void end(const InputState& inputState, const DragState&) override {
    m_tool.commit(m_resizeDragState);
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void setRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const override {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState&, const DragState&, Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override {
    auto edgeRenderer = buildEdgeRenderer(m_resizeDragState.currentDragFaces);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
  }
};

auto makeMoveDragTracker(
  ResizeBrushesTool& tool, const InputState& inputState, const vm::vec3& initialHitPoint) {
  // todo: compute handle offset correctly
  return createHandleDragTracker(
    MoveDragDelegate{
      tool,
      {initialHitPoint, tool.proposedDragHandles(),
       ResizeBrushesTool::getDragFaces(tool.proposedDragHandles()), false, vm::vec3::zero()}},
    inputState, initialHitPoint, vm::vec3::zero());
}

auto makeResizeDragTracker(
  ResizeBrushesTool& tool, const InputState& inputState, const vm::vec3& initialHitPoint,
  const bool split) {
  // todo: compute handle offset correctly
  return createHandleDragTracker(
    ResizeDragDelegate{
      tool,
      {initialHitPoint, tool.proposedDragHandles(),
       ResizeBrushesTool::getDragFaces(tool.proposedDragHandles()), split, vm::vec3::zero()}},
    inputState, initialHitPoint, vm::vec3::zero());
}
} // namespace

std::unique_ptr<DragTracker> ResizeBrushesToolController::acceptMouseDrag(
  const InputState& inputState) {
  using namespace Model::HitFilters;

  if (!handleInput(inputState)) {
    return nullptr;
  }
  // NOTE: We check for MBLeft here rather than in handleInput because we want the
  // yellow highlight to render as a preview when Shift is down, before you press MBLeft.
  if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft)) {
    return nullptr;
  }

  m_tool.updateProposedDragHandles(inputState.pickResult());
  if (inputState.modifierKeysDown(ModifierKeys::MKAlt)) {
    const auto& hit = inputState.pickResult().first(type(ResizeBrushesTool::Resize2DHitType));
    if (hit.isMatch()) {
      m_tool.beginMove();
      return makeMoveDragTracker(m_tool, inputState, hit.hitPoint());
    }
  } else {
    const auto& hit = inputState.pickResult().first(
      type(ResizeBrushesTool::Resize2DHitType | ResizeBrushesTool::Resize3DHitType));
    if (hit.isMatch()) {
      const auto split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
      m_tool.beginResize();
      return makeResizeDragTracker(m_tool, inputState, hit.hitPoint(), split);
    }
  }

  return nullptr;
}

void ResizeBrushesToolController::render(
  const InputState& inputState, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
  const auto proposedDragHandles = m_tool.proposedDragHandles();
  if (!inputState.anyToolDragging() && !proposedDragHandles.empty()) {
    auto edgeRenderer = buildEdgeRenderer(proposedDragHandles);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ResizeHandleColor));
  }
}

bool ResizeBrushesToolController::cancel() {
  return false;
}

bool ResizeBrushesToolController::handleInput(const InputState& inputState) const {
  return (doHandleInput(inputState) && m_tool.applies());
}

ResizeBrushesToolController2D::ResizeBrushesToolController2D(ResizeBrushesTool& tool)
  : ResizeBrushesToolController{tool} {}

Model::Hit ResizeBrushesToolController2D::doPick(
  const vm::ray3& pickRay, const Model::PickResult& pickResult) {
  return m_tool.pick2D(pickRay, pickResult);
}

bool ResizeBrushesToolController2D::doHandleInput(const InputState& inputState) const {
  return (
    inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd) ||
    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKAlt));
}

ResizeBrushesToolController3D::ResizeBrushesToolController3D(ResizeBrushesTool& tool)
  : ResizeBrushesToolController{tool} {}

Model::Hit ResizeBrushesToolController3D::doPick(
  const vm::ray3& pickRay, const Model::PickResult& pickResult) {
  return m_tool.pick3D(pickRay, pickResult);
}

bool ResizeBrushesToolController3D::doHandleInput(const InputState& inputState) const {
  return (
    inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd));
}
} // namespace TrenchBroom::View
