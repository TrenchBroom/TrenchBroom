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
#include "View/BorderLine.h"
#include "View/GameListBox.h"
#include "View/GameSelectionCommand.h"
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        GamesPreferencePane::GamesPreferencePane(wxWindow* parent) :
        PreferencePane(parent) {
            createGui();
            bindEvents();
        }
        
        void GamesPreferencePane::OnGameSelectionChanged(GameSelectionCommand& event) {
            if (IsBeingDeleted()) return;

            updateControls();
        }

        void GamesPreferencePane::OnChooseGamePathClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

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
            m_gameListBox = new GameListBox(this, wxBORDER_NONE);
            m_gameListBox->selectGame(0);
            
            wxWindow* gamePreferences = createGamePreferences();
            
            wxSizer* prefMarginSizer = new wxBoxSizer(wxVERTICAL);
            prefMarginSizer->AddSpacer(LayoutConstants::WideVMargin);
            prefMarginSizer->Add(gamePreferences, 0, wxEXPAND);
            prefMarginSizer->AddSpacer(LayoutConstants::ChoiceSizeDelta);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_gameListBox, 0, wxEXPAND);
            sizer->Add(new BorderLine(this, BorderLine::Direction_Vertical), 0, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->Add(prefMarginSizer, 1, wxEXPAND);
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->SetItemMinSize(m_gameListBox, 200, 200);
            
            SetSizer(sizer);
            SetMinSize(wxSize(600, 300));
            SetBackgroundColour(*wxWHITE);
        }
        
        wxWindow* GamesPreferencePane::createGamePreferences() {
            wxPanel* box = new wxPanel(this);
            box->SetBackgroundColour(*wxWHITE);
            
            wxStaticText* gamePathLabel = new wxStaticText(box, wxID_ANY, "Game Path");
            gamePathLabel->SetFont(gamePathLabel->GetFont().Bold());
            m_gamePathValueLabel = new wxStaticText(box, wxID_ANY, "Not set");
            m_chooseGamePathButton = new wxButton(box, wxID_ANY, "Choose...");
            
            wxFlexGridSizer* sizer = new wxFlexGridSizer(3, LayoutConstants::WideHMargin, LayoutConstants::WideVMargin);
            sizer->AddGrowableCol(1);
            sizer->Add(gamePathLabel, 0, wxALIGN_CENTER_VERTICAL);
            sizer->Add(m_gamePathValueLabel, 0, wxALIGN_CENTER_VERTICAL);
            sizer->Add(m_chooseGamePathButton, 0, wxALIGN_CENTER_VERTICAL);
            sizer->SetItemMinSize(m_chooseGamePathButton, wxDefaultCoord, m_chooseGamePathButton->GetSize().y + 1);
            
            box->SetSizer(sizer);
            return box;
        }

        void GamesPreferencePane::bindEvents() {
            m_gameListBox->Bind(GAME_SELECTION_CHANGE_EVENT, &GamesPreferencePane::OnGameSelectionChanged, this);
            m_chooseGamePathButton->Bind(wxEVT_BUTTON, &GamesPreferencePane::OnChooseGamePathClicked, this);
        }
        
        bool GamesPreferencePane::doCanResetToDefaults() {
            return false;
        }

        void GamesPreferencePane::doResetToDefaults() {}

        void GamesPreferencePane::doUpdateControls() {
            const String gameName = m_gameListBox->selectedGameName();
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(gameName);
            m_gamePathValueLabel->SetLabel(gamePath.isEmpty() ? "not set" : gamePath.asString());
            m_gameListBox->reloadGameInfos();
            Layout();
        }
        
        bool GamesPreferencePane::doValidate() {
            return true;
        }
    }
}
