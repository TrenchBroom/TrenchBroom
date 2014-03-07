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

#include "BaseMapView.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "View/Animation.h"
#include "View/CameraAnimation.h"
#include "View/ControllerFacade.h"
#include "View/MapDocument.h"
#include "View/Tool.h"

#include <wx/dcclient.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        BaseMapView::BaseMapView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller, Renderer::Camera& camera, const GLAttribs& attribs, const wxGLContext* sharedContext) :
        wxGLCanvas(parent, wxID_ANY, &attribs.front()),
        m_document(document),
        m_controller(controller),
        m_camera(camera),
        m_inputState(m_camera),
        m_glContext(new wxGLContext(this, sharedContext)),
        m_initialized(false),
        m_animationManager(new AnimationManager()),
        m_toolChain(NULL),
        m_dragReceiver(NULL),
        m_modalReceiver(NULL),
        m_dropReceiver(NULL),
        m_savedDropReceiver(NULL),
        m_ignoreNextDrag(false),
        m_ignoreNextClick(false),
        m_lastFrameActivation(wxDateTime::Now()) {
            bindEvents();
            bindObservers();
        }

        BaseMapView::~BaseMapView() {
            unbindObservers();
            m_animationManager->Delete();
            m_animationManager = NULL;
            delete m_glContext;
            m_glContext = NULL;
        }

        bool BaseMapView::dragEnter(const wxCoord x, const wxCoord y, const String& text) {
            assert(m_dropReceiver == NULL);
            
            deactivateAllTools();
            m_inputState.mouseMove(x, y);
            updatePickResults(x, y);
            m_dropReceiver = m_toolChain->dragEnter(m_inputState, text);
            Refresh();
            
            return m_dropReceiver != NULL;
        }
        
        bool BaseMapView::dragMove(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL)
                return false;
            
            m_inputState.mouseMove(x, y);
            updatePickResults(x, y);
            m_dropReceiver->dragMove(m_inputState);
            Refresh();
            
            return true;
        }
        
        void BaseMapView::dragLeave() {
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
        
        bool BaseMapView::dragDrop(const wxCoord x, const wxCoord y, const String& text) {
            if (m_dropReceiver == NULL && m_savedDropReceiver == NULL)
                return false;
            
            if (m_dropReceiver == NULL) {
                m_dropReceiver = m_savedDropReceiver;
                m_dropReceiver->activate(m_inputState); // GTK2 fix: has been deactivated by dragLeave()
                m_dropReceiver->dragEnter(m_inputState, text);
            }
            
            updatePickResults(x, y);
            const bool success = m_dropReceiver->dragDrop(m_inputState);
            m_dropReceiver->deactivate(m_inputState);
            m_dropReceiver = NULL;
            m_savedDropReceiver = NULL;
            Refresh();
            
            return success;
        }
        
        void BaseMapView::OnKey(wxKeyEvent& event) {
            if (updateModifierKeys()) {
                m_movementRestriction.setVerticalRestriction(m_inputState.modifierKeysDown(ModifierKeys::MKAlt));
                updatePickResults(event.GetX(), event.GetY());
                m_toolChain->modifierKeyChange(m_inputState);
            }
            Refresh();
            event.Skip();
        }
        
        void BaseMapView::OnMouseButton(wxMouseEvent& event) {
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
            
            updatePickResults(event.GetX(), event.GetY());
            m_ignoreNextDrag = false;
            
            Refresh();
            event.Skip();
        }
        
        void BaseMapView::OnMouseDoubleClick(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);
            updateModifierKeys();
            
            m_clickPos = event.GetPosition();
            m_inputState.mouseDown(button);
            m_toolChain->mouseDoubleClick(m_inputState);
            m_inputState.mouseUp(button);
            
            updatePickResults(event.GetX(), event.GetY());
            
            Refresh();
            event.Skip();
        }
        
        void BaseMapView::OnMouseMotion(wxMouseEvent& event) {
            updateModifierKeys();
            updatePickResults(event.GetX(), event.GetY());
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
        
        void BaseMapView::OnMouseWheel(wxMouseEvent& event) {
            updateModifierKeys();
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            m_toolChain->scroll(m_inputState);
            
            updatePickResults(event.GetX(), event.GetY());
            Refresh();
            event.Skip();
        }
        
        void BaseMapView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            cancelCurrentDrag();
            Refresh();
            event.Skip();
        }
        
        void BaseMapView::OnSetFocus(wxFocusEvent& event) {
            if (updateModifierKeys())
                m_toolChain->modifierKeyChange(m_inputState);
            Refresh();
            SetCursor(wxCursor(wxCURSOR_ARROW));
            
            // if this focus event happens as a result of a window activation, the don't ignore the next click
            if ((wxDateTime::Now() - m_lastFrameActivation).IsShorterThan(wxTimeSpan(0, 0, 0, 100)))
                m_ignoreNextClick = false;
            
            event.Skip();
        }
        
        void BaseMapView::OnKillFocus(wxFocusEvent& event) {
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
        
        void BaseMapView::OnActivateFrame(wxActivateEvent& event) {
            m_lastFrameActivation = wxDateTime::Now();
        }
        
        void BaseMapView::OnPaint(wxPaintEvent& event) {
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
        
        void BaseMapView::OnSize(wxSizeEvent& event) {
            const wxSize clientSize = GetClientSize();
            const Renderer::Camera::Viewport viewport(0, 0, clientSize.x, clientSize.y);
            m_camera.setViewport(viewport);
            event.Skip();
        }

        void BaseMapView::toggleMovementRestriction() {
            m_movementRestriction.toggleHorizontalRestriction(m_camera);
            Refresh();
        }

        const wxGLContext* BaseMapView::glContext() const {
            return m_glContext;
        }

        void BaseMapView::resetCamera() {
            doResetCamera();
        }
        
        void BaseMapView::animateCamera(const Vec3f& position, const Vec3f& direction, const Vec3f& up, const wxLongLong duration) {
            CameraAnimation* animation = new CameraAnimation(m_camera, position, direction, up, duration);
            m_animationManager->runAnimation(animation, true);
        }

        bool BaseMapView::anyToolActive() const {
            return m_modalReceiver != NULL;
        }
        
        void BaseMapView::addTool(Tool* tool) {
            if (m_toolChain == NULL)
                m_toolChain = tool;
            else
                m_toolChain->appendTool(tool);
        }

        bool BaseMapView::toolActive(const Tool* tool) const {
            return m_modalReceiver == tool;
        }
        
        void BaseMapView::toggleTool(Tool* tool) {
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

        void BaseMapView::deactivateAllTools() {
            toggleTool(NULL);
        }

        void BaseMapView::setRenderOptions(Renderer::RenderContext& renderContext) {
            if (m_toolChain != NULL)
                m_toolChain->setRenderOptions(m_inputState, renderContext);
        }
        
        void BaseMapView::renderTools(Renderer::RenderContext& renderContext) {
            if (m_modalReceiver != NULL)
                m_modalReceiver->renderOnly(m_inputState, renderContext);
            else if (m_toolChain != NULL)
                m_toolChain->renderChain(m_inputState, renderContext);
        }

        void BaseMapView::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &BaseMapView::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &BaseMapView::documentWasNewedOrLoaded);
            document->objectWasAddedNotifier.addObserver(this, &BaseMapView::objectWasAddedOrDidChange);
            document->objectDidChangeNotifier.addObserver(this, &BaseMapView::objectWasAddedOrDidChange);
            document->faceDidChangeNotifier.addObserver(this, &BaseMapView::faceDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &BaseMapView::selectionDidChange);
            document->modsDidChangeNotifier.addObserver(this, &BaseMapView::modsDidChange);

            ControllerSPtr controller = lock(m_controller);
            controller->commandDoneNotifier.addObserver(this, &BaseMapView::commandDoneOrUndone);
            controller->commandUndoneNotifier.addObserver(this, &BaseMapView::commandDoneOrUndone);
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &BaseMapView::preferenceDidChange);

            m_camera.cameraDidChangeNotifier.addObserver(this, &BaseMapView::cameraDidChange);
        }
        
        void BaseMapView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &BaseMapView::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &BaseMapView::documentWasNewedOrLoaded);
                document->objectWasAddedNotifier.removeObserver(this, &BaseMapView::objectWasAddedOrDidChange);
                document->objectDidChangeNotifier.removeObserver(this, &BaseMapView::objectWasAddedOrDidChange);
                document->faceDidChangeNotifier.removeObserver(this, &BaseMapView::faceDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &BaseMapView::selectionDidChange);
                document->modsDidChangeNotifier.removeObserver(this, &BaseMapView::modsDidChange);
            }
            
            if (!expired(m_controller)) {
                ControllerSPtr controller = lock(m_controller);
                controller->commandDoneNotifier.removeObserver(this, &BaseMapView::commandDoneOrUndone);
                controller->commandUndoneNotifier.removeObserver(this, &BaseMapView::commandDoneOrUndone);
            }
            
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &BaseMapView::preferenceDidChange);

            m_camera.cameraDidChangeNotifier.removeObserver(this, &BaseMapView::cameraDidChange);
        }
        
        void BaseMapView::documentWasNewedOrLoaded() {
            resetCamera();
        }

        void BaseMapView::objectWasAddedOrDidChange(Model::Object* object) {
            Refresh();
        }

        void BaseMapView::faceDidChange(Model::BrushFace* face) {
            Refresh();
        }

        void BaseMapView::selectionDidChange(const Model::SelectionResult& result) {
            Refresh();
        }

        void BaseMapView::modsDidChange() {
            Refresh();
        }

        void BaseMapView::commandDoneOrUndone(Controller::Command::Ptr command) {
            const wxMouseState mouseState = wxGetMouseState();
            const wxPoint clientPos = ScreenToClient(mouseState.GetPosition());
            updatePickResults(clientPos.x, clientPos.y);
            Refresh();
        }

        void BaseMapView::preferenceDidChange(const IO::Path& path) {
            Refresh();
        }

        void BaseMapView::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }
        
        void BaseMapView::bindEvents() {
            Bind(wxEVT_KEY_DOWN, &BaseMapView::OnKey, this);
            Bind(wxEVT_KEY_UP, &BaseMapView::OnKey, this);
            Bind(wxEVT_LEFT_DOWN, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_LEFT_DCLICK, &BaseMapView::OnMouseDoubleClick, this);
            Bind(wxEVT_RIGHT_DOWN, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_UP, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_DCLICK, &BaseMapView::OnMouseDoubleClick, this);
            Bind(wxEVT_MIDDLE_DOWN, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_UP, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_DCLICK, &BaseMapView::OnMouseDoubleClick, this);
            Bind(wxEVT_AUX1_DOWN, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_AUX1_UP, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_AUX1_DCLICK, &BaseMapView::OnMouseDoubleClick, this);
            Bind(wxEVT_AUX2_DOWN, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_AUX2_UP, &BaseMapView::OnMouseButton, this);
            Bind(wxEVT_AUX2_DCLICK, &BaseMapView::OnMouseDoubleClick, this);
            Bind(wxEVT_MOTION, &BaseMapView::OnMouseMotion, this);
            Bind(wxEVT_MOUSEWHEEL, &BaseMapView::OnMouseWheel, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &BaseMapView::OnMouseCaptureLost, this);
            Bind(wxEVT_SET_FOCUS, &BaseMapView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &BaseMapView::OnKillFocus, this);
            
            Bind(wxEVT_PAINT, &BaseMapView::OnPaint, this);
            Bind(wxEVT_SIZE, &BaseMapView::OnSize, this);
        }

        void BaseMapView::initializeGL() {
            if (SetCurrent(*m_glContext))
                doInitializeGL();
            m_initialized = true;
        }

        void BaseMapView::render() {
            doRender();
        }

        void BaseMapView::cancelCurrentDrag() {
            if (m_dragReceiver != NULL) {
                m_toolChain->cancelMouseDrag(m_inputState);
                m_inputState.clearMouseButtons();
                m_dragReceiver = NULL;
            }
        }
        
        ModifierKeyState BaseMapView::modifierKeys() {
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
        
        bool BaseMapView::updateModifierKeys() {
            const ModifierKeyState keys = modifierKeys();
            if (keys != m_inputState.modifierKeys()) {
                m_inputState.setModifierKeys(keys);
                return true;
            }
            return false;
        }
        
        bool BaseMapView::clearModifierKeys() {
            if (m_inputState.modifierKeys() != ModifierKeys::MKNone) {
                m_inputState.setModifierKeys(ModifierKeys::MKNone);
                return true;
            }
            return false;
        }
        
        MouseButtonState BaseMapView::mouseButton(wxMouseEvent& event) {
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
        
        void BaseMapView::updatePickResults(const int x, const int y) {
            MapDocumentSPtr document = lock(m_document);
            
            m_inputState.setPickRay(m_camera.pickRay(x, y));
            Model::PickResult pickResult = document->pick(m_inputState.pickRay());
            m_toolChain->pick(m_inputState, pickResult);
            pickResult.sortHits();
            m_inputState.setPickResult(pickResult);
        }

        void BaseMapView::showPopupMenu() {
            doShowPopupMenu();
        }

        void BaseMapView::doInitializeGL() {}
        
        void BaseMapView::doShowPopupMenu() {}
    }
}
