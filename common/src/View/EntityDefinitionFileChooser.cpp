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

#include "EntityDefinitionFileChooser.h"

#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "Assets/EntityDefinitionFileSpec.h"
#include "IO/PathQt.h"
#include "Model/Game.h"
#include "View/BorderLine.h"
#include "View/MapDocument.h"
#include "View/QtUtils.h"
#include "View/TitledPanel.h"
#include "View/ViewConstants.h"
#include "View/ViewUtils.h"

#include "kdl/memory_utils.h"
#include "kdl/vector_utils.h"

namespace TrenchBroom::View
{
// SingleSelectionListWidget

SingleSelectionListWidget::SingleSelectionListWidget(QWidget* parent)
  : QListWidget{parent}
  , m_allowDeselectAll{true}
{
}

void SingleSelectionListWidget::selectionChanged(
  const QItemSelection& selected, const QItemSelection& deselected)
{
  QListWidget::selectionChanged(selected, deselected);

  if (!m_allowDeselectAll)
  {
    if (selectedIndexes().isEmpty() && !deselected.isEmpty())
    {
      // reselect the items that were just deselected
      selectionModel()->select(deselected, QItemSelectionModel::Select);
    }
  }
}

void SingleSelectionListWidget::setAllowDeselectAll(bool allow)
{
  m_allowDeselectAll = allow;
}

bool SingleSelectionListWidget::allowDeselectAll() const
{
  return m_allowDeselectAll;
}

// EntityDefinitionFileChooser

EntityDefinitionFileChooser::EntityDefinitionFileChooser(
  std::weak_ptr<MapDocument> document, QWidget* parent)
  : QWidget{parent}
  , m_document{std::move(document)}
{
  createGui();
  bindEvents();
  connectObservers();
}

void EntityDefinitionFileChooser::createGui()
{
  auto* builtinPanel = new TitledPanel{tr("Builtin"), false, true};
  builtinPanel->setBackgroundRole(QPalette::Base);
  builtinPanel->setAutoFillBackground(true);

  m_builtin = new SingleSelectionListWidget{};
  m_builtin->setAllowDeselectAll(false);

  auto* builtinLayout = new QVBoxLayout{};
  builtinLayout->setContentsMargins(0, 0, 0, 0);
  builtinLayout->addWidget(m_builtin, 1);

  builtinPanel->getPanel()->setLayout(builtinLayout);

  auto* externalPanel = new TitledPanel{tr("External"), false, true};
  externalPanel->setBackgroundRole(QPalette::Base);
  externalPanel->setAutoFillBackground(true);

  m_externalLabel = new QLabel{tr("use builtin")};
  m_browseExternal = new QPushButton{tr("Browse...")};
  m_browseExternal->setToolTip(tr("Click to browse for an entity definition file"));
  m_reloadExternal = new QPushButton{tr("Reload")};
  m_reloadExternal->setToolTip(tr("Reload the currently loaded entity definition file"));

  auto* externalLayout = new QHBoxLayout{};
  externalLayout->addWidget(m_externalLabel, 1);
  externalLayout->addWidget(m_browseExternal, 0);
  externalLayout->addWidget(m_reloadExternal, 0);

  externalPanel->getPanel()->setLayout(externalLayout);

  auto* layout = new QVBoxLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(builtinPanel, 1);
  layout->addWidget(new BorderLine{}, 0);
  layout->addWidget(externalPanel, 0);
  m_builtin->setMinimumSize(100, 70);

  setLayout(layout);
}

void EntityDefinitionFileChooser::bindEvents()
{
  connect(
    m_builtin,
    &QListWidget::itemSelectionChanged,
    this,
    &EntityDefinitionFileChooser::builtinSelectionChanged);
  connect(
    m_browseExternal,
    &QAbstractButton::clicked,
    this,
    &EntityDefinitionFileChooser::chooseExternalClicked);
  connect(
    m_reloadExternal,
    &QAbstractButton::clicked,
    this,
    &EntityDefinitionFileChooser::reloadExternalClicked);
}

void EntityDefinitionFileChooser::connectObservers()
{
  auto document = kdl::mem_lock(m_document);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &EntityDefinitionFileChooser::documentWasNewed);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &EntityDefinitionFileChooser::documentWasLoaded);
  m_notifierConnection += document->entityDefinitionsDidChangeNotifier.connect(
    this, &EntityDefinitionFileChooser::entityDefinitionsDidChange);
}

void EntityDefinitionFileChooser::documentWasNewed(MapDocument*)
{
  updateControls();
}

void EntityDefinitionFileChooser::documentWasLoaded(MapDocument*)
{
  updateControls();
}

void EntityDefinitionFileChooser::entityDefinitionsDidChange()
{
  updateControls();
}

void EntityDefinitionFileChooser::updateControls()
{
  m_builtin->setAllowDeselectAll(true);
  m_builtin->clear();
  m_builtin->setAllowDeselectAll(false);

  auto document = kdl::mem_lock(m_document);
  auto specs = document->allEntityDefinitionFiles();
  specs = kdl::vec_sort(std::move(specs));

  for (const auto& spec : specs)
  {
    const auto& path = spec.path();

    auto* item = new QListWidgetItem();
    item->setData(Qt::DisplayRole, IO::pathAsQString(path.filename()));
    item->setData(Qt::UserRole, QVariant::fromValue(spec));

    m_builtin->addItem(item);
  }

  const auto spec = document->entityDefinitionFile();
  if (spec.builtin())
  {
    if (const auto index = kdl::vec_index_of(specs, spec))
    {
      // the chosen builtin entity definition file might not be in the game config anymore
      // if the config has changed after the definition file was chosen
      m_builtin->setCurrentRow(static_cast<int>(*index));
    }
    m_externalLabel->setText(tr("use builtin"));

    auto lightText = QPalette{};
    lightText.setColor(QPalette::WindowText, Colors::disabledText());
    m_externalLabel->setPalette(lightText);

    auto font = m_externalLabel->font();
    font.setStyle(QFont::StyleOblique);
    m_externalLabel->setFont(font);
  }
  else
  {
    m_builtin->clearSelection();
    m_externalLabel->setText(IO::pathAsQString(spec.path()));

    auto normalPal = QPalette{};
    m_externalLabel->setPalette(normalPal);

    auto font = m_externalLabel->font();
    font.setStyle(QFont::StyleNormal);
    m_externalLabel->setFont(font);
  }

  m_reloadExternal->setEnabled(document->entityDefinitionFile().external());
}

void EntityDefinitionFileChooser::builtinSelectionChanged()
{
  if (!m_builtin->selectedItems().isEmpty())
  {

    auto* item = m_builtin->selectedItems().first();
    auto spec = item->data(Qt::UserRole).value<Assets::EntityDefinitionFileSpec>();

    auto document = kdl::mem_lock(m_document);
    if (document->entityDefinitionFile() != spec)
    {
      document->setEntityDefinitionFile(spec);
    }
  }
}

void EntityDefinitionFileChooser::chooseExternalClicked()
{
  const auto fileName = QFileDialog::getOpenFileName(
    nullptr,
    tr("Load Entity Definition File"),
    fileDialogDefaultDirectory(FileDialogDir::EntityDefinition),
    "All supported entity definition files (*.fgd *.def *.ent);;"
    "Worldcraft / Hammer files (*.fgd);;"
    "QuakeC files (*.def);;"
    "Radiant XML files (*.ent)");

  if (!fileName.isEmpty())
  {
    updateFileDialogDefaultDirectoryWithFilename(
      FileDialogDir::EntityDefinition, fileName);
    loadEntityDefinitionFile(m_document, this, fileName);
  }
}

void EntityDefinitionFileChooser::reloadExternalClicked()
{
  auto document = kdl::mem_lock(m_document);
  document->reloadEntityDefinitions();
}

} // namespace TrenchBroom::View
