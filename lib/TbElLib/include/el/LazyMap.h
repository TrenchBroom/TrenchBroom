/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Value.h"

#include "kd/flat_map.h"
#include "kd/ranges/to.h"

#include <functional>
#include <ranges>
#include <string>

namespace tb::el
{

/**
 * A field-name -> accessor-function table for exposing a `T` as a `LazyMap`.
 */
template <typename T>
using LazyMapFields = kdl::flat_map<std::string, std::function<Value(const T&)>>;

/**
 * Builds a LazyMap that looks up keys against the given object via fields. A key absent
 * from fields is simply not in the map, so at() returns nullopt for it.
 */
template <typename T>
LazyMap makeLazyMapOf(T object, const LazyMapFields<T>& fields)
{
  return LazyMap{
    .at = [object, &fields](const auto& key) -> std::optional<Value> {
      const auto it = fields.find(key);
      return it != fields.end() ? std::optional{it->second(object)} : std::nullopt;
    },
    .keys =
      [&fields] { return fields | std::views::keys | kdl::ranges::to<std::vector>(); },
  };
}

/**
 * Same as `makeLazyMapOf`, but wraps the result in a Value.
 */
template <typename T>
Value makeLazyMap(T object, const LazyMapFields<T>& fields)
{
  return Value{makeLazyMapOf(object, fields)};
}

} // namespace tb::el
