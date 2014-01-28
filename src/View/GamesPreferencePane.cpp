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
#include "View/ViewConstants.h"

#include <wx/button.h>
#include <wx/choice.h>
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
        
        void GamesPreferencePane::OnGameSelectionChanged(wxCommandEvent& event) {
            updateControls();
        }

        void GamesPreferencePane::OnChooseGamePathClicked(wxCommandEvent& event) {
            const wxString pathStr = ::wxDirSelector("Choose game directory", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (!pathStr.empty()) {
                const IO::Path gamePath(pathStr.ToStdString());
                const String gameName = m_gameSelectionChoice->GetString(m_gameSelectionChoice->GetSelection()).ToStdString();
                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                gameFactory.setGamePath(gameName, gamePath);
                
                updateControls();
            }
        }

        void GamesPreferencePane::createGui() {
            wxWindow* gameSelectionBox = createGameSelectionBox();
            wxWindow* gamePreferences = createGamePreferences();
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(gameSelectionBox, 0, wxEXPAND);
            outerSizer->AddSpacer(LayoutConstants::ControlVerticalMargin);
            outerSizer->Add(gamePreferences, 0, wxEXPAND);
            
#ifdef __APPLE__
            outerSizer->SetItemMinSize(gameSelectionBox, wxDefaultSize.x, gameSelectionBox->GetSize().y + 2);
#endif
            
            SetSizer(outerSizer);
            SetMinSize(wxSize(600, 300));
        }
        
        wxWindow* GamesPreferencePane::createGameSelectionBox() {
            wxPanel* container = new wxPanel(this, wxID_ANY);
            container->SetBackgroundColour(::wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
            
            wxStaticText* gameSelectionChoiceLabel = new wxStaticText(container, wxID_ANY, "Select a game from the list: ");
            
            const Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const StringList gameList = gameFactory.gameList();
            
            wxArrayString wxGameList;
            for (size_t i = 0; i < gameList.size(); ++i)
                wxGameList.Add(wxString(gameList[i]));
            m_gameSelectionChoice = new wxChoice(container, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxGameList);
            m_gameSelectionChoice->SetSelection(0);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(gameSelectionChoiceLabel, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->Add(m_gameSelectionChoice, 0, wxALIGN_CENTER_VERTICAL);
            
            wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxALL, LayoutConstants::HighlightBoxMargin);
            
            container->SetSizerAndFit(outerSizer);
            return container;
        }
        
        wxWindow* GamesPreferencePane::createGamePreferences() {
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, "Game Preferences");
            
            wxStaticText* gamePathLabel = new wxStaticText(box, wxID_ANY, "Game Path");
            m_gamePathValueLabel = new wxStaticText(box, wxID_ANY, "Not set");
            m_chooseGamePathButton = new wxButton(box, wxID_ANY, "Choose...");
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(3, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
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
            m_gameSelectionChoice->Bind(wxEVT_CHOICE, &GamesPreferencePane::OnGameSelectionChanged, this);
            m_chooseGamePathButton->Bind(wxEVT_BUTTON, &GamesPreferencePane::OnChooseGamePathClicked, this);
        }
        
        void GamesPreferencePane::updateControls() {
            const String gameName = m_gameSelectionChoice->GetString(m_gameSelectionChoice->GetSelection()).ToStdString();
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path gamePath = gameFactory.gamePath(gameName);
            m_gamePathValueLabel->SetLabel(gamePath.isEmpty() ? "not set" : gamePath.asString());
        }
        
        bool GamesPreferencePane::doValidate() {
            return true;
        }
    }
}
