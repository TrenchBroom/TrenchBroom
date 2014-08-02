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

#include "EntityPropertyGrid.h"

#include "Model/Object.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/EntityPropertyGridTable.h"
#include "View/EntityPropertySelectedCommand.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityPropertyGrid::EntityPropertyGrid(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_lastHoveredCell(wxGridCellCoords(-1, -1)),
        m_ignoreSelection(false),
        m_lastSelectedCol(0) {
            createGui(document, controller);
            bindEvents();
            bindObservers();
        }
        
        EntityPropertyGrid::~EntityPropertyGrid() {
            unbindObservers();
        }
        
        void EntityPropertyGrid::OnPropertyGridSize(wxSizeEvent& event) {
            m_grid->SetColSize(0, 100);
            const int colSize = std::max(1, m_grid->GetClientSize().x - m_grid->GetColSize(0));
            m_grid->SetColSize(1, colSize);
            event.Skip();
        }
        
        void EntityPropertyGrid::OnPropertyGridSelectCell(wxGridEvent& event) {
            const Model::PropertyKey key = m_table->propertyKey(event.GetRow());
            if (!m_ignoreSelection) {
                m_lastSelectedKey = key;
                m_lastSelectedCol = event.GetCol();
            }
            
            EntityPropertySelectedCommand command;
            command.setKey(key);
            command.SetEventObject(this);
            command.SetId(GetId());
            ProcessEvent(command);
        }
        
        void EntityPropertyGrid::OnPropertyGridTab(wxGridEvent& event) {
            if (event.ShiftDown()) {
                if (event.GetCol() > 0) {
                    m_grid->SelectRow(event.GetRow());
                    m_grid->GoToCell(event.GetRow(), event.GetCol() - 1);
                } else if (event.GetRow() > 0) {
                    m_grid->SelectRow(event.GetRow() - 1);
                    m_grid->GoToCell(event.GetRow() - 1, m_grid->GetNumberCols() - 1);
                }
            } else {
                if (event.GetCol() < m_grid->GetNumberCols() - 1) {
                    m_grid->SelectRow(event.GetRow());
                    m_grid->GoToCell(event.GetRow(), event.GetCol() + 1);
                } else if (event.GetRow() < m_grid->GetNumberRows() - 1) {
                    m_grid->SelectRow(event.GetRow() + 1);
                    m_grid->GoToCell(event.GetRow() + 1, 0);
                }
            }
        }
        
        void EntityPropertyGrid::OnPropertyGridKeyDown(wxKeyEvent& event) {
            if (isInsertRowShortcut(event)) {
                m_grid->AppendRows();
                m_grid->SelectRow(m_table->GetNumberPropertyRows() - 1);
                m_grid->GoToCell(m_table->GetNumberPropertyRows() - 1, 0);
            } else if (isDeleteRowShortcut(event)) {
                int firstRowIndex = m_grid->GetNumberRows();
                wxArrayInt selectedRows = m_grid->GetSelectedRows();
                wxArrayInt::reverse_iterator it, end;
                for (it = selectedRows.rbegin(), end = selectedRows.rend(); it != end; ++it) {
                    
                    m_grid->DeleteRows(*it, 1);
                    firstRowIndex = std::min(*it, firstRowIndex);
                }
                
                if (firstRowIndex < m_grid->GetNumberRows())
                    m_grid->SelectRow(firstRowIndex);
            } else {
                event.Skip();
            }
        }
        
        void EntityPropertyGrid::OnPropertyGridKeyUp(wxKeyEvent& event) {
            if (!isInsertRowShortcut(event) && !isDeleteRowShortcut(event))
                event.Skip();
        }

        bool EntityPropertyGrid::isInsertRowShortcut(const wxKeyEvent& event) const {
            return event.GetKeyCode() == WXK_RETURN && event.ControlDown();
        }
        
        bool EntityPropertyGrid::isDeleteRowShortcut(const wxKeyEvent& event) const {
            return (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_BACK) && !m_grid->IsCellEditControlShown();
        }

        void EntityPropertyGrid::OnPropertyGridMouseMove(wxMouseEvent& event) {
            int logicalX, logicalY;
            m_grid->CalcUnscrolledPosition(event.GetX(), event.GetY(), &logicalX, &logicalY);
            
            const wxGridCellCoords currentCell = m_grid->XYToCell(logicalX, logicalY);
            if (m_lastHoveredCell != currentCell) {
                const String tooltip = m_table->tooltip(currentCell);
                m_grid->SetToolTip(tooltip);
                m_lastHoveredCell = currentCell;
            }
            event.Skip();
        }
        
        void EntityPropertyGrid::OnUpdatePropertyView(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedObjects());
        }

        void EntityPropertyGrid::OnAddPropertyButton(wxCommandEvent& event) {
            m_grid->AppendRows();
            
            m_grid->SetFocus();
            int row = m_table->GetNumberPropertyRows() - 1;
            m_grid->SelectRow(row);
            m_grid->GoToCell(row, 0);
            m_grid->ShowCellEditControl();
        }
        
        void EntityPropertyGrid::OnRemovePropertiesButton(wxCommandEvent& event) {
            int firstRowIndex = m_grid->GetNumberRows();
            wxArrayInt selectedRows = m_grid->GetSelectedRows();
            wxArrayInt::reverse_iterator it, end;
            for (it = selectedRows.rbegin(), end = selectedRows.rend(); it != end; ++it) {
                m_grid->DeleteRows(*it, 1);
                firstRowIndex = std::min(*it, firstRowIndex);
            }
            
            if (firstRowIndex < m_grid->GetNumberRows())
                m_grid->SelectRow(firstRowIndex);
        }
        
        void EntityPropertyGrid::OnShowDefaultPropertiesCheckBox(wxCommandEvent& event) {
            m_table->setShowDefaultRows(event.IsChecked());
        }
        
        void EntityPropertyGrid::OnUpdateAddPropertyButton(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedObjects());
        }
        
        void EntityPropertyGrid::OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event) {
            event.Enable(!m_grid->GetSelectedRows().IsEmpty());
        }

        void EntityPropertyGrid::OnUpdateShowDefaultPropertiesCheckBox(wxUpdateUIEvent& event) {
            event.Check(m_table->showDefaultRows());
        }

        /*
         void EntityPropertyGrid::OnAddPropertyPressed(wxCommandEvent& event) {
         m_grid->AppendRows();
         
         m_grid->SetFocus();
         int row = m_table->GetNumberPropertyRows() - 1;
         m_grid->SelectRow(row);
         m_grid->GoToCell(row, 0);
         m_grid->ShowCellEditControl();
         }

         void EntityPropertyGrid::OnRemovePropertiesPressed(wxCommandEvent& event) {
            int firstRowIndex = m_grid->GetNumberRows();
            wxArrayInt selectedRows = m_grid->GetSelectedRows();
            wxArrayInt::reverse_iterator it, end;
            for (it = selectedRows.rbegin(), end = selectedRows.rend(); it != end; ++it) {
                m_grid->DeleteRows(*it, 1);
                firstRowIndex = std::min(*it, firstRowIndex);
            }
            
            if (firstRowIndex < m_grid->GetNumberRows())
                m_grid->SelectRow(firstRowIndex);
        }
        
        void EntityPropertyGrid::OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event) {
            wxArrayInt selectedRows = m_grid->GetSelectedRows();
            event.Enable(!selectedRows.empty());
            
            wxArrayInt::const_iterator it, end;
            for (it = selectedRows.begin(), end = selectedRows.end(); it != end; ++it) {
                wxGridCellAttr* attr = m_table->GetAttr(*it, 0, wxGridCellAttr::Cell);
                if (attr != NULL && attr->IsReadOnly()) {
                    event.Enable(false);
                    return;
                }
            }
        }
         */
        
        void EntityPropertyGrid::createGui(MapDocumentWPtr document, ControllerWPtr controller) {
            SetBackgroundColour(*wxWHITE);
            
            m_table = new EntityPropertyGridTable(document, controller);
            
            m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_grid->SetTable(m_table, true, wxGrid::wxGridSelectRows);
            // m_grid->SetUseNativeColLabels();
            // m_grid->UseNativeColHeader();
            m_grid->SetColLabelSize(18);
            m_grid->SetDefaultCellBackgroundColour(*wxWHITE);
            m_grid->HideRowLabels();
            
            m_grid->DisableColResize(0);
            m_grid->DisableColResize(1);
            m_grid->DisableDragColMove();
            m_grid->DisableDragCell();
            m_grid->DisableDragColSize();
            m_grid->DisableDragGridSize();
            m_grid->DisableDragRowSize();
            
            const wxBitmap addBitmap = IO::loadImageResource(IO::Path("images/Add.png"));
            const wxBitmap removeBitmap = IO::loadImageResource(IO::Path("images/Remove.png"));

            m_addPropertyButton = new wxBitmapButton(this, wxID_ANY, addBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_addPropertyButton->SetToolTip("Add a new property");
            m_addPropertyButton->SetBackgroundColour(*wxWHITE);
            m_removePropertiesButton = new wxBitmapButton(this, wxID_ANY, removeBitmap, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
            m_removePropertiesButton->SetToolTip("Remove the selected properties");
            m_removePropertiesButton->SetBackgroundColour(*wxWHITE);

            m_showDefaultPropertiesCheckBox = new wxCheckBox(this, wxID_ANY, "Show default properties");
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(m_addPropertyButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(m_removePropertiesButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddSpacer(LayoutConstants::WideHMargin);
            buttonSizer->Add(m_showDefaultPropertiesCheckBox, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_grid, 1, wxEXPAND);
            sizer->Add(buttonSizer, 0, wxEXPAND);
            SetSizer(sizer);
        }
        
        void EntityPropertyGrid::bindEvents() {
            m_grid->Bind(wxEVT_SIZE, &EntityPropertyGrid::OnPropertyGridSize, this);
            m_grid->Bind(wxEVT_GRID_SELECT_CELL, &EntityPropertyGrid::OnPropertyGridSelectCell, this);
            m_grid->Bind(wxEVT_GRID_TABBING, &EntityPropertyGrid::OnPropertyGridTab, this);
            m_grid->Bind(wxEVT_KEY_DOWN, &EntityPropertyGrid::OnPropertyGridKeyDown, this);
            m_grid->Bind(wxEVT_KEY_UP, &EntityPropertyGrid::OnPropertyGridKeyUp, this);
            m_grid->GetGridWindow()->Bind(wxEVT_MOTION, &EntityPropertyGrid::OnPropertyGridMouseMove, this);
            m_grid->Bind(wxEVT_UPDATE_UI, &EntityPropertyGrid::OnUpdatePropertyView, this);
            
            m_addPropertyButton->Bind(wxEVT_BUTTON, &EntityPropertyGrid::OnAddPropertyButton, this);
            m_addPropertyButton->Bind(wxEVT_UPDATE_UI, &EntityPropertyGrid::OnUpdateAddPropertyButton, this);
            m_removePropertiesButton->Bind(wxEVT_BUTTON, &EntityPropertyGrid::OnRemovePropertiesButton, this);
            m_removePropertiesButton->Bind(wxEVT_UPDATE_UI, &EntityPropertyGrid::OnUpdateRemovePropertiesButton, this);
            m_showDefaultPropertiesCheckBox->Bind(wxEVT_CHECKBOX, &EntityPropertyGrid::OnShowDefaultPropertiesCheckBox, this);
            m_showDefaultPropertiesCheckBox->Bind(wxEVT_UPDATE_UI, &EntityPropertyGrid::OnUpdateShowDefaultPropertiesCheckBox, this);
        }
        
        void EntityPropertyGrid::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityPropertyGrid::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityPropertyGrid::documentWasLoaded);
            document->objectDidChangeNotifier.addObserver(this, &EntityPropertyGrid::objectDidChange);
            document->selectionWillChangeNotifier.addObserver(this, &EntityPropertyGrid::selectionWillChange);
            document->selectionDidChangeNotifier.addObserver(this, &EntityPropertyGrid::selectionDidChange);
        }
        
        void EntityPropertyGrid::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityPropertyGrid::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityPropertyGrid::documentWasLoaded);
                document->objectDidChangeNotifier.removeObserver(this, &EntityPropertyGrid::objectDidChange);
                document->selectionWillChangeNotifier.removeObserver(this, &EntityPropertyGrid::selectionWillChange);
                document->selectionDidChangeNotifier.removeObserver(this, &EntityPropertyGrid::selectionDidChange);
            }
        }
        
        void EntityPropertyGrid::documentWasNewed() {
            updateControls();
        }
        
        void EntityPropertyGrid::documentWasLoaded() {
            updateControls();
        }
        
        void EntityPropertyGrid::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::Type_Entity)
                updateControls();
        }
        
        void EntityPropertyGrid::selectionWillChange() {
            m_grid->SaveEditControlValue();
            m_grid->HideCellEditControl();
        }
        
        void EntityPropertyGrid::selectionDidChange(const Model::SelectionResult& result) {
            updateControls();
        }
        
        void EntityPropertyGrid::updateControls() {
            const SetBool ignoreSelection(m_ignoreSelection);
            m_table->update();
            
            const int row = m_table->rowForKey(m_lastSelectedKey);
            if (row != -1) {
                m_grid->SelectRow(row);
                m_grid->GoToCell(row, m_lastSelectedCol);
            }
        }
        
        Model::PropertyKey EntityPropertyGrid::selectedRowKey() const {
            wxArrayInt selectedRows = m_grid->GetSelectedRows();
            if (selectedRows.empty())
                return "";
            const int row = selectedRows.front();
            return m_table->propertyKey(row);
        }
    }
}
