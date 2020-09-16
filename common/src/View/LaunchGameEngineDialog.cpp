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

#include "LaunchGameEngineDialog.h"

#include "EL/EvaluationContext.h"
#include "EL/Interpolator.h"
#include "IO/PathQt.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GameFactory.h"
#include "Model/GameEngineProfile.h"
#include "View/BorderLine.h"
#include "View/CompilationVariables.h"
#include "View/CurrentGameIndicator.h"
#include "View/GameEngineDialog.h"
#include "View/GameEngineProfileListBox.h"
#include "View/MapDocument.h"
#include "View/MultiCompletionLineEdit.h"
#include "View/VariableStoreModel.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>
#include <kdl/string_utils.h>

#include <string>

#include <QCompleter>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>

namespace TrenchBroom {
    namespace View {
        LaunchGameEngineDialog::LaunchGameEngineDialog(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QDialog(parent),
        m_document(std::move(document)),
        m_gameEngineList(nullptr),
        m_parameterText(nullptr),
        m_launchButton(nullptr),
        m_lastProfile(nullptr) {
            createGui();
        }

        void LaunchGameEngineDialog::createGui() {
            setWindowIconTB(this);
            setWindowTitle("Launch Engine");

            auto document = kdl::mem_lock(m_document);
            const auto& gameName = document->game()->gameName();
            auto* gameIndicator = new CurrentGameIndicator(gameName);

            auto* midPanel = new QWidget(this);

            auto& gameFactory = Model::GameFactory::instance();
            const auto& gameConfig = gameFactory.gameConfig(gameName);
            m_config = gameConfig.gameEngineConfig();
            m_gameEngineList = new GameEngineProfileListBox(&m_config);
            m_gameEngineList->setEmptyText("Click the 'Configure engines...' button to create a game engine profile.");
            m_gameEngineList->setMinimumSize(250, 280);

            auto* header = new QLabel("Launch Engine");
            makeHeader(header);

            auto* message = new QLabel("Select a game engine from the list on the right and edit the commandline parameters in the text box below. You can use variables to refer to the map name and other values.");
            message->setWordWrap(true);

            auto* openPreferencesButton = new QPushButton("Configure engines...");

            auto* parameterLabel = new QLabel("Parameters");
            makeEmphasized(parameterLabel);

            m_parameterText = new MultiCompletionLineEdit();
            m_parameterText->setMultiCompleter(new QCompleter(new VariableStoreModel(variables())));
            m_parameterText->setWordDelimiters(QRegularExpression("\\$"), QRegularExpression("\\}"));

            auto* midLeftLayout = new QVBoxLayout();
            midLeftLayout->setContentsMargins(0, 0, 0, 0);
            midLeftLayout->setSpacing(0);
            midLeftLayout->addSpacing(20);
            midLeftLayout->addWidget(header);
            midLeftLayout->addSpacing(20);
            midLeftLayout->addWidget(message);
            midLeftLayout->addSpacing(10);
            midLeftLayout->addWidget(openPreferencesButton, 0, Qt::AlignHCenter);
            midLeftLayout->addStretch(1);
            midLeftLayout->addWidget(parameterLabel);
            midLeftLayout->addSpacing(LayoutConstants::NarrowVMargin);
            midLeftLayout->addWidget(m_parameterText);
            midLeftLayout->addSpacing(20);

            auto* midLayout = new QHBoxLayout();
            midLayout->setContentsMargins(0, 0, 0, 0);
            midLayout->setSpacing(0);
            midLayout->addSpacing(20);
            midLayout->addLayout(midLeftLayout, 1);
            midLayout->addSpacing(20);
            midLayout->addWidget(new BorderLine(BorderLine::Direction::Vertical));
            midLayout->addWidget(m_gameEngineList);
            midPanel->setLayout(midLayout);

            auto* buttonBox = new QDialogButtonBox();
            m_launchButton = buttonBox->addButton("Launch", QDialogButtonBox::AcceptRole);
            auto* closeButton = buttonBox->addButton("Close", QDialogButtonBox::RejectRole);

            auto* outerLayout = new QVBoxLayout();
            outerLayout->setContentsMargins(0, 0, 0, 0);
            outerLayout->setSpacing(0);
            outerLayout->addWidget(gameIndicator);
            outerLayout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
            outerLayout->addWidget(midPanel, 1);
            outerLayout->addLayout(wrapDialogButtonBox(buttonBox));

            setLayout(outerLayout);

            m_parameterText->setEnabled(false);
            m_launchButton->setEnabled(false);

            connect(openPreferencesButton, &QPushButton::clicked, this, &LaunchGameEngineDialog::editGameEngines);

            connect(m_parameterText, &QLineEdit::textChanged, this, &LaunchGameEngineDialog::parametersChanged);
            connect(m_parameterText, &QLineEdit::returnPressed, this, &LaunchGameEngineDialog::launchEngine);

            connect(m_launchButton, &QPushButton::clicked, this, &LaunchGameEngineDialog::launchEngine);
            connect(closeButton, &QPushButton::clicked, this, &LaunchGameEngineDialog::close);

            connect(m_gameEngineList, &GameEngineProfileListBox::currentProfileChanged, this,
                &LaunchGameEngineDialog::gameEngineProfileChanged);
            connect(m_gameEngineList, &GameEngineProfileListBox::profileSelected, this,
                &LaunchGameEngineDialog::launchEngine);

            if (m_gameEngineList->count() > 0) {
                m_gameEngineList->setCurrentRow(0);
            }
        }

        void LaunchGameEngineDialog::reloadConfig() {
            auto document = kdl::mem_lock(m_document);
            const auto& gameName = document->game()->gameName();

            auto& gameFactory = Model::GameFactory::instance();
            const auto& gameConfig = gameFactory.gameConfig(gameName);
            m_config = gameConfig.gameEngineConfig();

            m_gameEngineList->setConfig(&m_config);
        }

        LaunchGameEngineVariables LaunchGameEngineDialog::variables() const {
            return LaunchGameEngineVariables(kdl::mem_lock(m_document));
        }

        void LaunchGameEngineDialog::gameEngineProfileChanged() {
            m_lastProfile = m_gameEngineList->selectedProfile();
            if (m_lastProfile != nullptr) {
                m_parameterText->setText(QString::fromStdString(m_lastProfile->parameterSpec()));
                m_parameterText->setEnabled(true);
                m_launchButton->setEnabled(true);
            } else {
                m_parameterText->setText("");
                m_parameterText->setEnabled(false);
                m_launchButton->setEnabled(false);
            }
        }

        void LaunchGameEngineDialog::parametersChanged(const QString& text) {
            Model::GameEngineProfile* profile = m_gameEngineList->selectedProfile();
            if (profile != nullptr) {
                const auto parameterSpec = text.toStdString();
                if (profile->parameterSpec() != parameterSpec) {
                    profile->setParameterSpec(parameterSpec);
                }
            }
        }

        void LaunchGameEngineDialog::editGameEngines() {
            saveConfig();

            const bool wasEmpty = m_gameEngineList->count() == 0;

            GameEngineDialog dialog(kdl::mem_lock(m_document)->game()->gameName(), this);
            dialog.exec();

            // reload m_config as it may have been changed by the GameEngineDialog
            reloadConfig();

            if (wasEmpty && m_gameEngineList->count() > 0) {
                m_gameEngineList->setCurrentRow(0);
            }
        }

        void LaunchGameEngineDialog::launchEngine() {
            try {
                const auto* profile = m_gameEngineList->selectedProfile();
                ensure(profile != nullptr, "profile is null");

                const auto& executablePath = profile->path();

                const std::string& parameterSpec = profile->parameterSpec();
                const std::string parameters = EL::interpolate(parameterSpec, EL::EvaluationContext(variables()));

                const auto workDir = IO::pathAsQString(executablePath.deleteLastComponent());

#ifdef __APPLE__
                // We have to launch apps via the 'open' command so that we can properly pass parameters.
                QStringList arguments;
                arguments.append("-a");
                arguments.append(IO::pathAsQString(executablePath));
                arguments.append("--args");
                arguments.append(QString::fromStdString(parameters));

                if (!QProcess::startDetached("/usr/bin/open", arguments, workDir)) {
                    throw Exception("Unknown error");
                }
#else
                const QString commandAndArgs = QString::fromLatin1("\"%1\" %2")
                    .arg(IO::pathAsQString(executablePath))
                    .arg(QString::fromStdString(parameters));

                const QString oldWorkDir = QDir::currentPath();
                QDir::setCurrent(workDir);
                const bool success = QProcess::startDetached(commandAndArgs);
                QDir::setCurrent(oldWorkDir);

                if (!success) {
                    throw Exception("Unknown error");
                }
#endif

                accept();
            } catch (const Exception& e) {
                const auto message = kdl::str_to_string("Could not launch game engine: ", e.what());
                QMessageBox::critical(this, "TrenchBroom", QString::fromStdString(message), QMessageBox::Ok);
            }
        }

        void LaunchGameEngineDialog::done(int r) {
            saveConfig();

            QDialog::done(r);
        }

        void LaunchGameEngineDialog::saveConfig() {
            auto document = kdl::mem_lock(m_document);
            const auto& gameName = document->game()->gameName();
            auto& gameFactory = Model::GameFactory::instance();
            gameFactory.saveGameEngineConfig(gameName, m_config);
        }
    }
}
