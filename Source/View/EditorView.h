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

#include "Model/TextureTypes.h"

#include <wx/cmdproc.h>
#include <wx/docview.h>

namespace TrenchBroom {
    namespace Controller {
        class CameraMoveEvent;
        class CameraLookEvent;
        class CameraOrbitEvent;
    }
    
    namespace Model {
        class Filter;
        class MapDocument;
    }
    
    namespace Renderer {
        class Camera;
        class MapRenderer;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class Inspector;
        class MapWindow;
        class ViewOptions;
        
        class EditorView : public wxView {
            DECLARE_DYNAMIC_CLASS(EditorView)
        protected:
            Renderer::Camera* m_camera;
            Renderer::MapRenderer* m_renderer;
            Model::Filter* m_filter;
            ViewOptions* m_viewOptions;
            Utility::Console* m_console;
            
            void submit(wxCommand* command);
            void updateFaceInspector();
        public:
            EditorView();

            ViewOptions& viewOptions() const;
            Model::Filter& filter() const;
            Model::MapDocument& mapDocument() const;
            Renderer::Camera& camera() const;
            Renderer::MapRenderer& renderer() const;
            View::Inspector& inspector() const;
            Utility::Console& console() const;
            
            bool OnCreate(wxDocument* doc, long flags);
            void OnUpdate(wxView* sender, wxObject* hint = (wxObject*) NULL);
            void OnDraw(wxDC* dc);

            bool OnClose(bool deleteWindow = true);

            void OnCameraMove(Controller::CameraMoveEvent& event);
            void OnCameraLook(Controller::CameraLookEvent& event);
            void OnCameraOrbit(Controller::CameraOrbitEvent& event);

            void OnUndo(wxCommandEvent& event);
            void OnRedo(wxCommandEvent& event);

            void OnEditSelectAll(wxCommandEvent& event);
            void OnEditSelectNone(wxCommandEvent& event);
            
            void OnEditHideSelected(wxCommandEvent& event);
            void OnEditHideUnselected(wxCommandEvent& event);
            void OnEditUnhideAll(wxCommandEvent& event);
            
            void OnEditLockSelected(wxCommandEvent& event);
            void OnEditLockUnselected(wxCommandEvent& event);
            void OnEditUnlockAll(wxCommandEvent& event);
            
            void OnUpdateMenuItem(wxUpdateUIEvent& event);
            
            DECLARE_EVENT_TABLE();
        };
    }
}

#endif /* defined(__TrenchBroom__EditorView__) */
