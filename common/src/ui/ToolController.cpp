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

#include "ToolController.h"

#include "Contracts.h"
#include "ui/DropTracker.h"
#include "ui/GestureTracker.h"
#include "ui/Tool.h"

namespace tb::ui
{

ToolController::~ToolController() = default;

bool ToolController::toolActive() const
{
  return tool().active();
}

void ToolController::pick(const InputState&, mdl::PickResult&) {}

void ToolController::modifierKeyChange(const InputState&) {}

void ToolController::mouseDown(const InputState&) {}

void ToolController::mouseUp(const InputState&) {}

bool ToolController::mouseClick(const InputState&)
{
  return false;
}

bool ToolController::mouseDoubleClick(const InputState&)
{
  return false;
}

void ToolController::mouseMove(const InputState&) {}

void ToolController::mouseScroll(const InputState&) {}

std::unique_ptr<GestureTracker> ToolController::acceptMouseDrag(const InputState&)
{
  return nullptr;
}

std::unique_ptr<GestureTracker> ToolController::acceptGesture(const InputState&)
{
  return nullptr;
}

bool ToolController::shouldAcceptDrop(const InputState&, const std::string&) const
{
  return false;
}

void ToolController::setRenderOptions(const InputState&, render::RenderContext&) const {}

void ToolController::render(
  const InputState&, render::RenderContext&, render::RenderBatch&)
{
}

bool ToolController::cancel()
{
  return false;
}

void ToolController::refreshViews()
{
  tool().refreshViews();
}

std::unique_ptr<DropTracker> ToolController::acceptDrop(
  const InputState&, const std::string& /* payload */)
{
  return nullptr;
}

ToolControllerGroup::ToolControllerGroup() = default;

ToolControllerGroup::~ToolControllerGroup() = default;

void ToolControllerGroup::addController(std::unique_ptr<ToolController> controller)
{
  contract_pre(controller != nullptr);

  m_chain.append(std::move(controller));
}

void ToolControllerGroup::pick(const InputState& inputState, mdl::PickResult& pickResult)
{
  m_chain.pick(inputState, pickResult);
}

void ToolControllerGroup::modifierKeyChange(const InputState& inputState)
{
  m_chain.modifierKeyChange(inputState);
}

void ToolControllerGroup::mouseDown(const InputState& inputState)
{
  m_chain.mouseDown(inputState);
}

void ToolControllerGroup::mouseUp(const InputState& inputState)
{
  m_chain.mouseUp(inputState);
}

bool ToolControllerGroup::mouseClick(const InputState& inputState)
{
  return m_chain.mouseClick(inputState);
}

bool ToolControllerGroup::mouseDoubleClick(const InputState& inputState)
{
  return m_chain.mouseDoubleClick(inputState);
}

void ToolControllerGroup::mouseMove(const InputState& inputState)
{
  m_chain.mouseMove(inputState);
}

void ToolControllerGroup::mouseScroll(const InputState& inputState)
{
  m_chain.mouseScroll(inputState);
}

std::unique_ptr<GestureTracker> ToolControllerGroup::acceptMouseDrag(
  const InputState& inputState)
{
  return doShouldHandleMouseDrag(inputState) ? m_chain.acceptMouseDrag(inputState)
                                             : nullptr;
}

std::unique_ptr<DropTracker> ToolControllerGroup::acceptDrop(
  const InputState& inputState, const std::string& payload)
{
  return doShouldAcceptDrop(inputState, payload) ? m_chain.dragEnter(inputState, payload)
                                                 : nullptr;
}

void ToolControllerGroup::setRenderOptions(
  const InputState& inputState, render::RenderContext& renderContext) const
{
  m_chain.setRenderOptions(inputState, renderContext);
}

void ToolControllerGroup::render(
  const InputState& inputState,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  m_chain.render(inputState, renderContext, renderBatch);
}

bool ToolControllerGroup::cancel()
{
  return m_chain.cancel();
}

bool ToolControllerGroup::doShouldHandleMouseDrag(const InputState&) const
{
  return true;
}

bool ToolControllerGroup::doShouldAcceptDrop(
  const InputState& inputState, const std::string& payload) const
{
  return m_chain.shouldAcceptDrop(inputState, payload);
}

} // namespace tb::ui
