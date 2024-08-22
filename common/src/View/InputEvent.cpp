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

#include "InputEvent.h"

#include <QApplication>

#include "kdl/reflection_impl.h"

#include <iostream>
#include <string_view>

namespace TrenchBroom
{
namespace View
{
InputEvent::~InputEvent() = default;

bool InputEvent::collateWith(const KeyEvent& /* event */)
{
  return false;
}

bool InputEvent::collateWith(const MouseEvent& /* event */)
{
  return false;
}

bool InputEvent::collateWith(const CancelEvent& /* event */)
{
  return false;
}

KeyEvent::KeyEvent(const Type i_type)
  : type(i_type)
{
}

void KeyEvent::processWith(InputEventProcessor& processor) const
{
  processor.processEvent(*this);
}

kdl_reflect_impl(KeyEvent);

std::ostream& operator<<(std::ostream& lhs, const KeyEvent::Type& rhs)
{
  switch (rhs)
  {
  case KeyEvent::Type::Down:
    lhs << "KeyEvent { type=Down }";
    break;
  case KeyEvent::Type::Up:
    lhs << "KeyEVent { type=Up }";
    break;
  }
  return lhs;
}

MouseEvent::MouseEvent(
  const Type i_type,
  const Button i_button,
  const WheelAxis i_wheelAxis,
  const float i_posX,
  const float i_posY,
  const float i_scrollDistance)
  : type(i_type)
  , button(i_button)
  , wheelAxis(i_wheelAxis)
  , posX(i_posX)
  , posY(i_posY)
  , scrollDistance(i_scrollDistance)
{
}

bool MouseEvent::collateWith(const MouseEvent& event)
{
  if (
    (type == Type::Motion && event.type == Type::Motion)
    || (type == Type::Drag && event.type == Type::Drag))
  {
    posX = event.posX;
    posY = event.posY;
    return true;
  }

  if (type == Type::Scroll && event.type == Type::Scroll)
  {
    if (wheelAxis == event.wheelAxis)
    {
      scrollDistance += event.scrollDistance;
      return true;
    }
  }

  return false;
}

void MouseEvent::processWith(InputEventProcessor& processor) const
{
  processor.processEvent(*this);
}

kdl_reflect_impl(MouseEvent);

std::ostream& operator<<(std::ostream& lhs, const MouseEvent::Type& rhs)
{
  switch (rhs)
  {
  case MouseEvent::Type::Down:
    lhs << "Down";
    break;
  case MouseEvent::Type::Up:
    lhs << "Up";
    break;
  case MouseEvent::Type::Click:
    lhs << "Click";
    break;
  case MouseEvent::Type::DoubleClick:
    lhs << "DoubleClick";
    break;
  case MouseEvent::Type::Motion:
    lhs << "Motion";
    break;
  case MouseEvent::Type::Scroll:
    lhs << "Scroll";
    break;
  case MouseEvent::Type::DragStart:
    lhs << "DragStart";
    break;
  case MouseEvent::Type::Drag:
    lhs << "Drag";
    break;
  case MouseEvent::Type::DragEnd:
    lhs << "DragEnd";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const MouseEvent::Button& rhs)
{
  switch (rhs)
  {
  case MouseEvent::Button::None:
    lhs << "None";
    break;
  case MouseEvent::Button::Left:
    lhs << "Left";
    break;
  case MouseEvent::Button::Middle:
    lhs << "Middle";
    break;
  case MouseEvent::Button::Right:
    lhs << "Right";
    break;
  case MouseEvent::Button::Aux1:
    lhs << "Aux1";
    break;
  case MouseEvent::Button::Aux2:
    lhs << "Aux2";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const MouseEvent::WheelAxis& rhs)
{
  switch (rhs)
  {
  case MouseEvent::WheelAxis::None:
    lhs << "None";
    break;
  case MouseEvent::WheelAxis::Horizontal:
    lhs << "Horizontal";
    break;
  case MouseEvent::WheelAxis::Vertical:
    lhs << "Vertical";
    break;
  }
  return lhs;
}

void CancelEvent::processWith(InputEventProcessor& processor) const
{
  processor.processEvent(*this);
}

kdl_reflect_impl(CancelEvent);

void InputEventQueue::processEvents(InputEventProcessor& processor)
{
  // Swap out the queue before processing it, because if processing an event blocks (e.g.
  // a popup menu), then stale events maybe processed again.

  EventQueue copy;
  using std::swap;
  swap(copy, m_eventQueue);

  for (const auto& event : copy)
  {
    event->processWith(processor);
  }
}

InputEventRecorder::InputEventRecorder()
  : m_dragging(false)
  , m_anyMouseButtonDown(false)
  , m_lastClickX(0.0f)
  , m_lastClickY(0.0f)
  , m_lastClickTime(std::chrono::high_resolution_clock::now())
  , m_nextMouseUpIsRMB(false)
  , m_nextMouseUpIsDblClick(false)
{
}

void InputEventRecorder::recordEvent(const QKeyEvent& qEvent)
{
  m_queue.enqueueEvent(std::make_unique<KeyEvent>(getEventType(qEvent)));
}

void InputEventRecorder::recordEvent(const QMouseEvent& qEvent)
{
  auto type = getEventType(qEvent);
  auto button = getButton(qEvent);
  const auto posX = static_cast<float>(qEvent.localPos().x());
  const auto posY = static_cast<float>(qEvent.localPos().y());

  const auto wheelAxis = MouseEvent::WheelAxis::None;
  const float scrollDistance = 0.0f;

  if (type == MouseEvent::Type::Down)
  {
    // macOS: apply Ctrl+click = right click emulation
    // (Implemented ourselves rather than using Qt's implementation to work around Qt bug,
    // see Main.cpp)
    if (qEvent.modifiers() & Qt::MetaModifier)
    {
      button = MouseEvent::Button::Right;
      m_nextMouseUpIsRMB = true;
    }

    m_lastClickX = posX;
    m_lastClickY = posY;
    m_lastClickTime = std::chrono::high_resolution_clock::now();
    m_anyMouseButtonDown = true;
    m_queue.enqueueEvent(std::make_unique<MouseEvent>(
      MouseEvent::Type::Down, button, wheelAxis, posX, posY, scrollDistance));
  }
  else if (type == MouseEvent::Type::Up)
  {
    // macOS: apply Ctrl+click = right click
    if (m_nextMouseUpIsRMB)
    {
      m_nextMouseUpIsRMB = false;
      if (button == MouseEvent::Button::Left)
      {
        button = MouseEvent::Button::Right;
      }
    }

    if (m_dragging)
    {
      const auto now = std::chrono::high_resolution_clock::now();
      const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastClickTime);
      const auto minDuration = std::chrono::milliseconds(100);
      if (duration < minDuration)
      {
        // This was an accidental drag.
        m_queue.enqueueEvent(std::make_unique<CancelEvent>());
        m_dragging = false;

        // Synthesize a click event
        if (!isDrag(posX, posY))
        {
          m_queue.enqueueEvent(std::make_unique<MouseEvent>(
            MouseEvent::Type::Click,
            button,
            wheelAxis,
            m_lastClickX,
            m_lastClickY,
            scrollDistance));
        }
      }
      else
      {
        m_queue.enqueueEvent(std::make_unique<MouseEvent>(
          MouseEvent::Type::DragEnd, button, wheelAxis, posX, posY, scrollDistance));
        m_dragging = false;
      }
    }
    else if (!m_nextMouseUpIsDblClick)
    {
      // Synthesize a click event
      m_queue.enqueueEvent(std::make_unique<MouseEvent>(
        MouseEvent::Type::Click,
        button,
        wheelAxis,
        m_lastClickX,
        m_lastClickY,
        scrollDistance));
    }
    m_anyMouseButtonDown = false;
    m_nextMouseUpIsDblClick = false;
    m_queue.enqueueEvent(std::make_unique<MouseEvent>(
      MouseEvent::Type::Up, button, wheelAxis, posX, posY, scrollDistance));
  }
  else if (type == MouseEvent::Type::Motion)
  {
    if (!m_dragging && m_anyMouseButtonDown)
    {
      if (isDrag(posX, posY))
      {
        m_queue.enqueueEvent(std::make_unique<MouseEvent>(
          MouseEvent::Type::DragStart,
          button,
          wheelAxis,
          m_lastClickX,
          m_lastClickY,
          scrollDistance));
        m_dragging = true;
      }
    }
    if (m_dragging)
    {
      m_queue.enqueueEvent(std::make_unique<MouseEvent>(
        MouseEvent::Type::Drag, button, wheelAxis, posX, posY, scrollDistance));
    }
    else
    {
      m_queue.enqueueEvent(std::make_unique<MouseEvent>(
        MouseEvent::Type::Motion, button, wheelAxis, posX, posY, scrollDistance));
    }
  }
  else if (type == MouseEvent::Type::DoubleClick)
  {
    m_queue.enqueueEvent(std::make_unique<MouseEvent>(
      MouseEvent::Type::Down, button, wheelAxis, posX, posY, scrollDistance));
    m_queue.enqueueEvent(std::make_unique<MouseEvent>(
      MouseEvent::Type::DoubleClick, button, wheelAxis, posX, posY, scrollDistance));
    m_nextMouseUpIsDblClick = true;
  }
  else
  {
    m_queue.enqueueEvent(
      std::make_unique<MouseEvent>(type, button, wheelAxis, posX, posY, scrollDistance));
  }
}

QPointF InputEventRecorder::scrollLinesForEvent(const QWheelEvent& qtEvent)
{
  // TODO: support pixel scrolling via qtEvent.pixelDelta()?
  const int linesPerStep = QApplication::wheelScrollLines();
  const QPointF angleDelta = QPointF(qtEvent.angleDelta()); // in eighths-of-degrees
  constexpr float EighthsOfDegreesPerStep =
    120.0f; // see: https://doc.qt.io/qt-5/qwheelevent.html#angleDelta

  const QPointF lines = (angleDelta / EighthsOfDegreesPerStep) * linesPerStep;
  return lines;
}

void InputEventRecorder::recordEvent(const QWheelEvent& qtEvent)
{
  // These are the mouse X and Y position, not the wheel delta, in points relative to top
  // left of widget.
  const auto posX = static_cast<float>(qtEvent.x());
  const auto posY = static_cast<float>(qtEvent.y());

  // Number of "lines" to scroll
  QPointF scrollDistance = scrollLinesForEvent(qtEvent);

  // Qt switches scroll axis when alt is pressed, but unfortunately, not consistently on
  // all OS'es and doesn't give any way of knowing. see:
  // https://bugreports.qt.io/browse/QTBUG-30948
  const bool swapXY =
#ifdef __APPLE__
    false;
#else
    qtEvent.modifiers().testFlag(Qt::AltModifier);
#endif
  if (swapXY)
  {
    scrollDistance = QPointF(scrollDistance.y(), scrollDistance.x());
  }

  if (scrollDistance.x() != 0.0f)
  {
    m_queue.enqueueEvent(std::make_unique<MouseEvent>(
      MouseEvent::Type::Scroll,
      MouseEvent::Button::None,
      MouseEvent::WheelAxis::Horizontal,
      posX,
      posY,
      static_cast<float>(scrollDistance.x())));
  }
  if (scrollDistance.y() != 0.0f)
  {
    m_queue.enqueueEvent(std::make_unique<MouseEvent>(
      MouseEvent::Type::Scroll,
      MouseEvent::Button::None,
      MouseEvent::WheelAxis::Vertical,
      posX,
      posY,
      static_cast<float>(scrollDistance.y())));
  }
}

void InputEventRecorder::processEvents(InputEventProcessor& processor)
{
  m_queue.processEvents(processor);
}

bool InputEventRecorder::isDrag(const float posX, const float posY) const
{
  static const auto MinDragDistance = 2.0f;
  return std::abs(posX - m_lastClickX) > MinDragDistance
         || std::abs(posY - m_lastClickY) > MinDragDistance;
}

KeyEvent::Type InputEventRecorder::getEventType(const QKeyEvent& qEvent)
{
  const auto qEventType = qEvent.type();
  if (qEventType == QEvent::KeyPress)
  {
    return KeyEvent::Type::Down;
  }
  else if (qEventType == QEvent::KeyRelease)
  {
    return KeyEvent::Type::Up;
  }
  else
  {
    throw std::runtime_error("Unexpected qEvent type");
  }
}

MouseEvent::Type InputEventRecorder::getEventType(const QMouseEvent& qEvent)
{
  if (qEvent.type() == QEvent::MouseButtonPress)
  {
    return MouseEvent::Type::Down;
  }
  else if (qEvent.type() == QEvent::MouseButtonRelease)
  {
    return MouseEvent::Type::Up;
  }
  else if (qEvent.type() == QEvent::MouseButtonDblClick)
  {
    return MouseEvent::Type::DoubleClick;
  }
  else if (qEvent.type() == QEvent::MouseMove)
  {
    return MouseEvent::Type::Motion;
  }
  else
  {
    throw std::runtime_error("Unexpected qEvent type");
  }
}

MouseEvent::Button InputEventRecorder::getButton(const QMouseEvent& qEvent)
{
  if (qEvent.button() == Qt::LeftButton)
  {
    return MouseEvent::Button::Left;
  }
  else if (qEvent.button() == Qt::MiddleButton)
  {
    return MouseEvent::Button::Middle;
  }
  else if (qEvent.button() == Qt::RightButton)
  {
    return MouseEvent::Button::Right;
  }
  else if (qEvent.button() == Qt::XButton1)
  {
    return MouseEvent::Button::Aux1;
  }
  else if (qEvent.button() == Qt::XButton2)
  {
    return MouseEvent::Button::Aux2;
  }
  else
  {
    return MouseEvent::Button::None;
  }
}

InputEventProcessor::~InputEventProcessor() = default;
} // namespace View
} // namespace TrenchBroom
