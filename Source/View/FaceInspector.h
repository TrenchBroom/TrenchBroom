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


#ifndef __TrenchBroom__FaceInspector__
#define __TrenchBroom__FaceInspector__

#include <wx/panel.h>

#include "Model/BrushTypes.h"
#include "Model/FaceTypes.h"

class wxGLContext;
class wxSpinCtrlDouble;
class wxStaticText;

namespace TrenchBroom {
    namespace Model {
        class Texture;
    }
    
    namespace View {
        class DocumentViewHolder;
        class SingleTextureViewer;
        class TextureBrowser;
        
        class FaceInspector : public wxPanel {
        protected:
            DocumentViewHolder& m_documentViewHolder;
            
            SingleTextureViewer* m_textureViewer;
            wxStaticText* m_textureNameLabel;
            
            wxSpinCtrlDouble* m_xOffsetEditor;
            wxSpinCtrlDouble* m_yOffsetEditor;
            wxSpinCtrlDouble* m_xScaleEditor;
            wxSpinCtrlDouble* m_yScaleEditor;
            wxSpinCtrlDouble* m_rotationEditor;
            
            TextureBrowser* m_textureBrowser;
            
            wxWindow* createFaceEditor(wxGLContext* sharedContext);
            wxWindow* createTextureBrowser(wxGLContext* sharedContext);
        public:
            FaceInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder, wxGLContext* sharedContext);
            
            void update(const Model::FaceList& faces);
            void update(const Model::BrushList& brushes);
            void updateSelectedTexture(Model::Texture* texture);
            void updateTextureBrowser();
        };
    }
}

#endif /* defined(__TrenchBroom__FaceInspector__) */
