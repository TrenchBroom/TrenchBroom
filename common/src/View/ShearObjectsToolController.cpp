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

#include "ShearObjectsToolController.h"

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
#include "View/ShearObjectsTool.h"

#include "kdl/memory_utils.h"

#include "vm/line.h"
#include "vm/plane.h"
#include "vm/polygon.h"

#include <cassert>

namespace TrenchBroom::View
{
namespace
{

HandlePositionProposer makeHandlePositionProposer(
  const InputState& inputState,
  const Grid& grid,
  const Model::Hit& dragStartHit,
  const vm::bbox3d& bboxAtDragStart,
  const vm::vec3d& handleOffset)
{
  const auto vertical = inputState.modifierKeysDown(ModifierKeys::MKAlt);
  const auto& camera = inputState.camera();

  const auto side = dragStartHit.target<BBoxSide>();
  const auto sideCenter = centerForBBoxSide(bboxAtDragStart, side);

  if (camera.perspectiveProjection())
  {
    if (vm::abs(side.normal) == vm::vec3d{0, 0, 1})
    {
      return makeHandlePositionProposer(
        makePlaneHandlePicker(vm::plane3d{sideCenter, side.normal}, handleOffset),
        makeRelativeHandleSnapper(grid));
    }

    if (vertical)
    {
      const auto verticalLine = vm::line3d{sideCenter, vm::vec3d{0, 0, 1}};
      return makeHandlePositionProposer(
        makeLineHandlePicker(verticalLine, handleOffset),
        makeRelativeHandleSnapper(grid));
    }

    const auto sideways =
      vm::line3d{sideCenter, vm::normalize(vm::cross(side.normal, vm::vec3d{0, 0, 1}))};
    return makeHandlePositionProposer(
      makeLineHandlePicker(sideways, handleOffset), makeRelativeHandleSnapper(grid));
  }

  const auto sideways = vm::line3d{
    sideCenter, vm::normalize(vm::cross(side.normal, vm::vec3d{camera.direction()}))};
  return makeHandlePositionProposer(
    makeLineHandlePicker(sideways, handleOffset), makeRelativeHandleSnapper(grid));
}

class ShearObjectsDragDelegate : public HandleDragTrackerDelegate
{
private:
  ShearObjectsTool& m_tool;

public:
  explicit ShearObjectsDragDelegate(ShearObjectsTool& tool)
    : m_tool{tool}
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
      m_tool.dragStartHit(),
      m_tool.bboxAtDragStart(),
      handleOffset);
  }

  std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState& inputState, const DragState& dragState) override
  {
    // Modifiers are only used for the perspective camera
    if (!inputState.camera().perspectiveProjection())
    {
      return std::nullopt;
    }

    const bool vertical = inputState.modifierKeysDown(ModifierKeys::MKAlt);
    if (vertical == m_tool.constrainVertical())
    {
      return std::nullopt;
    }

    // Can't do vertical restraint on these
    const auto side = m_tool.dragStartHit().target<BBoxSide>();
    if (side.normal == vm::vec3d{0, 0, 1} || side.normal == vm::vec3d{0, 0, -1})
    {
      return std::nullopt;
    }

    // Mouse might be over a different handle afterwards
    m_tool.refreshViews();

    m_tool.setConstrainVertical(vertical);
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
    m_tool.shearByDelta(delta);
    return DragStatus::Continue;
  }

  void end(const InputState& inputState, const DragState&) override
  {
    m_tool.commitShear();

    // The mouse is in a different place now so update the highlighted side
    m_tool.updatePickedSide(inputState.pickResult());
  }

  void cancel(const DragState&) override { m_tool.cancelShear(); }
};

std::tuple<vm::vec3d, vm::vec3d> getInitialHandlePositionAndHitPoint(
  const vm::bbox3d& bounds, const auto& hit)
{
  assert(hit.isMatch());
  assert(hit.hasType(ShearObjectsTool::ShearToolSideHitType));

  const auto side = hit.template target<BBoxSide>();
  return {centerForBBoxSide(bounds, side), hit.hitPoint()};
}

} // namespace

ShearObjectsToolController::ShearObjectsToolController(
  ShearObjectsTool& tool, std::weak_ptr<MapDocument> document)
  : m_tool{tool}
  , m_document{std::move(document)}
{
}

ShearObjectsToolController::~ShearObjectsToolController() = default;

Tool& ShearObjectsToolController::tool()
{
  return m_tool;
}

const Tool& ShearObjectsToolController::tool() const
{
  return m_tool;
}

void ShearObjectsToolController::pick(
  const InputState& inputState, Model::PickResult& pickResult)
{
  if (m_tool.applies())
  {
    // forward to either ShearObjectsTool::pick2D or ShearObjectsTool::pick3D
    doPick(inputState.pickRay(), inputState.camera(), pickResult);
  }
}

void ShearObjectsToolController::mouseMove(const InputState& inputState)
{
  if (m_tool.applies() && !inputState.anyToolDragging())
  {
    m_tool.updatePickedSide(inputState.pickResult());
  }
}

std::unique_ptr<GestureTracker> ShearObjectsToolController::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace Model::HitFilters;

  if (!inputState.mouseButtonsPressed(MouseButtons::Left))
  {
    return nullptr;
  }

  const auto vertical = inputState.modifierKeysDown(ModifierKeys::MKAlt);
  if (!(inputState.modifierKeysPressed(ModifierKeys::None) || vertical))
  {
    return nullptr;
  }

  if (!m_tool.applies())
  {
    return nullptr;
  }

  auto document = kdl::mem_lock(m_document);

  const auto& hit =
    inputState.pickResult().first(type(ShearObjectsTool::ShearToolSideHitType));
  if (!hit.isMatch())
  {
    return nullptr;
  }

  m_tool.startShearWithHit(hit);
  m_tool.setConstrainVertical(vertical);

  const auto [handlePosition, hitPoint] =
    getInitialHandlePositionAndHitPoint(m_tool.bounds(), hit);
  return createHandleDragTracker(
    ShearObjectsDragDelegate{m_tool}, inputState, handlePosition, hitPoint);
}

void ShearObjectsToolController::setRenderOptions(
  const InputState&, Renderer::RenderContext& renderContext) const
{
  renderContext.setForceHideSelectionGuide();
}

void ShearObjectsToolController::render(
  const InputState&,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  // render sheared box
  {
    auto renderService = Renderer::RenderService{renderContext, renderBatch};
    renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
    const auto mat = m_tool.bboxShearMatrix();
    const auto op = [&](const vm::vec3d& start, const vm::vec3d& end) {
      renderService.renderLine(vm::vec3f(mat * start), vm::vec3f(mat * end));
    };
    m_tool.bboxAtDragStart().for_each_edge(op);
  }

  // render shear handle
  if (const auto poly = m_tool.shearHandle())
  {
    // fill
    {
      auto renderService = Renderer::RenderService{renderContext, renderBatch};
      renderService.setShowBackfaces();
      renderService.setForegroundColor(pref(Preferences::ShearFillColor));
      renderService.renderFilledPolygon(poly->vertices());
    }

    // outline
    {
      auto renderService = Renderer::RenderService{renderContext, renderBatch};
      renderService.setLineWidth(2.0);
      renderService.setForegroundColor(pref(Preferences::ShearOutlineColor));
      renderService.renderPolygonOutline(poly->vertices());
    }
  }
}

bool ShearObjectsToolController::cancel()
{
  return false;
}

// ShearObjectsToolController2D

ShearObjectsToolController2D::ShearObjectsToolController2D(
  ShearObjectsTool& tool, std::weak_ptr<MapDocument> document)
  : ShearObjectsToolController{tool, std::move(document)}
{
}

void ShearObjectsToolController2D::doPick(
  const vm::ray3d& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult)
{
  m_tool.pick2D(pickRay, camera, pickResult);
}

// ShearObjectsToolController3D

ShearObjectsToolController3D::ShearObjectsToolController3D(
  ShearObjectsTool& tool, std::weak_ptr<MapDocument> document)
  : ShearObjectsToolController{tool, std::move(document)}
{
}

void ShearObjectsToolController3D::doPick(
  const vm::ray3d& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult)
{
  m_tool.pick3D(pickRay, camera, pickResult);
}

} // namespace TrenchBroom::View
