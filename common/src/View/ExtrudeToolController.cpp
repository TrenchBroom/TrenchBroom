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

#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/line.h"
#include "vm/plane.h"
#include "vm/scalar.h"

namespace TrenchBroom::View
{
ExtrudeToolController::ExtrudeToolController(ExtrudeTool& tool)
  : m_tool{tool}
{
}

ExtrudeToolController::~ExtrudeToolController() = default;

Tool& ExtrudeToolController::tool()
{
  return m_tool;
}

const Tool& ExtrudeToolController::tool() const
{
  return m_tool;
}

void ExtrudeToolController::pick(
  const InputState& inputState, Model::PickResult& pickResult)
{
  if (handleInput(inputState))
  {
    const Model::Hit hit = doPick(inputState.pickRay(), pickResult);
    if (hit.isMatch())
    {
      pickResult.addHit(hit);
    }
  }
}

void ExtrudeToolController::modifierKeyChange(const InputState& inputState)
{
  if (!inputState.anyToolDragging())
  {
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }
}

void ExtrudeToolController::mouseMove(const InputState& inputState)
{
  if (handleInput(inputState) && !inputState.anyToolDragging())
  {
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }
}

namespace
{
Renderer::DirectEdgeRenderer buildEdgeRenderer(
  const std::vector<Model::BrushFaceHandle>& dragHandles)
{
  using Vertex = Renderer::GLVertexTypes::P3::Vertex;
  auto vertices = std::vector<Vertex>{};

  for (const auto& dragHandle : dragHandles)
  {
    const auto& dragFace = dragHandle.face();
    for (const auto* edge : dragFace.edges())
    {
      vertices.emplace_back(vm::vec3f{edge->firstVertex()->position()});
      vertices.emplace_back(vm::vec3f{edge->secondVertex()->position()});
    }
  }

  return Renderer::DirectEdgeRenderer{
    Renderer::VertexArray::move(std::move(vertices)), Renderer::PrimType::Lines};
}

Renderer::DirectEdgeRenderer buildEdgeRenderer(
  const std::vector<ExtrudeDragHandle>& dragHandles)
{
  return buildEdgeRenderer(
    kdl::vec_transform(dragHandles, [](const auto& h) { return h.faceHandle; }));
}

struct ExtrudeDragDelegate : public HandleDragTrackerDelegate
{
  ExtrudeTool& m_tool;
  ExtrudeDragState m_extrudeDragState;

  ExtrudeDragDelegate(ExtrudeTool& tool, ExtrudeDragState extrudeDragState)
    : m_tool{tool}
    , m_extrudeDragState{std::move(extrudeDragState)}
  {
  }

  vm::vec3 getAverageFaceNormal()
  {
    auto result = vm::vec3{};
    for (const auto& dragHandle : m_extrudeDragState.initialDragHandles)
    {
      result = result + dragHandle.faceNormal();
    }
    return result / static_cast<FloatType>(m_extrudeDragState.initialDragHandles.size());
  }

  /**
   * In 3D views or 2D views, we use a picking plane when the user picks a face by
   * clicking outside of the brush. With this, we can make the drag feel as if the user is
   * dragging the closest brush edge around because any movement that is orthogonal to the
   * face normal is ignored.
   *
   * After picking a point on the plane, we project that point onto the face normal to
   * make it canonical. In the end, we are only interested in picking a point on a line
   * through the initial handle position. This allows us to ignore all drags that are
   * snapped onto the same distance by the snapper.
   *
   * Why can't we just use this line for picking right away without picking a plane first?
   * This would change the feeling of the drag significantly, particularly in 3D. It's
   * difficult to put into words, but the user would no longer feel as if they are
   * dragging the closest brush edge.
   */
  DragHandlePicker makeCanonicalHandlePicker(
    const vm::plane3& plane,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset)
  {
    return
      [planeHandlePicker = makePlaneHandlePicker(plane, handleOffset),
       faceNormal = m_extrudeDragState.initialDragHandles.front().faceNormal(),
       initialHandlePosition](const InputState& inputState_) -> std::optional<vm::vec3> {
        if (const auto pointOnPlane = planeHandlePicker(inputState_))
        {
          const auto moveDelta = *pointOnPlane - initialHandlePosition;
          const auto canonicalMoveDistance = vm::dot(moveDelta, faceNormal);
          return initialHandlePosition + canonicalMoveDistance * faceNormal;
        }
        return std::nullopt;
      };
  }

  auto makePicker(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset)
  {
    using namespace Model::HitFilters;

    const auto& hit = inputState.pickResult().first(type(ExtrudeTool::ExtrudeHitType));
    assert(hit.isMatch());

    const auto& hitData = hit.target<ExtrudeHitData>();
    return std::visit(
      kdl::overload(
        [&](const vm::line3& line) { return makeLineHandlePicker(line, handleOffset); },
        [&](const vm::plane3& plane) {
          return makeCanonicalHandlePicker(plane, initialHandlePosition, handleOffset);
        }),
      hitData.dragReference);
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset) override
  {
    auto picker = makePicker(inputState, initialHandlePosition, handleOffset);
    auto snapper = [&](
                     const InputState&,
                     const DragState& dragState,
                     const vm::vec3& proposedHandlePosition) {
      auto& grid = m_tool.grid();
      if (!grid.snap())
      {
        return proposedHandlePosition;
      }

      const auto moveDelta = proposedHandlePosition - dragState.initialHandlePosition;
      const auto moveDirection = vm::normalize(moveDelta);
      const auto moveDistance = vm::dot(moveDelta, moveDirection);

      auto snappedMoveDistance = std::numeric_limits<FloatType>::max();
      for (const auto& dragHandle : m_extrudeDragState.initialDragHandles)
      {
        const auto moveDistanceOnFaceNormal = vm::dot(moveDelta, dragHandle.faceNormal());
        const auto snappedMoveDistanceOnFaceNormal = grid.snapMoveDistanceForFace(
          dragHandle.faceAtDragStart(), moveDistanceOnFaceNormal);
        const auto snappedMoveDistanceForFace =
          snappedMoveDistanceOnFaceNormal
          / vm::dot(moveDirection, dragHandle.faceNormal());
        if (
          vm::abs(snappedMoveDistanceForFace - moveDistance)
          < vm::abs(snappedMoveDistance - moveDistance))
        {
          snappedMoveDistance = snappedMoveDistanceForFace;
        }
      }

      return dragState.initialHandlePosition + snappedMoveDistance * moveDirection;
    };

    return makeHandlePositionProposer(std::move(picker), std::move(snapper));
  }

  DragStatus drag(
    const InputState&,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override
  {
    const auto handleDelta = proposedHandlePosition - dragState.initialHandlePosition;
    if (m_tool.extrude(handleDelta, m_extrudeDragState))
    {
      return DragStatus::Continue;
    }
    return DragStatus::Deny;
  }

  void end(const InputState& inputState, const DragState&) override
  {
    m_tool.commit(m_extrudeDragState);
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void setRenderOptions(
    const InputState&, Renderer::RenderContext& renderContext) const override
  {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState&,
    const DragState&,
    Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override
  {
    auto edgeRenderer = buildEdgeRenderer(m_extrudeDragState.currentDragFaces);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ExtrudeHandleColor));
  }
};

auto createExtrudeDragTracker(
  ExtrudeTool& tool,
  const InputState& inputState,
  const Model::Hit& hit,
  const bool split)
{
  const auto initialHandlePosition = hit.target<ExtrudeHitData>().initialHandlePosition;

  return createHandleDragTracker(
    ExtrudeDragDelegate{
      tool,
      {tool.proposedDragHandles(),
       ExtrudeTool::getDragFaces(tool.proposedDragHandles()),
       split}},
    inputState,
    initialHandlePosition,
    hit.hitPoint());
}

struct MoveDragDelegate : public HandleDragTrackerDelegate
{
  ExtrudeTool& m_tool;
  ExtrudeDragState m_moveDragState;

  MoveDragDelegate(ExtrudeTool& tool, ExtrudeDragState moveDragState)
    : m_tool{tool}
    , m_moveDragState{std::move(moveDragState)}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset) override
  {
    auto picker = makePlaneHandlePicker(
      vm::plane3{initialHandlePosition, vm::vec3{inputState.camera().direction()}},
      handleOffset);

    auto snapper = [&](
                     const InputState&,
                     const DragState& dragState,
                     const vm::vec3& proposedHandlePosition) {
      auto& grid = m_tool.grid();
      if (!grid.snap())
      {
        return proposedHandlePosition;
      }

      const auto totalDelta = proposedHandlePosition - dragState.initialHandlePosition;
      const auto snappedDelta = grid.snap(totalDelta);
      return dragState.initialHandlePosition + snappedDelta;
    };

    return makeHandlePositionProposer(std::move(picker), std::move(snapper));
  }

  DragStatus drag(
    const InputState&,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override
  {
    const auto delta = proposedHandlePosition - dragState.initialHandlePosition;
    if (m_tool.move(delta, m_moveDragState))
    {
      return DragStatus::Continue;
    }
    return DragStatus::Deny;
  }

  void end(const InputState& inputState, const DragState&) override
  {
    m_tool.commit(m_moveDragState);
    m_tool.updateProposedDragHandles(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void setRenderOptions(
    const InputState&, Renderer::RenderContext& renderContext) const override
  {
    renderContext.setForceShowSelectionGuide();
  }

  void render(
    const InputState&,
    const DragState&,
    Renderer::RenderContext&,
    Renderer::RenderBatch& renderBatch) const override
  {
    auto edgeRenderer = buildEdgeRenderer(m_moveDragState.currentDragFaces);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ExtrudeHandleColor));
  }
};

auto createMoveDragTracker(
  ExtrudeTool& tool, const InputState& inputState, const Model::Hit& hit)
{
  const auto initialHandlePosition = hit.target<ExtrudeHitData>().initialHandlePosition;

  return createHandleDragTracker(
    MoveDragDelegate{
      tool,
      {tool.proposedDragHandles(),
       ExtrudeTool::getDragFaces(tool.proposedDragHandles())}},
    inputState,
    initialHandlePosition,
    hit.hitPoint());
}
} // namespace

std::unique_ptr<DragTracker> ExtrudeToolController::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace Model::HitFilters;

  if (!handleInput(inputState))
  {
    return nullptr;
  }
  // NOTE: We check for MBLeft here rather than in handleInput because we want the
  // yellow highlight to render as a preview when Shift is down, before you press MBLeft.
  if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
  {
    return nullptr;
  }

  m_tool.updateProposedDragHandles(inputState.pickResult());

  const auto& hit = inputState.pickResult().first(type(ExtrudeTool::ExtrudeHitType));
  if (hit.isMatch())
  {
    if (inputState.modifierKeysDown(ModifierKeys::MKAlt))
    {
      if (inputState.camera().orthographicProjection())
      {
        m_tool.beginMove();
        return createMoveDragTracker(m_tool, inputState, hit);
      }
    }
    else
    {
      const auto split = inputState.modifierKeysDown(ModifierKeys::MKCtrlCmd);
      m_tool.beginExtrude();
      return createExtrudeDragTracker(m_tool, inputState, hit, split);
    }
  }

  return nullptr;
}

void ExtrudeToolController::render(
  const InputState& inputState,
  Renderer::RenderContext&,
  Renderer::RenderBatch& renderBatch)
{
  const auto proposedDragHandles = m_tool.proposedDragHandles();
  if (!inputState.anyToolDragging() && !proposedDragHandles.empty())
  {
    auto edgeRenderer = buildEdgeRenderer(proposedDragHandles);
    edgeRenderer.renderOnTop(renderBatch, pref(Preferences::ExtrudeHandleColor));
  }
}

bool ExtrudeToolController::cancel()
{
  return false;
}

bool ExtrudeToolController::handleInput(const InputState& inputState) const
{
  return (doHandleInput(inputState) && m_tool.applies());
}

ExtrudeToolController2D::ExtrudeToolController2D(ExtrudeTool& tool)
  : ExtrudeToolController{tool}
{
}

Model::Hit ExtrudeToolController2D::doPick(
  const vm::ray3& pickRay, const Model::PickResult& pickResult)
{
  return m_tool.pick2D(pickRay, pickResult);
}

bool ExtrudeToolController2D::doHandleInput(const InputState& inputState) const
{
  return (
    inputState.modifierKeysPressed(ModifierKeys::MKShift)
    || inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd)
    || inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKAlt));
}

ExtrudeToolController3D::ExtrudeToolController3D(ExtrudeTool& tool)
  : ExtrudeToolController{tool}
{
}

Model::Hit ExtrudeToolController3D::doPick(
  const vm::ray3& pickRay, const Model::PickResult& pickResult)
{
  return m_tool.pick3D(pickRay, pickResult);
}

bool ExtrudeToolController3D::doHandleInput(const InputState& inputState) const
{
  return (
    inputState.modifierKeysPressed(ModifierKeys::MKShift)
    || inputState.modifierKeysPressed(ModifierKeys::MKShift | ModifierKeys::MKCtrlCmd));
}
} // namespace TrenchBroom::View
