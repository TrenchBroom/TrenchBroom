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

#include "GameImpl.h"

#include "Exceptions.h"
#include "Logger.h"
#include "io/BrushFaceReader.h"
#include "io/DefParser.h"
#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/EntParser.h"
#include "io/FgdParser.h"
#include "io/GameConfigParser.h"
#include "io/LoadEntityModel.h"
#include "io/NodeReader.h"
#include "io/PathInfo.h"
#include "io/SystemPaths.h"
#include "io/TraversalMode.h"
#include "io/WorldReader.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/EntityProperties.h"
#include "mdl/GameConfig.h"
#include "mdl/MaterialManager.h"

#include "kdl/result.h"
#include "kdl/string_compare.h"
#include "kdl/string_utils.h"
#include "kdl/vector_utils.h"

#include <string>
#include <vector>

namespace tb::mdl
{
GameImpl::GameImpl(GameConfig& config, std::filesystem::path gamePath, Logger& logger)
  : m_config{config}
  , m_gamePath{std::move(gamePath)}
{
  initializeFileSystem(logger);
}

Result<std::vector<std::unique_ptr<EntityDefinition>>> GameImpl::loadEntityDefinitions(
  io::ParserStatus& status, const std::filesystem::path& path) const
{
  const auto extension = path.extension().string();
  const auto& defaultColor = m_config.entityConfig.defaultColor;

  try
  {
    if (kdl::ci::str_is_equal(".fgd", extension))
    {
      return io::Disk::openFile(path) | kdl::transform([&](auto file) {
               auto reader = file->reader().buffer();
               auto parser = io::FgdParser{reader.stringView(), defaultColor, path};
               return parser.parseDefinitions(status);
             });
    }
    if (kdl::ci::str_is_equal(".def", extension))
    {
      return io::Disk::openFile(path) | kdl::transform([&](auto file) {
               auto reader = file->reader().buffer();
               auto parser = io::DefParser{reader.stringView(), defaultColor};
               return parser.parseDefinitions(status);
             });
    }
    if (kdl::ci::str_is_equal(".ent", extension))
    {
      return io::Disk::openFile(path) | kdl::transform([&](auto file) {
               auto reader = file->reader().buffer();
               auto parser = io::EntParser{reader.stringView(), defaultColor};
               return parser.parseDefinitions(status);
             });
    }

    return Error{"Unknown entity definition format: '" + path.string() + "'"};
  }
  catch (const ParserException& e)
  {
    return Error{e.what()};
  }
}

const GameConfig& GameImpl::config() const
{
  return m_config;
}

const io::FileSystem& GameImpl::gameFileSystem() const
{
  return m_fs;
}

std::filesystem::path GameImpl::gamePath() const
{
  return m_gamePath;
}

void GameImpl::setGamePath(const std::filesystem::path& gamePath, Logger& logger)
{
  if (gamePath != m_gamePath)
  {
    m_gamePath = gamePath;
    initializeFileSystem(logger);
  }
}

void GameImpl::setAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  if (searchPaths != m_additionalSearchPaths)
  {
    m_additionalSearchPaths = searchPaths;
    initializeFileSystem(logger);
  }
}

Game::PathErrors GameImpl::checkAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths) const
{
  auto result = PathErrors{};
  for (const auto& searchPath : searchPaths)
  {
    const auto absPath = m_gamePath / searchPath;
    if (!absPath.is_absolute() || io::Disk::pathInfo(absPath) != io::PathInfo::Directory)
    {
      result.emplace(searchPath, "Directory not found: '" + searchPath.string() + "'");
    }
  }
  return result;
}

Game::SoftMapBounds GameImpl::extractSoftMapBounds(const Entity& entity) const
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

void GameImpl::reloadWads(
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

bool GameImpl::isEntityDefinitionFile(const std::filesystem::path& path) const
{
  static const auto extensions = {".fgd", ".def", ".ent"};

  return std::any_of(extensions.begin(), extensions.end(), [&](const auto& extension) {
    return kdl::ci::str_is_equal(extension, path.extension().string());
  });
}

std::vector<EntityDefinitionFileSpec> GameImpl::allEntityDefinitionFiles() const
{
  return kdl::vec_transform(m_config.entityConfig.defFilePaths, [](const auto& path) {
    return EntityDefinitionFileSpec::builtin(path);
  });
}

EntityDefinitionFileSpec GameImpl::extractEntityDefinitionFile(const Entity& entity) const
{
  if (const auto* defValue = entity.property(EntityPropertyKeys::EntityDefinitions))
  {
    return EntityDefinitionFileSpec::parse(*defValue);
  }

  return defaultEntityDefinitionFile();
}

EntityDefinitionFileSpec GameImpl::defaultEntityDefinitionFile() const
{
  if (const auto paths = m_config.entityConfig.defFilePaths; !paths.empty())
  {
    return EntityDefinitionFileSpec::builtin(paths.front());
  }

  throw GameException{
    "No entity definition files found for game '" + config().name + "'"};
}

std::filesystem::path GameImpl::findEntityDefinitionFile(
  const EntityDefinitionFileSpec& spec,
  const std::vector<std::filesystem::path>& searchPaths) const
{
  if (!spec.valid())
  {
    throw GameException{"Invalid entity definition file spec"};
  }

  const auto& path = spec.path();
  if (spec.builtin())
  {
    return m_config.findConfigFile(path);
  }

  if (path.is_absolute())
  {
    return path;
  }

  return io::Disk::resolvePath(searchPaths, path);
}

Result<std::vector<std::string>> GameImpl::availableMods() const
{
  if (m_gamePath.empty() || io::Disk::pathInfo(m_gamePath) != io::PathInfo::Directory)
  {
    return Result<std::vector<std::string>>{std::vector<std::string>{}};
  }

  const auto& defaultMod = m_config.fileSystemConfig.searchPath.filename().string();
  const auto fs = io::DiskFileSystem{m_gamePath};
  return fs.find(
           "",
           io::TraversalMode::Flat,
           io::makePathInfoPathMatcher({io::PathInfo::Directory}))
         | kdl::transform([](auto subDirs) {
             return kdl::vec_transform(std::move(subDirs), [](auto subDir) {
               return subDir.filename().string();
             });
           })
         | kdl::transform([&](auto mods) {
             return kdl::vec_filter(std::move(mods), [&](const auto& mod) {
               return !kdl::ci::str_is_equal(mod, defaultMod);
             });
           });
}

std::vector<std::string> GameImpl::extractEnabledMods(const Entity& entity) const
{
  if (const auto* modStr = entity.property(EntityPropertyKeys::Mods))
  {
    return kdl::str_split(*modStr, ";");
  }
  return {};
}

std::string GameImpl::defaultMod() const
{
  return m_config.fileSystemConfig.searchPath.string();
}

void GameImpl::initializeFileSystem(Logger& logger)
{
  m_fs.initialize(m_config, m_gamePath, m_additionalSearchPaths, logger);
}

EntityPropertyConfig GameImpl::entityPropertyConfig() const
{
  return {
    m_config.entityConfig.scaleExpression, m_config.entityConfig.setDefaultProperties};
}

void GameImpl::writeLongAttribute(
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

std::string GameImpl::readLongAttribute(
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
