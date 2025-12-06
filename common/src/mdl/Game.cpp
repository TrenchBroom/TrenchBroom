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

#include "Game.h"

#include "Logger.h"
#include "PreferenceManager.h"
#include "io/LoadEntityModel.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/MaterialManager.h"

#include "kd/const_overload.h"

#include <vector>

namespace tb::mdl
{

Game::Game(const GameInfo& gameInfo, Logger& logger)
  : m_gameInfo{gameInfo}
{
  initializeFileSystem({}, logger);
}

const GameInfo& Game::info() const
{
  return m_gameInfo;
}

const GameConfig& Game::config() const
{
  return m_gameInfo.gameConfig;
}

const GameFileSystem& Game::gameFileSystem() const
{
  return m_fs;
}

GameFileSystem& Game::gameFileSystem()
{
  return KDL_CONST_OVERLOAD(gameFileSystem());
}

void Game::updateFileSystem(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  initializeFileSystem(searchPaths, logger);
}

void Game::initializeFileSystem(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  const auto gamePath = pref(info().gamePathPreference);
  m_fs.initialize(config(), gamePath, searchPaths, logger);
}

} // namespace tb::mdl
