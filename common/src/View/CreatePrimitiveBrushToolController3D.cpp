/*
 Copyright (C) 2010-2023 Kristian Duske, Nathan "jitspoe" Wulf

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

#include "CreatePrimitiveBrushToolController3D.h"

#include "FloatType.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "View/CreatePrimitiveBrushTool.h"
#include "View/Grid.h"
#include "View/HandleDragTracker.h"
#include "View/InputState.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>

#include <vecmath/bbox.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/vec.h>

#include <cassert>

namespace TrenchBroom
{
namespace View
{
CreatePrimitiveBrushToolController3D::CreatePrimitiveBrushToolController3D(
  CreatePrimitiveBrushTool& tool, std::weak_ptr<MapDocument> document)
  : m_tool{tool}
  , m_document{document}
{
}

Tool& CreatePrimitiveBrushToolController3D::tool()
{
  return m_tool;
}

const Tool& CreatePrimitiveBrushToolController3D::tool() const
{
  return m_tool;
}

namespace
{
class CreatePrimitiveBrushDragDelegate : public HandleDragTrackerDelegate
{
private:
  CreatePrimitiveBrushTool& m_tool;
  vm::bbox3 m_worldBounds;

public:
  CreatePrimitiveBrushDragDelegate(CreatePrimitiveBrushTool& tool, const vm::bbox3& worldBounds)
    : m_tool{tool}
    , m_worldBounds{worldBounds}
  {
  }

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3& initialHandlePosition,
    const vm::vec3& handleOffset)
  {
    const auto currentBounds =
      makeBounds(inputState, initialHandlePosition, initialHandlePosition);
    m_tool.update(currentBounds);
    m_tool.refreshViews();

    return makeHandlePositionProposer(
      makePlaneHandlePicker(vm::horizontal_plane(initialHandlePosition), handleOffset),
      makeIdentityHandleSnapper());
  }

  std::optional<UpdateDragConfig> modifierKeyChange(
    const InputState& inputState, const DragState& dragState)
  {
    if (inputState.modifierKeys() == ModifierKeys::MKAlt)
    {
      return UpdateDragConfig{
        makeHandlePositionProposer(
          makeLineHandlePicker(
            vm::line3{dragState.currentHandlePosition, vm::vec3::pos_z()},
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

  DragStatus drag(
    const InputState& inputState,
    const DragState& dragState,
    const vm::vec3& proposedHandlePosition)
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

  void end(const InputState&, const DragState&) { m_tool.createBrush(); }

  void cancel(const DragState&) { m_tool.cancel(); }

  void render(
    const InputState&,
    const DragState&,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch) const
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
    const auto bounds = vm::bbox3{
      vm::min(initialHandlePosition, currentHandlePosition),
      vm::max(initialHandlePosition, currentHandlePosition)};
    return vm::intersect(snapBounds(inputState, bounds), m_worldBounds);
  }

  vm::bbox3 snapBounds(const InputState& inputState, vm::bbox3 bounds) const
  {

    // prevent flickering due to very small rounding errors
    bounds.min = vm::correct(bounds.min);
    bounds.max = vm::correct(bounds.max);

    const auto& grid = m_tool.grid();
    bounds.min = grid.snapDown(bounds.min);
    bounds.max = grid.snapUp(bounds.max);

    const auto& camera = inputState.camera();
    const auto cameraPosition = vm::vec3{camera.position()};

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

std::unique_ptr<DragTracker> CreatePrimitiveBrushToolController3D::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace Model::HitFilters;

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
    document->deselectAll();
  }

  const auto& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
  const auto initialHandlePosition =
    hit.isMatch() ? hit.hitPoint() : inputState.defaultPointUnderMouse();

  return createHandleDragTracker(
    CreatePrimitiveBrushDragDelegate{m_tool, document->worldBounds()},
    inputState,
    initialHandlePosition,
    initialHandlePosition);
}

bool CreatePrimitiveBrushToolController3D::cancel()
{
  return false;
}
} // namespace View
} // namespace TrenchBroom
