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

#include "ui/RotateToolController.h"

#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "render/RenderContext.h"
#include "ui/RotateHandle.h"
#include "ui/RotateHandleController.h"
#include "ui/RotateTool.h"
#include "ui/ToolController.h"

namespace tb::ui
{

RotateToolController::RotateToolController(RotateTool& tool)
  : m_tool{tool}
{
}

RotateToolController::~RotateToolController() = default;

Tool& RotateToolController::tool()
{
  return m_tool;
}

const Tool& RotateToolController::tool() const
{
  return m_tool;
}

void RotateToolController::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  const auto hit = doPick(inputState);
  if (hit.isMatch())
  {
    pickResult.addHit(hit);
  }
}

void RotateToolController::setRenderOptions(
  const InputState& inputState, render::RenderContext& renderContext) const
{
  setHandleRenderOptions(inputState, renderContext, RotateHandle::HandleHitType);
}

void RotateToolController::render(
  const InputState& inputState,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  doRenderHandle(renderContext, renderBatch);
  ToolControllerGroup::render(inputState, renderContext, renderBatch);
}

bool RotateToolController::cancel()
{
  return false;
}

RotateToolController2D::RotateToolController2D(RotateTool& tool)
  : RotateToolController{tool}
{
  addController(createCenterHandlePart2D(tool));
  addController(createRingHandlePart2D(tool));
}

mdl::Hit RotateToolController2D::doPick(const InputState& inputState)
{
  return m_tool.handle().pick2D(inputState.pickRay(), inputState.camera());
}

void RotateToolController2D::doRenderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_tool.handle().renderHandle2D(renderContext, renderBatch);
}

RotateToolController3D::RotateToolController3D(RotateTool& tool)
  : RotateToolController{tool}
{
  addController(createCenterHandlePart3D(tool));
  addController(createRingHandlePart3D(tool));
}

mdl::Hit RotateToolController3D::doPick(const InputState& inputState)
{
  return m_tool.handle().pick3D(inputState.pickRay(), inputState.camera());
}

void RotateToolController3D::doRenderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_tool.handle().renderHandle3D(renderContext, renderBatch);
}

} // namespace tb::ui
