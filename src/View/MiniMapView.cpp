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

#include "MiniMapView.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderResources.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>
#include <wx/settings.h>

namespace TrenchBroom {
    namespace View {
        MiniMapView::MiniMapView(wxWindow* parent, View::MapDocumentWPtr document, Renderer::RenderResources& renderResources) :
        wxGLCanvas(parent, wxID_ANY, &renderResources.glAttribs().front()),
        m_document(document),
        m_renderResources(renderResources),
        m_glContext(new wxGLContext(this, m_renderResources.sharedContext())),
        m_camera(new Renderer::OrthographicCamera()),
        m_renderer(m_document),
        m_auxVbo(0xFF) {
            const wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
            const float r = static_cast<float>(color.Red()) / 0xFF;
            const float g = static_cast<float>(color.Green()) / 0xFF;
            const float b = static_cast<float>(color.Blue()) / 0xFF;
            const float a = 1.0f;
            m_focusColor = Color(r, g, b, a);

            m_camera->setNearPlane(-0xFFFF);
            m_camera->setFarPlane(0xFFFF);
            m_camera->setDirection(Vec3f::NegZ, Vec3f::PosY);
            m_camera->moveTo(Vec3f::Null);
            m_camera->setZoom(0.15f);
            
            SetCursor(wxCursor(wxCURSOR_OPEN_HAND));
            bindEvents();
        }
        
        MiniMapView::~MiniMapView() {
            delete m_camera;
            m_camera = NULL;
        }

        void MiniMapView::setZPosition(const float zPosition) {
            View::MapDocumentSPtr document = lock(m_document);
            const BBox3& worldBounds = document->worldBounds();
            const float maxBounds = static_cast<float>(worldBounds.max.z() - worldBounds.min.z());

            Vec3 cameraPosition = m_camera->position();
            cameraPosition[2] = Math::clamp(zPosition) * maxBounds + worldBounds.min.z();
            m_camera->moveTo(cameraPosition);
            
            Refresh();
        }

        void MiniMapView::OnMouseButton(wxMouseEvent& event) {
            if (event.ButtonDown()) {
                if (!HasCapture())
                    CaptureMouse();
                m_lastPos = event.GetPosition();
                SetCursor(wxCursor(wxCURSOR_CLOSED_HAND));
            } else {
                if (HasCapture())
                    ReleaseMouse();
                SetCursor(wxCursor(wxCURSOR_OPEN_HAND));
            }
        }
        
        void MiniMapView::OnMouseDoubleClick(wxMouseEvent& event) {
        }
        
        void MiniMapView::OnMouseMotion(wxMouseEvent& event) {
            if (HasCapture() && event.LeftIsDown()) {
                const wxPoint currentPos = event.GetPosition();

                const Vec3f lastWorldPos = m_camera->unproject(static_cast<float>(m_lastPos.x),
                                                               static_cast<float>(m_lastPos.y),
                                                               0.0f);
                const Vec3f currentWorldPos = m_camera->unproject(static_cast<float>(currentPos.x),
                                                                  static_cast<float>(currentPos.y),
                                                                  0.0f);
                
                const Vec3f diff(lastWorldPos.x() - currentWorldPos.x(),
                                 lastWorldPos.y() - currentWorldPos.y(),
                                 0.0f);
                m_camera->moveBy(diff);
                m_lastPos = currentPos;
                
                Refresh();
            }
        }
        
        void MiniMapView::OnMouseWheel(wxMouseEvent& event) {
            const wxPoint mousePos = event.GetPosition();
            const Vec3f before = m_camera->unproject(static_cast<float>(mousePos.x),
                                                     static_cast<float>(mousePos.y),
                                                     0.0f);
            
            if (event.GetWheelRotation() > 0)
                m_camera->zoom(1.1f);
            else
                m_camera->zoom(1.0f / 1.1f);

            
            const Vec3f after = m_camera->unproject(static_cast<float>(mousePos.x),
                                                     static_cast<float>(mousePos.y),
                                                     0.0f);

            const Vec3f diff(before.x() - after.x(),
                             before.y() - after.y(),
                             0.0f);
            m_camera->moveBy(diff);
            
            Refresh();
        }
        
        void MiniMapView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            SetCursor(wxCursor(wxCURSOR_OPEN_HAND));
        }

        void MiniMapView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);
                
                { // new block to make sure that the render context is destroyed before SwapBuffers is called
                    Renderer::RenderContext context(*m_camera, m_renderResources.shaderManager(), false, 16);
                    setupGL(context);
                    clearBackground(context);
                    renderMap(context);
                }
                SwapBuffers();
            }
#endif
        }
        
        void MiniMapView::OnSize(wxSizeEvent& event) {
            const wxSize clientSize = GetClientSize();
            const Renderer::Camera::Viewport viewport(0, 0, clientSize.x, clientSize.y);
            m_camera->setViewport(viewport);
            event.Skip();
        }
        
        void MiniMapView::bindEvents() {
            Bind(wxEVT_LEFT_DOWN, &MiniMapView::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &MiniMapView::OnMouseButton, this);
            Bind(wxEVT_MOTION, &MiniMapView::OnMouseMotion, this);
            Bind(wxEVT_MOUSEWHEEL, &MiniMapView::OnMouseWheel, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &MiniMapView::OnMouseCaptureLost, this);
            
            Bind(wxEVT_PAINT, &MiniMapView::OnPaint, this);
            Bind(wxEVT_SIZE, &MiniMapView::OnSize, this);
        }

        void MiniMapView::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }

        void MiniMapView::clearBackground(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.get(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void MiniMapView::renderMap(Renderer::RenderContext& context) {
            m_renderer.render(context);
        }
    }
}
