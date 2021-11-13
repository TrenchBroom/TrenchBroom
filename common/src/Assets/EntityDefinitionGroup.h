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

#include <string>
#include <vector>

namespace TrenchBroom {
namespace Assets {
class EntityDefinition;
enum class EntityDefinitionSortOrder;
enum class EntityDefinitionType;

class EntityDefinitionGroup {
private:
  std::string m_name;
  std::vector<EntityDefinition*> m_definitions;

public:
  EntityDefinitionGroup(const std::string& name, std::vector<EntityDefinition*> definitions);

  size_t index() const;
  const std::string& name() const;
  const std::string displayName() const;
  const std::vector<EntityDefinition*>& definitions() const;
  std::vector<EntityDefinition*> definitions(
    EntityDefinitionType type, EntityDefinitionSortOrder order) const;
};
} // namespace Assets
} // namespace TrenchBroom
