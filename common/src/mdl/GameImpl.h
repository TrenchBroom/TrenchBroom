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
#include "mdl/Game.h"
#include "mdl/GameFileSystem.h"

#include <filesystem>
#include <string>
#include <vector>

namespace tb
{
class Logger;
} // namespace tb

namespace tb::mdl
{
struct EntityPropertyConfig;

class GameImpl : public Game
{
private:
  GameConfig m_config;
  GameFileSystem m_fs;
  std::filesystem::path m_gamePath;
  std::vector<std::filesystem::path> m_additionalSearchPaths;

public:
  GameImpl(GameConfig config, std::filesystem::path gamePath, Logger& logger);

public: // implement EntityDefinitionLoader interface:
  Result<std::vector<EntityDefinition>> loadEntityDefinitions(
    io::ParserStatus& status, const std::filesystem::path& path) const override;

public: // implement Game interface
  const GameConfig& config() const override;
  const io::FileSystem& gameFileSystem() const override;

  std::filesystem::path gamePath() const override;

  void setGamePath(const std::filesystem::path& gamePath, Logger& logger) override;
  void setAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths, Logger& logger) override;
  PathErrors checkAdditionalSearchPaths(
    const std::vector<std::filesystem::path>& searchPaths) const override;

  SoftMapBounds extractSoftMapBounds(const Entity& entity) const override;

  void reloadWads(
    const std::filesystem::path& documentPath,
    const std::vector<std::filesystem::path>& wadPaths,
    Logger& logger) override;

  bool isEntityDefinitionFile(const std::filesystem::path& path) const override;
  std::vector<EntityDefinitionFileSpec> allEntityDefinitionFiles() const override;
  EntityDefinitionFileSpec extractEntityDefinitionFile(
    const Entity& entity) const override;
  EntityDefinitionFileSpec defaultEntityDefinitionFile() const;
  std::filesystem::path findEntityDefinitionFile(
    const EntityDefinitionFileSpec& spec,
    const std::vector<std::filesystem::path>& searchPaths) const override;

  Result<std::vector<std::string>> availableMods() const override;
  std::vector<std::string> extractEnabledMods(const Entity& entity) const override;
  std::string defaultMod() const override;

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

} // namespace tb::mdl
