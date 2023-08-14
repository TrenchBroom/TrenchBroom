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

#include "Assets/EntityDefinitionFileSpec.h"
#include "Assets/EntityModel.h"
#include "Assets/TextureManager.h"
#include "Error.h"
#include "Exceptions.h"
#include "IO/BrushFaceReader.h"
#include "IO/DiskFileSystem.h"
#include "IO/DiskIO.h"
#include "IO/ExportOptions.h"
#include "IO/LoadTextureCollection.h"
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

#include <kdl/result.h>
#include <kdl/string_utils.h>

#include <fstream>
#include <memory>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom
{
namespace Model
{
TestGame::TestGame()
  : m_defaultFaceAttributes{Model::BrushFaceAttributes::NoTextureName}
  , m_fs{std::make_unique<IO::VirtualFileSystem>()}
{
  m_fs->mount("", std::make_unique<IO::DiskFileSystem>(std::filesystem::current_path()));
}

TestGame::~TestGame() = default;

void TestGame::setWorldNodeToLoad(std::unique_ptr<WorldNode> worldNode)
{
  m_worldNodeToLoad = std::move(worldNode);
}

void TestGame::setSmartTags(std::vector<SmartTag> smartTags)
{
  m_smartTags = std::move(smartTags);
}

void TestGame::setDefaultFaceAttributes(
  const Model::BrushFaceAttributes& defaultFaceAttributes)
{
  m_defaultFaceAttributes = defaultFaceAttributes;
}

const std::string& TestGame::doGameName() const
{
  static const std::string name("Test");
  return name;
}

std::filesystem::path TestGame::doGamePath() const
{
  return ".";
}

void TestGame::doSetGamePath(
  const std::filesystem::path& /* gamePath */, Logger& /* logger */)
{
}

std::optional<vm::bbox3> TestGame::doSoftMapBounds() const
{
  return {vm::bbox3()};
}

Game::SoftMapBounds TestGame::doExtractSoftMapBounds(const Entity&) const
{
  return {Game::SoftMapBoundsType::Game, vm::bbox3()};
}

void TestGame::doSetAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */, Logger& /* logger */)
{
}
Game::PathErrors TestGame::doCheckAdditionalSearchPaths(
  const std::vector<std::filesystem::path>& /* searchPaths */) const
{
  return PathErrors();
}

const CompilationConfig& TestGame::doCompilationConfig()
{
  static CompilationConfig config;
  return config;
}

size_t TestGame::doMaxPropertyLength() const
{
  return 1024;
}

const std::vector<SmartTag>& TestGame::doSmartTags() const
{
  return m_smartTags;
}

kdl::result<std::unique_ptr<WorldNode>, Error> TestGame::doNewMap(
  const MapFormat format, const vm::bbox3& /* worldBounds */, Logger& /* logger */) const
{
  return std::make_unique<WorldNode>(EntityPropertyConfig{}, Entity{}, format);
}

kdl::result<std::unique_ptr<WorldNode>, Error> TestGame::doLoadMap(
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

kdl::result<void, Error> TestGame::doWriteMap(
  WorldNode& world, const std::filesystem::path& path) const
{
  return IO::Disk::withOutputStream(path, [&](auto& stream) {
    IO::NodeWriter writer(world, stream);
    writer.writeMap();
  });
}

kdl::result<void, Error> TestGame::doExportMap(
  WorldNode& /* world */, const IO::ExportOptions& /* options */) const
{
  return kdl::void_success;
}

std::vector<Node*> TestGame::doParseNodes(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  const std::vector<std::string>& linkedGroupsToKeep,
  Logger& /* logger */) const
{
  IO::TestParserStatus status;
  return IO::NodeReader::read(
    str, mapFormat, worldBounds, {}, linkedGroupsToKeep, status);
}

std::vector<BrushFace> TestGame::doParseBrushFaces(
  const std::string& str,
  const MapFormat mapFormat,
  const vm::bbox3& worldBounds,
  Logger& /* logger */) const
{
  IO::TestParserStatus status;
  IO::BrushFaceReader reader(str, mapFormat);
  return reader.read(worldBounds, status);
}

void TestGame::doWriteNodesToStream(
  WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const
{
  IO::NodeWriter writer(world, stream);
  writer.writeNodes(nodes);
}

void TestGame::doWriteBrushFacesToStream(
  WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const
{
  IO::NodeWriter writer(world, stream);
  writer.writeBrushFaces(faces);
}

void TestGame::doLoadTextureCollections(Assets::TextureManager& textureManager) const
{
  const Model::TextureConfig textureConfig{
    "textures",
    {".D"},
    "fixture/test/palette.lmp",
    "wad",
    "",
    {},
  };

  textureManager.reload(*m_fs, textureConfig);
}

void TestGame::doReloadWads(
  const std::filesystem::path&,
  const std::vector<std::filesystem::path>& wadPaths,
  Logger&)
{
  m_fs->unmountAll();
  m_fs->mount("", std::make_unique<IO::DiskFileSystem>(std::filesystem::current_path()));

  for (const auto& wadPath : wadPaths)
  {
    const auto absoluteWadPath = std::filesystem::current_path() / wadPath;
    m_fs->mount(
      "textures" / wadPath.filename(), IO::openFS<IO::WadFileSystem>(absoluteWadPath));
  }
}

kdl::result<void, Error> TestGame::doReloadShaders()
{
  return kdl::void_success;
  ;
}

bool TestGame::doIsEntityDefinitionFile(const std::filesystem::path& /* path */) const
{
  return false;
}

std::vector<Assets::EntityDefinitionFileSpec> TestGame::doAllEntityDefinitionFiles() const
{
  return std::vector<Assets::EntityDefinitionFileSpec>();
}

Assets::EntityDefinitionFileSpec TestGame::doExtractEntityDefinitionFile(
  const Entity& /* entity */) const
{
  return Assets::EntityDefinitionFileSpec();
}

std::filesystem::path TestGame::doFindEntityDefinitionFile(
  const Assets::EntityDefinitionFileSpec& /* spec */,
  const std::vector<std::filesystem::path>& /* searchPaths */) const
{
  return {};
}

kdl::result<std::vector<std::string>, Error> TestGame::doAvailableMods() const
{
  return std::vector<std::string>{};
}

std::vector<std::string> TestGame::doExtractEnabledMods(const Entity& /* entity */) const
{
  return {};
}

std::string TestGame::doDefaultMod() const
{
  return "";
}

const Model::FlagsConfig& TestGame::doSurfaceFlags() const
{
  static const Model::FlagsConfig config;
  return config;
}

const Model::FlagsConfig& TestGame::doContentFlags() const
{
  static const Model::FlagsConfig config;
  return config;
}

const Model::BrushFaceAttributes& TestGame::doDefaultFaceAttribs() const
{
  return m_defaultFaceAttributes;
}

const std::vector<CompilationTool>& TestGame::doCompilationTools() const
{
  return m_compilationTools;
}

kdl::result<std::vector<Assets::EntityDefinition*>, Error> TestGame::
  doLoadEntityDefinitions(
    IO::ParserStatus& /* status */, const std::filesystem::path& /* path */) const
{
  return kdl::result<std::vector<Assets::EntityDefinition*>, Error>{
    std::vector<Assets::EntityDefinition*>{}};
}

std::unique_ptr<Assets::EntityModel> TestGame::doInitializeModel(
  const std::filesystem::path& /* path */, Logger& /* logger */) const
{
  return nullptr;
}
void TestGame::doLoadFrame(
  const std::filesystem::path& /* path */,
  size_t /* frameIndex */,
  Assets::EntityModel& /* model */,
  Logger& /* logger */) const
{
}
} // namespace Model
} // namespace TrenchBroom
