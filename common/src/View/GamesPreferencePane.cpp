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
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/dirdlg.h>
#include <wx/gbsizer.h>
#include <wx/listbox.h>
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
        m_enginesListBox(NULL) {
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

        void GamesPreferencePane::OnUpdateGamePathText(wxIdleEvent& event) {
            if (isValidGamePath(m_gamePathText->GetValue()))
                m_gamePathText->SetForegroundColour(GetForegroundColour());
            else
                m_gamePathText->SetForegroundColour(*wxRED);
        }

        void GamesPreferencePane::OnAddEngines(wxCommandEvent& event) {
        }
        
        void GamesPreferencePane::OnRemoveEngines(wxCommandEvent& event) {
        }
        
        void GamesPreferencePane::OnUpdateRemoveEngines(wxUpdateUIEvent& event) {
            wxArrayInt selections;
            m_enginesListBox->GetSelections(selections);
            event.Enable(!selections.IsEmpty());
        }

        void GamesPreferencePane::OnSetDefaultEngine(wxCommandEvent& event) {
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
            prefMarginSizer->Add(m_book, wxSizerFlags().Expand().Proportion(1));
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
            wxPanel* containerPanel = new wxPanel(parent);

            wxStaticText* gamePathLabel = new wxStaticText(containerPanel, wxID_ANY, "Game Path");
            m_gamePathText = new wxTextCtrl(containerPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            m_gamePathText->SetHint("Not set");
            m_chooseGamePathButton = new wxButton(containerPanel, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            
            wxStaticText* engineLabel = new wxStaticText(containerPanel, wxID_ANY, "Engines");
            m_enginesListBox = new wxListBox(containerPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED | wxLB_NEEDED_SB);
            
            m_addEnginesButton = createBitmapButton(containerPanel, "Add.png", "Add engine(s)");
            m_removeEnginesButton = createBitmapButton(containerPanel, "Remove.png", "Remove the selected engines");
            
            wxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
            buttonSizer->Add(m_addEnginesButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->Add(m_removeEnginesButton, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM, LayoutConstants::NarrowVMargin);
            buttonSizer->AddStretchSpacer();

            wxGridBagSizer* containerSizer = new wxGridBagSizer(LayoutConstants::WideVMargin, LayoutConstants::WideHMargin);
            containerSizer->Add(gamePathLabel,           wxGBPosition(0,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
            containerSizer->Add(m_gamePathText,          wxGBPosition(0,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxEXPAND);
            containerSizer->Add(m_chooseGamePathButton,  wxGBPosition(0,2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
            containerSizer->Add(engineLabel,             wxGBPosition(1,0), wxGBSpan(2,1), wxALIGN_TOP | wxALIGN_RIGHT);
            containerSizer->Add(m_enginesListBox,        wxGBPosition(1,1), wxGBSpan(1,2), wxALIGN_TOP | wxEXPAND);
            containerSizer->Add(buttonSizer,             wxGBPosition(2,1), wxGBSpan(1,2), wxEXPAND);
            containerSizer->AddGrowableCol(1);
            containerSizer->AddGrowableRow(1);
            
            containerPanel->SetSizer(containerSizer);
            return containerPanel;
        }

        void GamesPreferencePane::bindEvents() {
            m_gameListBox->Bind(GAME_SELECTION_CHANGE_EVENT, &GamesPreferencePane::OnGameSelectionChanged, this);
            m_gamePathText->Bind(wxEVT_TEXT_ENTER, &GamesPreferencePane::OnGamePathChanged, this);
            m_chooseGamePathButton->Bind(wxEVT_BUTTON, &GamesPreferencePane::OnChooseGamePathClicked, this);
            m_enginesListBox->Bind(wxEVT_LISTBOX_DCLICK, &GamesPreferencePane::OnSetDefaultEngine, this);
            
            m_addEnginesButton->Bind(wxEVT_BUTTON, &GamesPreferencePane::OnAddEngines, this);
            m_removeEnginesButton->Bind(wxEVT_BUTTON, &GamesPreferencePane::OnRemoveEngines, this);
            m_removeEnginesButton->Bind(wxEVT_UPDATE_UI, &GamesPreferencePane::OnUpdateRemoveEngines, this);
            
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
                m_enginesListBox->Set(engines());
                // m_defaultEngineChoice->Set(engines());
                // m_defaultEngineChoice->SetStringSelection(gameFactory.defaultEngine(gameName).asString());
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
