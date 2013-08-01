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
#include "Preferences.h"
#include "Renderer/RenderContext.h"
#include "View/CameraTool.h"
#include "View/SelectionTool.h"
#include "View/Logger.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>

namespace TrenchBroom {
    namespace View {
        MapView::MapView(wxWindow* parent, Logger* logger, View::MapDocumentPtr document, Controller::ControllerFacade& controller) :
        wxGLCanvas(parent, wxID_ANY, attribs()),
        m_logger(logger),
        m_initialized(false),
        m_glContext(new wxGLContext(this)),
        m_document(document),
        m_controller(controller),
        m_renderer(m_fontManager, document->filter()),
        m_cameraTool(NULL),
        m_toolChain(NULL),
        m_dragReceiver(NULL) {
            m_camera.setDirection(Vec3f(-1.0f, -1.0f, -0.65f).normalized(), Vec3f::PosZ);
            m_camera.moveTo(Vec3f(160.0f, 160.0f, 48.0f));

            createTools();
            bindEvents();
        }
        
        MapView::~MapView() {
            deleteTools();
            delete m_glContext;
            m_glContext = NULL;
            m_logger = NULL;
        }
        
        void MapView::OnMouseButton(wxMouseEvent& event) {
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
        }
        
        void MapView::OnMouseWheel(wxMouseEvent& event) {
            const float delta = static_cast<float>(event.GetWheelRotation()) / event.GetWheelDelta() * event.GetLinesPerAction();
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputState.scroll(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputState.scroll(0.0f, delta);
            m_toolChain->scroll(m_inputState);
            Refresh();
        }

        void MapView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            if (m_dragReceiver != NULL) {
                m_toolChain->cancelMouseDrag(m_inputState);
                m_inputState.clearMouseButtons();
                m_dragReceiver = NULL;
            }
            Refresh();
        }

        void MapView::OnPaint(wxPaintEvent& event) {
#ifndef TESTING
            if (!IsShownOnScreen())
                return;
            
            if (!m_initialized)
                initializeGL();
            
            if (SetCurrent(*m_glContext)) {
                wxPaintDC(this);

                m_document->commitPendingRenderStateChanges();
                { // new block to make sure that the render context is destroyed before SwapBuffers is called
                    Renderer::RenderContext context(m_camera, m_shaderManager);
                    m_renderer.render(context);
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
        }
        
        void MapView::commandDoFailed(Controller::Command::Ptr command) {
        }
        
        void MapView::commandUndo(Controller::Command::Ptr command) {
        }

        void MapView::commandUndone(Controller::Command::Ptr command) {
            m_renderer.commandUndone(command);
        }

        void MapView::commandUndoFailed(Controller::Command::Ptr command) {
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
            
            Bind(wxEVT_PAINT, &MapView::OnPaint, this);
            Bind(wxEVT_SIZE, &MapView::OnSize, this);
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
        
        const int* MapView::attribs() {
            typedef std::vector<int> Attribs;

            static bool initialized = false;
            static Attribs attribs;
            if (initialized)
                return &attribs[0];

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
            return &attribs[0];
        }

        int MapView::depthBits() {
            return attribs()[3];
        }
        
        bool MapView::multisample() {
            return attribs()[4] != 0;
        }
    }
}
