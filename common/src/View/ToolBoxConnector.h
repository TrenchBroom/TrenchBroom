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

#pragma once

#include "Macros.h"
#include "View/InputEvent.h"
#include "View/InputState.h"

#include <memory>
#include <string>

namespace TrenchBroom
{
namespace Model
{
class PickResult;
}

namespace Renderer
{
class Camera;
class RenderBatch;
class RenderContext;
} // namespace Renderer

namespace View
{
class PickRequest;
class ToolController;
class ToolBox;
class ToolChain;

class ToolBoxConnector : public InputEventProcessor
{
private:
  ToolBox* m_toolBox;
  ToolChain* m_toolChain;

  InputState m_inputState;

  float m_lastMouseX;
  float m_lastMouseY;
  bool m_ignoreNextDrag;

public:
  ToolBoxConnector();
  ~ToolBoxConnector() override;

public:
  const vm::ray3& pickRay() const;
  const Model::PickResult& pickResult() const;

  void updatePickResult();

protected:
  void setToolBox(ToolBox& toolBox);
  void addTool(std::unique_ptr<ToolController> tool);

public: // drag and drop
  bool dragEnter(float x, float y, const std::string& text);
  bool dragMove(float x, float y, const std::string& text);
  void dragLeave();
  bool dragDrop(float x, float y, const std::string& text);

public: // cancel
  bool cancel();

protected: // rendering
  void setRenderOptions(Renderer::RenderContext& renderContext);
  void renderTools(
    Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);

private:
  ModifierKeyState modifierKeys();
  bool setModifierKeys();

protected:
  bool clearModifierKeys();
  void updateModifierKeys();

private:
  void showPopupMenu();

public: // implement InputEventProcessor interface
  void processEvent(const KeyEvent& event) override;
  void processEvent(const MouseEvent& event) override;
  void processEvent(const CancelEvent& event) override;

private:
  void processMouseButtonDown(const MouseEvent& event);
  void processMouseButtonUp(const MouseEvent& event);
  void processMouseClick(const MouseEvent& event);
  void processMouseDoubleClick(const MouseEvent& event);
  void processMouseMotion(const MouseEvent& event);
  void processScroll(const MouseEvent& event);
  void processDragStart(const MouseEvent& event);
  void processDrag(const MouseEvent& event);
  void processDragEnd(const MouseEvent& event);

  MouseButtonState mouseButton(const MouseEvent& event);
  void mouseMoved(float x, float y);

public:
  bool cancelDrag();

private:
  virtual PickRequest doGetPickRequest(float x, float y) const = 0;
  virtual Model::PickResult doPick(const vm::ray3& pickRay) const = 0;
  virtual void doShowPopupMenu();

  deleteCopyAndMove(ToolBoxConnector);
};
} // namespace View
} // namespace TrenchBroom
