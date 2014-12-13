/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

namespace TrenchBroom {
    namespace View {
        ToolBoxConnector::ToolBoxConnector(wxWindow* window, ToolBox& toolBox, const InputSource inputSource) :
        m_window(window),
        m_toolBox(toolBox),
        m_toolChain(NULL),
        m_inputState(inputSource) {
            assert(m_window != NULL);
            bindEvents();
        }

        ToolBoxConnector::~ToolBoxConnector() {
            unbindEvents();
        }
        
        const Ray3& ToolBoxConnector::pickRay() const {
            return m_inputState.pickRay();
        }
        
        const Hits& ToolBoxConnector::hits() const {
            return m_inputState.hits();
        }
        
        void ToolBoxConnector::updateHits() {
            m_inputState.setPickRequest(doGetPickRequest(m_inputState.mouseX(),  m_inputState.mouseY()));
            Hits hits = doPick(m_inputState.pickRay());
            if (m_toolChain != NULL)
                m_toolBox.pick(m_toolChain, m_inputState, hits);
            m_inputState.setHits(hits);
        }
        
        void ToolBoxConnector::updateLastActivation() {
            m_toolBox.updateLastActivation();
        }

        void ToolBoxConnector::addTool(Tool* tool) {
            if (m_toolChain == NULL)
                m_toolChain = new ToolChain(tool);
            else
                m_toolChain->append(new ToolChain(tool));
        }

        bool ToolBoxConnector::dragEnter(const wxCoord x, const wxCoord y, const String& text) {
            mouseMoved(wxPoint(x, y));
            updateHits();

            if (m_toolChain == NULL)
                return false;
            
            const bool result = m_toolBox.dragEnter(m_toolChain, m_inputState, text);
            m_window->Refresh();
            return result;
        }

        bool ToolBoxConnector::dragMove(const wxCoord x, const wxCoord y, const String& text) {
            mouseMoved(wxPoint(x, y));
            updateHits();
            
            if (m_toolChain == NULL)
                return false;
            
            const bool result = m_toolBox.dragMove(m_toolChain, m_inputState, text);
            m_window->Refresh();
            return result;
        }
        
        void ToolBoxConnector::dragLeave() {
            m_toolBox.dragLeave(m_toolChain, m_inputState);
            m_window->Refresh();
        }

        bool ToolBoxConnector::dragDrop(const wxCoord x, const wxCoord y, const String& text) {
            updateHits();
            
            if (m_toolChain == NULL)
                return false;
            
            const bool result = m_toolBox.dragDrop(m_toolChain, m_inputState, text);
            m_window->Refresh();
            if (result)
                m_window->SetFocus();
            return result;
        }

        bool ToolBoxConnector::cancel() {
            return m_toolBox.cancel(m_toolChain);
        }

        void ToolBoxConnector::setRenderOptions(Renderer::RenderContext& renderContext) {
            if (m_toolChain != NULL)
                m_toolBox.setRenderOptions(m_toolChain, m_inputState, renderContext);
        }
        
        void ToolBoxConnector::renderTools(Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
            if (m_toolChain != NULL)
                m_toolBox.renderTools(m_toolChain, m_inputState, renderContext, renderBatch);
        }

        void ToolBoxConnector::OnKey(wxKeyEvent& event) {
            if (updateModifierKeys()) {
                updateHits();
                if (m_toolChain != NULL)
                    m_toolBox.modifierKeyChange(m_toolChain, m_inputState);
            }
            m_window->Refresh();
        }
        
        void ToolBoxConnector::OnMouseButton(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);
            if (m_toolBox.ignoreNextClick() && button == MouseButtons::MBLeft) {
                if (event.ButtonUp())
                    m_toolBox.clearIgnoreNextClick();
                event.Skip();
                return;
            }
            
            m_window->SetFocus();
            if (event.ButtonUp())
                m_toolBox.clearIgnoreNextClick();
            
            updateModifierKeys();
            if (event.ButtonDown()) {
                captureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(button);
                if (m_toolChain != NULL)
                    m_toolBox.mouseDown(m_toolChain, m_inputState);
            } else {
                if (m_toolBox.dragging()) {
                    m_toolBox.endMouseDrag(m_inputState);
                    m_inputState.mouseUp(button);
                    releaseMouse();
                } else if (!m_ignoreNextDrag) {
                    const bool handled = m_toolChain == NULL ? false : m_toolBox.mouseUp(m_toolChain, m_inputState);
                    m_inputState.mouseUp(button);
                    releaseMouse();
                    
                    if (button == MouseButtons::MBRight && !handled) {
                        // We miss mouse events when a popup menu is already open, so we must make sure that the input
                        // state is up to date.
                        
                        mouseMoved(event.GetPosition());
                        updateHits();
                        
                        showPopupMenu();
                    }
                } else {
                    m_inputState.mouseUp(button);
                    releaseMouse();
                }
            }
            
            updateHits();
            m_ignoreNextDrag = false;
            
            m_window->Refresh();
        }

        void ToolBoxConnector::OnMouseDoubleClick(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);
            updateModifierKeys();
            
            m_clickPos = event.GetPosition();
            m_inputState.mouseDown(button);
            if (m_toolChain != NULL)
                m_toolBox.mouseDoubleClick(m_toolChain, m_inputState);
            m_inputState.mouseUp(button);
            
            updateHits();
            
            m_window->Refresh();
        }
        
        void ToolBoxConnector::OnMouseMotion(wxMouseEvent& event) {
            updateModifierKeys();
            if (m_toolBox.dragging()) {
                mouseMoved(event.GetPosition());
                updateHits();
                if (!m_toolBox.mouseDrag(m_inputState)) {
                    m_toolBox.endMouseDrag(m_inputState);
                    m_ignoreNextDrag = true;
                }
            } else if (!m_ignoreNextDrag) {
                if (m_inputState.mouseButtons() != MouseButtons::MBNone) {
                    if (std::abs(event.GetPosition().x - m_clickPos.x) > 1 ||
                        std::abs(event.GetPosition().y - m_clickPos.y) > 1) {
                        const bool dragStarted = m_toolChain == NULL ? false : m_toolBox.startMouseDrag(m_toolChain, m_inputState);
                        if (dragStarted)
                            m_ignoreNextDrag = true;
                        mouseMoved(event.GetPosition());
                        updateHits();
                        if (dragStarted)
                            m_toolBox.mouseDrag(m_inputState);
                    }
                } else {
                    mouseMoved(event.GetPosition());
                    updateHits();
                    if (m_toolChain != NULL)
                        m_toolBox.mouseMove(m_toolChain, m_inputState);
                }
            }
            
            m_window->Refresh();
        }
        
        void ToolBoxConnector::OnMouseWheel(wxMouseEvent& event) {
            updateModifierKeys();
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            if (m_toolChain != NULL)
                m_toolBox.mouseWheel(m_toolChain, m_inputState);
            
            updateHits();
            m_window->Refresh();
        }

        
        void ToolBoxConnector::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            cancelDrag();
            m_window->Refresh();
            event.Skip();
        }

        void ToolBoxConnector::OnSetFocus(wxFocusEvent& event) {
            if (m_toolChain != NULL && updateModifierKeys())
                m_toolBox.modifierKeyChange(m_toolChain, m_inputState);
            m_window->Refresh();
            m_window->SetCursor(wxCursor(wxCURSOR_ARROW));
            
            mouseMoved(m_window->ScreenToClient(wxGetMousePosition()));
            
            // if this focus event happens as a result of a window activation, the don't ignore the next click
            m_toolBox.clearIgnoreNextClickWithinActivationTime();
            
            event.Skip();
        }
        
        void ToolBoxConnector::OnKillFocus(wxFocusEvent& event) {
            cancelDrag();
            releaseMouse();
            if (m_toolChain != NULL && clearModifierKeys())
                m_toolBox.modifierKeyChange(m_toolChain, m_inputState);
            if (m_toolBox.clickToActivate()) {
                m_toolBox.setIgnoreNextClick();
                m_window->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            m_window->Refresh();
            event.Skip();
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

        void ToolBoxConnector::captureMouse() {
            if (!m_window->HasCapture() && !m_toolBox.dragging())
                m_window->CaptureMouse();
        }
        
        void ToolBoxConnector::releaseMouse() {
            if (m_window->HasCapture() && !m_toolBox.dragging())
                m_window->ReleaseMouse();
        }
        
        
        void ToolBoxConnector::cancelDrag() {
            if (m_toolBox.dragging()) {
                m_toolBox.cancelDrag();
                m_inputState.clearMouseButtons();
            }
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
        
        bool ToolBoxConnector::updateModifierKeys() {
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
        }

        void ToolBoxConnector::doShowPopupMenu() {}
    }
}
