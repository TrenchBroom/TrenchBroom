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

#include "FloatType.h"
#include "Model/Game.h"
#include "Model/GameFileSystem.h"

#include <kdl/result_forward.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;

namespace Assets
{
class Palette;
} // namespace Assets

namespace IO
{
struct FileSystemError;
}

namespace Model
{
struct EntityPropertyConfig;

class GameImpl : public Game
{
private:
  GameConfig& m_config;
  GameFileSystem m_fs;
  std::filesystem::path m_gamePath;
  std::vector<std::filesystem::path> m_additionalSearchPaths;

public:
  GameImpl(GameConfig& config, std::filesystem::path gamePath, Logger& logger);

private:
  void initializeFileSystem(Logger& logger);

private:
  const std::string& doGameName() const override;
  std::filesystem::path doGamePath() const override;
  void doSetGamePath(const std::filesystem::path& gamePath, Logger& logger) override;
  void doSetAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) override;
  PathErrors doCheckAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const override;

  const CompilationConfig& doCompilationConfig() override;

  size_t doMaxPropertyLength() const override;

  std::optional<vm::bbox3> doSoftMapBounds() const override;
  SoftMapBounds doExtractSoftMapBounds(const Entity& entity) const override;

  const std::vector<SmartTag>& doSmartTags() const override;

  kdl::result<std::unique_ptr<WorldNode>, Error> doNewMap(
    MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const override;
  kdl::result<std::unique_ptr<WorldNode>, Error> doLoadMap(
    MapFormat format,
    const vm::bbox3& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const override;
  kdl::result<void, Error> doWriteMap(
    WorldNode& world, const std::filesystem::path& path, bool exporting) const;
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
  kdl::result<std::vector<Assets::EntityDefinition*>, Error> doLoadEntityDefinitions(
    IO::ParserStatus& status, const std::filesystem::path& path) const override;
  std::vector<Assets::EntityDefinitionFileSpec> doAllEntityDefinitionFiles()
    const override;
  Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(
    const Entity& entity) const override;
  Assets::EntityDefinitionFileSpec defaultEntityDefinitionFile() const;
  std::filesystem::path doFindEntityDefinitionFile(
    const Assets::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const override;

  std::unique_ptr<Assets::EntityModel> doInitializeModel(
    const std::filesystem::path& path, Logger& logger) const override;
  void doLoadFrame(
    const std::filesystem::path& path,
    size_t frameIndex,
    Assets::EntityModel& model,
    Logger& logger) const override;

  kdl::result<Assets::Palette, Error> loadTexturePalette() const;

  kdl::result<std::vector<std::string>, Error> doAvailableMods() const override;
  std::vector<std::string> doExtractEnabledMods(const Entity& entity) const override;
  std::string doDefaultMod() const override;

  const FlagsConfig& doSurfaceFlags() const override;
  const FlagsConfig& doContentFlags() const override;
  const BrushFaceAttributes& doDefaultFaceAttribs() const override;
  const std::vector<CompilationTool>& doCompilationTools() const override;

private:
  EntityPropertyConfig entityPropertyConfig() const;

  void writeLongAttribute(
    EntityNodeBase& node,
    const std::string& baseName,
    const std::string& value,
    size_t maxLength) const;
  std::string readLongAttribute(
    const EntityNodeBase& node, const std::string& baseName) const;
};
} // namespace Model
} // namespace TrenchBroom
