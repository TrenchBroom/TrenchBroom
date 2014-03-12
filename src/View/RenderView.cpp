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

#include "RenderView.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"

#include <wx/dcclient.h>
#include <wx/settings.h>

namespace TrenchBroom {
    namespace View {
        RenderView::RenderView(wxWindow* parent, const GLAttribs& attribs, const wxGLContext* sharedContext) :
        wxGLCanvas(parent, wxID_ANY, &attribs.front()),
        m_glContext(new wxGLContext(this, sharedContext)),
        m_initialized(false),
        m_vbo(0xFFF) {
            const wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
            const float r = static_cast<float>(color.Red()) / 0xFF;
            const float g = static_cast<float>(color.Green()) / 0xFF;
            const float b = static_cast<float>(color.Blue()) / 0xFF;
            const float a = 1.0f;
            m_focusColor = Color(r, g, b, a);

            bindEvents();
        }
        
        RenderView::~RenderView() {
            delete m_glContext;
            m_glContext = NULL;
        }
        
        void RenderView::OnPaint(wxPaintEvent& event) {
            if (!IsShownOnScreen())
                return;
            
            if (!m_initialized)
                initializeGL();
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);
                render();
                SwapBuffers();
            }
        }
        
        void RenderView::OnSize(wxSizeEvent& event) {
            updateViewport();
            event.Skip();
        }

        void RenderView::OnSetFocus(wxFocusEvent& event) {
            Refresh();
            event.Skip();
        }
        
        void RenderView::OnKillFocus(wxFocusEvent& event) {
            Refresh();
            event.Skip();
        }

        const wxGLContext* RenderView::glContext() const {
            return m_glContext;
        }
        
        void RenderView::bindEvents() {
            Bind(wxEVT_PAINT, &RenderView::OnPaint, this);
            Bind(wxEVT_SIZE, &RenderView::OnSize, this);
            Bind(wxEVT_SET_FOCUS, &RenderView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &RenderView::OnKillFocus, this);
        }

        void RenderView::initializeGL() {
            if (SetCurrent(*m_glContext))
                doInitializeGL();
            m_initialized = true;
        }

        void RenderView::updateViewport() {
            const wxSize clientSize = GetClientSize();
            doUpdateViewport(0, 0, clientSize.x, clientSize.y);
        }
        
        void RenderView::render() {
            clearBackground();
            doRender();
            renderFocusRect();
        }
        
        void RenderView::clearBackground() {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.get(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void RenderView::renderFocusRect() {
            if (!HasFocus())
                return;
            
            const Color& outer = m_focusColor;
            const Color inner(m_focusColor, 0.7f);

            const wxSize clientSize = GetClientSize();
            const float w = static_cast<float>(clientSize.x);
            const float h = static_cast<float>(clientSize.y);
            const float t = 3.0f;
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices(16);
            
            // top
            vertices[ 0] = Vertex(Vec3f(0.0f, 0.0f, 0.0f), outer);
            vertices[ 1] = Vertex(Vec3f(w, 0.0f, 0.0f), outer);
            vertices[ 2] = Vertex(Vec3f(w-t, t, 0.0f), inner);
            vertices[ 3] = Vertex(Vec3f(t, t, 0.0f), inner);
            
            // right
            vertices[ 4] = Vertex(Vec3f(w, 0.0f, 0.0f), outer);
            vertices[ 5] = Vertex(Vec3f(w, h, 0.0f), outer);
            vertices[ 6] = Vertex(Vec3f(w-t, h-t, 0.0f), inner);
            vertices[ 7] = Vertex(Vec3f(w-t, t, 0.0f), inner);
            
            // bottom
            vertices[ 8] = Vertex(Vec3f(w, h, 0.0f), outer);
            vertices[ 9] = Vertex(Vec3f(0.0f, h, 0.0f), outer);
            vertices[10] = Vertex(Vec3f(t, h-t, 0.0f), inner);
            vertices[11] = Vertex(Vec3f(w-t, h-t, 0.0f), inner);
            
            // left
            vertices[12] = Vertex(Vec3f(0.0f, h, 0.0f), outer);
            vertices[13] = Vertex(Vec3f(0.0f, 0.0f, 0.0f), outer);
            vertices[14] = Vertex(Vec3f(t, t, 0.0f), inner);
            vertices[15] = Vertex(Vec3f(t, h-t, 0.0f), inner);
            
            glViewport(0, 0, w, h);

            const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f, 0.0f, 0.0f, w, h);
            Renderer::Transformation transformation(projection, Mat4x4f::Identity);
            
            glDisable(GL_DEPTH_TEST);
            Renderer::VertexArray array = Renderer::VertexArray::swap(GL_QUADS, vertices);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            array.prepare(m_vbo);
            setVboState.active();
            array.render();
            glEnable(GL_DEPTH_TEST);
        }

        void RenderView::doInitializeGL() {}

        void RenderView::doUpdateViewport(const int x, const int y, const int width, const int height) {}
    }
}
