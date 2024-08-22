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

#include "MaterialCollectionEditor.h"

#include <QListWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include "IO/PathQt.h"
#include "Model/Node.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"

#include "kdl/memory_utils.h"
#include "kdl/vector_utils.h"

#include <cassert>

namespace TrenchBroom::View
{

MaterialCollectionEditor::MaterialCollectionEditor(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui();
  connectObservers();
  updateAllMaterialCollections();
  updateButtons();
}

void MaterialCollectionEditor::addSelectedMaterialCollections()
{
  const auto availableCollections = availableMaterialCollections();
  auto enabledCollections = enabledMaterialCollections();

  for (auto* selectedItem : m_availableCollectionsList->selectedItems())
  {
    const auto index = size_t(m_availableCollectionsList->row(selectedItem));
    enabledCollections.push_back(availableCollections[index]);
  }

  enabledCollections = kdl::vec_sort_and_remove_duplicates(std::move(enabledCollections));

  auto document = kdl::mem_lock(m_document);
  document->setEnabledMaterialCollections(enabledCollections);
}

void MaterialCollectionEditor::removeSelectedMaterialCollections()
{
  auto enabledCollections = enabledMaterialCollections();

  auto selections = std::vector<int>{};
  for (auto* selectedItem : m_enabledCollectionsList->selectedItems())
  {
    selections.push_back(m_enabledCollectionsList->row(selectedItem));
  }

  // erase back to front
  for (auto sIt = std::rbegin(selections), sEnd = std::rend(selections); sIt != sEnd;
       ++sIt)
  {
    const auto index = size_t(*sIt);
    enabledCollections = kdl::vec_erase_at(std::move(enabledCollections), index);
  }

  auto document = kdl::mem_lock(m_document);
  document->setEnabledMaterialCollections(enabledCollections);
}

void MaterialCollectionEditor::reloadMaterialCollections()
{
  auto document = kdl::mem_lock(m_document);
  document->reloadMaterialCollections();
}

void MaterialCollectionEditor::availableMaterialCollectionSelectionChanged()
{
  updateButtons();
}

void MaterialCollectionEditor::enabledMaterialCollectionSelectionChanged()
{
  updateButtons();
}

bool MaterialCollectionEditor::canAddMaterialCollections() const
{
  return !m_availableCollectionsList->selectedItems().empty();
}

bool MaterialCollectionEditor::canRemoveMaterialCollections() const
{
  return !m_enabledCollectionsList->selectedItems().empty();
}

bool MaterialCollectionEditor::canReloadMaterialCollections() const
{
  return m_enabledCollectionsList->count() != 0;
}

/**
 * See ModEditor::createGui
 */
void MaterialCollectionEditor::createGui()
{
  auto* availableCollectionsContainer = new TitledPanel{"Available", false, true};
  availableCollectionsContainer->setBackgroundRole(QPalette::Base);
  availableCollectionsContainer->setAutoFillBackground(true);

  m_availableCollectionsList = new QListWidget{};
  m_availableCollectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto* availableCollectionsContainerLayout = new QVBoxLayout{};
  availableCollectionsContainerLayout->setContentsMargins(0, 0, 0, 0);
  availableCollectionsContainerLayout->setSpacing(0);
  availableCollectionsContainerLayout->addWidget(m_availableCollectionsList);
  availableCollectionsContainer->getPanel()->setLayout(
    availableCollectionsContainerLayout);

  auto* enabledCollectionsContainer = new TitledPanel{"Enabled", false, true};
  enabledCollectionsContainer->setBackgroundRole(QPalette::Base);
  enabledCollectionsContainer->setAutoFillBackground(true);

  m_enabledCollectionsList = new QListWidget{};
  m_enabledCollectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto* enabledCollectionsContainerLayout = new QVBoxLayout{};
  enabledCollectionsContainerLayout->setContentsMargins(0, 0, 0, 0);
  enabledCollectionsContainerLayout->setSpacing(0);
  enabledCollectionsContainerLayout->addWidget(m_enabledCollectionsList);
  enabledCollectionsContainer->getPanel()->setLayout(enabledCollectionsContainerLayout);

  m_addCollectionsButton =
    createBitmapButton("Add.svg", tr("Enable the selected material collections"), this);
  m_removeCollectionsButton = createBitmapButton(
    "Remove.svg", tr("Disable the selected material collections"), this);
  m_reloadCollectionsButton = createBitmapButton(
    "Refresh.svg", tr("Reload all enabled material collections"), this);

  auto* toolBar = createMiniToolBarLayout(
    m_addCollectionsButton,
    m_removeCollectionsButton,
    LayoutConstants::WideHMargin,
    m_reloadCollectionsButton);

  auto* layout = new QGridLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(availableCollectionsContainer, 0, 0);
  layout->addWidget(new BorderLine{BorderLine::Direction::Vertical}, 0, 1, 3, 1);
  layout->addWidget(enabledCollectionsContainer, 0, 2);
  layout->addWidget(new BorderLine{BorderLine::Direction::Horizontal}, 1, 0, 1, 3);
  layout->addLayout(toolBar, 2, 2);

  setLayout(layout);

  updateAllMaterialCollections();

  connect(
    m_availableCollectionsList,
    &QListWidget::itemSelectionChanged,
    this,
    &MaterialCollectionEditor::availableMaterialCollectionSelectionChanged);
  connect(
    m_enabledCollectionsList,
    &QListWidget::itemSelectionChanged,
    this,
    &MaterialCollectionEditor::enabledMaterialCollectionSelectionChanged);
  connect(
    m_availableCollectionsList,
    &QListWidget::itemDoubleClicked,
    this,
    &MaterialCollectionEditor::addSelectedMaterialCollections);
  connect(
    m_enabledCollectionsList,
    &QListWidget::itemDoubleClicked,
    this,
    &MaterialCollectionEditor::removeSelectedMaterialCollections);
  connect(
    m_addCollectionsButton,
    &QAbstractButton::clicked,
    this,
    &MaterialCollectionEditor::addSelectedMaterialCollections);
  connect(
    m_removeCollectionsButton,
    &QAbstractButton::clicked,
    this,
    &MaterialCollectionEditor::removeSelectedMaterialCollections);
  connect(
    m_reloadCollectionsButton,
    &QAbstractButton::clicked,
    this,
    &MaterialCollectionEditor::reloadMaterialCollections);
}

void MaterialCollectionEditor::updateButtons()
{
  m_addCollectionsButton->setEnabled(canAddMaterialCollections());
  m_removeCollectionsButton->setEnabled(canRemoveMaterialCollections());
  m_reloadCollectionsButton->setEnabled(canReloadMaterialCollections());
}

void MaterialCollectionEditor::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &MaterialCollectionEditor::documentWasNewedOrLoaded);
  m_notifierConnection += document->nodesDidChangeNotifier.connect(
    this, &MaterialCollectionEditor::nodesDidChange);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &MaterialCollectionEditor::documentWasNewedOrLoaded);
  m_notifierConnection += document->materialCollectionsDidChangeNotifier.connect(
    this, &MaterialCollectionEditor::materialCollectionsDidChange);
  m_notifierConnection += document->modsDidChangeNotifier.connect(
    this, &MaterialCollectionEditor::modsDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection += prefs.preferenceDidChangeNotifier.connect(
    this, &MaterialCollectionEditor::preferenceDidChange);
}

void MaterialCollectionEditor::documentWasNewedOrLoaded(MapDocument*)
{
  updateAllMaterialCollections();
  updateButtons();
}

void MaterialCollectionEditor::nodesDidChange(const std::vector<Model::Node*>& nodes)
{
  auto document = kdl::mem_lock(m_document);
  if (kdl::vec_contains(nodes, document->world()))
  {
    updateAllMaterialCollections();
    updateButtons();
  }
}

void MaterialCollectionEditor::materialCollectionsDidChange()
{
  updateAllMaterialCollections();
  updateButtons();
}

void MaterialCollectionEditor::modsDidChange()
{
  updateAllMaterialCollections();
  updateButtons();
}

void MaterialCollectionEditor::preferenceDidChange(const std::filesystem::path& path)
{
  auto document = kdl::mem_lock(m_document);
  if (document->isGamePathPreference(path))
  {
    updateAllMaterialCollections();
    updateButtons();
  }
}

void MaterialCollectionEditor::updateAllMaterialCollections()
{
  updateAvailableMaterialCollections();
  updateEnabledMaterialCollections();
}

namespace
{

void updateListBox(QListWidget* box, const std::vector<std::filesystem::path>& paths)
{
  // We need to block QListWidget::itemSelectionChanged from firing while clearing and
  // rebuilding the list because it will cause debugUIConsistency() to fail, as the
  // number of list items in the UI won't match the document's material collections
  // lists.
  auto blocker = QSignalBlocker{box};

  box->clear();
  for (const auto& path : paths)
  {
    box->addItem(IO::pathAsQString(path));
  }
}

} // namespace

void MaterialCollectionEditor::updateAvailableMaterialCollections()
{
  updateListBox(m_availableCollectionsList, availableMaterialCollections());
}

void MaterialCollectionEditor::updateEnabledMaterialCollections()
{
  updateListBox(m_enabledCollectionsList, enabledMaterialCollections());
}

std::vector<std::filesystem::path> MaterialCollectionEditor::
  availableMaterialCollections() const
{
  auto document = kdl::mem_lock(m_document);
  return document->disabledMaterialCollections();
}

std::vector<std::filesystem::path> MaterialCollectionEditor::enabledMaterialCollections()
  const
{
  auto document = kdl::mem_lock(m_document);
  return document->enabledMaterialCollections();
}

} // namespace TrenchBroom::View
