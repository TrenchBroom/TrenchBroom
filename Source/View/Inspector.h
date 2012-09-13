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

namespace TrenchBroom {
    namespace Model {
        class MapDocument;
    }
    
    namespace View {
        class EditorView;
        
        class Inspector : public wxPanel {
        protected:
            Model::MapDocument& m_document;
            EditorView& m_view;
            
            wxNotebook* m_notebook;
            wxNotebookPage* CreateMapInspector();
            wxNotebookPage* CreateEntityInspector();
            wxNotebookPage* CreateBrushInspector();
            wxNotebookPage* CreateFaceInspector();
            wxNotebookPage* CreateViewInspector();
        public:
            Inspector(wxWindow* parent, Model::MapDocument& document, EditorView& view);
        };
    }
}

#endif /* defined(__TrenchBroom__Inspector__) */
