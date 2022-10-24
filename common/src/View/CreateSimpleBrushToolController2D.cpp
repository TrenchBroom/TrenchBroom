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

#include "CreateSimpleBrushToolController2D.h"

#include "Renderer/Camera.h"
#include "View/CreateSimpleBrushTool.h"
#include "View/Grid.h"
#include "View/HandleDragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vecmath/intersection.h>
#include <vecmath/scalar.h>

#include <optional>

namespace TrenchBroom
{
namespace View
{
CreateSimpleBrushToolController2D::CreateSimpleBrushToolController2D(
  CreateSimpleBrushTool& tool, std::weak_ptr<MapDocument> document)
  : m_tool{tool}
  , m_document{document}
{
}

Tool& CreateSimpleBrushToolController2D::tool()
{
  return m_tool;
}

const Tool& CreateSimpleBrushToolController2D::tool() const
{
  return m_tool;
}

namespace
{
class CreateSimpleBrushDragDelegate : public HandleDragTrackerDelegate
{
private:
  CreateSimpleBrushTool& m_tool;
  vm::bbox3 m_worldBounds;
  vm::bbox3 m_referenceBounds;

public:
  CreateSimpleBrushDragDelegate(
    CreateSimpleBrushTool& tool,
    const vm::bbox3& worldBounds,
    const vm::bbox3& referenceBounds)
    : m_tool{tool}
    , m_worldBounds{worldBounds}
    , m_referenceBounds{referenceBounds}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset) override
  {
    const auto currentBounds =
      makeBounds(inputState, initialHandlePosition, initialHandlePosition);
    m_tool.update(currentBounds);
    m_tool.refreshViews();

    const auto& camera = inputState.camera();
    const auto plane = vm::plane3{
      initialHandlePosition,
      vm::vec3{vm::get_abs_max_component_axis(camera.direction())}};

    return makeHandlePositionProposer(
      makePlaneHandlePicker(plane, handleOffset), makeIdentityHandleSnapper());
  }

  DragStatus drag(
    const InputState& inputState,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition) override
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

  void end(const InputState&, const DragState&) override { m_tool.createBrush(); }

  void cancel(const DragState&) override { m_tool.cancel(); }

  void render(
    const InputState&,
    const DragState&,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const override
  {
    m_tool.render(renderContext, renderBatch);
  }

private:
  bool updateBounds(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& lastHandlePosition,
    const vm::vec3& currentHandlePosition)
  {
    const auto lastBounds =
      makeBounds(inputState, initialHandlePosition, lastHandlePosition);
    const auto currentBounds =
      makeBounds(inputState, initialHandlePosition, currentHandlePosition);

    if (currentBounds.is_empty() || currentBounds == lastBounds)
    {
      return false;
    }

    m_tool.update(currentBounds);
    return true;
  }

  vm::bbox3 makeBounds(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& currentHandlePosition) const
  {
    const auto bounds = vm::merge(
      vm::bbox3{initialHandlePosition, initialHandlePosition}, currentHandlePosition);
    return vm::intersect(snapBounds(inputState, bounds), m_worldBounds);
  }

  vm::bbox3 snapBounds(const InputState& inputState, const vm::bbox3& bounds) const
  {
    const auto& grid = m_tool.grid();
    auto min = grid.snapDown(bounds.min);
    auto max = grid.snapUp(bounds.max);

    const auto& camera = inputState.camera();
    const auto& refBounds = m_referenceBounds;
    const auto factors =
      vm::vec3{vm::abs(vm::get_abs_max_component_axis(camera.direction()))};
    min = vm::mix(min, refBounds.min, factors);
    max = vm::mix(max, refBounds.max, factors);

    return vm::bbox3{min, max};
  }
};
} // namespace

std::unique_ptr<DragTracker> CreateSimpleBrushToolController2D::acceptMouseDrag(
  const InputState& inputState)
{
  if (!inputState.mouseButtonsPressed(MouseButtons::MBLeft))
  {
    return nullptr;
  }
  if (!inputState.modifierKeysPressed(ModifierKeys::MKNone))
  {
    return nullptr;
  }

  auto document = kdl::mem_lock(m_document);
  if (document->hasSelection())
  {
    return nullptr;
  }

  const auto& bounds = document->referenceBounds();
  const auto& camera = inputState.camera();
  const auto plane =
    vm::plane3{bounds.min, vm::vec3{vm::get_abs_max_component_axis(camera.direction())}};

  const auto distance = vm::intersect_ray_plane(inputState.pickRay(), plane);
  if (vm::is_nan(distance))
  {
    return nullptr;
  }

  const auto initialHandlePosition =
    vm::point_at_distance(inputState.pickRay(), distance);
  return createHandleDragTracker(
    CreateSimpleBrushDragDelegate{
      m_tool, document->worldBounds(), document->referenceBounds()},
    inputState,
    initialHandlePosition,
    initialHandlePosition);
}

bool CreateSimpleBrushToolController2D::cancel()
{
  return false;
}
} // namespace View
} // namespace TrenchBroom
