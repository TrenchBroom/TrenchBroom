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
GameImpl::GameImpl(GameConfig config, std::filesystem::path gamePath, Logger& logger)
  : m_config{std::move(config)}
  , m_gamePath{std::move(gamePath)}
{
  initializeFileSystem(logger);
}

Result<std::vector<EntityDefinition>> GameImpl::loadEntityDefinitions(
  io::ParserStatus& status, const std::filesystem::path& path) const
{
  const auto extension = kdl::path_to_lower(path.extension());
  const auto& defaultColor = m_config.entityConfig.defaultColor;

  if (extension == ".fgd")
  {
    return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             auto parser = io::FgdParser{reader.stringView(), defaultColor, path};
             return parser.parseDefinitions(status);
           });
  }
  if (extension == ".def")
  {
    return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             auto parser = io::DefParser{reader.stringView(), defaultColor};
             return parser.parseDefinitions(status);
           });
  }
  if (extension == ".ent")
  {
    return io::Disk::openFile(path) | kdl::and_then([&](auto file) {
             auto reader = file->reader().buffer();
             auto parser = io::EntParser{reader.stringView(), defaultColor};
             return parser.parseDefinitions(status);
           });
  }

  return Error{fmt::format("Unknown entity definition format: {}", path)};
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
      result.emplace(searchPath, fmt::format("Directory not found: {}", searchPath));
    }
  }
  return result;
}

SoftMapBounds GameImpl::extractSoftMapBounds(const Entity& entity) const
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

  return std::ranges::any_of(extensions, [&](const auto& extension) {
    return kdl::path_has_extension(kdl::path_to_lower(path), extension);
  });
}

std::vector<EntityDefinitionFileSpec> GameImpl::allEntityDefinitionFiles() const
{
  return m_config.entityConfig.defFilePaths | std::views::transform([](const auto& path) {
           return EntityDefinitionFileSpec::makeBuiltin(path);
         })
         | kdl::ranges::to<std::vector>();
}

std::filesystem::path GameImpl::findEntityDefinitionFile(
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

  return io::Disk::resolvePath(searchPaths, spec.path);
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
