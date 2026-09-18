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

#include <optional>
#include <string>
#include <vector>

namespace tb
{
namespace mdl
{
class Entity;
class EntityNode;
class Map;
} // namespace mdl

namespace ql
{

std::optional<el::Value> entityPropertyValue(
  const mdl::Entity& entity, const std::string& key);

std::vector<std::string> entityPropertyNames(const mdl::Entity& entity);

el::Value makeEntityPropertiesLazyMap(const mdl::Entity& entity);

/**
 * A lazy map exposing an entity's data.
 */
el::Value makeEntityLazyMap(const mdl::Entity& entity);

/**
 * A lazy map exposing an entity node's data.
 */
el::LazyMap makeEntityNodeLazyMap(const mdl::Map& map, const mdl::EntityNode& node);

/**
 * The field names makeEntityNodeLazyMap's result exposes.
 */
std::vector<std::string> entityNodeFieldNames();

} // namespace ql
} // namespace tb
