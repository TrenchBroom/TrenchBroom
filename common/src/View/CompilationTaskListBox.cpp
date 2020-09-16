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

#include "CompilationTaskListBox.h"

#include "EL/EvaluationContext.h"
#include "EL/Interpolator.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"
#include "View/BorderLine.h"
#include "View/CompilationVariables.h"
#include "View/MultiCompletionLineEdit.h"
#include "View/QtUtils.h"
#include "View/TitleBar.h"
#include "View/TitledPanel.h"
#include "View/VariableStoreModel.h"
#include "View/ViewConstants.h"

#include <kdl/memory_utils.h>

#include <QBoxLayout>
#include <QCheckBox>
#include <QCompleter>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

namespace TrenchBroom {
    namespace View {
        // CompilationTaskEditorBase

        CompilationTaskEditorBase::CompilationTaskEditorBase(const QString& title, std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationTask& task, QWidget* parent) :
        ControlListBoxItemRenderer(parent),
        m_title(title),
        m_document(std::move(document)),
        m_profile(&profile),
        m_task(&task),
        m_enabledCheckbox(nullptr),
        m_taskLayout(nullptr) {
            auto* panel = new TitledPanel(m_title);

            auto* layout = new QVBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(panel);
            layout->addWidget(new BorderLine());
            setLayout(layout);

            m_enabledCheckbox = new QCheckBox();
            m_enabledCheckbox->setToolTip(tr("Whether to include this task when running the compile profile"));

            m_taskLayout = new QHBoxLayout();
            m_taskLayout->setContentsMargins(0, 0, 0, 0);
            m_taskLayout->addSpacing(LayoutConstants::NarrowHMargin);
            m_taskLayout->addWidget(m_enabledCheckbox, 0, Qt:: AlignVCenter);
            m_taskLayout->addSpacing(LayoutConstants::NarrowHMargin);
            // subclasses call addMainLayout() to add their contents after the checkbox
            panel->getPanel()->setLayout(m_taskLayout);

            connect(m_enabledCheckbox, &QCheckBox::clicked, this, [&](const bool checked) {
                m_task->setEnabled(checked);
            });
        }

        void CompilationTaskEditorBase::setupCompleter(MultiCompletionLineEdit* lineEdit) {
            auto* completer = new QCompleter();
            completer->setCaseSensitivity(Qt::CaseInsensitive);
            lineEdit->setMultiCompleter(completer);
            lineEdit->setWordDelimiters(QRegularExpression("\\$"), QRegularExpression("\\}"));

            m_completers.push_back(completer);
            updateCompleter(completer);
        }

        void CompilationTaskEditorBase::addMainLayout(QLayout* layout) {
            m_taskLayout->addLayout(layout, 1);
        }

        void CompilationTaskEditorBase::updateItem() {
            m_enabledCheckbox->setChecked(m_task->enabled());
        }

        void CompilationTaskEditorBase::updateCompleter(QCompleter* completer) {
            const auto workDir = EL::interpolate(m_profile->workDirSpec(), EL::EvaluationContext(CompilationWorkDirVariables(
                kdl::mem_lock(m_document))));
            const auto variables = CompilationVariables(kdl::mem_lock(m_document), workDir);
            completer->setModel(new VariableStoreModel(variables));
        }

        // CompilationExportMapTaskEditor

        CompilationExportMapTaskEditor::CompilationExportMapTaskEditor(std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationExportMap& task, QWidget* parent) :
        CompilationTaskEditorBase("Export Map", std::move(document), profile, task, parent),
        m_targetEditor(nullptr) {
            auto* formLayout = new QFormLayout();
            formLayout->setContentsMargins(LayoutConstants::WideHMargin, LayoutConstants::WideVMargin, LayoutConstants::WideHMargin, LayoutConstants::WideVMargin);
            formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
            formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            addMainLayout(formLayout);

            m_targetEditor = new MultiCompletionLineEdit();
            setupCompleter(m_targetEditor);
            formLayout->addRow("Target", m_targetEditor);

            connect(m_targetEditor, &QLineEdit::textChanged, this, &CompilationExportMapTaskEditor::targetSpecChanged);
        }

        void CompilationExportMapTaskEditor::updateItem() {
            CompilationTaskEditorBase::updateItem();

            const auto targetSpec = QString::fromStdString(task().targetSpec());
            if (m_targetEditor->text() != targetSpec) {
                m_targetEditor->setText(targetSpec);
            }
        }

        Model::CompilationExportMap& CompilationExportMapTaskEditor::task() {
            // This is safe because we know what type of task the editor was initialized with.
            // We have to do this to avoid using a template as the base class.
            return static_cast<Model::CompilationExportMap&>(*m_task);
        }

        void CompilationExportMapTaskEditor::targetSpecChanged(const QString& text) {
            const auto targetSpec = text.toStdString();
            if (task().targetSpec() != targetSpec) {
                task().setTargetSpec(targetSpec);
            }
        }

        CompilationCopyFilesTaskEditor::CompilationCopyFilesTaskEditor(std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationCopyFiles& task, QWidget* parent) :
        CompilationTaskEditorBase("Copy Files", std::move(document), profile, task, parent),
        m_sourceEditor(nullptr),
        m_targetEditor(nullptr) {
            auto* formLayout = new QFormLayout();
            formLayout->setContentsMargins(LayoutConstants::WideHMargin, LayoutConstants::WideVMargin, LayoutConstants::WideHMargin, LayoutConstants::WideVMargin);
            formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
            formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            addMainLayout(formLayout);

            m_sourceEditor = new MultiCompletionLineEdit();
            setupCompleter(m_sourceEditor);
            formLayout->addRow("Source", m_sourceEditor);

            m_targetEditor = new MultiCompletionLineEdit();
            setupCompleter(m_targetEditor);
            formLayout->addRow("Target", m_targetEditor);

            connect(m_sourceEditor, &QLineEdit::textChanged, this, &CompilationCopyFilesTaskEditor::sourceSpecChanged);
            connect(m_targetEditor, &QLineEdit::textChanged, this, &CompilationCopyFilesTaskEditor::targetSpecChanged);
        }

        void CompilationCopyFilesTaskEditor::updateItem() {
            CompilationTaskEditorBase::updateItem();

            const auto sourceSpec = QString::fromStdString(task().sourceSpec());
            if (m_sourceEditor->text() != sourceSpec) {
                m_sourceEditor->setText(sourceSpec);
            }

            const auto targetSpec = QString::fromStdString(task().targetSpec());
            if (m_targetEditor->text() != targetSpec) {
                m_targetEditor->setText(targetSpec);
            }
        }

        Model::CompilationCopyFiles& CompilationCopyFilesTaskEditor::task() {
            // This is safe because we know what type of task the editor was initialized with.
            // We have to do this to avoid using a template as the base class.
            return static_cast<Model::CompilationCopyFiles&>(*m_task);
        }

        void CompilationCopyFilesTaskEditor::sourceSpecChanged(const QString& text) {
            const auto sourceSpec = text.toStdString();
            if (task().sourceSpec() != sourceSpec) {
                task().setSourceSpec(sourceSpec);
            }
        }

        void CompilationCopyFilesTaskEditor::targetSpecChanged(const QString& text) {
            const auto targetSpec = text.toStdString();
            if (task().targetSpec() != targetSpec) {
                task().setTargetSpec(targetSpec);
            }
        }

        // CompilationRunToolTaskEditor

        CompilationRunToolTaskEditor::CompilationRunToolTaskEditor(std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, Model::CompilationRunTool& task, QWidget* parent) :
        CompilationTaskEditorBase("Run Tool", std::move(document), profile, task, parent),
        m_toolEditor(nullptr),
        m_parametersEditor(nullptr) {
            auto* formLayout = new QFormLayout();
            formLayout->setContentsMargins(LayoutConstants::WideHMargin, LayoutConstants::WideVMargin, LayoutConstants::WideHMargin, LayoutConstants::WideVMargin);
            formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
            formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
            addMainLayout(formLayout);

            m_toolEditor = new MultiCompletionLineEdit();
            setupCompleter(m_toolEditor);

            auto* browseToolButton = new QPushButton("...");
            browseToolButton->setToolTip("Click to browse");

            auto* toolLayout = new QHBoxLayout();
            toolLayout->setContentsMargins(0, 0, 0, 0);
            toolLayout->setSpacing(LayoutConstants::NarrowHMargin);
            toolLayout->addWidget(m_toolEditor, 1);
            toolLayout->addWidget(browseToolButton);

            formLayout->addRow("Tool", toolLayout);

            m_parametersEditor = new MultiCompletionLineEdit();
            setupCompleter(m_parametersEditor);
            formLayout->addRow("Parameters", m_parametersEditor);

            connect(m_toolEditor, &QLineEdit::textChanged, this, &CompilationRunToolTaskEditor::toolSpecChanged);
            connect(browseToolButton, &QPushButton::clicked, this, &CompilationRunToolTaskEditor::browseTool);
            connect(m_parametersEditor, &QLineEdit::textChanged, this, &CompilationRunToolTaskEditor::parameterSpecChanged);
        }

        void CompilationRunToolTaskEditor::updateItem() {
            CompilationTaskEditorBase::updateItem();

            const auto toolSpec = QString::fromStdString(task().toolSpec());
            if (m_toolEditor->text() != toolSpec) {
                m_toolEditor->setText(toolSpec);
            }

            const auto parametersSpec = QString::fromStdString(task().parameterSpec());
            if (m_parametersEditor->text() != parametersSpec) {
                m_parametersEditor->setText(parametersSpec);
            }

        }

        Model::CompilationRunTool& CompilationRunToolTaskEditor::task() {
            // This is safe because we know what type of task the editor was initialized with.
            // We have to do this to avoid using a template as the base class.
            return static_cast<Model::CompilationRunTool&>(*m_task);
        }

        void CompilationRunToolTaskEditor::browseTool() {
            const QString toolSpec = QFileDialog::getOpenFileName(this, tr("Select Tool"), fileDialogDefaultDirectory(FileDialogDir::CompileTool));
            if (!toolSpec.isEmpty()) {
                updateFileDialogDefaultDirectoryWithFilename(FileDialogDir::CompileTool, toolSpec);

                // will call toolSpecChanged and update the model there
                m_toolEditor->setText(toolSpec);
            }
        }

        void CompilationRunToolTaskEditor::toolSpecChanged(const QString& text) {
            const auto toolSpec = text.toStdString();
            if (task().toolSpec() != toolSpec) {
                task().setToolSpec(toolSpec);
            }
        }

        void CompilationRunToolTaskEditor::parameterSpecChanged(const QString& text) {
            const auto parameterSpec = text.toStdString();
            if (task().parameterSpec() != parameterSpec) {
                task().setParameterSpec(parameterSpec);
            }
        }

        // CompilationTaskListBox

        CompilationTaskListBox::CompilationTaskListBox(std::weak_ptr<MapDocument> document, QWidget* parent) :
        ControlListBox("Click the '+' button to create a task.", QMargins(), false, parent),
        m_document(std::move(document)),
        m_profile(nullptr) {}

        void CompilationTaskListBox::setProfile(Model::CompilationProfile* profile) {
            m_profile = profile;
            reload();
        }

        void CompilationTaskListBox::reloadTasks() {
            reload();
        }

        class CompilationTaskListBox::CompilationTaskEditorFactory : public Model::CompilationTaskVisitor {
        private:
            std::weak_ptr<MapDocument> m_document;
            Model::CompilationProfile& m_profile;
            QWidget* m_parent;
            ControlListBoxItemRenderer* m_result;
        public:
            CompilationTaskEditorFactory(std::weak_ptr<MapDocument> document, Model::CompilationProfile& profile, QWidget* parent) :
            m_document(std::move(document)),
            m_profile(profile),
            m_parent(parent),
            m_result(nullptr) {}

            ControlListBoxItemRenderer* result() const {
                return m_result;
            }

            void visit(Model::CompilationExportMap& task) override {
                m_result = new CompilationExportMapTaskEditor(m_document, m_profile, task, m_parent);
            }

            void visit(Model::CompilationCopyFiles& task) override {
                m_result = new CompilationCopyFilesTaskEditor(m_document, m_profile, task, m_parent);
            }

            void visit(Model::CompilationRunTool& task) override {
                m_result = new CompilationRunToolTaskEditor(m_document, m_profile, task, m_parent);
            }
        };

        size_t CompilationTaskListBox::itemCount() const {
            if (m_profile == nullptr) {
                return 0;
            }
            return m_profile->taskCount();
        }

        ControlListBoxItemRenderer* CompilationTaskListBox::createItemRenderer(QWidget* parent, const size_t index) {
            ensure(m_profile != nullptr, "profile is null");

            CompilationTaskEditorFactory factory(m_document, *m_profile, parent);
            auto* task = m_profile->task(index);
            task->accept(factory);
            return factory.result();
        }
    }
}
