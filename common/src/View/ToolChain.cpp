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

#include "ToolChain.h"

#include "Ensure.h"
#include "View/DragTracker.h"
#include "View/DropTracker.h"
#include "View/ToolController.h"

#include <cassert>
#include <string>

namespace TrenchBroom::View
{
ToolChain::ToolChain() = default;

ToolChain::~ToolChain() = default;

void ToolChain::append(std::unique_ptr<ToolController> tool)
{
  assert(checkInvariant());
  if (chainEndsHere())
  {
    assert(m_suffix == nullptr);
    m_tool = std::move(tool);
    m_suffix = std::make_unique<ToolChain>();
  }
  else
  {
    ensure(m_suffix != nullptr, "suffix is null");
    m_suffix->append(std::move(tool));
  }
  assert(checkInvariant());
}

void ToolChain::pick(const InputState& inputState, Model::PickResult& pickResult)
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->pick(inputState, pickResult);
    }
    m_suffix->pick(inputState, pickResult);
  }
}

void ToolChain::modifierKeyChange(const InputState& inputState)
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->modifierKeyChange(inputState);
    }
    m_suffix->modifierKeyChange(inputState);
  }
}

void ToolChain::mouseDown(const InputState& inputState)
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->mouseDown(inputState);
    }
    m_suffix->mouseDown(inputState);
  }
}

void ToolChain::mouseUp(const InputState& inputState)
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->mouseUp(inputState);
    }
    m_suffix->mouseUp(inputState);
  }
}

bool ToolChain::mouseClick(const InputState& inputState)
{
  assert(checkInvariant());
  if (chainEndsHere())
  {
    return false;
  }
  if (m_tool->toolActive() && m_tool->mouseClick(inputState))
  {
    return true;
  }
  return m_suffix->mouseClick(inputState);
}

bool ToolChain::mouseDoubleClick(const InputState& inputState)
{
  assert(checkInvariant());
  if (chainEndsHere())
  {
    return false;
  }
  if (m_tool->toolActive() && m_tool->mouseDoubleClick(inputState))
  {
    return true;
  }
  return m_suffix->mouseDoubleClick(inputState);
}

void ToolChain::mouseScroll(const InputState& inputState)
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->mouseScroll(inputState);
    }
    m_suffix->mouseScroll(inputState);
  }
}

void ToolChain::mouseMove(const InputState& inputState)
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->mouseMove(inputState);
    }
    m_suffix->mouseMove(inputState);
  }
}

std::unique_ptr<DragTracker> ToolChain::startMouseDrag(const InputState& inputState)
{
  assert(checkInvariant());
  if (chainEndsHere())
  {
    return nullptr;
  }
  if (m_tool->toolActive())
  {
    if (auto dragTracker = m_tool->acceptMouseDrag(inputState))
    {
      return dragTracker;
    }
  }
  return m_suffix->startMouseDrag(inputState);
}

bool ToolChain::shouldAcceptDrop(
  const InputState& inputState, const std::string& payload) const
{
  assert(checkInvariant());
  if (chainEndsHere())
  {
    return false;
  }
  return (m_tool->toolActive() && m_tool->shouldAcceptDrop(inputState, payload))
         || m_suffix->shouldAcceptDrop(inputState, payload);
}

std::unique_ptr<DropTracker> ToolChain::dragEnter(
  const InputState& inputState, const std::string& payload)
{
  assert(checkInvariant());
  if (chainEndsHere())
  {
    return nullptr;
  }
  if (m_tool->toolActive())
  {
    if (auto dropTracker = m_tool->acceptDrop(inputState, payload))
    {
      return dropTracker;
    }
  }
  return m_suffix->dragEnter(inputState, payload);
}

void ToolChain::setRenderOptions(
  const InputState& inputState, Renderer::RenderContext& renderContext) const
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->setRenderOptions(inputState, renderContext);
    }
    m_suffix->setRenderOptions(inputState, renderContext);
  }
}

void ToolChain::render(
  const InputState& inputState,
  Renderer::RenderContext& renderContext,
  Renderer::RenderBatch& renderBatch)
{
  assert(checkInvariant());
  if (!chainEndsHere())
  {
    if (m_tool->toolActive())
    {
      m_tool->render(inputState, renderContext, renderBatch);
    }
    m_suffix->render(inputState, renderContext, renderBatch);
  }
}

bool ToolChain::cancel()
{
  assert(checkInvariant());
  if (chainEndsHere())
  {
    return false;
  }
  if (m_tool->cancel())
  {
    return true;
  }
  return m_suffix->cancel();
}

bool ToolChain::checkInvariant() const
{
  return (m_tool == nullptr) == (m_suffix == nullptr);
}

bool ToolChain::chainEndsHere() const
{
  return m_tool == nullptr;
}
} // namespace TrenchBroom::View
