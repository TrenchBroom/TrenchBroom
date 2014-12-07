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

#ifndef __TrenchBroom__EntityAttributeGrid__
#define __TrenchBroom__EntityAttributeGrid__

#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

#include <wx/grid.h>
#include <wx/panel.h>

class wxButton;
class wxCheckBox;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class EntityAttributeGridTable;
        class Selection;
        
        class EntityAttributeGrid : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            
            EntityAttributeGridTable* m_table;
            wxGrid* m_grid;
            wxGridCellCoords m_lastHoveredCell;
            
            bool m_ignoreSelection;
            Model::AttributeName m_lastSelectedName;
            int m_lastSelectedCol;
        public:
            EntityAttributeGrid(wxWindow* parent, MapDocumentWPtr document);
            ~EntityAttributeGrid();
            
            void OnPropertyGridSize(wxSizeEvent& event);
            void OnPropertyGridSelectCell(wxGridEvent& event);
            void OnPropertyGridTab(wxGridEvent& event);
            
            void OnPropertyGridKeyDown(wxKeyEvent& event);
            void OnPropertyGridKeyUp(wxKeyEvent& event);
        private:
            bool isInsertRowShortcut(const wxKeyEvent& event) const;
            bool isDeleteRowShortcut(const wxKeyEvent& event) const;
        public:
            void OnPropertyGridMouseMove(wxMouseEvent& event);

            void OnUpdatePropertyView(wxUpdateUIEvent& event);

            void OnAddPropertyButton(wxCommandEvent& event);
            void OnRemovePropertiesButton(wxCommandEvent& event);
            void OnShowDefaultPropertiesCheckBox(wxCommandEvent& event);
            void OnUpdateAddPropertyButton(wxUpdateUIEvent& event);
            void OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event);
            void OnUpdateShowDefaultPropertiesCheckBox(wxUpdateUIEvent& event);
        private:
            void createGui(MapDocumentWPtr document);
            
            void bindObservers();
            void unbindObservers();
            
            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void nodesDidChange(const Model::NodeList& nodes);
            void selectionWillChange();
            void selectionDidChange(const Selection& selection);
            
            void updateControls();
            Model::AttributeName selectedRowName() const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityAttributeGrid__) */
