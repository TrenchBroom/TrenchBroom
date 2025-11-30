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
#include "fs/DiskFileSystem.h"
#include "fs/DiskIO.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"
#include "io/FgdParser.h"
#include "io/GameConfigParser.h"
#include "io/LoadEntityModel.h"
#include "io/SystemPaths.h"
#include "io/WorldReader.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/EntityProperties.h"
#include "mdl/GameConfig.h"
#include "mdl/GameFactory.h"
#include "mdl/MaterialManager.h"

#include "kd/path_utils.h"
#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/string_compare.h"

#include <fmt/format.h>
#include <fmt/std.h>

#include <algorithm>
#include <ranges>
#include <string>
#include <vector>

namespace tb::mdl
{
Game::Game(GameConfig config, std::filesystem::path gamePath, Logger& logger)
  : m_config{std::move(config)}
  , m_gamePath{std::move(gamePath)}
{
  initializeFileSystem(logger);
}

bool Game::isGamePathPreference(const std::filesystem::path& prefPath) const
{
  const auto& gameFactory = GameFactory::instance();
  return gameFactory.isGamePathPreference(config().name, prefPath);
}

const GameConfig& Game::config() const
{
  return m_config;
}

const fs::FileSystem& Game::gameFileSystem() const
{
  return m_fs;
}

std::filesystem::path Game::gamePath() const
{
  return m_gamePath;
}

void Game::setGamePath(const std::filesystem::path& gamePath, Logger& logger)
{
  if (gamePath != m_gamePath)
  {
    m_gamePath = gamePath;
    initializeFileSystem(logger);
  }
}

void Game::setAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  if (searchPaths != m_additionalSearchPaths)
  {
    m_additionalSearchPaths = searchPaths;
    initializeFileSystem(logger);
  }
}

SoftMapBounds Game::extractSoftMapBounds(const Entity& entity) const
{
  if (const auto* mapValue = entity.property(EntityPropertyKeys::SoftMapBounds))
  {
    return *mapValue == EntityPropertyValues::NoSoftMapBounds
             ? SoftMapBounds{SoftMapBoundsType::Map, std::nullopt}
             : SoftMapBounds{
                 SoftMapBoundsType::Map, io::parseSoftMapBoundsString(*mapValue)};
  }

  // Not set in map -> use Game value
  return SoftMapBounds{SoftMapBoundsType::Game, config().softMapBounds};
}

void Game::reloadWads(
  const std::filesystem::path& documentPath,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger& logger)
{
  const auto searchPaths = std::vector<std::filesystem::path>{
    documentPath.parent_path(), // Search for assets relative to the map file.
    m_gamePath,                 // Search for assets relative to the location of the game.
    io::SystemPaths::appDirectory(), // Search for assets relative to the application.
  };
  m_fs.reloadWads(m_config.materialConfig.root, searchPaths, wadPaths, logger);
}

bool Game::isEntityDefinitionFile(const std::filesystem::path& path) const
{
  static const auto extensions = {".fgd", ".def", ".ent"};

  return std::ranges::any_of(extensions, [&](const auto& extension) {
    return kdl::path_has_extension(kdl::path_to_lower(path), extension);
  });
}

std::vector<EntityDefinitionFileSpec> Game::allEntityDefinitionFiles() const
{
  return m_config.entityConfig.defFilePaths | std::views::transform([](const auto& path) {
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
    return m_config.findConfigFile(spec.path);
  }

  if (spec.path.is_absolute())
  {
    return spec.path;
  }

  return fs::Disk::resolvePath(searchPaths, spec.path);
}

Result<std::vector<std::string>> Game::availableMods() const
{
  if (m_gamePath.empty() || fs::Disk::pathInfo(m_gamePath) != fs::PathInfo::Directory)
  {
    return Result<std::vector<std::string>>{std::vector<std::string>{}};
  }

  const auto& defaultMod = m_config.fileSystemConfig.searchPath.filename().string();
  const auto fs = fs::DiskFileSystem{m_gamePath};
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
  return m_config.fileSystemConfig.searchPath.string();
}

void Game::initializeFileSystem(Logger& logger)
{
  m_fs.initialize(m_config, m_gamePath, m_additionalSearchPaths, logger);
}

EntityPropertyConfig Game::entityPropertyConfig() const
{
  return {
    m_config.entityConfig.scaleExpression, m_config.entityConfig.setDefaultProperties};
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
