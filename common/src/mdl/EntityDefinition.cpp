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

#include "EntityDefinition.h"

#include "kdl/reflection_impl.h"

#include "vm/bbox_io.h" // IWYU pragma: keep

#include <algorithm>

namespace tb::mdl
{

kdl_reflect_impl(PointEntityDefinition);

size_t EntityDefinition::usageCount() const
{
  return *m_usageCount;
}

void EntityDefinition::incUsageCount() const
{
  ++*m_usageCount;
}

void EntityDefinition::decUsageCount() const
{
  --*m_usageCount;
}

kdl_reflect_impl(EntityDefinition);

const PropertyDefinition* getPropertyDefinition(
  const EntityDefinition& entityDefinition, const std::string& key)
{
  if (const auto it = std::ranges::find_if(
        entityDefinition.propertyDefinitions,
        [&](const PropertyDefinition& property) { return property.key == key; });
      it != entityDefinition.propertyDefinitions.end())
  {
    return &*it;
  }
  return nullptr;
}

const PropertyDefinition* getPropertyDefinition(
  const EntityDefinition* entityDefinition, const std::string& key)
{
  return entityDefinition ? getPropertyDefinition(*entityDefinition, key) : nullptr;
}

std::string_view getShortName(const EntityDefinition& entityDefinition)
{
  const auto index = entityDefinition.name.find_first_of('_');
  return index == std::string::npos
           ? std::string_view{entityDefinition.name}
           : std::string_view{entityDefinition.name}.substr(index + 1);
}

std::string_view getGroupName(const EntityDefinition& entityDefinition)
{
  const auto index = entityDefinition.name.find_first_of('_');
  return index == std::string::npos
           ? std::string_view{entityDefinition.name}
           : std::string_view{entityDefinition.name}.substr(0, index);
}

EntityDefinitionType getType(const EntityDefinition& entityDefinition)
{
  return entityDefinition.pointEntityDefinition ? EntityDefinitionType::Point
                                                : EntityDefinitionType::Brush;
}

const PointEntityDefinition* getPointEntityDefinition(
  const EntityDefinition* entityDefinition)
{
  return entityDefinition && entityDefinition->pointEntityDefinition
           ? &*entityDefinition->pointEntityDefinition
           : nullptr;
}

} // namespace tb::mdl
