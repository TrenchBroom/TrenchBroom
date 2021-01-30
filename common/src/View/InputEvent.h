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

#include <QKeyEvent>

#include <chrono>
#include <iosfwd>
#include <memory>
#include <vector>

// Undefine this symbol since it interferes somehow with our enums.
#undef None

namespace TrenchBroom {
    namespace View {
        class CancelEvent;
        class InputEventProcessor;
        class KeyEvent;
        class MouseEvent;

        /**
         * Superclass for all input events. Provides protocols for event collation and processing.
         */
        class InputEvent {
        public:
            virtual ~InputEvent();

            /**
             * Collate this event with the given key event.
             *
             * @param event the event to collate with
             * @return true if this event was collated with the given event and false otherwise
             */
            virtual bool collateWith(const KeyEvent& event);

            /**
             * Collate this event with the given mouse event.
             *
             * @param event the event to collate with
             * @return true if this event was collated with the given event and false otherwise
             */
            virtual bool collateWith(const MouseEvent& event);

            /**
             * Collate this event with the given cancellation event.
             *
             * @param event the event to collate with
             * @return true if this event was collated with the given event and false otherwise
             */
            virtual bool collateWith(const CancelEvent& event);

            /**
             * Process this event using the given event processor.
             *
             * @param processor the event processor
             */
            virtual void processWith(InputEventProcessor& processor) const = 0;
        };

        /**
         * A keyboard event. Supports only key up and down events.
         */
        class KeyEvent : public InputEvent {
        public:
            enum class Type {
                /**
                 * A key was pressed.
                 */
                Down,
                /**
                 * A key was released.
                 */
                Up
            };
        public:
            Type type;
        public:
            /**
             * Creates a new key event with the given type.
             *
             * @param type the type of the key event to create
             */
            explicit KeyEvent(Type type);
        public:
            /**
             * Process this key event with the given event processor.
             *
             * @param processor the event processor
             */
            void processWith(InputEventProcessor& processor) const override;
            
            /**
             * Indicates whether the given two key events are equal.
             */
            friend bool operator==(const KeyEvent& lhs, const KeyEvent& rhs);
            
            /**
             * Prints a textual representation of the given event to the given output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const KeyEvent& event);
        };

        /**
         * A mouse event. Supports several event types such as button down and button up, up to five mouse buttons,
         * and mouse wheel events.
         */
        class MouseEvent : public InputEvent {
        public:
            enum class Type {
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
                 * The mouse wheel was scrolled.
                 */
                Scroll,
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
            enum class Button {
                None, Left, Middle, Right, Aux1, Aux2
            };
            enum class WheelAxis {
                None, Vertical, Horizontal
            };
        public:
            Type type;
            Button button;
            WheelAxis wheelAxis;
            int posX;
            int posY;
            float scrollDistance;
        public:
            /**
             * Creates a new mouse event with the given parameters.
             *
             * @param type the event type
             * @param button the button that triggered the event, if any
             * @param wheelAxis the wheel axies that was scrolled, if any
             * @param posX the current X position of the mouse pointer
             * @param posY the current Y position of the mouse pointer
             * @param scrollDistance the distance by which the mouse wheel was scrolled, in lines
             */
            MouseEvent(Type type, Button button, WheelAxis wheelAxis, int posX, int posY, float scrollDistance);
        public:
            /**
             * Collates this mouse event with the given mouse event. Only successive Motion, Drag and Scroll events are
             * collated.
             *
             * @param event the mouse event to collate with
             * @return true if this event was collated with the given mouse event and false otherwise
             */
            bool collateWith(const MouseEvent& event) override;

            /**
             * Process this mouse event using the given event processor.
             *
             * @param processor the event processor
             */
            void processWith(InputEventProcessor& processor) const override;
            
            /**
             * Indicates whether the given two mouse events are equal.
             */
            friend bool operator==(const MouseEvent& lhs, const MouseEvent& rhs);

            /**
             * Prints a textual representation of the given event to the given output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const MouseEvent& event);
        };

        /**
         * Event to signal that a mouse drag was cancelled by the windowing system, e.g. when the window lost focus.
         */
        class CancelEvent : public InputEvent {
        public:
            /**
             * Process this event using the given event processor.
             *
             * @param processor the event processor
             */
            void processWith(InputEventProcessor& processor) const override;
            
            /**
             * Indicates whether the given two cancel events are equal.
             */
            friend bool operator==(const CancelEvent&, const CancelEvent&);
            
            /**
             * Prints a textual representation of the given event to the given output stream.
             */
            friend std::ostream& operator<<(std::ostream& out, const CancelEvent& event);
        };

        /**
         * Collects input events in a queue and processes them when instructed.
         */
        class InputEventQueue {
        private:
            using EventQueue = std::vector<std::unique_ptr<InputEvent>>;
            EventQueue m_eventQueue;
        public:
            /**
             * Enqueues an event into this event queue. The given event will be collated with the last event in this
             * queue, if any. If the event was collated, the given event is discarded since its information will be
             * recorded in the last event.
             *
             * @tparam T the type of the event to enqueue
             * @param event the event to enqueue
             */
            template <typename T>
            void enqueueEvent(std::unique_ptr<T> event) {
                if (m_eventQueue.empty() || !m_eventQueue.back()->collateWith(*event)) {
                    m_eventQueue.push_back(std::move(event));
                }
            }
            /**
             * Process the events in this queue with the given event processor. The events are forwarded to the processor
             * in the order in which they were enqeued.
             *
             * When all events have been processed, the event queue is cleared.
             *
             * @param processor the event processor
             */
            void processEvents(InputEventProcessor& processor);

        };

        /**
         * Handles and records input events. May synthesize new events such as mouse click and drag events depending on
         * the current state of this handler and the information of the events being recorded.
         *
         * Mouse clicks are generated only if a mouse down and a mouse up event were recorded within 100ms and the
         * mouse pointer has not travelled by more than 1 pixel in each direction.
         *
         * Drag events are synthesized once a motion event occurs while a mouse button is pressed and the total distance
         * from the position at which the mouse button was pressed is more than 1 pixel in any direction.
         */
        class InputEventRecorder {
        private:
            InputEventQueue m_queue;

            /**
             * Indicates whether or not a mouse drag is taking place.
             */
            bool m_dragging;
            /**
             Indicates that we received a mouse down event, cleared on mouse up.
             */
            bool m_anyMouseButtonDown;
            /**
             * The X position of the last mouse down event.
             */
            int m_lastClickX;
            /**
             * The Y position of the last mouse down event.
             */
            int m_lastClickY;
            /**
             * The time at which the last mouse down event was recorded.
             */
            std::chrono::time_point<std::chrono::high_resolution_clock> m_lastClickTime;
            /**
             * Used in implementing the macOS behaviour where Ctrl+Click is RMB.
             */
            bool m_nextMouseUpIsRMB;
            /**
             * Used to suppress a click event for the mouse up event that follows a double click.
             */
            bool m_nextMouseUpIsDblClick;
        public:
            /**
             * Creates a new event handler.
             */
            InputEventRecorder();

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
             * Processes all recorded events using the given event processor.
             *
             * @param processor the event processor
             */
            void processEvents(InputEventProcessor& processor);
        private:
            /**
             * Determines if the given mouse position should start a drag when compared to the last click position.
             *
             * @param posX the X position
             * @param posY the Y position
             * @return true if the given position should start a drag and false otherwise
             */
            bool isDrag(int posX, int posY) const;

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
        class InputEventProcessor {
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
             * Process a cancellation event.
             *
             * @param event the event to process
             */
            virtual void processEvent(const CancelEvent& event) = 0;
        };
    }
}

