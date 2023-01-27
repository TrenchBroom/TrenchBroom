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
#include "View/QtUtils.h"
#include "View/VariableStoreModel.h"
#include "View/ViewConstants.h"

#include "kdl/vector_utils.h"
#include <kdl/memory_utils.h>

#include <QCompleter>
#include <QFormLayout>
#include <QLineEdit>
#include <QMenu>
#include <QStackedWidget>
#include <QToolButton>

namespace TrenchBroom
{
namespace View
{
CompilationProfileEditor::CompilationProfileEditor(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  setBaseWindowColor(this);

  m_stackedWidget = new QStackedWidget{this};
  m_stackedWidget->addWidget(
    createDefaultPage("Select a compilation profile", m_stackedWidget));
  m_stackedWidget->addWidget(createEditorPage(m_stackedWidget));

  auto* layout = new QHBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(m_stackedWidget);
  setLayout(layout);
}

QWidget* CompilationProfileEditor::createEditorPage(QWidget* parent)
{
  auto* containerPanel = new QWidget{parent};
  auto* upperPanel = new QWidget{containerPanel};
  setDefaultWindowColor(upperPanel);

  m_nameTxt = new QLineEdit{};
  m_workDirTxt = new MultiCompletionLineEdit{};

  const auto variables = CompilationWorkDirVariables{kdl::mem_lock(m_document)};
  auto* completer = new QCompleter{new VariableStoreModel{variables}};
  completer->setCaseSensitivity(Qt::CaseInsensitive);

  m_workDirTxt->setMultiCompleter(completer);
  m_workDirTxt->setWordDelimiters(QRegularExpression{"\\$"}, QRegularExpression{"\\}"});
  m_workDirTxt->setFont(Fonts::fixedWidthFont());
  m_workDirTxt->setToolTip(R"(A working directory for the compilation profile.
Variables are allowed.)");

  auto* upperLayout = new QFormLayout{};
  upperLayout->setContentsMargins(
    LayoutConstants::MediumHMargin,
    LayoutConstants::WideVMargin,
    LayoutConstants::MediumHMargin,
    LayoutConstants::WideVMargin);
  upperLayout->setHorizontalSpacing(LayoutConstants::MediumHMargin);
  upperLayout->setVerticalSpacing(0);
  upperLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  upperLayout->addRow("Name", m_nameTxt);
  upperLayout->addRow("Working Directory", m_workDirTxt);
  upperPanel->setLayout(upperLayout);

  m_taskList = new CompilationTaskListBox{m_document, containerPanel};

  m_addTaskButton = createBitmapButton("Add.svg", "Add task");
  m_removeTaskButton = createBitmapButton("Remove.svg", "Remove the selected task");
  m_moveTaskUpButton = createBitmapButton("Up.svg", "Move the selected task up");
  m_moveTaskDownButton = createBitmapButton("Down.svg", "Move the selected task down");

  auto* buttonLayout = createMiniToolBarLayout(
    m_addTaskButton, m_removeTaskButton, m_moveTaskUpButton, m_moveTaskDownButton);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(upperPanel);
  layout->addWidget(new BorderLine{BorderLine::Direction::Horizontal});
  layout->addWidget(m_taskList, 1);
  layout->addWidget(new BorderLine{BorderLine::Direction::Horizontal});
  layout->addLayout(buttonLayout);

  containerPanel->setLayout(layout);

  connect(
    m_nameTxt, &QLineEdit::textChanged, this, &CompilationProfileEditor::nameChanged);
  connect(
    m_workDirTxt,
    &QLineEdit::textChanged,
    this,
    &CompilationProfileEditor::workDirChanged);
  connect(
    m_taskList,
    &ControlListBox::itemSelectionChanged,
    this,
    &CompilationProfileEditor::taskSelectionChanged);
  connect(
    m_taskList,
    &CompilationTaskListBox::taskContextMenuRequested,
    this,
    [&](const QPoint& globalPos, const Model::CompilationTask& task) {
      const auto index = static_cast<int>(*kdl::vec_index_of(m_profile->tasks, task));

      auto menu = QMenu{this};
      auto* moveUpAction =
        menu.addAction(tr("Move Up"), this, [=]() { moveTaskUp(index); });
      auto* moveDownAction =
        menu.addAction(tr("Move Down"), this, [=]() { moveTaskDown(index); });
      menu.addSeparator();
      menu.addAction(tr("Duplicate"), this, [=]() { duplicateTask(index); });
      menu.addAction(tr("Remove"), this, [=]() { removeTask(index); });

      moveUpAction->setEnabled(index > 0);
      moveDownAction->setEnabled(
        static_cast<size_t>(index + 1) < m_profile->tasks.size());

      menu.exec(globalPos);
    });
  connect(
    m_addTaskButton, &QAbstractButton::clicked, this, &CompilationProfileEditor::addTask);
  connect(
    m_removeTaskButton,
    &QAbstractButton::clicked,
    this,
    qOverload<>(&CompilationProfileEditor::removeTask));
  connect(
    m_moveTaskUpButton,
    &QAbstractButton::clicked,
    this,
    qOverload<>(&CompilationProfileEditor::moveTaskUp));
  connect(
    m_moveTaskDownButton,
    &QAbstractButton::clicked,
    this,
    qOverload<>(&CompilationProfileEditor::moveTaskDown));

  return containerPanel;
}

void CompilationProfileEditor::nameChanged(const QString& text)
{
  ensure(m_profile != nullptr, "profile is null");
  auto name = text.toStdString();
  if (m_profile->name != name)
  {
    m_profile->name = std::move(name);
    emit profileChanged();
  }
}

void CompilationProfileEditor::workDirChanged(const QString& text)
{
  ensure(m_profile != nullptr, "profile is null");
  auto workDirSpec = text.toStdString();
  if (m_profile->workDirSpec != workDirSpec)
  {
    m_profile->workDirSpec = std::move(workDirSpec);
    emit profileChanged();
  }
}

void CompilationProfileEditor::addTask()
{
  auto menu = QMenu{};
  auto* exportMapAction = menu.addAction("Export Map");
  auto* copyFilesAction = menu.addAction("Copy Files");
  auto* renameFileAction = menu.addAction("Rename File");
  auto* deleteFilesAction = menu.addAction("Delete Files");
  auto* runToolAction = menu.addAction("Run Tool");

  auto task = [&](const auto* chosenAction) -> std::optional<Model::CompilationTask> {
    if (chosenAction == exportMapAction)
    {
      return Model::CompilationExportMap{
        true, "${WORK_DIR_PATH}/${MAP_BASE_NAME}-compile.map"};
    }
    if (chosenAction == copyFilesAction)
    {
      return Model::CompilationCopyFiles{true, "", ""};
    }
    if (chosenAction == renameFileAction)
    {
      return Model::CompilationRenameFile{true, "", ""};
    }
    if (chosenAction == deleteFilesAction)
    {
      return Model::CompilationDeleteFiles{true, ""};
    }
    if (chosenAction == runToolAction)
    {
      return Model::CompilationRunTool{true, "", ""};
    }
    {
      return std::nullopt;
    }
  }(menu.exec(QCursor::pos()));

  if (task)
  {
    const auto index = m_taskList->currentRow();
    if (index < 0)
    {
      m_profile->tasks.push_back(std::move(*task));
      m_taskList->reloadTasks();
      m_taskList->setCurrentRow(static_cast<int>(m_profile->tasks.size()) - 1);
    }
    else
    {
      m_profile->tasks.insert(
        std::next(m_profile->tasks.begin(), index + 1), std::move(*task));
      m_taskList->reloadTasks();
      m_taskList->setCurrentRow(index + 1);
    }
    emit profileChanged();
  }
}

void CompilationProfileEditor::removeTask()
{
  removeTask(m_taskList->currentRow());
}

void CompilationProfileEditor::removeTask(const int index)
{
  assert(index >= 0);

  if (m_profile->tasks.size() == 1)
  {
    m_taskList->setCurrentRow(-1);
    m_profile->tasks.clear();
    m_taskList->reloadTasks();
  }
  else if (index > 0)
  {
    m_taskList->setCurrentRow(index - 1);
    kdl::vec_erase_at(m_profile->tasks, size_t(index));
    m_taskList->reloadTasks();
  }
  else
  {
    m_taskList->setCurrentRow(1);
    kdl::vec_erase_at(m_profile->tasks, size_t(index));
    m_taskList->reloadTasks();
    m_taskList->setCurrentRow(0);
  }
  emit profileChanged();
}

void CompilationProfileEditor::duplicateTask(const int index)
{
  auto task = m_profile->tasks[size_t(index)];
  m_profile->tasks.insert(
    std::next(m_profile->tasks.begin(), index + 1), std::move(task));
  m_taskList->reloadTasks();
  m_taskList->setCurrentRow(index + 1);
  emit profileChanged();
}

void CompilationProfileEditor::moveTaskUp()
{
  moveTaskUp(m_taskList->currentRow());
}

void CompilationProfileEditor::moveTaskUp(const int index)
{
  assert(index > 0);

  auto it = std::next(m_profile->tasks.begin(), index);
  std::iter_swap(it, std::prev(it));
  m_taskList->reloadTasks();
  m_taskList->setCurrentRow(index - 1);
  emit profileChanged();
}

void CompilationProfileEditor::moveTaskDown()
{
  moveTaskDown(m_taskList->currentRow());
}

void CompilationProfileEditor::moveTaskDown(const int index)
{
  assert(index >= 0 && index < static_cast<int>(m_profile->tasks.size()) - 1);

  auto it = std::next(m_profile->tasks.begin(), index);
  std::iter_swap(it, std::next(it));
  m_taskList->reloadTasks();
  m_taskList->setCurrentRow(index + 1);
  emit profileChanged();
}

void CompilationProfileEditor::taskSelectionChanged()
{
  refresh();
}

void CompilationProfileEditor::setProfile(Model::CompilationProfile* profile)
{
  m_profile = profile;
  m_taskList->setProfile(profile);
  m_stackedWidget->setCurrentIndex(m_profile ? 1 : 0);
  refresh();
}

void CompilationProfileEditor::refresh()
{
  if (m_profile)
  {
    if (m_nameTxt->text().toStdString() != m_profile->name)
    {
      m_nameTxt->setText(QString::fromStdString(m_profile->name));
    }
    if (m_workDirTxt->text().toStdString() != m_profile->workDirSpec)
    {
      m_workDirTxt->setText(QString::fromStdString(m_profile->workDirSpec));
    }
  }
  m_addTaskButton->setEnabled(m_profile);
  m_removeTaskButton->setEnabled(m_profile && m_taskList->currentRow() >= 0);
  m_moveTaskUpButton->setEnabled(m_profile && m_taskList->currentRow() > 0);
  m_moveTaskDownButton->setEnabled(
    m_profile && m_taskList->currentRow() >= 0
    && m_taskList->currentRow() < static_cast<int>(m_profile->tasks.size()) - 1);
}
} // namespace View
} // namespace TrenchBroom
