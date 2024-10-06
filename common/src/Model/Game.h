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

#pragma once

#include "Model/GameConfig.h"
#include "Model/MapFormat.h"
#include "Result.h"
#include "asset/TextureResource.h"
#include "io/EntityDefinitionLoader.h"
#include "io/ExportOptions.h"

#include "vm/bbox.h"

#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tb
{
class Logger;
} // namespace tb

namespace tb::asset
{
class EntityDefinitionFileSpec;
class MaterialManager;
} // namespace tb::asset

namespace tb::io
{
class FileSystem;
}

namespace tb::Model
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

class Game : public io::EntityDefinitionLoader
{
public: // game configuration
  virtual const GameConfig& config() const = 0;
  virtual const io::FileSystem& gameFileSystem() const = 0;

  bool isGamePathPreference(const std::filesystem::path& prefPath) const;

  virtual std::filesystem::path gamePath() const = 0;
  virtual void setGamePath(const std::filesystem::path& gamePath, Logger& logger) = 0;

  virtual void setAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) = 0;

  using PathErrors = std::map<std::filesystem::path, std::string>;
  virtual PathErrors checkAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const = 0;

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
    std::optional<vm::bbox3d> bounds;
  };

  /**
   * Returns the soft map bounds specified in the given World entity, or if unset, the
   * value from softMapBounds()
   */
  virtual SoftMapBounds extractSoftMapBounds(const Entity& entity) const = 0;

public: // loading and writing map files
  virtual Result<std::unique_ptr<WorldNode>> newMap(
    MapFormat format, const vm::bbox3d& worldBounds, Logger& logger) const = 0;
  virtual Result<std::unique_ptr<WorldNode>> loadMap(
    MapFormat format,
    const vm::bbox3d& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const = 0;
  virtual Result<void> writeMap(
    WorldNode& world, const std::filesystem::path& path) const = 0;
  virtual Result<void> exportMap(
    WorldNode& world, const io::ExportOptions& options) const = 0;

public: // parsing and serializing objects
  virtual std::vector<Node*> parseNodes(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    Logger& logger) const = 0;
  virtual std::vector<BrushFace> parseBrushFaces(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3d& worldBounds,
    Logger& logger) const = 0;

  virtual void writeNodesToStream(
    WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const = 0;
  virtual void writeBrushFacesToStream(
    WorldNode& world,
    const std::vector<BrushFace>& faces,
    std::ostream& stream) const = 0;

public: // material collection handling
  virtual void loadMaterialCollections(
    asset::MaterialManager& materialManager,
    const asset::CreateTextureResource& createResource) const = 0;

  virtual void reloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) = 0;

public: // entity definition handling
  virtual bool isEntityDefinitionFile(const std::filesystem::path& path) const = 0;
  virtual std::vector<asset::EntityDefinitionFileSpec> allEntityDefinitionFiles()
    const = 0;
  virtual asset::EntityDefinitionFileSpec extractEntityDefinitionFile(
    const Entity& entity) const = 0;
  virtual std::filesystem::path findEntityDefinitionFile(
    const asset::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const = 0;

public: // mods
  virtual Result<std::vector<std::string>> availableMods() const = 0;
  virtual std::vector<std::string> extractEnabledMods(const Entity& entity) const = 0;
  virtual std::string defaultMod() const = 0;
};
} // namespace tb::Model
