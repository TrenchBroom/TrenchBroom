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

#include "VertexToolController.h"

#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "ui/VertexTool.h"

#include <memory>

namespace tb::ui
{

/*
 * This is a bit awkward, but I'd rather not duplicate this logic into the two part
 * classes, and I can't move it up the inheritance hierarchy either. Nor can I introduce a
 * separate common base class for the two parts to contain this method due to the call to
 * the inherited findDraggableHandle method.
 */
mdl::Hit VertexToolController::findHandleHit(
  const InputState& inputState, const VertexToolController::PartBase& base)
{
  using namespace mdl::HitFilters;

  if (const auto vertexHit =
        base.findDraggableHandle(inputState, mdl::VertexHandleManager::HandleHitType);
      vertexHit.isMatch())
  {
    return vertexHit;
  }

  if (
    inputState.modifierKeysDown(ModifierKeys::Shift) && !inputState.pickResult().empty())
  {
    if (const auto& anyHit = inputState.pickResult().all().front(); anyHit.hasType(
          mdl::EdgeHandleManager::HandleHitType | mdl::FaceHandleManager::HandleHitType))
    {
      return anyHit;
    }
  }
  return mdl::Hit::NoHit;
}

std::vector<mdl::Hit> VertexToolController::findHandleHits(
  const InputState& inputState, const VertexToolController::PartBase& base)
{
  using namespace mdl::HitFilters;


  if (const auto vertexHits =
        base.findDraggableHandles(inputState, mdl::VertexHandleManager::HandleHitType);
      !vertexHits.empty())
  {
    return vertexHits;
  }

  if (
    inputState.modifierKeysDown(ModifierKeys::Shift) && !inputState.pickResult().empty())
  {
    const auto& anyHit = inputState.pickResult().all().front();
    if (anyHit.hasType(mdl::EdgeHandleManager::HandleHitType))
    {
      if (const auto edgeHits =
            inputState.pickResult().all(type(mdl::EdgeHandleManager::HandleHitType));
          !edgeHits.empty())
      {
        return edgeHits;
      }
    }
    else if (anyHit.hasType(mdl::FaceHandleManager::HandleHitType))
    {
      if (const auto faceHits =
            inputState.pickResult().all(type(mdl::FaceHandleManager::HandleHitType));
          !faceHits.empty())
      {
        return faceHits;
      }
    }
  }
  return {};
}

class VertexToolController::SelectVertexPart : public SelectPartBase<vm::vec3d>
{
public:
  explicit SelectVertexPart(VertexTool& tool)
    : SelectPartBase{tool, mdl::VertexHandleManager::HandleHitType}
  {
  }

protected:
  mdl::Hit findDraggableHandle(const InputState& inputState) const override
  {
    return VertexToolController::findHandleHit(inputState, *this);
  }

  std::vector<mdl::Hit> findDraggableHandles(const InputState& inputState) const override
  {
    return VertexToolController::findHandleHits(inputState, *this);
  }

private:
  bool equalHandles(const vm::vec3d& lhs, const vm::vec3d& rhs) const override
  {
    return vm::squared_distance(lhs, rhs) < MaxHandleDistance * MaxHandleDistance;
  }
};

class VertexToolController::MoveVertexPart : public MovePartBase
{
public:
  explicit MoveVertexPart(VertexTool& tool)
    : MovePartBase{tool, mdl::VertexHandleManager::HandleHitType}
  {
  }

private:
  bool mouseClick(const InputState& inputState) override
  {
    if (
      inputState.mouseButtonsPressed(MouseButtons::Left)
      && inputState.modifierKeysPressed(ModifierKeys::Alt | ModifierKeys::Shift)
      && m_tool.handleManager().selectedHandleCount() == 1)
    {
      if (const auto hit = VertexToolController::findHandleHit(inputState, *this);
          hit.hasType(mdl::VertexHandleManager::HandleHitType))
      {
        const auto sourcePos = m_tool.handleManager().selectedHandles().front();
        const auto targetPos = hit.target<vm::vec3d>();
        const auto delta = targetPos - sourcePos;
        m_tool.moveSelection(delta);
        return true;
      }
    }

    return false;
  }

  bool shouldStartMove(const InputState& inputState) const override
  {
    return (
      inputState.mouseButtonsPressed(MouseButtons::Left) &&
      (inputState.modifierKeysPressed(ModifierKeys::None) ||    // horizontal movement
       inputState.modifierKeysPressed(ModifierKeys::Alt) ||     // vertical movement
       inputState.modifierKeysPressed(ModifierKeys::CtrlCmd) || // horizontal absolute snap
       inputState.modifierKeysPressed(
         ModifierKeys::CtrlCmd | ModifierKeys::Alt) || // vertical absolute snap
       inputState.modifierKeysPressed(
         ModifierKeys::Shift) || // add new vertex and horizontal movement
       inputState.modifierKeysPressed(
         ModifierKeys::Shift | ModifierKeys::Alt) || // add new vertex and vertical movement
       inputState.modifierKeysPressed(
         ModifierKeys::Shift |
         ModifierKeys::CtrlCmd) || // add new vertex and horizontal movement with absolute snap
       inputState.modifierKeysPressed(
         ModifierKeys::Shift | ModifierKeys::CtrlCmd |
         ModifierKeys::Alt) // add new vertex and vertical movement with absolute snap
       ));
  }

  void render(
    const InputState& inputState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) override
  {
    MovePartBase::render(inputState, renderContext, renderBatch);

    if (!inputState.anyToolDragging())
    {
      if (const auto hit = findDraggableHandle(inputState); hit.hasType(
            mdl::EdgeHandleManager::HandleHitType
            | mdl::FaceHandleManager::HandleHitType))
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

protected:
  mdl::Hit findDraggableHandle(const InputState& inputState) const override
  {
    return VertexToolController::findHandleHit(inputState, *this);
  }

  std::vector<mdl::Hit> findDraggableHandles(const InputState& inputState) const override
  {
    return VertexToolController::findHandleHits(inputState, *this);
  }
};

VertexToolController::VertexToolController(VertexTool& tool)
  : VertexToolControllerBase(tool)
{
  addController(std::make_unique<MoveVertexPart>(tool));
  addController(std::make_unique<SelectVertexPart>(tool));
}

} // namespace tb::ui
