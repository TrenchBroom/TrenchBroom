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

#include "ql/EntityNodeLazyMap.h"

#include "el/LazyMap.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "ql/NodeQueryValues.h"

#include "kd/ranges/to.h"

#include <ranges>

namespace tb::ql
{

namespace
{

const el::LazyMapFields<mdl::Entity>& entityLazyMapFields()
{
  static const auto fields = el::LazyMapFields<mdl::Entity>{
    {"classname", [](const auto& b) { return el::Value{b.classname()}; }},
    {"properties", [](const auto& b) { return makeEntityPropertiesLazyMap(b); }},
  };
  return fields;
}

struct LazyEntityNode
{
  const mdl::Map& map;
  const mdl::EntityNode& node;
};

const el::LazyMapFields<LazyEntityNode>& entityNodeLazyMapFields()
{
  static const auto fields = el::LazyMapFields<LazyEntityNode>{
    {"type", [](const auto&) { return el::Value{"entity"}; }},
    {"classname", [](const auto& l) { return el::Value{l.node.entity().classname()}; }},
    {"properties",
     [](const auto& l) { return makeEntityPropertiesLazyMap(l.node.entity()); }},
    {"tags", [](const auto& l) { return tagsValue(l.map, l.node); }},
    {"bounds", [](const auto& l) { return el::Value{l.node.logicalBounds()}; }},
    {"center", [](const auto& l) { return el::Value{l.node.logicalBounds().center()}; }},
    {"visible", [](const auto& l) { return el::Value{l.node.visible()}; }},
    {"locked", [](const auto& l) { return el::Value{l.node.locked()}; }},
    {"selected", [](const auto& l) { return el::Value{l.node.selected()}; }},
    {"layerName", [](const auto& l) { return layerNameValue(l.node); }},
    {"groupName", [](const auto& l) { return groupNameValue(l.node); }},
  };
  return fields;
}

} // namespace

std::optional<el::Value> entityPropertyValue(
  const mdl::Entity& entity, const std::string& key)
{
  const auto* value = entity.property(key);
  return value ? std::optional{el::Value{*value}} : std::nullopt;
}

std::vector<std::string> entityPropertyNames(const mdl::Entity& entity)
{
  return entity.propertyKeys();
}

el::Value makeEntityPropertiesLazyMap(const mdl::Entity& entity)
{
  return el::Value{el::LazyMap{
    .at = [&entity](const auto& key) { return entityPropertyValue(entity, key); },
    .keys = [&entity] { return entityPropertyNames(entity); },
  }};
}

el::Value makeEntityLazyMap(const mdl::Entity& entity)
{
  return el::makeLazyMap(entity, entityLazyMapFields());
}

el::LazyMap makeEntityNodeLazyMap(const mdl::Map& map, const mdl::EntityNode& node)
{
  return el::makeLazyMapOf(LazyEntityNode{map, node}, entityNodeLazyMapFields());
}

std::vector<std::string> entityNodeFieldNames()
{
  return entityNodeLazyMapFields() | std::views::keys | kdl::ranges::to<std::vector>();
}

} // namespace tb::ql
