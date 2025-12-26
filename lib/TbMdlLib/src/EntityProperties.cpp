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

#include "mdl/EntityProperties.h"

#include "kd/reflection_impl.h"
#include "kd/string_compare.h"

#include <algorithm>
#include <string>
#include <vector>

namespace tb::mdl
{
namespace EntityPropertyKeys
{
extern const std::string Classname = "classname";
extern const std::string Origin = "origin";
extern const std::string Wad = "wad";
extern const std::string Mods = "_tb_mod";
extern const std::string EnabledMaterialCollections = "_tb_textures";
extern const std::string Spawnflags = "spawnflags";
extern const std::string EntityDefinitions = "_tb_def";
extern const std::string Angle = "angle";
extern const std::string Angles = "angles";
extern const std::string Mangle = "mangle";
extern const std::string Target = "target";
extern const std::string Targetname = "targetname";
extern const std::string Killtarget = "killtarget";
extern const std::string ProtectedEntityProperties = "_tb_protected_properties";
extern const std::string GroupType = "_tb_type";
extern const std::string LayerId = "_tb_id";
extern const std::string LayerName = "_tb_name";
extern const std::string LayerSortIndex = "_tb_layer_sort_index";
extern const std::string LayerColor = "_tb_layer_color";
extern const std::string LayerLocked = "_tb_layer_locked";
extern const std::string LayerHidden = "_tb_layer_hidden";
extern const std::string LayerOmitFromExport = "_tb_layer_omit_from_export";
extern const std::string Layer = "_tb_layer";
extern const std::string GroupId = "_tb_id";
extern const std::string GroupName = "_tb_name";
extern const std::string Group = "_tb_group";
extern const std::string GroupTransformation = "_tb_transformation";
extern const std::string LinkId = "_tb_linked_group_id";
extern const std::string Message = "_tb_message";
extern const std::string ValveVersion = "mapversion";
extern const std::string SoftMapBounds = "_tb_soft_map_bounds";
} // namespace EntityPropertyKeys

namespace EntityPropertyValues
{
extern const std::string WorldspawnClassname = "worldspawn";
extern const std::string NoClassname = "undefined";
extern const std::string LayerClassname = "func_group";
extern const std::string GroupClassname = "func_group";
extern const std::string GroupTypeLayer = "_tb_layer";
extern const std::string GroupTypeGroup = "_tb_group";
extern const std::string DefaultValue = std::string{};
extern const std::string NoSoftMapBounds = "none";
extern const std::string LayerLockedValue = "1";
extern const std::string LayerHiddenValue = "1";
extern const std::string LayerOmitFromExportValue = "1";
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
  return std::ranges::find_if(
    properties, [&](const auto& property) { return property.hasKey(key); });
}

std::vector<EntityProperty>::iterator findEntityProperty(
  std::vector<EntityProperty>& properties, const std::string& key)
{
  return std::ranges::find_if(
    properties, [&](const auto& property) { return property.hasKey(key); });
}

const std::string& findEntityPropertyOrDefault(
  const std::vector<EntityProperty>& properties,
  const std::string& key,
  const std::string& defaultValue)
{
  const auto it = findEntityProperty(properties, key);
  return it != std::end(properties) ? it->value() : defaultValue;
}

} // namespace tb::mdl
