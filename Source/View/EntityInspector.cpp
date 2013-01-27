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

#include "EntityInspector.h"

#include "Model/EditStateManager.h"
#include "Model/MapDocument.h"
#include "View/CommandIds.h"
#include "View/DocumentViewHolder.h"
#include "View/EntityBrowser.h"
#include "View/EntityPropertyGridTable.h"
#include "View/LayoutConstants.h"
#include "View/SmartPropertyEditor.h"

#include <wx/button.h>
#include <wx/grid.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statline.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EntityInspector, wxPanel)
        EVT_BUTTON(CommandIds::EntityInspector::AddEntityPropertyButtonId, EntityInspector::OnAddPropertyPressed)
        EVT_BUTTON(CommandIds::EntityInspector::RemoveEntityPropertiesButtonId, EntityInspector::OnRemovePropertiesPressed)
        EVT_UPDATE_UI(CommandIds::EntityInspector::EntityPropertyViewId, EntityInspector::OnUpdatePropertyViewOrAddPropertiesButton)
        EVT_UPDATE_UI(CommandIds::EntityInspector::AddEntityPropertyButtonId, EntityInspector::OnUpdatePropertyViewOrAddPropertiesButton)
        EVT_UPDATE_UI(CommandIds::EntityInspector::RemoveEntityPropertiesButtonId, EntityInspector::OnUpdateRemovePropertiesButton)
        END_EVENT_TABLE()

        wxWindow* EntityInspector::createPropertyEditor(wxWindow* parent) {
            wxPanel* propertyEditorPanel = new wxPanel(parent);
            
            m_propertyTable = new EntityPropertyGridTable(m_documentViewHolder.document());
            
            m_propertyGrid = new wxGrid(propertyEditorPanel, CommandIds::EntityInspector::EntityPropertyViewId);
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

            // TODO: implemented better TAB behavior once wxWidgets 2.9.5 is out
            // see http://docs.wxwidgets.org/trunk/classwx_grid_event.html for wxEVT_GRID_TABBING

            wxPanel* smartPropertyEditorPanel = new wxPanel(propertyEditorPanel);
            m_smartPropertyEditorManager = new SmartPropertyEditorManager(smartPropertyEditorPanel, m_documentViewHolder);
            
            wxSizer* propertyEditorSizer = new wxBoxSizer(wxVERTICAL);
            propertyEditorSizer->Add(m_propertyGrid, 1, wxEXPAND);
            propertyEditorSizer->AddSpacer(LayoutConstants::ControlMargin);
            propertyEditorSizer->Add(smartPropertyEditorPanel, 0, wxEXPAND);
            propertyEditorSizer->SetMinSize(wxDefaultSize.x, 300);
            
            m_addPropertyButton = new wxButton(propertyEditorPanel, CommandIds::EntityInspector::AddEntityPropertyButtonId, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_removePropertiesButton = new wxButton(propertyEditorPanel, CommandIds::EntityInspector::RemoveEntityPropertiesButtonId, wxT("-"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            
            wxSizer* propertyViewButtonsSizer = new wxBoxSizer(wxVERTICAL);
            propertyViewButtonsSizer->Add(m_addPropertyButton, 0, wxEXPAND);
            propertyViewButtonsSizer->AddSpacer(LayoutConstants::ControlMargin);
            propertyViewButtonsSizer->Add(m_removePropertiesButton, 0, wxEXPAND);

            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(propertyEditorSizer, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::ControlMargin);
            outerSizer->Add(propertyViewButtonsSizer, 0, wxEXPAND);
            
            propertyEditorPanel->SetSizerAndFit(outerSizer);
            
            return propertyEditorPanel;
        }
        
        wxWindow* EntityInspector::createEntityBrowser(wxWindow* parent) {
            m_entityBrowser = new EntityBrowser(parent, CommandIds::EntityInspector::EntityBrowserId, m_documentViewHolder);
            return m_entityBrowser;
        }

        void EntityInspector::updateSmartEditor(int row) {
            Model::PropertyKey key = row >= 0 && row < m_propertyTable->GetNumberRows() ? m_propertyTable->GetValue(row, 0).ToStdString() : "";
            m_smartPropertyEditorManager->selectEditor(key);
        }

        EntityInspector::EntityInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder) :
        wxPanel(parent),
        m_documentViewHolder(documentViewHolder) {

            /*
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(0.0f);
            inspectorSplitter->SetMinimumPaneSize(50);
             */

            // creates 5 pixel border inside the page
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(createPropertyEditor(this), 0, wxEXPAND | wxLEFT | wxTOP | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            outerSizer->Add(createEntityBrowser(this), 1, wxEXPAND | wxLEFT | wxBOTTOM | wxRIGHT, LayoutConstants::NotebookPageInnerMargin);
            SetSizerAndFit(outerSizer);

            // inspectorSplitter->SplitHorizontally(createPropertyEditor(inspectorSplitter), createEntityBrowser(inspectorSplitter));
        }

        void EntityInspector::updateProperties() {
            m_propertyTable->update();
            updateSmartEditor(m_propertyGrid->GetGridCursorRow());
        }
        
        void EntityInspector::updateSmartEditor() {
            m_smartPropertyEditorManager->updateEditor();
        }
        
        void EntityInspector::updateEntityBrowser() {
            m_entityBrowser->reload();
        }

        void EntityInspector::OnPropertyGridSize(wxSizeEvent& event) {
            m_propertyGrid->SetColSize(0, 100);
            m_propertyGrid->SetColSize(1, event.GetSize().x - m_propertyGrid->GetColSize(0));
            event.Skip();
        }

        void EntityInspector::OnPropertyGridSelectCell(wxGridEvent& event) {
            updateSmartEditor(event.GetRow());
        }

        void EntityInspector::OnAddPropertyPressed(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            m_propertyGrid->AppendRows();
         
            m_propertyGrid->SetFocus();
            int row = m_propertyGrid->GetNumberRows() - 1;
            m_propertyGrid->SelectBlock(row, 0, row, 0);
            m_propertyGrid->GoToCell(row, 0);
            m_propertyGrid->ShowCellEditControl();

            updateSmartEditor(m_propertyGrid->GetGridCursorRow());
        }
        
        void EntityInspector::OnRemovePropertiesPressed(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            
            wxArrayInt selectedRows = m_propertyGrid->GetSelectedRows();
            wxArrayInt::reverse_iterator it, end;
            for (it = selectedRows.rbegin(), end = selectedRows.rend(); it != end; ++it)
                m_propertyGrid->DeleteRows(*it, 1);
            updateSmartEditor(m_propertyGrid->GetGridCursorRow());
        }
        
        void EntityInspector::OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event) {
            if (!m_documentViewHolder.valid()) {
                event.Enable(false);
                return;
            }
            
            Model::EditStateManager& editStateManager = m_documentViewHolder.document().editStateManager();
            event.Enable(!editStateManager.selectedEntities().empty() || !editStateManager.selectedBrushes().empty());
        }

        void EntityInspector::OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event) {
            if (!m_documentViewHolder.valid()) {
                event.Enable(false);
                return;
            }
            
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

    }
}
