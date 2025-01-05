/*
 Copyright (C) 2010 Kristian Duske

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

#include <QBoxLayout>
#include <QCheckBox>
#include <QCompleter>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "el/EvaluationContext.h"
#include "el/Interpolator.h"
#include "mdl/CompilationProfile.h"
#include "mdl/CompilationTask.h"
#include "ui/BorderLine.h"
#include "ui/CompilationVariables.h"
#include "ui/MultiCompletionLineEdit.h"
#include "ui/QtUtils.h"
#include "ui/TitledPanel.h"
#include "ui/VariableStoreModel.h"
#include "ui/ViewConstants.h"

#include "kdl/memory_utils.h"
#include "kdl/overload.h"

namespace tb::ui
{
// CompilationTaskEditorBase

CompilationTaskEditorBase::CompilationTaskEditorBase(
  QString title,
  std::weak_ptr<MapDocument> document,
  mdl::CompilationProfile& profile,
  mdl::CompilationTask& task,
  QWidget* parent)
  : ControlListBoxItemRenderer{parent}
  , m_title{std::move(title)}
  , m_document{std::move(document)}
  , m_profile{profile}
  , m_task{task}
{
  // request customContextMenuRequested() to be emitted
  setContextMenuPolicy(Qt::CustomContextMenu);

  auto* panel = new TitledPanel{m_title};

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(panel);
  layout->addWidget(new BorderLine{});
  setLayout(layout);

  m_enabledCheckbox = new QCheckBox{};
  m_enabledCheckbox->setToolTip(
    tr("Whether to include this task when running the compile profile"));

  m_taskLayout = new QHBoxLayout{};
  m_taskLayout->setContentsMargins(0, 0, 0, 0);
  m_taskLayout->addSpacing(LayoutConstants::NarrowHMargin);
  m_taskLayout->addWidget(m_enabledCheckbox, 0, Qt::AlignVCenter);
  m_taskLayout->addSpacing(LayoutConstants::NarrowHMargin);
  // subclasses call addMainLayout() to add their contents after the checkbox
  panel->getPanel()->setLayout(m_taskLayout);

  connect(m_enabledCheckbox, &QCheckBox::clicked, this, [&](const bool checked) {
    std::visit([&](auto& t) { t.enabled = checked; }, m_task);
  });
}

void CompilationTaskEditorBase::setupCompleter(MultiCompletionLineEdit* lineEdit)
{
  auto* completer = new QCompleter{};
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  lineEdit->setMultiCompleter(completer);
  lineEdit->setWordDelimiters(QRegularExpression{"\\$"}, QRegularExpression{"\\}"});

  m_completers.push_back(completer);
  updateCompleter(completer);
}

void CompilationTaskEditorBase::addMainLayout(QLayout* layout)
{
  m_taskLayout->addLayout(layout, 1);
}

void CompilationTaskEditorBase::updateItem()
{
  std::visit([&](const auto& t) { m_enabledCheckbox->setChecked(t.enabled); }, m_task);
}

void CompilationTaskEditorBase::updateCompleter(QCompleter* completer)
{
  auto workDir = std::string{};
  try
  {
    workDir = el::interpolate(
      m_profile.workDirSpec,
      el::EvaluationContext{CompilationWorkDirVariables{kdl::mem_lock(m_document)}});
  }
  catch (const Exception&)
  {
  }

  const auto variables = CompilationVariables{kdl::mem_lock(m_document), workDir};
  completer->setModel(new VariableStoreModel{variables});
}

// CompilationExportMapTaskEditor

CompilationExportMapTaskEditor::CompilationExportMapTaskEditor(
  std::weak_ptr<MapDocument> document,
  mdl::CompilationProfile& profile,
  mdl::CompilationTask& task,
  QWidget* parent)
  : CompilationTaskEditorBase{"Export Map", std::move(document), profile, task, parent}
{
  assert(std::holds_alternative<mdl::CompilationExportMap>(task));

  auto* formLayout = new QFormLayout{};
  formLayout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  addMainLayout(formLayout);

  m_targetEditor = new MultiCompletionLineEdit{};
  m_targetEditor->setFont(Fonts::fixedWidthFont());
  m_targetEditor->setToolTip(R"(The path of the exported file.
Variables are allowed.)");
  setupCompleter(m_targetEditor);
  formLayout->addRow("File Path", m_targetEditor);

  connect(
    m_targetEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationExportMapTaskEditor::targetSpecChanged);
}

void CompilationExportMapTaskEditor::updateItem()
{
  CompilationTaskEditorBase::updateItem();

  const auto targetSpec = QString::fromStdString(task().targetSpec);
  if (m_targetEditor->text() != targetSpec)
  {
    m_targetEditor->setText(targetSpec);
  }
}

mdl::CompilationExportMap& CompilationExportMapTaskEditor::task()
{
  // This is safe because we know what type of task the editor was initialized with.
  // We have to do this to avoid using a template as the base class.
  return std::get<mdl::CompilationExportMap>(m_task);
}

void CompilationExportMapTaskEditor::targetSpecChanged(const QString& text)
{
  task().targetSpec = text.toStdString();
}

CompilationCopyFilesTaskEditor::CompilationCopyFilesTaskEditor(
  std::weak_ptr<MapDocument> document,
  mdl::CompilationProfile& profile,
  mdl::CompilationTask& task,
  QWidget* parent)
  : CompilationTaskEditorBase{"Copy Files", std::move(document), profile, task, parent}
{
  assert(std::holds_alternative<mdl::CompilationCopyFiles>(task));

  auto* formLayout = new QFormLayout{};
  formLayout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  addMainLayout(formLayout);

  m_sourceEditor = new MultiCompletionLineEdit{};
  m_sourceEditor->setFont(Fonts::fixedWidthFont());
  m_sourceEditor->setToolTip(R"(The file(s) to copy.

Use wildcards (*,?) in the filename to specify more than one file.
Variables are allowed.)");
  setupCompleter(m_sourceEditor);
  formLayout->addRow("Source File Path", m_sourceEditor);

  m_targetEditor = new MultiCompletionLineEdit{};
  m_targetEditor->setFont(Fonts::fixedWidthFont());
  m_targetEditor->setToolTip(R"(The directory to copy the files to.

The directory is recursively created if it does not exist.
Existing files are overwritten without prompt.
Variables are allowed.)");
  setupCompleter(m_targetEditor);
  formLayout->addRow("Target Directory Path", m_targetEditor);

  connect(
    m_sourceEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationCopyFilesTaskEditor::sourceSpecChanged);
  connect(
    m_targetEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationCopyFilesTaskEditor::targetSpecChanged);
}

void CompilationCopyFilesTaskEditor::updateItem()
{
  CompilationTaskEditorBase::updateItem();

  const auto sourceSpec = QString::fromStdString(task().sourceSpec);
  if (m_sourceEditor->text() != sourceSpec)
  {
    m_sourceEditor->setText(sourceSpec);
  }

  const auto targetSpec = QString::fromStdString(task().targetSpec);
  if (m_targetEditor->text() != targetSpec)
  {
    m_targetEditor->setText(targetSpec);
  }
}

mdl::CompilationCopyFiles& CompilationCopyFilesTaskEditor::task()
{
  // This is safe because we know what type of task the editor was initialized with.
  // We have to do this to avoid using a template as the base class.
  return std::get<mdl::CompilationCopyFiles>(m_task);
}

void CompilationCopyFilesTaskEditor::sourceSpecChanged(const QString& text)
{
  task().sourceSpec = text.toStdString();
}

void CompilationCopyFilesTaskEditor::targetSpecChanged(const QString& text)
{
  task().targetSpec = text.toStdString();
}

CompilationRenameFileTaskEditor::CompilationRenameFileTaskEditor(
  std::weak_ptr<MapDocument> document,
  mdl::CompilationProfile& profile,
  mdl::CompilationTask& task,
  QWidget* parent)
  : CompilationTaskEditorBase{"Rename File", std::move(document), profile, task, parent}
{
  assert(std::holds_alternative<mdl::CompilationRenameFile>(task));

  auto* formLayout = new QFormLayout{};
  formLayout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  addMainLayout(formLayout);

  m_sourceEditor = new MultiCompletionLineEdit{};
  m_sourceEditor->setFont(Fonts::fixedWidthFont());
  m_sourceEditor->setToolTip(R"(The file to rename or move.

Wildcards are not supported.
Variables are allowed.)");
  setupCompleter(m_sourceEditor);
  formLayout->addRow("Source File Path", m_sourceEditor);

  m_targetEditor = new MultiCompletionLineEdit{};
  m_targetEditor->setFont(Fonts::fixedWidthFont());
  m_targetEditor->setToolTip(R"(The new path for the file.

The path must end in a filename.
The containing directory is recursively created if it does not exist.
Existing files are overwritten without prompt.
Variables are allowed.)");
  setupCompleter(m_targetEditor);
  formLayout->addRow("Target File Path", m_targetEditor);

  connect(
    m_sourceEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationRenameFileTaskEditor::sourceSpecChanged);
  connect(
    m_targetEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationRenameFileTaskEditor::targetSpecChanged);
}

void CompilationRenameFileTaskEditor::updateItem()
{
  CompilationTaskEditorBase::updateItem();

  const auto sourceSpec = QString::fromStdString(task().sourceSpec);
  if (m_sourceEditor->text() != sourceSpec)
  {
    m_sourceEditor->setText(sourceSpec);
  }

  const auto targetSpec = QString::fromStdString(task().targetSpec);
  if (m_targetEditor->text() != targetSpec)
  {
    m_targetEditor->setText(targetSpec);
  }
}

mdl::CompilationRenameFile& CompilationRenameFileTaskEditor::task()
{
  // This is safe because we know what type of task the editor was initialized with.
  // We have to do this to avoid using a template as the base class.
  return std::get<mdl::CompilationRenameFile>(m_task);
}

void CompilationRenameFileTaskEditor::sourceSpecChanged(const QString& text)
{
  task().sourceSpec = text.toStdString();
}

void CompilationRenameFileTaskEditor::targetSpecChanged(const QString& text)
{
  task().targetSpec = text.toStdString();
}

CompilationDeleteFilesTaskEditor::CompilationDeleteFilesTaskEditor(
  std::weak_ptr<MapDocument> document,
  mdl::CompilationProfile& profile,
  mdl::CompilationTask& task,
  QWidget* parent)
  : CompilationTaskEditorBase{"Delete Files", std::move(document), profile, task, parent}
{
  assert(std::holds_alternative<mdl::CompilationDeleteFiles>(task));

  auto* formLayout = new QFormLayout{};
  formLayout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  addMainLayout(formLayout);

  m_targetEditor = new MultiCompletionLineEdit{};
  m_targetEditor->setFont(Fonts::fixedWidthFont());
  m_targetEditor->setToolTip(R"(The file(s) to delete.

Use wildcards (*,?) in the filename to specify more than one file.
Variables are allowed.)");
  setupCompleter(m_targetEditor);
  formLayout->addRow("File Path", m_targetEditor);

  connect(
    m_targetEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationDeleteFilesTaskEditor::targetSpecChanged);
}

void CompilationDeleteFilesTaskEditor::updateItem()
{
  CompilationTaskEditorBase::updateItem();

  const auto targetSpec = QString::fromStdString(task().targetSpec);
  if (m_targetEditor->text() != targetSpec)
  {
    m_targetEditor->setText(targetSpec);
  }
}

mdl::CompilationDeleteFiles& CompilationDeleteFilesTaskEditor::task()
{
  // This is safe because we know what type of task the editor was initialized with.
  // We have to do this to avoid using a template as the base class.
  return std::get<mdl::CompilationDeleteFiles>(m_task);
}

void CompilationDeleteFilesTaskEditor::targetSpecChanged(const QString& text)
{
  task().targetSpec = text.toStdString();
}

// CompilationRunToolTaskEditor

CompilationRunToolTaskEditor::CompilationRunToolTaskEditor(
  std::weak_ptr<MapDocument> document,
  mdl::CompilationProfile& profile,
  mdl::CompilationTask& task,
  QWidget* parent)
  : CompilationTaskEditorBase{"Run Tool", std::move(document), profile, task, parent}
{
  assert(std::holds_alternative<mdl::CompilationRunTool>(task));

  auto* formLayout = new QFormLayout{};
  formLayout->setContentsMargins(
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::WideHMargin,
    LayoutConstants::WideVMargin);
  formLayout->setVerticalSpacing(LayoutConstants::NarrowVMargin);
  formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  addMainLayout(formLayout);

  m_toolEditor = new MultiCompletionLineEdit{};
  m_toolEditor->setFont(Fonts::fixedWidthFont());
  m_toolEditor->setToolTip(
    R"(The absolute path to the executable of the tool that should be run.

The working "directory is set to the profile's working directory if configured.
Variables are allowed.)");
  setupCompleter(m_toolEditor);

  auto* browseToolButton = new QPushButton{"..."};
  browseToolButton->setToolTip("Click to browse");

  auto* toolLayout = new QHBoxLayout{};
  toolLayout->setContentsMargins(0, 0, 0, 0);
  toolLayout->setSpacing(LayoutConstants::NarrowHMargin);
  toolLayout->addWidget(m_toolEditor, 1);
  toolLayout->addWidget(browseToolButton);

  formLayout->addRow("Tool Path", toolLayout);

  m_parametersEditor = new MultiCompletionLineEdit{};
  m_parametersEditor->setFont(Fonts::fixedWidthFont());
  m_parametersEditor->setToolTip(
    R"(The parameters that should be passed to the tool when it is executed.
Variables are allowed.)");

  setupCompleter(m_parametersEditor);
  formLayout->addRow("Parameters", m_parametersEditor);

  m_treatNonZeroResultCodeAsError = new QCheckBox{"Stop on nonzero error code"};
  m_treatNonZeroResultCodeAsError->setToolTip(
    tr("Stop compilation if the tool returns a nonzero error code"));
  formLayout->addRow("", m_treatNonZeroResultCodeAsError);

  connect(
    m_toolEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationRunToolTaskEditor::toolSpecChanged);
  connect(
    browseToolButton,
    &QPushButton::clicked,
    this,
    &CompilationRunToolTaskEditor::browseTool);
  connect(
    m_parametersEditor,
    &QLineEdit::textChanged,
    this,
    &CompilationRunToolTaskEditor::parameterSpecChanged);
  connect(
    m_treatNonZeroResultCodeAsError,
    &QCheckBox::checkStateChanged,
    this,
    &CompilationRunToolTaskEditor::treatNonZeroResultCodeAsErrorChanged);
}

void CompilationRunToolTaskEditor::updateItem()
{
  CompilationTaskEditorBase::updateItem();

  const auto toolSpec = QString::fromStdString(task().toolSpec);
  if (m_toolEditor->text() != toolSpec)
  {
    m_toolEditor->setText(toolSpec);
  }

  const auto parametersSpec = QString::fromStdString(task().parameterSpec);
  if (m_parametersEditor->text() != parametersSpec)
  {
    m_parametersEditor->setText(parametersSpec);
  }

  if (
    m_treatNonZeroResultCodeAsError->isChecked() != task().treatNonZeroResultCodeAsError)
  {
    m_treatNonZeroResultCodeAsError->setCheckState(
      task().treatNonZeroResultCodeAsError ? Qt::CheckState::Checked
                                           : Qt::CheckState::Unchecked);
  }
}

mdl::CompilationRunTool& CompilationRunToolTaskEditor::task()
{
  // This is safe because we know what type of task the editor was initialized with.
  // We have to do this to avoid using a template as the base class.
  return std::get<mdl::CompilationRunTool>(m_task);
}

void CompilationRunToolTaskEditor::browseTool()
{
  const QString toolSpec = QFileDialog::getOpenFileName(
    this, tr("Select Tool"), fileDialogDefaultDirectory(FileDialogDir::CompileTool));
  if (!toolSpec.isEmpty())
  {
    updateFileDialogDefaultDirectoryWithFilename(FileDialogDir::CompileTool, toolSpec);

    // will call toolSpecChanged and update the model there
    m_toolEditor->setText(toolSpec);
  }
}

void CompilationRunToolTaskEditor::toolSpecChanged(const QString& text)
{
  task().toolSpec = text.toStdString();
}

void CompilationRunToolTaskEditor::parameterSpecChanged(const QString& text)
{
  task().parameterSpec = text.toStdString();
}

void CompilationRunToolTaskEditor::treatNonZeroResultCodeAsErrorChanged(const int state)
{
  const auto value = (state == Qt::Checked);
  task().treatNonZeroResultCodeAsError = value;
}

// CompilationTaskListBox

CompilationTaskListBox::CompilationTaskListBox(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : ControlListBox{"Click the '+' button to create a task.", QMargins{}, false, parent}
  , m_document{std::move(document)}
{
}

void CompilationTaskListBox::setProfile(mdl::CompilationProfile* profile)
{
  m_profile = profile;
  reload();
}

void CompilationTaskListBox::reloadTasks()
{
  reload();
}

size_t CompilationTaskListBox::itemCount() const
{
  return m_profile ? m_profile->tasks.size() : 0;
}

ControlListBoxItemRenderer* CompilationTaskListBox::createItemRenderer(
  QWidget* parent, const size_t index)
{
  ensure(m_profile != nullptr, "profile is null");

  auto& task = m_profile->tasks[index];
  auto* renderer = std::visit(
    kdl::overload(
      [&](const mdl::CompilationExportMap&) -> ControlListBoxItemRenderer* {
        return new CompilationExportMapTaskEditor{m_document, *m_profile, task, parent};
      },
      [&](const mdl::CompilationCopyFiles&) -> ControlListBoxItemRenderer* {
        return new CompilationCopyFilesTaskEditor{m_document, *m_profile, task, parent};
      },
      [&](const mdl::CompilationRenameFile&) -> ControlListBoxItemRenderer* {
        return new CompilationRenameFileTaskEditor{m_document, *m_profile, task, parent};
      },
      [&](const mdl::CompilationDeleteFiles&) -> ControlListBoxItemRenderer* {
        return new CompilationDeleteFilesTaskEditor(m_document, *m_profile, task, parent);
      },
      [&](const mdl::CompilationRunTool&) -> ControlListBoxItemRenderer* {
        return new CompilationRunToolTaskEditor{m_document, *m_profile, task, parent};
      }),
    task);

  connect(
    renderer,
    &QWidget::customContextMenuRequested,
    this,
    [&, renderer, index](const QPoint& pos) {
      emit this->taskContextMenuRequested(
        renderer->mapToGlobal(pos), m_profile->tasks[index]);
    });

  return renderer;
}

} // namespace tb::ui
