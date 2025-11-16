/*
 Copyright (C) 2025 Kristian Duske

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

#include "mdl/Map_Entities.h"

#include "Ensure.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/Entity.h"
#include "mdl/EntityColorPropertyValue.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kdl/string_utils.h"

namespace tb::mdl
{
namespace
{

Result<std::string> convertEntityColor(
  const std::string_view str, const ColorRange::Type colorRange)
{
  return Color::parse(str) | kdl::transform([&](const auto& color) {
           return toColorRange(color, colorRange).toString();
         });
}

/**
 * Search the given linked groups for an entity node at the given node path, and return
 * its unprotected value for the given property key.
 */
std::optional<std::string> findUnprotectedPropertyValue(
  const std::string& key, const std::vector<EntityNodeBase*>& linkedEntities)
{
  for (const auto* entityNode : linkedEntities)
  {
    if (!kdl::vec_contains(entityNode->entity().protectedProperties(), key))
    {
      if (const auto* value = entityNode->entity().property(key))
      {
        return *value;
      }
    }
  }
  return std::nullopt;
}

/**
 * Find the unprotected property value of the given key in the corresponding linked
 * nodes of the given entity nodes. This value is used to restore the original value
 * when a property is set from protected to unprotected.
 */
std::optional<std::string> findUnprotectedPropertyValue(
  const std::string& key, const EntityNodeBase& entityNode, WorldNode& worldNode)
{
  const auto linkedNodes = collectLinkedNodes({&worldNode}, entityNode);
  if (linkedNodes.size() > 1)
  {
    if (const auto value = findUnprotectedPropertyValue(key, linkedNodes))
    {
      return value;
    }
  }

  return std::nullopt;
}

} // namespace

EntityNode* createPointEntity(
  Map& map, const EntityDefinition& definition, const vm::vec3d& delta)
{
  ensure(
    getType(definition) == EntityDefinitionType::Point,
    "definition is a point entity definition");

  auto entity = Entity{{{EntityPropertyKeys::Classname, definition.name}}};

  if (map.world()->entityPropertyConfig().setDefaultProperties)
  {
    mdl::setDefaultProperties(definition, entity, SetDefaultPropertyMode::SetAll);
  }

  auto* entityNode = new EntityNode{std::move(entity)};

  auto transaction = Transaction{map, "Create " + definition.name};
  deselectAll(map);
  if (addNodes(map, {{parentForNodes(map), {entityNode}}}).empty())
  {
    transaction.cancel();
    return nullptr;
  }
  selectNodes(map, {entityNode});
  if (!transformSelection(map, "Translate Objects", vm::translation_matrix(delta)))
  {
    transaction.cancel();
    return nullptr;
  }

  if (!transaction.commit())
  {
    return nullptr;
  }

  return entityNode;
}

EntityNode* createBrushEntity(Map& map, const EntityDefinition& definition)
{
  ensure(
    getType(definition) == EntityDefinitionType::Brush,
    "definition is a brush entity definition");

  const auto brushes = map.selection().brushes;
  assert(!brushes.empty());

  // if all brushes belong to the same entity, and that entity is not worldspawn, copy
  // its properties
  auto entity =
    (brushes.front()->entity() != map.world()
     && std::all_of(
       std::next(brushes.begin()),
       brushes.end(),
       [&](const auto* brush) { return brush->entity() == brushes.front()->entity(); }))
      ? brushes.front()->entity()->entity()
      : Entity{};

  entity.addOrUpdateProperty(EntityPropertyKeys::Classname, definition.name);

  if (map.world()->entityPropertyConfig().setDefaultProperties)
  {
    mdl::setDefaultProperties(definition, entity, SetDefaultPropertyMode::SetAll);
  }

  auto* entityNode = new EntityNode{std::move(entity)};

  const auto nodes = kdl::vec_static_cast<Node*>(brushes);

  auto transaction = Transaction{map, "Create " + definition.name};
  deselectAll(map);
  if (addNodes(map, {{parentForNodes(map), {entityNode}}}).empty())
  {
    transaction.cancel();
    return nullptr;
  }
  if (!reparentNodes(map, {{entityNode, nodes}}))
  {
    transaction.cancel();
    return nullptr;
  }
  selectNodes(map, nodes);

  if (!transaction.commit())
  {
    return nullptr;
  }

  return entityNode;
}

bool setEntityProperty(
  Map& map,
  const std::string& key,
  const std::string& value,
  const bool defaultToProtected)
{
  const auto entityNodes = map.selection().allEntities();
  return applyAndSwap(
    map,
    "Set Property",
    entityNodes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [&](Entity& entity) {
        entity.addOrUpdateProperty(key, value, defaultToProtected);
        return true;
      },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

bool renameEntityProperty(Map& map, const std::string& oldKey, const std::string& newKey)
{
  const auto entityNodes = map.selection().allEntities();
  return applyAndSwap(
    map,
    "Rename Property",
    entityNodes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [&](Entity& entity) {
        entity.renameProperty(oldKey, newKey);
        return true;
      },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

bool removeEntityProperty(Map& map, const std::string& key)
{
  const auto entityNodes = map.selection().allEntities();
  return applyAndSwap(
    map,
    "Remove Property",
    entityNodes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [&](Entity& entity) {
        entity.removeProperty(key);
        return true;
      },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

bool setEntityColorProperty(Map& map, const std::string& key, const Rgb& newColor)
{
  const auto entityNodes = map.selection().allEntities();
  return applyAndSwap(
    map,
    "Set Color",
    entityNodes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [&](Entity& entity) {
        const auto* oldValue = entity.property(key);
        const auto extraComponents =
          oldValue
            ? parseEntityColorPropertyValue(entity.definition(), key, *oldValue)
                | kdl::transform(
                  [](const auto& oldColor) { return oldColor.extraComponents; })
                | kdl::transform_error([](const auto&) { return std::vector<float>{}; })
                | kdl::value()
            : std::vector<float>{};

        return entityColorPropertyToString(
                 entity.definition(), key, {newColor, extraComponents})
               | kdl::transform([&](const auto& newValueStr) {
                   entity.addOrUpdateProperty(key, newValueStr);
                 })
               | kdl::is_success();
      },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

bool convertEntityColorRange(Map& map, const std::string& key, ColorRange::Type range)
{
  const auto entityNodes = map.selection().allEntities();
  return applyAndSwap(
    map,
    "Convert Color",
    entityNodes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [&](Entity& entity) {
        if (const auto* oldValue = entity.property(key))
        {
          return convertEntityColor(*oldValue, range)
                 | kdl::transform(
                   [&](const auto& value) { entity.addOrUpdateProperty(key, value); })
                 | kdl::is_success();
        }
        return true;
      },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

bool updateEntitySpawnflag(
  Map& map, const std::string& key, const size_t flagIndex, const bool setFlag)
{
  const auto entityNodes = map.selection().allEntities();
  return applyAndSwap(
    map,
    setFlag ? "Set Spawnflag" : "Unset Spawnflag",
    entityNodes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [&](Entity& entity) {
        const auto* strValue = entity.property(key);
        int intValue = strValue ? kdl::str_to_int(*strValue).value_or(0) : 0;
        const int flagValue = (1 << flagIndex);

        intValue = setFlag ? intValue | flagValue : intValue & ~flagValue;
        entity.addOrUpdateProperty(key, kdl::str_to_string(intValue));

        return true;
      },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

bool setProtectedEntityProperty(Map& map, const std::string& key, const bool value)
{
  const auto entityNodes = map.selection().allEntities();

  auto nodesToUpdate = std::vector<std::pair<Node*, NodeContents>>{};
  for (auto* entityNode : entityNodes)
  {
    auto entity = entityNode->entity();
    auto protectedProperties = entity.protectedProperties();
    if (value && !kdl::vec_contains(protectedProperties, key))
    {
      protectedProperties.push_back(key);
    }
    else if (!value && kdl::vec_contains(protectedProperties, key))
    {
      if (
        const auto newValue =
          findUnprotectedPropertyValue(key, *entityNode, *map.world()))
      {
        entity.addOrUpdateProperty(key, *newValue);
      }

      protectedProperties = kdl::vec_erase(std::move(protectedProperties), key);
    }
    entity.setProtectedProperties(std::move(protectedProperties));
    nodesToUpdate.emplace_back(entityNode, std::move(entity));
  }

  return updateNodeContents(
    map,
    "Set Protected Property",
    nodesToUpdate,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)));
}

bool clearProtectedEntityProperties(Map& map)
{
  const auto entityNodes = map.selection().allEntities();

  auto nodesToUpdate = std::vector<std::pair<Node*, NodeContents>>{};
  for (auto* entityNode : entityNodes)
  {
    if (entityNode->entity().protectedProperties().empty())
    {
      continue;
    }

    const auto linkedEntities = collectLinkedNodes({map.world()}, *entityNode);
    if (linkedEntities.size() <= 1)
    {
      continue;
    }

    auto entity = entityNode->entity();
    for (const auto& key : entity.protectedProperties())
    {
      if (const auto newValue = findUnprotectedPropertyValue(key, linkedEntities))
      {
        entity.addOrUpdateProperty(key, *newValue);
      }
    }

    entity.setProtectedProperties({});
    nodesToUpdate.emplace_back(entityNode, std::move(entity));
  }

  return updateNodeContents(
    map,
    "Clear Protected Properties",
    nodesToUpdate,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)));
}

bool canClearProtectedEntityProperties(const Map& map)
{
  const auto entityNodes = map.selection().allEntities();
  if (
    entityNodes.empty()
    || (entityNodes.size() == 1u && entityNodes.front() == map.world()))
  {
    return false;
  }

  return canUpdateLinkedGroups(kdl::vec_static_cast<Node*>(entityNodes));
}

void setDefaultEntityProperties(Map& map, const SetDefaultPropertyMode mode)
{
  const auto entityNodes = map.selection().allEntities();
  applyAndSwap(
    map,
    "Reset Default Properties",
    entityNodes,
    collectContainingGroups(kdl::vec_static_cast<Node*>(entityNodes)),
    kdl::overload(
      [](Layer&) { return true; },
      [](Group&) { return true; },
      [&](Entity& entity) {
        if (const auto* definition = entity.definition())
        {
          mdl::setDefaultProperties(*definition, entity, mode);
        }
        return true;
      },
      [](Brush&) { return true; },
      [](BezierPatch&) { return true; }));
}

} // namespace tb::mdl
