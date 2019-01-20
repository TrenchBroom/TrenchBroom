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

#ifndef InputEvent_h
#define InputEvent_h

#include <wx/event.h>

#include <chrono>
#include <list>
#include <memory>

namespace TrenchBroom {
    namespace View {
        class KeyEvent;
        class MouseEvent;
        class InputEventProcessor;

        class InputEvent {
        public:
            virtual ~InputEvent();
            virtual bool collateWith(const KeyEvent& event);
            virtual bool collateWith(const MouseEvent& event);
            virtual void processWith(InputEventProcessor& processor) const = 0;
        };

        class KeyEvent : public InputEvent {
        public:
            enum class Type {
                Down, Up
            };
        public:
            Type type;
        public:
            explicit KeyEvent(Type type);
        public:
            void processWith(InputEventProcessor& processor) const override;
        };

        class MouseEvent : public InputEvent {
        public:
            enum class Type {
                Down, Up, Click, DoubleClick, Motion, Scroll, DragStart, Drag, DragEnd
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
            MouseEvent(Type type, Button button, WheelAxis wheelAxis, int posX, int posY, float scrollDistance);
        public:
            bool collateWith(const MouseEvent& event) override;
            void processWith(InputEventProcessor& processor) const override;
        };

        class CancelEvent : public InputEvent {
        public:
            void processWith(InputEventProcessor& processor) const override;
        };

        class InputEventQueue {
        private:
            using EventQueue = std::list<std::unique_ptr<InputEvent>>;
            EventQueue m_eventQueue;

            bool m_dragging;
            int m_lastClickX;
            int m_lastClickY;
            std::chrono::time_point<std::chrono::high_resolution_clock> m_lastClickTime;
        public:
            InputEventQueue();
            void recordEvent(const wxKeyEvent& event);
            void recordEvent(const wxMouseEvent& event);
            void recordEvent(const wxMouseCaptureLostEvent& event);
            void processEvents(InputEventProcessor& processor);
        private:
            template <typename T>
            void enqueueEvent(std::unique_ptr<T> event) {
                if (m_eventQueue.empty() || !m_eventQueue.back()->collateWith(*event)) {
                    m_eventQueue.push_back(std::move(event));
                }
            }

            static KeyEvent::Type getEventType(const wxKeyEvent& wxEvent);
            static MouseEvent::Type getEventType(const wxMouseEvent& wxEvent);
            static MouseEvent::Button getButton(const wxMouseEvent& wxEvent);
            static MouseEvent::WheelAxis getWheelAxis(const wxMouseEvent& wxEvent);
            static float getScrollDistance(const wxMouseEvent& wxEvent);
        };

        class InputEventProcessor {
        public:
            virtual ~InputEventProcessor();
        public:
            virtual void processEvent(const KeyEvent& event) = 0;
            virtual void processEvent(const MouseEvent& event) = 0;
            virtual void processEvent(const CancelEvent& event) = 0;
        };
    }
}

#endif /* InputEvent_hpp */
