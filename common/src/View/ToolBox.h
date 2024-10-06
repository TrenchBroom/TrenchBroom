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

#pragma once

#include <QObject>

#include "Notifier.h"
#include "NotifierConnection.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class QWindow;
class QFocusEvent;
class QMouseEvent;

namespace tb::mdl
{
class PickResult;
}

namespace tb::Renderer
{
class RenderBatch;
class RenderContext;
} // namespace tb::Renderer

namespace tb::View
{

class GestureTracker;
class DropTracker;
class InputState;
class Tool;
class ToolController;
class ToolChain;

class ToolBox : public QObject
{
  Q_OBJECT
private:
  std::unique_ptr<GestureTracker> m_gestureTracker;
  std::unique_ptr<DropTracker> m_dropTracker;
  Tool* m_modalTool = nullptr;

  std::map<Tool*, std::vector<Tool*>> m_suppressedTools;

  bool m_enabled = true;

  NotifierConnection m_notifierConnection;

public:
  Notifier<Tool&> toolActivatedNotifier;
  Notifier<Tool&> toolDeactivatedNotifier;
  Notifier<Tool&> refreshViewsNotifier;
  Notifier<Tool&> toolHandleSelectionChangedNotifier;

public:
  ToolBox();
  ~ToolBox() override;

protected:
  void addTool(Tool& tool);

public: // picking
  void pick(ToolChain& chain, const InputState& inputState, mdl::PickResult& pickResult);

public: // event handling
  bool dragEnter(ToolChain& chain, const InputState& inputState, const std::string& text);
  bool dragMove(ToolChain& chain, const InputState& inputState, const std::string& text);
  void dragLeave(ToolChain& chain, const InputState& inputState);
  bool dragDrop(ToolChain& chain, const InputState& inputState, const std::string& text);

  void modifierKeyChange(ToolChain& chain, const InputState& inputState);
  void mouseDown(ToolChain& chain, const InputState& inputState) const;
  void mouseUp(ToolChain& chain, const InputState& inputState) const;
  bool mouseClick(ToolChain& chain, const InputState& inputState) const;
  void mouseDoubleClick(ToolChain& chain, const InputState& inputState) const;
  void mouseMove(ToolChain& chain, const InputState& inputState) const;

  bool dragging() const;
  void startMouseDrag(ToolChain& chain, const InputState& inputState);
  bool mouseDrag(const InputState& inputState);
  void endMouseDrag(const InputState& inputState);
  void cancelMouseDrag();

  void mouseScroll(ToolChain& chain, const InputState& inputState);

  void startGesture(ToolChain& chain, const InputState& inputState);
  void gesturePan(const InputState& inputState);
  void gestureZoom(const InputState& inputState);
  void gestureRotate(const InputState& inputState);
  void endGesture(const InputState& inputState);

  bool cancel(ToolChain& chain);

public: // tool management
  /**
   * Suppress a tool when another becomes active. The suppressed tool becomes temporarily
   * deactivated.
   *
   * @param suppressedTool the tool that becomes supressed while the other is active
   * @param primaryTool the tool that controls when the suppressed tool is deactivated
   */
  void suppressWhileActive(Tool& suppressedTool, Tool& primaryTool);

  bool anyToolActive() const;
  Tool* activeTool();
  void toggleTool(Tool& tool);
  void deactivateAllTools();

  bool enabled() const;
  void enable();
  void disable();

public: // rendering
  void setRenderOptions(
    ToolChain& chain,
    const InputState& inputState,
    Renderer::RenderContext& renderContext);
  void renderTools(
    ToolChain& chain,
    const InputState& inputState,
    Renderer::RenderContext& renderContext,
    Renderer::RenderBatch& renderBatch);

private:
  void activateTool(Tool& tool);
  void deactivateTool(Tool& tool);
};

} // namespace tb::View
