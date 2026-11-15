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

#include "ui/ScaleToolController.h"

#include "base/PreferenceManager.h"
#include "gl/Camera.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "prefs/Preferences.h"
#include "render/RenderContext.h"
#include "render/RenderService.h"
#include "ui/HandleDragTracker.h"
#include "ui/InputState.h"
#include "ui/ScaleTool.h"

#include "kd/ranges/to.h"

#include "vm/line.h"
#include "vm/plane.h"
#include "vm/polygon.h"
#include "vm/segment.h"

#include <cassert>
#include <ranges>
#include <tuple>
#include <utility>

namespace tb::ui
{
namespace
{

std::pair<AnchorPos, ProportionalAxes> modifierSettingsForInputState(
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

std::tuple<vm::vec3d, vm::vec3d> getInitialHandlePositionAndHitPoint(
  const vm::bbox3d& bboxAtDragStart, const mdl::Hit& dragStartHit)
{
  const auto handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);
  return {handleLine.get_origin(), dragStartHit.hitPoint()};
}

HandlePositionProposer makeHandlePositionProposer(
  const InputState& inputState,
  const mdl::Grid& grid,
  const mdl::Hit& dragStartHit,
  const vm::bbox3d& bboxAtDragStart,
  const vm::vec3d& handleOffset)
{
  const bool scaleAllAxes = inputState.modifierKeysDown(ModifierKeys::Shift);

  if (
    dragStartHit.type() == ScaleTool::EdgeHitType
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
    contract_assert(dragStartHit.hasType(ScaleTool::AnyHitType));

    const auto handleLine = handleLineForHit(bboxAtDragStart, dragStartHit);

    return makeHandlePositionProposer(
      makeLineHandlePicker(handleLine, handleOffset),
      makeAbsoluteLineHandleSnapper(grid, handleLine));
  }
}

void renderBounds(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::bbox3d& bounds)
{
  auto renderService = render::RenderService{renderContext, renderBatch};
  renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
  renderService.renderBounds(vm::bbox3f{bounds});
}

void renderCornerHandles(
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

std::vector<vm::vec3d> visibleCornerHandles(
  const vm::bbox3d& bounds, const gl::Camera& camera)
{
  using namespace mdl::HitFilters;

  const auto corners = cornerHandles(bounds);
  if (!camera.perspectiveProjection())
  {
    return corners;
  }

  const auto isVisible = [&](const auto& corner) {
    const auto ray = vm::ray3d{camera.pickRay(vm::vec3f{corner})};

    auto pr = mdl::PickResult{};
    pickBBoxHandles3D(ray, camera, bounds, pr);

    return !pr.empty() && pr.all().front().type() == ScaleTool::CornerHitType;
  };

  return corners | std::views::filter(isVisible) | kdl::ranges::to<std::vector>();
}

void renderDragSideHighlights(
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
        pref(Preferences::ScaleOutlineColor).to<RgbF>(),
        pref(Preferences::ScaleOutlineDimAlpha)});
      renderService.renderPolygonOutline(side.vertices());
    }
  }
}

void renderDragSide(
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

void renderDragEdge(
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

void renderDragCorner(
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

void renderDragHighlights(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::bbox3d& bounds,
  const mdl::Hit& hit,
  const ProportionalAxes& proportionalAxes,
  const AnchorPos anchorPos)
{
  renderDragSideHighlights(
    renderContext,
    renderBatch,
    polygonsHighlightedByDrag(bounds, hit, proportionalAxes, anchorPos));

  if (hit.type() == ScaleTool::SideHitType)
  {
    const auto side = hit.target<BBoxSide>();
    renderDragSide(
      renderContext, renderBatch, vm::polygon3f{polygonForBBoxSide(bounds, side)});
  }
  else if (hit.type() == ScaleTool::EdgeHitType)
  {
    const auto edge = hit.target<BBoxEdge>();
    renderDragEdge(
      renderContext, renderBatch, vm::segment3f{pointsForBBoxEdge(bounds, edge)});
  }
  else if (hit.type() == ScaleTool::CornerHitType)
  {
    const auto corner = hit.target<BBoxCorner>();
    renderDragCorner(
      renderContext, renderBatch, vm::vec3f{pointForBBoxCorner(bounds, corner)});
  }
}

void renderBoundsAndHandles(
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch,
  const vm::bbox3d& bounds,
  const mdl::Hit& hit,
  const ProportionalAxes& proportionalAxes,
  const AnchorPos anchorPos)
{
  if (!bounds.is_empty())
  {
    renderBounds(renderContext, renderBatch, bounds);
    renderCornerHandles(
      renderContext, renderBatch, visibleCornerHandles(bounds, renderContext.camera()));
  }

  renderDragHighlights(
    renderContext, renderBatch, bounds, hit, proportionalAxes, anchorPos);
}

class ScaleDragDelegate : public HandleDragTrackerDelegate
{
private:
  ScaleTool& m_tool;
  ScaleDragState m_scaleDragState;

public:
  ScaleDragDelegate(ScaleTool& tool, ScaleDragState scaleDragState)
    : m_tool{tool}
    , m_scaleDragState{std::move(scaleDragState)}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& /* initialHandlePosition */,
    const vm::vec3d& handleOffset) override
  {
    return makeHandlePositionProposer(
      inputState,
      m_tool.grid(),
      m_scaleDragState.dragStartHit,
      m_scaleDragState.bboxAtDragStart,
      handleOffset);
  }

  std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState& inputState, const DragState& dragState) override
  {
    std::tie(m_scaleDragState.anchorPos, m_scaleDragState.proportionalAxes) =
      modifierSettingsForInputState(inputState);

    return UpdateDragConfig{
      makeHandlePositionProposer(
        inputState,
        m_tool.grid(),
        m_scaleDragState.dragStartHit,
        m_scaleDragState.bboxAtDragStart,
        dragState.handleOffset),
      ResetInitialHandlePosition::Keep};
  }

  DragStatus update(
    const InputState&,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override
  {
    const auto delta = proposedHandlePosition - dragState.currentHandlePosition;
    return m_tool.scaleByDelta(delta, m_scaleDragState) ? DragStatus::Continue
                                                        : DragStatus::Deny;
  }

  void end(const InputState& inputState, const DragState&) override
  {
    m_tool.commitScale(m_scaleDragState);

    // The mouse is in a different place now so update the highlighted side
    m_tool.updatePickedHandle(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancelScale(); }

  void render(
    const InputState&,
    const DragState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) const override
  {
    renderBoundsAndHandles(
      renderContext,
      renderBatch,
      m_scaleDragState.bboxDuringDrag,
      m_scaleDragState.dragStartHit,
      m_scaleDragState.proportionalAxes,
      m_scaleDragState.anchorPos);
  }
};

} // namespace

ScaleToolController::ScaleToolController(ScaleTool& tool)
  : m_tool{tool}
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

  const auto& hit = inputState.pickResult().first(type(ScaleTool::AnyHitType));
  if (!hit.isMatch())
  {
    return nullptr;
  }

  m_tool.beginScale();

  const auto [anchorPos, proportionalAxes] = modifierSettingsForInputState(inputState);
  const auto [handlePosition, hitPoint] =
    getInitialHandlePositionAndHitPoint(m_tool.bounds(), hit);
  return createHandleDragTracker(
    ScaleDragDelegate{
      m_tool, ScaleDragState{hit, m_tool.bounds(), anchorPos, proportionalAxes}},
    inputState,
    handlePosition,
    hitPoint);
}

void ScaleToolController::setRenderOptions(
  const InputState&, render::RenderContext& renderContext) const
{
  renderContext.setForceHideSelectionGuide();
}

void ScaleToolController::render(
  const InputState&,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  if (m_tool.applies() && !m_tool.dragging())
  {
    renderBoundsAndHandles(
      renderContext,
      renderBatch,
      m_tool.bounds(),
      m_tool.pickedHandle(),
      m_tool.proportionalAxes(),
      m_tool.anchorPos());
  }
}

bool ScaleToolController::cancel()
{
  return false;
}

// ScaleToolController2D

ScaleToolController2D::ScaleToolController2D(ScaleTool& tool)
  : ScaleToolController(tool)
{
}

void ScaleToolController2D::doPick(
  const vm::ray3d& pickRay, const gl::Camera& camera, mdl::PickResult& pickResult) const
{
  m_tool.pick2D(pickRay, camera, pickResult);
}

// ScaleToolController3D

ScaleToolController3D::ScaleToolController3D(ScaleTool& tool)
  : ScaleToolController(tool)
{
}

void ScaleToolController3D::doPick(
  const vm::ray3d& pickRay, const gl::Camera& camera, mdl::PickResult& pickResult) const
{
  m_tool.pick3D(pickRay, camera, pickResult);
}

} // namespace tb::ui
