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

#include <QKeyEvent>

#include "kdl/reflection_decl.h"

#include <chrono>
#include <variant>
#include <vector>

// Undefine this symbol since it interferes somehow with our enums.
#undef None

namespace tb::View
{

class InputEventProcessor;

/**
 * A keyboard event. Supports only key up and down events.
 */
struct KeyEvent
{
  enum class Type
  {
    /**
     * A key was pressed.
     */
    Down,
    /**
     * A key was released.
     */
    Up
  };

  Type type;

  /**
   * Process this key event with the given event processor.
   *
   * @param processor the event processor
   */
  void processWith(InputEventProcessor& processor) const;

  kdl_reflect_decl(KeyEvent, type);
};

std::ostream& operator<<(std::ostream& lhs, const KeyEvent::Type& rhs);

/**
 * A mouse event. Supports several event types such as button down and button up, up to
 * five mouse buttons, and mouse wheel events.
 */
struct MouseEvent
{
  enum class Type
  {
    /**
     * A button was pressed.
     */
    Down,
    /**
     * A button was released.
     */
    Up,
    /**
     * A button was clicked.
     */
    Click,
    /**
     * A button was double clicked.
     */
    DoubleClick,
    /**
     * The mouse was moved.
     */
    Motion,
    /**
     * A mouse drag was started.
     */
    DragStart,
    /**
     * The mouse was moved during a mouse drag.
     */
    Drag,
    /**
     * The mouse drag ended.
     */
    DragEnd
  };

  enum class Button
  {
    None,
    Left,
    Middle,
    Right,
    Aux1,
    Aux2
  };

  Type type;
  Button button;

  /** Cursor position in Points, relative to top left of widget. */
  float posX;
  float posY;

  /**
   * Collates this mouse event with the given mouse event. Only successive Motion, Drag
   * and Scroll events are collated.
   *
   * @param event the mouse event to collate with
   * @return true if this event was collated with the given mouse event and false
   * otherwise
   */
  bool collateWith(const MouseEvent& event);

  /**
   * Process this mouse event using the given event processor.
   *
   * @param processor the event processor
   */
  void processWith(InputEventProcessor& processor) const;

  kdl_reflect_decl(MouseEvent, type, button, posX, posY);
};

std::ostream& operator<<(std::ostream& lhs, const MouseEvent::Type& rhs);
std::ostream& operator<<(std::ostream& lhs, const MouseEvent::Button& rhs);

struct ScrollEvent
{
  enum class Source
  {
    Mouse,
    Trackpad,
  };

  enum class Axis
  {
    Vertical,
    Horizontal,
  };

  Source source;
  Axis axis;
  float distance;

  /**
   * Collates this scroll event with the given scroll event. Only successive Pan,
   * Zoom and Rotate events are collated.
   *
   * @param event the scroll event to collate with
   * @return true if this event was collated with the given scroll event and false
   * otherwise
   */
  bool collateWith(const ScrollEvent& event);

  /**
   * Process this scroll event using the given event processor.
   *
   * @param processor the event processor
   */
  void processWith(InputEventProcessor& processor) const;

  kdl_reflect_decl(ScrollEvent, source, axis, distance);
};

std::ostream& operator<<(std::ostream& lhs, const ScrollEvent::Source& rhs);
std::ostream& operator<<(std::ostream& lhs, const ScrollEvent::Axis& rhs);

/**
 * A gesture event. Supports several gesture types such as pan, zoom, and rotate.
 */
struct GestureEvent
{
  enum class Type
  {
    /**
     * A gesture was started.
     */
    Start,
    /**
     * A gesture has ended.
     */
    End,
    /**
     * A panning gesture update.
     */
    Pan,
    /**
     * A zoom gesture update.
     */
    Zoom,
    /**
     * A rotate gesture update.
     */
    Rotate,
  };

  Type type;

  /** Cursor position in Points, relative to top left of widget. */
  float posX;
  float posY;
  float value;

  /**
   * Collates this gesture event with the given gesture event. Only successive Pan,
   * Zoom and Rotate events are collated.
   *
   * @param event the gesture event to collate with
   * @return true if this event was collated with the given gesture event and false
   * otherwise
   */
  bool collateWith(const GestureEvent& event);

  /**
   * Process this gesture event using the given event processor.
   *
   * @param processor the event processor
   */
  void processWith(InputEventProcessor& processor) const;

  kdl_reflect_decl(GestureEvent, type, posX, posY, value);
};

std::ostream& operator<<(std::ostream& lhs, const GestureEvent::Type& rhs);

/**
 * Event to signal that a mouse drag was cancelled by the windowing system, e.g. when the
 * window lost focus.
 */
struct CancelEvent
{
  /**
   * Process this event using the given event processor.
   *
   * @param processor the event processor
   */
  void processWith(InputEventProcessor& processor) const;

  kdl_reflect_decl_empty(CancelEvent);
};

using InputEvent =
  std::variant<KeyEvent, MouseEvent, GestureEvent, ScrollEvent, CancelEvent>;

/**
 * Collects input events in a queue and processes them when instructed.
 */
class InputEventQueue
{
private:
  std::vector<InputEvent> m_eventQueue;

public:
  /**
   * Enqueues an event into this event queue. The given event will be collated with the
   * last event in this queue, if any. If the event was collated, the given event is
   * discarded since its information will be recorded in the last event.
   *
   * @param event the event to enqueue
   */
  void enqueueEvent(InputEvent event);

  /**
   * Process the events in this queue with the given event processor. The events are
   * forwarded to the processor in the order in which they were enqeued.
   *
   * When all events have been processed, the event queue is cleared.
   *
   * @param processor the event processor
   */
  void processEvents(InputEventProcessor& processor);
};

/**
 * Handles and records input events. May synthesize new events such as mouse click and
 * drag events depending on the current state of this handler and the information of the
 * events being recorded.
 *
 * Mouse clicks are generated only if a mouse down and a mouse up event were recorded
 * within 100ms and the mouse pointer has not travelled by more than 1 pixel in each
 * direction.
 *
 * Drag events are synthesized once a motion event occurs while a mouse button is pressed
 * and the total distance from the position at which the mouse button was pressed is more
 * than 1 pixel in any direction.
 */
class InputEventRecorder
{
private:
  InputEventQueue m_queue;

  /**
   * Indicates whether or not a mouse drag is taking place.
   */
  bool m_dragging = false;
  /**
   Indicates that we received a mouse down event, cleared on mouse up.
   */
  bool m_anyMouseButtonDown = false;
  /**
   * The X position of the last mouse down event.
   */
  float m_lastClickX = 0.0f;
  /**
   * The Y position of the last mouse down event.
   */
  float m_lastClickY = 0.0f;
  /**
   * The time at which the last mouse down event was recorded.
   */
  std::chrono::time_point<std::chrono::high_resolution_clock> m_lastClickTime =
    std::chrono::high_resolution_clock::now();
  /**
   * Used in implementing the macOS behaviour where Ctrl+Click is RMB.
   */
  bool m_nextMouseUpIsRMB = false;
  /**
   * Used to suppress a click event for the mouse up event that follows a double click.
   */
  bool m_nextMouseUpIsDblClick = false;
  /**
   * The number of active gestures. Used to send start / end events when the first gesture
   * starts and the last gesture ends.
   */
  size_t m_activeGestures = 0;

public:
  /**
   * Records the given key event.
   *
   * @param event the event to record
   */
  void recordEvent(const QKeyEvent& event);

  /**
   * Records the given mouse event.
   *
   * @param event the event to record
   */
  void recordEvent(const QMouseEvent& event);

  static QPointF scrollLinesForEvent(const QWheelEvent& event);

  void recordEvent(const QWheelEvent& event);

  /**
   * Records the given native gesture event.
   *
   * @param event the event to record
   */
  void recordEvent(const QNativeGestureEvent& event);

  /**
   * Processes all recorded events using the given event processor.
   *
   * @param processor the event processor
   */
  void processEvents(InputEventProcessor& processor);

private:
  /**
   * Determines if the given mouse position should start a drag when compared to the last
   * click position.
   *
   * @param posX the X position
   * @param posY the Y position
   * @return true if the given position should start a drag and false otherwise
   */
  bool isDrag(float posX, float posY) const;

  /**
   * Decodes the event type of the given key event.
   *
   * @param event the event to decode
   * @return the event type
   */
  static KeyEvent::Type getEventType(const QKeyEvent& event);

  /**
   * Decodes the event type of the given mouse event.
   *
   * @param event the event to decode
   * @return the event type
   */
  static MouseEvent::Type getEventType(const QMouseEvent& event);

  /**
   * Decodes the button of the given mouse event, if any.
   *
   * @param event the event to decode
   * @return the mouse button
   */
  static MouseEvent::Button getButton(const QMouseEvent& event);
};

/**
 * Processes input events.
 */
class InputEventProcessor
{
public:
  virtual ~InputEventProcessor();

public:
  /**
   * Process a key event.
   *
   * @param event the event to process
   */
  virtual void processEvent(const KeyEvent& event) = 0;

  /**
   * Process a mouse event.
   *
   * @param event the event to process
   */
  virtual void processEvent(const MouseEvent& event) = 0;

  /**
   * Process a gesture event.
   *
   * @param event the event to process
   */
  virtual void processEvent(const GestureEvent& event) = 0;

  /**
   * Process a scroll event.
   *
   * @param event the event to process
   */
  virtual void processEvent(const ScrollEvent& event) = 0;

  /**
   * Process a cancellation event.
   *
   * @param event the event to process
   */
  virtual void processEvent(const CancelEvent& event) = 0;
};
} // namespace tb::View
