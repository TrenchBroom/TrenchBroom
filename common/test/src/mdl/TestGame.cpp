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

#include "TestGame.h"

#include "TestUtils.h"
#include "io/BrushFaceReader.h"
#include "io/DiskFileSystem.h"
#include "io/DiskIO.h"
#include "io/ExportOptions.h"
#include "io/NodeReader.h"
#include "io/NodeWriter.h"
#include "io/TestParserStatus.h"
#include "io/VirtualFileSystem.h"
#include "io/WadFileSystem.h"
#include "mdl/BrushFace.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/GameConfig.h"
#include "mdl/MaterialManager.h"
#include "mdl/WorldNode.h"

#include "kdl/result.h"

#include <memory>
#include <vector>

namespace tb::mdl
{

TestGame::TestGame()
  : m_fs{std::make_unique<io::VirtualFileSystem>()}
{
  m_fs->mount("", std::make_unique<io::DiskFileSystem>(std::filesystem::current_path()));
}

TestGame::~TestGame() = default;

const GameConfig& TestGame::config() const
{
  return m_config;
}

const io::FileSystem& TestGame::gameFileSystem() const
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
  return {Game::SoftMapBoundsType::Game, vm::bbox3d()};
}

void TestGame::setAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */, Logger& /* logger */)
{
}

Game::PathErrors TestGame::checkAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */) const
{
  return {};
}

Result<std::unique_ptr<WorldNode>> TestGame::newMap(
  const MapFormat format, const vm::bbox3d& /* worldBounds */, Logger& /* logger */) const
{
  return std::make_unique<WorldNode>(EntityPropertyConfig{}, Entity{}, format);
}

Result<std::unique_ptr<WorldNode>> TestGame::loadMap(
  const MapFormat format,
  const vm::bbox3d& /* worldBounds */,
  const std::filesystem::path& /* path */,
  Logger& /* logger */) const
{
  return m_worldNodeToLoad
           ? std::move(m_worldNodeToLoad)
           : std::make_unique<WorldNode>(EntityPropertyConfig{}, Entity{}, format);
}

Result<void> TestGame::writeMap(WorldNode& world, const std::filesystem::path& path) const
{
  return io::Disk::withOutputStream(path, [&](auto& stream) {
    auto writer = io::NodeWriter{world, stream};
    writer.writeMap();
  });
}

Result<void> TestGame::exportMap(
  WorldNode& /* world */, const io::ExportOptions& /* options */) const
{
  return kdl::void_success;
}

std::vector<Node*> TestGame::parseNodes(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  Logger& /* logger */) const
{
  auto status = io::TestParserStatus{};
  return io::NodeReader::read(str, mapFormat, worldBounds, {}, status);
}

std::vector<BrushFace> TestGame::parseBrushFaces(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3d& worldBounds,
  Logger& /* logger */) const
{
  auto status = io::TestParserStatus{};
  auto reader = io::BrushFaceReader{str, mapFormat};
  return reader.read(worldBounds, status);
}

void TestGame::writeNodesToStream(
  WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const
{
  auto writer = io::NodeWriter{world, stream};
  writer.writeNodes(nodes);
}

void TestGame::writeBrushFacesToStream(
  WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const
{
  auto writer = io::NodeWriter{world, stream};
  writer.writeBrushFaces(faces);
}

void TestGame::reloadWads(
  const std::filesystem::path&,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger&)
{
  m_fs->unmountAll();
  m_fs->mount("", std::make_unique<io::DiskFileSystem>(std::filesystem::current_path()));

  for (const auto& wadPath : wadPaths)
  {
    const auto absoluteWadPath = std::filesystem::current_path() / wadPath;
    m_fs->mount("textures", io::openFS<io::WadFileSystem>(absoluteWadPath));
  }
}

bool TestGame::isEntityDefinitionFile(const std::filesystem::path& /* path */) const
{
  return false;
}

std::vector<EntityDefinitionFileSpec> TestGame::allEntityDefinitionFiles() const
{
  return {};
}

EntityDefinitionFileSpec TestGame::extractEntityDefinitionFile(
  const Entity& /* entity */) const
{
  return {};
}

std::filesystem::path TestGame::findEntityDefinitionFile(
  const EntityDefinitionFileSpec& /* spec */,
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

Result<std::vector<std::unique_ptr<EntityDefinition>>> TestGame::loadEntityDefinitions(
  io::ParserStatus& /* status */, const std::filesystem::path& /* path */) const
{
  return std::vector<std::unique_ptr<EntityDefinition>>{};
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
  const mdl::BrushFaceAttributes& defaultFaceAttributes)
{
  m_config.faceAttribsConfig.defaults = defaultFaceAttributes;
}

} // namespace tb::mdl
