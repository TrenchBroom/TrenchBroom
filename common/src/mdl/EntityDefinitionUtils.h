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

} // namespace tb::mdl
