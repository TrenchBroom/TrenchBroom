/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_FaceInspector
#define TrenchBroom_FaceInspector

#include "View/TabBook.h"
#include "View/ViewTypes.h"

class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class Object;
        class SelectionResult;
    }
    
    namespace View {
        class FaceAttribsEditor;
        class GLContextManager;
        class TextureBrowser;
        class TextureCollectionEditor;
        class TextureSelectedCommand;
        
        class FaceInspector : public TabBookPage {
        private:
            MapDocumentWPtr m_document;
            
            FaceAttribsEditor* m_faceAttribsEditor;
            TextureBrowser* m_textureBrowser;
            TextureCollectionEditor* m_textureCollectionEditor;
        public:
            FaceInspector(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager);

            void OnTextureSelected(TextureSelectedCommand& event);
        private:
            void createGui(MapDocumentWPtr document, GLContextManager& contextManager);
            wxWindow* createFaceAttribsEditor(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            wxWindow* createTextureBrowser(wxWindow* parent, MapDocumentWPtr document, GLContextManager& contextManager);
            wxWindow* createTextureCollectionEditor(wxWindow* parent, MapDocumentWPtr document);
            
            void bindEvents();
        };
    }
}

#endif /* defined(TrenchBroom_FaceInspector) */
