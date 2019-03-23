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

namespace TrenchBroom {
    namespace View {
        InputEvent::~InputEvent() = default;

        bool InputEvent::collateWith(const KeyEvent& event) {
            return false;
        }

        bool InputEvent::collateWith(const MouseEvent& event) {
            return false;
        }

        bool InputEvent::collateWith(const CancelEvent& event) {
            return false;
        }

        KeyEvent::KeyEvent(const Type i_type) :
        type(i_type) {}

        void KeyEvent::processWith(InputEventProcessor& processor) const {
            processor.processEvent(*this);
        }

        MouseEvent::MouseEvent(const Type i_type, const Button i_button, const WheelAxis i_wheelAxis, const int i_posX, const int i_posY, const float i_scrollDistance):
        type(i_type),
        button(i_button),
        wheelAxis(i_wheelAxis),
        posX(i_posX),
        posY(i_posY),
        scrollDistance(i_scrollDistance) {}

        bool MouseEvent::collateWith(const MouseEvent& event) {
            if ((type == Type::Motion && event.type == Type::Motion) ||
                (type == Type::Drag && event.type == Type::Drag)) {
                posX = event.posX;
                posY = event.posY;
                return true;
            }

            if (type == Type::Scroll && event.type == Type::Scroll) {
                if (wheelAxis == event.wheelAxis) {
                    scrollDistance += event.scrollDistance;
                    return true;
                }
            }

            return false;
        }

        void MouseEvent::processWith(InputEventProcessor& processor) const {
            processor.processEvent(*this);
        }

        void CancelEvent::processWith(InputEventProcessor& processor) const {
            processor.processEvent(*this);
        }

        void InputEventQueue::processEvents(InputEventProcessor& processor) {
            // Swap out the queue before processing it, because if processing an event blocks (e.g. a popup menu), then
            // stale events maybe processed again.

            EventQueue copy;
            using std::swap;
            swap(copy, m_eventQueue);

            for (const auto& event : copy) {
                event->processWith(processor);
            }
        }

        InputEventRecorder::InputEventRecorder() :
        m_dragging(false),
        m_anyMouseButtonDown(false),
        m_lastClickX(0),
        m_lastClickY(0),
        m_lastClickTime(std::chrono::high_resolution_clock::now()) {}


        void InputEventRecorder::recordEvent(const wxKeyEvent& wxEvent) {
            m_queue.enqueueEvent(std::make_unique<KeyEvent>(getEventType(wxEvent)));
        }

        void InputEventRecorder::recordEvent(const wxMouseEvent& wxEvent) {
                  auto type = getEventType(wxEvent);
            const auto button = getButton(wxEvent);
            const auto wheelAxis = getWheelAxis(wxEvent);
            const auto posX = wxEvent.GetX();
            const auto posY = wxEvent.GetY();
            const auto scrollDistance = getScrollDistance(wxEvent);

            if (type == MouseEvent::Type::Down) {
                m_lastClickX = posX;
                m_lastClickY = posY;
                m_lastClickTime = std::chrono::high_resolution_clock::now();
                m_anyMouseButtonDown = true;
                m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::Down, button, wheelAxis, posX, posY, scrollDistance));
            } else if (type == MouseEvent::Type::Up) {
                if (m_dragging) {
                    const auto now = std::chrono::high_resolution_clock::now();
                    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastClickTime);
                    const auto minDuration = std::chrono::milliseconds(100);
                    if (duration < minDuration) {
                        // This was an accidental drag.
                        m_queue.enqueueEvent(std::make_unique<CancelEvent>());
                        m_dragging = false;

                        // Synthesize a click event if the drag distance was not exceeded.
                        if (!isDrag(posX, posY)) {
                            m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::Click, button, wheelAxis, m_lastClickX, m_lastClickY, scrollDistance));
                        }
                    } else {
                        m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::DragEnd, button, wheelAxis, posX, posY, scrollDistance));
                        m_dragging = false;
                    }
                } else {
                    // Synthesize a click event if the drag distance was not exceeded.
                    if (!isDrag(posX, posY)) {
                        m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::Click, button, wheelAxis, m_lastClickX, m_lastClickY, scrollDistance));
                    }
                }
                m_anyMouseButtonDown = false;
                m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::Up, button, wheelAxis, posX, posY, scrollDistance));
            } else if (type == MouseEvent::Type::Motion) {
                if (!m_dragging && m_anyMouseButtonDown) {
                    if (isDrag(posX, posY)) {
                        m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::DragStart, button, wheelAxis, m_lastClickX, m_lastClickY, scrollDistance));
                        m_dragging = true;
                    }
                }
                if (m_dragging) {
                    m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::Drag, button, wheelAxis, posX, posY, scrollDistance));
                } else {
                    m_queue.enqueueEvent(std::make_unique<MouseEvent>(MouseEvent::Type::Motion, button, wheelAxis, posX, posY, scrollDistance));
                }
            } else {
                m_queue.enqueueEvent(std::make_unique<MouseEvent>(type, button, wheelAxis, posX, posY, scrollDistance));
            }
        }

        void InputEventRecorder::recordEvent(const wxMouseCaptureLostEvent& wxEvent) {
            m_queue.enqueueEvent(std::make_unique<CancelEvent>());
            m_anyMouseButtonDown = false;
        }

        void InputEventRecorder::processEvents(InputEventProcessor& processor) {
            m_queue.processEvents(processor);
        }

        bool InputEventRecorder::isDrag(int posX, int posY) const {
            static const auto MinDragDistance = 2;
            return std::abs(posX - m_lastClickX) > MinDragDistance || std::abs(posY - m_lastClickY) > MinDragDistance;
        }

        KeyEvent::Type InputEventRecorder::getEventType(const wxKeyEvent& wxEvent) {
            const auto wxType = wxEvent.GetEventType();
            if (wxType == wxEVT_KEY_DOWN) {
                return KeyEvent::Type::Down;
            } else if (wxType == wxEVT_KEY_UP) {
                return KeyEvent::Type::Up;
            } else {
                throw std::runtime_error("Unexpected wxEvent type");
            }
        }

        MouseEvent::Type InputEventRecorder::getEventType(const wxMouseEvent& wxEvent) {
            if (wxEvent.ButtonDown(wxMOUSE_BTN_ANY)) {
                return MouseEvent::Type::Down;
            } else if (wxEvent.ButtonUp(wxMOUSE_BTN_ANY)) {
                return MouseEvent::Type::Up;
            } else if (wxEvent.ButtonDClick(wxMOUSE_BTN_ANY)) {
                return MouseEvent::Type::DoubleClick;
            } else if (wxEvent.GetEventType() == wxEVT_MOTION) {
                return MouseEvent::Type::Motion;
            } else if (wxEvent.GetEventType() == wxEVT_MOUSEWHEEL) {
                return MouseEvent::Type::Scroll;
            } else {
                throw std::runtime_error("Unexpected wxEvent type");
            }
        }

        MouseEvent::Button InputEventRecorder::getButton(const wxMouseEvent& wxEvent) {
            if (wxEvent.Button(wxMOUSE_BTN_LEFT)) {
                return MouseEvent::Button::Left;
            } else if (wxEvent.Button(wxMOUSE_BTN_MIDDLE)) {
                return MouseEvent::Button::Middle;
            } else if (wxEvent.Button(wxMOUSE_BTN_RIGHT)) {
                return MouseEvent::Button::Right;
            } else if (wxEvent.Button(wxMOUSE_BTN_AUX1)) {
                return MouseEvent::Button::Aux1;
            } else if (wxEvent.Button(wxMOUSE_BTN_AUX2)) {
                return MouseEvent::Button::Aux2;
            } else {
                return MouseEvent::Button::None;
            }
        }

        MouseEvent::WheelAxis InputEventRecorder::getWheelAxis(const wxMouseEvent& wxEvent) {
            if (wxEvent.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL) {
                return MouseEvent::WheelAxis::Vertical;
            } else if (wxEvent.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL) {
                return MouseEvent::WheelAxis::Horizontal;
            } else {
                return MouseEvent::WheelAxis::None;
            }
        }

        float InputEventRecorder::getScrollDistance(const wxMouseEvent& wxEvent) {
            return static_cast<float>(wxEvent.GetWheelRotation()) / wxEvent.GetWheelDelta() * wxEvent.GetLinesPerAction();
        }

        InputEventProcessor::~InputEventProcessor() = default;
    }
}
