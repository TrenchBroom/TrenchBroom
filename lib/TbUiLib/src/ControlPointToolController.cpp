/*
 Copyright (C) 2026 Kristian Duske

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

#include "ui/ControlPointToolController.h"

#include "ui/ControlPointTool.h"
#include "ui/NodeHandleToolControllerParts.h"

#include <memory>

namespace tb::ui
{
namespace
{

class SelectControlPointPart
  : public NodeHandleToolSelectPartBase<ControlPointTool, mdl::ControlPointHandle>
{
public:
  explicit SelectControlPointPart(ControlPointTool& tool)
    : NodeHandleToolSelectPartBase{tool, mdl::ControlPointHandle::HandleHitType}
  {
  }

private:
  bool equalHandles(
    const mdl::ControlPointHandle& lhs, const mdl::ControlPointHandle& rhs) const override
  {
    return vm::squared_distance(lhs.position, rhs.position)
           < MaxHandleDistance * MaxHandleDistance;
  }
};

class MoveControlPointPart : public NodeHandleToolMovePartBase<ControlPointTool>
{
public:
  explicit MoveControlPointPart(ControlPointTool& tool)
    : NodeHandleToolMovePartBase{tool, mdl::ControlPointHandle::HandleHitType}
  {
  }

  bool shouldStartMove(const InputState& inputState) const override
  {
    return (
      inputState.mouseButtonsPressed(MouseButtons::Left) &&
      (inputState.modifierKeysPressed(ModifierKeys::None) ||    // horizontal movement
       inputState.modifierKeysPressed(ModifierKeys::Alt) ||     // vertical movement
       inputState.modifierKeysPressed(ModifierKeys::CtrlCmd) || // horizontal absolute snap
       inputState.modifierKeysPressed(
         ModifierKeys::CtrlCmd | ModifierKeys::Alt) // vertical absolute snap
       ));
  }

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override
  {
    NodeHandleToolMovePartBase::render(inputState, renderContext, renderBatch);

    if (!inputState.anyToolDragging())
    {
      if (const auto hit = m_tool.findDraggableHandle(inputState, m_hitType);
          hit.hasType(mdl::ControlPointHandle::HandleHitType))
      {
        const auto handle = m_tool.getHandlePosition(hit);
        if (inputState.mouseButtonsPressed(MouseButtons::Left))
        {
          m_tool.renderHandle(
            renderContext, renderBatch, handle, pref(Preferences::SelectedHandleColor));
        }
        else
        {
          m_tool.renderHandle(renderContext, renderBatch, handle);
        }
        m_tool.renderHighlight(renderContext, renderBatch, handle);
      }
    }
  }
};

} // namespace

ControlPointToolController::ControlPointToolController(ControlPointTool& tool)
  : NodeHandleToolControllerBase{tool}
{
  addController(std::make_unique<MoveControlPointPart>(tool));
  addController(std::make_unique<SelectControlPointPart>(tool));
}

} // namespace tb::ui
