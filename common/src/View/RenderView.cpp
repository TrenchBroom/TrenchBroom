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

#include "RenderView.h"
#include "Exceptions.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/GLContextManager.h"
#include "View/InputEvent.h"
#include "View/wxUtils.h"
#include "TrenchBroomApp.h"

#include <wx/dcclient.h>
#include <wx/settings.h>

#ifdef _WIN32
#include <GL/wglew.h>
#endif

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include <iostream>

namespace TrenchBroom {
    namespace View {
        RenderView::RenderView(wxWindow* parent, GLContextManager& contextManager, wxGLAttributes attribs) :
        wxGLCanvas(parent, attribs, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE),
        m_glContext(contextManager.createContext(this)),
        m_attribs(attribs),
        m_initialized(false) {
            const wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
            m_focusColor = fromWxColor(color);

            bindEvents();
        }
        
        RenderView::~RenderView() {}

        void RenderView::OnKey(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            m_eventRecorder.recordEvent(event);
            event.Skip();
            Refresh();
        }

        void RenderView::OnMouse(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            if (event.ButtonDown(wxMOUSE_BTN_ANY) && !HasCapture()) {
                CaptureMouse();
            } else if (event.ButtonUp(wxMOUSE_BTN_ANY) && HasCapture()) {
                if (!event.LeftIsDown() && !event.MiddleIsDown() && !event.RightIsDown() && !event.Aux1IsDown() && !event.Aux2IsDown()) {
                    ReleaseMouse();
                }
            }

            m_eventRecorder.recordEvent(event);
            event.Skip();
            Refresh();
        }

        void RenderView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            if (IsBeingDeleted()) return;

            m_eventRecorder.recordEvent(event);
            event.Skip();
            Refresh();
        }

        // to prevent flickering, see https://wiki.wxwidgets.org/Flicker-Free_Drawing
        void RenderView::OnEraseBackground(wxEraseEvent& event) {}

        void RenderView::OnPaint(wxPaintEvent& event) {
            if (IsBeingDeleted()) return;
            if (TrenchBroom::View::isReportingCrash()) return;

            if (m_glContext->SetCurrent(this)) {
                if (!m_initialized) {
                    initializeGL();
                }

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

            event.Skip();
            Refresh();
        }
        
        void RenderView::OnKillFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

           event.Skip();
           Refresh();
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
            return GLAttribs::depth();
        }
        
        bool RenderView::multisample() const {
            return GLAttribs::multisample();
        }

        void RenderView::bindEvents() {
            Bind(wxEVT_ERASE_BACKGROUND, &RenderView::OnEraseBackground, this);
            Bind(wxEVT_PAINT, &RenderView::OnPaint, this);
            Bind(wxEVT_SIZE, &RenderView::OnSize, this);
            Bind(wxEVT_SET_FOCUS, &RenderView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &RenderView::OnKillFocus, this);
            Bind(wxEVT_KEY_DOWN, &RenderView::OnKey, this);
            Bind(wxEVT_KEY_UP, &RenderView::OnKey, this);
            Bind(wxEVT_LEFT_DOWN, &RenderView::OnMouse, this);
            Bind(wxEVT_LEFT_UP, &RenderView::OnMouse, this);
            Bind(wxEVT_LEFT_DCLICK, &RenderView::OnMouse, this);
            Bind(wxEVT_RIGHT_DOWN, &RenderView::OnMouse, this);
            Bind(wxEVT_RIGHT_UP, &RenderView::OnMouse, this);
            Bind(wxEVT_RIGHT_DCLICK, &RenderView::OnMouse, this);
            Bind(wxEVT_MIDDLE_DOWN, &RenderView::OnMouse, this);
            Bind(wxEVT_MIDDLE_UP, &RenderView::OnMouse, this);
            Bind(wxEVT_MIDDLE_DCLICK, &RenderView::OnMouse, this);
            Bind(wxEVT_AUX1_DOWN, &RenderView::OnMouse, this);
            Bind(wxEVT_AUX1_UP, &RenderView::OnMouse, this);
            Bind(wxEVT_AUX1_DCLICK, &RenderView::OnMouse, this);
            Bind(wxEVT_AUX2_DOWN, &RenderView::OnMouse, this);
            Bind(wxEVT_AUX2_UP, &RenderView::OnMouse, this);
            Bind(wxEVT_AUX2_DCLICK, &RenderView::OnMouse, this);
            Bind(wxEVT_MOTION, &RenderView::OnMouse, this);
            Bind(wxEVT_MOUSEWHEEL, &RenderView::OnMouse, this);
            Bind(wxEVT_MOUSE_CAPTURE_LOST, &RenderView::OnMouseCaptureLost, this);
        }

        void RenderView::initializeGL() {
            const bool firstInitialization = m_glContext->initialize();
            doInitializeGL(firstInitialization);
            
#ifdef _WIN32
            if (wglSwapIntervalEXT) {
                wglSwapIntervalEXT(1);
            }
#endif

            m_initialized = true;
        }

        void RenderView::updateViewport() {
            const wxSize clientSize = GetClientSize();
            doUpdateViewport(0, 0, clientSize.x, clientSize.y);
        }
        
        void RenderView::render() {
            processInput();
            clearBackground();
            doRender();
            renderFocusIndicator();
        }

        void RenderView::processInput() {
            m_eventRecorder.processEvents(*this);
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
            const auto w = static_cast<float>(clientSize.x);
            const auto h = static_cast<float>(clientSize.y);
            const auto t = 1.0f;
            
            using Vertex = Renderer::VertexSpecs::P3C4::Vertex;
            Vertex::List vertices(16);
            
            // top
            vertices[ 0] = Vertex(vm::vec3f(0.0f, 0.0f, 0.0f), outer);
            vertices[ 1] = Vertex(vm::vec3f(w, 0.0f, 0.0f), outer);
            vertices[ 2] = Vertex(vm::vec3f(w-t, t, 0.0f), inner);
            vertices[ 3] = Vertex(vm::vec3f(t, t, 0.0f), inner);
            
            // right
            vertices[ 4] = Vertex(vm::vec3f(w, 0.0f, 0.0f), outer);
            vertices[ 5] = Vertex(vm::vec3f(w, h, 0.0f), outer);
            vertices[ 6] = Vertex(vm::vec3f(w-t, h-t, 0.0f), inner);
            vertices[ 7] = Vertex(vm::vec3f(w-t, t, 0.0f), inner);
            
            // bottom
            vertices[ 8] = Vertex(vm::vec3f(w, h, 0.0f), outer);
            vertices[ 9] = Vertex(vm::vec3f(0.0f, h, 0.0f), outer);
            vertices[10] = Vertex(vm::vec3f(t, h-t, 0.0f), inner);
            vertices[11] = Vertex(vm::vec3f(w-t, h-t, 0.0f), inner);
            
            // left
            vertices[12] = Vertex(vm::vec3f(0.0f, h, 0.0f), outer);
            vertices[13] = Vertex(vm::vec3f(0.0f, 0.0f, 0.0f), outer);
            vertices[14] = Vertex(vm::vec3f(t, t, 0.0f), inner);
            vertices[15] = Vertex(vm::vec3f(t, h-t, 0.0f), inner);
            
            glAssert(glViewport(0, 0, clientSize.x, clientSize.y));

            const auto projection = vm::orthoMatrix(-1.0f, 1.0f, 0.0f, 0.0f, w, h);
            Renderer::Transformation transformation(projection, vm::mat4x4f::identity);
            
            glAssert(glDisable(GL_DEPTH_TEST));
            auto array = Renderer::VertexArray::swap(vertices);
            
            Renderer::ActivateVbo activate(vertexVbo());
            array.prepare(vertexVbo());
            array.render(GL_QUADS);
            glAssert(glEnable(GL_DEPTH_TEST));
        }

        void RenderView::doInitializeGL(const bool firstInitialization) {}

        void RenderView::doUpdateViewport(const int x, const int y, const int width, const int height) {}
    }
}
