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

#include "View/CommandIds.h"
#include "View/DocumentViewHolder.h"
#include "View/LayoutConstants.h"

#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statline.h>

namespace TrenchBroom {
    namespace View {
        BEGIN_EVENT_TABLE(EntityInspector, wxPanel)
        END_EVENT_TABLE()

        wxWindow* EntityInspector::createPropertyEditor(wxWindow* parent) {
            wxPanel* propertyEditorPanel = new wxPanel(parent);

            m_propertyViewModel = new EntityPropertyDataViewModel(m_documentViewHolder.document());
            m_propertyView = new wxDataViewCtrl(propertyEditorPanel, CommandIds::EntityInspector::EntityPropertyViewId, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_VERT_RULES);
            m_propertyView->AssociateModel(m_propertyViewModel.get());
            m_keyColumn = m_propertyView->AppendTextColumn("Key", 0, wxDATAVIEW_CELL_EDITABLE, 100, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
            m_valueColumn = m_propertyView->AppendTextColumn("Value", 1, wxDATAVIEW_CELL_EDITABLE, 190, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE);
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_propertyView, 1, wxEXPAND);
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
    }
}