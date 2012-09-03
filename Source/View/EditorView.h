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

#ifndef __TrenchBroom__EditorView__
#define __TrenchBroom__EditorView__

#include <wx/docview.h>

namespace TrenchBroom {
    namespace Controller {
        class CameraMoveEvent;
        class CameraLookEvent;
        class CameraOrbitEvent;
    }
    
    namespace Renderer {
        class Camera;
        class MapRenderer;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class MapWindow;
        
        class EditorView : public wxView {
            DECLARE_DYNAMIC_CLASS(EditorView)
        protected:
            Renderer::Camera* m_camera;
            Renderer::MapRenderer* m_renderer;
            Utility::Console* m_console;
        public:
            EditorView();

            Utility::Console& Console() const;
            Renderer::Camera& Camera() const;
            Renderer::MapRenderer& Renderer() const;
            
            bool OnCreate(wxDocument* doc, long flags);
            void OnUpdate(wxView* sender, wxObject* hint = (wxObject*) NULL);
            void OnDraw(wxDC* dc);

            bool OnClose(bool deleteWindow = true);

            void OnCameraMove(Controller::CameraMoveEvent& event);
            void OnCameraLook(Controller::CameraLookEvent& event);
            void OnCameraOrbit(Controller::CameraOrbitEvent& event);
            
            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__EditorView__) */
