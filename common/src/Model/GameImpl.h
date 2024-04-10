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
#include "Result.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;
} // namespace TrenchBroom

namespace TrenchBroom::Assets
{
class Palette;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
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

public: // implement EntityDefinitionLoader interface:
  Result<std::vector<std::unique_ptr<Assets::EntityDefinition>>> loadEntityDefinitions(
    IO::ParserStatus& status, const std::filesystem::path& path) const override;

public: // implement EntityModelLoader interface:
  std::unique_ptr<Assets::EntityModel> initializeModel(
    const std::filesystem::path& path, Logger& logger) const override;
  void loadFrame(
    const std::filesystem::path& path,
    size_t frameIndex,
    Assets::EntityModel& model,
    Logger& logger) const override;

public: // implement Game interface
  const std::string& gameName() const override;
  std::filesystem::path gamePath() const override;

  void setGamePath(const std::filesystem::path& gamePath, Logger& logger) override;
  void setAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) override;
  PathErrors checkAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const override;

  const CompilationConfig& compilationConfig() override;

  const std::vector<CompilationTool>& compilationTools() const override;

  size_t maxPropertyLength() const override;

  std::optional<vm::bbox3> softMapBounds() const override;
  SoftMapBounds extractSoftMapBounds(const Entity& entity) const override;

  const std::vector<SmartTag>& smartTags() const override;

  Result<std::unique_ptr<WorldNode>> newMap(
    MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const override;
  Result<std::unique_ptr<WorldNode>> loadMap(
    MapFormat format,
    const vm::bbox3& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const override;
  Result<void> writeMap(
    WorldNode& world, const std::filesystem::path& path, bool exporting) const;
  Result<void> writeMap(
    WorldNode& world, const std::filesystem::path& path) const override;
  Result<void> exportMap(
    WorldNode& world, const IO::ExportOptions& options) const override;

  std::vector<Node*> parseNodes(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    Logger& logger) const override;
  std::vector<BrushFace> parseBrushFaces(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    Logger& logger) const override;

  void writeNodesToStream(
    WorldNode& world,
    const std::vector<Node*>& nodes,
    std::ostream& stream) const override;
  void writeBrushFacesToStream(
    WorldNode& world,
    const std::vector<BrushFace>& faces,
    std::ostream& stream) const override;

  void loadTextureCollections(Assets::TextureManager& textureManager) const override;

  const std::optional<std::string>& wadProperty() const override;
  void reloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) override;
  Result<void> reloadShaders() override;

  bool isEntityDefinitionFile(const std::filesystem::path& path) const override;
  std::vector<Assets::EntityDefinitionFileSpec> allEntityDefinitionFiles() const override;
  Assets::EntityDefinitionFileSpec extractEntityDefinitionFile(
    const Entity& entity) const override;
  Assets::EntityDefinitionFileSpec defaultEntityDefinitionFile() const;
  std::filesystem::path findEntityDefinitionFile(
    const Assets::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const override;

  Result<Assets::Palette> loadTexturePalette() const;

  Result<std::vector<std::string>> availableMods() const override;
  std::vector<std::string> extractEnabledMods(const Entity& entity) const override;
  std::string defaultMod() const override;

  const FlagsConfig& surfaceFlags() const override;
  const FlagsConfig& contentFlags() const override;
  const BrushFaceAttributes& defaultFaceAttribs() const override;

private:
  void initializeFileSystem(Logger& logger);

  EntityPropertyConfig entityPropertyConfig() const;

  void writeLongAttribute(
    EntityNodeBase& node,
    const std::string& baseName,
    const std::string& value,
    size_t maxLength) const;
  std::string readLongAttribute(
    const EntityNodeBase& node, const std::string& baseName) const;
};
} // namespace TrenchBroom::Model
