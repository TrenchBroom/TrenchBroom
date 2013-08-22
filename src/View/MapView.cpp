/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MapView.h"

#include "Exceptions.h"
#include "Logger.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Transformation.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/CameraTool.h"
#include "View/SelectionTool.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>
#include <wx/settings.h>

namespace TrenchBroom {
    namespace View {
        MapView::MapView(wxWindow* parent, Logger* logger, View::MapDocumentPtr document, Controller::ControllerFacade& controller) :
        wxGLCanvas(parent, wxID_ANY, &attribs().front()),
        m_logger(logger),
        m_initialized(false),
        m_glContext(new wxGLContext(this)),
        m_document(document),
        m_controller(controller),
        m_renderResources(attribs(), m_glContext),
        m_renderer(m_renderResources.fontManager(), document->filter()),
        m_auxVbo(0xFFF),
        m_inputState(document->filter(), m_camera),
        m_cameraTool(NULL),
        m_toolChain(NULL),
        m_dragReceiver(NULL),
        m_ignoreNextClick(false){
            m_camera.setDirection(Vec3f(-1.0f, -1.0f, -0.65f).normalized(), Vec3f::PosZ);
            m_camera.moveTo(Vec3f(160.0f, 160.0f, 48.0f));

            const wxColour color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
            const float r = static_cast<float>(color.Red()) / 0xFF;
            const float g = static_cast<float>(color.Green()) / 0xFF;
            const float b = static_cast<float>(color.Blue()) / 0xFF;
            const float a = 1.0f;
            m_focusColor = Color(r, g, b, a);

            createTools();
            bindEvents();
        }
        
        MapView::~MapView() {
            deleteTools();
            delete m_glContext;
            m_glContext = NULL;
            m_logger = NULL;
        }
        
        Renderer::RenderResources& MapView::renderResources() {
            return m_renderResources;
        }

        void MapView::OnMouseButton(wxMouseEvent& event) {
            if (m_ignoreNextClick && (event.LeftDown() || event.LeftUp())) {
                if (event.LeftUp())
                    m_ignoreNextClick = false;
                event.Skip();
                return;
            }
            
            if (event.LeftDown()) {
                CaptureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(MouseButtons::MBLeft);
                m_toolChain->mouseDown(m_inputState);
            } else if (event.LeftUp()) {
                if (m_dragReceiver != NULL) {
                    m_toolChain->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                }
                m_toolChain->mouseUp(m_inputState);
                m_inputState.mouseUp(MouseButtons::MBLeft);
                if (GetCapture() == this)
                    ReleaseMouse();
            } else if (event.MiddleDown()) {
                CaptureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(MouseButtons::MBMiddle);
                m_toolChain->mouseDown(m_inputState);
            } else if (event.MiddleUp()) {
                if (m_dragReceiver != NULL) {
                    m_toolChain->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                }
                m_toolChain->mouseUp(m_inputState);
                m_inputState.mouseUp(MouseButtons::MBMiddle);
                if (GetCapture() == this)
                    ReleaseMouse();
            } else if (event.RightDown()) {
                CaptureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(MouseButtons::MBRight);
                m_toolChain->mouseDown(m_inputState);
            } else if (event.RightUp()) {
                if (m_dragReceiver != NULL) {
                    m_toolChain->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                }
                m_toolChain->mouseUp(m_inputState);
                m_inputState.mouseUp(MouseButtons::MBRight);
                if (GetCapture() == this)
                    ReleaseMouse();
            }
            Refresh();
            event.Skip();
        }
        
        void MapView::OnMouseMotion(wxMouseEvent& event) {
            m_inputState.setPickRay(m_camera.pickRay(event.GetX(), event.GetY()));
            Model::PickResult pickResult = m_document->pick(m_inputState.pickRay());
            m_inputState.setPickResult(pickResult);
            
            if (m_dragReceiver != NULL) {
                m_inputState.mouseMove(event.GetX(), event.GetY());
                m_dragReceiver->mouseDrag(m_inputState);
            } else {
                if (m_inputState.mouseButtons() != MouseButtons::MBNone &&
                    (std::abs(event.GetX() - m_clickPos.x) > 1 ||
                     std::abs(event.GetY() - m_clickPos.y) > 1)) {
                        m_dragReceiver = m_toolChain->startMouseDrag(m_inputState);
                    }
                if (m_dragReceiver != NULL) {
                    m_inputState.mouseMove(event.GetX(), event.GetY());
                    m_toolChain->mouseDrag(m_inputState);
                } else {
                    m_inputState.mouseMove(event.GetX(), event.GetY());
                    m_toolChain->mouseMove(m_inputState);
                }
            }
            Refresh();
            event.Skip();
        }
        
        void MapView::OnMouseWheel(wxMouseEvent& event) {
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            m_toolChain->scroll(m_inputState);
            Refresh();
            event.Skip();
        }

        void MapView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            cancelCurrentDrag();
            Refresh();
            event.Skip();
        }

        void MapView::OnSetFocus(wxFocusEvent& event) {
            Refresh();
            event.Skip();
        }
        
        void MapView::OnKillFocus(wxFocusEvent& event) {
            cancelCurrentDrag();
            if (GetCapture() == this)
                ReleaseMouse();
            m_ignoreNextClick = true;
            Refresh();
            event.Skip();
        }

        void MapView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (!m_initialized)
                initializeGL();
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC paintDC(this);

                m_document->commitPendingRenderStateChanges();
                { // new block to make sure that the render context is destroyed before SwapBuffers is called
                    Renderer::RenderContext context(m_camera, m_renderResources.shaderManager());
                    setupGL(context);
                    clearBackground(context);
                    renderCoordinateSystem(context);
                    renderMap(context);
                    renderTools(context);
                    renderFocusRect(context);
                }
                SwapBuffers();
            }
#endif
        }

        void MapView::OnSize(wxSizeEvent& event) {
            const wxSize clientSize = GetClientSize();
            const Renderer::Camera::Viewport viewport(0, 0, clientSize.x, clientSize.y);
            m_camera.setViewport(viewport);
            event.Skip();
        }
        
        void MapView::commandDo(Controller::Command::Ptr command) {
        }

        void MapView::commandDone(Controller::Command::Ptr command) {
            m_renderer.commandDone(command);
            Refresh();
        }
        
        void MapView::commandDoFailed(Controller::Command::Ptr command) {
            Refresh();
        }
        
        void MapView::commandUndo(Controller::Command::Ptr command) {
        }

        void MapView::commandUndone(Controller::Command::Ptr command) {
            m_renderer.commandUndone(command);
            Refresh();
        }

        void MapView::commandUndoFailed(Controller::Command::Ptr command) {
            Refresh();
        }

        void MapView::createTools() {
            m_cameraTool = new CameraTool(NULL, m_camera);
            m_selectionTool = new SelectionTool(m_cameraTool, m_controller);
            m_toolChain = m_selectionTool;
        }
        
        void MapView::deleteTools() {
            m_toolChain = NULL;
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_selectionTool;
            m_selectionTool = NULL;
        }

        void MapView::cancelCurrentDrag() {
            if (m_dragReceiver != NULL) {
                m_toolChain->cancelMouseDrag(m_inputState);
                m_inputState.clearMouseButtons();
                m_dragReceiver = NULL;
            }
        }

        void MapView::bindEvents() {
            Bind(wxEVT_LEFT_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_LEFT_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_LEFT_DCLICK, &MapView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_RIGHT_DCLICK, &MapView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_MIDDLE_DCLICK, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX1_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX1_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX1_DCLICK, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX2_DOWN, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX2_UP, &MapView::OnMouseButton, this);
            Bind(wxEVT_AUX2_DCLICK, &MapView::OnMouseButton, this);
            Bind(wxEVT_MOTION, &MapView::OnMouseMotion, this);
            Bind(wxEVT_MOUSEWHEEL, &MapView::OnMouseWheel, this);
            Bind(wxEVT_SET_FOCUS, &MapView::OnSetFocus, this);
            Bind(wxEVT_KILL_FOCUS, &MapView::OnKillFocus, this);
            
            Bind(wxEVT_PAINT, &MapView::OnPaint, this);
            Bind(wxEVT_SIZE, &MapView::OnSize, this);
        }
        
        void MapView::setupGL(Renderer::RenderContext& context) {
            const Renderer::Camera::Viewport& viewport = context.camera().viewport();
            glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
            
            glEnable(GL_MULTISAMPLE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glShadeModel(GL_SMOOTH);
        }

        void MapView::clearBackground(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
            glClearColor(backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void MapView::renderCoordinateSystem(Renderer::RenderContext& context) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const Color& xAxisColor = prefs.getColor(Preferences::XAxisColor);
            const Color& yAxisColor = prefs.getColor(Preferences::YAxisColor);
            const Color& zAxisColor = prefs.getColor(Preferences::ZAxisColor);
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            
            vertices.push_back(Vertex(Vec3f(-128.0f, 0.0f, 0.0f), xAxisColor));
            vertices.push_back(Vertex(Vec3f( 128.0f, 0.0f, 0.0f), xAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, -128.0f, 0.0f), yAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f,  128.0f, 0.0f), yAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f, -128.0f), zAxisColor));
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f,  128.0f), zAxisColor));
            
            Renderer::VertexArray array(m_auxVbo, GL_LINES, vertices);
            
            Renderer::SetVboState setVboState(m_auxVbo);
            setVboState.active();
            array.render();
        }
        
        void MapView::renderMap(Renderer::RenderContext& context) {
            m_renderer.render(context);
        }
        
        void MapView::renderTools(Renderer::RenderContext& context) {
            m_toolChain->render(m_inputState, context);
        }

        void MapView::renderFocusRect(Renderer::RenderContext& context) {
            if (!HasFocus())
                return;
            
            const Color& outer = m_focusColor;
            const Color inner(m_focusColor, 0.7f);
            const float w = static_cast<float>(context.camera().viewport().width);
            const float h = static_cast<float>(context.camera().viewport().height);
            const float t = 3.0f;
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices;
            vertices.reserve(16);
            
            // top
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(w, 0.0f, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(w-t, t, 0.0f), inner));
            vertices.push_back(Vertex(Vec3f(t, t, 0.0f), inner));
            
            // right
            vertices.push_back(Vertex(Vec3f(w, 0.0f, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(w, h, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(w-t, h-t, 0.0f), inner));
            vertices.push_back(Vertex(Vec3f(w-t, t, 0.0f), inner));
            
            // bottom
            vertices.push_back(Vertex(Vec3f(w, h, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(0.0f, h, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(t, h-t, 0.0f), inner));
            vertices.push_back(Vertex(Vec3f(w-t, h-t, 0.0f), inner));
            
            // left
            vertices.push_back(Vertex(Vec3f(0.0f, h, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(0.0f, 0.0f, 0.0f), outer));
            vertices.push_back(Vertex(Vec3f(t, t, 0.0f), inner));
            vertices.push_back(Vertex(Vec3f(t, h-t, 0.0f), inner));
            
            const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f, 0.0f, 0.0f, w, h);
            Renderer::ReplaceTransformation ortho(context.transformation(), projection, Mat4x4f::Identity);
            
            Renderer::VertexArray array(m_auxVbo, GL_QUADS, vertices);
            Renderer::SetVboState setVboState(m_auxVbo);
            setVboState.active();
            
            glDisable(GL_DEPTH_TEST);
            array.render();
            glEnable(GL_DEPTH_TEST);
        }

        void MapView::initializeGL() {
            if (SetCurrent(*m_glContext)) {
                const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
                const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
                const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
                m_logger->info("Renderer info: %s version %s from %s", renderer, version, vendor);
                m_logger->info("Depth buffer bits: %d", depthBits());
                
                if (multisample())
                    m_logger->info("Multisampling enabled");
                else
                    m_logger->info("Multisampling disabled");
                
#ifndef TESTING
                glewExperimental = GL_TRUE;
                const GLenum glewState = glewInit();
                if (glewState != GLEW_OK)
                    m_logger->error("Unable to initialize glew: %s", glewGetErrorString(glewState));
#endif
            } else {
                m_logger->info("Unable to set current GL context");
            }
            m_initialized = true;
        }
        
        const Renderer::RenderResources::GLAttribs& MapView::attribs() {
            static bool initialized = false;
            static Renderer::RenderResources::GLAttribs attribs;
            if (initialized)
                return attribs;

            int testAttribs[] =
            {
                // 32 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 24 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 32 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 24 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 16 bit depth buffer, 4 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          4,
                0,
                // 16 bit depth buffer, 2 samples
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                WX_GL_SAMPLE_BUFFERS,   1,
                WX_GL_SAMPLES,          2,
                0,
                // 32 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       32,
                0,
                // 24 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       24,
                0,
                // 16 bit depth buffer, no multisampling
                WX_GL_RGBA,
                WX_GL_DOUBLEBUFFER,
                WX_GL_DEPTH_SIZE,       16,
                0,
                0,
            };

            size_t index = 0;
            while (!initialized && testAttribs[index] != 0) {
                size_t count = 0;
                for (; testAttribs[index + count] != 0; ++count);
                if (wxGLCanvas::IsDisplaySupported(&testAttribs[index])) {
                    for (size_t i = 0; i < count; ++i)
                        attribs.push_back(testAttribs[index + i]);
                    attribs.push_back(0);
                    initialized = true;
                }
                index += count + 1;
            }

            assert(initialized);
            assert(!attribs.empty());
            return attribs;
        }

        int MapView::depthBits() {
            return attribs()[3];
        }
        
        bool MapView::multisample() {
            return attribs()[4] != 0;
        }
    }
}
