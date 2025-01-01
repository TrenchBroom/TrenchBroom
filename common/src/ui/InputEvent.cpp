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

#include "InputEvent.h"

#include <QApplication>

#include "Ensure.h"

#include "kdl/overload.h"
#include "kdl/reflection_impl.h"

#include <iostream>
#include <string_view>

namespace tb::ui
{

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
    lhs << "KeyEvent { type=Up }";
    break;
  }
  return lhs;
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

bool ScrollEvent::collateWith(const ScrollEvent& event)
{
  if (source == event.source && axis == event.axis)
  {
    distance += event.distance;
    return true;
  }

  return false;
}

void ScrollEvent::processWith(InputEventProcessor& processor) const
{
  processor.processEvent(*this);
}

kdl_reflect_impl(ScrollEvent);

std::ostream& operator<<(std::ostream& lhs, const ScrollEvent::Source& rhs)
{
  switch (rhs)
  {
  case ScrollEvent::Source::Mouse:
    lhs << "Mouse";
    break;
  case ScrollEvent::Source::Trackpad:
    lhs << "Trackpad";
    break;
  }
  return lhs;
}

std::ostream& operator<<(std::ostream& lhs, const ScrollEvent::Axis& rhs)
{
  switch (rhs)
  {
  case ScrollEvent::Axis::Horizontal:
    lhs << "Horizontal";
    break;
  case ScrollEvent::Axis::Vertical:
    lhs << "Vertical";
    break;
  }
  return lhs;
}

bool GestureEvent::collateWith(const GestureEvent& event)
{
  if (
    (type == Type::Pan && event.type == Type::Pan)
    || (type == Type::Zoom && event.type == Type::Zoom)
    || (type == Type::Rotate && event.type == Type::Rotate))
  {
    posX = event.posX;
    posY = event.posY;
    value = event.value;
    return true;
  }

  return false;
}

kdl_reflect_impl(GestureEvent);

std::ostream& operator<<(std::ostream& lhs, const GestureEvent::Type& rhs)
{
  switch (rhs)
  {
  case GestureEvent::Type::Start:
    lhs << "Begin";
    break;
  case GestureEvent::Type::End:
    lhs << "End";
    break;
  case GestureEvent::Type::Pan:
    lhs << "Pan";
    break;
  case GestureEvent::Type::Zoom:
    lhs << "Zoom";
    break;
  case GestureEvent::Type::Rotate:
    lhs << "Rotate";
    break;
  }
  return lhs;
}

void CancelEvent::processWith(InputEventProcessor& processor) const
{
  processor.processEvent(*this);
}

void GestureEvent::processWith(InputEventProcessor& processor) const
{
  processor.processEvent(*this);
}

kdl_reflect_impl(CancelEvent);

namespace
{
bool collateEvents(InputEvent& lhs, const InputEvent& rhs)
{
  return std::visit(
    kdl::overload(
      [](MouseEvent& lhsMouseEvent, const MouseEvent& rhsMouseEvent) {
        return lhsMouseEvent.collateWith(rhsMouseEvent);
      },
      [](ScrollEvent& lhsScrollEvent, const ScrollEvent& rhsScrollEvent) {
        return lhsScrollEvent.collateWith(rhsScrollEvent);
      },
      [](GestureEvent& lhsGestureEvent, const GestureEvent& rhsGestureEvent) {
        return lhsGestureEvent.collateWith(rhsGestureEvent);
      },
      [](auto&, const auto&) { return false; }),
    lhs,
    rhs);
}
} // namespace

void InputEventQueue::enqueueEvent(InputEvent event)
{
  if (m_eventQueue.empty() || !collateEvents(m_eventQueue.back(), event))
  {
    m_eventQueue.push_back(std::move(event));
  }
}

void InputEventQueue::processEvents(InputEventProcessor& processor)
{
  // Swap out the queue before processing it, because if processing an event blocks (e.g.
  // a popup menu), then stale events maybe processed again.

  auto eventQueue = std::exchange(m_eventQueue, std::vector<InputEvent>{});
  for (const auto& event : eventQueue)
  {
    std::visit([&processor](const auto& x) { x.processWith(processor); }, event);
  }
}

void InputEventRecorder::recordEvent(const QKeyEvent& qEvent)
{
  m_queue.enqueueEvent(KeyEvent{getEventType(qEvent)});
}

void InputEventRecorder::recordEvent(const QMouseEvent& qEvent)
{
  auto type = getEventType(qEvent);
  auto button = getButton(qEvent);
  const auto posX = static_cast<float>(qEvent.position().x());
  const auto posY = static_cast<float>(qEvent.position().y());

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
    m_queue.enqueueEvent(MouseEvent{MouseEvent::Type::Down, button, posX, posY});
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
        m_queue.enqueueEvent(CancelEvent{});
        m_dragging = false;

        // Synthesize a click event
        if (!isDrag(posX, posY))
        {
          m_queue.enqueueEvent(
            MouseEvent{MouseEvent::Type::Click, button, m_lastClickX, m_lastClickY});
        }
      }
      else
      {
        m_queue.enqueueEvent(MouseEvent{MouseEvent::Type::DragEnd, button, posX, posY});
        m_dragging = false;
      }
    }
    else if (!m_nextMouseUpIsDblClick)
    {
      // Synthesize a click event
      m_queue.enqueueEvent(
        MouseEvent{MouseEvent::Type::Click, button, m_lastClickX, m_lastClickY});
    }
    m_anyMouseButtonDown = false;
    m_nextMouseUpIsDblClick = false;
    m_queue.enqueueEvent(MouseEvent{MouseEvent::Type::Up, button, posX, posY});
  }
  else if (type == MouseEvent::Type::Motion)
  {
    if (!m_dragging && m_anyMouseButtonDown)
    {
      if (isDrag(posX, posY))
      {
        m_queue.enqueueEvent(
          MouseEvent{MouseEvent::Type::DragStart, button, m_lastClickX, m_lastClickY});
        m_dragging = true;
      }
    }
    if (m_dragging)
    {
      m_queue.enqueueEvent(MouseEvent{MouseEvent::Type::Drag, button, posX, posY});
    }
    else
    {
      m_queue.enqueueEvent(MouseEvent{MouseEvent::Type::Motion, button, posX, posY});
    }
  }
  else if (type == MouseEvent::Type::DoubleClick)
  {
    m_queue.enqueueEvent(MouseEvent{MouseEvent::Type::Down, button, posX, posY});
    m_queue.enqueueEvent(MouseEvent{MouseEvent::Type::DoubleClick, button, posX, posY});
    m_nextMouseUpIsDblClick = true;
  }
  else
  {
    m_queue.enqueueEvent(MouseEvent{type, button, posX, posY});
  }
}

QPointF InputEventRecorder::scrollLinesForEvent(const QWheelEvent& qtEvent)
{
  // see: https://doc.qt.io/qt-5/qwheelevent.html#angleDelta
  constexpr auto EighthsOfDegreesPerStep = 120.0f;

  // TODO: support pixel scrolling via qtEvent.pixelDelta()?
  const auto linesPerStep = QApplication::wheelScrollLines();
  const auto angleDelta = QPointF{qtEvent.angleDelta()}; // in eighths-of-degrees

  return (angleDelta / EighthsOfDegreesPerStep) * linesPerStep;
}

void InputEventRecorder::recordEvent(const QWheelEvent& qtEvent)
{
  const auto source = qtEvent.source() == Qt::MouseEventNotSynthesized
                        ? ScrollEvent::Source::Mouse
                        : ScrollEvent::Source::Trackpad;

  // Number of "lines" to scroll
  auto scrollDistance = scrollLinesForEvent(qtEvent);

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
    scrollDistance = QPointF{scrollDistance.y(), scrollDistance.x()};
  }

  if (scrollDistance.x() != 0.0f)
  {
    m_queue.enqueueEvent(
      ScrollEvent{source, ScrollEvent::Axis::Horizontal, float(scrollDistance.x())});
  }
  if (scrollDistance.y() != 0.0f)
  {
    m_queue.enqueueEvent(
      ScrollEvent{source, ScrollEvent::Axis::Vertical, float(scrollDistance.y())});
  }
}

namespace
{
std::optional<GestureEvent::Type> getGestureType(
  const Qt::NativeGestureType qtGestureType)
{
  switch (qtGestureType)
  {
  case Qt::BeginNativeGesture:
    return GestureEvent::Type::Start;
  case Qt::EndNativeGesture:
    return GestureEvent::Type::End;
  case Qt::PanNativeGesture:
    return GestureEvent::Type::Pan;
  case Qt::ZoomNativeGesture:
    return GestureEvent::Type::Zoom;
  case Qt::RotateNativeGesture:
    return GestureEvent::Type::Rotate;
  default:
    return std::nullopt;
  }
}
} // namespace

void InputEventRecorder::recordEvent(const QNativeGestureEvent& qEvent)
{
  if (const auto type = getGestureType(qEvent.gestureType()))
  {
    if (*type == GestureEvent::Type::Start)
    {
      ++m_activeGestures;
      if (m_activeGestures > 1)
      {
        return;
      }
    }
    else if (*type == GestureEvent::Type::End)
    {
      ensure(m_activeGestures > 0, "a gesture is active");

      --m_activeGestures;
      if (m_activeGestures > 0)
      {
        return;
      }
    }

    const auto posX = float(qEvent.position().x());
    const auto posY = float(qEvent.position().y());
    const auto value = float(qEvent.value());
    m_queue.enqueueEvent(GestureEvent{*type, posX, posY, value});
  }
}

void InputEventRecorder::processEvents(InputEventProcessor& processor)
{
  m_queue.processEvents(processor);
}

bool InputEventRecorder::isDrag(const float posX, const float posY) const
{
  constexpr auto MinDragDistance = 2.0f;

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
  if (qEventType == QEvent::KeyRelease)
  {
    return KeyEvent::Type::Up;
  }
  throw std::runtime_error("Unexpected qEvent type");
}

MouseEvent::Type InputEventRecorder::getEventType(const QMouseEvent& qEvent)
{
  if (qEvent.type() == QEvent::MouseButtonPress)
  {
    return MouseEvent::Type::Down;
  }
  if (qEvent.type() == QEvent::MouseButtonRelease)
  {
    return MouseEvent::Type::Up;
  }
  if (qEvent.type() == QEvent::MouseButtonDblClick)
  {
    return MouseEvent::Type::DoubleClick;
  }
  if (qEvent.type() == QEvent::MouseMove)
  {
    return MouseEvent::Type::Motion;
  }
  throw std::runtime_error("Unexpected qEvent type");
}

MouseEvent::Button InputEventRecorder::getButton(const QMouseEvent& qEvent)
{
  if (qEvent.button() == Qt::LeftButton)
  {
    return MouseEvent::Button::Left;
  }
  if (qEvent.button() == Qt::MiddleButton)
  {
    return MouseEvent::Button::Middle;
  }
  if (qEvent.button() == Qt::RightButton)
  {
    return MouseEvent::Button::Right;
  }
  if (qEvent.button() == Qt::XButton1)
  {
    return MouseEvent::Button::Aux1;
  }
  if (qEvent.button() == Qt::XButton2)
  {
    return MouseEvent::Button::Aux2;
  }
  return MouseEvent::Button::None;
}

InputEventProcessor::~InputEventProcessor() = default;

} // namespace tb::ui
