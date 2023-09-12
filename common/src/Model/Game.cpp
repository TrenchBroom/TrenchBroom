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

#include "Game.h"

#include "Assets/EntityDefinitionFileSpec.h"
#include "Error.h"
#include "IO/ExportOptions.h"
#include "Model/BrushFace.h"
#include "Model/GameFactory.h"
#include "Model/WorldNode.h"

#include <kdl/result.h>

#include <string>
#include <vector>

namespace TrenchBroom::Model
{
const std::string& Game::gameName() const
{
  return doGameName();
}

bool Game::isGamePathPreference(const std::filesystem::path& prefPath) const
{
  const GameFactory& gameFactory = GameFactory::instance();
  return gameFactory.isGamePathPreference(gameName(), prefPath);
}

std::filesystem::path Game::gamePath() const
{
  return doGamePath();
}

void Game::setGamePath(const std::filesystem::path& gamePath, Logger& logger)
{
  doSetGamePath(gamePath, logger);
}

void Game::setAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths, Logger& logger)
{
  doSetAdditionalSearchPaths(searchPaths, logger);
}

Game::PathErrors Game::checkAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& searchPaths) const
{
  return doCheckAdditionalSearchPaths(searchPaths);
}

const CompilationConfig& Game::compilationConfig()
{
  return doCompilationConfig();
}

size_t Game::maxPropertyLength() const
{
  return doMaxPropertyLength();
}

const std::vector<SmartTag>& Game::smartTags() const
{
  return doSmartTags();
}

std::optional<vm::bbox3> Game::softMapBounds() const
{
  return doSoftMapBounds();
}

Game::SoftMapBounds Game::extractSoftMapBounds(const Entity& entity) const
{
  return doExtractSoftMapBounds(entity);
}

Result<std::unique_ptr<WorldNode>> Game::newMap(
  const MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const
{
  return doNewMap(format, worldBounds, logger);
}

Result<std::unique_ptr<WorldNode>> Game::loadMap(
  const MapFormat format,
  const vm::bbox3& worldBounds,
  const std::filesystem::path& path,
  Logger& logger) const
{
  return doLoadMap(format, worldBounds, path, logger);
}

Result<void> Game::writeMap(WorldNode& world, const std::filesystem::path& path) const
{
  return doWriteMap(world, path);
}

Result<void> Game::exportMap(WorldNode& world, const IO::ExportOptions& options) const
{
  return doExportMap(world, options);
}

std::vector<Node*> Game::parseNodes(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  const std::vector<std::string>& linkedGroupsToKeep,
  Logger& logger) const
{
  return doParseNodes(str, mapFormat, worldBounds, linkedGroupsToKeep, logger);
}

std::vector<BrushFace> Game::parseBrushFaces(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  Logger& logger) const
{
  return doParseBrushFaces(str, mapFormat, worldBounds, logger);
}

void Game::writeNodesToStream(
  WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const
{
  doWriteNodesToStream(world, nodes, stream);
}

void Game::writeBrushFacesToStream(
  WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const
{
  doWriteBrushFacesToStream(world, faces, stream);
}

void Game::loadTextureCollections(Assets::TextureManager& textureManager) const
{
  doLoadTextureCollections(textureManager);
}

const std::optional<std::string>& Game::wadProperty() const
{
  return doGetWadProperty();
}

void Game::reloadWads(
  const std::filesystem::path& documentPath,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger& logger)
{
  doReloadWads(documentPath, wadPaths, logger);
}

Result<void> Game::reloadShaders()
{
  return doReloadShaders();
}

bool Game::isEntityDefinitionFile(const std::filesystem::path& path) const
{
  return doIsEntityDefinitionFile(path);
}

std::vector<Assets::EntityDefinitionFileSpec> Game::allEntityDefinitionFiles() const
{
  return doAllEntityDefinitionFiles();
}

Assets::EntityDefinitionFileSpec Game::extractEntityDefinitionFile(
  const Entity& entity) const
{
  return doExtractEntityDefinitionFile(entity);
}

std::filesystem::path Game::findEntityDefinitionFile(
  const Assets::EntityDefinitionFileSpec& spec,
  const std::vector<std::filesystem::path>& searchPaths) const
{
  return doFindEntityDefinitionFile(spec, searchPaths);
}

Result<std::vector<std::string>> Game::availableMods() const
{
  return doAvailableMods();
}

std::vector<std::string> Game::extractEnabledMods(const Entity& entity) const
{
  return doExtractEnabledMods(entity);
}

std::string Game::defaultMod() const
{
  return doDefaultMod();
}

const FlagsConfig& Game::surfaceFlags() const
{
  return doSurfaceFlags();
}

const FlagsConfig& Game::contentFlags() const
{
  return doContentFlags();
}

const BrushFaceAttributes& Game::defaultFaceAttribs() const
{
  return doDefaultFaceAttribs();
}

const std::vector<CompilationTool>& Game::compilationTools() const
{
  return doCompilationTools();
}
} // namespace TrenchBroom::Model
