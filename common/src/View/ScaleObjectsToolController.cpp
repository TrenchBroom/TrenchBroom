/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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

#include "ScaleObjectsToolController.h"

#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "View/HandleDragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"
#include "View/ScaleObjectsTool.h"

#include "kdl/memory_utils.h"
#include "kdl/vector_utils.h"

#include <vecmath/polygon.h>
#include <vecmath/segment.h>

#include <cassert>

namespace TrenchBroom
{
namespace View
{
ScaleObjectsToolController::ScaleObjectsToolController(
  ScaleObjectsTool& tool, std::weak_ptr<MapDocument> document)
  : m_tool{tool}
  , m_document{document}
{
}

ScaleObjectsToolController::~ScaleObjectsToolController() = default;

Tool& ScaleObjectsToolController::tool()
{
  return m_tool;
}

const Tool& ScaleObjectsToolController::tool() const
{
  return m_tool;
}

void ScaleObjectsToolController::pick(
  const InputState& inputState, Model::PickResult& pickResult)
{
  if (m_tool.applies())
  {
    doPick(inputState.pickRay(), inputState.camera(), pickResult);
  }
}

static HandlePositionProposer makeHandlePositionProposer(
  const InputState& inputState,
  const Grid& grid,
  const Model::Hit& dragStartHit,
  const vm::bbox3& bboxAtDragStart,
  const vm::vec3& handleOffset)
{
  const bool scaleAllAxes = inputState.modifierKeysDown(ModifierKeys::MKShift);

  if (
    dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHitType
    && inputState.camera().orthographicProjection() && !scaleAllAxes)
  {
    const auto plane = vm::plane3{
      dragStartHit.hitPoint() + handleOffset,
      vm::vec3{inputState.camera().direction()} * -1.0};
    return makeHandlePositionProposer(
      makePlaneHandlePicker(plane, handleOffset), makeRelativeHandleSnapper(grid));
  }
  else
  {
    assert(
      dragStartHit.type() == ScaleObjectsTool::ScaleToolSideHitType
      || dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHitType
      || dragStartHit.type() == ScaleObjectsTool::ScaleToolCornerHitType);

    const vm::line3 handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);

    return makeHandlePositionProposer(
      makeLineHandlePicker(handleLine, handleOffset),
      makeAbsoluteLineHandleSnapper(grid, handleLine));
  }
}

static std::pair<AnchorPos, ProportionalAxes> modifierSettingsForInputState(
  const InputState& inputState)
{
  const auto centerAnchor = inputState.modifierKeysDown(ModifierKeys::MKAlt)
                              ? AnchorPos::Center
                              : AnchorPos::Opposite;

  ProportionalAxes scaleAllAxes = ProportionalAxes::None();
  if (inputState.modifierKeysDown(ModifierKeys::MKShift))
  {
    scaleAllAxes = ProportionalAxes::All();

    const auto& camera = inputState.camera();
    if (camera.orthographicProjection())
    {
      // special case for 2D: don't scale along the axis of the camera
      const size_t cameraComponent = vm::find_abs_max_component(camera.direction());
      scaleAllAxes.setAxisProportional(cameraComponent, false);
    }
  }

  return {centerAnchor, scaleAllAxes};
}

void ScaleObjectsToolController::modifierKeyChange(const InputState& inputState)
{
  const auto [centerAnchor, scaleAllAxes] = modifierSettingsForInputState(inputState);

  if ((centerAnchor != m_tool.anchorPos()) || (scaleAllAxes != m_tool.proportionalAxes()))
  {
    // update state
    m_tool.setProportionalAxes(scaleAllAxes);
    m_tool.setAnchorPos(centerAnchor);
  }

  // Mouse might be over a different handle now
  m_tool.refreshViews();
}

void ScaleObjectsToolController::mouseMove(const InputState& inputState)
{
  if (m_tool.applies() && !inputState.anyToolDragging())
  {
    m_tool.updatePickedHandle(inputState.pickResult());
  }
}

namespace
{
class ScaleObjectsDragDelegate : public HandleDragTrackerDelegate
{
private:
  ScaleObjectsTool& m_tool;

public:
  ScaleObjectsDragDelegate(ScaleObjectsTool& tool)
    : m_tool{tool}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3& /* initialHandlePosition */,
    const vm::vec3& handleOffset) override
  {
    // update modifier settings
    const auto [centerAnchor, scaleAllAxes] = modifierSettingsForInputState(inputState);
    m_tool.setAnchorPos(centerAnchor);
    m_tool.setProportionalAxes(scaleAllAxes);

    return makeHandlePositionProposer(
      inputState,
      m_tool.grid(),
      m_tool.dragStartHit(),
      m_tool.bboxAtDragStart(),
      handleOffset);
  }

  std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState& inputState, const DragState& dragState) override
  {
    return UpdateDragConfig{
      makeHandlePositionProposer(
        inputState,
        m_tool.grid(),
        m_tool.dragStartHit(),
        m_tool.bboxAtDragStart(),
        dragState.handleOffset),
      ResetInitialHandlePosition::Keep};
  }

  DragStatus drag(
    const InputState&,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override
  {
    const auto delta = proposedHandlePosition - dragState.currentHandlePosition;
    m_tool.scaleByDelta(delta);
    return DragStatus::Continue;
  }

  void end(const InputState& inputState, const DragState&) override
  {
    m_tool.commitScale();

    // The mouse is in a different place now so update the highlighted side
    m_tool.updatePickedHandle(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancelScale(); }
};
} // namespace

static std::tuple<vm::vec3, vm::vec3> getInitialHandlePositionAndHitPoint(
  const vm::bbox3& bboxAtDragStart, const Model::Hit& dragStartHit)
{
  const vm::line3 handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);
  return {handleLine.get_origin(), dragStartHit.hitPoint()};
}

std::unique_ptr<DragTracker> ScaleObjectsToolController::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace Model::HitFilters;

  if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
  {
    return nullptr;
  }

  if (!m_tool.applies())
  {
    return nullptr;
  }
  auto document = kdl::mem_lock(m_document);

  const Model::Hit& hit = inputState.pickResult().first(type(
    ScaleObjectsTool::ScaleToolSideHitType | ScaleObjectsTool::ScaleToolEdgeHitType
    | ScaleObjectsTool::ScaleToolCornerHitType));
  if (!hit.isMatch())
  {
    return nullptr;
  }

  m_tool.startScaleWithHit(hit);

  const auto [handlePosition, hitPoint] =
    getInitialHandlePositionAndHitPoint(m_tool.bounds(), hit);
  return createHandleDragTracker(
    ScaleObjectsDragDelegate{m_tool}, inputState, handlePosition, hitPoint);
}

void ScaleObjectsToolController::setRenderOptions(
  const InputState&, Renderer::RenderContext& renderContext) const
{
  renderContext.setForceHideSelectionGuide();
}

static void renderBounds(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const vm::bbox3& bounds)
{
  auto renderService = Renderer::RenderService{renderContext, renderBatch};
  renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
  renderService.renderBounds(vm::bbox3f{bounds});
}

static void renderCornerHandles(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const std::vector<vm::vec3>& corners)
{
  auto renderService = Renderer::RenderService{renderContext, renderBatch};
  renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));

  for (const auto& corner : corners)
  {
    renderService.renderHandle(vm::vec3f(corner));
  }
}

static void renderDragSideHighlights(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const std::vector<vm::polygon3f>& sides)
{
  // Highlight all sides that will be moving as a result of the Shift/Alt modifiers
  // (proporitional scaling or center anchor modifiers)
  for (const auto& side : sides)
  {
    {
      auto renderService = Renderer::RenderService{renderContext, renderBatch};
      renderService.setShowBackfaces();
      renderService.setForegroundColor(pref(Preferences::ScaleFillColor));
      renderService.renderFilledPolygon(side.vertices());
    }

    // In 2D, additionally stroke the edges of this polyhedron, so it's visible even when
    // looking at it from an edge
    if (renderContext.camera().orthographicProjection())
    {
      auto renderService = Renderer::RenderService{renderContext, renderBatch};
      renderService.setLineWidth(2.0);
      renderService.setForegroundColor(Color(
        pref(Preferences::ScaleOutlineColor), pref(Preferences::ScaleOutlineDimAlpha)));
      renderService.renderPolygonOutline(side.vertices());
    }
  }
}

static void renderDragSide(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const vm::polygon3f& side)
{
  // draw the main highlighted handle
  auto renderService = Renderer::RenderService{renderContext, renderBatch};
  renderService.setLineWidth(2.0);
  renderService.setForegroundColor(pref(Preferences::ScaleOutlineColor));
  renderService.renderPolygonOutline(side.vertices());
}

static void renderDragEdge(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const vm::segment3f& edge)
{
  const auto& camera = renderContext.camera();

  auto renderService = Renderer::RenderService{renderContext, renderBatch};
  if (
    camera.orthographicProjection()
    && vm::is_parallel(edge.direction(), camera.direction()))
  {
    // for the 2D view, for drag edges that are parallel to the camera,
    // render the highlight with a ring around the handle
    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
    renderService.renderHandleHighlight(edge.start());
  }
  else
  {
    // render as a thick line
    renderService.setForegroundColor(pref(Preferences::ScaleOutlineColor));
    renderService.setLineWidth(2.0);
    renderService.renderLine(edge.start(), edge.end());
  }
}

static void renderDragCorner(
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch,
  const vm::vec3f& corner)
{
  auto renderService = Renderer::RenderService{renderContext, renderBatch};

  // the filled circular handle
  renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));
  renderService.renderHandle(corner);

  // the ring around the handle
  renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
  renderService.renderHandleHighlight(corner);
}

static std::vector<vm::vec3> visibleCornerHandles(
  const ScaleObjectsTool& tool, const Renderer::Camera& camera)
{
  using namespace Model::HitFilters;

  const auto cornerHandles = tool.cornerHandles();
  if (!camera.perspectiveProjection())
  {
    return cornerHandles;
  }

  return kdl::vec_filter(cornerHandles, [&](const auto& corner) {
    const auto ray = vm::ray3{camera.pickRay(vm::vec3f{corner})};

    auto pr = Model::PickResult{};
    if (camera.orthographicProjection())
    {
      tool.pick2D(ray, camera, pr);
    }
    else
    {
      tool.pick3D(ray, camera, pr);
    }

    return !pr.empty()
           && pr.all().front().type() == ScaleObjectsTool::ScaleToolCornerHitType;
  });
}

void ScaleObjectsToolController::render(
  const InputState&,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  if (!m_tool.bounds().is_empty())
  {
    renderBounds(renderContext, renderBatch, m_tool.bounds());
    renderCornerHandles(
      renderContext, renderBatch, visibleCornerHandles(m_tool, renderContext.camera()));
  }

  renderDragSideHighlights(
    renderContext, renderBatch, m_tool.polygonsHighlightedByDrag());

  if (m_tool.hasDragSide())
  {
    renderDragSide(renderContext, renderBatch, m_tool.dragSide());
  }

  if (m_tool.hasDragEdge())
  {
    renderDragEdge(renderContext, renderBatch, m_tool.dragEdge());
  }

  if (m_tool.hasDragCorner())
  {
    renderDragCorner(renderContext, renderBatch, m_tool.dragCorner());
  }
}

bool ScaleObjectsToolController::cancel()
{
  return false;
}

// ScaleObjectsToolController2D

ScaleObjectsToolController2D::ScaleObjectsToolController2D(
  ScaleObjectsTool& tool, std::weak_ptr<MapDocument> document)
  : ScaleObjectsToolController(tool, document)
{
}

void ScaleObjectsToolController2D::doPick(
  const vm::ray3& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  m_tool.pick2D(pickRay, camera, pickResult);
}

// ScaleObjectsToolController3D

ScaleObjectsToolController3D::ScaleObjectsToolController3D(
  ScaleObjectsTool& tool, std::weak_ptr<MapDocument> document)
  : ScaleObjectsToolController(tool, document)
{
}

void ScaleObjectsToolController3D::doPick(
  const vm::ray3& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  m_tool.pick3D(pickRay, camera, pickResult);
}
} // namespace View
} // namespace TrenchBroom
