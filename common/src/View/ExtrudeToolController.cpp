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

#include "ExtrudeToolController.h"

#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/HitAdapter.h"
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
#include "View/ExtrudeTool.h"
#include "View/Grid.h"
#include "View/HandleDragTracker.h"
#include "View/InputState.h"

#include <vecmath/distance.h>
#include <vecmath/intersection.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/scalar.h>

namespace TrenchBroom::View {
ExtrudeToolController::ExtrudeToolController(ExtrudeTool& tool)
  : m_tool{tool} {}

ExtrudeToolController::~ExtrudeToolController() = default;

Tool& ExtrudeToolController::tool() {
  return m_tool;
}

const Tool& ExtrudeToolController::tool() const {
  return m_tool;
}

void ExtrudeToolController::pick(const InputState& inputState, Model::PickResult& pickResult) {
  if (handleInput(inputState)) {
    const Model::Hit hit = doPick(inputState.pickRay(), pickResult);
    if (hit.isMatch()) {
      pickResult.addHit(hit);
    }
  }
}

void ExtrudeToolController::modifierKeyChange(const InputState& inputState) {
  if (!anyToolDragging(inputState)) {
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }
}

void ExtrudeToolController::mouseMove(const InputState& inputState) {
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

Renderer::DirectEdgeRenderer buildEdgeRenderer(const std::vector<ExtrudeDragHandle>& dragHandles) {
  return buildEdgeRenderer(kdl::vec_transform(dragHandles, [](const auto& h) {
    return h.faceHandle;
  }));
}

struct ExtrudeDragDelegate : public HandleDragTrackerDelegate {
  ExtrudeTool& m_tool;
  ExtrudeDragState m_extrudeDragState;

  ExtrudeDragDelegate(ExtrudeTool& tool, ExtrudeDragState extrudeDragState)
    : m_tool{tool}
    , m_extrudeDragState{std::move(extrudeDragState)} {}
  auto makePicker(const InputState& inputState, const vm::vec3& handleOffset) {
    using namespace Model::HitFilters;

    const auto& hit = inputState.pickResult().first(type(ExtrudeTool::ExtrudeHitType));
    assert(hit.isMatch());

    const auto& hitData = hit.target<ExtrudeHitData>();
    return std::visit(
      kdl::overload(
        [&](const vm::line3& line) {
          return makeLineHandlePicker(line, handleOffset);
        },
        [&](const vm::plane3& plane) {
          return makePlaneHandlePicker(plane, handleOffset);
        }),
      hitData.dragReference);
  }

  HandlePositionProposer start(
    const InputState& inputState, const vm::vec3&, const vm::vec3& handleOffset) override {
    const auto& dragFaceHandle = m_extrudeDragState.initialDragHandles.at(0);
    const auto& dragFace = dragFaceHandle.faceAtDragStart();

    auto picker = makePicker(inputState, handleOffset);
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
    const auto handleDelta = proposedHandlePosition - dragState.initialHandlePosition;
    if (m_tool.extrude(handleDelta, m_extrudeDragState)) {
      return DragStatus::Continue;
    }
    return DragStatus::Deny;
  }

  void end(const InputState& inputState, const DragState&) override {
    m_tool.commit(m_extrudeDragState);
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void setRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const override {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState&, const DragState&, Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override {
    auto edgeRenderer = buildEdgeRenderer(m_extrudeDragState.currentDragFaces);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ExtrudeHandleColor));
  }
};

auto createExtrudeDragTracker(
  ExtrudeTool& tool, const InputState& inputState, const Model::Hit& hit, const bool split) {
  const auto initialHandlePosition = hit.target<ExtrudeHitData>().initialHandlePosition();
  const auto handleOffset = initialHandlePosition - hit.hitPoint();

  return createHandleDragTracker(
    ExtrudeDragDelegate{
      tool,
      {tool.proposedDragHandles(), ExtrudeTool::getDragFaces(tool.proposedDragHandles()), split}},
    inputState, initialHandlePosition, handleOffset);
}

struct MoveDragDelegate : public HandleDragTrackerDelegate {
  ExtrudeTool& m_tool;
  ExtrudeDragState m_moveDragState;

  MoveDragDelegate(ExtrudeTool& tool, ExtrudeDragState moveDragState)
    : m_tool{tool}
    , m_moveDragState{std::move(moveDragState)} {}

  HandlePositionProposer start(
    const InputState& inputState, const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset) override {
    auto picker = makePlaneHandlePicker(
      vm::plane3{initialHandlePosition, vm::vec3{inputState.camera().direction()}}, handleOffset);

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
    if (m_tool.move(delta, m_moveDragState)) {
      return DragStatus::Continue;
    }
    return DragStatus::Deny;
  }

  void end(const InputState& inputState, const DragState&) override {
    m_tool.commit(m_moveDragState);
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void setRenderOptions(const InputState&, Renderer::RenderContext& renderContext) const override {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState&, const DragState&, Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override {
    auto edgeRenderer = buildEdgeRenderer(m_moveDragState.currentDragFaces);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ExtrudeHandleColor));
  }
};

auto createMoveDragTracker(ExtrudeTool& tool, const InputState& inputState, const Model::Hit& hit) {
  const auto initialHandlePosition = hit.target<ExtrudeHitData>().initialHandlePosition();
  const auto handleOffset = initialHandlePosition - hit.hitPoint();

  return createHandleDragTracker(
    MoveDragDelegate{
      tool, {tool.proposedDragHandles(), ExtrudeTool::getDragFaces(tool.proposedDragHandles())}},
    inputState, initialHandlePosition, handleOffset);
}
} // namespace

std::unique_ptr<DragTracker> ExtrudeToolController::acceptMouseDrag(const InputState& inputState) {
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

  const auto& hit = inputState.pickResult().first(type(ExtrudeTool::ExtrudeHitType));
  if (hit.isMatch()) {
    if (inputState.modifierKeysDown(ModifierKeys::MKAlt)) {
      if (inputState.camera().orthographicProjection()) {
        m_tool.beginMove();
        return createMoveDragTracker(m_tool, inputState, hit);
      }
    } else {
      const auto split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
      m_tool.beginExtrude();
      return createExtrudeDragTracker(m_tool, inputState, hit, split);
    }
  }

  return nullptr;
}

void ExtrudeToolController::render(
  const InputState& inputState, Renderer::RenderContext&, Renderer::RenderBatch& renderBatch) {
  const auto proposedDragHandles = m_tool.proposedDragHandles();
  if (!inputState.anyToolDragging() && !proposedDragHandles.empty()) {
    auto edgeRenderer = buildEdgeRenderer(proposedDragHandles);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ExtrudeHandleColor));
  }
}

bool ExtrudeToolController::cancel() {
  return false;
}

bool ExtrudeToolController::handleInput(const InputState& inputState) const {
  return (doHandleInput(inputState) && m_tool.applies());
}

ExtrudeToolController2D::ExtrudeToolController2D(ExtrudeTool& tool)
  : ExtrudeToolController{tool} {}

Model::Hit ExtrudeToolController2D::doPick(
  const vm::ray3& pickRay, const Model::PickResult& pickResult) {
  return m_tool.pick2D(pickRay, pickResult);
}

bool ExtrudeToolController2D::doHandleInput(const InputState& inputState) const {
  return (
    inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd) ||
    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKAlt));
}

ExtrudeToolController3D::ExtrudeToolController3D(ExtrudeTool& tool)
  : ExtrudeToolController{tool} {}

Model::Hit ExtrudeToolController3D::doPick(
  const vm::ray3& pickRay, const Model::PickResult& pickResult) {
  return m_tool.pick3D(pickRay, pickResult);
}

bool ExtrudeToolController3D::doHandleInput(const InputState& inputState) const {
  return (
    inputState.modifierKeysPressed(ModifierKeys::MKShift) ||
    inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd));
}
} // namespace TrenchBroom::View
