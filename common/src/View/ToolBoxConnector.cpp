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

#include <wx/window.h>
#include <wx/time.h>

namespace TrenchBroom {
    namespace View {
        ToolBoxConnector::ToolBoxConnector(wxWindow* window) :
        m_window(window),
        m_toolBox(nullptr),
        m_toolChain(new ToolChain()),
        m_ignoreNextDrag(false) {
            ensure(m_window != nullptr, "window is null");
            bindEvents();
        }

        ToolBoxConnector::~ToolBoxConnector() {
            unbindEvents();
            delete m_toolChain;
        }

        const Ray3& ToolBoxConnector::pickRay() const {
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

        bool ToolBoxConnector::dragEnter(const wxCoord x, const wxCoord y, const String& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            mouseMoved(wxPoint(x, y));
            updatePickResult();

            const bool result = m_toolBox->dragEnter(m_toolChain, m_inputState, text);
            m_window->Refresh();
            return result;
        }

        bool ToolBoxConnector::dragMove(const wxCoord x, const wxCoord y, const String& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            mouseMoved(wxPoint(x, y));
            updatePickResult();

            const bool result = m_toolBox->dragMove(m_toolChain, m_inputState, text);
            m_window->Refresh();
            return result;
        }

        void ToolBoxConnector::dragLeave() {
            ensure(m_toolBox != nullptr, "toolBox is null");

            m_toolBox->dragLeave(m_toolChain, m_inputState);
            m_window->Refresh();
        }

        bool ToolBoxConnector::dragDrop(const wxCoord x, const wxCoord y, const String& text) {
            ensure(m_toolBox != nullptr, "toolBox is null");

            updatePickResult();

            const bool result = m_toolBox->dragDrop(m_toolChain, m_inputState, text);
            m_window->Refresh();
            if (result)
                m_window->SetFocus();
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
        
        void ToolBoxConnector::bindEvents() {
            m_window->Bind(wxEVT_KEY_DOWN, &ToolBoxConnector::OnKey, this);
            m_window->Bind(wxEVT_KEY_UP, &ToolBoxConnector::OnKey, this);
            m_window->Bind(wxEVT_LEFT_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_LEFT_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_LEFT_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_RIGHT_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_RIGHT_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_RIGHT_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_MIDDLE_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_MIDDLE_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_MIDDLE_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_AUX1_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX1_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX1_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_AUX2_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX2_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Bind(wxEVT_AUX2_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Bind(wxEVT_MOTION, &ToolBoxConnector::OnMouseMotion, this);
            m_window->Bind(wxEVT_MOUSEWHEEL, &ToolBoxConnector::OnMouseWheel, this);
            m_window->Bind(wxEVT_MOUSE_CAPTURE_LOST, &ToolBoxConnector::OnMouseCaptureLost, this);
            m_window->Bind(wxEVT_SET_FOCUS, &ToolBoxConnector::OnSetFocus, this);
            m_window->Bind(wxEVT_KILL_FOCUS, &ToolBoxConnector::OnKillFocus, this);
        }
        
        void ToolBoxConnector::unbindEvents() {
            m_window->Unbind(wxEVT_KEY_DOWN, &ToolBoxConnector::OnKey, this);
            m_window->Unbind(wxEVT_KEY_UP, &ToolBoxConnector::OnKey, this);
            m_window->Unbind(wxEVT_LEFT_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_LEFT_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_LEFT_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_RIGHT_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_RIGHT_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_RIGHT_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_MIDDLE_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_MIDDLE_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_MIDDLE_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_AUX1_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX1_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX1_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_AUX2_DOWN, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX2_UP, &ToolBoxConnector::OnMouseButton, this);
            m_window->Unbind(wxEVT_AUX2_DCLICK, &ToolBoxConnector::OnMouseDoubleClick, this);
            m_window->Unbind(wxEVT_MOTION, &ToolBoxConnector::OnMouseMotion, this);
            m_window->Unbind(wxEVT_MOUSEWHEEL, &ToolBoxConnector::OnMouseWheel, this);
            m_window->Unbind(wxEVT_MOUSE_CAPTURE_LOST, &ToolBoxConnector::OnMouseCaptureLost, this);
            m_window->Unbind(wxEVT_SET_FOCUS, &ToolBoxConnector::OnSetFocus, this);
            m_window->Unbind(wxEVT_KILL_FOCUS, &ToolBoxConnector::OnKillFocus, this);
        }

        void ToolBoxConnector::OnKey(wxKeyEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");

            event.Skip();
            updateModifierKeys();
            m_window->Refresh();
        }

        void ToolBoxConnector::OnMouseButton(wxMouseEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");

            event.Skip();

            const MouseButtonState button = mouseButton(event);
            if (m_toolBox->ignoreNextClick() && button == MouseButtons::MBLeft) {
                if (event.ButtonUp())
                    m_toolBox->clearIgnoreNextClick();
                return;
            }

            m_window->SetFocus();
            if (event.ButtonUp())
                m_toolBox->clearIgnoreNextClick();

            updateModifierKeys();
            if (event.ButtonDown()) {
                captureMouse();
                m_clickTime = wxGetLocalTimeMillis();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(button);
                m_toolBox->mouseDown(m_toolChain, m_inputState);
            } else {
                if (m_toolBox->dragging()) {
                    endDrag(event);
                } else if (!m_ignoreNextDrag) {
                    m_toolBox->mouseUp(m_toolChain, m_inputState);
                    const bool handled = isWithinClickDistance(event.GetPosition()) && m_toolBox->mouseClick(m_toolChain, m_inputState);
                    m_inputState.mouseUp(button);
                    releaseMouse();

                    if (button == MouseButtons::MBRight && !handled) {
                        // We miss mouse events when a popup menu is already open, so we must make sure that the input
                        // state is up to date.
                        mouseMoved(event.GetPosition());
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

            m_window->Refresh();
        }

        void ToolBoxConnector::OnMouseDoubleClick(wxMouseEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");

            event.Skip();

            m_window->SetFocus();
            m_toolBox->clearIgnoreNextClick();

            const MouseButtonState button = mouseButton(event);
            updateModifierKeys();

            if (m_toolBox->dragging()) {
                endDrag(event);
            } else {
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(button);
                m_toolBox->mouseDoubleClick(m_toolChain, m_inputState);
                m_inputState.mouseUp(button);
            }
            

            updatePickResult();

            m_window->Refresh();
        }

        void ToolBoxConnector::OnMouseMotion(wxMouseEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");

            event.Skip();

            updateModifierKeys();
            if (m_toolBox->dragging()) {
                drag(event);
            } else if (!m_ignoreNextDrag) {
                if (m_inputState.mouseButtons() != MouseButtons::MBNone) {
                    startDrag(event);
                } else {
                    mouseMoved(event.GetPosition());
                    updatePickResult();
                    m_toolBox->mouseMove(m_toolChain, m_inputState);
                }
            }

            m_window->Refresh();
#ifdef _WIN32
			m_window->Update(); // neccessary for smooth rendering on Windows
#endif
        }

        void ToolBoxConnector::OnMouseWheel(wxMouseEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");

            event.Skip();

            updateModifierKeys();
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            m_toolBox->mouseScroll(m_toolChain, m_inputState);

            updatePickResult();
            m_window->Refresh();
        }


        void ToolBoxConnector::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");

            event.Skip();
            
            cancelDrag();
            m_window->Refresh();
        }

        void ToolBoxConnector::OnSetFocus(wxFocusEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");
            
            event.Skip();
            updateModifierKeys();
            m_window->Refresh();

            mouseMoved(m_window->ScreenToClient(wxGetMousePosition()));
        }

        void ToolBoxConnector::OnKillFocus(wxFocusEvent& event) {
            if (m_window->IsBeingDeleted()) return;

            ensure(m_toolBox != nullptr, "toolBox is null");

            event.Skip();
            
            cancelDrag();
            releaseMouse();
            updateModifierKeys();
            m_window->Refresh();
        }

        bool ToolBoxConnector::isWithinClickDistance(const wxPoint& pos) const {
            return (std::abs(pos.x - m_clickPos.x) <= 1 &&
                    std::abs(pos.y - m_clickPos.y) <= 1);
        }

        
        void ToolBoxConnector::startDrag(wxMouseEvent& event) {
            if (!isWithinClickDistance(event.GetPosition())) {
                const bool dragStarted = m_toolBox->startMouseDrag(m_toolChain, m_inputState);
                if (dragStarted) {
                    m_ignoreNextDrag = true;
                    m_inputState.setAnyToolDragging(true);
                    drag(event);
                } else {
                    mouseMoved(event.GetPosition());
                    updatePickResult();
                }
            }
        }
        
        void ToolBoxConnector::drag(wxMouseEvent& event) {
            mouseMoved(event.GetPosition());
            updatePickResult();
            if (!m_toolBox->mouseDrag(m_inputState)) {
                endDrag(event);
                m_ignoreNextDrag = true;
            }
        }
        
        void ToolBoxConnector::endDrag(wxMouseEvent& event) {
            assert(m_toolBox->dragging());
            
            const wxLongLong clickInterval = wxGetLocalTimeMillis() - m_clickTime;
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
            if (!m_window->HasCapture() && !m_toolBox->dragging())
                m_window->CaptureMouse();
        }

        void ToolBoxConnector::releaseMouse() {
            if (m_window->HasCapture() && !m_toolBox->dragging())
                m_window->ReleaseMouse();
        }


        ModifierKeyState ToolBoxConnector::modifierKeys() {
            const wxMouseState mouseState = wxGetMouseState();

            ModifierKeyState state = ModifierKeys::MKNone;
            if (mouseState.CmdDown())
                state |= ModifierKeys::MKCtrlCmd;
            if (mouseState.ShiftDown())
                state |= ModifierKeys::MKShift;
            if (mouseState.AltDown())
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

        MouseButtonState ToolBoxConnector::mouseButton(wxMouseEvent& event) {
            switch (event.GetButton()) {
                case wxMOUSE_BTN_LEFT:
                    return MouseButtons::MBLeft;
                case wxMOUSE_BTN_MIDDLE:
                    return MouseButtons::MBMiddle;
                case wxMOUSE_BTN_RIGHT:
                    return MouseButtons::MBRight;
                default:
                    return MouseButtons::MBNone;
            }
        }

        void ToolBoxConnector::mouseMoved(const wxPoint& position) {
            const wxPoint delta = position - m_lastMousePos;
            m_inputState.mouseMove(position.x, position.y, delta.x, delta.y);
            m_lastMousePos = position;
        }

        void ToolBoxConnector::showPopupMenu() {
            doShowPopupMenu();
            updateModifierKeys();
        }

        void ToolBoxConnector::doShowPopupMenu() {}
    }
}
