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

#include <kdl/result_forward.h>

#include <vecmath/bbox.h>
#include <vecmath/forward.h>

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

namespace Assets
{
class EntityDefinitionFileSpec;
class TextureManager;
} // namespace Assets

namespace IO
{
struct FileSystemError;
}

namespace Model
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
public:
  const std::string& gameName() const;
  bool isGamePathPreference(const std::filesystem::path& prefPath) const;

  std::filesystem::path gamePath() const;
  void setGamePath(const std::filesystem::path& gamePath, Logger& logger);
  void setAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger);

  using PathErrors = std::map<std::filesystem::path, std::string>;
  PathErrors checkAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const;

  const CompilationConfig& compilationConfig();

  size_t maxPropertyLength() const;

  const std::vector<SmartTag>& smartTags() const;

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
  std::optional<vm::bbox3> softMapBounds() const;
  /**
   * Returns the soft map bounds specified in the given World entity, or if unset, the
   * value from softMapBounds()
   */
  SoftMapBounds extractSoftMapBounds(const Entity& entity) const;

public: // loading and writing map files
  std::unique_ptr<WorldNode> newMap(
    MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const;
  std::unique_ptr<WorldNode> loadMap(
    MapFormat format,
    const vm::bbox3& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const;
  kdl::result<void, IO::FileSystemError> writeMap(
    WorldNode& world, const std::filesystem::path& path) const;
  kdl::result<void, IO::FileSystemError> exportMap(
    WorldNode& world, const IO::ExportOptions& options) const;

public: // parsing and serializing objects
  std::vector<Node*> parseNodes(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    const std::vector<std::string>& linkedGroupsToKeep,
    Logger& logger) const;
  std::vector<BrushFace> parseBrushFaces(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    Logger& logger) const;

  void writeNodesToStream(
    WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const;
  void writeBrushFacesToStream(
    WorldNode& world, const std::vector<BrushFace>& faces, std::ostream& stream) const;

public: // texture collection handling
  void loadTextureCollections(Assets::TextureManager& textureManagerr) const;

  void reloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger);
  void reloadShaders();

public: // entity definition handling
  bool isEntityDefinitionFile(const std::filesystem::path& path) const;
  std::vector<Assets::EntityDefinitionFileSpec> allEntityDefinitionFiles() const;
  Assets::EntityDefinitionFileSpec extractEntityDefinitionFile(
    const Entity& entity) const;
  std::filesystem::path findEntityDefinitionFile(
    const Assets::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const;

public: // mods
  std::vector<std::string> availableMods() const;
  std::vector<std::string> extractEnabledMods(const Entity& entity) const;
  std::string defaultMod() const;

public: // configs for faces
  const FlagsConfig& surfaceFlags() const;
  const FlagsConfig& contentFlags() const;
  const BrushFaceAttributes& defaultFaceAttribs() const;

public: // compilation tools
  const std::vector<CompilationTool>& compilationTools() const;

private: // subclassing interface
  virtual const std::string& doGameName() const = 0;
  virtual std::filesystem::path doGamePath() const = 0;
  virtual void doSetGamePath(const std::filesystem::path& gamePath, Logger& logger) = 0;
  virtual void doSetAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) = 0;
  virtual PathErrors doCheckAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const = 0;

  virtual const CompilationConfig& doCompilationConfig() = 0;
  virtual size_t doMaxPropertyLength() const = 0;
  virtual std::optional<vm::bbox3> doSoftMapBounds() const = 0;
  virtual SoftMapBounds doExtractSoftMapBounds(const Entity& entity) const = 0;

  virtual const std::vector<SmartTag>& doSmartTags() const = 0;

  virtual std::unique_ptr<WorldNode> doNewMap(
    MapFormat format, const vm::bbox3& worldBounds, Logger& logger) const = 0;
  virtual std::unique_ptr<WorldNode> doLoadMap(
    MapFormat format,
    const vm::bbox3& worldBounds,
    const std::filesystem::path& path,
    Logger& logger) const = 0;
  virtual kdl::result<void, IO::FileSystemError> doWriteMap(
    WorldNode& world, const std::filesystem::path& path) const = 0;
  virtual kdl::result<void, IO::FileSystemError> doExportMap(
    WorldNode& world, const IO::ExportOptions& options) const = 0;

  virtual std::vector<Node*> doParseNodes(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    const std::vector<std::string>& linkedGroupsToKeep,
    Logger& logger) const = 0;
  virtual std::vector<BrushFace> doParseBrushFaces(
    const std::string& str,
    MapFormat mapFormat,
    const vm::bbox3& worldBounds,
    Logger& logger) const = 0;
  virtual void doWriteNodesToStream(
    WorldNode& world, const std::vector<Node*>& nodes, std::ostream& stream) const = 0;
  virtual void doWriteBrushFacesToStream(
    WorldNode& world,
    const std::vector<BrushFace>& faces,
    std::ostream& stream) const = 0;

  virtual void doLoadTextureCollections(Assets::TextureManager& textureManager) const = 0;
  virtual void doReloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) = 0;
  virtual void doReloadShaders() = 0;

  virtual bool doIsEntityDefinitionFile(const std::filesystem::path& path) const = 0;
  virtual std::vector<Assets::EntityDefinitionFileSpec> doAllEntityDefinitionFiles()
    const = 0;
  virtual Assets::EntityDefinitionFileSpec doExtractEntityDefinitionFile(
    const Entity& entity) const = 0;
  virtual std::filesystem::path doFindEntityDefinitionFile(
    const Assets::EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const = 0;

  virtual std::vector<std::string> doAvailableMods() const = 0;
  virtual std::vector<std::string> doExtractEnabledMods(const Entity& entity) const = 0;
  virtual std::string doDefaultMod() const = 0;

  virtual const FlagsConfig& doSurfaceFlags() const = 0;
  virtual const FlagsConfig& doContentFlags() const = 0;
  virtual const BrushFaceAttributes& doDefaultFaceAttribs() const = 0;

  virtual const std::vector<CompilationTool>& doCompilationTools() const = 0;
};
} // namespace Model
} // namespace TrenchBroom
