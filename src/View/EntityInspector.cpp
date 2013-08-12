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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityInspector.h"

#include "StringUtils.h"
#include "Controller/EntityPropertyCommand.h"
#include "Controller/NewDocumentCommand.h"
#include "Controller/OpenDocumentCommand.h"
#include "Controller/SelectionCommand.h"
#include "Model/EntityProperties.h"
#include "View/CommandIds.h"
#include "View/EntityPropertyGridTable.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        EntityInspector::EntityInspector(wxWindow* parent, MapDocumentPtr document, Controller::ControllerFacade& controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_lastHoveredCell(wxGridCellCoords(-1, -1)) {
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(createPropertyEditor(this), 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            outerSizer->Add(createEntityBrowser(this), 1, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            SetSizerAndFit(outerSizer);
        }

        void EntityInspector::update(Controller::Command::Ptr command) {
            using namespace Controller;
            
            if (command->type() == NewDocumentCommand::Type ||
                command->type() == OpenDocumentCommand::Type) {
                updatePropertyGrid();
                updateEntityBrowser();
            } else if (command->type() == EntityPropertyCommand::Type) {
                updatePropertyGrid();
                EntityPropertyCommand::Ptr entityPropertyCommand = Command::cast<EntityPropertyCommand>(command);
                if (entityPropertyCommand->propertyAffected(Model::PropertyKeys::Mod) ||
                    entityPropertyCommand->propertyAffected(Model::PropertyKeys::EntityDefinitions))
                    updateEntityBrowser();
            } else if (command->type() == SelectionCommand::Type) {
                updatePropertyGrid();
            }
        }

        void EntityInspector::OnPropertyGridSize(wxSizeEvent& event) {
            m_propertyGrid->SetColSize(0, 100);
            const int colSize = std::max(1, m_propertyGrid->GetClientSize().x - m_propertyGrid->GetColSize(0));
            m_propertyGrid->SetColSize(1, colSize);
            event.Skip();
        }
        
        void EntityInspector::OnPropertyGridSelectCell(wxGridEvent& event) {
            // updateSmartEditor(event.GetRow());
        }
        
        void EntityInspector::OnPropertyGridTab(wxGridEvent& event) {
            if (event.ShiftDown()) {
                if (event.GetCol() > 0) {
                    m_propertyGrid->GoToCell(event.GetRow(), event.GetCol() - 1);
                } else if (event.GetRow() > 0) {
                    m_propertyGrid->GoToCell(event.GetRow() - 1, m_propertyGrid->GetNumberCols() - 1);
                }
            } else {
                if (event.GetCol() < m_propertyGrid->GetNumberCols() - 1) {
                    m_propertyGrid->GoToCell(event.GetRow(), event.GetCol() + 1);
                } else if (event.GetRow() < m_propertyGrid->GetNumberRows() - 1) {
                    m_propertyGrid->GoToCell(event.GetRow() + 1, 0);
                } else {
                    m_propertyTable->AppendRows();
                    m_propertyGrid->GoToCell(m_propertyGrid->GetNumberRows() - 1, 0);
                }
            }
        }

        void EntityInspector::OnPropertyGridMouseMove(wxMouseEvent& event) {
            int logicalX, logicalY;
            m_propertyGrid->CalcUnscrolledPosition(event.GetX(), event.GetY(), &logicalX, &logicalY);
            logicalY -= m_propertyGrid->GetRowHeight(0); // compensate for header row
            wxGridCellCoords currentCell = m_propertyGrid->XYToCell(logicalX, logicalY);
            if (m_lastHoveredCell != currentCell) {
                String tooltip = m_propertyTable->tooltip(currentCell);
                m_propertyGrid->SetToolTip(tooltip);
                m_lastHoveredCell = currentCell;
            }
        }
        
        void EntityInspector::OnAddPropertyPressed(wxCommandEvent& event) {
            m_propertyGrid->AppendRows();
            
            m_propertyGrid->SetFocus();
            int row = m_propertyGrid->GetNumberRows() - 1;
            m_propertyGrid->SelectBlock(row, 0, row, 0);
            m_propertyGrid->GoToCell(row, 0);
            m_propertyGrid->ShowCellEditControl();
            
            // updateSmartEditor(m_propertyGrid->GetGridCursorRow());
        }
        
        void EntityInspector::OnRemovePropertiesPressed(wxCommandEvent& event) {
            wxArrayInt selectedRows = m_propertyGrid->GetSelectedRows();
            wxArrayInt::reverse_iterator it, end;
            for (it = selectedRows.rbegin(), end = selectedRows.rend(); it != end; ++it)
                m_propertyGrid->DeleteRows(*it, 1);
            // updateSmartEditor(m_propertyGrid->GetGridCursorRow());
        }
        
        void EntityInspector::OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event) {
            event.Enable(!m_document->selectedObjects().empty());
        }
        
        void EntityInspector::OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event) {
            wxArrayInt selectedRows = m_propertyGrid->GetSelectedRows();
            event.Enable(!selectedRows.empty());
            
            wxArrayInt::const_iterator it, end;
            for (it = selectedRows.begin(), end = selectedRows.end(); it != end; ++it) {
                wxGridCellAttr* attr = m_propertyTable->GetAttr(*it, 0, wxGridCellAttr::Cell);
                if (attr != NULL && attr->IsReadOnly()) {
                    event.Enable(false);
                    return;
                }
            }
        }

        wxWindow* EntityInspector::createPropertyEditor(wxWindow* parent) {
            wxPanel* propertyEditorPanel = new wxPanel(parent);
            
            m_propertyTable = new EntityPropertyGridTable(m_document, m_controller);
            
            m_propertyGrid = new wxGrid(propertyEditorPanel, CommandIds::EntityInspector::EntityPropertyViewId, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_propertyGrid->Bind(wxEVT_SIZE, &EntityInspector::OnPropertyGridSize, this);
            m_propertyGrid->Bind(wxEVT_GRID_SELECT_CELL, &EntityInspector::OnPropertyGridSelectCell, this);
            m_propertyGrid->SetTable(m_propertyTable, true, wxGrid::wxGridSelectRows);
            m_propertyGrid->SetUseNativeColLabels();
            m_propertyGrid->UseNativeColHeader();
            m_propertyGrid->SetDefaultCellBackgroundColour(*wxWHITE);
            m_propertyGrid->HideRowLabels();
            
            m_propertyGrid->DisableColResize(0);
            m_propertyGrid->DisableColResize(1);
            m_propertyGrid->DisableDragColMove();
            m_propertyGrid->DisableDragCell();
            m_propertyGrid->DisableDragColSize();
            m_propertyGrid->DisableDragGridSize();
            m_propertyGrid->DisableDragRowSize();
            m_propertyGrid->Bind(wxEVT_GRID_TABBING, &EntityInspector::OnPropertyGridTab, this);
            m_propertyGrid->Bind(wxEVT_MOTION, &EntityInspector::OnPropertyGridMouseMove, this);
            m_propertyGrid->Bind(wxEVT_UPDATE_UI, &EntityInspector::OnUpdatePropertyViewOrAddPropertiesButton, this);
            
            // TODO: implemented better TAB behavior once wxWidgets 2.9.5 is out
            // see http://docs.wxwidgets.org/trunk/classwx_grid_event.html for wxEVT_GRID_TABBING
            
            /*
            wxPanel* smartPropertyEditorPanel = new wxPanel(propertyEditorPanel);
            m_smartPropertyEditorManager = new SmartPropertyEditorManager(smartPropertyEditorPanel, m_documentViewHolder);
            
            wxSizer* propertyEditorSizer = new wxBoxSizer(wxVERTICAL);
            propertyEditorSizer->Add(m_propertyGrid, 1, wxEXPAND);
            propertyEditorSizer->AddSpacer(LayoutConstants::ControlMargin);
            propertyEditorSizer->Add(smartPropertyEditorPanel, 0, wxEXPAND);
            propertyEditorSizer->SetMinSize(wxDefaultSize.x, 300);
             */
            
            m_addPropertyButton = new wxButton(propertyEditorPanel, CommandIds::EntityInspector::AddEntityPropertyButtonId, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_removePropertiesButton = new wxButton(propertyEditorPanel, CommandIds::EntityInspector::RemoveEntityPropertiesButtonId, wxT("-"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            
            m_addPropertyButton->Bind(wxEVT_BUTTON, &EntityInspector::OnAddPropertyPressed, this);
            m_addPropertyButton->Bind(wxEVT_UPDATE_UI, &EntityInspector::OnUpdatePropertyViewOrAddPropertiesButton, this);
            m_removePropertiesButton->Bind(wxEVT_BUTTON, &EntityInspector::OnRemovePropertiesPressed, this);
            m_removePropertiesButton->Bind(wxEVT_UPDATE_UI, &EntityInspector::OnUpdateRemovePropertiesButton, this);
            
            wxSizer* propertyViewButtonsSizer = new wxBoxSizer(wxVERTICAL);
            propertyViewButtonsSizer->Add(m_addPropertyButton, 0, wxEXPAND);
            propertyViewButtonsSizer->AddSpacer(LayoutConstants::ControlMargin);
            propertyViewButtonsSizer->Add(m_removePropertiesButton, 0, wxEXPAND);
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_propertyGrid, 1, wxEXPAND);
            outerSizer->SetItemMinSize(m_propertyGrid, wxDefaultSize.x, 300);
            outerSizer->AddSpacer(LayoutConstants::ControlMargin);
            outerSizer->Add(propertyViewButtonsSizer, 0, wxEXPAND);
            
            propertyEditorPanel->SetSizerAndFit(outerSizer);
            
            return propertyEditorPanel;
        }
        
        void EntityInspector::updatePropertyGrid() {
            m_propertyTable->update();
        }
        
        void EntityInspector::updateEntityBrowser() {
        }

        wxWindow* EntityInspector::createEntityBrowser(wxWindow* parent) {
            return new wxPanel(parent);
        }
    }
}
