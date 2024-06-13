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

#include "TestGame.h"

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/EntityModel.h"
#include "Assets/MaterialManager.h"
#include "Error.h"
#include "Exceptions.h"
#include "IO/BrushFaceReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/ExportOptions.h"
#include "IO/LoadMaterialCollection.h"
#include "IO/NodeReader.h"
#include "IO/NodeWriter.h"
#include "IO/TestParserStatus.h"
#include "IO/VirtualFileSystem.h"
#include "IO/WadFileSystem.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/GameConfig.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"

#include "kdl/result.h"
#include "kdl/string_utils.h"

#include <fstream>
#include <memory>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom::Model
{
TestGame::TestGame()
  : m_fs{std::make_unique<IO::VirtualFileSystem>()}
{
  m_fs->mount("", std::make_unique<IO::DiskFileSystem>(std::filesystem::current_path()));
}

TestGame::~TestGame() = default;

const GameConfig& TestGame::config() const
{
  return m_config;
}

const IO::FileSystem& TestGame::gameFileSystem() const
{
  return *m_fs;
}

std::filesystem::path TestGame::gamePath() const
{
  return ".";
}

void TestGame::setGamePath(
  const std::filesystem::path& /* gamePath */, Logger& /* logger */)
{
}

Game::SoftMapBounds TestGame::extractSoftMapBounds(const Entity&) const
{
  return {Game::SoftMapBoundsType::Game, vm::bbox3()};
}

void TestGame::setAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */, Logger& /* logger */)
{
}
Game::PathErrors TestGame::checkAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */) const
{
  return PathErrors();
}

Result<std::unique_ptr<WorldNode>> TestGame::newMap(
  const MapFormat format, const vm::bbox3& /* worldBounds */, Logger& /* logger */) const
{
  return std::make_unique<WorldNode>(EntityPropertyConfig{}, Entity{}, format);
}

Result<std::unique_ptr<WorldNode>> TestGame::loadMap(
  const MapFormat format,
  const vm::bbox3& /* worldBounds */,
  const std::filesystem::path& /* path */,
  Logger& /* logger */) const
{
  if (!m_worldNodeToLoad)
  {
    return std::make_unique<WorldNode>(EntityPropertyConfig{}, Entity{}, format);
  }
  else
  {
    return std::move(m_worldNodeToLoad);
  }
}

Result<void> TestGame::writeMap(WorldNode& world, const std::filesystem::path& path) const
{
  return IO::Disk::withOutputStream(path, [&](auto& stream) {
    IO::NodeWriter writer(world, stream);
    writer.writeMap();
  });
}

Result<void> TestGame::exportMap(
  WorldNode& /* world */, const IO::ExportOptions& /* options */) const
{
  return kdl::void_success;
}

std::vector<Node*> TestGame::parseNodes(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  Logger& /* logger */) const
{
  IO::TestParserStatus status;
  return IO::NodeReader::read(str, mapFormat, worldBounds, {}, status);
}

std::vector<BrushFace> TestGame::parseBrushFaces(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  Logger& /* logger */) const
{
  IO::TestParserStatus status;
  IO::BrushFaceReader reader(str, mapFormat);
  return reader.read(worldBounds, status);
}

void TestGame::writeNodesToStream(
  WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const
{
  IO::NodeWriter writer(world, stream);
  writer.writeNodes(nodes);
}

void TestGame::writeBrushFacesToStream(
  WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const
{
  IO::NodeWriter writer(world, stream);
  writer.writeBrushFaces(faces);
}

void TestGame::loadMaterialCollections(
  Assets::MaterialManager& materialManager,
  const Assets::CreateTextureResource& createResource) const
{
  const Model::MaterialConfig materialConfig{
    "textures",
    {".D"},
    "fixture/test/palette.lmp",
    "wad",
    "",
    {},
  };

  materialManager.reload(*m_fs, materialConfig, createResource);
}

void TestGame::reloadWads(
  const std::filesystem::path&,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger&)
{
  m_fs->unmountAll();
  m_fs->mount("", std::make_unique<IO::DiskFileSystem>(std::filesystem::current_path()));

  for (const auto& wadPath : wadPaths)
  {
    const auto absoluteWadPath = std::filesystem::current_path() / wadPath;
    m_fs->mount("textures", IO::openFS<IO::WadFileSystem>(absoluteWadPath));
  }
}

bool TestGame::isEntityDefinitionFile(const std::filesystem::path& /* path */) const
{
  return false;
}

std::vector<Assets::EntityDefinitionFileSpec> TestGame::allEntityDefinitionFiles() const
{
  return std::vector<Assets::EntityDefinitionFileSpec>();
}

Assets::EntityDefinitionFileSpec TestGame::extractEntityDefinitionFile(
  const Entity& /* entity */) const
{
  return Assets::EntityDefinitionFileSpec();
}

std::filesystem::path TestGame::findEntityDefinitionFile(
  const Assets::EntityDefinitionFileSpec& /* spec */,
  const std::vector<std::filesystem::path>& /* searchPaths */) const
{
  return {};
}

Result<std::vector<std::string>> TestGame::availableMods() const
{
  return std::vector<std::string>{};
}

std::vector<std::string> TestGame::extractEnabledMods(const Entity& /* entity */) const
{
  return {};
}

std::string TestGame::defaultMod() const
{
  return "";
}

Result<std::vector<std::unique_ptr<Assets::EntityDefinition>>> TestGame::
  loadEntityDefinitions(
    IO::ParserStatus& /* status */, const std::filesystem::path& /* path */) const
{
  return Result<std::vector<std::unique_ptr<Assets::EntityDefinition>>>{
    std::vector<std::unique_ptr<Assets::EntityDefinition>>{}};
}

std::unique_ptr<Assets::EntityModel> TestGame::loadModel(
  const std::filesystem::path& /* path */, Logger& /* logger */) const
{
  return nullptr;
}

void TestGame::setWorldNodeToLoad(std::unique_ptr<WorldNode> worldNode)
{
  m_worldNodeToLoad = std::move(worldNode);
}

void TestGame::setSmartTags(std::vector<SmartTag> smartTags)
{
  m_config.smartTags = std::move(smartTags);
}

void TestGame::setDefaultFaceAttributes(
  const Model::BrushFaceAttributes& defaultFaceAttributes)
{
  m_config.faceAttribsConfig.defaults = defaultFaceAttributes;
}

} // namespace TrenchBroom::Model
