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
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace View {
        GameDialog::~GameDialog() {
            unbindObservers();
        }

        bool GameDialog::showNewDocumentDialog(QWidget* parent, std::string& gameName, Model::MapFormat& mapFormat) {
            GameDialog dialog("Select Game", "Select a game from the list on the right, then click OK. Once the new document is created, you can set up mod directories, entity definitions and textures by going to the map inspector, the entity inspector and the face inspector, respectively.", true, parent);
            if (dialog.exec() == QDialog::Rejected) {
                return false;
            } else {
                gameName = dialog.currentGameName();
                mapFormat = dialog.currentMapFormat();
                return true;
            }
        }

        bool GameDialog::showOpenDocumentDialog(QWidget* parent, std::string& gameName) {
            GameDialog dialog("Select Game",
                "TrenchBroom was unable to detect the game for the map document. Please choose a game in the game list and click OK.",
                false, parent);
            if (dialog.exec() == QDialog::Rejected) {
                return false;
            } else {
                gameName = dialog.currentGameName();
                return true;
            }
        }

        std::string GameDialog::currentGameName() const {
            return m_gameListBox->selectedGameName();
        }

        Model::MapFormat GameDialog::currentMapFormat() const {
            const auto formatName = m_mapFormatComboBox->currentText();
            assert(!formatName.isEmpty());
            return Model::formatFromName(formatName.toStdString());
        }

        void GameDialog::currentGameChanged(const QString& gameName) {
            updateMapFormats(gameName.toStdString());
            m_okButton->setEnabled(!gameName.isEmpty());
        }

        void GameDialog::gameSelected(const QString& /* gameName */) {
            accept();
        }

        void GameDialog::openPreferencesClicked() {
            auto& app = TrenchBroomApp::instance();
            app.openPreferences();
        }

        GameDialog::GameDialog(const QString& title, const QString& infoText, const bool formatPrompt, QWidget* parent) :
        QDialog(parent),
        m_gameListBox(nullptr),
        m_mapFormatComboBox(nullptr),
        m_openPreferencesButton(nullptr),
        m_okButton(nullptr) {
            createGui(title, infoText, formatPrompt);
            updateMapFormats("");
            bindObservers();
        }

        void GameDialog::createGui(const QString& title, const QString& infoText, const bool formatPrompt) {
            setWindowTitle(title);
            setWindowIconTB(this);

            auto* infoPanel = createInfoPanel(this, title, infoText);
            auto* selectionPanel = createSelectionPanel(this, formatPrompt);
            selectionPanel->setMinimumWidth(300);

            auto* innerLayout = new QHBoxLayout();
            innerLayout->setContentsMargins(QMargins());
            innerLayout->setSpacing(0);
            innerLayout->addWidget(infoPanel, 1);
            innerLayout->addWidget(new BorderLine(BorderLine::Direction::Vertical), 1);
            innerLayout->addWidget(selectionPanel, 1);

            auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
            connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

            m_okButton = buttonBox->button(QDialogButtonBox::Ok);
            m_okButton->setEnabled(false);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(QMargins());
            outerLayout->setSpacing(0);
            outerLayout->addLayout(innerLayout, 1);
            outerLayout->addLayout(wrapDialogButtonBox(buttonBox), 1);
            insertTitleBarSeparator(outerLayout);

            setLayout(outerLayout);
        }

        QWidget* GameDialog::createInfoPanel(QWidget* parent, const QString& title, const QString& infoText) {
            auto* infoPanel = new QWidget(parent);

            auto* header = new QLabel(title);
            makeHeader(header);

            auto* info = new QLabel(infoText);
            info->setWordWrap(true);

            auto* setupMsg = new QLabel("To set up the game paths, click on the button below to open the preferences dialog.");
            setupMsg->setWordWrap(true);

            m_openPreferencesButton = new QPushButton("Open preferences...");
            m_openPreferencesButton->setToolTip("Open the preferences dialog to manage game paths,");

            auto* layout = new QVBoxLayout();
            layout->setSpacing(0);
            layout->setContentsMargins(20, 20, 20, 20);

            layout->addWidget(header);
            layout->addSpacing(20);
            layout->addWidget(info);
            layout->addSpacing(10);
            layout->addWidget(setupMsg);
            layout->addSpacing(10);
            layout->addWidget(m_openPreferencesButton, 0, Qt::AlignHCenter);
            infoPanel->setLayout(layout);
            infoPanel->setMaximumWidth(350);

            connect(m_openPreferencesButton, &QPushButton::clicked, this, &GameDialog::openPreferencesClicked);

            return infoPanel;
        }

        QWidget* GameDialog::createSelectionPanel(QWidget* parent, const bool formatPrompt) {
            auto* panel = new QWidget(parent);

            m_gameListBox = new GameListBox();
            m_gameListBox->setToolTip("Double click on a game to select it");

            auto* label = new QLabel("Map Format");
            makeEmphasized(label);

            m_mapFormatComboBox = new QComboBox();
            m_mapFormatComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

            auto* mapFormatLayout = new QHBoxLayout();
            mapFormatLayout->setContentsMargins(LayoutConstants::WideHMargin, LayoutConstants::NarrowVMargin, LayoutConstants::WideHMargin, LayoutConstants::NarrowVMargin);
            mapFormatLayout->setSpacing(LayoutConstants::WideHMargin);
            mapFormatLayout->addWidget(label, 0, Qt::AlignRight | Qt::AlignVCenter);
            mapFormatLayout->addWidget(m_mapFormatComboBox, 1, Qt::AlignLeft | Qt::AlignVCenter);

            auto* mapFormatWidget = new QWidget();
            mapFormatWidget->setLayout(mapFormatLayout);

            auto* outerSizer = new QVBoxLayout();
            outerSizer->setContentsMargins(QMargins());
            outerSizer->setSpacing(0);
            outerSizer->addWidget(m_gameListBox, 1);
            outerSizer->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 1);
            outerSizer->addWidget(mapFormatWidget);
            panel->setLayout(outerSizer);

            if (!formatPrompt) {
                mapFormatWidget->hide();
            }

            connect(m_gameListBox, &GameListBox::currentGameChanged, this, &GameDialog::currentGameChanged);
            connect(m_gameListBox, &GameListBox::selectCurrentGame, this, &GameDialog::gameSelected);

            return panel;
        }

        void GameDialog::updateMapFormats(const std::string& gameName) {
            const auto& gameFactory = Model::GameFactory::instance();
            const auto fileFormats = gameName.empty() ? std::vector<std::string>({}) : gameFactory.fileFormats(gameName);

            m_mapFormatComboBox->clear();
            for (const auto& fileFormat : fileFormats) {
                m_mapFormatComboBox->addItem(QString::fromStdString(fileFormat));
            }

            m_mapFormatComboBox->setEnabled(m_mapFormatComboBox->count() > 1);
            if (m_mapFormatComboBox->count() > 0) {
                m_mapFormatComboBox->setCurrentIndex(0);
            }
        }

        void GameDialog::bindObservers() {
            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.addObserver(this, &GameDialog::preferenceDidChange);
        }

        void GameDialog::unbindObservers() {
            auto& prefs = PreferenceManager::instance();
            prefs.preferenceDidChangeNotifier.removeObserver(this, &GameDialog::preferenceDidChange);
        }

        void GameDialog::preferenceDidChange(const IO::Path& /* path */) {
            m_gameListBox->reloadGameInfos();
            m_okButton->setEnabled(!currentGameName().empty());
        }
    }
}
