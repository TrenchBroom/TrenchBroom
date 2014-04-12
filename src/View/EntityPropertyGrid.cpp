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
#include "View/EntityPropertyGridTable.h"
#include "View/EntityPropertySelectedCommand.h"
#include "View/ViewConstants.h"
#include "View/MapDocument.h"

#include <wx/button.h>
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
                } else {
                    m_table->AppendRows();
                    m_grid->SelectRow(event.GetRow() - 1);
                    m_grid->GoToCell(m_grid->GetNumberRows() - 1, 0);
                }
            }
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
        
        void EntityPropertyGrid::OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event) {
            MapDocumentSPtr document = lock(m_document);
            event.Enable(document->hasSelectedObjects());
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
        
        void EntityPropertyGrid::createGui(MapDocumentWPtr document, ControllerWPtr controller) {
            m_table = new EntityPropertyGridTable(document, controller);
            
            m_grid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_grid->SetTable(m_table, true, wxGrid::wxGridSelectRows);
            m_grid->SetUseNativeColLabels();
            m_grid->UseNativeColHeader();
            m_grid->SetDefaultCellBackgroundColour(*wxWHITE);
            m_grid->HideRowLabels();
            
            m_grid->DisableColResize(0);
            m_grid->DisableColResize(1);
            m_grid->DisableDragColMove();
            m_grid->DisableDragCell();
            m_grid->DisableDragColSize();
            m_grid->DisableDragGridSize();
            m_grid->DisableDragRowSize();
            
            m_addPropertyButton = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_addPropertyButton->SetToolTip("Add a new property");
            m_removePropertiesButton = new wxButton(this, wxID_ANY, "-", wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_removePropertiesButton->SetToolTip("Remove the selected properties");
            
            wxSizer* propertyViewButtonsSizer = new wxBoxSizer(wxVERTICAL);
            propertyViewButtonsSizer->Add(m_addPropertyButton, 0, wxEXPAND);
            propertyViewButtonsSizer->AddSpacer(LayoutConstants::ControlMargin);
            propertyViewButtonsSizer->Add(m_removePropertiesButton, 0, wxEXPAND);
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_grid, 1, wxEXPAND);
            outerSizer->SetItemMinSize(m_grid, wxDefaultSize.x, 300);
            outerSizer->AddSpacer(LayoutConstants::ControlMargin);
            outerSizer->Add(propertyViewButtonsSizer, 0, wxEXPAND);
            
            SetSizerAndFit(outerSizer);
        }
        
        void EntityPropertyGrid::bindEvents() {
            m_grid->Bind(wxEVT_SIZE, &EntityPropertyGrid::OnPropertyGridSize, this);
            m_grid->Bind(wxEVT_GRID_SELECT_CELL, &EntityPropertyGrid::OnPropertyGridSelectCell, this);
            m_grid->Bind(wxEVT_GRID_TABBING, &EntityPropertyGrid::OnPropertyGridTab, this);
            m_grid->GetGridWindow()->Bind(wxEVT_MOTION, &EntityPropertyGrid::OnPropertyGridMouseMove, this);
            m_grid->Bind(wxEVT_UPDATE_UI, &EntityPropertyGrid::OnUpdatePropertyViewOrAddPropertiesButton, this);
            
            m_addPropertyButton->Bind(wxEVT_BUTTON, &EntityPropertyGrid::OnAddPropertyPressed, this);
            m_addPropertyButton->Bind(wxEVT_UPDATE_UI, &EntityPropertyGrid::OnUpdatePropertyViewOrAddPropertiesButton, this);
            m_removePropertiesButton->Bind(wxEVT_BUTTON, &EntityPropertyGrid::OnRemovePropertiesPressed, this);
            m_removePropertiesButton->Bind(wxEVT_UPDATE_UI, &EntityPropertyGrid::OnUpdateRemovePropertiesButton, this);
        }
        
        void EntityPropertyGrid::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasNewedNotifier.addObserver(this, &EntityPropertyGrid::documentWasNewed);
            document->documentWasLoadedNotifier.addObserver(this, &EntityPropertyGrid::documentWasLoaded);
            document->objectDidChangeNotifier.addObserver(this, &EntityPropertyGrid::objectDidChange);
            document->selectionDidChangeNotifier.addObserver(this, &EntityPropertyGrid::selectionDidChange);
        }
        
        void EntityPropertyGrid::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasNewedNotifier.removeObserver(this, &EntityPropertyGrid::documentWasNewed);
                document->documentWasLoadedNotifier.removeObserver(this, &EntityPropertyGrid::documentWasLoaded);
                document->objectDidChangeNotifier.removeObserver(this, &EntityPropertyGrid::objectDidChange);
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
