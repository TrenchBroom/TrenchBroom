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

#include "Result.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>


namespace TrenchBroom::IO
{
class EntityDefinitionLoader;
class ParserStatus;
} // namespace TrenchBroom::IO

namespace TrenchBroom::Model
{
class EntityNodeBase;
}

namespace TrenchBroom::Assets
{
class EntityDefinition;
class EntityDefinitionGroup;
enum class EntityDefinitionSortOrder;
enum class EntityDefinitionType;

class EntityDefinitionManager
{
private:
  using Cache = std::map<std::string, EntityDefinition*>;
  std::vector<std::unique_ptr<EntityDefinition>> m_definitions;
  std::vector<EntityDefinitionGroup> m_groups;
  Cache m_cache;

public:
  ~EntityDefinitionManager();

  Result<void> loadDefinitions(
    const std::filesystem::path& path,
    const IO::EntityDefinitionLoader& loader,
    IO::ParserStatus& status);
  void setDefinitions(std::vector<std::unique_ptr<EntityDefinition>> newDefinitions);
  void clear();

  EntityDefinition* definition(const Model::EntityNodeBase* node) const;
  EntityDefinition* definition(const std::string& classname) const;
  std::vector<EntityDefinition*> definitions(
    EntityDefinitionType type, EntityDefinitionSortOrder order) const;
  std::vector<EntityDefinition*> definitions() const;

  const std::vector<EntityDefinitionGroup>& groups() const;

private:
  void updateIndices();
  void updateGroups();
  void updateCache();
  void clearCache();
  void clearGroups();
};
} // namespace TrenchBroom::Assets
