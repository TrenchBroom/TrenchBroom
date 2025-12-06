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
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"
#include "io/LoadEntityModel.h"
#include "io/WorldReader.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/EntityProperties.h"
#include "mdl/GameConfig.h"
#include "mdl/GameInfo.h"
#include "mdl/MaterialManager.h"

#include "kd/const_overload.h"
#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/string_compare.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <ranges>
#include <string>
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

std::vector<EntityDefinitionFileSpec> Game::allEntityDefinitionFiles() const
{
  return config().entityConfig.defFilePaths | std::views::transform([](const auto& path) {
           return EntityDefinitionFileSpec::makeBuiltin(path);
         })
         | kdl::ranges::to<std::vector>();
}

std::filesystem::path Game::findEntityDefinitionFile(
  const EntityDefinitionFileSpec& spec,
  const std::vector<std::filesystem::path>& searchPaths) const
{
  if (spec.type == EntityDefinitionFileSpec::Type::Builtin)
  {
    return config().findConfigFile(spec.path);
  }

  if (spec.path.is_absolute())
  {
    return spec.path;
  }

  return fs::Disk::resolvePath(searchPaths, spec.path);
}

Result<std::vector<std::string>> Game::availableMods() const
{
  const auto gamePath = pref(info().gamePathPreference);
  if (gamePath.empty() || fs::Disk::pathInfo(gamePath) != fs::PathInfo::Directory)
  {
    return Result<std::vector<std::string>>{std::vector<std::string>{}};
  }

  const auto& defaultMod = config().fileSystemConfig.searchPath.filename().string();
  const auto fs = fs::DiskFileSystem{gamePath};
  return fs.find(
           "",
           fs::TraversalMode::Flat,
           fs::makePathInfoPathMatcher({fs::PathInfo::Directory}))
         | kdl::transform([](const auto& subDirs) {
             return subDirs | std::views::transform([](const auto& subDir) {
                      return subDir.filename().string();
                    });
           })
         | kdl::transform([&](auto mods) {
             return mods | std::views::filter([&](const auto& mod) {
                      return !kdl::ci::str_is_equal(mod, defaultMod);
                    })
                    | kdl::views::as_rvalue | kdl::ranges::to<std::vector>();
           });
}

std::string Game::defaultMod() const
{
  return config().fileSystemConfig.searchPath.string();
}

void Game::initializeFileSystem(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  const auto gamePath = pref(info().gamePathPreference);
  m_fs.initialize(config(), gamePath, searchPaths, logger);
}

EntityPropertyConfig Game::entityPropertyConfig() const
{
  return {
    config().entityConfig.scaleExpression, config().entityConfig.setDefaultProperties};
}

void Game::writeLongAttribute(
  EntityNodeBase& node,
  const std::string& baseName,
  const std::string& value,
  const size_t maxLength) const
{
  auto entity = node.entity();
  entity.removeNumberedProperty(baseName);

  auto nameStr = std::stringstream{};
  for (size_t i = 0; i <= value.size() / maxLength; ++i)
  {
    nameStr.str("");
    nameStr << baseName << i + 1;
    entity.addOrUpdateProperty(nameStr.str(), value.substr(i * maxLength, maxLength));
  }

  node.setEntity(std::move(entity));
}

std::string Game::readLongAttribute(
  const EntityNodeBase& node, const std::string& baseName) const
{
  size_t index = 1;
  auto nameStr = std::stringstream{};
  auto valueStr = std::stringstream{};
  nameStr << baseName << index;

  const auto& entity = node.entity();
  while (entity.hasProperty(nameStr.str()))
  {
    if (const auto* value = entity.property(nameStr.str()))
    {
      valueStr << *value;
    }
    nameStr.str("");
    nameStr << baseName << ++index;
  }

  return valueStr.str();
}

} // namespace tb::mdl
