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

#include "GameEngineProfileEditor.h"

#include "Model/GameEngineProfile.h"
#include "IO/DiskIO.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"
#include "TemporarilySetAny.h"

#include <wx/button.h>
#include <wx/filedlg.h>
#include <wx/gbsizer.h>
#include <wx/settings.h>
#include <wx/simplebook.h>
#include <QLabel>
#include <wx/textctrl.h>

namespace TrenchBroom {
    namespace View {
        GameEngineProfileEditor::GameEngineProfileEditor(QWidget* parent) :
        QWidget(parent),
        m_profile(nullptr),
        m_book(nullptr),
        m_nameText(nullptr),
        m_pathText(nullptr),
        m_ignoreNotifications(false) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            m_book = new wxSimplebook(this);
            m_book->AddPage(createDefaultPage(m_book, "Select a game engine profile."), "Default");
            m_book->AddPage(createEditorPage(m_book), "Editor");
            m_book->SetSelection(0);

            auto* bookSizer = new QVBoxLayout();
            bookSizer->addWidget(m_book, wxSizerFlags().Expand().Proportion(1));
            setLayout(bookSizer);
        }

        GameEngineProfileEditor::~GameEngineProfileEditor() {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &GameEngineProfileEditor::profileDidChange);
            }
        }

        QWidget* GameEngineProfileEditor::createEditorPage(QWidget* parent) {
            auto* containerPanel = new QWidget(parent);
            containerPanel->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));

            auto* nameLabel = new QLabel(containerPanel, wxID_ANY, "Name");
            auto* pathLabel = new QLabel(containerPanel, wxID_ANY, "Path");

            m_nameText = new wxTextCtrl(containerPanel, wxID_ANY);
            setHint(m_nameText, "Choose a name");
            m_pathText = new wxTextCtrl(containerPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
            setHint(m_pathText, "Click on the button to choose...");
            auto* choosePathButton = new wxButton(containerPanel, wxID_ANY, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

            m_nameText->Bind(wxEVT_TEXT, &GameEngineProfileEditor::OnNameChanged, this);
            m_pathText->Bind(wxEVT_TEXT_ENTER, &GameEngineProfileEditor::OnPathChanged, this);
            choosePathButton->Bind(&QAbstractButton::clicked, &GameEngineProfileEditor::OnChangePathClicked, this);
            Bind(wxEVT_IDLE, &GameEngineProfileEditor::OnUpdatePathTextUI, this);

            const auto LabelFlags   = Qt::AlignVCenter | wxALIGN_RIGHT;
            const auto EditorFlags  = Qt::AlignVCenter | wxEXPAND;

            auto* containerInnerSizer = new wxGridBagSizer(LayoutConstants::NarrowVMargin, LayoutConstants::NarrowHMargin);
            containerInnerSizer->addWidget(nameLabel,         wxGBPosition(0,0), wxDefaultSpan, LabelFlags);
            containerInnerSizer->addWidget(m_nameText,        wxGBPosition(0,1), wxGBSpan(1,2), EditorFlags);
            containerInnerSizer->addWidget(pathLabel,         wxGBPosition(1,0), wxDefaultSpan, LabelFlags);
            containerInnerSizer->addWidget(m_pathText,        wxGBPosition(1,1), wxDefaultSpan, EditorFlags);
            containerInnerSizer->addWidget(choosePathButton,  wxGBPosition(1,2), wxDefaultSpan, Qt::AlignVCenter);
            containerInnerSizer->AddGrowableCol(1);

            auto* containerOuterSizer = new QVBoxLayout();
            containerOuterSizer->addSpacing(LayoutConstants::WideVMargin);
            containerOuterSizer->addWidget(containerInnerSizer, 0, wxEXPAND | wxLEFT | wxRIGHT, LayoutConstants::MediumHMargin);
            containerOuterSizer->addSpacing(LayoutConstants::WideVMargin);

            containerPanel->setLayout(containerOuterSizer);
            return containerPanel;
        }

        void GameEngineProfileEditor::OnNameChanged() {
            ensure(m_profile != nullptr, "profile is null");

            const TemporarilySetBool ignore(m_ignoreNotifications);
            m_profile->setName(m_nameText->GetValue().ToStdString());
        }

        void GameEngineProfileEditor::OnPathChanged() {
            ensure(m_profile != nullptr, "profile is null");

            const TemporarilySetBool ignore(m_ignoreNotifications);
            updatePath(m_pathText->GetValue());
        }

        void GameEngineProfileEditor::OnChangePathClicked() {
            const auto pathStr = ::wxFileSelector("Choose engine", wxEmptyString, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (!pathStr.empty()) {
                updatePath(pathStr);
            }
        }

        void GameEngineProfileEditor::OnUpdatePathTextUI(wxIdleEvent &event) {
            if (m_pathText->GetValue().IsEmpty() || isValidEnginePath(m_pathText->GetValue())) {
                m_pathText->SetForegroundColour(m_nameText->GetForegroundColour());
            } else {
                m_pathText->SetForegroundColour(*wxRED);
            }
        }

        void GameEngineProfileEditor::updatePath(const QString& str) {
            if (isValidEnginePath(str)) {
                const auto path = IO::Path(str.ToStdString());
                m_profile->setPath(path);
                if (m_profile->name().empty()) {
                    m_profile->setName(path.lastComponent().deleteExtension().asString());
                }
                refresh();
            }
        }

        void GameEngineProfileEditor::setProfile(Model::GameEngineProfile* profile) {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &GameEngineProfileEditor::profileDidChange);
            }
            m_profile = profile;
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.addObserver(this, &GameEngineProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.addObserver(this, &GameEngineProfileEditor::profileDidChange);
                m_book->SetSelection(1);
            } else {
                m_book->SetSelection(0);
            }
            refresh();
        }

        void GameEngineProfileEditor::profileWillBeRemoved() {
            setProfile(nullptr);
        }

        void GameEngineProfileEditor::profileDidChange() {
            refresh();
        }

        void GameEngineProfileEditor::refresh() {
            if (m_profile != nullptr && !m_ignoreNotifications) {
                m_nameText->ChangeValue(m_profile->name());
                m_pathText->ChangeValue(m_profile->path().asString());
            }
        }

        bool GameEngineProfileEditor::isValidEnginePath(const QString& str) const {
            try {
                const auto path = IO::Path(str.ToStdString());
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
