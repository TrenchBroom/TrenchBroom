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

#include "EntityDefinitionGroup.h"

#include "Assets/EntityDefinition.h"

#include "kdl/string_format.h"

#include <string>

namespace TrenchBroom::Assets
{

EntityDefinitionGroup::EntityDefinitionGroup(
  std::string name, std::vector<EntityDefinition*> definitions)
  : m_name{std::move(name)}
  , m_definitions{std::move(definitions)}
{
}

const std::string& EntityDefinitionGroup::name() const
{
  return m_name;
}

const std::string EntityDefinitionGroup::displayName() const
{
  return !m_name.empty() ? kdl::str_capitalize(m_name) : "Misc";
}

const std::vector<EntityDefinition*>& EntityDefinitionGroup::definitions() const
{
  return m_definitions;
}

std::vector<EntityDefinition*> EntityDefinitionGroup::definitions(
  const EntityDefinitionType type, const EntityDefinitionSortOrder order) const
{
  return EntityDefinition::filterAndSort(m_definitions, type, order);
}

} // namespace TrenchBroom::Assets
