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
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionGroup.h"

#include <filesystem>
#include <string_view>
#include <vector>


namespace tb::io
{
class EntityDefinitionLoader;
class ParserStatus;
} // namespace tb::io

namespace tb::mdl
{
class EntityNodeBase;
enum class EntityDefinitionSortOrder;
enum class EntityDefinitionType;

class EntityDefinitionManager
{
private:
  std::vector<EntityDefinition> m_definitions;
  std::vector<EntityDefinitionGroup> m_groups;

public:
  ~EntityDefinitionManager();

  Result<void> loadDefinitions(
    const std::filesystem::path& path,
    const io::EntityDefinitionLoader& loader,
    io::ParserStatus& status);
  void setDefinitions(std::vector<EntityDefinition> newDefinitions);
  void clear();

  const EntityDefinition* definition(const mdl::EntityNodeBase* node) const;
  const EntityDefinition* definition(std::string_view classname) const;
  std::vector<const EntityDefinition*> definitions(
    EntityDefinitionType type, EntityDefinitionSortOrder order) const;
  const std::vector<EntityDefinition>& definitions() const;

  const std::vector<EntityDefinitionGroup>& groups() const;

private:
  void updateIndices();
  void updateGroups();
  void clearGroups();
};
} // namespace tb::mdl
