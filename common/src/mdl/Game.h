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

#include "Result.h"
#include "io/EntityDefinitionLoader.h"
#include "mdl/EntityDefinitionFileSpec.h"
#include "mdl/GameConfig.h"
#include "mdl/SoftMapBounds.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace tb
{
class Logger;

namespace io
{
class FileSystem;
}

namespace mdl
{
class BrushFace;
class BrushFaceAttributes;
class Entity;
class EntityNodeBase;
class MaterialManager;
class Node;
class SmartTag;
class WorldNode;
struct CompilationConfig;
struct EntityDefinitionFileSpec;
struct FlagsConfig;

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

  /**
   * Returns the soft map bounds specified in the given World entity, or if unset, the
   * value from softMapBounds()
   */
  virtual SoftMapBounds extractSoftMapBounds(const Entity& entity) const = 0;

public: // material collection handling
  virtual void reloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) = 0;

public: // entity definition handling
  virtual bool isEntityDefinitionFile(const std::filesystem::path& path) const = 0;
  virtual std::vector<EntityDefinitionFileSpec> allEntityDefinitionFiles() const = 0;
  virtual std::filesystem::path findEntityDefinitionFile(
    const EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const = 0;

public: // mods
  virtual Result<std::vector<std::string>> availableMods() const = 0;
  virtual std::string defaultMod() const = 0;
};

} // namespace mdl
} // namespace tb
