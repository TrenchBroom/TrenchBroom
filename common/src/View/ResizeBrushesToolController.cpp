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
#include "Renderer/GLVertexType.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderContext.h"
#include "Renderer/VertexArray.h"
#include "View/DragTracker.h"
#include "View/InputState.h"
#include "View/ResizeBrushesTool.h"

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
} // namespace

std::unique_ptr<DragTracker> ResizeBrushesToolController::acceptMouseDrag(
  const InputState& inputState) {
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
    // move mode
    if (auto dragState = m_tool.beginMove(inputState.pickResult())) {
      return std::make_unique<ResizeToolDragTracker>(
        m_tool, std::move(*dragState),
        [&](const InputState& inputState_, ResizeDragState& dragState_) {
          return m_tool.move(inputState_.pickRay(), inputState_.camera(), dragState_);
        });
    }
  } else {
    // resize mode
    const auto split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
    if (auto dragState = m_tool.beginResize(inputState.pickResult(), split)) {
      return std::make_unique<ResizeToolDragTracker>(
        m_tool, std::move(*dragState),
        [&](const InputState& inputState_, ResizeDragState& dragState_) {
          return m_tool.resize(inputState_.pickRay(), inputState_.camera(), dragState_);
        });
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
