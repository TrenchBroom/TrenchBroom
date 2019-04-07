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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GamesPreferencePane.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "IO/Path.h"
#include "Model/Game.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/GameEngineDialog.h"
#include "View/GameListBox.h"
#include "View/ViewConstants.h"
#include "View/wxUtils.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

namespace TrenchBroom {
    namespace View {
        GamesPreferencePane::GamesPreferencePane(QWidget* parent) :
        PreferencePane(parent),
        m_gameListBox(nullptr),
        m_stackedWidget(nullptr),
        m_gamePathText(nullptr),
        m_chooseGamePathButton(nullptr) {
            createGui();
            updateControls();
        }

        void GamesPreferencePane::createGui() {
            m_gameListBox = new GameListBox();
            m_gameListBox->selectGame(0);
            m_gameListBox->setMinimumSize(200, 300);

            connect(m_gameListBox, &GameListBox::currentGameChanged, this, &GamesPreferencePane::currentGameChanged);

            m_stackedWidget = new QStackedWidget();
            m_stackedWidget->addWidget(createDefaultPage("Select a game."));
            m_stackedWidget->addWidget(createGamePreferencesPage());

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(QMargins());
            layout->setSpacing(0);
            setLayout(layout);

            layout->addWidget(m_gameListBox);
            layout->addWidget(new BorderLine(BorderLine::Direction_Vertical));
            layout->addSpacing(LayoutConstants::WideVMargin);
            layout->addWidget(m_stackedWidget, 1);

            setMinimumSize(600, 300);
        }

        QWidget* GamesPreferencePane::createGamePreferencesPage() {
            auto* container = new QWidget();

            m_gamePathText = new QLineEdit();
            m_gamePathText->setReadOnly(true);
            setHint(m_gamePathText, "Click on the button to change...");

            m_chooseGamePathButton = new QPushButton("...");
            connect(m_chooseGamePathButton, &QPushButton::clicked, this, &GamesPreferencePane::chooseGamePathClicked);

            auto* configureEnginesButton = new QPushButton("Configure engines...");
            connect(configureEnginesButton, &QPushButton::clicked, this, &GamesPreferencePane::configureEnginesClicked);

            auto* gamePathLayout = new QHBoxLayout();
            gamePathLayout->addWidget(m_gamePathText, 1);
            gamePathLayout->addWidget(m_chooseGamePathButton);

            auto* layout = new QFormLayout();
            container->setLayout(layout);

            layout->addRow("Game Path", gamePathLayout);
            layout->addRow("Engines", configureEnginesButton);

            return container;
        }

        void GamesPreferencePane::currentGameChanged(const QString& gameName) {
            updateControls();
        }

        void GamesPreferencePane::chooseGamePathClicked() {
            const QString pathStr = QFileDialog::getExistingDirectory(this);
            if (!pathStr.isEmpty()) {
                updateGamePath(pathStr);
            }
        }

        void GamesPreferencePane::updateGamePath(const QString& str) {
            const auto gamePath = IO::Path(str.toStdString());
            const auto gameName = m_gameListBox->selectedGameName();
            auto& gameFactory = Model::GameFactory::instance();
            if (gameFactory.setGamePath(gameName, gamePath)) {
                updateControls();
            }
        }

        void GamesPreferencePane::configureEnginesClicked() {
            const auto gameName = m_gameListBox->selectedGameName();
            GameEngineDialog dialog(gameName, this);
            dialog.exec();
        }

        bool GamesPreferencePane::doCanResetToDefaults() {
            return false;
        }

        void GamesPreferencePane::doResetToDefaults() {}

        void GamesPreferencePane::doUpdateControls() {
            if (m_gameListBox->currentRow() < 0) {
                m_stackedWidget->setCurrentIndex(0);
            } else {
                m_stackedWidget->setCurrentIndex(1);
                const auto gameName = m_gameListBox->selectedGameName();
                Model::GameFactory& gameFactory = Model::GameFactory::instance();
                const auto gamePath = gameFactory.gamePath(gameName);
                m_gamePathText->setText(QString::fromStdString(gamePath.asString()));
                m_gameListBox->reloadGameInfos();
            }
        }

        bool GamesPreferencePane::doValidate() {
            return true;
        }
    }
}
