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

#include "Ensure.h"
#include "Macros.h"
#include "View/PickRequest.h"
#include "View/ToolBox.h"
#include "View/ToolChain.h"

#include <string>

#include <QGuiApplication>

namespace TrenchBroom {
    namespace View {
        ToolBoxConnector::ToolBoxConnector() :
        m_toolBox(nullptr),
        m_toolChain(new ToolChain()),
        m_lastMouseX(0.0f),
        m_lastMouseY(0.0f),
        m_ignoreNextDrag(false) {}

        ToolBoxConnector::~ToolBoxConnector() {
            delete m_toolChain;
        }

        const vm::ray3& ToolBoxConnector::pickRay() const {
            return m_inputState.pickRay();
        }

        const Model::PickResult& ToolBoxConnector::pickResult() const {
            return m_inputState.pickResult();
        }

        void ToolBoxConnector::updatePickResult() {
            ensure(m_toolBox != nullptr, "toolBox is null");

            m_inputState.setPickRequest(doGetPickRequest(m_inputState.mouseX(),  m_inputState.mouseY()));
            Model::PickResult pickResult = doPick(m_inputState.pickRay());
            m_toolBox->pick(m_toolChain, m_inputState, pickResult);
            m_inputState.setPickResult(std::move(pickResult));
        }

        void ToolBoxConnector::setToolBox(ToolBox& toolBox) {
            assert(m_toolBox == nullptr);
            m_toolBox = &toolBox;
        }

        void ToolBoxConnector::addTool(ToolController* tool) {
            m_toolChain->append(tool);
        }

        bool ToolBoxConnector::dragEnter(const float x, const float y, const std::string& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            mouseMoved(x, y);
            updatePickResult();

            return m_toolBox->dragEnter(m_toolChain, m_inputState, text);
        }

        bool ToolBoxConnector::dragMove(const float x, const float y, const std::string& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            mouseMoved(x, y);
            updatePickResult();

            return m_toolBox->dragMove(m_toolChain, m_inputState, text);
        }

        void ToolBoxConnector::dragLeave() {
            ensure(m_toolBox != nullptr, "toolBox is null");

            m_toolBox->dragLeave(m_toolChain, m_inputState);
        }

        bool ToolBoxConnector::dragDrop(const float /* x */, const float /* y */, const std::string& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            updatePickResult();

            return m_toolBox->dragDrop(m_toolChain, m_inputState, text);
        }

        bool ToolBoxConnector::cancel() {
            ensure(m_toolBox != nullptr, "toolBox is null");
            const bool result = m_toolBox->cancel(m_toolChain);
            m_inputState.setAnyToolDragging(false);
            return result;
        }

        void ToolBoxConnector::setRenderOptions(Renderer::RenderContext& renderContext) {
            ensure(m_toolBox != nullptr, "toolBox is null");
            m_toolBox->setRenderOptions(m_toolChain, m_inputState, renderContext);
        }

        void ToolBoxConnector::renderTools(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            ensure(m_toolBox != nullptr, "toolBox is null");
            m_toolBox->renderTools(m_toolChain, m_inputState, renderContext, renderBatch);
        }

        ModifierKeyState ToolBoxConnector::modifierKeys() {
            // QGuiApplication::queryKeyboardModifiers() is needed instead of QGuiApplication::keyboardModifiers(),
            // because when a modifier (e.g. Shift) is pressed, QGuiApplication::keyboardModifiers() isn't updated
            // soon enough.
            const Qt::KeyboardModifiers mouseState = QGuiApplication::queryKeyboardModifiers();

            ModifierKeyState state = ModifierKeys::MKNone;
            if (mouseState & Qt::ControlModifier) {
                state |= ModifierKeys::MKCtrlCmd;
            }
            if (mouseState & Qt::ShiftModifier) {
                state |= ModifierKeys::MKShift;
            }
            if (mouseState & Qt::AltModifier) {
                state |= ModifierKeys::MKAlt;
            }
            return state;
        }

        /**
         * Updates the TB modifier key state from the Qt state.
         * Returns whether the TB modifier key state changed from its previously cached value.
         */
        bool ToolBoxConnector::setModifierKeys() {
            const ModifierKeyState keys = modifierKeys();
            if (keys != m_inputState.modifierKeys()) {
                m_inputState.setModifierKeys(keys);
                return true;
            } else {
                return false;
            }
        }

        bool ToolBoxConnector::clearModifierKeys() {
            if (m_inputState.modifierKeys() != ModifierKeys::MKNone) {
                m_inputState.setModifierKeys(ModifierKeys::MKNone);
                updatePickResult();
                m_toolBox->modifierKeyChange(m_toolChain, m_inputState);
                return true;
            } else {
                return false;
            }
        }

        void ToolBoxConnector::updateModifierKeys() {
            if (setModifierKeys()) {
                updatePickResult();
                m_toolBox->modifierKeyChange(m_toolChain, m_inputState);
            }
        }

        void ToolBoxConnector::showPopupMenu() {
            doShowPopupMenu();
            updateModifierKeys();
        }

        void ToolBoxConnector::processEvent(const KeyEvent&) {
            updateModifierKeys();
        }

        void ToolBoxConnector::processEvent(const MouseEvent& event) {
            switch (event.type) {
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
                case MouseEvent::Type::Scroll:
                    processScroll(event);
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
                switchDefault()
            }

        }

        void ToolBoxConnector::processEvent(const CancelEvent&) {
            cancelDrag();
        }

        void ToolBoxConnector::processMouseButtonDown(const MouseEvent& event) {
            updateModifierKeys();
            m_inputState.mouseDown(mouseButton(event));
            m_toolBox->mouseDown(m_toolChain, m_inputState);

            updatePickResult();
            m_ignoreNextDrag = false;
        }

        void ToolBoxConnector::processMouseButtonUp(const MouseEvent& event) {
            updateModifierKeys();
            m_toolBox->mouseUp(m_toolChain, m_inputState);
            m_inputState.mouseUp(mouseButton(event));

            updatePickResult();
            m_ignoreNextDrag = false;
        }

        void ToolBoxConnector::processMouseClick(const MouseEvent& event) {
            const auto handled = m_toolBox->mouseClick(m_toolChain, m_inputState);
            if (event.button == MouseEvent::Button::Right && !handled) {
                // We miss mouse events when a popup menu is already open, so we must make sure that the input
                // state is up to date.
                mouseMoved(event.posX, event.posY);
                updatePickResult();
                showPopupMenu();
            }
        }

        void ToolBoxConnector::processMouseDoubleClick(const MouseEvent& event) {
            updateModifierKeys();
            m_inputState.mouseDown(mouseButton(event));
            m_toolBox->mouseDoubleClick(m_toolChain, m_inputState);
            m_inputState.mouseUp(mouseButton(event));
            updatePickResult();
        }

        void ToolBoxConnector::processMouseMotion(const MouseEvent& event) {
            mouseMoved(event.posX, event.posY);
            updatePickResult();
            m_toolBox->mouseMove(m_toolChain, m_inputState);
        }

        void ToolBoxConnector::processScroll(const MouseEvent& event) {
            updateModifierKeys();
            if (event.wheelAxis == MouseEvent::WheelAxis::Horizontal) {
                m_inputState.scroll(event.scrollDistance, 0.0f);
            } else if (event.wheelAxis == MouseEvent::WheelAxis::Vertical) {
                m_inputState.scroll(0.0f, event.scrollDistance);
            }
            m_toolBox->mouseScroll(m_toolChain, m_inputState);

            updatePickResult();
        }

        void ToolBoxConnector::processDragStart(const MouseEvent& event) {
            // Move the mouse back to where it was when the user clicked (see InputEventRecorder::recordEvent)
            // and re-pick, since we're currently 2px off from there, and the user will expects to drag exactly
            // what was under the pixel they clicked.
            // See: https://github.com/TrenchBroom/TrenchBroom/issues/2808
            mouseMoved(event.posX, event.posY);
            updatePickResult();

            if (m_toolBox->startMouseDrag(m_toolChain, m_inputState)) {
                m_inputState.setAnyToolDragging(true);
            }
        }

        void ToolBoxConnector::processDrag(const MouseEvent& event) {
            mouseMoved(event.posX, event.posY);
            updatePickResult();
            if (m_toolBox->dragging()) {
                if (!m_toolBox->mouseDrag(m_inputState)) {
                        processDragEnd(event);
                }
            }
        }

        void ToolBoxConnector::processDragEnd(const MouseEvent&) {
            if (m_toolBox->dragging()) {
                m_toolBox->endMouseDrag(m_inputState);
                m_inputState.setAnyToolDragging(false);
            }
        }

        MouseButtonState ToolBoxConnector::mouseButton(const MouseEvent& event) {
            switch (event.button) {
                case MouseEvent::Button::Left:
                    return MouseButtons::MBLeft;
                case MouseEvent::Button::Middle:
                    return MouseButtons::MBMiddle;
                case MouseEvent::Button::Right:
                    return MouseButtons::MBRight;
                case MouseEvent::Button::Aux1:
                case MouseEvent::Button::Aux2:
                case MouseEvent::Button::None:
                    return MouseButtons::MBNone;
                switchDefault()
            }
        }

        void ToolBoxConnector::mouseMoved(const float x, const float y) {
            const auto dx = x - m_lastMouseX;
            const auto dy = y - m_lastMouseY;
            m_inputState.mouseMove(x, y, dx, dy);
            m_lastMouseX = x;
            m_lastMouseY = y;
        }

        bool ToolBoxConnector::cancelDrag() {
            if (m_toolBox->dragging()) {
                m_toolBox->cancelMouseDrag();
                m_inputState.setAnyToolDragging(false);
                return true;
            } else {
                return false;
            }
        }

        void ToolBoxConnector::doShowPopupMenu() {}
    }
}
