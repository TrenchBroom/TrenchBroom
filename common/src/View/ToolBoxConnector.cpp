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

#include "View/ToolBox.h"
#include "View/ToolChain.h"

#include <QWidget>
#include <QCursor>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGuiApplication>

namespace TrenchBroom {
    namespace View {
        // ToolBoxConnector::EventFilter

        ToolBoxConnector::EventFilter::EventFilter(ToolBoxConnector* owner) :
        QObject(),
        m_owner(owner) {}

        bool ToolBoxConnector::EventFilter::eventFilter(QObject *obj, QEvent *ev) {
            switch (ev->type()) {
                case QEvent::KeyPress:
                case QEvent::KeyRelease:
                    m_owner->OnKey(dynamic_cast<QKeyEvent *>(ev));
                    break;
                case QEvent::MouseButtonPress:
                case QEvent::MouseButtonRelease:
                    m_owner->OnMouseButton(dynamic_cast<QMouseEvent *>(ev));
                    // FIXME: We need to consume this otherwise some Qt themes let you drag the window around by dragging on the map view - when exactly should it be consumed?
                    return true;
                case QEvent::MouseButtonDblClick:
                    m_owner->OnMouseDoubleClick(dynamic_cast<QMouseEvent *>(ev));
                    break;
                case QEvent::MouseMove:
                    m_owner->OnMouseMotion(dynamic_cast<QMouseEvent *>(ev));
                    break;
                case QEvent::Wheel:
                    m_owner->OnMouseWheel(dynamic_cast<QWheelEvent *>(ev));
                    break;
                case QEvent::FocusIn:
                    m_owner->OnSetFocus(dynamic_cast<QFocusEvent *>(ev));
                    break;
                case QEvent::FocusOut:
                    m_owner->OnKillFocus(dynamic_cast<QFocusEvent *>(ev));
                    break;
                default:
                    break;
                    // FIXME: handle ToolBoxConnector::OnMouseCaptureLost?
            }

            // Continue normal Qt event handling
            return QObject::eventFilter(obj, ev);
        }

        // ToolBoxConnector

        ToolBoxConnector::ToolBoxConnector(QWidget* window) :
        m_window(window),
        m_toolBox(nullptr),
        m_toolChain(new ToolChain()),
        m_ignoreNextDrag(false),
        m_eventFilter(new EventFilter(this)) {
            ensure(m_window != nullptr, "window is null");
            m_window->installEventFilter(m_eventFilter);
            m_window->setMouseTracking(true);
        }

        ToolBoxConnector::~ToolBoxConnector() {
            m_window->removeEventFilter(m_eventFilter);
            delete m_eventFilter;
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
            m_inputState.setPickResult(pickResult);
        }

        void ToolBoxConnector::updateLastActivation() {
            ensure(m_toolBox != nullptr, "toolBox is null");
            m_toolBox->updateLastActivation();
        }

        void ToolBoxConnector::setToolBox(ToolBox& toolBox) {
            assert(m_toolBox == nullptr);
            m_toolBox = &toolBox;
        }

        void ToolBoxConnector::addTool(ToolController* tool) {
            m_toolChain->append(tool);
        }

        bool ToolBoxConnector::dragEnter(const int x, const int y, const String& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            mouseMoved(QPoint(x, y));
            updatePickResult();

            const bool result = m_toolBox->dragEnter(m_toolChain, m_inputState, text);
            m_window->update();
            return result;
        }

        bool ToolBoxConnector::dragMove(const int x, const int y, const String& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            mouseMoved(QPoint(x, y));
            updatePickResult();

            const bool result = m_toolBox->dragMove(m_toolChain, m_inputState, text);
            m_window->update();
            return result;
        }

        void ToolBoxConnector::dragLeave() {
            ensure(m_toolBox != nullptr, "toolBox is null");

            m_toolBox->dragLeave(m_toolChain, m_inputState);
            m_window->update();
        }

        bool ToolBoxConnector::dragDrop(const int x, const int y, const String& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            updatePickResult();

            const bool result = m_toolBox->dragDrop(m_toolChain, m_inputState, text);
            m_window->update();
            if (result)
                m_window->setFocus();
            return result;
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

        void ToolBoxConnector::OnKey(QKeyEvent* event) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            updateModifierKeys();
            m_window->update();
        }

        void ToolBoxConnector::OnMouseButton(QMouseEvent* event) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            const MouseButtonState button = mouseButton(event);
            if (m_toolBox->ignoreNextClick() && button == MouseButtons::MBLeft) {
                if (event->type() == QEvent::MouseButtonRelease)
                    m_toolBox->clearIgnoreNextClick();
                return;
            }

            m_window->setFocus();
            if (event->type() == QEvent::MouseButtonRelease)
                m_toolBox->clearIgnoreNextClick();

            updateModifierKeys();
            if (event->type() == QEvent::MouseButtonPress) {
                captureMouse();
                m_clickTime = event->timestamp();
                m_clickPos = event->pos();
                m_inputState.mouseDown(button);
                m_toolBox->mouseDown(m_toolChain, m_inputState);
            } else {
                if (m_toolBox->dragging()) {
                    endDrag(event);
                } else if (!m_ignoreNextDrag) {
                    m_toolBox->mouseUp(m_toolChain, m_inputState);
                    const bool handled = isWithinClickDistance(event->pos()) && m_toolBox->mouseClick(m_toolChain, m_inputState);
                    m_inputState.mouseUp(button);
                    releaseMouse();

                    if (button == MouseButtons::MBRight && !handled) {
                        // We miss mouse events when a popup menu is already open, so we must make sure that the input
                        // state is up to date.
                        mouseMoved(event->pos());
                        updatePickResult();
                        showPopupMenu();
                    }
                } else {
                    m_toolBox->mouseUp(m_toolChain, m_inputState);
                    m_inputState.mouseUp(button);
                    releaseMouse();
                }
            }

            updatePickResult();
            m_ignoreNextDrag = false;

            m_window->update();
        }

        void ToolBoxConnector::OnMouseDoubleClick(QMouseEvent* event) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            m_window->setFocus();
            m_toolBox->clearIgnoreNextClick();

            const MouseButtonState button = mouseButton(event);
            updateModifierKeys();

            if (m_toolBox->dragging()) {
                endDrag(event);
            } else {
                m_clickPos = event->pos();
                m_inputState.mouseDown(button);
                m_toolBox->mouseDoubleClick(m_toolChain, m_inputState);
                m_inputState.mouseUp(button);
            }
            

            updatePickResult();

            m_window->update();
        }

        void ToolBoxConnector::OnMouseMotion(QMouseEvent* event) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            updateModifierKeys();
            if (m_toolBox->dragging()) {
                drag(event);
            } else if (!m_ignoreNextDrag) {
                if (m_inputState.mouseButtons() != MouseButtons::MBNone) {
                    startDrag(event);
                } else {
                    mouseMoved(event->pos());
                    updatePickResult();
                    m_toolBox->mouseMove(m_toolChain, m_inputState);
                }
            }

            m_window->update();
        }

        void ToolBoxConnector::OnMouseWheel(QWheelEvent* event) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            updateModifierKeys();

            const float deltaX = event->angleDelta().x() / 120.0f;
            const float deltaY = event->angleDelta().y() / 120.0f;
            m_inputState.scroll(deltaX, deltaY);
            m_toolBox->mouseScroll(m_toolChain, m_inputState);

            updatePickResult();
            m_window->update();
        }

//        void ToolBoxConnector::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
//            ensure(m_toolBox != nullptr, "toolBox is null");
//
//            cancelDrag();
//            m_window->update();
//        }

        void ToolBoxConnector::OnSetFocus(QFocusEvent* event) {
            ensure(m_toolBox != nullptr, "toolBox is null");
            
            updateModifierKeys();
            m_window->update();

            mouseMoved(m_window->mapFromGlobal(QCursor::pos()));
        }

        void ToolBoxConnector::OnKillFocus(QFocusEvent* event) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            cancelDrag();
            releaseMouse();
            updateModifierKeys();
            m_window->update();
        }

        bool ToolBoxConnector::isWithinClickDistance(const QPoint& pos) const {
            return (std::abs(pos.x() - m_clickPos.x()) <= 1 &&
                    std::abs(pos.y() - m_clickPos.y()) <= 1);
        }

        void ToolBoxConnector::startDrag(QMouseEvent* event) {
            if (!isWithinClickDistance(event->pos())) {
                const bool dragStarted = m_toolBox->startMouseDrag(m_toolChain, m_inputState);
                if (dragStarted) {
                    m_ignoreNextDrag = true;
                    m_inputState.setAnyToolDragging(true);
                    drag(event);
                } else {
                    mouseMoved(event->pos());
                    updatePickResult();
                }
            }
        }
        
        void ToolBoxConnector::drag(QMouseEvent* event) {
            mouseMoved(event->pos());
            updatePickResult();
            if (!m_toolBox->mouseDrag(m_inputState)) {
                endDrag(event);
                m_ignoreNextDrag = true;
            }
        }
        
        void ToolBoxConnector::endDrag(QMouseEvent* event) {
            assert(m_toolBox->dragging());
            
            const auto clickInterval = event->timestamp() - m_clickTime;
            if (clickInterval <= 100) {
                cancelDrag();
            } else {
                m_toolBox->endMouseDrag(m_inputState);
                m_toolBox->mouseUp(m_toolChain, m_inputState);
            }
            
            const MouseButtonState button = mouseButton(event);
            m_inputState.mouseUp(button);
            m_inputState.setAnyToolDragging(false);
            releaseMouse();
        }

        bool ToolBoxConnector::cancelDrag() {
            if (m_toolBox->dragging()) {
                m_toolBox->cancelMouseDrag();
                m_inputState.setAnyToolDragging(false);
                m_inputState.clearMouseButtons();
                return true;
            } else {
                return false;
            }
        }
        
        void ToolBoxConnector::captureMouse() {
        }

        void ToolBoxConnector::releaseMouse() {
        }

        ModifierKeyState ToolBoxConnector::modifierKeys() {
            const Qt::KeyboardModifiers mouseState = QGuiApplication::keyboardModifiers();

            ModifierKeyState state = ModifierKeys::MKNone;
            if (mouseState & Qt::ControlModifier)
                state |= ModifierKeys::MKCtrlCmd;
            if (mouseState & Qt::ShiftModifier)
                state |= ModifierKeys::MKShift;
            if (mouseState & Qt::AltModifier)
                state |= ModifierKeys::MKAlt;
            return state;
        }

        bool ToolBoxConnector::setModifierKeys() {
            const ModifierKeyState keys = modifierKeys();
            if (keys != m_inputState.modifierKeys()) {
                m_inputState.setModifierKeys(keys);
                return true;
            }
            return false;
        }

        bool ToolBoxConnector::clearModifierKeys() {
            if (m_inputState.modifierKeys() != ModifierKeys::MKNone) {
                m_inputState.setModifierKeys(ModifierKeys::MKNone);
                return true;
            }
            return false;
        }

        void ToolBoxConnector::updateModifierKeys() {
            if (setModifierKeys()) {
                updatePickResult();
                m_toolBox->modifierKeyChange(m_toolChain, m_inputState);
            }
        }

        MouseButtonState ToolBoxConnector::mouseButton(QMouseEvent* event) {
            switch (event->button()) {
                case Qt::LeftButton:
                    return MouseButtons::MBLeft;
                case Qt::MidButton:
                    return MouseButtons::MBMiddle;
                case Qt::RightButton:
                    return MouseButtons::MBRight;
                default:
                    return MouseButtons::MBNone;
            }
        }

        void ToolBoxConnector::mouseMoved(const QPoint& position) {
            const auto delta = position - m_lastMousePos;
            m_inputState.mouseMove(position.x(), position.y(), delta.x(), delta.y());
            m_lastMousePos = position;
        }

        void ToolBoxConnector::showPopupMenu() {
            doShowPopupMenu();
            updateModifierKeys();
        }

        void ToolBoxConnector::doShowPopupMenu() {}
    }
}
