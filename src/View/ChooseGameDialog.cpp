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

#include "ChooseGameDialog.h"

#include "PreferenceManager.h"
#include "View/GameListBox.h"
#include "View/GameSelectedCommand.h"
#include "View/ViewConstants.h"
#include "TrenchBroomApp.h"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        String ChooseGameDialog::ShowNewDocument(wxWindow* parent) {
            ChooseGameDialog dialog(parent, "Select Game", "Select a game from the list on the right, then click OK. Once the new document is created, you can set up mod directories, entity definitions and textures by going to the map inspector, the entity inspector and the face inspector, respectively.");
            if (dialog.ShowModal() == wxID_OK)
                return dialog.selectedGameName();
            return "";
        }
        
        String ChooseGameDialog::ShowOpenDocument(wxWindow* parent) {
            ChooseGameDialog dialog(parent, "Select Game", "TrenchBroom was unable to detect the game for the map document. Please choose a game in the game list and click OK.");
            if (dialog.ShowModal() == wxID_OK)
                return dialog.selectedGameName();
            return "";
        }
        
        ChooseGameDialog::~ChooseGameDialog() {
            unbindObservers();
        }
        
        const String ChooseGameDialog::selectedGameName() const {
            return m_gameListBox->selectedGameName();
        }
        
        void ChooseGameDialog::OnGameSelected(GameSelectedCommand& event) {
            EndModal(wxID_OK);
        }
        
        void ChooseGameDialog::OnOpenPreferencesClicked(wxCommandEvent& event) {
            TrenchBroomApp& app = TrenchBroomApp::instance();
            app.openPreferences();
        }
        
        void ChooseGameDialog::OnUpdateOkButton(wxUpdateUIEvent& event) {
            event.Enable(m_gameListBox->GetSelectedCount() > 0);
        }
        
        ChooseGameDialog::ChooseGameDialog(wxWindow* parent, const wxString& title, const wxString& infoText) :
        wxDialog(parent, wxID_ANY, "Create New Map") {
            createGui(title, infoText);
            bindEvents();
            bindObservers();
            CentreOnParent();
        }
        
        void ChooseGameDialog::createGui(const wxString& title, const wxString& infoText) {
            wxPanel* infoPanel = createInfoPanel(this, title, infoText);
            wxPanel* gameList = createGameList(this);
            
            wxBoxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->Add(infoPanel, 0, wxEXPAND);
            innerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
            innerSizer->Add(gameList, 1, wxEXPAND);
            innerSizer->SetItemMinSize(gameList, 300, wxDefaultSize.y);
            
            wxSizer* buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
            
            wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 1, wxEXPAND);
            outerSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::ChooseGameDialogButtonTopMargin);
            outerSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::ChooseGameDialogButtonSideMargin);
            outerSizer->AddSpacer(LayoutConstants::ChooseGameDialogButtonBottomMargin);
            
            SetSizerAndFit(outerSizer);
        }
        
        wxPanel* ChooseGameDialog::createInfoPanel(wxWindow* parent, const wxString& title, const wxString& infoText) {
            wxPanel* infoPanel = new wxPanel(parent);
            infoPanel->SetBackgroundColour(*wxWHITE);
            
            wxStaticText* header = new wxStaticText(infoPanel, wxID_ANY, title);
            header->SetFont(header->GetFont().Larger().Larger().Bold());
            
            wxStaticText* info = new wxStaticText(infoPanel, wxID_ANY, infoText);
            info->Wrap(250);
            
            wxStaticText* setupMsg = new wxStaticText(infoPanel, wxID_ANY, "To set up the game paths, click on the button below to open the preferences dialog.");
            setupMsg->Wrap(250);
            
            m_openPreferencesButton = new wxButton(infoPanel, wxID_ANY, "Open Preferences...");
            m_openPreferencesButton->SetToolTip("Open the preferences dialog to edit game paths");
            
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->AddSpacer(20);
            sizer->Add(header, 0, wxLEFT | wxRIGHT, 20);
            sizer->AddSpacer(20);
            sizer->Add(info, 0, wxLEFT | wxRIGHT, 20);
            sizer->AddSpacer(10);
            sizer->Add(setupMsg, 0, wxLEFT | wxRIGHT, 20);
            sizer->AddSpacer(10);
            sizer->Add(m_openPreferencesButton, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 20);
            sizer->AddSpacer(20);
            infoPanel->SetSizerAndFit(sizer);
            
            return infoPanel;
        }
        
        wxPanel* ChooseGameDialog::createGameList(wxWindow* parent) {
            m_gameListBox = new GameListBox(parent);
            m_gameListBox->SetToolTip("Double click on a game to select it");
            return m_gameListBox;
        }
        
        void ChooseGameDialog::bindEvents() {
            m_gameListBox->Bind(EVT_GAME_SELECTED_EVENT, EVT_GAME_SELECTED_HANDLER(ChooseGameDialog::OnGameSelected), this);
            m_openPreferencesButton->Bind(wxEVT_BUTTON, &ChooseGameDialog::OnOpenPreferencesClicked, this);
            FindWindow(wxID_OK)->Bind(wxEVT_UPDATE_UI, &ChooseGameDialog::OnUpdateOkButton, this);
        }
        
        void ChooseGameDialog::bindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &ChooseGameDialog::preferenceDidChange);
            
        }
        
        void ChooseGameDialog::unbindObservers() {
            PreferenceManager& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &ChooseGameDialog::preferenceDidChange);
        }
        
        void ChooseGameDialog::preferenceDidChange(const IO::Path& path) {
            m_gameListBox->reloadGameInfos();
        }
    }
}
