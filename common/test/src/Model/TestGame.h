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

#pragma once

#include "IO/VirtualFileSystem.h"
#include "Model/BrushFaceAttributes.h"
#include "Model/Game.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;

namespace IO
{
class Path;
class VirtualFileSystem;
} // namespace IO

namespace Model
{
class TestGame : public Game
{
private:
  mutable std::unique_ptr<WorldNode> m_worldNodeToLoad;
  std::vector<SmartTag> m_smartTags;
  Model::BrushFaceAttributes m_defaultFaceAttributes;
  std::vector<CompilationTool> m_compilationTools;
  std::unique_ptr<IO::VirtualFileSystem> m_fs;

public:
  TestGame();
  ~TestGame() override;

public:
  void setWorldNodeToLoad(std::unique_ptr<WorldNode> worldNode);
  void setSmartTags(std::vector<SmartTag> smartTags);
  void setDefaultFaceAttributes(const Model::BrushFaceAttributes& newDefaults);

private:
  const std::string& doGameName() const override;
  std::filesystem::path doGamePath() const override;
  void doSetGamePath(const std::filesystem::path& gamePath, Logger& logger) override;
  std::optional<vm::bbox3> doSoftMapBounds() const override;
  Game::SoftMapBounds doExtractSoftMapBounds(const Entity& entity) const override;
  void doSetAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) override;
  PathErrors doCheckAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const override;

  const CompilationConfig& doCompilationConfig() override;
  size_t doMaxPropertyLength() const override;

  const std::vector<SmartTag>& doSmartTags() const override;

  kdl::result<std::unique_ptr<WorldNode>, Error> doNewMap(
    MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const override;
  kdl::result<std::unique_ptr<WorldNode>, Error> doLoadMap(
    MapFormat format,
    const vm::bbox3& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const override;
  kdl::result<void, Error> doWriteMap(
    WorldNode& world, const std::filesystem::path& path) const override;
  kdl::result<void, Error> doExportMap(
    WorldNode& world, const IO::ExportOptions& options) const override;

  std::vector<Node*> doParseNodes(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    const std::vector<std::string>& linkedGroupsToKeep,
    Logger& logger) const override;
  std::vector<BrushFace> doParseBrushFaces(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    Logger& logger) const override;
  void doWriteNodesToStream(
    WorldNode& world,
    const std::vector<Node*>& nodes,
    std::ostream& stream) const override;
  void doWriteBrushFacesToStream(
    WorldNode& world,
    const std::vector<BrushFace>& faces,
    std::ostream& stream) const override;

  void doLoadTextureCollections(Assets::TextureManager& textureManager) const override;

  void doReloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) override;
  kdl::result<void, Error> doReloadShaders() override;

  bool doIsEntityDefinitionFile(const std::filesystem::path& path) const override;
  std::vector<Assets::EntityDefinitionFileSpec> doAllEntityDefinitionFiles()
    const override;
  Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(
    const Entity& entity) const override;
  std::filesystem::path doFindEntityDefinitionFile(
    const Assets::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const override;

  kdl::result<std::vector<std::string>, Error> doAvailableMods() const override;
  std::vector<std::string> doExtractEnabledMods(const Entity& entity) const override;
  std::string doDefaultMod() const override;

  const FlagsConfig& doSurfaceFlags() const override;
  const FlagsConfig& doContentFlags() const override;
  const BrushFaceAttributes& doDefaultFaceAttribs() const override;
  const std::vector<CompilationTool>& doCompilationTools() const override;

  kdl::result<std::vector<Assets::EntityDefinition*>, Assets::AssetError>
  doLoadEntityDefinitions(
    IO::ParserStatus& status, const std::filesystem::path& path) const override;
  std::unique_ptr<Assets::EntityModel> doInitializeModel(
    const std::filesystem::path& path, Logger& logger) const override;
  void doLoadFrame(
    const std::filesystem::path& path,
    size_t frameIndex,
    Assets::EntityModel& model,
    Logger& logger) const override;
};
} // namespace Model
} // namespace TrenchBroom
