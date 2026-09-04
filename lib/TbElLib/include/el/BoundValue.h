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

#include "kd/ranges/to.h"

#include <functional>
#include <map>
#include <ranges>
#include <string>

namespace tb::el
{

/**
 * A field-name -> accessor-function table for exposing a `T` as a `BoundValue`.
 */
template <typename T>
using BoundValueFields = std::map<std::string, std::function<Value(const T&)>>;

/**
 * Builds a `BoundValue` that looks up keys against `object` (captured by value --
 * expected to be either reference-sized or a small aggregate of references, never the
 * bound object itself) via `fields`. A key absent from `fields` is simply not in the map,
 * so `at()` returns nullopt for it.
 */
template <typename T>
BoundValue makeBoundValueOf(T object, const BoundValueFields<T>& fields)
{
  return BoundValue{
    .at = [object, &fields](const std::string& key) -> std::optional<Value> {
      const auto it = fields.find(key);
      return it != fields.end() ? std::optional{it->second(object)} : std::nullopt;
    },
    .keys =
      [&fields] { return fields | std::views::keys | kdl::ranges::to<std::vector>(); },
  };
}

/**
 * Same as `makeBoundValueOf`, wrapped in a `Value` -- for the common case of returning
 * one directly as a field's value (e.g. a lazy `entity` or `properties` field).
 */
template <typename T>
Value makeBoundValue(T object, const BoundValueFields<T>& fields)
{
  return Value{makeBoundValueOf(object, fields)};
}

} // namespace tb::el
