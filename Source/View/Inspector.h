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

#ifndef __TrenchBroom__Inspector__
#define __TrenchBroom__Inspector__

#include <wx/notebook.h>
#include <wx/panel.h>

#include "Model/BrushTypes.h"
#include "Model/FaceTypes.h"

class wxGLContext;

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
        class Texture;
    }
    
    namespace View {
        class DocumentViewHolder;
        class EditorView;
        class EntityInspector;
        class FaceInspector;
        class ViewInspector;
        
        class Inspector : public wxPanel {
        protected:
            DocumentViewHolder& m_documentViewHolder;
            
            EntityInspector* m_entityInspector;
            FaceInspector* m_faceInspector;
            
            wxNotebook* m_notebook;
            wxNotebookPage* CreateMapInspector();
            EntityInspector* CreateEntityInspector();
            FaceInspector* CreateFaceInspector();
            ViewInspector* CreateViewInspector();
        public:
            Inspector(wxWindow* parent, DocumentViewHolder& documentViewHolder);

            inline EntityInspector& entityInspector() const {
                return *m_entityInspector;
            }
            
            inline FaceInspector& faceInspector() const {
                return *m_faceInspector;
            }
            
            void switchToInspector(size_t index);
        };
    }
}

#endif /* defined(__TrenchBroom__Inspector__) */
