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

#include "GameDialog.h"

#include "TrenchBroomApp.h"
#include "PreferenceManager.h"
#include "Model/GameConfig.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/GameListBox.h"
#include "View/GameSelectionCommand.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <QLabel>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        GameDialog::~GameDialog() {
            unbindObservers();
        }

        bool GameDialog::showNewDocumentDialog(QWidget* parent, String& gameName, Model::MapFormat& mapFormat) {
            GameDialog dialog;
            dialog.createDialog(parent, "Select Game", "Select a game from the list on the right, then click OK. Once the new document is created, you can set up mod directories, entity definitions and textures by going to the map inspector, the entity inspector and the face inspector, respectively.");
            if (dialog.ShowModal() != wxID_OK)
                return false;
            gameName = dialog.selectedGameName();
            mapFormat = dialog.selectedMapFormat();
            return true;
        }

        bool GameDialog::showOpenDocumentDialog(QWidget* parent, String& gameName, Model::MapFormat& mapFormat) {
            GameDialog dialog;
            dialog.createDialog(parent, "Select Game", "TrenchBroom was unable to detect the game for the map document. Please choose a game in the game list and click OK.");
            if (dialog.ShowModal() != wxID_OK)
                return false;
            gameName = dialog.selectedGameName();
            mapFormat = dialog.selectedMapFormat();
            return true;        }

        String GameDialog::selectedGameName() const {
            return m_gameListBox->selectedGameName();
        }

        Model::MapFormat GameDialog::selectedMapFormat() const {
            assert(!m_mapFormatChoice->IsEmpty());

            const auto index = m_mapFormatChoice->GetSelection();
            assert(index >= 0);

            const auto formatName = m_mapFormatChoice->GetString(static_cast<unsigned int>(index)).ToStdString();
            return Model::mapFormat(formatName);
        }

        void GameDialog::OnGameSelectionChanged(GameSelectionCommand& command) {
            if (IsBeingDeleted()) return;

            gameSelectionChanged(command.gameName());
        }

        void GameDialog::OnGameSelected(GameSelectionCommand& command) {
            if (IsBeingDeleted()) return;

            gameSelected(command.gameName());
        }

        void GameDialog::OnUpdateMapFormatChoice(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(m_mapFormatChoice->GetCount() > 1);
        }

        void GameDialog::OnOpenPreferencesClicked(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            auto& app = TrenchBroomApp::instance();
            app.openPreferences();
        }

        void GameDialog::OnUpdateOkButton(wxUpdateUIEvent& event) {
            if (IsBeingDeleted()) return;

            event.Enable(isOkEnabled());
        }

        void GameDialog::OnClose(wxCloseEvent& event) {
            if (GetParent() != nullptr)
                GetParent()->Raise();
            event.Skip();
        }

        GameDialog::GameDialog() :
        wxDialog(),
        m_gameListBox(nullptr),
        m_mapFormatChoice(nullptr),
        m_openPreferencesButton(nullptr) {}

        void GameDialog::createDialog(QWidget* parent, const QString& title, const QString& infoText) {
            Create(wxGetTopLevelParent(parent), wxID_ANY, title);
            createGui(title, infoText);
            bindObservers();
            CentreOnParent();
        }

        void GameDialog::createGui(const QString& title, const QString& infoText) {
            setWindowIconTB(this);

            auto* infoPanel = createInfoPanel(this, title, infoText);
            auto* selectionPanel = createSelectionPanel(this);

            auto* innerSizer = new QHBoxLayout();
            innerSizer->Add(infoPanel, wxSizerFlags().Expand());
            innerSizer->Add(new BorderLine(this, BorderLine::Direction_Vertical), wxSizerFlags().Expand());
            innerSizer->Add(selectionPanel, wxSizerFlags().Expand().Proportion(1));
            innerSizer->SetItemMinSize(selectionPanel, 300, wxDefaultSize.y);

            auto* buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);

            auto* outerSizer = new QVBoxLayout();
#if !defined __APPLE__
			outerSizer->Add(new BorderLine(this), wxSizerFlags().Expand());
#endif
			outerSizer->Add(innerSizer, wxSizerFlags().Expand().Proportion(1));
            outerSizer->Add(wrapDialogButtonSizer(buttonSizer, this), wxSizerFlags().Expand());
            setLayout(outerSizer);

            FindWindow(wxID_OK)->Bind(wxEVT_UPDATE_UI, &GameDialog::OnUpdateOkButton, this);

            Bind(wxEVT_CLOSE_WINDOW, &GameDialog::OnClose, this);
        }

        QWidget* GameDialog::createInfoPanel(QWidget* parent, const QString& title, const QString& infoText) {
            auto* infoPanel = new QWidget(parent);

            auto* header = new QLabel(infoPanel, wxID_ANY, title);
            header->SetFont(header->GetFont().Larger().Larger().Bold());

            auto* info = new QLabel(infoPanel, wxID_ANY, infoText);
            info->Wrap(250);

            auto* setupMsg = new QLabel(infoPanel, wxID_ANY, "To set up the game paths, click on the button below to open the preferences dialog.");
            setupMsg->Wrap(250);

            m_openPreferencesButton = new wxButton(infoPanel, wxID_ANY, "Open preferences...");
            m_openPreferencesButton->SetToolTip("Open the preferences dialog to manage game paths,");

            auto* sizer = new QVBoxLayout();
            sizer->addSpacing(20);
            sizer->addWidget(header, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20));
            sizer->addSpacing(20);
            sizer->addWidget(info, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20));
            sizer->addSpacing(10);
            sizer->addWidget(setupMsg, wxSizerFlags().Border(wxLEFT | wxRIGHT, 20));
            sizer->addSpacing(10);
            sizer->addWidget(m_openPreferencesButton, wxSizerFlags().CenterHorizontal().Border(wxLEFT | wxRIGHT, 20));
            sizer->addSpacing(20);
            infoPanel->setLayout(sizer);

            m_openPreferencesButton->Bind(wxEVT_BUTTON, &GameDialog::OnOpenPreferencesClicked, this);

            return infoPanel;
        }

        QWidget* GameDialog::createSelectionPanel(QWidget* parent) {
            auto* panel = new QWidget(parent);

            m_gameListBox = new GameListBox(panel);
            m_gameListBox->SetToolTip("Double click on a game to select it");
            m_gameListBox->Bind(GAME_SELECTION_CHANGE_EVENT, &GameDialog::OnGameSelectionChanged, this);
            m_gameListBox->Bind(GAME_SELECTION_DBLCLICK_EVENT, &GameDialog::OnGameSelected, this);

            auto* header = new QLabel(panel, wxID_ANY, "Map Format");
            header->SetFont(header->GetFont().Bold());

            m_mapFormatChoice = new wxChoice(panel, wxID_ANY);
            m_mapFormatChoice->Bind(wxEVT_UPDATE_UI, &GameDialog::OnUpdateMapFormatChoice, this);

            auto* mapFormatSizer = new QHBoxLayout();
            mapFormatSizer->addSpacing(LayoutConstants::WideHMargin);
            mapFormatSizer->Add(header, wxSizerFlags().CenterVertical());
            mapFormatSizer->addSpacing(LayoutConstants::WideHMargin);
            mapFormatSizer->addSpacing(LayoutConstants::ChoiceLeftMargin);
            mapFormatSizer->Add(m_mapFormatChoice, wxSizerFlags().Border(wxTOP, LayoutConstants::ChoiceTopMargin));
            mapFormatSizer->addSpacing(LayoutConstants::WideHMargin);

            auto* outerSizer = new QVBoxLayout();
            outerSizer->Add(m_gameListBox, wxSizerFlags().Expand().Proportion(1));
            outerSizer->Add(new BorderLine(panel, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());
            outerSizer->addSpacing(LayoutConstants::WideVMargin);
            outerSizer->Add(mapFormatSizer);
            outerSizer->addSpacing(LayoutConstants::WideVMargin);
            panel->SetSizer(outerSizer);

            return panel;
        }

        bool GameDialog::isOkEnabled() const {
            return m_gameListBox->GetSelection() != wxNOT_FOUND;
        }

        void GameDialog::gameSelectionChanged(const String& gameName) {
            updateMapFormats(gameName);
        }

        void GameDialog::updateMapFormats(const String& gameName) {
            const auto& gameFactory = Model::GameFactory::instance();
            const auto& fileFormats = gameName.empty() ? EmptyStringList : gameFactory.fileFormats(gameName);

            m_mapFormatChoice->Clear();
            for (size_t i = 0; i < fileFormats.size(); ++i)
                m_mapFormatChoice->Append(fileFormats[i]);

            if (!m_mapFormatChoice->IsEmpty())
                m_mapFormatChoice->SetSelection(0);
        }

        void GameDialog::gameSelected(const String& gameName) {
            EndModal(wxID_OK);
        }

        void GameDialog::bindObservers() {
            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &GameDialog::preferenceDidChange);
        }

        void GameDialog::unbindObservers() {
            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &GameDialog::preferenceDidChange);
        }

        void GameDialog::preferenceDidChange(const IO::Path& path) {
            m_gameListBox->reloadGameInfos();
        }
    }
}
