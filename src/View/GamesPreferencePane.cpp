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

#include "GamesPreferencePane.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/Game.h"
#include "View/LayoutConstants.h"

#include <wx/button.h>
#include <wx/choice.h>
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
        
        void GamesPreferencePane::OnChooseGamePathClicked(wxCommandEvent& event) {
            wxDirDialog chooseQuakePathDialog(NULL, _("Choose game directory"), _(""), wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (chooseQuakePathDialog.ShowModal() == wxID_OK) {
                String quakePath = chooseQuakePathDialog.GetPath().ToStdString();
                PreferenceManager& prefs = PreferenceManager::instance();
                // prefs.set(Preferences::QuakePath, quakePath);
                
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
            
            wxStaticText* gameSelectionChoiceLabel = new wxStaticText(container, wxID_ANY, _("Select a game from the list: "));
            
            wxArrayString games;
            for (size_t i = 0; i < Model::Game::GameCount; ++i)
                games.Add(wxString(Model::Game::GameNames[i]));
            m_gameSelectionChoice = new wxChoice(container, wxID_ANY, wxDefaultPosition, wxDefaultSize, games);
            
            wxFlexGridSizer* innerSizer = new wxFlexGridSizer(2, LayoutConstants::ControlHorizontalMargin, LayoutConstants::ControlVerticalMargin);
            innerSizer->Add(gameSelectionChoiceLabel, 0, wxALIGN_CENTER_VERTICAL);
            innerSizer->Add(m_gameSelectionChoice, 0, wxALIGN_CENTER_VERTICAL);
            
            wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->Add(innerSizer, 0, wxALL, LayoutConstants::HighlightBoxMargin);
            
            container->SetSizerAndFit(outerSizer);
            return container;
        }
        
        wxWindow* GamesPreferencePane::createGamePreferences() {
            wxStaticBox* box = new wxStaticBox(this, wxID_ANY, _("Game Preferences"));
            
            wxStaticText* gamePathLabel = new wxStaticText(box, wxID_ANY, _("Game Path"));
            m_gamePathValueLabel = new wxStaticText(box, wxID_ANY, _("Not set"));
            m_chooseGamePathButton = new wxButton(box, wxID_ANY, _("Choose..."));
            
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
        }
        
        void GamesPreferencePane::updateControls() {
        }
        
        bool GamesPreferencePane::doValidate() {
            return true;
        }
    }
}
