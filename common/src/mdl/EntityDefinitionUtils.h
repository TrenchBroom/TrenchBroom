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

#include "Macros.h"
#include "mdl/EntityDefinition.h"

#include "kdl/ranges/to.h"

#include <algorithm>
#include <ranges>
#include <vector>

namespace tb::mdl
{

class Map;

enum class EntityDefinitionSortOrder
{
  Name,
  Usage,
};

template <std::ranges::range R>
std::vector<const EntityDefinition*> filterAndSort(
  R&& entityDefinitions, EntityDefinitionType type, EntityDefinitionSortOrder order)
{
  auto result = entityDefinitions | std::views::filter([&](const auto* entityDefinition) {
                  return getType(*entityDefinition) == type;
                })
                | kdl::ranges::to<std::vector>();

  std::ranges::sort(result, [&](const auto* lhs, const auto* rhs) {
    switch (order)
    {
    case EntityDefinitionSortOrder::Name:
      return lhs->name < rhs->name;
    case EntityDefinitionSortOrder::Usage:
      return lhs->usageCount() < rhs->usageCount();
      switchDefault();
    }
  });
  return result;
}

std::vector<const PropertyDefinition*> getLinkSourcePropertyDefinitions(
  const EntityDefinition* entityDefinition);

std::vector<const PropertyDefinition*> getLinkTargetPropertyDefinitions(
  const EntityDefinition* entityDefinition);

bool isLinkSourceProperty(
  const EntityDefinition* entityDefinition, const std::string& key);

bool isLinkTargetProperty(
  const EntityDefinition* entityDefinition, const std::string& key);

/**
 * Overrides the entity property values types of the given entity definitions.
 *
 * If the given vector contains no entity definition that uses the TargetSource or
 * TargetDestination property value types, we switch to legacy mode to support the default
 * entity linking properties.
 */
void addOrSetDefaultEntityLinkProperties(
  std::vector<EntityDefinition>& entityDefinitions);

/**
 * Overrides the entity property values types of the given entity definitions.
 *
 * For any point entity definition, change its "origin" property definition to type
 * Origin, or add an "origin" property of that type if it is missing.
 */
void addOrConvertOriginProperties(std::vector<EntityDefinition>& entityDefinitions);

} // namespace tb::mdl
