/*
 Copyright (C) 2026 Jackson Palmer

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

#include "ui/SweepToolController.h"

#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "render/RenderContext.h"
#include "ui/InputState.h"
#include "ui/RotateHandle.h"
#include "ui/RotateHandleController.h"
#include "ui/SweepTool.h"
#include "ui/ToolController.h"

#include <memory>

namespace tb::ui
{
namespace
{

// one part serves both the 2D and 3D controllers since the handle is a sphere
std::unique_ptr<ToolController> createScaleHandlePart(SweepTool& tool)
{
  using namespace mdl::HitFilters;
  return createPointHandlePart(
    tool, type(SweepTool::ScaleHitType), [](const InputState& inputState) {
      return inputState.mouseButtons() == MouseButtons::Left
             && inputState.modifierKeys() == ModifierKeys::None;
    });
}

} // namespace

SweepToolController::SweepToolController(SweepTool& tool)
  : m_tool{tool}
{
}

SweepToolController::~SweepToolController() = default;

Tool& SweepToolController::tool()
{
  return m_tool;
}

const Tool& SweepToolController::tool() const
{
  return m_tool;
}

void SweepToolController::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  const auto rotateHit = doPick(inputState);
  const auto scaleHit = m_tool.pickScaleHandle(inputState.pickRay(), inputState.camera());

  const auto hit = scaleHit.isMatch() ? scaleHit : rotateHit;
  if (hit.isMatch())
  {
    pickResult.addHit(hit);
  }
}

void SweepToolController::setRenderOptions(
  const InputState& inputState, render::RenderContext& renderContext) const
{
  setHandleRenderOptions(
    inputState, renderContext, RotateHandle::HandleHitType | SweepTool::ScaleHitType);
}

void SweepToolController::render(
  const InputState& inputState,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  m_tool.renderPreview(renderContext, renderBatch);
  doRenderHandle(renderContext, renderBatch);
  m_tool.renderDestinationGhost(renderContext, renderBatch);
  m_tool.renderScaleHandle(renderContext, renderBatch);
  ToolControllerGroup::render(inputState, renderContext, renderBatch);
}

bool SweepToolController::cancel()
{
  return m_tool.cancel();
}

SweepToolController2D::SweepToolController2D(SweepTool& tool)
  : SweepToolController{tool}
{
  addController(createCenterHandlePart2D(tool));
  addController(createScaleHandlePart(tool));
  addController(createRingHandlePart2D(tool));
}

mdl::Hit SweepToolController2D::doPick(const InputState& inputState)
{
  return m_tool.handle().pick2D(inputState.pickRay(), inputState.camera());
}

void SweepToolController2D::doRenderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_tool.handle().renderHandle2D(renderContext, renderBatch);
}

SweepToolController3D::SweepToolController3D(SweepTool& tool)
  : SweepToolController{tool}
{
  addController(createCenterHandlePart3D(tool));
  addController(createScaleHandlePart(tool));
  addController(createRingHandlePart3D(tool));
}

mdl::Hit SweepToolController3D::doPick(const InputState& inputState)
{
  return m_tool.handle().pick3D(inputState.pickRay(), inputState.camera());
}

void SweepToolController3D::doRenderHandle(
  render::RenderContext& renderContext, render::RenderBatch& renderBatch)
{
  m_tool.handle().renderHandle3D(renderContext, renderBatch);
}

} // namespace tb::ui
