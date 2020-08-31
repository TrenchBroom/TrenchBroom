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

#include "CompilationProfileEditor.h"

#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"
#include "View/BorderLine.h"
#include "View/CompilationTaskListBox.h"
#include "View/CompilationVariables.h"
#include "View/MultiCompletionLineEdit.h"
#include "View/VariableStoreModel.h"
#include "View/ViewConstants.h"
#include "View/QtUtils.h"

#include <kdl/memory_utils.h>

#include <QAbstractButton>
#include <QCompleter>
#include <QFormLayout>
#include <QLineEdit>
#include <QMenu>
#include <QStackedWidget>

namespace TrenchBroom {
    namespace View {
        CompilationProfileEditor::CompilationProfileEditor(std::weak_ptr<MapDocument> document, QWidget* parent) :
        QWidget(parent),
        m_document(document),
        m_profile(nullptr),
        m_stackedWidget(nullptr),
        m_nameTxt(nullptr),
        m_workDirTxt(nullptr),
        m_taskList(nullptr),
        m_addTaskButton(nullptr),
        m_removeTaskButton(nullptr),
        m_moveTaskUpButton(nullptr),
        m_moveTaskDownButton(nullptr) {
            setBaseWindowColor(this);

            m_stackedWidget = new QStackedWidget(this);
            m_stackedWidget->addWidget(createDefaultPage("Select a compilation profile", m_stackedWidget));
            m_stackedWidget->addWidget(createEditorPage(m_stackedWidget));

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(m_stackedWidget);
            setLayout(layout);
        }

        CompilationProfileEditor::~CompilationProfileEditor() {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &CompilationProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &CompilationProfileEditor::profileDidChange);
            }
        }

        QWidget* CompilationProfileEditor::createEditorPage(QWidget* parent) {
            auto* containerPanel = new QWidget(parent);
            auto* upperPanel = new QWidget(containerPanel);
            setDefaultWindowColor(upperPanel);


            m_nameTxt = new QLineEdit();
            m_workDirTxt = new MultiCompletionLineEdit();

            const auto variables = CompilationWorkDirVariables(kdl::mem_lock(m_document));
            auto* completer = new QCompleter(new VariableStoreModel(variables));
            completer->setCaseSensitivity(Qt::CaseInsensitive);

            m_workDirTxt->setMultiCompleter(completer);
            m_workDirTxt->setWordDelimiters(QRegularExpression("\\$"), QRegularExpression("\\}"));

            auto* upperLayout = new QFormLayout();
            upperLayout->setContentsMargins(LayoutConstants::MediumHMargin, LayoutConstants::WideVMargin, LayoutConstants::MediumHMargin, LayoutConstants::WideVMargin);
            upperLayout->setHorizontalSpacing(LayoutConstants::MediumHMargin);
            upperLayout->setVerticalSpacing(0);
            upperLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            upperLayout->addRow("Name", m_nameTxt);
            upperLayout->addRow("Working Directory", m_workDirTxt);
            upperPanel->setLayout(upperLayout);

            m_taskList = new CompilationTaskListBox(m_document, containerPanel);

            m_addTaskButton = createBitmapButton("Add.svg", "Add task");
            m_removeTaskButton = createBitmapButton("Remove.svg", "Remove the selected task");
            m_moveTaskUpButton = createBitmapButton("Up.svg", "Move the selected task up");
            m_moveTaskDownButton = createBitmapButton("Down.svg", "Move the selected task down");

            auto* buttonLayout = createMiniToolBarLayout(m_addTaskButton, m_removeTaskButton, m_moveTaskUpButton, m_moveTaskDownButton);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(upperPanel);
            layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
            layout->addWidget(m_taskList, 1);
            layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal));
            layout->addLayout(buttonLayout);

            containerPanel->setLayout(layout);

            connect(m_nameTxt, &QLineEdit::editingFinished, this, &CompilationProfileEditor::nameChanged);
            connect(m_workDirTxt, &QLineEdit::editingFinished, this, &CompilationProfileEditor::workDirChanged);
            connect(m_taskList, &ControlListBox::itemSelectionChanged, this, &CompilationProfileEditor::taskSelectionChanged);
            connect(m_addTaskButton, &QAbstractButton::clicked, this, &CompilationProfileEditor::addTask);
            connect(m_removeTaskButton, &QAbstractButton::clicked, this, &CompilationProfileEditor::removeTask);
            connect(m_moveTaskUpButton, &QAbstractButton::clicked, this, &CompilationProfileEditor::moveTaskUp);
            connect(m_moveTaskDownButton, &QAbstractButton::clicked, this, &CompilationProfileEditor::moveTaskDown);

            return containerPanel;
        }

        void CompilationProfileEditor::nameChanged() {
            ensure(m_profile != nullptr, "profile is null");
            const auto name = m_nameTxt->text().toStdString();
            if (m_profile->name() != name) {
                m_profile->setName(name);
            }
        }

        void CompilationProfileEditor::workDirChanged() {
            ensure(m_profile != nullptr, "profile is null");
            const auto workDirSpec = m_workDirTxt->text().toStdString();
            if (m_profile->workDirSpec() != workDirSpec) {
                m_profile->setWorkDirSpec(workDirSpec);
            }
        }

        void CompilationProfileEditor::addTask() {
            QMenu menu;
            auto* exportMapAction = menu.addAction("Export Map");
            auto* copyFilesAction = menu.addAction("Copy Files");
            auto* runToolAction   = menu.addAction("Run Tool");

            std::unique_ptr<Model::CompilationTask> task = nullptr;
            auto* chosenAction = menu.exec(QCursor::pos());
            if (chosenAction == exportMapAction) {
                task = std::make_unique<Model::CompilationExportMap>("${WORK_DIR_PATH}/${MAP_BASE_NAME}-compile.map");
            } else if (chosenAction == copyFilesAction) {
                task = std::make_unique<Model::CompilationCopyFiles>("", "");
            } else if (chosenAction == runToolAction) {
                task = std::make_unique<Model::CompilationRunTool>("", "");
            } else {
                return;
            }

            const int index = m_taskList->currentRow();
            if (index < 0) {
                m_profile->addTask(std::move(task));
                m_taskList->setCurrentRow(static_cast<int>(m_profile->taskCount()) - 1);
            } else {
                m_profile->insertTask(static_cast<size_t>(index + 1), std::move(task));
                m_taskList->setCurrentRow(index + 1);
            }
        }

        void CompilationProfileEditor::removeTask() {
            const int index = m_taskList->currentRow();
            assert(index >= 0);

            if (m_profile->taskCount() == 1) {
                m_taskList->setCurrentRow(-1);
                m_profile->removeTask(static_cast<size_t>(index));
            } else if (index > 0) {
                m_taskList->setCurrentRow(index - 1);
                m_profile->removeTask(static_cast<size_t>(index));
            } else {
                m_taskList->setCurrentRow(1);
                m_profile->removeTask(static_cast<size_t>(index));
                m_taskList->setCurrentRow(0);
            }
        }


        void CompilationProfileEditor::moveTaskUp() {
            const int index = m_taskList->currentRow();
            assert(index > 0);
            m_profile->moveTaskUp(static_cast<size_t>(index));
            m_taskList->setCurrentRow(index - 1);
        }

        void CompilationProfileEditor::moveTaskDown() {
            const int index = m_taskList->currentRow();
            assert(index >= 0 && index < static_cast<int>(m_profile->taskCount()) - 1);
            m_profile->moveTaskDown(static_cast<size_t>(index));
            m_taskList->setCurrentRow(index + 1);
        }

        void CompilationProfileEditor::taskSelectionChanged() {
            refresh();
        }

        void CompilationProfileEditor::setProfile(Model::CompilationProfile* profile) {
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.removeObserver(this, &CompilationProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &CompilationProfileEditor::profileDidChange);
            }
            m_profile = profile;
            m_taskList->setProfile(profile);
            if (m_profile != nullptr) {
                m_profile->profileWillBeRemoved.addObserver(this, &CompilationProfileEditor::profileWillBeRemoved);
                m_profile->profileDidChange.addObserver(this, &CompilationProfileEditor::profileDidChange);
                m_stackedWidget->setCurrentIndex(1);
            } else {
                m_stackedWidget->setCurrentIndex(0);
            }
            refresh();
        }

        void CompilationProfileEditor::profileWillBeRemoved() {
            setProfile(nullptr);
        }

        void CompilationProfileEditor::profileDidChange() {
            refresh();
            emit profileChanged();
        }

        void CompilationProfileEditor::refresh() {
            if (m_profile != nullptr) {
                if (m_nameTxt->text().toStdString() != m_profile->name()) {
                    m_nameTxt->setText(QString::fromStdString(m_profile->name()));
                }
                if (m_workDirTxt->text().toStdString() != m_profile->workDirSpec()) {
                    m_workDirTxt->setText(QString::fromStdString(m_profile->workDirSpec()));
                }
            }
            m_addTaskButton->setEnabled(m_profile != nullptr);
            m_removeTaskButton->setEnabled(m_profile != nullptr && m_taskList->currentRow() >= 0);
            m_moveTaskUpButton->setEnabled(m_profile != nullptr && m_taskList->currentRow() > 0);
            m_moveTaskDownButton->setEnabled(m_profile != nullptr && m_taskList->currentRow() >= 0 && m_taskList->currentRow() < static_cast<int>(m_profile->taskCount()) - 1);
        }
    }
}
