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

#include "MoveObjectsToolController.h"

#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/ModelUtils.h"
#include "render/RenderContext.h"
#include "ui/GestureTracker.h"
#include "ui/MoveHandleDragTracker.h"
#include "ui/MoveObjectsTool.h"

#include <cassert>

namespace tb::ui
{
namespace
{

class MoveObjectsDragDelegate : public MoveHandleDragTrackerDelegate
{
private:
  MoveObjectsTool& m_tool;

public:
  explicit MoveObjectsDragDelegate(MoveObjectsTool& tool)
    : m_tool{tool}
  {
  }

  DragStatus move(
    const InputState& inputState,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override
  {
    switch (
      m_tool.move(inputState, proposedHandlePosition - dragState.currentHandlePosition))
    {
    case MoveObjectsTool::MoveResult::Continue:
      return DragStatus::Continue;
    case MoveObjectsTool::MoveResult::Deny:
      return DragStatus::Deny;
    case MoveObjectsTool::MoveResult::Cancel:
      return DragStatus::End;
      switchDefault();
    }
  }

  void end(const InputState& inputState, const DragState&) override
  {
    m_tool.endMove(inputState);
  }

  void cancel(const DragState&) override { m_tool.cancelMove(); }

  void setRenderOptions(
    const InputState&, render::RenderContext& renderContext) const override
  {
    renderContext.setForceShowSelectionGuide();
  }

  DragHandleSnapper makeDragHandleSnapper(
    const InputState&, const SnapMode) const override
  {
    return makeRelativeHandleSnapper(m_tool.grid());
  }
};

} // namespace

MoveObjectsToolController::MoveObjectsToolController(MoveObjectsTool& tool)
  : m_tool{tool}
{
}

MoveObjectsToolController::~MoveObjectsToolController() = default;

Tool& MoveObjectsToolController::tool()
{
  return m_tool;
}

const Tool& MoveObjectsToolController::tool() const
{
  return m_tool;
}

std::unique_ptr<GestureTracker> MoveObjectsToolController::acceptMouseDrag(
  const InputState& inputState)
{
  using namespace mdl::HitFilters;

  if (
    !inputState.modifierKeysPressed(ModifierKeys::None)
    && !inputState.modifierKeysPressed(ModifierKeys::MKAlt)
    && !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd)
    && !inputState.modifierKeysPressed(ModifierKeys::MKCtrlCmd | ModifierKeys::MKAlt))
  {
    return nullptr;
  }

  // The transitivelySelected() lets the hit query match entities/brushes inside a
  // selected group, even though the entities/brushes aren't selected themselves.

  if (const auto& hit =
        inputState.pickResult().first(type(mdl::nodeHitType()) && transitivelySelected());
      hit.isMatch())
  {
    if (m_tool.startMove(inputState))
    {
      return createMoveHandleDragTracker(
        MoveObjectsDragDelegate{m_tool}, inputState, hit.hitPoint(), hit.hitPoint());
    }
  }

  return nullptr;
}

bool MoveObjectsToolController::cancel()
{
  return false;
}

} // namespace tb::ui
