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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
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
#include "View/ClipTool.h"
#include "View/CreateBrushTool.h"
#include "View/SelectionTool.h"
#include "View/MapDocument.h"

#include <wx/dcclient.h>
#include <wx/settings.h>
#include <wx/timer.h>

namespace TrenchBroom {
    namespace View {
        MapView::MapView(wxWindow* parent, Logger* logger, View::MapDocumentPtr document, ControllerFacade& controller) :
        wxGLCanvas(parent, wxID_ANY, &attribs().front()),
        m_logger(logger),
        m_initialized(false),
        m_glContext(new wxGLContext(this)),
        m_document(document),
        m_controller(controller),
        m_renderResources(attribs(), m_glContext),
        m_renderer(m_document, m_renderResources.fontManager()),
        m_auxVbo(0xFFF),
        m_inputState(m_camera),
        m_cameraTool(NULL),
        m_clipTool(NULL),
        m_createBrushTool(NULL),
        m_selectionTool(NULL),
        m_toolChain(NULL),
        m_dragReceiver(NULL),
        m_modalReceiver(NULL),
        m_ignoreNextClick(false) {
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

        void MapView::toggleClipTool() {
            toggleTool(m_clipTool);
        }
        
        bool MapView::clipToolActive() const {
            return m_modalReceiver == m_clipTool;
        }

        void MapView::OnKey(wxKeyEvent& event) {
            updateModifierKeys();
            updatePickResults(event.GetX(), event.GetY());
            event.Skip();
        }
        
        void MapView::OnMouseButton(wxMouseEvent& event) {
            const MouseButtonState button = mouseButton(event);

            if (m_ignoreNextClick /* && button == MouseButtons::MBLeft */) {
                if (event.ButtonUp())
                    m_ignoreNextClick = false;
                event.Skip();
                return;
            }
            
            if (event.ButtonDown()) {
                CaptureMouse();
                m_clickPos = event.GetPosition();
                m_inputState.mouseDown(button);
                m_toolChain->mouseDown(m_inputState);
            } else {
                if (m_dragReceiver != NULL) {
                    m_dragReceiver->endMouseDrag(m_inputState);
                    m_dragReceiver = NULL;
                } else {
                    m_toolChain->mouseUp(m_inputState);
                }
                m_inputState.mouseUp(button);
                if (GetCapture() == this)
                    ReleaseMouse();
            }

            updatePickResults(event.GetX(), event.GetY());
            Refresh();
            event.Skip();
        }
        
        void MapView::OnMouseMotion(wxMouseEvent& event) {
            updatePickResults(event.GetX(), event.GetY());
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
                    m_dragReceiver->mouseDrag(m_inputState);
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

            updatePickResults(event.GetX(), event.GetY());
            Refresh();
            event.Skip();
        }

        void MapView::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
            cancelCurrentDrag();
            Refresh();
            event.Skip();
        }

        void MapView::OnSetFocus(wxFocusEvent& event) {
            updateModifierKeys();
            Refresh();
            event.Skip();
        }
        
        void MapView::OnKillFocus(wxFocusEvent& event) {
            cancelCurrentDrag();
            if (GetCapture() == this)
                ReleaseMouse();
            clearModifierKeys();
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

        void MapView::updatePickResults(const int x, const int y) {
            m_inputState.setPickRay(m_camera.pickRay(x, y));
            Model::PickResult pickResult = m_document->pick(m_inputState.pickRay());
            m_toolChain->pick(m_inputState, pickResult);
            pickResult.sortHits();
            m_inputState.setPickResult(pickResult);
        }

        void MapView::createTools() {
            m_selectionTool = new SelectionTool(NULL, m_document, m_controller);
            m_createBrushTool = new CreateBrushTool(m_selectionTool, m_document, m_controller);
            m_clipTool = new ClipTool(m_createBrushTool, m_document, m_controller, m_camera);
            m_cameraTool = new CameraTool(m_clipTool, m_document, m_controller, m_camera);
            m_toolChain = m_cameraTool;
        }
        
        void MapView::deleteTools() {
            m_toolChain = NULL;
            delete m_cameraTool;
            m_cameraTool = NULL;
            delete m_clipTool;
            m_clipTool = NULL;
            delete m_createBrushTool;
            m_createBrushTool = NULL;
            delete m_selectionTool;
            m_selectionTool = NULL;
        }

        void MapView::toggleTool(BaseTool* tool) {
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
                if (m_clipTool->activate(m_inputState))
                    m_modalReceiver = m_clipTool;
            }
            Refresh();
        }
        
        void MapView::cancelCurrentDrag() {
            if (m_dragReceiver != NULL) {
                m_toolChain->cancelMouseDrag(m_inputState);
                m_inputState.clearMouseButtons();
                m_dragReceiver = NULL;
            }
        }

        ModifierKeyState MapView::modifierKeys() {
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
        
        void MapView::updateModifierKeys() {
            const ModifierKeyState keys = modifierKeys();
            if (keys != m_inputState.modifierKeys()) {
                m_inputState.setModifierKeys(keys);
                m_toolChain->modifierKeyChange(m_inputState);
            }
        }
        
        void MapView::clearModifierKeys() {
            if (m_inputState.modifierKeys() != ModifierKeys::MKNone) {
                m_inputState.setModifierKeys(ModifierKeys::MKNone);
                m_toolChain->modifierKeyChange(m_inputState);
            }
        }

        MouseButtonState MapView::mouseButton(wxMouseEvent& event) {
            if (event.LeftDown() || event.LeftUp())
                return MouseButtons::MBLeft;
            if (event.MiddleDown() || event.MiddleUp())
                return MouseButtons::MBMiddle;
            if (event.RightDown() || event.RightUp())
                return MouseButtons::MBRight;
            return MouseButtons::MBNone;
        }

        void MapView::bindEvents() {
            Bind(wxEVT_KEY_DOWN, &MapView::OnKey, this);
            Bind(wxEVT_KEY_UP, &MapView::OnKey, this);
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
            const float axisLength = prefs.getFloat(Preferences::AxisLength);
            const Color& xAxisColor = prefs.getColor(Preferences::XAxisColor);
            const Color& yAxisColor = prefs.getColor(Preferences::YAxisColor);
            const Color& zAxisColor = prefs.getColor(Preferences::ZAxisColor);
            
            typedef Renderer::VertexSpecs::P3C4::Vertex Vertex;
            Vertex::List vertices(6);
            
            vertices[0] = Vertex(Vec3f(-axisLength,        0.0f,        0.0f), xAxisColor);
            vertices[1] = Vertex(Vec3f( axisLength,        0.0f,        0.0f), xAxisColor);
            vertices[2] = Vertex(Vec3f(       0.0f, -axisLength,        0.0f), yAxisColor);
            vertices[3] = Vertex(Vec3f(       0.0f,  axisLength,        0.0f), yAxisColor);
            vertices[4] = Vertex(Vec3f(       0.0f,        0.0f, -axisLength), zAxisColor);
            vertices[5] = Vertex(Vec3f(       0.0f,        0.0f,  axisLength), zAxisColor);
            
            Renderer::SetVboState setVboState(m_auxVbo);
            setVboState.active();

            Renderer::VertexArray array(m_auxVbo, GL_LINES, vertices);
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
            
            const Mat4x4f projection = orthoMatrix(-1.0f, 1.0f, 0.0f, 0.0f, w, h);
            Renderer::ReplaceTransformation ortho(context.transformation(), projection, Mat4x4f::Identity);
            
            Renderer::SetVboState setVboState(m_auxVbo);
            setVboState.active();
            
            glDisable(GL_DEPTH_TEST);
            Renderer::VertexArray array(m_auxVbo, GL_QUADS, vertices);
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
                    m_logger->error("Error initializing glew: %s", glewGetErrorString(glewState));
#endif
            } else {
                m_logger->info("Cannot set current GL context");
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
