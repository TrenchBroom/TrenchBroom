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
#include "MapRenderer.h"
#include "Editor.h"
#include "Tool.h"
#include "InputController.h"
#include "Camera.h"

namespace TrenchBroom {
    namespace Gui {
        MapRendererControl::MapRendererControl(Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            m_mapRenderer = new Renderer::MapRenderer(m_editor);
            SetKeyboardInputEnabled(true);
            SetMouseInputEnabled(true);
        }
        
        MapRendererControl::~MapRendererControl() {
            delete m_mapRenderer;
        }
        
        void MapRendererControl::Render(Skin::Base* skin) {
            const Gwen::Rect& bounds = GetBounds();
            
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT | GL_CLIENT_PIXEL_STORE_BIT);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
            
            Controller::Camera& camera = m_editor.camera();
            camera.update(bounds.x, bounds.y, bounds.w, bounds.h);
            
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
            m_editor.inputController().mouseMoved(x, y, deltaX, deltaY);
        }
        
        bool MapRendererControl::OnMouseWheeled( int iDelta ) {
            m_editor.inputController().scrolled(iDelta, 0);
            return true;
        }
        
        void MapRendererControl::OnMouseClickLeft( int x, int y, bool bDown ) {
            Focus();
            if (bDown) m_editor.inputController().mouseDown(Controller::MB_LEFT);
            else m_editor.inputController().mouseUp(Controller::MB_LEFT);

            // keep receiving mouse events even if the mouse leaves this control
            if (bDown) Gwen::MouseFocus = this;
            else Gwen::MouseFocus = NULL;
        }
        
        void MapRendererControl::OnMouseClickRight( int x, int y, bool bDown ) {
            Focus();
            if (bDown)m_editor.inputController().mouseDown(Controller::MB_RIGHT);
            else m_editor.inputController().mouseUp(Controller::MB_RIGHT);
            
            // keep receiving mouse events even if the mouse leaves this control
            if (bDown) Gwen::MouseFocus = this;
            else Gwen::MouseFocus = NULL;
        }

        bool MapRendererControl::OnKeyPress(int iKey, bool bPress) {
            switch (iKey) {
                case Gwen::Key::Shift:
                    m_editor.inputController().modifierKeyDown(Controller::MK_SHIFT);
                    return true;
                case Gwen::Key::Control:
                    m_editor.inputController().modifierKeyDown(Controller::MK_CTRL);
                    return true;
                case Gwen::Key::Alt:
                    m_editor.inputController().modifierKeyDown(Controller::MK_ALT);
                    return true;
                case Gwen::Key::Command:
                    m_editor.inputController().modifierKeyDown(Controller::MK_CMD);
                    return true;
                default:
                    return false;
            }
        }
        
        bool MapRendererControl::OnKeyRelease(int iKey) {
            switch (iKey) {
                case Gwen::Key::Shift:
                    m_editor.inputController().modifierKeyUp(Controller::MK_SHIFT);
                    return true;
                case Gwen::Key::Control:
                    m_editor.inputController().modifierKeyUp(Controller::MK_CTRL);
                    return true;
                case Gwen::Key::Alt:
                    m_editor.inputController().modifierKeyUp(Controller::MK_ALT);
                    return true;
                case Gwen::Key::Command:
                    m_editor.inputController().modifierKeyUp(Controller::MK_CMD);
                    return true;
                default:
                    return false;
            }
        }
    }
}