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

#include "EntityBrowser.h"

#include "Model/EntityDefinitionManager.h"
#include "View/CommandIds.h"
#include "View/LayoutConstants.h"
#include "View/EntityBrowserCanvas.h"

#include <wx/tglbtn.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        namespace LayoutConstants {
#if defined _WIN32
            static const int ChoiceLeftMargin                   = 0;
            static const int BrowserControlsHorizontalMargin    = 3;
#elif defined __APPLE__
            static const int ChoiceLeftMargin                   = 1;
            static const int BrowserControlsHorizontalMargin    = 3;
#elif defined __linux__
            static const int ChoiceLeftMargin                   = 0;
            static const int BrowserControlsHorizontalMargin    = 3;
#endif
        }
        
        BEGIN_EVENT_TABLE(EntityBrowser, wxPanel)
        EVT_CHOICE(CommandIds::EntityInspector::EntityBrowserSortOrderChoiceId, EntityBrowser::OnSortOrderChanged)
        EVT_TOGGLEBUTTON(CommandIds::EntityInspector::EntityBrowserGroupButtonId, EntityBrowser::OnGroupButtonToggled)
        EVT_TOGGLEBUTTON(CommandIds::EntityInspector::EntityBrowserUsedButtonId, EntityBrowser::OnUsedButtonToggled)
        EVT_TEXT(CommandIds::EntityInspector::EntityBrowserFilterBoxId, EntityBrowser::OnFilterPatternChanged)
        END_EVENT_TABLE()
        
        EntityBrowser::EntityBrowser(wxWindow* parent, wxWindowID windowId, DocumentViewHolder& documentViewHolder) :
        wxPanel(parent, windowId) {
            wxString sortOrders[2] = {wxT("Name"), wxT("Usage")};
            m_sortOrderChoice = new wxChoice(this, CommandIds::EntityInspector::EntityBrowserSortOrderChoiceId, wxDefaultPosition, wxDefaultSize, 2, sortOrders);
            
            m_groupButton = new wxToggleButton(this, CommandIds::EntityInspector::EntityBrowserGroupButtonId, wxT("Group"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            m_usedButton = new wxToggleButton(this, CommandIds::EntityInspector::EntityBrowserUsedButtonId, wxT("Used"), wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxBU_EXACTFIT);
            
            m_filterBox = new wxSearchCtrl(this, CommandIds::EntityInspector::EntityBrowserFilterBoxId);
            m_filterBox->ShowCancelButton(true);
            
            wxSizer* controlSizer = new wxBoxSizer(wxHORIZONTAL);
            controlSizer->AddSpacer(LayoutConstants::ChoiceLeftMargin);
            controlSizer->Add(m_sortOrderChoice, 0, wxEXPAND);
            controlSizer->AddSpacer(LayoutConstants::BrowserControlsHorizontalMargin);
            controlSizer->Add(m_groupButton, 0, wxEXPAND);
            controlSizer->AddSpacer(LayoutConstants::BrowserControlsHorizontalMargin);
            controlSizer->Add(m_usedButton, 0, wxEXPAND);
            controlSizer->AddSpacer(LayoutConstants::BrowserControlsHorizontalMargin);
            controlSizer->Add(m_filterBox, 1, wxEXPAND);
            
            wxPanel* browserPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
            m_scrollBar = new wxScrollBar(browserPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSB_VERTICAL);
            m_canvas = new EntityBrowserCanvas(browserPanel, wxID_ANY, m_scrollBar, documentViewHolder);
            
            wxSizer* browserPanelSizer = new wxBoxSizer(wxHORIZONTAL);
            browserPanelSizer->Add(m_canvas, 1, wxEXPAND);
            browserPanelSizer->Add(m_scrollBar, 0, wxEXPAND);
            browserPanel->SetSizerAndFit(browserPanelSizer);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(controlSizer, 0, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(browserPanel, 1, wxEXPAND);
            
            SetSizerAndFit(outerSizer);
        }
        
        void EntityBrowser::reload() {
            m_canvas->clear();
            m_canvas->reload();
        }
        
        void EntityBrowser::OnSortOrderChanged(wxCommandEvent& event) {
            Model::EntityDefinitionManager::SortOrder sortOrder = event.GetSelection() == 0 ? Model::EntityDefinitionManager::Name : Model::EntityDefinitionManager::Usage;
            m_canvas->setSortOrder(sortOrder);
        }
        
        void EntityBrowser::OnGroupButtonToggled(wxCommandEvent& event) {
            m_canvas->setGroup(m_groupButton->GetValue());
        }
        
        void EntityBrowser::OnUsedButtonToggled(wxCommandEvent& event) {
            m_canvas->setHideUnused(m_usedButton->GetValue());
        }
        
        void EntityBrowser::OnFilterPatternChanged(wxCommandEvent& event) {
            m_canvas->setFilterText(m_filterBox->GetValue().ToStdString());
        }
    }
}
