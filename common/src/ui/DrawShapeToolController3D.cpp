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

#include "DrawShapeToolController3D.h"

#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"
#include "render/Camera.h"
#include "ui/DrawShapeTool.h"
#include "ui/Grid.h"
#include "ui/HandleDragTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"

#include "kdl/memory_utils.h"

#include "vm/bbox.h"
#include "vm/line.h"
#include "vm/plane.h"

namespace tb::ui
{
namespace
{

class DrawShapeDragDelegate : public HandleDragTrackerDelegate
{
private:
  DrawShapeTool& m_tool;
  vm::bbox3d m_worldBounds;

public:
  DrawShapeDragDelegate(DrawShapeTool& tool, const vm::bbox3d& worldBounds)
    : m_tool{tool}
    , m_worldBounds{worldBounds}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& handleOffset) override
  {
    const auto currentBounds =
      makeBounds(inputState, initialHandlePosition, initialHandlePosition);
    m_tool.update(currentBounds, vm::axis::z);
    m_tool.refreshViews();

    return makeHandlePositionProposer(
      makePlaneHandlePicker(vm::horizontal_plane(initialHandlePosition), handleOffset),
      makeIdentityHandleSnapper());
  }

  std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState& inputState, const DragState& dragState) override
  {
    if (inputState.modifierKeys() == ModifierKeys::Shift)
    {
      const auto currentBounds = makeBounds(
        inputState, dragState.initialHandlePosition, dragState.currentHandlePosition);

      if (!currentBounds.is_empty())
      {
        m_tool.update(currentBounds, vm::axis::z);
        m_tool.refreshViews();
      }
    }

    if (inputState.modifierKeys() == ModifierKeys::MKAlt)
    {
      return UpdateDragConfig{
        makeHandlePositionProposer(
          makeLineHandlePicker(
            vm::line3d{dragState.currentHandlePosition, vm::vec3d{0, 0, 1}},
            dragState.handleOffset),
          makeIdentityHandleSnapper()),
        ResetInitialHandlePosition::Keep};
    }

    return UpdateDragConfig{
      makeHandlePositionProposer(
        makePlaneHandlePicker(
          vm::horizontal_plane(dragState.currentHandlePosition), dragState.handleOffset),
        makeIdentityHandleSnapper()),
      ResetInitialHandlePosition::Keep};
  }

  DragStatus update(
    const InputState& inputState,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override
  {
    if (updateBounds(
          inputState,
          dragState.initialHandlePosition,
          dragState.currentHandlePosition,
          proposedHandlePosition))
    {
      m_tool.refreshViews();
      return DragStatus::Continue;
    }
    return DragStatus::Deny;
  }

  void end(const InputState&, const DragState&) override { m_tool.createBrushes(); }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void render(
    const InputState&,
    const DragState&,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) const override
  {
    m_tool.render(renderContext, renderBatch);
  }

private:
  bool updateBounds(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& lastHandlePosition,
    const vm::vec3d& currentHandlePosition)
  {
    const auto lastBounds =
      makeBounds(inputState, initialHandlePosition, lastHandlePosition);
    const auto currentBounds =
      makeBounds(inputState, initialHandlePosition, currentHandlePosition);

    if (currentBounds.is_empty() || currentBounds == lastBounds)
    {
      return false;
    }

    m_tool.update(currentBounds, vm::axis::z);
    return true;
  }

  vm::bbox3d makeBounds(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& currentHandlePosition) const
  {
    auto bounds = snapBounds(
      inputState,
      vm::bbox3d{
        vm::min(initialHandlePosition, currentHandlePosition),
        vm::max(initialHandlePosition, currentHandlePosition),
      });

    if (inputState.modifierKeysDown(ModifierKeys::Shift))
    {
      const auto includeZAxis = inputState.modifierKeysDown(ModifierKeys::MKAlt);

      const auto xyAxes = vm::vec3d{1, 0, 0} + vm::vec3d{0, 1, 0};
      const auto zAxis = vm::vec3d{0, 0, 1};
      const auto allAxes = vm::vec3d{1, 1, 1};
      const auto noAxis = vm::vec3d{0, 0, 0};
      const auto maxLengthAxes = includeZAxis ? allAxes : xyAxes;
      const auto zLengthAxis = includeZAxis ? noAxis : zAxis;

      const auto maxLength = vm::get_abs_max_component(bounds.size() * maxLengthAxes);

      const auto lengthDiff = zLengthAxis * bounds.size() + maxLengthAxes * maxLength;

      // The direction in which the user is dragging per component:
      const auto dragDir = vm::step(initialHandlePosition, currentHandlePosition);
      bounds = vm::bbox3d{
        vm::mix(bounds.min, bounds.max - lengthDiff, vm::vec3d{1, 1, 1} - dragDir),
        vm::mix(bounds.max, bounds.min + lengthDiff, dragDir),
      };
    }

    return vm::intersect(bounds, m_worldBounds);
  }

  vm::bbox3d snapBounds(const InputState& inputState, vm::bbox3d bounds) const
  {

    // prevent flickering due to very small rounding errors
    bounds.min = vm::correct(bounds.min);
    bounds.max = vm::correct(bounds.max);

    const auto& grid = m_tool.grid();
    bounds.min = grid.snapDown(bounds.min);
    bounds.max = grid.snapUp(bounds.max);

    const auto& camera = inputState.camera();
    const auto cameraPosition = vm::vec3d{camera.position()};

    for (size_t i = 0; i < 3; i++)
    {
      if (bounds.max[i] <= bounds.min[i])
      {
        if (bounds.min[i] < cameraPosition[i])
        {
          bounds.max[i] = bounds.min[i] + grid.actualSize();
        }
        else
        {
          bounds.min[i] = bounds.max[i] - grid.actualSize();
        }
      }
    }

    return bounds;
  }
};

} // namespace

DrawShapeToolController3D::DrawShapeToolController3D(
  DrawShapeTool& tool, std::weak_ptr<MapDocument> document)
  : m_tool{tool}
  , m_document{std::move(document)}
{
}

Tool& DrawShapeToolController3D::tool()
{
  return m_tool;
}

const Tool& DrawShapeToolController3D::tool() const
{
  return m_tool;
}

std::unique_ptr<GestureTracker> DrawShapeToolController3D::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace mdl::HitFilters;

  if (!inputState.mouseButtonsPressed(MouseButtons::Left))
  {
    return nullptr;
  }

  if (!inputState.checkModifierKeys(
        ModifierKeyPressed::No, ModifierKeyPressed::No, ModifierKeyPressed::DontCare))
  {
    return nullptr;
  }

  auto document = kdl::mem_lock(m_document);
  if (document->hasSelection())
  {
    return nullptr;
  }

  const auto& hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
  const auto initialHandlePosition =
    hit.isMatch() ? hit.hitPoint() : inputState.defaultPointUnderMouse();

  return createHandleDragTracker(
    DrawShapeDragDelegate{m_tool, document->worldBounds()},
    inputState,
    initialHandlePosition,
    initialHandlePosition);
}

bool DrawShapeToolController3D::cancel()
{
  return m_tool.cancel();
}

} // namespace tb::ui
