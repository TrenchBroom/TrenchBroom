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

namespace TrenchBroom {
    namespace Renderer {
        class RenderResources;
    }
    
    namespace View {
        class FaceAttribsEditor;
        class TextureBrowser;
        class TextureSelectedCommand;
        
        class FaceInspector : public wxPanel {
        private:
            MapDocumentPtr m_document;
            ControllerFacade& m_controller;
            
            FaceAttribsEditor* m_faceAttribsEditor;
            TextureBrowser* m_textureBrowser;
        public:
            FaceInspector(wxWindow* parent, MapDocumentPtr document, ControllerFacade& controller, Renderer::RenderResources& resources);

            void update(Controller::Command::Ptr command);

            void OnTextureSelected(TextureSelectedCommand& event);
        };
    }
}

#endif /* defined(__TrenchBroom__FaceInspector__) */
