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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GamesPreferencePane.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/GameListBox.h"
#include "View/GameSelectionCommand.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/statbox.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        GamesPreferencePane::GamesPreferencePane(wxWindow* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
            updateControls();
        }
        
        void GamesPreferencePane::OnGameSelectionChanged(GameSelectionCommand& event) {
            updateControls();
        }

        void GamesPreferencePane::OnChooseGamePathClicked(wxCommandEvent& event) {
            const wxString pathStr = ::wxDirSelector("Choose game directory", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (!pathStr.empty()) {
                const IO::Path gamePath(pathStr.ToStdString());
                const String gameName = m_gameListBox->selectedGameName();
                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                gameFactory.setGamePath(gameName, gamePath);
                
                updateControls();
            }
        }

        void GamesPreferencePane::createGui() {
            m_gameListBox = new GameListBox(this, wxBORDER_THEME);
            m_gameListBox->selectGame(0);
            
            wxWindow* gamePreferences = createGamePreferences();
            
            wxSizer* outerSizer = new wxBoxSizer(wxHORIZONTAL);
            outerSizer->Add(m_gameListBox, 0, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::WideVMargin);
            outerSizer->Add(gamePreferences, 1, wxEXPAND);
            outerSizer->SetItemMinSize(m_gameListBox, 200, 200);
            
            SetSizer(outerSizer);
            SetMinSize(wxSize(600, 300));
        }
        
        wxWindow* GamesPreferencePane::createGamePreferences() {
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, "Game Preferences");
            
            wxStaticText* gamePathLabel = new wxStaticText(box, wxID_ANY, "Game Path");
            m_gamePathValueLabel = new wxStaticText(box, wxID_ANY, "Not set");
            m_chooseGamePathButton = new wxButton(box, wxID_ANY, "Choose...");
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(3, LayoutConstants::WideHMargin, LayoutConstants::WideVMargin);
            innerSizer->AddGrowableCol(1);
            innerSizer->Add(gamePathLabel, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->Add(m_gamePathValueLabel, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->Add(m_chooseGamePathButton, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->SetItemMinSize(gamePathLabel, LayoutConstants::MinPreferenceLabelWidth, wxDefaultSize.y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxTopMargin);
            outerSizer->Add(innerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::StaticBoxSideMargin);
            outerSizer->AddSpacer(LayoutConstants::StaticBoxBottomMargin);
            
            box->SetSizerAndFit(outerSizer);
            return box;
        }

        void GamesPreferencePane::bindEvents() {
            m_gameListBox->Bind(EVT_GAME_SELECTION_CHANGE_EVENT, EVT_GAME_SELECTION_DBLCLICK_HANDLER(GamesPreferencePane::OnGameSelectionChanged), this);
            m_chooseGamePathButton->Bind(wxEVT_BUTTON, &GamesPreferencePane::OnChooseGamePathClicked, this);
        }
        
        void GamesPreferencePane::updateControls() {
            const String gameName = m_gameListBox->selectedGameName();
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(gameName);
            m_gamePathValueLabel->SetLabel(gamePath.isEmpty() ? "not set" : gamePath.asString());
            m_gameListBox->reloadGameInfos();
        }
        
        bool GamesPreferencePane::doValidate() {
            return true;
        }
    }
}
