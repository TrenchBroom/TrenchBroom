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

#ifndef __TrenchBroom__FaceInspector__
#define __TrenchBroom__FaceInspector__

#include "Controller/Command.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxCollapsiblePaneEvent;
class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class Object;
        class SelectionResult;
    }

    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        class FaceAttribsEditor;
        class TextureBrowser;
        class TextureCollectionEditor;
        class TextureSelectedCommand;
        
        class FaceInspector : public wxPanel {
        private:
            MapDocumentPtr m_document;
            ControllerPtr m_controller;
            
            FaceAttribsEditor* m_faceAttribsEditor;
            TextureBrowser* m_textureBrowser;
            TextureCollectionEditor* m_textureCollectionEditor;
        public:
            FaceInspector(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller, Renderer::RenderResources& resources);

            void OnTextureSelected(TextureSelectedCommand& event);
            void OnTextureCollectionEditorPaneChanged(wxCollapsiblePaneEvent& event);
        private:
            void createGui(Renderer::RenderResources& resources);
            wxWindow* createFaceAttribsEditor(wxWindow* parent, Renderer::RenderResources& resources);
            wxWindow* createTextureBrowser(wxWindow* parent, Renderer::RenderResources& resources);
            wxWindow* createTextureCollectionEditor(wxWindow* parent);
            
            void bindEvents();
        };
    }
}

#endif /* defined(__TrenchBroom__FaceInspector__) */
