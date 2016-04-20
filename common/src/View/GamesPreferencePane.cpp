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
#include <wx/choice.h>
#include <wx/dirdlg.h>
#include <wx/gbsizer.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        GamesPreferencePane::GamesPreferencePane(wxWindow* parent) :
        PreferencePane(parent),
        m_gameListBox(NULL),
        m_book(NULL),
        m_gamePathText(NULL),
        m_chooseGamePathButton(NULL),
        m_defaultEngineChoice(NULL) {
            createGui();
            bindEvents();
        }
        
        void GamesPreferencePane::OnGameSelectionChanged(GameSelectionCommand& event) {
            if (IsBeingDeleted()) return;

            updateControls();
        }

        void GamesPreferencePane::OnGamePathChanged(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;
            
            updateGamePath(m_gamePathText->GetValue());
        }

        void GamesPreferencePane::OnChooseGamePathClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            const wxString pathStr = ::wxDirSelector("Choose game directory", wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (!pathStr.empty())
                updateGamePath(pathStr);
        }

        void GamesPreferencePane::updateGamePath(const wxString& str) {
            if (isValidGamePath(str)) {
                const IO::Path gamePath(str.ToStdString());
                const String gameName = m_gameListBox->selectedGameName();
                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                gameFactory.setGamePath(gameName, gamePath);
                updateControls();
            }
        }

        void GamesPreferencePane::OnDefaultEngineChanged(wxCommandEvent& event) {
            const String gameName = m_gameListBox->selectedGameName();
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            gameFactory.setDefaultEngine(gameName, IO::Path(m_defaultEngineChoice->GetStringSelection().ToStdString()));
            
            updateControls();
        }

        void GamesPreferencePane::OnUpdateGamePathText(wxIdleEvent& event) {
            if (isValidGamePath(m_gamePathText->GetValue()))
                m_gamePathText->SetForegroundColour(GetForegroundColour());
            else
                m_gamePathText->SetForegroundColour(*wxRED);
        }

        bool GamesPreferencePane::isValidGamePath(const wxString& str) const {
            try {
                const IO::Path gamePath(str.ToStdString());
                return IO::Disk::directoryExists(gamePath);
            } catch (const PathException&) {
                return false;
            }
        }

        void GamesPreferencePane::createGui() {
            m_gameListBox = new GameListBox(this);
            m_gameListBox->selectGame(0);
            
            m_book = new wxSimplebook(this);
            m_book->AddPage(createDefaultPage(m_book), "Default");
            m_book->AddPage(createGamePreferencesPage(m_book), "Game");
            m_book->SetSelection(0);
            
            wxSizer* prefMarginSizer = new wxBoxSizer(wxVERTICAL);
            prefMarginSizer->AddSpacer(LayoutConstants::WideVMargin);
            prefMarginSizer->Add(m_book, wxSizerFlags().Expand());
            prefMarginSizer->AddSpacer(LayoutConstants::WideVMargin);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(m_gameListBox, wxSizerFlags().Expand());
            sizer->Add(new BorderLine(this, BorderLine::Direction_Vertical), wxSizerFlags().Expand());
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->Add(prefMarginSizer, wxSizerFlags().Expand().Proportion(1));
            sizer->AddSpacer(LayoutConstants::WideVMargin);
            sizer->SetItemMinSize(m_gameListBox, 200, 200);
            
            SetSizer(sizer);
            SetMinSize(wxSize(600, 300));
        }
        
        wxWindow* GamesPreferencePane::createDefaultPage(wxWindow* parent) {
            wxPanel* containerPanel = new wxPanel(parent);
            
            wxStaticText* emptyText = new wxStaticText(containerPanel, wxID_ANY, "Select a game.");
            emptyText->SetFont(emptyText->GetFont().Bold());
            emptyText->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
            
            wxSizer* justifySizer = new wxBoxSizer(wxHORIZONTAL);
            justifySizer->AddStretchSpacer();
            justifySizer->AddSpacer(LayoutConstants::WideHMargin);
            justifySizer->Add(emptyText, wxSizerFlags().Expand());
            justifySizer->AddSpacer(LayoutConstants::WideHMargin);
            justifySizer->AddStretchSpacer();
            
            wxSizer* containerSizer = new wxBoxSizer(wxVERTICAL);
            containerSizer->AddSpacer(LayoutConstants::WideVMargin);
            containerSizer->Add(justifySizer, wxSizerFlags().Expand());
            containerSizer->AddSpacer(LayoutConstants::WideVMargin);
            containerSizer->AddStretchSpacer();
            
            containerPanel->SetSizer(containerSizer);
            return containerPanel;
        }

        wxWindow* GamesPreferencePane::createGamePreferencesPage(wxWindow* parent) {
            wxPanel* box = new wxPanel(parent);

            wxStaticText* gamePathLabel = new wxStaticText(box, wxID_ANY, "Game Path");
            m_gamePathText = new wxTextCtrl(box, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            m_gamePathText->SetHint("Not set");
            m_chooseGamePathButton = new wxButton(box, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            
            wxStaticText* defaultEngineLabel = new wxStaticText(box, wxID_ANY, "Default Engine");
            m_defaultEngineChoice = new wxChoice(box, wxID_ANY);
            
            wxGridBagSizer* sizer = new wxGridBagSizer(LayoutConstants::WideVMargin, LayoutConstants::WideHMargin);
            sizer->Add(gamePathLabel,           wxGBPosition(0,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
            sizer->Add(m_gamePathText,          wxGBPosition(0,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxEXPAND);
            sizer->Add(m_chooseGamePathButton,  wxGBPosition(0,2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
            sizer->Add(defaultEngineLabel,      wxGBPosition(1,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
            sizer->Add(m_defaultEngineChoice,   wxGBPosition(1,1), wxGBSpan(1,2), wxALIGN_CENTER_VERTICAL);
            sizer->AddGrowableCol(1);
            
            box->SetSizer(sizer);
            return box;
        }

        void GamesPreferencePane::bindEvents() {
            m_gameListBox->Bind(GAME_SELECTION_CHANGE_EVENT, &GamesPreferencePane::OnGameSelectionChanged, this);
            m_gamePathText->Bind(wxEVT_TEXT_ENTER, &GamesPreferencePane::OnGamePathChanged, this);
            m_chooseGamePathButton->Bind(wxEVT_BUTTON, &GamesPreferencePane::OnChooseGamePathClicked, this);
            m_defaultEngineChoice->Bind(wxEVT_CHOICE, &GamesPreferencePane::OnDefaultEngineChanged, this);
            Bind(wxEVT_IDLE, &GamesPreferencePane::OnUpdateGamePathText, this);
        }
        
        bool GamesPreferencePane::doCanResetToDefaults() {
            return false;
        }

        void GamesPreferencePane::doResetToDefaults() {}

        void GamesPreferencePane::doUpdateControls() {
            if (m_gameListBox->GetSelection() == wxNOT_FOUND) {
                m_book->SetSelection(0);
            } else {
                m_book->SetSelection(1);
                const String gameName = m_gameListBox->selectedGameName();
                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                const IO::Path gamePath = gameFactory.gamePath(gameName);
                m_gamePathText->ChangeValue(gamePath.asString());
                m_gameListBox->reloadGameInfos();
                m_defaultEngineChoice->Set(engines());
                m_defaultEngineChoice->SetStringSelection(gameFactory.defaultEngine(gameName).asString());
            }
            Layout();
                
        }
        
        wxArrayString GamesPreferencePane::engines() const {
            const String gameName = m_gameListBox->selectedGameName();
            Model::GameFactory& gameFactory = Model::GameFactory::instance();
            const IO::Path::List paths = gameFactory.findEngines(gameName);
            
            wxArrayString result;
            IO::Path::List::const_iterator it, end;
            for (it = paths.begin(), end = paths.end(); it != end; ++it) {
                const IO::Path& path = *it;
                result.Add(path.lastComponent().asString());
            }
            
            return result;
        }

        bool GamesPreferencePane::doValidate() {
            return true;
        }
    }
}
