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

#ifndef __TrenchBroom__FaceAttribsEditor__
#define __TrenchBroom__FaceAttribsEditor__

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxStaticText;
class SpinControl;
class SpinControlEvent;

namespace TrenchBroom {
    namespace Model {
        class SelectionResult;
    }
    
    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        class ControllerFacade;
        class FlagsPopupEditor;
        class TextureView;
        
        class FaceAttribsEditor : public wxPanel {
        private:
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            Model::BrushFaceList m_faces;
            
            TextureView* m_textureView;
            wxStaticText* m_textureNameLabel;
            
            SpinControl* m_xOffsetEditor;
            SpinControl* m_yOffsetEditor;
            SpinControl* m_xScaleEditor;
            SpinControl* m_yScaleEditor;
            SpinControl* m_rotationEditor;
            
            FlagsPopupEditor* m_surfaceFlagsEditor;
            FlagsPopupEditor* m_contentFlagsEditor;
        public:
            FaceAttribsEditor(wxWindow* parent, Renderer::RenderResources& resources, MapDocumentPtr document, ControllerPtr controller);
            ~FaceAttribsEditor();
            
            void OnXOffsetChanged(SpinControlEvent& event);
            void OnYOffsetChanged(SpinControlEvent& event);
            void OnRotationChanged(SpinControlEvent& event);
            void OnXScaleChanged(SpinControlEvent& event);
            void OnYScaleChanged(SpinControlEvent& event);
            void OnIdle(wxIdleEvent& event);
        private:
            void createGui(Renderer::RenderResources& resources);
            void bindEvents();
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void faceDidChange(Model::BrushFace* face);
            void selectionDidChange(const Model::SelectionResult& result);
            void textureCollectionsDidChange();
            
            void updateControls();
            void getSurfaceFlags(wxArrayString& names, wxArrayString& descriptions) const;
            void getContentFlags(wxArrayString& names, wxArrayString& descriptions) const;
        };
    }
}

#endif /* defined(__TrenchBroom__FaceAttribsEditor__) */
