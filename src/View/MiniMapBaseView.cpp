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

#include "MiniMapBaseView.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "Renderer/MiniMapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderResources.h"
#include "Renderer/Transformation.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>

DEFINE_EVENT_TYPE(EVT_MINIMAP_VIEW_CHANGED_EVENT)

namespace TrenchBroom {
    namespace View {
        MiniMapBaseView::~MiniMapBaseView() {
            unbindObservers();
            delete m_glContext;
            m_glContext = NULL;
        }
        
        void MiniMapBaseView::OnMouseButton(wxMouseEvent& event) {
            if (event.ButtonDown()) {
                m_lastPos = event.GetPosition();
                if (event.LeftIsDown()) {
                    if (!HasCapture()) {
                        CaptureMouse();
                        doShowDrag3DCameraCursor();
                    }
                } else if (event.RightIsDown()) {
                    if (!HasCapture())
                        CaptureMouse();
                    SetCursor(wxCursor(wxCURSOR_CLOSED_HAND));
                }
            } else {
                if (HasCapture())
                    ReleaseMouse();
                SetCursor(*wxSTANDARD_CURSOR);
            }
        }
        
        void MiniMapBaseView::OnMouseDoubleClick(wxMouseEvent& event) {
            if (event.LeftDClick()) {
                const Vec3f& oldPos = m_camera3D.position();
                const Vec3f newPos = viewCamera().unproject(static_cast<float>(event.GetX()),
                                                            static_cast<float>(event.GetY()),
                                                            0.0f);
                const Vec3f delta = newPos - oldPos;
                doDrag3DCamera(delta, m_camera3D);
                Refresh();
            }
        }
        
        void MiniMapBaseView::OnMouseMotion(wxMouseEvent& event) {
            const wxPoint currentPos = event.GetPosition();
            if (HasCapture()) {
                if (event.LeftIsDown())
                    drag3DCamera(m_lastPos, currentPos);
                else if (event.RightIsDown())
                    panView(m_lastPos, currentPos);
                m_lastPos = currentPos;
            } else {
                const Ray3f pickRay = viewCamera().pickRay(currentPos.x, currentPos.y);
                const float distance = pick3DCamera(pickRay);
                if (Math::isnan(distance))
                    SetCursor(*wxSTANDARD_CURSOR);
                else
                    doShowDrag3DCameraCursor();
            }
        }
        
        void MiniMapBaseView::drag3DCamera(const wxPoint& lastPos, const wxPoint& currentPos) {
            const Vec3f lastWorldPos = viewCamera().unproject(static_cast<float>(lastPos.x),
                                                              static_cast<float>(lastPos.y),
                                                              0.0f);
            const Vec3f currentWorldPos = viewCamera().unproject(static_cast<float>(currentPos.x),
                                                                 static_cast<float>(currentPos.y),
                                                                 0.0f);
            const Vec3f delta = currentWorldPos - lastWorldPos;
            doDrag3DCamera(delta, m_camera3D);
            Refresh();
        }
        
        void MiniMapBaseView::panView(const wxPoint& lastPos, const wxPoint& currentPos) {
            const Vec3f lastWorldPos = viewCamera().unproject(static_cast<float>(lastPos.x),
                                                              static_cast<float>(lastPos.y),
                                                              0.0f);
            const Vec3f currentWorldPos = viewCamera().unproject(static_cast<float>(currentPos.x),
                                                                 static_cast<float>(currentPos.y),
                                                                 0.0f);
            
            panView(lastWorldPos - currentWorldPos);
            Refresh();
        }
        
        
        void MiniMapBaseView::OnMouseWheel(wxMouseEvent& event) {
            const Vec3f oldWorldPos = viewCamera().unproject(static_cast<float>(event.GetX()),
                                                             static_cast<float>(event.GetY()),
                                                             0.0f);
            if (event.GetWheelRotation() > 0)
                zoomView(Vec2f(1.1f, 1.1f));
            else
                zoomView(Vec2f(1.0f, 1.0f) / 1.1f);
            
            const Vec3f newWorldPos = viewCamera().unproject(static_cast<float>(event.GetX()),
                                                             static_cast<float>(event.GetY()),
                                                             0.0f);
            panView(oldWorldPos - newWorldPos);
            Refresh();
        }
        
        void MiniMapBaseView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            SetCursor(wxCursor(wxCURSOR_OPEN_HAND));
        }
        
        void MiniMapBaseView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);
                
                { // new block to make sure that the render context is destroyed before SwapBuffers is called
                    Renderer::RenderContext context(viewCamera(), m_renderResources.shaderManager(), false, 16);
                    setupGL(context);
                    clearBackground(context);
                    renderMap(context);
                    render3DCamera(context);
                }
                SwapBuffers();
            }
#endif
        }
        
        void MiniMapBaseView::OnSize(wxSizeEvent& event) {
            const wxSize clientSize = GetClientSize();
            const Renderer::Camera::Viewport viewport(0, 0, clientSize.x, clientSize.y);
            updateViewport(viewport);
            event.Skip();
        }
        
        MiniMapBaseView::MiniMapBaseView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources, Renderer::MiniMapRenderer& renderer, Renderer::Camera& camera3D) :
        wxGLCanvas(parent, wxID_ANY, &renderResources.glAttribs().front()),
        m_document(document),
        m_renderResources(renderResources),
        m_camera3D(camera3D),
        m_glContext(new wxGLContext(this, m_renderResources.sharedContext())),
        m_renderer(renderer),
        m_auxVbo(0xFF) {
            bindEvents();
            bindObservers();
        }
        
        View::MapDocumentSPtr MiniMapBaseView::document() const {
            assert(!expired(m_document));
            return lock(m_document);
        }
        
        const Renderer::Camera& MiniMapBaseView::viewCamera() const {
            return doGetViewCamera();
        }
        
        void MiniMapBaseView::updateViewport(const Renderer::Camera::Viewport& viewport) {
            doUpdateViewport(viewport);
        }
        
        void MiniMapBaseView::panView(const Vec3f& delta) {
            doPanView(delta);
            fireChangeEvent();
        }
        
        void MiniMapBaseView::zoomView(const Vec3f& factors) {
            doZoomView(factors);
            fireChangeEvent();
        }
        
        void MiniMapBaseView::bindObservers() {
            View::MapDocumentSPtr document = lock(m_document);
            document->documentWasClearedNotifier.addObserver(this, &MiniMapBaseView::documentWasCleared);
            document->documentWasNewedNotifier.addObserver(this, &MiniMapBaseView::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &MiniMapBaseView::documentWasNewedOrLoaded);
            document->objectWasAddedNotifier.addObserver(this, &MiniMapBaseView::objectWasAdded);
            document->objectWillBeRemovedNotifier.addObserver(this, &MiniMapBaseView::objectWillBeRemoved);
            document->objectDidChangeNotifier.addObserver(this, &MiniMapBaseView::objectDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &MiniMapBaseView::selectionDidChange);
            m_camera3D.cameraDidChangeNotifier.addObserver(this, &MiniMapBaseView::cameraDidChange);
        }
        
        void MiniMapBaseView::unbindObservers() {
            if (!expired(m_document)) {
                View::MapDocumentSPtr document = lock(m_document);
                document->documentWasClearedNotifier.removeObserver(this, &MiniMapBaseView::documentWasCleared);
                document->documentWasNewedNotifier.removeObserver(this, &MiniMapBaseView::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &MiniMapBaseView::documentWasNewedOrLoaded);
                document->objectWasAddedNotifier.removeObserver(this, &MiniMapBaseView::objectWasAdded);
                document->objectWillBeRemovedNotifier.removeObserver(this, &MiniMapBaseView::objectWillBeRemoved);
                document->objectDidChangeNotifier.removeObserver(this, &MiniMapBaseView::objectDidChange);
                document->selectionDidChangeNotifier.removeObserver(this, &MiniMapBaseView::selectionDidChange);
            }
            m_camera3D.cameraDidChangeNotifier.removeObserver(this, &MiniMapBaseView::cameraDidChange);
        }
        
        void MiniMapBaseView::documentWasCleared() {
            Refresh();
        }
        
        void MiniMapBaseView::documentWasNewedOrLoaded() {
            Refresh();
        }
        
        void MiniMapBaseView::objectWasAdded(Model::Object* object) {
            Refresh();
        }
        
        void MiniMapBaseView::objectWillBeRemoved(Model::Object* object) {
            Refresh();
        }
        
        void MiniMapBaseView::objectDidChange(Model::Object* object) {
            Refresh();
        }
        
        void MiniMapBaseView::selectionDidChange(const Model::SelectionResult& result) {
            Refresh();
        }
        
        void MiniMapBaseView::cameraDidChange(const Renderer::Camera* camera) {
            Refresh();
        }
        
        void MiniMapBaseView::bindEvents() {
            Bind(wxEVT_LEFT_DOWN, &MiniMapBaseView::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &MiniMapBaseView::OnMouseButton, this);
            Bind(wxEVT_LEFT_DCLICK, &MiniMapBaseView::OnMouseDoubleClick, this);
            Bind(wxEVT_RIGHT_DOWN, &MiniMapBaseView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_UP, &MiniMapBaseView::OnMouseButton, this);
            Bind(wxEVT_MOTION, &MiniMapBaseView::OnMouseMotion, this);
            Bind(wxEVT_MOUSEWHEEL, &MiniMapBaseView::OnMouseWheel, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &MiniMapBaseView::OnMouseCaptureLost, this);
            
            Bind(wxEVT_PAINT, &MiniMapBaseView::OnPaint, this);
            Bind(wxEVT_SIZE, &MiniMapBaseView::OnSize, this);
        }
        
        void MiniMapBaseView::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }
        
        void MiniMapBaseView::clearBackground(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.get(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        
        void MiniMapBaseView::renderMap(Renderer::RenderContext& context) {
            BBox3f bounds;
            doComputeBounds(bounds);
            m_renderer.render(context, bounds);
        }
        
        void MiniMapBaseView::fireChangeEvent() {
            wxCommandEvent event(EVT_MINIMAP_VIEW_CHANGED_EVENT);
            event.SetEventObject(this);
            event.SetId(GetId());
            ProcessEvent(event);
        }
        
        float MiniMapBaseView::pick3DCamera(const Ray3f& pickRay) const {
            return doPick3DCamera(pickRay, m_camera3D);
        }
        
        void MiniMapBaseView::render3DCamera(Renderer::RenderContext& context) {
            doRender3DCamera(context, m_auxVbo, m_camera3D);
        }
    }
}
