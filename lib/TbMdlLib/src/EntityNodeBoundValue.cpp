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

#include "mdl/EntityNodeBoundValue.h"

#include "el/BoundValue.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/NodeQueryValues.h"

namespace tb::mdl
{

namespace
{

struct EntityBinding
{
  const Entity& entity;
};

const el::BoundValueFields<EntityBinding>& entityBoundValueFields()
{
  static const auto fields = el::BoundValueFields<EntityBinding>{
    {"classname", [](const EntityBinding& b) { return el::Value{b.entity.classname()}; }},
    {"properties",
     [](const EntityBinding& b) { return makeEntityPropertiesBoundValue(b.entity); }},
  };
  return fields;
}

struct EntityNodeBinding
{
  const Map& map;
  const EntityNode& node;
};

const el::BoundValueFields<EntityNodeBinding>& entityNodeBoundValueFields()
{
  static const auto fields = el::BoundValueFields<EntityNodeBinding>{
    {"type", [](const EntityNodeBinding&) { return el::Value{"entity"}; }},
    {"name",
     [](const EntityNodeBinding& b) { return el::Value{b.node.entity().classname()}; }},
    {"classname",
     [](const EntityNodeBinding& b) { return el::Value{b.node.entity().classname()}; }},
    {"properties",
     [](const EntityNodeBinding& b) {
       return makeEntityPropertiesBoundValue(b.node.entity());
     }},
    {"entity",
     [](const EntityNodeBinding& b) { return makeEntityBoundValue(b.node.entity()); }},
    {"tags", [](const EntityNodeBinding& b) { return tagsValue(b.map, b.node); }},
    {"bounds",
     [](const EntityNodeBinding& b) { return el::Value{b.node.logicalBounds()}; }},
    {"center",
     [](const EntityNodeBinding& b) {
       return el::Value{b.node.logicalBounds().center()};
     }},
    {"visible", [](const EntityNodeBinding& b) { return el::Value{b.node.visible()}; }},
    {"locked", [](const EntityNodeBinding& b) { return el::Value{b.node.locked()}; }},
    {"selected", [](const EntityNodeBinding& b) { return el::Value{b.node.selected()}; }},
    {"layerName", [](const EntityNodeBinding& b) { return layerNameValue(b.node); }},
    {"groupName", [](const EntityNodeBinding& b) { return groupNameValue(b.node); }},
    {"linked",
     [](const EntityNodeBinding& b) { return el::Value{isLinked(b.map, b.node)}; }},
  };
  return fields;
}

} // namespace

std::optional<el::Value> entityPropertyValue(const Entity& entity, const std::string& key)
{
  const auto* value = entity.property(key);
  return value ? std::optional{el::Value{*value}} : std::nullopt;
}

std::vector<std::string> entityPropertyNames(const Entity& entity)
{
  return entity.propertyKeys();
}

el::Value makeEntityPropertiesBoundValue(const Entity& entity)
{
  return el::Value{el::BoundValue{
    .at = [&entity](const auto& key) { return entityPropertyValue(entity, key); },
    .keys = [&entity] { return entityPropertyNames(entity); },
  }};
}

el::Value makeEntityBoundValue(const Entity& entity)
{
  return el::makeBoundValue(EntityBinding{entity}, entityBoundValueFields());
}

el::BoundValue makeEntityNodeBoundValue(const Map& map, const EntityNode& node)
{
  return el::makeBoundValueOf(EntityNodeBinding{map, node}, entityNodeBoundValueFields());
}

} // namespace tb::mdl
