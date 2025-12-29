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

#include "ModEditor.h"

#include <QLineEdit>
#include <QListWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "Logger.h"
#include "Notifier.h"
#include "PreferenceManager.h"
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"
#include "mdl/GameInfo.h"
#include "mdl/Map.h"
#include "mdl/Map_World.h"
#include "ui/BitmapButton.h"
#include "ui/BorderLine.h"
#include "ui/MapDocument.h"
#include "ui/MiniToolBarLayout.h"
#include "ui/SearchBox.h"
#include "ui/TitledPanel.h"
#include "ui/ViewConstants.h"

#include "kd/collection_utils.h"
#include "kd/contracts.h"
#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/string_compare.h"
#include "kd/vector_utils.h"

#include <ranges>

namespace tb::ui
{
namespace
{

Result<std::vector<std::string>> findAvailableMods(const mdl::GameInfo& gameInfo)
{
  const auto gamePath = pref(gameInfo.gamePathPreference);
  if (gamePath.empty() || fs::Disk::pathInfo(gamePath) != fs::PathInfo::Directory)
  {
    return Result<std::vector<std::string>>{std::vector<std::string>{}};
  }

  const auto defaultMod =
    gameInfo.gameConfig.fileSystemConfig.searchPath.filename().string();
  const auto fs = fs::DiskFileSystem{gamePath};
  return fs.find(
           "",
           fs::TraversalMode::Flat,
           fs::makePathInfoPathMatcher({fs::PathInfo::Directory}))
         | kdl::transform([&](const auto& subDirs) {
             return subDirs | std::views::transform([](const auto& subDir) {
                      return subDir.filename().string();
                    })
                    | std::views::filter([&](const auto& mod) {
                        return !kdl::ci::str_is_equal(mod, defaultMod);
                      })
                    | kdl::views::as_rvalue | kdl::ranges::to<std::vector>();
           });
}

} // namespace

ModEditor::ModEditor(MapDocument& document, QWidget* parent)
  : QWidget{parent}
  , m_document{document}
{
  createGui();
  connectObservers();
}

void ModEditor::createGui()
{
  auto* availableModContainer = new TitledPanel{"Available", false, true};
  availableModContainer->setBackgroundRole(QPalette::Base);
  availableModContainer->setAutoFillBackground(true);

  m_availableModList = new QListWidget{};
  m_availableModList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto* availableModContainerSizer = new QVBoxLayout{};
  availableModContainerSizer->setContentsMargins(0, 0, 0, 0);
  availableModContainerSizer->setSpacing(0);
  availableModContainerSizer->addWidget(m_availableModList, 1);
  availableModContainer->getPanel()->setLayout(availableModContainerSizer);

  m_filterBox = createSearchBox();
  m_filterBox->setToolTip(tr("Filter the list of available mods"));

  auto* filterBoxSizer = new QVBoxLayout{};
  filterBoxSizer->setContentsMargins(0, 0, 0, 0);
  filterBoxSizer->setSpacing(0);
  filterBoxSizer->addWidget(m_filterBox, 1);

  auto* enabledModContainer = new TitledPanel{"Enabled", false, true};
  enabledModContainer->setBackgroundRole(QPalette::Base);
  enabledModContainer->setAutoFillBackground(true);

  m_enabledModList = new QListWidget{};
  m_enabledModList->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto* enabledModContainerSizer = new QVBoxLayout{};
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

  auto* layout = new QGridLayout{};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(availableModContainer, 0, 0);
  layout->addWidget(new BorderLine{BorderLine::Direction::Vertical}, 0, 1, 3, 1);
  layout->addWidget(enabledModContainer, 0, 2);
  layout->addWidget(new BorderLine{}, 1, 0, 1, 3);
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
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect(this, &ModEditor::documentWasLoaded);
  m_notifierConnection +=
    m_document.modsDidChangeNotifier.connect(this, &ModEditor::modsDidChange);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &ModEditor::preferenceDidChange);
}

void ModEditor::documentWasLoaded()
{
  updateAvailableMods();
  updateMods();
}

void ModEditor::modsDidChange()
{
  updateMods();
}

void ModEditor::preferenceDidChange(const std::filesystem::path& path)
{
  if (path == pref(m_document.map().gameInfo().gamePathPreference))
  {
    updateAvailableMods();
    updateMods();
  }
}

void ModEditor::updateAvailableMods()
{
  auto& map = m_document.map();
  findAvailableMods(map.gameInfo()) | kdl::transform([&](auto availableMods) {
    m_availableMods = kdl::col_sort(std::move(availableMods), kdl::ci::string_less{});
  }) | kdl::transform_error([&](auto e) {
    m_availableMods.clear();
    map.logger().error() << "Could not update available mods: " << e.msg;
  });
}

void ModEditor::updateMods()
{
  const auto pattern = m_filterBox->text().toStdString();

  const auto enabledMods = mdl::enabledMods(m_document.map());

  m_availableModList->clear();
  m_availableModList->addItems(
    m_availableMods | std::views::filter([&](const auto& mod) {
      return kdl::ci::str_contains(mod, pattern) && !kdl::vec_contains(enabledMods, mod);
    })
    | std::views::transform(QString::fromStdString) | kdl::ranges::to<QStringList>());

  m_enabledModList->clear();
  m_enabledModList->addItems(
    enabledMods | std::views::filter([&](const auto& mod) {
      return kdl::ci::str_contains(mod, pattern);
    })
    | std::views::transform(QString::fromStdString) | kdl::ranges::to<QStringList>());
}

void ModEditor::addModClicked()
{
  if (const auto selections = m_availableModList->selectedItems(); !selections.empty())
  {
    auto& map = m_document.map();

    auto enabledMods = mdl::enabledMods(map);
    for (const auto* item : selections)
    {
      enabledMods.push_back(item->text().toStdString());
    }
    setEnabledMods(map, enabledMods);
  }
}

void ModEditor::removeModClicked()
{
  if (const auto selections = m_enabledModList->selectedItems(); !selections.empty())
  {
    auto& map = m_document.map();

    auto enabledMods = mdl::enabledMods(map);
    for (const auto* item : selections)
    {
      const auto mod = item->text().toStdString();
      enabledMods = kdl::vec_erase(std::move(enabledMods), mod);
    }
    setEnabledMods(map, enabledMods);
  }
}

void ModEditor::moveModUpClicked()
{
  const auto selections = m_enabledModList->selectedItems();
  contract_assert(selections.size() == 1);

  auto& map = m_document.map();
  auto enabledMods = mdl::enabledMods(map);

  const auto index = size_t(m_enabledModList->row(selections.first()));
  contract_assert(index < enabledMods.size());

  using std::swap;
  swap(enabledMods[index - 1], enabledMods[index]);
  setEnabledMods(map, enabledMods);

  m_enabledModList->clearSelection();
  m_enabledModList->setCurrentRow(int(index - 1));
}

void ModEditor::moveModDownClicked()
{
  const auto selections = m_enabledModList->selectedItems();
  contract_assert(selections.size() == 1);

  auto& map = m_document.map();
  auto enabledMods = mdl::enabledMods(map);

  const auto index = size_t(m_enabledModList->row(selections.first()));
  contract_assert(index < enabledMods.size() - 1);

  using std::swap;
  swap(enabledMods[index + 1], enabledMods[index]);
  setEnabledMods(map, enabledMods);

  m_enabledModList->clearSelection();
  m_enabledModList->setCurrentRow(int(index + 1));
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

} // namespace tb::ui
