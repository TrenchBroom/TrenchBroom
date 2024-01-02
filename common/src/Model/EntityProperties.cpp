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

#include "EntityProperties.h"

#include "Assets/EntityDefinition.h"

#include <kdl/reflection_impl.h>
#include <kdl/string_compare.h>
#include <kdl/vector_set.h>

#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
namespace EntityPropertyKeys
{
const std::string Classname = "classname";
const std::string Origin = "origin";
const std::string Wad = "wad";
const std::string Mods = "_tb_mod";
const std::string Spawnflags = "spawnflags";
const std::string EntityDefinitions = "_tb_def";
const std::string Angle = "angle";
const std::string Angles = "angles";
const std::string Mangle = "mangle";
const std::string Target = "target";
const std::string Targetname = "targetname";
const std::string Killtarget = "killtarget";
const std::string ProtectedEntityProperties = "_tb_protected_properties";
const std::string GroupType = "_tb_type";
const std::string LayerId = "_tb_id";
const std::string LayerName = "_tb_name";
const std::string LayerSortIndex = "_tb_layer_sort_index";
const std::string LayerColor = "_tb_layer_color";
const std::string LayerLocked = "_tb_layer_locked";
const std::string LayerHidden = "_tb_layer_hidden";
const std::string LayerOmitFromExport = "_tb_layer_omit_from_export";
const std::string Layer = "_tb_layer";
const std::string GroupId = "_tb_id";
const std::string GroupName = "_tb_name";
const std::string Group = "_tb_group";
const std::string GroupTransformation = "_tb_transformation";
const std::string LinkId = "_tb_linked_group_id";
const std::string Message = "_tb_message";
const std::string ValveVersion = "mapversion";
const std::string SoftMapBounds = "_tb_soft_map_bounds";
} // namespace EntityPropertyKeys

namespace EntityPropertyValues
{
const std::string WorldspawnClassname = "worldspawn";
const std::string NoClassname = "undefined";
const std::string LayerClassname = "func_group";
const std::string GroupClassname = "func_group";
const std::string GroupTypeLayer = "_tb_layer";
const std::string GroupTypeGroup = "_tb_group";
const std::string DefaultValue;
const std::string NoSoftMapBounds = "none";
const std::string LayerLockedValue = "1";
const std::string LayerHiddenValue = "1";
const std::string LayerOmitFromExportValue = "1";
} // namespace EntityPropertyValues

kdl_reflect_impl(EntityPropertyConfig);

bool isNumberedProperty(std::string_view prefix, std::string_view key)
{
  // %* matches 0 or more digits
  const std::string pattern = std::string(prefix) + "%*";
  return kdl::cs::str_matches_glob(key, pattern);
}

EntityProperty::EntityProperty() = default;

EntityProperty::EntityProperty(std::string key, std::string value)
  : m_key{std::move(key)}
  , m_value{std::move(value)}
{
}

kdl_reflect_impl(EntityProperty);

const std::string& EntityProperty::key() const
{
  return m_key;
}

const std::string& EntityProperty::value() const
{
  return m_value;
}

bool EntityProperty::hasKey(std::string_view key) const
{
  return kdl::cs::str_is_equal(m_key, key);
}

bool EntityProperty::hasValue(const std::string_view value) const
{
  return kdl::cs::str_is_equal(m_value, value);
}

bool EntityProperty::hasKeyAndValue(std::string_view key, std::string_view value) const
{
  return hasKey(key) && hasValue(value);
}

bool EntityProperty::hasPrefix(const std::string_view prefix) const
{
  return kdl::cs::str_is_prefix(m_key, prefix);
}

bool EntityProperty::hasPrefixAndValue(
  const std::string_view prefix, const std::string_view value) const
{
  return hasPrefix(prefix) && hasValue(value);
}

bool EntityProperty::hasNumberedPrefix(const std::string_view prefix) const
{
  return isNumberedProperty(prefix, m_key);
}

bool EntityProperty::hasNumberedPrefixAndValue(
  const std::string_view prefix, const std::string_view value) const
{
  return hasNumberedPrefix(prefix) && hasValue(value);
}

void EntityProperty::setKey(std::string key)
{
  m_key = std::move(key);
}

void EntityProperty::setValue(std::string value)
{
  m_value = std::move(value);
}

bool isLayer(const std::string& classname, const std::vector<EntityProperty>& properties)
{
  if (classname != EntityPropertyValues::LayerClassname)
  {
    return false;
  }
  else
  {
    const std::string& groupType =
      findEntityPropertyOrDefault(properties, EntityPropertyKeys::GroupType);
    return groupType == EntityPropertyValues::GroupTypeLayer;
  }
}

bool isGroup(const std::string& classname, const std::vector<EntityProperty>& properties)
{
  if (classname != EntityPropertyValues::GroupClassname)
  {
    return false;
  }
  else
  {
    const std::string& groupType =
      findEntityPropertyOrDefault(properties, EntityPropertyKeys::GroupType);
    return groupType == EntityPropertyValues::GroupTypeGroup;
  }
}

bool isWorldspawn(const std::string& classname)
{
  return classname == EntityPropertyValues::WorldspawnClassname;
}

std::vector<EntityProperty>::const_iterator findEntityProperty(
  const std::vector<EntityProperty>& properties, const std::string& key)
{
  return std::find_if(
    std::begin(properties), std::end(properties), [&](const auto& property) {
      return property.hasKey(key);
    });
}

std::vector<EntityProperty>::iterator findEntityProperty(
  std::vector<EntityProperty>& properties, const std::string& key)
{
  return std::find_if(
    std::begin(properties), std::end(properties), [&](const auto& property) {
      return property.hasKey(key);
    });
}

const std::string& findEntityPropertyOrDefault(
  const std::vector<EntityProperty>& properties,
  const std::string& key,
  const std::string& defaultValue)
{
  const auto it = findEntityProperty(properties, key);
  return it != std::end(properties) ? it->value() : defaultValue;
}

} // namespace Model
} // namespace TrenchBroom
