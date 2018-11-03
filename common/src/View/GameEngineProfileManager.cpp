/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "GameEngineProfileManager.h"

#include "IO/Path.h"
#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"
#include "View/BorderLine.h"
#include "View/GameEngineProfileListBox.h"
#include "View/GameEngineProfileEditor.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/settings.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        GameEngineProfileManager::GameEngineProfileManager(wxWindow* parent, Model::GameEngineConfig& config) :
        wxPanel(parent),
        m_config(config),
        m_profileList(nullptr),
        m_profileEditor(nullptr) {
            auto* listPanel = new TitledPanel(this, "Profiles");
            auto* editorPanel = new TitledPanel(this, "Details");

            m_profileList = new GameEngineProfileListBox(listPanel->getPanel(), m_config);
            m_profileEditor = new GameEngineProfileEditor(editorPanel->getPanel());
            
            auto* addProfileButton = createBitmapButton(listPanel->getPanel(), "Add.png", "Add profile");
            auto* removeProfileButton = createBitmapButton(listPanel->getPanel(), "Remove.png", "Remove the selected profile");
            
            addProfileButton->Bind(wxEVT_BUTTON, &GameEngineProfileManager::OnAddProfile, this);
            removeProfileButton->Bind(wxEVT_BUTTON, &GameEngineProfileManager::OnRemoveProfile, this);
            addProfileButton->Bind(wxEVT_UPDATE_UI, &GameEngineProfileManager::OnUpdateAddProfileButtonUI, this);
            removeProfileButton->Bind(wxEVT_UPDATE_UI, &GameEngineProfileManager::OnUpdateRemoveProfileButtonUI, this);
            
            auto* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(addProfileButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(removeProfileButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();
            
            auto* listSizer = new wxBoxSizer(wxVERTICAL);
            listSizer->Add(m_profileList, 1, wxEXPAND);
            listSizer->Add(new BorderLine(listPanel->getPanel(), BorderLine::Direction_Horizontal), 0, wxEXPAND);
            listSizer->Add(buttonSizer);
            listPanel->getPanel()->SetSizer(listSizer);
            
            auto* editorSizer = new wxBoxSizer(wxVERTICAL);
            editorSizer->Add(m_profileEditor, 1, wxEXPAND);
            editorPanel->getPanel()->SetSizer(editorSizer);
            
            auto* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(listPanel, 0, wxEXPAND);
            outerSizer->Add(new BorderLine(this, BorderLine::Direction_Vertical), 0, wxEXPAND);
            outerSizer->Add(editorPanel, 1, wxEXPAND);
            outerSizer->SetItemMinSize(listPanel, wxSize(200, 200));
            SetSizer(outerSizer);
            
            m_profileList->Bind(wxEVT_LISTBOX, &GameEngineProfileManager::OnProfileSelectionChanged, this);
        }
        
        void GameEngineProfileManager::OnAddProfile(wxCommandEvent& event) {
            m_config.addProfile(new Model::GameEngineProfile("", IO::Path(), ""));
            m_profileList->SetSelection(static_cast<int>(m_config.profileCount() - 1));
        }
        
        void GameEngineProfileManager::OnRemoveProfile(wxCommandEvent& event) {
            const int index = m_profileList->GetSelection();
            assert(index != wxNOT_FOUND);
            
            if (m_config.profileCount() == 1) {
                m_profileList->SetSelection(wxNOT_FOUND);
                m_config.removeProfile(static_cast<size_t>(index));
            } else if (index > 0) {
                m_profileList->SetSelection(index - 1);
                m_config.removeProfile(static_cast<size_t>(index));
            } else {
                m_profileList->SetSelection(1);
                m_config.removeProfile(static_cast<size_t>(index));
                m_profileList->SetSelection(0);
            }
        }
        
        void GameEngineProfileManager::OnUpdateAddProfileButtonUI(wxUpdateUIEvent& event) {
            event.Enable(true);
        }
        
        void GameEngineProfileManager::OnUpdateRemoveProfileButtonUI(wxUpdateUIEvent& event) {
            event.Enable(m_profileList->GetSelection() != wxNOT_FOUND);
        }
        
        void GameEngineProfileManager::OnProfileSelectionChanged(wxCommandEvent& event) {
            const int selection = m_profileList->GetSelection();
            if (selection != wxNOT_FOUND) {
                Model::GameEngineProfile* profile = m_config.profile(static_cast<size_t>(selection));
                m_profileEditor->setProfile(profile);
            } else {
                m_profileEditor->setProfile(nullptr);
            }
        }
    }
}
