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

#ifndef __TrenchBroom__EntityPropertyEditor__
#define __TrenchBroom__EntityPropertyEditor__

#include "View/ViewTypes.h"

#include <wx/grid.h>
#include <wx/panel.h>

class wxWindow;

namespace TrenchBroom {
    namespace Model {
        class Object;
        class SelectionResult;
    }
    
    namespace View {
        class EntityPropertyGridTable;
        
        class EntityPropertyEditor : public wxPanel {
        private:
            MapDocumentPtr m_document;
            
            EntityPropertyGridTable* m_table;
            wxGrid* m_grid;
            wxGridCellCoords m_lastHoveredCell;
            wxButton* m_addPropertyButton;
            wxButton* m_removePropertiesButton;
        public:
            EntityPropertyEditor(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller);
            ~EntityPropertyEditor();
            
            void OnPropertyGridSize(wxSizeEvent& event);
            void OnPropertyGridSelectCell(wxGridEvent& event);
            void OnPropertyGridTab(wxGridEvent& event);
            void OnPropertyGridMouseMove(wxMouseEvent& event);

            void OnAddPropertyPressed(wxCommandEvent& event);
            void OnRemovePropertiesPressed(wxCommandEvent& event);
            void OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event);
            void OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event);
        private:
            void createGui(MapDocumentPtr document, ControllerPtr controller);
            void bindEvents();
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed();
            void documentWasLoaded();
            void objectDidChange(Model::Object* object);
            void selectionDidChange(const Model::SelectionResult& result);
            
            void updateControls();
        };
    }
}

#endif /* defined(__TrenchBroom__EntityPropertyEditor__) */
