/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "MapGLCanvas.h"

#include "Controller/CameraEvent.h"
#include "Controller/Input.h"
#include "Controller/InputController.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "Renderer/RenderContext.h"
#include "Model/Filter.h"
#include "Utility/Console.h"
#include "Utility/GLee.h"
#include "Utility/Preferences.h"
#include "Utility/VecMath.h"

#include <wx/wx.h>

#include <cassert>

using namespace TrenchBroom::Math;
using namespace TrenchBroom::Controller::ModifierKeys;
using namespace TrenchBroom::Controller::MouseButtons;

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(MapGLCanvas, wxGLCanvas)
        EVT_PAINT(MapGLCanvas::OnPaint)
        EVT_KEY_DOWN(MapGLCanvas::OnKeyDown)
        EVT_KEY_UP(MapGLCanvas::OnKeyUp)
        EVT_LEFT_DOWN(MapGLCanvas::OnMouseLeftDown)
        EVT_LEFT_UP(MapGLCanvas::OnMouseLeftUp)
        EVT_RIGHT_DOWN(MapGLCanvas::OnMouseRightDown)
        EVT_RIGHT_UP(MapGLCanvas::OnMouseRightUp)
        EVT_MIDDLE_DOWN(MapGLCanvas::OnMouseMiddleDown)
        EVT_MIDDLE_UP(MapGLCanvas::OnMouseMiddleUp)
        EVT_MOTION(MapGLCanvas::OnMouseMove)
        EVT_MOUSEWHEEL(MapGLCanvas::OnMouseWheel)
        END_EVENT_TABLE()
        
        int* MapGLCanvas::Attribs() {
            // Todo: make multisample and depth size configurable through prefs
            
            m_attribs = new int[10];
            m_attribs[0] = WX_GL_RGBA;
            m_attribs[1] = WX_GL_DOUBLEBUFFER;
            m_attribs[2] = WX_GL_SAMPLE_BUFFERS;
            m_attribs[3] = GL_TRUE;
            m_attribs[4] = WX_GL_SAMPLES;
            m_attribs[5] = 4;
            m_attribs[6] = WX_GL_DEPTH_SIZE;
            m_attribs[7] = 32;
            m_attribs[8] = 0;
            m_attribs[9] = 0;
            
            return m_attribs;
        }
        
        bool MapGLCanvas::HandleModifierKey(int keyCode, bool down) {
            Controller::ModifierKeyState key;
            switch (keyCode) {
                case WXK_SHIFT:
                    key = Controller::ModifierKeys::Shift;
                    break;
                case WXK_ALT:
                    key = Controller::ModifierKeys::Alt;
                    break;
                case WXK_CONTROL:
                    key = Controller::ModifierKeys::CtrlCmd;
                    break;
                default:
                    key = Controller::ModifierKeys::None;
                    break;
            }
            
            if (key != Controller::ModifierKeys::None) {
                if (down)
                    m_inputController->modifierKeyDown(key);
                else
                    m_inputController->modifierKeyUp(key);
                return true;
            }
            
            return false;
        }

        MapGLCanvas::MapGLCanvas(wxWindow* parent, Model::MapDocument& document, View::EditorView& view) :
        wxGLCanvas(parent, wxID_ANY, Attribs()),
        m_firstFrame(true),
        m_view(view) {
            m_inputController = new Controller::InputController(document, view);
            m_glContext = new wxGLContext(this);
            delete m_attribs;
            m_attribs = NULL;
        }
        
        MapGLCanvas::~MapGLCanvas() {
            if (m_inputController != NULL) {
                delete m_inputController;
                m_inputController = NULL;
            }
            if (m_glContext != NULL) {
                wxDELETE(m_glContext);
                m_glContext = NULL;
            }
            if (m_attribs != NULL) {
                delete m_attribs;
                m_attribs = NULL;
            }
        }
        
        void MapGLCanvas::OnPaint(wxPaintEvent& event) {
            wxPaintDC(this);
			if (SetCurrent(*m_glContext)) {
				if (m_firstFrame) {
					m_firstFrame = false;
					const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
					const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
					const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
                    m_view.Console().info("Created OpenGL context");
					m_view.Console().info("Renderer info: %s version %s from %s", renderer, version, vendor);
				}
            
				Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
				const Color& backgroundColor = prefs.getColor(Preferences::BackgroundColor);
				glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
				glDisableClientState(GL_VERTEX_ARRAY);
				glDisableClientState(GL_COLOR_ARRAY);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDisable(GL_TEXTURE_2D);
            
				m_view.Camera().update(0.0f, 0.0f, GetSize().x, GetSize().y);
            
				Renderer::RenderContext renderContext(m_view.Camera(), m_view.Filter());
				m_view.Renderer().render(renderContext);
            
				SwapBuffers();
			} else {
				m_view.Console().error("Unable to set current OpenGL context");
			}
        }
        
        void MapGLCanvas::OnKeyDown(wxKeyEvent& event) {
            HandleModifierKey(event.GetKeyCode(), true);
        }
        
        void MapGLCanvas::OnKeyUp(wxKeyEvent& event) {
            HandleModifierKey(event.GetKeyCode(), false);
        }
        
        void MapGLCanvas::OnMouseLeftDown(wxMouseEvent& event) {
			CaptureMouse();
            m_inputController->mouseDown(Left, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseLeftUp(wxMouseEvent& event) {
			if (GetCapture() == this)
				ReleaseMouse();
            m_inputController->mouseUp(Left, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseRightDown(wxMouseEvent& event) {
			CaptureMouse();
            m_inputController->mouseDown(Right, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseRightUp(wxMouseEvent& event) {
			if (GetCapture() == this)
				ReleaseMouse();
            m_inputController->mouseUp(Right, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseMiddleDown(wxMouseEvent& event) {
			CaptureMouse();
            m_inputController->mouseDown(Middle, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseMiddleUp(wxMouseEvent& event) {
			if (GetCapture() == this)
				ReleaseMouse();
            m_inputController->mouseUp(Middle, static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseMove(wxMouseEvent& event) {
            m_inputController->mouseMoved(static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));
        }
        
        void MapGLCanvas::OnMouseWheel(wxMouseEvent& event) {
            int lines = event.GetLinesPerAction();
            float delta = static_cast<float>(event.GetWheelDelta()) / lines;
			if (event.GetWheelRotation() < 0)
				delta *= -1.0f;
            if (event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL)
                m_inputController->scrolled(delta, 0.0f);
            else if (event.GetWheelAxis() == wxMOUSE_WHEEL_VERTICAL)
                m_inputController->scrolled(0.0f, delta);
        }
    }
}