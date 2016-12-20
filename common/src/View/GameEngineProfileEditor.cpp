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

#include "GameEngineProfileEditor.h"

#include "Model/GameEngineProfile.h"
#include "IO/DiskIO.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/settings.h>
#include <wx/simplebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        GameEngineProfileEditor::GameEngineProfileEditor(wxWindow* parent) :
        wxPanel(parent),
        m_profile(NULL),
        m_book(NULL),
        m_nameText(NULL),
        m_pathText(NULL) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            m_book = new wxSimplebook(this);
            m_book->AddPage(createDefaultPage(m_book, "Select a game engine profile."), "Default");
            m_book->AddPage(createEditorPage(m_book), "Editor");
            m_book->SetSelection(0);

            wxSizer* bookSizer = new wxBoxSizer(wxVERTICAL);
            bookSizer->Add(m_book, wxSizerFlags().Expand().Proportion(1));
            SetSizer(bookSizer);
        }

        GameEngineProfileEditor::~GameEngineProfileEditor() {
            if (m_profile != NULL) {
                m_profile->profileWillBeRemoved.removeObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &GameEngineProfileEditor::profileDidChange);
            }
        }

        wxWindow* GameEngineProfileEditor::createEditorPage(wxWindow* parent) {
            wxPanel* containerPanel = new wxPanel(parent);
            containerPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));

            wxStaticText* nameLabel = new wxStaticText(containerPanel, wxID_ANY, "Name");
            wxStaticText* pathLabel = new wxStaticText(containerPanel, wxID_ANY, "Path");

            m_nameText = new wxTextCtrl(containerPanel, wxID_ANY);
            m_nameText->SetHint("not set");
            m_pathText = new wxTextCtrl(containerPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            wxButton* choosePathButton = new wxButton(containerPanel, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
            m_pathText->SetHint("not set");

            m_nameText->Bind(wxEVT_TEXT, &GameEngineProfileEditor::OnNameChanged, this);
            m_pathText->Bind(wxEVT_TEXT_ENTER, &GameEngineProfileEditor::OnPathChanged, this);
            choosePathButton->Bind(wxEVT_BUTTON, &GameEngineProfileEditor::OnChangePathClicked, this);
            Bind(wxEVT_IDLE, &GameEngineProfileEditor::OnUpdatePathTextUI, this);

            const int LabelFlags   = wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT;
            const int EditorFlags  = wxALIGN_CENTER_VERTICAL | wxEXPAND;

            wxGridBagSizer* containerInnerSizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::NarrowHMargin);
            containerInnerSizer->Add(nameLabel,         wxGBPosition(0,0), wxDefaultSpan, LabelFlags);
            containerInnerSizer->Add(m_nameText,        wxGBPosition(0,1), wxGBSpan(1,2), EditorFlags);
            containerInnerSizer->Add(pathLabel,         wxGBPosition(1,0), wxDefaultSpan, LabelFlags);
            containerInnerSizer->Add(m_pathText,        wxGBPosition(1,1), wxDefaultSpan, EditorFlags);
            containerInnerSizer->Add(choosePathButton,  wxGBPosition(1,2), wxDefaultSpan, wxALIGN_CENTER_VERTICAL);
            containerInnerSizer->AddGrowableCol(1);

            wxSizer* containerOuterSizer = new wxBoxSizer(wxVERTICAL);
            containerOuterSizer->AddSpacer(LayoutConstants::WideVMargin);
            containerOuterSizer->Add(containerInnerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::MediumHMargin);
            containerOuterSizer->AddSpacer(LayoutConstants::WideVMargin);

            containerPanel->SetSizer(containerOuterSizer);
            return containerPanel;
        }

        void GameEngineProfileEditor::OnNameChanged(wxCommandEvent& event) {
            ensure(m_profile != NULL, "profile is null");
            m_profile->setName(m_nameText->GetValue().ToStdString());
        }

        void GameEngineProfileEditor::OnPathChanged(wxCommandEvent& event) {
            ensure(m_profile != NULL, "profile is null");
            updatePath(m_pathText->GetValue());
        }

        void GameEngineProfileEditor::OnChangePathClicked(wxCommandEvent& event) {
            const wxString pathStr = ::wxFileSelector("Choose engine", wxEmptyString, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (!pathStr.empty())
                updatePath(pathStr);
        }

        void GameEngineProfileEditor::OnUpdatePathTextUI(wxIdleEvent &event) {
            if (isValidEnginePath(m_pathText->GetValue()))
                m_pathText->SetForegroundColour(GetForegroundColour());
            else
                m_pathText->SetForegroundColour(*wxRED);
        }

        void GameEngineProfileEditor::updatePath(const wxString& str) {
            if (isValidEnginePath(str)) {
                const IO::Path path(str.ToStdString());
                m_profile->setPath(path);
                if (m_profile->name().empty())
                    m_profile->setName(path.lastComponent().deleteExtension().asString());
                refresh();
            }
        }

        void GameEngineProfileEditor::setProfile(Model::GameEngineProfile* profile) {
            if (m_profile != NULL) {
                m_profile->profileWillBeRemoved.removeObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &GameEngineProfileEditor::profileDidChange);
            }
            m_profile = profile;
            if (m_profile != NULL) {
                m_profile->profileWillBeRemoved.addObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.addObserver(this, &GameEngineProfileEditor::profileDidChange);
                m_book->SetSelection(1);
            } else {
                m_book->SetSelection(0);
            }
            refresh();
        }

        void GameEngineProfileEditor::profileWillBeRemoved() {
            setProfile(NULL);
        }

        void GameEngineProfileEditor::profileDidChange() {
            refresh();
        }

        void GameEngineProfileEditor::refresh() {
            if (m_profile != NULL) {
                m_nameText->ChangeValue(m_profile->name());
                m_pathText->ChangeValue(m_profile->path().asString());
            }
        }

        bool GameEngineProfileEditor::isValidEnginePath(const wxString& str) const {
            try {
                const IO::Path path(str.ToStdString());
                return IO::Disk::fileExists(path)
#ifdef __APPLE__
                || (IO::Disk::directoryExists(path) && StringUtils::caseInsensitiveEqual(path.extension(), "app"))
#endif
                ;
            } catch (...) {
                return false;
            }
        }
    }
}
