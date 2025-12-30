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

#include "GameListBox.h"

#include "PreferenceManager.h"
#include "TrenchBroomApp.h"
#include "mdl/GameConfig.h"
#include "mdl/GameManager.h"
#include "ui/AppController.h"
#include "ui/ImageUtils.h"

#include "kd/contracts.h"
#include "kd/range_utils.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tb::ui
{
namespace
{
auto makeGameDisplayInfo(const mdl::GameInfo& gameInfo)
{
  auto gameName = gameInfo.gameConfig.name;

  auto iconPath = gameInfo.gameConfig.findConfigFile(gameInfo.gameConfig.icon);
  if (iconPath.empty())
  {
    iconPath = std::filesystem::path{"DefaultGameIcon.svg"};
  }

  auto title = QString::fromStdString(
    gameInfo.gameConfig.name
    + (gameInfo.gameConfig.experimental ? " (experimental)" : ""));

  const auto gamePath = pref(gameInfo.gamePathPreference);
  auto subTitle = QString::fromStdString(
    gamePath.empty() ? std::string("Game not found") : gamePath.string());

  return GameDisplayInfo{
    std::move(gameName),
    loadPixmap(iconPath),
    std::move(title),
    std::move(subTitle),
  };
}
} // namespace

GameListBox::GameListBox(QWidget* parent)
  : ImageListBox{"No Games Found", true, parent}
{
  reloadGameInfos();
}

std::string GameListBox::selectedGameName() const
{
  auto& app = TrenchBroomApp::instance();
  const auto& gameManager = app.appController().gameManager();
  const auto& gameInfos = gameManager.gameInfos();

  const auto index = currentRow();
  return index >= 0 && index < static_cast<int>(gameInfos.size())
           ? gameInfos[static_cast<size_t>(index)].gameConfig.name
           : "";
}

void GameListBox::selectGame(const size_t index)
{
  setCurrentRow(static_cast<int>(index));
}

void GameListBox::reloadGameInfos()
{
  auto& app = TrenchBroomApp::instance();
  const auto& gameManager = app.appController().gameManager();

  const auto currentGameName = selectedGameName();
  m_displayInfos.clear();

  std::ranges::transform(
    gameManager.gameInfos(), std::back_inserter(m_displayInfos), makeGameDisplayInfo);

  reload();

  if (
    const auto selectedGameIndex = kdl::index_of(
      gameManager.gameInfos(),
      [&](const auto& gameInfo) { return gameInfo.gameConfig.name == currentGameName; }))
  {
    selectGame(*selectedGameIndex);
  }
}

void GameListBox::updateGameInfos()
{
  auto& app = TrenchBroomApp::instance();
  const auto& gameManager = app.appController().gameManager();

  for (auto& displayInfo : m_displayInfos)
  {
    if (const auto* gameInfo = gameManager.gameInfo(displayInfo.name))
    {
      displayInfo = makeGameDisplayInfo(*gameInfo);
    }
  }
  updateItems();
}

size_t GameListBox::itemCount() const
{
  return m_displayInfos.size();
}

QPixmap GameListBox::image(const size_t index) const
{
  contract_pre(index < m_displayInfos.size());

  return m_displayInfos[index].image;
}

QString GameListBox::title(const size_t n) const
{
  contract_pre(n < m_displayInfos.size());

  return m_displayInfos[n].title;
}

QString GameListBox::subtitle(const size_t n) const
{
  contract_pre(n < m_displayInfos.size());

  return m_displayInfos[n].subtitle;
}

void GameListBox::selectedRowChanged(const int index)
{
  if (index >= 0 && index < count())
  {
    emit currentGameChanged(
      QString::fromStdString(m_displayInfos[static_cast<size_t>(index)].name));
  }
}

void GameListBox::doubleClicked(const size_t index)
{
  if (index < static_cast<size_t>(count()))
  {
    emit selectCurrentGame(QString::fromStdString(m_displayInfos[index].name));
  }
}

} // namespace tb::ui
