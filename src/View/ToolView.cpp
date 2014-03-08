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

#include "ToolView.h"
#include "Renderer/Camera.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/Tool.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        ToolView::ToolView(wxWindow* parent, Renderer::Camera& camera, const GLAttribs& attribs, const wxGLContext* sharedContext) :
        wxGLCanvas(parent, wxID_ANY, &attribs.front()),
        m_camera(camera),
        m_inputState(m_camera),
        m_glContext(new wxGLContext(this, sharedContext)),
        m_initialized(false),
        m_toolChain(NULL),
        m_dragReceiver(NULL),
        m_modalReceiver(NULL),
        m_dropReceiver(NULL),
        m_savedDropReceiver(NULL),
        m_ignoreNextDrag(false),
        m_ignoreNextClick(false),
        m_lastFrameActivation(wxDateTime::Now()),
        m_animationManager(new AnimationManager()) {
            bindEvents();
        }
        
        ToolView::~ToolView() {
            m_animationManager->Delete();
            m_animationManager = NULL;
            delete m_glContext;
            m_glContext = NULL;
        }

        bool ToolView::dragEnter(const wxCoord x, const wxCoord y, const String& text) {
            assert(m_dropReceiver == NULL);
            
            deactivateAllTools();
            m_inputState.mouseMove(x, y);
            updateHits();
            m_dropReceiver = m_toolChain->dragEnter(m_inputState, text);
            Refresh();
            
            return m_dropReceiver != NULL;
        }
        
        bool ToolView::dragMove(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL)
                return false;
            
            m_inputState.mouseMove(x, y);
            updateHits();
            m_dropReceiver->dragMove(m_inputState);
            Refresh();
            
            return true;
        }
        
        void ToolView::dragLeave() {
            if (m_dropReceiver == NULL)
                return;
            
            // This is a workaround for a bug in wxWidgets 3.0.0 on GTK2, where a drag leave event
            // is sent right before the drop event. So we save the drag receiver in an instance variable
            // and if dragDrop() is called, it can use that variable to find out who the drop receiver is.
            m_savedDropReceiver = m_dropReceiver;
            
            m_dropReceiver->dragLeave(m_inputState);
            m_dropReceiver = NULL;
            Refresh();
        }
        
        bool ToolView::dragDrop(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL && m_savedDropReceiver == NULL)
                return false;
            
            if (m_dropReceiver == NULL) {
                m_dropReceiver = m_savedDropReceiver;
                m_dropReceiver->activate(m_inputState); // GTK2 fix: has been deactivated by dragLeave()
                m_dropReceiver->dragEnter(m_inputState, text);
            }
            
            updateHits();
            const bool success = m_dropReceiver->dragDrop(m_inputState);
            m_dropReceiver->deactivate(m_inputState);
            m_dropReceiver = NULL;
            m_savedDropReceiver = NULL;
            Refresh();
            
            return success;
        }
        
        void ToolView::OnKey(wxKeyEvent& event) {
            if (updateModifierKeys()) {
                updateHits();
                m_toolChain->modifierKeyChange(m_inputState);
            }
            Refresh();
            event.Skip();
        }
        
        void ToolView::OnMouseButton(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);
            
            if (m_ignoreNextClick && button == MouseButtons::MBLeft) {
                if (event.ButtonUp())
                    m_ignoreNextClick = false;
                event.Skip();
                return;
            }
            
            updateModifierKeys();
            if (event.ButtonDown()) {
                if (!HasCapture())
                    CaptureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(button);
                m_toolChain->mouseDown(m_inputState);
            } else {
                if (m_dragReceiver != NULL) {
                    m_dragReceiver->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                    
                    m_inputState.mouseUp(button);
                    if (HasCapture())
                        ReleaseMouse();
                } else if (!m_ignoreNextDrag) {
                    const bool handled = m_toolChain->mouseUp(m_inputState);
                    
                    m_inputState.mouseUp(button);
                    if (HasCapture())
                        ReleaseMouse();
                    
                    if (button == MouseButtons::MBRight && !handled)
                        showPopupMenu();
                } else {
                    m_inputState.mouseUp(button);
                    if (HasCapture())
                        ReleaseMouse();
                }
            }
            
            updateHits();
            m_ignoreNextDrag = false;
            
            Refresh();
            event.Skip();
        }
        
        void ToolView::OnMouseDoubleClick(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);
            updateModifierKeys();
            
            m_clickPos = event.GetPosition();
            m_inputState.mouseDown(button);
            m_toolChain->mouseDoubleClick(m_inputState);
            m_inputState.mouseUp(button);
            
            updateHits();
            
            Refresh();
            event.Skip();
        }
        
        void ToolView::OnMouseMotion(wxMouseEvent& event) {
            updateModifierKeys();
            updateHits();
            if (m_dragReceiver != NULL) {
                m_inputState.mouseMove(event.GetX(), event.GetY());
                if (!m_dragReceiver->mouseDrag(m_inputState)) {
                    m_dragReceiver->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                    m_ignoreNextDrag = true;
                }
            } else if (!m_ignoreNextDrag) {
                if (m_inputState.mouseButtons() != MouseButtons::MBNone &&
                    (std::abs(event.GetX() - m_clickPos.x) > 1 ||
                     std::abs(event.GetY() - m_clickPos.y) > 1)) {
                        m_dragReceiver = m_toolChain->startMouseDrag(m_inputState);
                        if (m_dragReceiver == NULL)
                            m_ignoreNextDrag = true;
                    }
                if (m_dragReceiver != NULL) {
                    m_inputState.mouseMove(event.GetX(), event.GetY());
                    m_dragReceiver->mouseDrag(m_inputState);
                } else {
                    m_inputState.mouseMove(event.GetX(), event.GetY());
                    m_toolChain->mouseMove(m_inputState);
                }
            }
            Refresh();
            event.Skip();
        }
        
        void ToolView::OnMouseWheel(wxMouseEvent& event) {
            updateModifierKeys();
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            m_toolChain->scroll(m_inputState);
            
            updateHits();
            Refresh();
            event.Skip();
        }
        
        void ToolView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            cancelCurrentDrag();
            Refresh();
            event.Skip();
        }
        
        void ToolView::OnSetFocus(wxFocusEvent& event) {
            if (updateModifierKeys())
                m_toolChain->modifierKeyChange(m_inputState);
            Refresh();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            
            // if this focus event happens as a result of a window activation, the don't ignore the next click
            if ((wxDateTime::Now() - m_lastFrameActivation).IsShorterThan(wxTimeSpan(0, 0, 0, 100)))
                m_ignoreNextClick = false;
            
            event.Skip();
        }
        
        void ToolView::OnKillFocus(wxFocusEvent& event) {
            cancelCurrentDrag();
            if (GetCapture() == this)
                ReleaseMouse();
            if (clearModifierKeys())
                m_toolChain->modifierKeyChange(m_inputState);
            m_ignoreNextClick = true;
            Refresh();
            SetCursor(wxCursor(wxCURSOR_HAND));
            event.Skip();
        }
        
        void ToolView::OnActivateFrame(wxActivateEvent& event) {
            m_lastFrameActivation = wxDateTime::Now();
        }
        
        void ToolView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (!m_initialized)
                initializeGL();
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);
                render();
                SwapBuffers();
            }
#endif
        }
        
        void ToolView::OnSize(wxSizeEvent& event) {
            updateViewport();
            event.Skip();
        }

        const wxGLContext* ToolView::glContext() const {
            return m_glContext;
        }
        
        bool ToolView::anyToolActive() const {
            return m_modalReceiver != NULL;
        }
        
        void ToolView::addTool(Tool* tool) {
            if (m_toolChain == NULL)
                m_toolChain = tool;
            else
                m_toolChain->appendTool(tool);
        }
        
        bool ToolView::toolActive(const Tool* tool) const {
            return m_modalReceiver == tool;
        }
        
        void ToolView::toggleTool(Tool* tool) {
            if (tool == NULL) {
                if (m_modalReceiver != NULL) {
                    m_modalReceiver->deactivate(m_inputState);
                    m_modalReceiver = NULL;
                }
            } else {
                if (m_modalReceiver == tool) {
                    assert(m_modalReceiver->active());
                    m_modalReceiver->deactivate(m_inputState);
                    m_modalReceiver = NULL;
                } else {
                    if (m_modalReceiver != NULL) {
                        assert(m_modalReceiver->active());
                        m_modalReceiver->deactivate(m_inputState);
                        m_modalReceiver = NULL;
                    }
                    if (tool->activate(m_inputState))
                        m_modalReceiver = tool;
                }
            }
            Refresh();
        }
        
        void ToolView::deactivateAllTools() {
            toggleTool(NULL);
        }
        
        void ToolView::cancelCurrentDrag() {
            if (m_dragReceiver != NULL) {
                m_toolChain->cancelMouseDrag(m_inputState);
                m_inputState.clearMouseButtons();
                m_dragReceiver = NULL;
            }
        }
        
        void ToolView::setRenderOptions(Renderer::RenderContext& renderContext) {
            if (m_toolChain != NULL)
                m_toolChain->setRenderOptions(m_inputState, renderContext);
        }
        
        void ToolView::renderTools(Renderer::RenderContext& renderContext) {
            if (m_modalReceiver != NULL)
                m_modalReceiver->renderOnly(m_inputState, renderContext);
            else if (m_toolChain != NULL)
                m_toolChain->renderChain(m_inputState, renderContext);
        }
        
        void ToolView::updateHits() {
            m_inputState.setPickRay(m_camera.pickRay(m_inputState.mouseX(),  m_inputState.mouseY()));
            
            Hits hits = doGetHits(m_inputState.pickRay());
            m_toolChain->pick(m_inputState, hits);
            m_inputState.setHits(hits);
        }
        
        void ToolView::resetCamera() {
            doResetCamera();
        }
        
        void ToolView::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }
        
        void ToolView::bindEvents() {
            Bind(wxEVT_KEY_DOWN, &ToolView::OnKey, this);
            Bind(wxEVT_KEY_UP, &ToolView::OnKey, this);
            Bind(wxEVT_LEFT_DOWN, &ToolView::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &ToolView::OnMouseButton, this);
            Bind(wxEVT_LEFT_DCLICK, &ToolView::OnMouseDoubleClick, this);
            Bind(wxEVT_RIGHT_DOWN, &ToolView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_UP, &ToolView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_DCLICK, &ToolView::OnMouseDoubleClick, this);
            Bind(wxEVT_MIDDLE_DOWN, &ToolView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_UP, &ToolView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_DCLICK, &ToolView::OnMouseDoubleClick, this);
            Bind(wxEVT_AUX1_DOWN, &ToolView::OnMouseButton, this);
            Bind(wxEVT_AUX1_UP, &ToolView::OnMouseButton, this);
            Bind(wxEVT_AUX1_DCLICK, &ToolView::OnMouseDoubleClick, this);
            Bind(wxEVT_AUX2_DOWN, &ToolView::OnMouseButton, this);
            Bind(wxEVT_AUX2_UP, &ToolView::OnMouseButton, this);
            Bind(wxEVT_AUX2_DCLICK, &ToolView::OnMouseDoubleClick, this);
            Bind(wxEVT_MOTION, &ToolView::OnMouseMotion, this);
            Bind(wxEVT_MOUSEWHEEL, &ToolView::OnMouseWheel, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &ToolView::OnMouseCaptureLost, this);
            Bind(wxEVT_SET_FOCUS, &ToolView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &ToolView::OnKillFocus, this);
            
            Bind(wxEVT_PAINT, &ToolView::OnPaint, this);
            Bind(wxEVT_SIZE, &ToolView::OnSize, this);
        }

        void ToolView::initializeGL() {
            if (SetCurrent(*m_glContext))
                doInitializeGL();
            m_initialized = true;
        }

        void ToolView::updateViewport() {
            const wxSize clientSize = GetClientSize();
            doUpdateViewport(0, 0, clientSize.x, clientSize.y);
        }
        
        void ToolView::render() {
            doRender();
        }

        ModifierKeyState ToolView::modifierKeys() {
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
        
        bool ToolView::updateModifierKeys() {
            const ModifierKeyState keys = modifierKeys();
            if (keys != m_inputState.modifierKeys()) {
                m_inputState.setModifierKeys(keys);
                return true;
            }
            return false;
        }
        
        bool ToolView::clearModifierKeys() {
            if (m_inputState.modifierKeys() != ModifierKeys::MKNone) {
                m_inputState.setModifierKeys(ModifierKeys::MKNone);
                return true;
            }
            return false;
        }
        
        MouseButtonState ToolView::mouseButton(wxMouseEvent& event) {
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
        
        void ToolView::showPopupMenu() {
            doShowPopupMenu();
        }
        
        void ToolView::doInitializeGL() {}
        
        void ToolView::doShowPopupMenu() {}

        Hits ToolView::doGetHits(const Ray3d& pickRay) const {
            return Hits();
        }
    }
}
