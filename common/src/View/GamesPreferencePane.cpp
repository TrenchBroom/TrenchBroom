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
        m_stackedWidget(nullptr) {
            createGui();
            updateControls();
            m_gameListBox->setFocus();
        }

        void GamesPreferencePane::createGui() {
            m_gameListBox = new GameListBox();
            m_gameListBox->selectGame(0);
            m_gameListBox->setMaximumWidth(220);
            m_gameListBox->setMinimumHeight(300);

            connect(m_gameListBox, &GameListBox::currentGameChanged, this, &GamesPreferencePane::currentGameChanged);

            m_stackedWidget = new QStackedWidget();
            m_stackedWidget->addWidget(createDefaultPage("Select a game."));
            //m_stackedWidget->addWidget(createGamePreferencesPage());

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(QMargins());
            layout->setSpacing(0);
            setLayout(layout);

            layout->addWidget(m_gameListBox);
            layout->addWidget(new BorderLine(BorderLine::Direction::Vertical));
            layout->addSpacing(LayoutConstants::MediumVMargin);
            layout->addWidget(m_stackedWidget, 1, Qt::AlignTop);

            setMinimumWidth(600);
        }

        QWidget* GamesPreferencePane::createGamePreferencesPage(const std::string& gameName) {
            auto* container = new QWidget();

            auto* gamePathText = new QLineEdit();
            setHint(gamePathText, "Click on the button to change...");
            connect(gamePathText, &QLineEdit::editingFinished, this, [=]() {
                updateGamePath(gamePathText->text());
            });

            auto* validDirectoryIcon = new QAction(gamePathText);
            gamePathText->addAction(validDirectoryIcon, QLineEdit::TrailingPosition);
            connect(gamePathText, &QLineEdit::textChanged, this, [validDirectoryIcon](const QString& text) {
                if (text.isEmpty() || QDir(text).exists()) {
                    validDirectoryIcon->setToolTip("");
                    validDirectoryIcon->setIcon(QIcon());
                } else {
                    validDirectoryIcon->setToolTip(tr("Directory not found"));
                    validDirectoryIcon->setIcon(IO::loadSVGIcon(IO::Path("IssueBrowser.svg")));
                }
            });

            auto* chooseGamePathButton = new QPushButton("...");
            connect(chooseGamePathButton, &QPushButton::clicked, this, &GamesPreferencePane::chooseGamePathClicked);

            auto* configureEnginesButton = new QPushButton("Configure engines...");
            connect(configureEnginesButton, &QPushButton::clicked, this, &GamesPreferencePane::configureEnginesClicked);

            auto* gamePathLayout = new QHBoxLayout();
            gamePathLayout->setContentsMargins(QMargins());
            gamePathLayout->setSpacing(LayoutConstants::MediumHMargin);
            gamePathLayout->addWidget(gamePathText, 1);
            gamePathLayout->addWidget(chooseGamePathButton);

            //auto* compilationToolsLayout = new QFormLayout();
            //compilationToolsLayout->setContentsMargins(LayoutConstants::MediumHMargin, LayoutConstants::MediumVMargin, LayoutConstants::MediumHMargin, LayoutConstants::MediumVMargin);
            //compilationToolsLayout->setHorizontalSpacing(LayoutConstants::MediumHMargin);
            //compilationToolsLayout->setVerticalSpacing(0);
            //compilationToolsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

            // m_compilationToolsLayout populated in updateCompilationTools()

            auto* layout = new FormWithSectionsLayout();
            layout->setContentsMargins(0, LayoutConstants::MediumVMargin, 0, 0);
            layout->setVerticalSpacing(2);
            layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

            layout->addSection("Game Path");
            layout->addRow("Game Path", gamePathLayout);
            layout->addRow("", configureEnginesButton);

            layout->addSection("Compilation Tools");

            //const std::string gameName = m_gameListBox->selectedGameName();
            auto& gameFactory = Model::GameFactory::instance();
            const auto& gameConfig = gameFactory.gameConfig(gameName);

            for (auto& tool : gameConfig.compilationToolDescriptions()) {
                const std::string toolName = tool.name;
                auto* edit = new QLineEdit();
                edit->setText(IO::pathAsQString(gameFactory.compilationToolPath(gameName, toolName)));
                connect(edit, &QLineEdit::editingFinished, this, [=](){
                    Model::GameFactory::instance().setCompilationToolPath(gameName, toolName, IO::pathFromQString(edit->text()));
                });

                auto* browseButton = new QPushButton("...");
                connect(browseButton, &QPushButton::clicked, this, [=](){
                    const QString pathStr = QFileDialog::getExistingDirectory(this, tr("%1 Path").arg(QString::fromStdString(toolName)),
                                                                              fileDialogDefaultDirectory(FileDialogDir::CompileTool));
                    if (pathStr.isEmpty()) {
                        return;
                    }
                    edit->setText(pathStr);
                    Model::GameFactory::instance().setCompilationToolPath(gameName, toolName, IO::pathFromQString(pathStr));
                });

                auto *rowLayout = new QHBoxLayout();
                rowLayout->setContentsMargins(QMargins());
                rowLayout->setSpacing(LayoutConstants::MediumHMargin);
                rowLayout->addWidget(edit, 1);
                rowLayout->addWidget(browseButton);

                layout->addRow(QString::fromStdString(tool.name), rowLayout);
            }

            container->setLayout(layout);

            return container;
        }

        void GamesPreferencePane::currentGameChanged(const QString& gameName) {
            if (gameName == m_currentGame) {
                return;
            }
            m_currentGame = gameName;

            // Delete previous game preference page
            if (m_stackedWidget->count() == 2) {
                delete m_stackedWidget->widget(1);
            }

            m_stackedWidget->addWidget(createGamePreferencesPage(gameName.toStdString()));
            m_stackedWidget->setCurrentIndex(1);
        }

        void GamesPreferencePane::chooseGamePathClicked() {
            const QString pathStr = QFileDialog::getExistingDirectory(this, tr("Game Path"), fileDialogDefaultDirectory(FileDialogDir::GamePath));
            if (!pathStr.isEmpty()) {
                updateGamePath(pathStr);
            }
        }

        void GamesPreferencePane::updateGamePath(const QString& str) {
            updateFileDialogDefaultDirectoryWithDirectory(FileDialogDir::GamePath, str);

            const auto gamePath = IO::pathFromQString(str);
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
            //if (m_gameListBox->currentRow() < 0) {
            //    m_stackedWidget->setCurrentIndex(0);
            //} else {
            //    m_stackedWidget->setCurrentIndex(1);
            //    const auto gameName = m_gameListBox->selectedGameName();
            //    auto& gameFactory = Model::GameFactory::instance();
            //    const auto gamePath = gameFactory.gamePath(gameName);
            //    m_gamePathText->setText(IO::pathAsQString(gamePath));
            //    m_gameListBox->updateGameInfos();
            //}

            //updateCompilationTools();
        }

        bool GamesPreferencePane::doValidate() {
            return true;
        }
    }
}
