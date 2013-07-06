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

#ifndef __TrenchBroom__MapView__
#define __TrenchBroom__MapView__

#include "Controller/Command.h"
#include "GL/GL.h"
#include "Renderer/Camera.h"
#include "Renderer/MapRenderer.h"
#include "View/InputState.h"

#include <vector>
#include <wx/event.h>
#include <wx/glcanvas.h>

namespace TrenchBroom {
    namespace View {
        class BaseTool;
        class CameraTool;
        class Console;
        
        class MapView : public wxGLCanvas {
        private:
            bool m_initialized;
            Console& m_console;
            wxGLContext* m_glContext;
            
            Renderer::Camera m_camera;
            Renderer::MapRenderer m_renderer;
            
            InputState m_inputState;
            bool m_drag;
            wxPoint m_clickPos;
            CameraTool* m_cameraTool;
            BaseTool* m_toolChain;
        public:
            MapView(wxWindow* parent, Console& console);
            ~MapView();
            
            void makeCurrent();
            
            void OnMouseButton(wxMouseEvent& event);
            void OnMouseMotion(wxMouseEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
            
            void OnPaint(wxPaintEvent& event);
            void OnSize(wxSizeEvent& event);

            void commandDone(Controller::Command::Ptr command);
            void commandUndone(Controller::Command::Ptr command);
        private:
            void createTools();
            void deleteTools();
            void bindEvents();
            void initializeGL();
            
            static const int* attribs();
            static int depthBits();
            static bool multisample();
        };
    }
}

#endif /* defined(__TrenchBroom__MapView__) */
