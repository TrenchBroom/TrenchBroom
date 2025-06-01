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

#include "ToolBox.h"

#include <QDateTime>
#include <QDebug>

#include "ui/DropTracker.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/Tool.h"
#include "ui/ToolChain.h"
#include "ui/ToolController.h"

#include "kdl/vector_utils.h"

#include <cassert>
#include <string>
#include <utility>

namespace tb::ui
{
ToolBox::ToolBox() = default;
ToolBox::~ToolBox() = default;

void ToolBox::addTool(Tool& tool)
{
  m_notifierConnection += tool.refreshViewsNotifier.connect(refreshViewsNotifier);
  m_notifierConnection +=
    tool.toolHandleSelectionChangedNotifier.connect(toolHandleSelectionChangedNotifier);
}

void ToolBox::pick(
  ToolChain& chain, const InputState& inputState, mdl::PickResult& pickResult)
{
  chain.pick(inputState, pickResult);
}

bool ToolBox::dragEnter(
  ToolChain& chain, const InputState& inputState, const std::string& text)
{
  if (!m_enabled || !chain.shouldAcceptDrop(inputState, text))
  {
    return false;
  }

  if (m_dropTracker)
  {
    dragLeave(chain, inputState);
  }

  deactivateAllTools();
  m_dropTracker = chain.dragEnter(inputState, text);
  return m_dropTracker != nullptr;
}

bool ToolBox::dragMove(
  ToolChain& /* chain */, const InputState& inputState, const std::string& /* text */)
{
  if (!m_enabled || !m_dropTracker)
  {
    return false;
  }

  m_dropTracker->move(inputState);
  return true;
}

void ToolBox::dragLeave(ToolChain& /* chain */, const InputState& inputState)
{
  if (!m_enabled || !m_dropTracker)
  {
    return;
  }

  m_dropTracker->leave(inputState);
  m_dropTracker = nullptr;
}

bool ToolBox::dragDrop(
  ToolChain& /* chain */, const InputState& inputState, const std::string& /* text */)
{
  if (!m_enabled || !m_dropTracker)
  {
    return false;
  }

  const auto result = m_dropTracker->drop(inputState);
  m_dropTracker = nullptr;
  return result;
}

void ToolBox::modifierKeyChange(ToolChain& chain, const InputState& inputState)
{
  if (m_enabled)
  {
    chain.modifierKeyChange(inputState);
    if (m_gestureTracker)
    {
      m_gestureTracker->modifierKeyChange(inputState);
    }
  }
}

void ToolBox::mouseDown(ToolChain& chain, const InputState& inputState) const
{
  if (m_enabled)
  {
    chain.mouseDown(inputState);
  }
}

void ToolBox::mouseUp(ToolChain& chain, const InputState& inputState) const
{
  if (m_enabled)
  {
    chain.mouseUp(inputState);
  }
}

bool ToolBox::mouseClick(ToolChain& chain, const InputState& inputState) const
{
  if (m_enabled)
  {
    return chain.mouseClick(inputState);
  }

  return false;
}

void ToolBox::mouseDoubleClick(ToolChain& chain, const InputState& inputState) const
{
  if (m_enabled)
  {
    chain.mouseDoubleClick(inputState);
  }
}

void ToolBox::mouseMove(ToolChain& chain, const InputState& inputState) const
{
  if (m_enabled)
  {
    chain.mouseMove(inputState);
  }
}

bool ToolBox::dragging() const
{
  return m_gestureTracker != nullptr;
}

void ToolBox::startMouseDrag(ToolChain& chain, const InputState& inputState)
{
  if (m_enabled)
  {
    m_gestureTracker = chain.acceptMouseDrag(inputState);
  }
}

bool ToolBox::mouseDrag(const InputState& inputState)
{
  assert(enabled() && dragging());
  return m_gestureTracker->update(inputState);
}

void ToolBox::endMouseDrag(const InputState& inputState)
{
  assert(enabled() && dragging());
  m_gestureTracker->end(inputState);
  m_gestureTracker = nullptr;
}

void ToolBox::cancelMouseDrag()
{
  assert(dragging());
  m_gestureTracker->cancel();
  m_gestureTracker = nullptr;
}

void ToolBox::mouseScroll(ToolChain& chain, const InputState& inputState)
{
  if (m_enabled)
  {
    if (m_gestureTracker)
    {
      m_gestureTracker->mouseScroll(inputState);
    }
    else
    {
      chain.mouseScroll(inputState);
    }
  }
}

void ToolBox::startGesture(ToolChain& chain, const InputState& inputState)
{
  assert(!m_gestureTracker);

  if (m_enabled)
  {
    m_gestureTracker = chain.acceptGesture(inputState);
  }
}

void ToolBox::gesturePan(const InputState& inputState)
{
  assert(enabled());
  if (m_gestureTracker)
  {
    m_gestureTracker->update(inputState);
  }
}

void ToolBox::gestureZoom(const InputState& inputState)
{
  assert(enabled());
  if (m_gestureTracker)
  {
    m_gestureTracker->update(inputState);
  }
}

void ToolBox::gestureRotate(const InputState& inputState)
{
  assert(enabled());
  if (m_gestureTracker)
  {
    m_gestureTracker->update(inputState);
  }
}

void ToolBox::endGesture(const InputState& inputState)
{
  assert(enabled());
  if (m_gestureTracker)
  {
    m_gestureTracker->end(inputState);
    m_gestureTracker = nullptr;
  }
}

bool ToolBox::cancel(ToolChain& chain)
{
  if (dragging())
  {
    cancelMouseDrag();
    return true;
  }

  if (chain.cancel())
  {
    return true;
  }

  return deactivateCurrentTool();
}

void ToolBox::toggleTool(Tool& tool)
{
  if (tool.active())
  {
    deactivateTool(tool);
  }
  else
  {
    activateTool(tool);
  }
}

bool ToolBox::deactivateCurrentTool()
{
  if (!m_modalToolStack.empty())
  {
    deactivateTool(*m_modalToolStack.back());
    return true;
  }
  return false;
}

void ToolBox::deactivateAllTools()
{
  while (!m_modalToolStack.empty())
  {
    deactivateTool(*m_modalToolStack.back());
  }
}

bool ToolBox::enabled() const
{
  return m_enabled;
}

void ToolBox::enable()
{
  m_enabled = true;
}

void ToolBox::disable()
{
  assert(!dragging());
  m_enabled = false;
}

void ToolBox::setRenderOptions(
  ToolChain& chain, const InputState& inputState, render::RenderContext& renderContext)
{
  chain.setRenderOptions(inputState, renderContext);
  if (m_gestureTracker)
  {
    m_gestureTracker->setRenderOptions(inputState, renderContext);
  }
}

void ToolBox::renderTools(
  ToolChain& chain,
  const InputState& inputState,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch)
{
  chain.render(inputState, renderContext, renderBatch);
  if (m_gestureTracker)
  {
    m_gestureTracker->render(inputState, renderContext, renderBatch);
  }
}

void ToolBox::activateTool(Tool& tool)
{
  for (auto* excludedTool : excludedTools(tool))
  {
    if (excludedTool->active())
    {
      deactivateTool(*excludedTool);
    }
  }

  const auto previouslySuppressedTools = currentlySuppressedTools();
  if (tool.activate())
  {
    const auto toolsToSuppress =
      kdl::set_difference(currentlySuppressedTools(), previouslySuppressedTools);
    for (auto* toolToSuppress : toolsToSuppress)
    {
      toolToSuppress->deactivate();
      toolDeactivatedNotifier(*toolToSuppress);
    }

    m_modalToolStack.push_back(&tool);
    toolActivatedNotifier(tool);
  }
}

void ToolBox::deactivateTool(Tool& tool)
{
  if (dragging())
  {
    cancelMouseDrag();
  }

  const auto previouslySuppressedTools = currentlySuppressedTools();

  tool.deactivate();
  m_modalToolStack.erase(
    std::remove(m_modalToolStack.begin(), m_modalToolStack.end(), &tool),
    m_modalToolStack.end());
  toolDeactivatedNotifier(tool);

  const auto toolsToRelease =
    kdl::set_difference(previouslySuppressedTools, currentlySuppressedTools());
  for (auto* toolToRelease : toolsToRelease)
  {
    toolToRelease->activate();
    toolActivatedNotifier(*toolToRelease);
  }
}

std::vector<Tool*> ToolBox::excludedTools(const Tool& tool) const
{
  auto result = std::vector<Tool*>{};
  for (const auto& exclusiveToolGroup : m_exclusiveToolGroups)
  {
    if (kdl::vec_contains(exclusiveToolGroup, &tool))
    {
      result = kdl::vec_concat(std::move(result), exclusiveToolGroup);
    }
  }
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

std::vector<Tool*> ToolBox::currentlySuppressedTools() const
{
  auto result = std::vector<Tool*>{};
  for (const auto& [primaryTool, suppressedTools] : m_suppressedTools)
  {
    if (primaryTool->active())
    {
      result = kdl::vec_concat(std::move(result), suppressedTools);
    }
  }
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

} // namespace tb::ui
