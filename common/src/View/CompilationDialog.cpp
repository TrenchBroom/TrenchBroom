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

#include "CompilationDialog.h"

#include "Model/Game.h"
#include "Model/CompilationProfile.h"
#include "View/CompilationContext.h"
#include "View/CompilationProfileManager.h"
#include "View/CompilationRunner.h"
#include "View/LaunchGameEngineDialog.h"
#include "View/MapDocument.h"
#include "View/MapFrame.h"
#include "View/Splitter.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>

namespace TrenchBroom {
    namespace View {
        CompilationDialog::CompilationDialog(MapFrame* mapFrame) :
        QDialog(mapFrame),
        m_mapFrame(mapFrame),
        m_launchButton(nullptr),
        m_compileButton(nullptr),
        m_closeButton(nullptr),
        m_currentRunLabel(nullptr),
        m_output(nullptr) {
            setAttribute(Qt::WA_DeleteOnClose);
            createGui();
            setMinimumSize(600, 300);
            resize(800, 600);
            updateCompileButton(false);
        }

        void CompilationDialog::createGui() {
            setWindowIconTB(this);
            setWindowTitle("Compile");

            auto document = m_mapFrame->document();
            auto game = document->game();
            auto& compilationConfig = game->compilationConfig();

            m_profileManager = new CompilationProfileManager(document , compilationConfig);

            auto* outputPanel = new TitledPanel("Output");
            m_output = new QTextEdit();
            m_output->setReadOnly(true);
            m_output->setFont(Fonts::fixedWidthFont());

            auto* outputLayout = new QVBoxLayout();
            outputLayout->setContentsMargins(0, 0, 0, 0);
            outputLayout->setSpacing(0);
            outputLayout->addWidget(m_output);
            outputPanel->getPanel()->setLayout(outputLayout);

            auto* splitter = new Splitter(Qt::Vertical);
            splitter->addWidget(m_profileManager);
            splitter->addWidget(m_output);
            splitter->setSizes({2, 1});

            auto* buttonBox = new QDialogButtonBox();
            m_compileButton = buttonBox->addButton("Compile", QDialogButtonBox::AcceptRole);
            m_closeButton = buttonBox->addButton("Close", QDialogButtonBox::RejectRole);
            m_launchButton = new QPushButton("Launch...");

            m_currentRunLabel = new QLabel("");
            m_currentRunLabel->setAlignment(Qt::AlignRight);

            auto* buttonLayout = new QHBoxLayout();
            buttonLayout->setContentsMargins(0, 0, 0, 0);
            buttonLayout->setSpacing(LayoutConstants::WideHMargin);
            buttonLayout->addWidget(m_launchButton, 0, Qt::AlignVCenter);
            buttonLayout->addWidget(m_currentRunLabel, 1, Qt::AlignVCenter);
            buttonLayout->addWidget(buttonBox);

            auto* dialogLayout = new QVBoxLayout();
            dialogLayout->setContentsMargins(0, 0, 0, 0);
            dialogLayout->setSpacing(0);
            dialogLayout->addWidget(splitter, 1);
            dialogLayout->addLayout(wrapDialogButtonBox(buttonLayout));
            setLayout(dialogLayout);

            connect(&m_run, &CompilationRun::compilationStarted, this, &CompilationDialog::compilationStarted);
            connect(&m_run, &CompilationRun::compilationEnded, this, &CompilationDialog::compilationEnded);
            connect(m_profileManager, &CompilationProfileManager::selectedProfileChanged, this, &CompilationDialog::selectedProfileChanged);

            connect(m_compileButton, &QPushButton::clicked, this, &CompilationDialog::toggleCompile);
            connect(m_launchButton, &QPushButton::clicked, this, &CompilationDialog::launchEngine);
            connect(m_closeButton, &QPushButton::clicked, this, &CompilationDialog::close);
        }

        void CompilationDialog::keyPressEvent(QKeyEvent* event) {
            QDialog::keyPressEvent(event);
            const auto test = (event->modifiers() &Qt::AltModifier);
            updateCompileButton(test);
        }

        void CompilationDialog::keyReleaseEvent(QKeyEvent* event) {
            QWidget::keyReleaseEvent(event);
            const auto test = (event->modifiers() &Qt::AltModifier);
            updateCompileButton(test);
        }

        void CompilationDialog::focusInEvent(QFocusEvent* event) {
            QWidget::focusInEvent(event);
            const auto test = (QApplication::keyboardModifiers() == Qt::AltModifier);
            updateCompileButton(test);
        }

        void CompilationDialog::focusOutEvent(QFocusEvent* event) {
            QWidget::focusOutEvent(event);
            const auto test = (QApplication::keyboardModifiers() == Qt::AltModifier);
            updateCompileButton(test);
        }

        void CompilationDialog::updateCompileButton(const bool test) {
            if (m_run.running()) {
                m_compileButton->setText("Stop");
                m_compileButton->setEnabled(true);
            } else {
                if (test) {
                    m_compileButton->setText("Test");
                } else {
                    m_compileButton->setText("Compile");
                }
                const auto* profile = m_profileManager->selectedProfile();
                m_compileButton->setEnabled(profile != nullptr && profile->taskCount() > 0);
            }
        }

        void CompilationDialog::closeEvent(QCloseEvent* event) {
            if (m_run.running()) {
                const auto result = QMessageBox::warning(this, "Warning",
                    "Closing this dialog will stop the running compilation. Are you sure?",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (result == QMessageBox::Yes) {
                    m_run.terminate();
                    event->accept();
                } else {
                    event->ignore();
                }
            } else {
                event->accept();
            }

            if (event->isAccepted()) {
                m_mapFrame->compilationDialogWillClose();
            }
        }

        void CompilationDialog::launchEngine() {
            LaunchGameEngineDialog dialog(m_mapFrame->document(), this);
            dialog.exec();
        }

        void CompilationDialog::toggleCompile() {
            if (m_run.running()) {
                m_run.terminate();
            } else {
                const auto* profile = m_profileManager->selectedProfile();
                ensure(profile != nullptr, "profile is null");
                ensure(profile->taskCount() > 0, "profile has no tasks");

                m_output->setText("");

                const auto test = (QApplication::keyboardModifiers() == Qt::AltModifier);
                if (test) {
                    m_run.test(profile, m_mapFrame->document(), m_output);
                } else {
                    m_run.run(profile, m_mapFrame->document(), m_output);
                }

                m_currentRunLabel->setText(QString::fromStdString("Running " + profile->name()));
            }
        }

        void CompilationDialog::compilationStarted() {
            m_launchButton->setEnabled(false);
            updateCompileButton(false);
        }

        void CompilationDialog::compilationEnded() {
            m_launchButton->setEnabled(true);
            m_currentRunLabel->setText("");
            const auto test = (QApplication::keyboardModifiers() == Qt::AltModifier);
            updateCompileButton(test);
        }

        void CompilationDialog::selectedProfileChanged() {
            const auto test = (QApplication::keyboardModifiers() == Qt::AltModifier);
            updateCompileButton(test);
        }
    }
}
