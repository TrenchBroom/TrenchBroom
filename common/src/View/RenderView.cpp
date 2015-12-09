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
#include "Exceptions.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/GLContextManager.h"
#include "View/wxUtils.h"

#include <wx/dcclient.h>
#include <wx/settings.h>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        RenderView::RenderView(wxWindow* parent, GLContextManager& contextManager, const GLAttribs& attribs) :
        wxGLCanvas(parent, wxID_ANY, &attribs.front(), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
        m_glContext(contextManager.createContext(this)),
        m_attribs(attribs),
        m_initialized(false) {
            const wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
            m_focusColor = fromWxColor(color);
            
            bindEvents();
        }
        
        void RenderView::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;

            if (m_glContext->SetCurrent(this)) {
                if (!m_initialized)
                    initializeGL();

                wxPaintDC paintDC(this);
                render();
                SwapBuffers();
            }
        }
        
        void RenderView::OnSize(wxSizeEvent& event) {
            if (IsBeingDeleted()) return;

            updateViewport();
            event.Skip();
        }

        void RenderView::OnSetFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            Refresh();
            event.Skip();
        }
        
        void RenderView::OnKillFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            Refresh();
            event.Skip();
        }

        Renderer::Vbo& RenderView::vertexVbo() {
            return m_glContext->vertexVbo();
        }

        Renderer::Vbo& RenderView::indexVbo() {
            return m_glContext->indexVbo();
        }
        
        Renderer::FontManager& RenderView::fontManager() {
            return m_glContext->fontManager();
        }
        
        Renderer::ShaderManager& RenderView::shaderManager() {
            return m_glContext->shaderManager();
        }

        int RenderView::depthBits() const {
            return m_attribs[3];
        }
        
        bool RenderView::multisample() const {
            return m_attribs[4] != 0;
        }

        void RenderView::bindEvents() {
            Bind(wxEVT_PAINT, &RenderView::OnPaint, this);
            Bind(wxEVT_SIZE, &RenderView::OnSize, this);
            Bind(wxEVT_SET_FOCUS, &RenderView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &RenderView::OnKillFocus, this);
        }

        void RenderView::initializeGL() {
            const bool firstInitialization = m_glContext->initialize();
            doInitializeGL(firstInitialization);
            m_initialized = true;
        }

        void RenderView::updateViewport() {
            const wxSize clientSize = GetClientSize();
            doUpdateViewport(0, 0, clientSize.x, clientSize.y);
        }
        
        void RenderView::render() {
            clearBackground();
            doRender();
            renderFocusIndicator();
        }
        
        void RenderView::clearBackground() {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.get(Preferences::BackgroundColor);

            glAssert(glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a()));
            glAssert(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        }

        void RenderView::renderFocusIndicator() {
            if (!doShouldRenderFocusIndicator() || !HasFocus())
                return;
            
            const Color& outer = m_focusColor;
            const Color& inner = m_focusColor;

            const wxSize clientSize = GetClientSize();
            const float w = static_cast<float>(clientSize.x);
            const float h = static_cast<float>(clientSize.y);
            const float t = 1.0f;
            
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
            
            glAssert(glViewport(0, 0, clientSize.x, clientSize.y));

            const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f, 0.0f, 0.0f, w, h);
            Renderer::Transformation transformation(projection, Mat4x4f::Identity);
            
            glAssert(glDisable(GL_DEPTH_TEST));
            Renderer::VertexArray array = Renderer::VertexArray::swap(vertices);
            
            Renderer::ActivateVbo activate(vertexVbo());
            array.prepare(vertexVbo());
            array.render(GL_QUADS);
            glAssert(glEnable(GL_DEPTH_TEST));
        }

        void RenderView::doInitializeGL(const bool firstInitialization) {}

        void RenderView::doUpdateViewport(const int x, const int y, const int width, const int height) {}
    }
}
