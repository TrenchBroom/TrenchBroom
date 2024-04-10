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
#include "IO/EntityDefinitionLoader.h"
#include "IO/EntityModelLoader.h"
#include "IO/ExportOptions.h"
#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Result.h"

#include "vm/bbox.h"
#include "vm/forward.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace TrenchBroom
{
class Logger;
} // namespace TrenchBroom

namespace TrenchBroom::Assets
{
class EntityDefinitionFileSpec;
class TextureManager;
} // namespace TrenchBroom::Assets

namespace TrenchBroom::Model
{
class EntityNodeBase;
class BrushFace;
class BrushFaceAttributes;
struct CompilationConfig;
class Entity;
struct FlagsConfig;
class Node;
class SmartTag;
class WorldNode;

class Game : public IO::EntityDefinitionLoader, public IO::EntityModelLoader
{
public: // game configuration
  virtual const std::string& gameName() const = 0;

  bool isGamePathPreference(const std::filesystem::path& prefPath) const;

  virtual std::filesystem::path gamePath() const = 0;
  virtual void setGamePath(const std::filesystem::path& gamePath, Logger& logger) = 0;

  virtual void setAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) = 0;

  using PathErrors = std::map<std::filesystem::path, std::string>;
  virtual PathErrors checkAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const = 0;

  virtual const CompilationConfig& compilationConfig() = 0;

  virtual const std::vector<CompilationTool>& compilationTools() const = 0;

  virtual size_t maxPropertyLength() const = 0;

  virtual const std::vector<SmartTag>& smartTags() const = 0;

  enum class SoftMapBoundsType
  {
    Game,
    Map
  };

  struct SoftMapBounds
  {
    SoftMapBoundsType source;
    /**
     * std::nullopt indicates unlimited soft map bounds
     */
    std::optional<vm::bbox3> bounds;
  };

  /**
   * Returns the soft map bounds configured in the game config
   */
  virtual std::optional<vm::bbox3> softMapBounds() const = 0;

  /**
   * Returns the soft map bounds specified in the given World entity, or if unset, the
   * value from softMapBounds()
   */
  virtual SoftMapBounds extractSoftMapBounds(const Entity& entity) const = 0;

public: // loading and writing map files
  virtual Result<std::unique_ptr<WorldNode>> newMap(
    MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const = 0;
  virtual Result<std::unique_ptr<WorldNode>> loadMap(
    MapFormat format,
    const vm::bbox3& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const = 0;
  virtual Result<void> writeMap(
    WorldNode& world, const std::filesystem::path& path) const = 0;
  virtual Result<void> exportMap(
    WorldNode& world, const IO::ExportOptions& options) const = 0;

public: // parsing and serializing objects
  virtual std::vector<Node*> parseNodes(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    Logger& logger) const = 0;
  virtual std::vector<BrushFace> parseBrushFaces(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    Logger& logger) const = 0;

  virtual void writeNodesToStream(
    WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const = 0;
  virtual void writeBrushFacesToStream(
    WorldNode& world,
    const std::vector<BrushFace>& faces,
    std::ostream& stream) const = 0;

public: // texture collection handling
  virtual void loadTextureCollections(Assets::TextureManager& textureManagerr) const = 0;

  virtual const std::optional<std::string>& wadProperty() const = 0;
  virtual void reloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) = 0;
  virtual Result<void> reloadShaders() = 0;

public: // entity definition handling
  virtual bool isEntityDefinitionFile(const std::filesystem::path& path) const = 0;
  virtual std::vector<Assets::EntityDefinitionFileSpec> allEntityDefinitionFiles()
    const = 0;
  virtual Assets::EntityDefinitionFileSpec extractEntityDefinitionFile(
    const Entity& entity) const = 0;
  virtual std::filesystem::path findEntityDefinitionFile(
    const Assets::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const = 0;

public: // mods
  virtual Result<std::vector<std::string>> availableMods() const = 0;
  virtual std::vector<std::string> extractEnabledMods(const Entity& entity) const = 0;
  virtual std::string defaultMod() const = 0;

public: // configs for faces
  virtual const FlagsConfig& surfaceFlags() const = 0;
  virtual const FlagsConfig& contentFlags() const = 0;
  virtual const BrushFaceAttributes& defaultFaceAttribs() const = 0;
};
} // namespace TrenchBroom::Model
