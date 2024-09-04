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

#include "ToolBoxConnector.h"

#include <QGuiApplication>

#include "Ensure.h"
#include "Macros.h"
#include "View/PickRequest.h"
#include "View/ToolBox.h"
#include "View/ToolChain.h"
#include "View/ToolController.h"

#include <string>

namespace TrenchBroom::View
{
ToolBoxConnector::ToolBoxConnector()
  : m_toolChain{std::make_unique<ToolChain>()}
{
}

ToolBoxConnector::~ToolBoxConnector() = default;

const vm::ray3& ToolBoxConnector::pickRay() const
{
  return m_inputState.pickRay();
}

const Model::PickResult& ToolBoxConnector::pickResult() const
{
  return m_inputState.pickResult();
}

void ToolBoxConnector::updatePickResult()
{
  ensure(m_toolBox, "toolBox is set");

  m_inputState.setPickRequest(
    doGetPickRequest(m_inputState.mouseX(), m_inputState.mouseY()));
  auto pickResult = doPick(m_inputState.pickRay());
  m_toolBox->pick(*m_toolChain, m_inputState, pickResult);
  m_inputState.setPickResult(std::move(pickResult));
}

void ToolBoxConnector::setToolBox(ToolBox& toolBox)
{
  assert(!m_toolBox);
  m_toolBox = &toolBox;
}

void ToolBoxConnector::addTool(std::unique_ptr<ToolController> tool)
{
  m_toolChain->append(std::move(tool));
}

bool ToolBoxConnector::dragEnter(const float x, const float y, const std::string& text)
{
  ensure(m_toolBox, "toolBox is set");

  mouseMoved(x, y);
  updatePickResult();

  return m_toolBox->dragEnter(*m_toolChain, m_inputState, text);
}

bool ToolBoxConnector::dragMove(const float x, const float y, const std::string& text)
{
  ensure(m_toolBox, "toolBox is set");

  mouseMoved(x, y);
  updatePickResult();

  return m_toolBox->dragMove(*m_toolChain, m_inputState, text);
}

void ToolBoxConnector::dragLeave()
{
  ensure(m_toolBox, "toolBox is set");

  m_toolBox->dragLeave(*m_toolChain, m_inputState);
}

bool ToolBoxConnector::dragDrop(
  const float /* x */, const float /* y */, const std::string& text)
{
  ensure(m_toolBox, "toolBox is set");

  updatePickResult();

  return m_toolBox->dragDrop(*m_toolChain, m_inputState, text);
}

bool ToolBoxConnector::cancel()
{
  ensure(m_toolBox, "toolBox is set");
  m_inputState.setAnyToolDragging(m_toolBox->dragging());
  return m_toolBox->cancel(*m_toolChain);
}

void ToolBoxConnector::setRenderOptions(Renderer::RenderContext& renderContext)
{
  ensure(m_toolBox, "toolBox is set");
  m_inputState.setAnyToolDragging(m_toolBox->dragging());
  m_toolBox->setRenderOptions(*m_toolChain, m_inputState, renderContext);
}

void ToolBoxConnector::renderTools(
  Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch)
{
  ensure(m_toolBox, "toolBox is set");
  m_inputState.setAnyToolDragging(m_toolBox->dragging());
  m_toolBox->renderTools(*m_toolChain, m_inputState, renderContext, renderBatch);
}

ModifierKeyState ToolBoxConnector::modifierKeys()
{
  // QGuiApplication::queryKeyboardModifiers() is needed instead of
  // QGuiApplication::keyboardModifiers(), because when a modifier (e.g. Shift) is
  // pressed, QGuiApplication::keyboardModifiers() isn't updated soon enough.
  const auto mouseState = QGuiApplication::queryKeyboardModifiers();

  auto state = ModifierKeys::None;
  if (mouseState & Qt::ControlModifier)
  {
    state |= ModifierKeys::MKCtrlCmd;
  }
  if (mouseState & Qt::ShiftModifier)
  {
    state |= ModifierKeys::Shift;
  }
  if (mouseState & Qt::AltModifier)
  {
    state |= ModifierKeys::MKAlt;
  }
  return state;
}

/**
 * Updates the TB modifier key state from the Qt state.
 * Returns whether the TB modifier key state changed from its previously cached value.
 */
bool ToolBoxConnector::setModifierKeys()
{
  m_inputState.setAnyToolDragging(m_toolBox->dragging());

  const auto keys = modifierKeys();
  if (keys != m_inputState.modifierKeys())
  {
    m_inputState.setModifierKeys(keys);
    return true;
  }
  return false;
}

bool ToolBoxConnector::clearModifierKeys()
{
  m_inputState.setAnyToolDragging(m_toolBox->dragging());

  if (m_inputState.modifierKeys() != ModifierKeys::None)
  {
    m_inputState.setModifierKeys(ModifierKeys::None);
    updatePickResult();
    m_toolBox->modifierKeyChange(*m_toolChain, m_inputState);
    return true;
  }
  return false;
}

void ToolBoxConnector::updateModifierKeys()
{
  if (setModifierKeys())
  {
    updatePickResult();
    m_toolBox->modifierKeyChange(*m_toolChain, m_inputState);
  }
}

void ToolBoxConnector::showPopupMenu()
{
  doShowPopupMenu();
  updateModifierKeys();
}

void ToolBoxConnector::processEvent(const KeyEvent&)
{
  updateModifierKeys();
}

void ToolBoxConnector::processEvent(const MouseEvent& event)
{
  switch (event.type)
  {
  case MouseEvent::Type::Down:
    processMouseButtonDown(event);
    break;
  case MouseEvent::Type::Up:
    processMouseButtonUp(event);
    break;
  case MouseEvent::Type::Click:
    processMouseClick(event);
    break;
  case MouseEvent::Type::DoubleClick:
    processMouseDoubleClick(event);
    break;
  case MouseEvent::Type::Motion:
    processMouseMotion(event);
    break;
  case MouseEvent::Type::DragStart:
    processDragStart(event);
    break;
  case MouseEvent::Type::Drag:
    processDrag(event);
    break;
  case MouseEvent::Type::DragEnd:
    processDragEnd(event);
    break;
    switchDefault();
  }
  m_inputState.setAnyToolDragging(m_toolBox->dragging());
}

namespace
{

auto getScrollSource(const ScrollEvent& event)
{
  switch (event.source)
  {
  case ScrollEvent::Source::Mouse:
    return ScrollSource::Mouse;
  case ScrollEvent::Source::Trackpad:
    return ScrollSource::Trackpad;
    switchDefault();
  }
}

} // namespace

void ToolBoxConnector::processEvent(const ScrollEvent& event)
{
  updateModifierKeys();
  const auto scrollSource = getScrollSource(event);
  if (event.axis == ScrollEvent::Axis::Horizontal)
  {
    m_inputState.scroll(scrollSource, event.distance, 0.0f);
  }
  else if (event.axis == ScrollEvent::Axis::Vertical)
  {
    m_inputState.scroll(scrollSource, 0.0f, event.distance);
  }
  m_toolBox->mouseScroll(*m_toolChain, m_inputState);

  updatePickResult();
}

void ToolBoxConnector::processEvent(const GestureEvent& event)
{
  switch (event.type)
  {
  case GestureEvent::Type::Start:
    processGestureStart(event);
    break;
  case GestureEvent::Type::End:
    processGestureEnd(event);
    break;
  case GestureEvent::Type::Pan:
    processGesturePan(event);
    break;
  case GestureEvent::Type::Zoom:
    processGestureZoom(event);
    break;
  case GestureEvent::Type::Rotate:
    processGestureRotate(event);
    break;
    switchDefault();
  }
}

void ToolBoxConnector::processEvent(const CancelEvent&)
{
  cancelDrag();
}

void ToolBoxConnector::processMouseButtonDown(const MouseEvent& event)
{
  updateModifierKeys();
  m_inputState.mouseDown(mouseButton(event));
  m_toolBox->mouseDown(*m_toolChain, m_inputState);

  updatePickResult();
  m_ignoreNextDrag = false;
}

void ToolBoxConnector::processMouseButtonUp(const MouseEvent& event)
{
  updateModifierKeys();
  m_toolBox->mouseUp(*m_toolChain, m_inputState);
  m_inputState.mouseUp(mouseButton(event));

  updatePickResult();
  m_ignoreNextDrag = false;
}

void ToolBoxConnector::processMouseClick(const MouseEvent& event)
{
  const auto handled = m_toolBox->mouseClick(*m_toolChain, m_inputState);
  if (event.button == MouseEvent::Button::Right && !handled)
  {
    // We miss mouse events when a popup menu is already open, so we must make sure that
    // the input state is up to date.
    mouseMoved(event.posX, event.posY);
    updatePickResult();
    showPopupMenu();
  }
}

void ToolBoxConnector::processMouseDoubleClick(const MouseEvent& event)
{
  updateModifierKeys();
  m_inputState.mouseDown(mouseButton(event));
  m_toolBox->mouseDoubleClick(*m_toolChain, m_inputState);
  m_inputState.mouseUp(mouseButton(event));
  updatePickResult();
}

void ToolBoxConnector::processMouseMotion(const MouseEvent& event)
{
  mouseMoved(event.posX, event.posY);
  updatePickResult();
  m_toolBox->mouseMove(*m_toolChain, m_inputState);
}

void ToolBoxConnector::processDragStart(const MouseEvent& event)
{
  // Move the mouse back to where it was when the user clicked (see
  // InputEventRecorder::recordEvent) and re-pick, since we're currently 2px off from
  // there, and the user will expects to drag exactly what was under the pixel they
  // clicked. See: https://github.com/TrenchBroom/TrenchBroom/issues/2808
  mouseMoved(event.posX, event.posY);
  updatePickResult();

  m_toolBox->startMouseDrag(*m_toolChain, m_inputState);
}

void ToolBoxConnector::processDrag(const MouseEvent& event)
{
  mouseMoved(event.posX, event.posY);
  updatePickResult();
  if (m_toolBox->dragging() && !m_toolBox->mouseDrag(m_inputState))
  {
    processDragEnd(event);
  }
}

void ToolBoxConnector::processDragEnd(const MouseEvent&)
{
  if (m_toolBox->dragging())
  {
    m_toolBox->endMouseDrag(m_inputState);
  }
}

MouseButtonState ToolBoxConnector::mouseButton(const MouseEvent& event)
{
  switch (event.button)
  {
  case MouseEvent::Button::Left:
    return MouseButtons::Left;
  case MouseEvent::Button::Middle:
    return MouseButtons::Middle;
  case MouseEvent::Button::Right:
    return MouseButtons::Right;
  case MouseEvent::Button::Aux1:
  case MouseEvent::Button::Aux2:
  case MouseEvent::Button::None:
    return MouseButtons::None;
    switchDefault();
  }
}

void ToolBoxConnector::mouseMoved(const float x, const float y)
{
  const auto dx = x - m_lastMousePos.x();
  const auto dy = y - m_lastMousePos.y();
  m_inputState.mouseMove(x, y, dx, dy);
  m_lastMousePos = {x, y};
}

void ToolBoxConnector::processGestureStart(const GestureEvent&)
{
  m_inputState.startGesture();
  m_toolBox->startGesture(*m_toolChain, m_inputState);
}

void ToolBoxConnector::processGestureEnd(const GestureEvent&)
{
  m_toolBox->endGesture(m_inputState);
  m_inputState.endGesture();

  m_lastGesturePanPos = std::nullopt;
}

void ToolBoxConnector::processGesturePan(const GestureEvent& event)
{
  const auto pos = vm::vec2f{event.posX, event.posY};
  const auto delta = pos - m_lastGesturePanPos.value_or(pos);

  m_inputState.gesturePan(pos.x(), pos.y(), delta.x(), delta.y());
  m_toolBox->gesturePan(m_inputState);

  m_lastGesturePanPos = pos;
}

void ToolBoxConnector::processGestureZoom(const GestureEvent& event)
{
  m_inputState.gestureZoom(event.value);
  m_toolBox->gestureZoom(m_inputState);
}

void ToolBoxConnector::processGestureRotate(const GestureEvent& event)
{
  m_inputState.gestureRotate(event.value);
  m_toolBox->gestureRotate(m_inputState);
}

bool ToolBoxConnector::cancelDrag()
{
  if (m_toolBox->dragging())
  {
    m_toolBox->cancelMouseDrag();
    return true;
  }
  return false;
}

void ToolBoxConnector::doShowPopupMenu() {}

} // namespace TrenchBroom::View
