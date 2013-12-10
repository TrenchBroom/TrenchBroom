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

#include "EntityPropertyEditor.h"

#include "Model/Object.h"
#include "View/EntityPropertyGridTable.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"

#include <wx/button.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityPropertyEditor::EntityPropertyEditor(wxWindow* parent, MapDocumentPtr document, ControllerPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_lastHoveredCell(wxGridCellCoords(-1, -1)) {
            createGui(document, controller);
            bindEvents();
            bindObservers();
        }
        
        EntityPropertyEditor::~EntityPropertyEditor() {
            unbindObservers();
        }
        
        void EntityPropertyEditor::OnPropertyGridSize(wxSizeEvent& event) {
            m_grid->SetColSize(0, 100);
            const int colSize = std::max(1, m_grid->GetClientSize().x - m_grid->GetColSize(0));
            m_grid->SetColSize(1, colSize);
            event.Skip();
        }
        
        void EntityPropertyEditor::OnPropertyGridSelectCell(wxGridEvent& event) {
            // updateSmartEditor(event.GetRow());
        }
        
        void EntityPropertyEditor::OnPropertyGridTab(wxGridEvent& event) {
            if (event.ShiftDown()) {
                if (event.GetCol() > 0) {
                    m_grid->GoToCell(event.GetRow(), event.GetCol() - 1);
                } else if (event.GetRow() > 0) {
                    m_grid->GoToCell(event.GetRow() - 1, m_grid->GetNumberCols() - 1);
                }
            } else {
                if (event.GetCol() < m_grid->GetNumberCols() - 1) {
                    m_grid->GoToCell(event.GetRow(), event.GetCol() + 1);
                } else if (event.GetRow() < m_grid->GetNumberRows() - 1) {
                    m_grid->GoToCell(event.GetRow() + 1, 0);
                } else {
                    m_table->AppendRows();
                    m_grid->GoToCell(m_grid->GetNumberRows() - 1, 0);
                }
            }
        }
        
        void EntityPropertyEditor::OnPropertyGridMouseMove(wxMouseEvent& event) {
            int logicalX, logicalY;
            m_grid->CalcUnscrolledPosition(event.GetX(), event.GetY(), &logicalX, &logicalY);
            // logicalY -= m_grid->GetRowHeight(0); // compensate for header row
            const wxGridCellCoords currentCell = m_grid->XYToCell(logicalX, logicalY);
            if (m_lastHoveredCell != currentCell) {
                const String tooltip = m_table->tooltip(currentCell);
                m_grid->SetToolTip(tooltip);
                m_lastHoveredCell = currentCell;
            }
            event.Skip();
        }
        
        void EntityPropertyEditor::OnAddPropertyPressed(wxCommandEvent& event) {
            m_grid->AppendRows();
            
            m_grid->SetFocus();
            int row = m_grid->GetNumberRows() - 1;
            m_grid->SelectBlock(row, 0, row, 0);
            m_grid->GoToCell(row, 0);
            m_grid->ShowCellEditControl();
            
            // updateSmartEditor(m_grid->GetGridCursorRow());
        }
        
        void EntityPropertyEditor::OnRemovePropertiesPressed(wxCommandEvent& event) {
            wxArrayInt selectedRows = m_grid->GetSelectedRows();
            wxArrayInt::reverse_iterator it, end;
            for (it = selectedRows.rbegin(), end = selectedRows.rend(); it != end; ++it)
                m_grid->DeleteRows(*it, 1);
            // updateSmartEditor(m_grid->GetGridCursorRow());
        }
        
        void EntityPropertyEditor::OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event) {
            event.Enable(m_document->hasSelectedObjects());
        }
        
        void EntityPropertyEditor::OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event) {
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

        void EntityPropertyEditor::createGui(MapDocumentPtr document, ControllerPtr controller) {
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
            
            /*
             wxPanel* smartPropertyEditorPanel = new wxPanel(propertyEditorPanel);
             m_smartPropertyEditorManager = new SmartPropertyEditorManager(smartPropertyEditorPanel, m_documentViewHolder);
             
             wxSizer* propertyEditorSizer = new wxBoxSizer(wxVERTICAL);
             propertyEditorSizer->Add(m_grid, 1, wxEXPAND);
             propertyEditorSizer->AddSpacer(LayoutConstants::ControlMargin);
             propertyEditorSizer->Add(smartPropertyEditorPanel, 0, wxEXPAND);
             propertyEditorSizer->SetMinSize(wxDefaultSize.x, 300);
             */
            
            m_addPropertyButton = new wxButton(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_addPropertyButton->SetToolTip(_("Add a new property"));
            m_removePropertiesButton = new wxButton(this, wxID_ANY, _("-"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_removePropertiesButton->SetToolTip(_("Remove the selected properties"));
            
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
        
        void EntityPropertyEditor::bindEvents() {
            m_grid->Bind(wxEVT_SIZE, &EntityPropertyEditor::OnPropertyGridSize, this);
            m_grid->Bind(wxEVT_GRID_SELECT_CELL, &EntityPropertyEditor::OnPropertyGridSelectCell, this);
            m_grid->Bind(wxEVT_GRID_TABBING, &EntityPropertyEditor::OnPropertyGridTab, this);
            m_grid->GetGridWindow()->Bind(wxEVT_MOTION, &EntityPropertyEditor::OnPropertyGridMouseMove, this);
            m_grid->Bind(wxEVT_UPDATE_UI, &EntityPropertyEditor::OnUpdatePropertyViewOrAddPropertiesButton, this);

            m_addPropertyButton->Bind(wxEVT_BUTTON, &EntityPropertyEditor::OnAddPropertyPressed, this);
            m_addPropertyButton->Bind(wxEVT_UPDATE_UI, &EntityPropertyEditor::OnUpdatePropertyViewOrAddPropertiesButton, this);
            m_removePropertiesButton->Bind(wxEVT_BUTTON, &EntityPropertyEditor::OnRemovePropertiesPressed, this);
            m_removePropertiesButton->Bind(wxEVT_UPDATE_UI, &EntityPropertyEditor::OnUpdateRemovePropertiesButton, this);
        }

        void EntityPropertyEditor::bindObservers() {
            m_document->documentWasNewedNotifier.addObserver(this, &EntityPropertyEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.addObserver(this, &EntityPropertyEditor::documentWasLoaded);
            m_document->objectDidChangeNotifier.addObserver(this, &EntityPropertyEditor::objectDidChange);
            m_document->selectionDidChangeNotifier.addObserver(this, &EntityPropertyEditor::selectionDidChange);
        }
        
        void EntityPropertyEditor::unbindObservers() {
            m_document->documentWasNewedNotifier.removeObserver(this, &EntityPropertyEditor::documentWasNewed);
            m_document->documentWasLoadedNotifier.removeObserver(this, &EntityPropertyEditor::documentWasLoaded);
            m_document->objectDidChangeNotifier.removeObserver(this, &EntityPropertyEditor::objectDidChange);
            m_document->selectionDidChangeNotifier.removeObserver(this, &EntityPropertyEditor::selectionDidChange);
        }
        
        void EntityPropertyEditor::documentWasNewed() {
            updateControls();
        }
        
        void EntityPropertyEditor::documentWasLoaded() {
            updateControls();
        }
        
        void EntityPropertyEditor::objectDidChange(Model::Object* object) {
            if (object->type() == Model::Object::OTEntity)
                updateControls();
        }
        
        void EntityPropertyEditor::selectionDidChange(const Model::SelectionResult& result) {
            updateControls();
        }

        void EntityPropertyEditor::updateControls() {
            m_table->update();
        }
    }
}
