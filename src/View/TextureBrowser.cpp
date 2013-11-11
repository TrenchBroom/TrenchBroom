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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TextureBrowser.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/TextureManager.h"
#include "View/LayoutConstants.h"
#include "View/MapDocument.h"
#include "View/TextureBrowserView.h"
#include "View/TextureSelectedCommand.h"

#include <wx/choice.h>
#include <wx/event.h>
#include <wx/tglbtn.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TextureBrowser::TextureBrowser(wxWindow* parent, Renderer::RenderResources& resources, MapDocumentPtr document) :
        wxPanel(parent) {
            const wxString sortOrders[2] = { _T("Name"), _T("Usage") };
            m_sortOrderChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, sortOrders);
            m_sortOrderChoice->SetSelection(0);
            m_sortOrderChoice->Bind(wxEVT_CHOICE, &TextureBrowser::OnSortOrderChanged, this);
            
            m_groupButton = new wxToggleButton(this, wxID_ANY, _("Group"), wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            m_groupButton->Bind(wxEVT_TOGGLEBUTTON, &TextureBrowser::OnGroupButtonToggled, this);
            m_usedButton = new wxToggleButton(this, wxID_ANY, _("Used"), wxDefaultPosition, wxDefaultSize, LayoutConstants::ToggleButtonStyle | wxBU_EXACTFIT);
            m_usedButton->Bind(wxEVT_TOGGLEBUTTON, &TextureBrowser::OnUsedButtonToggled, this);
            
            m_filterBox = new wxSearchCtrl(this, wxID_ANY);
            m_filterBox->ShowCancelButton(true);
            m_filterBox->Bind(wxEVT_TEXT, &TextureBrowser::OnFilterPatternChanged, this);
            
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
            m_view = new TextureBrowserView(browserPanel, wxID_ANY, m_scrollBar,
                                           resources,
                                           document->textureManager());
            m_view->Bind(EVT_TEXTURE_SELECTED_EVENT, EVT_TEXTURE_SELECTED_HANDLER(TextureBrowser::OnTextureSelected), this);
            
            wxSizer* browserPanelSizer = new wxBoxSizer(wxHORIZONTAL);
            browserPanelSizer->Add(m_view, 1, wxEXPAND);
            browserPanelSizer->Add(m_scrollBar, 0, wxEXPAND);
            browserPanel->SetSizerAndFit(browserPanelSizer);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(controlSizer, 0, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(browserPanel, 1, wxEXPAND);
            
            SetSizerAndFit(outerSizer);

            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &TextureBrowser::preferenceDidChange);
        }
        
        TextureBrowser::~TextureBrowser() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &TextureBrowser::preferenceDidChange);
        }

        void TextureBrowser::reload() {
            if (m_view != NULL) {
                m_view->clear();
                m_view->reload();
            }
        }
        
        Assets::Texture* TextureBrowser::selectedTexture() const {
            return m_view->selectedTexture();
        }
        
        void TextureBrowser::setSelectedTexture(Assets::Texture* selectedTexture) {
            m_view->setSelectedTexture(selectedTexture);
        }

        void TextureBrowser::OnSortOrderChanged(wxCommandEvent& event) {
            const Assets::TextureManager::SortOrder sortOrder = event.GetSelection() == 0 ? Assets::TextureManager::Name : Assets::TextureManager::Usage;
            m_view->setSortOrder(sortOrder);
        }
        
        void TextureBrowser::OnGroupButtonToggled(wxCommandEvent& event) {
            m_view->setGroup(m_groupButton->GetValue());
        }
        
        void TextureBrowser::OnUsedButtonToggled(wxCommandEvent& event) {
            m_view->setHideUnused(m_usedButton->GetValue());
        }
        
        void TextureBrowser::OnFilterPatternChanged(wxCommandEvent& event) {
            m_view->setFilterText(m_filterBox->GetValue().ToStdString());
        }

        void TextureBrowser::OnTextureSelected(TextureSelectedCommand& event) {
            // let the event bubble up to our own listeners
            event.SetEventObject(this);
            event.SetId(GetId());
            ProcessEvent(event);
        }

        void TextureBrowser::preferenceDidChange(const IO::Path& path) {
            if (path == Preferences::TextureBrowserIconSize.path())
                reload();
            else
                m_view->Refresh();
        }
 }
}
