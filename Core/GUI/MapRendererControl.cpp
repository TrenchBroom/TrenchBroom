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


#include "MapRendererControl.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include "Gwen/Structures.h"
#include "Tool.h"

namespace TrenchBroom {
    namespace Gui {
        MapRendererControl::MapRendererControl(Base* parent) : Base(parent) {
            m_camera = new Model::Camera(90, 0.1f, 2000, Vec3f(-32, -32, 32), XAxisPos);
            m_mapInputController = new Controller::MapInputController(*m_camera);
            m_mapRenderer = new Renderer::MapRenderer();
            SetKeyboardInputEnabled(true);
            SetMouseInputEnabled(true);
        }
        
        MapRendererControl::~MapRendererControl() {
            delete m_mapRenderer;
            delete m_mapInputController;
            delete m_camera;
        }
        
        void MapRendererControl::Render(Skin::Base* skin) {
            const Gwen::Rect& bounds = GetBounds();
            
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT | GL_CLIENT_PIXEL_STORE_BIT);
            
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            float vfrustum = tan(m_camera->fov() * M_PI / 360) * 0.75 * m_camera->near();
            float hfrustum = vfrustum * bounds.w / (float)bounds.h;
            glFrustum(-hfrustum, hfrustum, -vfrustum, vfrustum, m_camera->near(), m_camera->far());
            
            Vec3f pos = m_camera->position();
            Vec3f at = pos + m_camera->direction();
            Vec3f up = m_camera->up();
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glViewport(bounds.x, bounds.y, bounds.w, bounds.h);
            gluLookAt(pos.x, pos.y, pos.z, at.x, at.y, at.z, up.x, up.y, up.z);
            
            Renderer::RenderContext context;
            m_mapRenderer->render(context);

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            
            glPopClientAttrib();
            glPopAttrib();
        }

        void MapRendererControl::OnMouseMoved( int x, int y, int deltaX, int deltaY ) {
            m_mapInputController->mouseMoved(x, y, deltaX, deltaY);
        }
        
        bool MapRendererControl::OnMouseWheeled( int iDelta ) {
            m_mapInputController->scrolled(iDelta, 0);
            return true;
        }
        
        void MapRendererControl::OnMouseClickLeft( int x, int y, bool bDown ) {
            Focus();
            if (bDown) m_mapInputController->mouseDown(Controller::MB_LEFT);
            else m_mapInputController->mouseUp(Controller::MB_LEFT);

            // keep receiving mouse events even if the mouse leaves this control
            if (bDown) Gwen::MouseFocus = this;
            else Gwen::MouseFocus = NULL;
        }
        
        void MapRendererControl::OnMouseClickRight( int x, int y, bool bDown ) {
            Focus();
            if (bDown) m_mapInputController->mouseDown(Controller::MB_RIGHT);
            else m_mapInputController->mouseUp(Controller::MB_RIGHT);
            
            // keep receiving mouse events even if the mouse leaves this control
            if (bDown) Gwen::MouseFocus = this;
            else Gwen::MouseFocus = NULL;
        }

        bool MapRendererControl::OnKeyPress(int iKey, bool bPress) {
            switch (iKey) {
                case Gwen::Key::Shift:
                    m_mapInputController->modifierKeyDown(Controller::MK_SHIFT);
                    return true;
                case Gwen::Key::Control:
                    m_mapInputController->modifierKeyDown(Controller::MK_CTRL);
                    return true;
                case Gwen::Key::Alt:
                    m_mapInputController->modifierKeyDown(Controller::MK_ALT);
                    return true;
                case Gwen::Key::Command:
                    m_mapInputController->modifierKeyDown(Controller::MK_CMD);
                    return true;
                default:
                    return false;
            }
        }
        
        bool MapRendererControl::OnKeyRelease(int iKey) {
            switch (iKey) {
                case Gwen::Key::Shift:
                    m_mapInputController->modifierKeyUp(Controller::MK_SHIFT);
                    return true;
                case Gwen::Key::Control:
                    m_mapInputController->modifierKeyUp(Controller::MK_CTRL);
                    return true;
                case Gwen::Key::Alt:
                    m_mapInputController->modifierKeyUp(Controller::MK_ALT);
                    return true;
                case Gwen::Key::Command:
                    m_mapInputController->modifierKeyUp(Controller::MK_CMD);
                    return true;
                default:
                    return false;
            }
        }
    }
}