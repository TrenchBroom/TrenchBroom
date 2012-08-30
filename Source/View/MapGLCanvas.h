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

#ifndef __TrenchBroom__MapGLCanvas__
#define __TrenchBroom__MapGLCanvas__

#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace Controller {
        class CameraMoveEvent;
        class CameraLookEvent;
        class CameraOrbitEvent;
        class InputController;
    }
    
    namespace Renderer {
        class Camera;
        class MapRenderer;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class MapGLCanvas : public wxGLCanvas {
        protected:
            int* m_attribs;
            bool m_firstFrame;
            wxGLContext* m_glContext;
            Controller::InputController* m_inputController;
            Renderer::Camera& m_camera;
            Renderer::MapRenderer& m_renderer;
            Utility::Console& m_console;
            
            int* Attribs();
        public:
            MapGLCanvas(wxWindow* parent, Utility::Console& console, Renderer::Camera& camera, Renderer::MapRenderer& renderer);
            ~MapGLCanvas();
            
            void OnPaint(wxPaintEvent& event);

            void OnCameraMove(Controller::CameraMoveEvent& event);
            void OnCameraLook(Controller::CameraLookEvent& event);
            void OnCameraOrbit(Controller::CameraOrbitEvent& event);

            void OnMouseLeftDown(wxMouseEvent& event);
            void OnMouseLeftUp(wxMouseEvent& event);
            void OnMouseRightDown(wxMouseEvent& event);
            void OnMouseRightUp(wxMouseEvent& event);
            void OnMouseMiddleDown(wxMouseEvent& event);
            void OnMouseMiddleUp(wxMouseEvent& event);
            void OnMouseMove(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
        protected:
            DECLARE_EVENT_TABLE()
        };
    }
}

#endif /* defined(__TrenchBroom__MapGLCanvas__) */
