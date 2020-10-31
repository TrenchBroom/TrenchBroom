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
#include "IO/Path.h"
#include "IO/PathQt.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GameFactory.h"
#include "View/BorderLine.h"
#include "View/FormWithSectionsLayout.h"
#include "View/GameEngineDialog.h"
#include "View/GameListBox.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <QAction>
#include <QBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>

#include "IO/ResourceUtils.h"

namespace TrenchBroom {
    namespace View {
        GamesPreferencePane::GamesPreferencePane(QWidget* parent) :
        PreferencePane(parent),
        m_gameListBox(nullptr),
        m_stackedWidget(nullptr),
        m_defaultPage(nullptr),
        m_currentGamePage(nullptr) {
            createGui();
            updateControls();
            m_gameListBox->setFocus();
        }

        void GamesPreferencePane::createGui() {
            m_gameListBox = new GameListBox();
            m_gameListBox->selectGame(0);
            m_gameListBox->setMaximumWidth(220);
            m_gameListBox->setMinimumHeight(300);

            m_defaultPage = createDefaultPage("Select a game.");

            m_stackedWidget = new QStackedWidget();
            m_stackedWidget->addWidget(m_defaultPage);

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(QMargins());
            layout->setSpacing(0);
            setLayout(layout);

            layout->addWidget(m_gameListBox);
            layout->addWidget(new BorderLine(BorderLine::Direction::Vertical));
            layout->addSpacing(LayoutConstants::MediumVMargin);
            layout->addWidget(m_stackedWidget, 1, Qt::AlignTop);

            setMinimumWidth(600);

            connect(m_gameListBox, &GameListBox::currentGameChanged, this, [=](const QString& gameName) {
                if (gameName == m_currentGame) {
                    return;
                }
                m_currentGame = gameName;

                delete m_currentGamePage;
                m_currentGamePage = new GamePreferencePane(gameName.toStdString());

                m_stackedWidget->addWidget(m_currentGamePage);
                m_stackedWidget->setCurrentWidget(m_currentGamePage);
            });
        }

        bool GamesPreferencePane::doCanResetToDefaults() {
            return false;
        }

        void GamesPreferencePane::doResetToDefaults() {}

        void GamesPreferencePane::doUpdateControls() {
            m_gameListBox->updateGameInfos();

            if (m_currentGamePage != nullptr) {
                m_currentGamePage->updateControls();
            }
        }

        bool GamesPreferencePane::doValidate() {
            return true;
        }

        // GamePreferencePane

        GamePreferencePane::GamePreferencePane(const std::string& gameName, QWidget* parent) :
        QWidget(parent),
        m_gameName(gameName),
        m_gamePathText(nullptr),
        m_chooseGamePathButton(nullptr) {
            createGui();
        }

        void GamePreferencePane::createGui() {
            m_gamePathText = new QLineEdit();
            setHint(m_gamePathText, "Click on the button to change...");
            connect(m_gamePathText, &QLineEdit::editingFinished, this, [this]() {
                updateGamePath(this->m_gamePathText->text());
            });

            auto* validDirectoryIcon = new QAction(m_gamePathText);
            m_gamePathText->addAction(validDirectoryIcon, QLineEdit::TrailingPosition);
            connect(m_gamePathText, &QLineEdit::textChanged, this, [validDirectoryIcon](const QString& text) {
                if (text.isEmpty() || QDir(text).exists()) {
                    validDirectoryIcon->setToolTip("");
                    validDirectoryIcon->setIcon(QIcon());
                } else {
                    validDirectoryIcon->setToolTip(tr("Directory not found"));
                    validDirectoryIcon->setIcon(IO::loadSVGIcon(IO::Path("IssueBrowser.svg")));
                }
            });

            auto* chooseGamePathButton = new QPushButton("...");
            connect(chooseGamePathButton, &QPushButton::clicked, this, &GamePreferencePane::chooseGamePathClicked);

            auto* configureEnginesButton = new QPushButton("Configure engines...");
            connect(configureEnginesButton, &QPushButton::clicked, this, &GamePreferencePane::configureEnginesClicked);

            auto* gamePathLayout = new QHBoxLayout();
            gamePathLayout->setContentsMargins(QMargins());
            gamePathLayout->setSpacing(LayoutConstants::MediumHMargin);
            gamePathLayout->addWidget(m_gamePathText, 1);
            gamePathLayout->addWidget(chooseGamePathButton);

            auto* layout = new FormWithSectionsLayout();
            layout->setContentsMargins(0, LayoutConstants::MediumVMargin, 0, 0);
            layout->setVerticalSpacing(2);
            layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

            layout->addSection("Game");
            layout->addRow("Game Path", gamePathLayout);
            layout->addRow("", configureEnginesButton);

            layout->addSection("Compilation Tools");

            auto& gameFactory = Model::GameFactory::instance();
            const auto& gameConfig = gameFactory.gameConfig(m_gameName);

            for (auto& tool : gameConfig.compilationToolDescriptions()) {
                const std::string toolName = tool.name;
                auto* edit = new QLineEdit();
                edit->setText(IO::pathAsQString(gameFactory.compilationToolPath(m_gameName, toolName)));
                connect(edit, &QLineEdit::editingFinished, this, [=](){
                    Model::GameFactory::instance().setCompilationToolPath(m_gameName, toolName, IO::pathFromQString(edit->text()));
                });

                auto* browseButton = new QPushButton("...");
                connect(browseButton, &QPushButton::clicked, this, [=](){
                    const QString pathStr = QFileDialog::getOpenFileName(this, tr("%1 Path").arg(QString::fromStdString(toolName)),
                                                                         fileDialogDefaultDirectory(FileDialogDir::CompileTool));
                    if (pathStr.isEmpty()) {
                        return;
                    }
                    edit->setText(pathStr);
                    Model::GameFactory::instance().setCompilationToolPath(m_gameName, toolName, IO::pathFromQString(pathStr));
                });

                auto *rowLayout = new QHBoxLayout();
                rowLayout->setContentsMargins(QMargins());
                rowLayout->setSpacing(LayoutConstants::MediumHMargin);
                rowLayout->addWidget(edit, 1);
                rowLayout->addWidget(browseButton);

                layout->addRow(QString::fromStdString(tool.name), rowLayout);
            }

            setLayout(layout);

            updateControls();
        }

        void GamePreferencePane::chooseGamePathClicked() {
            const QString pathStr = QFileDialog::getExistingDirectory(this, tr("Game Path"), fileDialogDefaultDirectory(FileDialogDir::GamePath));
            if (!pathStr.isEmpty()) {
                updateGamePath(pathStr);
            }
        }

        void GamePreferencePane::updateGamePath(const QString& str) {
            updateFileDialogDefaultDirectoryWithDirectory(FileDialogDir::GamePath, str);

            const auto gamePath = IO::pathFromQString(str);
            auto& gameFactory = Model::GameFactory::instance();
            if (gameFactory.setGamePath(m_gameName, gamePath)) {
                updateControls();
            }
        }

        void GamePreferencePane::configureEnginesClicked() {
            GameEngineDialog dialog(m_gameName, this);
            dialog.exec();
        }

        void GamePreferencePane::updateControls() {
            auto& gameFactory = Model::GameFactory::instance();

            // Refresh tool paths from preferences
            for (auto& [toolName, toolPathEditor] : m_toolPathEditors) {
                toolPathEditor->setText(IO::pathAsQString(gameFactory.compilationToolPath(m_gameName, toolName)));
            }

            // Refresh game path
            const auto gamePath = gameFactory.gamePath(m_gameName);
            m_gamePathText->setText(IO::pathAsQString(gamePath));
        }
    }
}
