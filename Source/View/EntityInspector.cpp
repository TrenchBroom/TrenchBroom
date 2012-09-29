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
#include "View/LayoutConstants.h"

#include <wx/button.h>
#include <wx/dataview.h>
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

            m_propertyViewModel = new EntityPropertyDataViewModel(m_documentViewHolder.document());
            m_propertyView = new wxDataViewCtrl(propertyEditorPanel, CommandIds::EntityInspector::EntityPropertyViewId, wxDefaultPosition, wxDefaultSize/*, wxDV_HORIZ_RULES | wxDV_VERT_RULES*/);
            m_propertyView->AssociateModel(m_propertyViewModel.get());
            m_keyColumn = m_propertyView->AppendTextColumn("Key", 0, wxDATAVIEW_CELL_EDITABLE, 100, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
            m_valueColumn = m_propertyView->AppendTextColumn("Value", 1, wxDATAVIEW_CELL_EDITABLE, 170, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
            
            m_addPropertyButton = new wxButton(propertyEditorPanel, CommandIds::EntityInspector::AddEntityPropertyButtonId, wxT("+"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_removePropertiesButton = new wxButton(propertyEditorPanel, CommandIds::EntityInspector::RemoveEntityPropertiesButtonId, wxT("-"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            
            wxSizer* propertyViewButtonsSizer = new wxBoxSizer(wxVERTICAL);
            propertyViewButtonsSizer->Add(m_addPropertyButton, 0, wxEXPAND);
            propertyViewButtonsSizer->AddSpacer(LayoutConstants::ControlMargin);
            propertyViewButtonsSizer->Add(m_removePropertiesButton, 0, wxEXPAND);

            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_propertyView, 1, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::ControlMargin);
            outerSizer->Add(propertyViewButtonsSizer, 0, wxEXPAND);
            propertyEditorPanel->SetSizerAndFit(outerSizer);
            
            return propertyEditorPanel;
        }
        
        wxWindow* EntityInspector::createEntityBrowser(wxWindow* parent, wxGLContext* sharedContext) {
            return new wxPanel(parent);
        }

        EntityInspector::EntityInspector(wxWindow* parent, DocumentViewHolder& documentViewHolder, wxGLContext* sharedContext) :
        wxPanel(parent),
        m_documentViewHolder(documentViewHolder) {
            
            wxSplitterWindow* inspectorSplitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3DSASH | wxSP_LIVE_UPDATE);
            inspectorSplitter->SetSashGravity(1.0f);
            inspectorSplitter->SetMinimumPaneSize(50);

            inspectorSplitter->SplitHorizontally(createPropertyEditor(inspectorSplitter), createEntityBrowser(inspectorSplitter, sharedContext));
            
            // creates 5 pixel border inside the page
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(inspectorSplitter, 1, wxEXPAND | wxALL, LayoutConstants::NotebookPageInnerMargin);
            SetSizerAndFit(outerSizer);
        }

        void EntityInspector::update() {
            m_propertyViewModel->update();
        }

        void EntityInspector::OnAddPropertyPressed(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;

            unsigned int row = m_propertyViewModel->addNewRow();
            wxDataViewItem item = m_propertyViewModel->GetItem(row);
            m_propertyView->EditItem(item, m_keyColumn);
        }
        
        void EntityInspector::OnRemovePropertiesPressed(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            
            wxDataViewItemArray selection;
            if (m_propertyView->GetSelections(selection) > 0)
                m_propertyViewModel->removeRows(selection);
        }
        
        void EntityInspector::OnUpdatePropertyViewOrAddPropertiesButton(wxUpdateUIEvent& event) {
            if (!m_documentViewHolder.valid()) {
                event.Enable(false);
                return;
            }
            
            Model::EditStateManager& editStateManager = m_documentViewHolder.document().editStateManager();
            event.Enable(!editStateManager.selectedEntities().empty());
        }

        void EntityInspector::OnUpdateRemovePropertiesButton(wxUpdateUIEvent& event) {
            if (!m_documentViewHolder.valid()) {
                event.Enable(false);
                return;
            }
            
            Model::EditStateManager& editStateManager = m_documentViewHolder.document().editStateManager();
            event.Enable(!editStateManager.selectedEntities().empty() && m_propertyView->GetSelectedItemsCount() > 0);
        }

    }
}