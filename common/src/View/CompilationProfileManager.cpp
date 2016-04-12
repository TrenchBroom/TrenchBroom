/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CompilationProfileManager.h"

#include "Model/CompilationConfig.h"
#include "View/BorderLine.h"
#include "View/CompilationProfileListBox.h"
#include "View/CompilationProfileEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        CompilationProfileManager::CompilationProfileManager(wxWindow* parent, Model::CompilationConfig& config) :
        wxPanel(parent),
        m_config(config),
        m_listView(NULL),
        m_editor() {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            
            TitledPanel* listPanel = new TitledPanel(this, "Profiles");
            TitledPanel* editorPanel = new TitledPanel(this, "Details");
            listPanel->getPanel()->SetBackgroundColour(GetBackgroundColour());
            editorPanel->getPanel()->SetBackgroundColour(GetBackgroundColour());
            
            m_listView = new CompilationProfileListBox(listPanel->getPanel(), m_config);
            m_editor = new CompilationProfileEditor(editorPanel->getPanel());
            
            wxWindow* addProfileButton = createBitmapButton(listPanel->getPanel(), "Add.png", "Add profile");
            wxWindow* removeProfileButton = createBitmapButton(listPanel->getPanel(), "Remove.png", "Remove the selected profile");
            
            addProfileButton->Bind(wxEVT_BUTTON, &CompilationProfileManager::OnAddProfile, this);
            removeProfileButton->Bind(wxEVT_BUTTON, &CompilationProfileManager::OnRemoveProfile, this);
            addProfileButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileManager::OnUpdateAddProfileButtonUI, this);
            removeProfileButton->Bind(wxEVT_UPDATE_UI, &CompilationProfileManager::OnUpdateRemoveProfileButtonUI, this);
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addProfileButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeProfileButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();

            wxSizer* listSizer = new wxBoxSizer(wxVERTICAL);
            listSizer->Add(m_listView, 1, wxEXPAND);
            listSizer->Add(new BorderLine(listPanel->getPanel(), BorderLine::Direction_Horizontal), 0, wxEXPAND);
            listSizer->Add(buttonSizer);
            listPanel->getPanel()->SetSizer(listSizer);
            
            wxSizer* editorSizer = new wxBoxSizer(wxVERTICAL);
            editorSizer->Add(m_editor, 1, wxEXPAND);
            editorPanel->getPanel()->SetSizer(editorSizer);
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(listPanel, 0, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Vertical), 0, wxEXPAND);
            outerSizer->Add(editorPanel, 1, wxEXPAND);
            outerSizer->SetItemMinSize(listPanel, wxSize(200, 200));
            SetSizer(outerSizer);
            
            m_listView->Bind(wxEVT_LISTBOX, &CompilationProfileManager::OnProfileSelectionChanged, this);
        }

        void CompilationProfileManager::OnAddProfile(wxCommandEvent& event) {
        }
        
        void CompilationProfileManager::OnRemoveProfile(wxCommandEvent& event) {
        }
        
        void CompilationProfileManager::OnUpdateAddProfileButtonUI(wxUpdateUIEvent& event) {
            event.Enable(true);
        }
        
        void CompilationProfileManager::OnUpdateRemoveProfileButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_listView->GetSelection() != wxNOT_FOUND);
        }

        void CompilationProfileManager::OnProfileSelectionChanged(wxCommandEvent& event) {
            const int selection = m_listView->GetSelection();
            if (selection != wxNOT_FOUND) {
                Model::CompilationProfile* profile = m_config.profile(static_cast<size_t>(selection));
                m_editor->setProfile(profile);
            } else {
                m_editor->setProfile(NULL);
            }
        }
    }
}
