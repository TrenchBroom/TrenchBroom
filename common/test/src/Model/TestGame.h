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
#include "Result.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
class Logger;
}

namespace TrenchBroom::IO
{
class Path;
class VirtualFileSystem;
} // namespace TrenchBroom::IO

namespace TrenchBroom::Model
{
class TestGame : public Game
{
private:
  GameConfig m_config = {"Test", {}, {}, false, {}, {}, {}, {}, {}, {}, {}, {}};
  std::unique_ptr<IO::VirtualFileSystem> m_fs;
  mutable std::unique_ptr<WorldNode> m_worldNodeToLoad;

public:
  TestGame();
  ~TestGame() override;

  const GameConfig& config() const override;
  const IO::FileSystem& gameFileSystem() const override;

  std::filesystem::path gamePath() const override;
  void setGamePath(const std::filesystem::path& gamePath, Logger& logger) override;
  Game::SoftMapBounds extractSoftMapBounds(const Entity& entity) const override;
  void setAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) override;
  PathErrors checkAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const override;

  Result<std::unique_ptr<WorldNode>> newMap(
    MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const override;
  Result<std::unique_ptr<WorldNode>> loadMap(
    MapFormat format,
    const vm::bbox3& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const override;
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

  void loadMaterialCollections(
    Assets::MaterialManager& materialManager,
    const Assets::CreateTextureResource& createResource) const override;

  void reloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) override;

  bool isEntityDefinitionFile(const std::filesystem::path& path) const override;
  std::vector<Assets::EntityDefinitionFileSpec> allEntityDefinitionFiles() const override;
  Assets::EntityDefinitionFileSpec extractEntityDefinitionFile(
    const Entity& entity) const override;
  std::filesystem::path findEntityDefinitionFile(
    const Assets::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const override;

  Result<std::vector<std::string>> availableMods() const override;
  std::vector<std::string> extractEnabledMods(const Entity& entity) const override;
  std::string defaultMod() const override;

  Result<std::vector<std::unique_ptr<Assets::EntityDefinition>>> loadEntityDefinitions(
    IO::ParserStatus& status, const std::filesystem::path& path) const override;

  void setWorldNodeToLoad(std::unique_ptr<WorldNode> worldNode);
  void setSmartTags(std::vector<SmartTag> smartTags);
  void setDefaultFaceAttributes(const Model::BrushFaceAttributes& newDefaults);
};
} // namespace TrenchBroom::Model
