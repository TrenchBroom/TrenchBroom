/*
 Copyright (C) 2010 Kristian Duske
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

#include "ScaleToolController.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "render/Camera.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"
#include "ui/HandleDragTracker.h"
#include "ui/InputState.h"
#include "ui/ScaleTool.h"

#include "kdl/ranges/to.h"

#include "vm/line.h"
#include "vm/plane.h"
#include "vm/polygon.h"
#include "vm/segment.h"

#include <cassert>
#include <ranges>
#include <utility>

namespace tb::ui
{
ScaleToolController::ScaleToolController(ScaleTool& tool, mdl::Map& map)
  : m_tool{tool}
  , m_map{map}
{
}

ScaleToolController::~ScaleToolController() = default;

Tool& ScaleToolController::tool()
{
  return m_tool;
}

const Tool& ScaleToolController::tool() const
{
  return m_tool;
}

void ScaleToolController::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  if (m_tool.applies())
  {
    doPick(inputState.pickRay(), inputState.camera(), pickResult);
  }
}

static HandlePositionProposer makeHandlePositionProposer(
  const InputState& inputState,
  const mdl::Grid& grid,
  const mdl::Hit& dragStartHit,
  const vm::bbox3d& bboxAtDragStart,
  const vm::vec3d& handleOffset)
{
  const bool scaleAllAxes = inputState.modifierKeysDown(ModifierKeys::Shift);

  if (
    dragStartHit.type() == ScaleTool::ScaleToolEdgeHitType
    && inputState.camera().orthographicProjection() && !scaleAllAxes)
  {
    const auto plane = vm::plane3d{
      dragStartHit.hitPoint() + handleOffset,
      vm::vec3d{inputState.camera().direction()} * -1.0};
    return makeHandlePositionProposer(
      makePlaneHandlePicker(plane, handleOffset), makeRelativeHandleSnapper(grid));
  }
  else
  {
    assert(
      dragStartHit.type() == ScaleTool::ScaleToolSideHitType
      || dragStartHit.type() == ScaleTool::ScaleToolEdgeHitType
      || dragStartHit.type() == ScaleTool::ScaleToolCornerHitType);

    const auto handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);

    return makeHandlePositionProposer(
      makeLineHandlePicker(handleLine, handleOffset),
      makeAbsoluteLineHandleSnapper(grid, handleLine));
  }
}

static std::pair<AnchorPos, ProportionalAxes> modifierSettingsForInputState(
  const InputState& inputState)
{
  const auto centerAnchor = inputState.modifierKeysDown(ModifierKeys::Alt)
                              ? AnchorPos::Center
                              : AnchorPos::Opposite;

  auto scaleAllAxes = ProportionalAxes::None();
  if (inputState.modifierKeysDown(ModifierKeys::Shift))
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

void ScaleToolController::modifierKeyChange(const InputState& inputState)
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

void ScaleToolController::mouseMove(const InputState& inputState)
{
  if (m_tool.applies() && !inputState.anyToolDragging())
  {
    m_tool.updatePickedHandle(inputState.pickResult());
  }
}

namespace
{
class ScaleDragDelegate : public HandleDragTrackerDelegate
{
private:
  ScaleTool& m_tool;

public:
  explicit ScaleDragDelegate(ScaleTool& tool)
    : m_tool{tool}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& /* initialHandlePosition */,
    const vm::vec3d& handleOffset) override
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

  DragStatus update(
    const InputState&,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override
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

static std::tuple<vm::vec3d, vm::vec3d> getInitialHandlePositionAndHitPoint(
  const vm::bbox3d& bboxAtDragStart, const mdl::Hit& dragStartHit)
{
  const auto handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);
  return {handleLine.get_origin(), dragStartHit.hitPoint()};
}

std::unique_ptr<GestureTracker> ScaleToolController::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace mdl::HitFilters;

  if (!inputState.mouseButtonsPressed(MouseButtons::Left))
  {
    return nullptr;
  }

  if (!m_tool.applies())
  {
    return nullptr;
  }

  const auto& hit = inputState.pickResult().first(type(
    ScaleTool::ScaleToolSideHitType | ScaleTool::ScaleToolEdgeHitType
    | ScaleTool::ScaleToolCornerHitType));
  if (!hit.isMatch())
  {
    return nullptr;
  }

  m_tool.startScaleWithHit(hit);

  const auto [handlePosition, hitPoint] =
    getInitialHandlePositionAndHitPoint(m_tool.bounds(), hit);
  return createHandleDragTracker(
    ScaleDragDelegate{m_tool}, inputState, handlePosition, hitPoint);
}

void ScaleToolController::setRenderOptions(
  const InputState&, render::RenderContext& renderContext) const
{
  renderContext.setForceHideSelectionGuide();
}

static void renderBounds(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::bbox3d& bounds)
{
  auto renderService = render::RenderService{renderContext, renderBatch};
  renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
  renderService.renderBounds(vm::bbox3f{bounds});
}

static void renderCornerHandles(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const std::vector<vm::vec3d>& corners)
{
  auto renderService = render::RenderService{renderContext, renderBatch};
  renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));

  for (const auto& corner : corners)
  {
    renderService.renderHandle(vm::vec3f{corner});
  }
}

static void renderDragSideHighlights(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const std::vector<vm::polygon3f>& sides)
{
  // Highlight all sides that will be moving as a result of the Shift/Alt modifiers
  // (proporitional scaling or center anchor modifiers)
  for (const auto& side : sides)
  {
    {
      auto renderService = render::RenderService{renderContext, renderBatch};
      renderService.setShowBackfaces();
      renderService.setForegroundColor(pref(Preferences::ScaleFillColor));
      renderService.renderFilledPolygon(side.vertices());
    }

    // In 2D, additionally stroke the edges of this polyhedron, so it's visible even when
    // looking at it from an edge
    if (renderContext.camera().orthographicProjection())
    {
      auto renderService = render::RenderService{renderContext, renderBatch};
      renderService.setLineWidth(2.0);
      renderService.setForegroundColor(RgbaF{
        pref(Preferences::ScaleOutlineColor).toRgbF(),
        pref(Preferences::ScaleOutlineDimAlpha)});
      renderService.renderPolygonOutline(side.vertices());
    }
  }
}

static void renderDragSide(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::polygon3f& side)
{
  // draw the main highlighted handle
  auto renderService = render::RenderService{renderContext, renderBatch};
  renderService.setLineWidth(2.0);
  renderService.setForegroundColor(pref(Preferences::ScaleOutlineColor));
  renderService.renderPolygonOutline(side.vertices());
}

static void renderDragEdge(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::segment3f& edge)
{
  const auto& camera = renderContext.camera();

  auto renderService = render::RenderService{renderContext, renderBatch};
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
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::vec3f& corner)
{
  auto renderService = render::RenderService{renderContext, renderBatch};

  // the filled circular handle
  renderService.setForegroundColor(pref(Preferences::ScaleHandleColor));
  renderService.renderHandle(corner);

  // the ring around the handle
  renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
  renderService.renderHandleHighlight(corner);
}

static std::vector<vm::vec3d> visibleCornerHandles(
  const ScaleTool& tool, const render::Camera& camera)
{
  using namespace mdl::HitFilters;

  const auto cornerHandles = tool.cornerHandles();
  if (!camera.perspectiveProjection())
  {
    return cornerHandles;
  }

  const auto isVisible = [&](const auto& corner) {
    const auto ray = vm::ray3d{camera.pickRay(vm::vec3f{corner})};

    auto pr = mdl::PickResult{};
    if (camera.orthographicProjection())
    {
      tool.pick2D(ray, camera, pr);
    }
    else
    {
      tool.pick3D(ray, camera, pr);
    }

    return !pr.empty() && pr.all().front().type() == ScaleTool::ScaleToolCornerHitType;
  };

  return cornerHandles | std::views::filter(isVisible) | kdl::ranges::to<std::vector>();
}

void ScaleToolController::render(
  const InputState&,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
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

bool ScaleToolController::cancel()
{
  return false;
}

// ScaleToolController2D

ScaleToolController2D::ScaleToolController2D(ScaleTool& tool, mdl::Map& map)
  : ScaleToolController(tool, map)
{
}

void ScaleToolController2D::doPick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  m_tool.pick2D(pickRay, camera, pickResult);
}

// ScaleToolController3D

ScaleToolController3D::ScaleToolController3D(ScaleTool& tool, mdl::Map& map)
  : ScaleToolController(tool, map)
{
}

void ScaleToolController3D::doPick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  m_tool.pick3D(pickRay, camera, pickResult);
}

} // namespace tb::ui
