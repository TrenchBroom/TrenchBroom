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

#include "ModEditor.h"

#include "Model/EntityNode.h"
#include "Model/Game.h"
#include "Notifier.h"
#include "PreferenceManager.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include <kdl/collection_utils.h>
#include <kdl/memory_utils.h>
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <QAbstractButton>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <cassert>
#include <string>

namespace TrenchBroom
{
namespace View
{
ModEditor::ModEditor(std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget(parent)
  , m_document(document)
  , m_availableModList(nullptr)
  , m_enabledModList(nullptr)
  , m_filterBox(nullptr)
{
  createGui();
  connectObservers();
}

void ModEditor::createGui()
{
  auto* availableModContainer = new TitledPanel("Available", false, true);
  availableModContainer->setBackgroundRole(QPalette::Base);
  availableModContainer->setAutoFillBackground(true);

  m_availableModList = new QListWidget();
  m_availableModList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto* availableModContainerSizer = new QVBoxLayout();
  availableModContainerSizer->setContentsMargins(0, 0, 0, 0);
  availableModContainerSizer->setSpacing(0);
  availableModContainerSizer->addWidget(m_availableModList, 1);
  availableModContainer->getPanel()->setLayout(availableModContainerSizer);

  m_filterBox = createSearchBox();
  m_filterBox->setToolTip(tr("Filter the list of available mods"));

  auto* filterBoxSizer = new QVBoxLayout();
  filterBoxSizer->setContentsMargins(0, 0, 0, 0);
  filterBoxSizer->setSpacing(0);
  filterBoxSizer->addWidget(m_filterBox, 1);

  auto* enabledModContainer = new TitledPanel("Enabled", false, true);
  enabledModContainer->setBackgroundRole(QPalette::Base);
  enabledModContainer->setAutoFillBackground(true);

  m_enabledModList = new QListWidget();
  m_enabledModList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto* enabledModContainerSizer = new QVBoxLayout();
  enabledModContainerSizer->setContentsMargins(0, 0, 0, 0);
  enabledModContainerSizer->setSpacing(0);
  enabledModContainerSizer->addWidget(m_enabledModList, 1);
  enabledModContainer->getPanel()->setLayout(enabledModContainerSizer);

  m_addModsButton = createBitmapButton("Add.svg", tr("Enable the selected mods"));
  m_removeModsButton = createBitmapButton("Remove.svg", tr("Disable the selected mods"));
  m_moveModUpButton = createBitmapButton("Up.svg", tr("Move the selected mod up"));
  m_moveModDownButton = createBitmapButton("Down.svg", tr("Move the selected mod down"));

  auto* toolBar = createMiniToolBarLayout(
    m_addModsButton,
    m_removeModsButton,
    LayoutConstants::WideHMargin,
    m_moveModUpButton,
    m_moveModDownButton);

  auto* layout = new QGridLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(availableModContainer, 0, 0);
  layout->addWidget(new BorderLine(BorderLine::Direction::Vertical), 0, 1, 3, 1);
  layout->addWidget(enabledModContainer, 0, 2);
  layout->addWidget(new BorderLine(BorderLine::Direction::Horizontal), 1, 0, 1, 3);
  layout->addLayout(filterBoxSizer, 2, 0);
  layout->addLayout(toolBar, 2, 2);

  setLayout(layout);

  connect(
    m_availableModList, &QListWidget::itemDoubleClicked, this, &ModEditor::addModClicked);
  connect(
    m_enabledModList,
    &QListWidget::itemDoubleClicked,
    this,
    &ModEditor::removeModClicked);
  connect(m_filterBox, &QLineEdit::textEdited, this, &ModEditor::filterBoxChanged);
  connect(m_addModsButton, &QAbstractButton::clicked, this, &ModEditor::addModClicked);
  connect(
    m_removeModsButton, &QAbstractButton::clicked, this, &ModEditor::removeModClicked);
  connect(
    m_moveModUpButton, &QAbstractButton::clicked, this, &ModEditor::moveModUpClicked);
  connect(
    m_moveModDownButton, &QAbstractButton::clicked, this, &ModEditor::moveModDownClicked);

  connect(
    m_availableModList,
    &QListWidget::itemSelectionChanged,
    this,
    &ModEditor::updateButtons);
  connect(
    m_enabledModList,
    &QListWidget::itemSelectionChanged,
    this,
    &ModEditor::updateButtons);

  updateButtons();
}

void ModEditor::updateButtons()
{
  m_addModsButton->setEnabled(canEnableAddButton());
  m_removeModsButton->setEnabled(canEnableRemoveButton());
  m_moveModUpButton->setEnabled(canEnableMoveUpButton());
  m_moveModDownButton->setEnabled(canEnableMoveDownButton());
}

void ModEditor::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection +=
    document->documentWasNewedNotifier.connect(this, &ModEditor::documentWasNewed);
  m_notifierConnection +=
    document->documentWasLoadedNotifier.connect(this, &ModEditor::documentWasLoaded);
  m_notifierConnection +=
    document->modsDidChangeNotifier.connect(this, &ModEditor::modsDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &ModEditor::preferenceDidChange);
}

void ModEditor::documentWasNewed(MapDocument*)
{
  updateAvailableMods();
  updateMods();
}

void ModEditor::documentWasLoaded(MapDocument*)
{
  updateAvailableMods();
  updateMods();
}

void ModEditor::modsDidChange()
{
  updateMods();
}

void ModEditor::preferenceDidChange(const IO::Path& path)
{
  auto document = kdl::mem_lock(m_document);
  if (document->isGamePathPreference(path))
  {
    updateAvailableMods();
    updateMods();
  }
}

void ModEditor::updateAvailableMods()
{
  auto document = kdl::mem_lock(m_document);
  m_availableMods =
    kdl::col_sort(document->game()->availableMods(), kdl::ci::string_less());
}

void ModEditor::updateMods()
{
  m_availableModList->clear();
  m_enabledModList->clear();

  const auto pattern = m_filterBox->text().toStdString();

  auto document = kdl::mem_lock(m_document);
  auto enabledMods = document->mods();

  QStringList availableModItems;
  for (size_t i = 0; i < m_availableMods.size(); ++i)
  {
    const auto& mod = m_availableMods[i];
    if (kdl::ci::str_contains(mod, pattern) && !kdl::vec_contains(enabledMods, mod))
    {
      m_availableModList->addItem(QString::fromStdString(mod));
    }
  }

  QStringList enabledModItems;
  for (size_t i = 0; i < enabledMods.size(); ++i)
  {
    if (kdl::ci::str_contains(enabledMods[i], pattern))
    {
      m_enabledModList->addItem(QString::fromStdString(enabledMods[i]));
    }
  }
}

void ModEditor::addModClicked()
{
  const QList<QListWidgetItem*> selections = m_availableModList->selectedItems();
  if (selections.empty())
  {
    return;
  }

  auto document = kdl::mem_lock(m_document);

  std::vector<std::string> mods = document->mods();
  for (QListWidgetItem* item : selections)
  {
    mods.push_back(item->text().toStdString());
  }
  document->setMods(mods);
}

void ModEditor::removeModClicked()
{
  const QList<QListWidgetItem*> selections = m_enabledModList->selectedItems();
  if (selections.empty())
  {
    return;
  }

  auto document = kdl::mem_lock(m_document);

  std::vector<std::string> mods = document->mods();
  for (QListWidgetItem* item : selections)
  {
    const std::string mod = item->text().toStdString();
    mods = kdl::vec_erase(std::move(mods), mod);
  }
  document->setMods(mods);
}

void ModEditor::moveModUpClicked()
{
  const QList<QListWidgetItem*> selections = m_enabledModList->selectedItems();
  assert(selections.size() == 1);

  auto document = kdl::mem_lock(m_document);
  auto mods = document->mods();

  const size_t index = static_cast<size_t>(m_enabledModList->row(selections.first()));
  ensure(index < mods.size(), "index out of range");

  using std::swap;
  swap(mods[index - 1], mods[index]);
  document->setMods(mods);

  m_enabledModList->clearSelection();
  m_enabledModList->setCurrentRow(static_cast<int>(index - 1));
}

void ModEditor::moveModDownClicked()
{
  const QList<QListWidgetItem*> selections = m_enabledModList->selectedItems();
  assert(selections.size() == 1);

  auto document = kdl::mem_lock(m_document);
  auto mods = document->mods();

  const auto index = static_cast<size_t>(m_enabledModList->row(selections.first()));
  ensure(index < mods.size() - 1, "index out of range");

  using std::swap;
  swap(mods[index + 1], mods[index]);
  document->setMods(mods);

  m_enabledModList->clearSelection();
  m_enabledModList->setCurrentRow(static_cast<int>(index + 1));
}

bool ModEditor::canEnableAddButton() const
{
  return !m_availableModList->selectedItems().empty();
}

bool ModEditor::canEnableRemoveButton() const
{
  return !m_enabledModList->selectedItems().empty();
}

bool ModEditor::canEnableMoveUpButton() const
{
  return m_enabledModList->selectedItems().size() == 1
         && m_enabledModList->row(m_enabledModList->selectedItems().front()) > 0;
}

bool ModEditor::canEnableMoveDownButton() const
{
  const auto enabledModCount = m_enabledModList->count();

  return m_enabledModList->selectedItems().size() == 1
         && m_enabledModList->row(m_enabledModList->selectedItems().front())
              < enabledModCount - 1;
}

void ModEditor::filterBoxChanged()
{
  updateMods();
}
} // namespace View
} // namespace TrenchBroom
