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

namespace TrenchBroom {
namespace Model {
namespace EntityPropertyKeys {
const std::string Classname = "classname";
const std::string Origin = "origin";
const std::string Wad = "wad";
const std::string Textures = "_tb_textures";
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
const std::string LinkedGroupId = "_tb_linked_group_id";
const std::string Message = "_tb_message";
const std::string ValveVersion = "mapversion";
const std::string SoftMapBounds = "_tb_soft_map_bounds";
} // namespace EntityPropertyKeys

namespace EntityPropertyValues {
const std::string WorldspawnClassname = "worldspawn";
const std::string NoClassname = "undefined";
const std::string LayerClassname = "func_group";
const std::string GroupClassname = "func_group";
const std::string GroupTypeLayer = "_tb_layer";
const std::string GroupTypeGroup = "_tb_group";
const std::string DefaultValue = "";
const std::string NoSoftMapBounds = "none";
const std::string LayerLockedValue = "1";
const std::string LayerHiddenValue = "1";
const std::string LayerOmitFromExportValue = "1";
} // namespace EntityPropertyValues

kdl_reflect_impl(EntityPropertyConfig);

bool isNumberedProperty(std::string_view prefix, std::string_view key) {
  // %* matches 0 or more digits
  const std::string pattern = std::string(prefix) + "%*";
  return kdl::cs::str_matches_glob(key, pattern);
}

EntityProperty::EntityProperty() = default;

EntityProperty::EntityProperty(const std::string& key, const std::string& value)
  : m_key(key)
  , m_value(value) {}

kdl_reflect_impl(EntityProperty);

const std::string& EntityProperty::key() const {
  return m_key;
}

const std::string& EntityProperty::value() const {
  return m_value;
}

bool EntityProperty::hasKey(std::string_view key) const {
  return kdl::cs::str_is_equal(m_key, key);
}

bool EntityProperty::hasValue(const std::string_view value) const {
  return kdl::cs::str_is_equal(m_value, value);
}

bool EntityProperty::hasKeyAndValue(std::string_view key, std::string_view value) const {
  return hasKey(key) && hasValue(value);
}

bool EntityProperty::hasPrefix(const std::string_view prefix) const {
  return kdl::cs::str_is_prefix(m_key, prefix);
}

bool EntityProperty::hasPrefixAndValue(
  const std::string_view prefix, const std::string_view value) const {
  return hasPrefix(prefix) && hasValue(value);
}

bool EntityProperty::hasNumberedPrefix(const std::string_view prefix) const {
  return isNumberedProperty(prefix, m_key);
}

bool EntityProperty::hasNumberedPrefixAndValue(
  const std::string_view prefix, const std::string_view value) const {
  return hasNumberedPrefix(prefix) && hasValue(value);
}

void EntityProperty::setKey(const std::string& key) {
  m_key = key;
}

void EntityProperty::setValue(const std::string& value) {
  m_value = value;
}

bool isLayer(const std::string& classname, const std::vector<EntityProperty>& properties) {
  if (classname != EntityPropertyValues::LayerClassname) {
    return false;
  } else {
    const std::string& groupType = findProperty(properties, EntityPropertyKeys::GroupType);
    return groupType == EntityPropertyValues::GroupTypeLayer;
  }
}

bool isGroup(const std::string& classname, const std::vector<EntityProperty>& properties) {
  if (classname != EntityPropertyValues::GroupClassname) {
    return false;
  } else {
    const std::string& groupType = findProperty(properties, EntityPropertyKeys::GroupType);
    return groupType == EntityPropertyValues::GroupTypeGroup;
  }
}

bool isWorldspawn(
  const std::string& classname, const std::vector<EntityProperty>& /* properties */) {
  return classname == EntityPropertyValues::WorldspawnClassname;
}

const std::string& findProperty(
  const std::vector<EntityProperty>& properties, const std::string& key,
  const std::string& defaultValue) {
  for (const EntityProperty& property : properties) {
    if (key == property.key()) {
      return property.value();
    }
  }
  return defaultValue;
}

// EntityProperties
EntityProperties::EntityProperties() = default;

EntityProperties::EntityProperties(std::vector<EntityProperty> properties)
  : m_properties(std::move(properties)) {}

std::vector<EntityProperty> EntityProperties::releaseProperties() {
  return std::move(m_properties);
}

const std::vector<EntityProperty>& EntityProperties::properties() const {
  return m_properties;
}

void EntityProperties::setProperties(const std::vector<EntityProperty>& properties) {
  m_properties.clear();

  // ensure that there are no duplicate keys
  kdl::vector_set<std::string> keys(properties.size());
  for (const auto& property : properties) {
    if (keys.insert(property.key()).second) {
      m_properties.push_back(property);
    }
  }
}

const EntityProperty& EntityProperties::addOrUpdateProperty(
  const std::string& key, const std::string& value) {
  auto it = findProperty(key);
  if (it != std::end(m_properties)) {
    it->setValue(value);
    return *it;
  } else {
    m_properties.push_back(EntityProperty(key, value));
    return m_properties.back();
  }
}

void EntityProperties::renameProperty(const std::string& key, const std::string& newKey) {
  if (!hasProperty(key)) {
    return;
  }

  const std::string value = *properties(key);
  removeProperty(key);
  addOrUpdateProperty(newKey, value);
}

void EntityProperties::removeProperty(const std::string& key) {
  auto it = findProperty(key);
  if (it != std::end(m_properties)) {
    m_properties.erase(it);
  }
}

bool EntityProperties::hasProperty(const std::string& key) const {
  return findProperty(key) != std::end(m_properties);
}

bool EntityProperties::hasProperty(const std::string& key, const std::string& value) const {
  for (const auto& property : m_properties) {
    if (property.hasKeyAndValue(key, value)) {
      return true;
    }
  }
  return false;
}

bool EntityProperties::hasPropertyWithPrefix(
  const std::string& prefix, const std::string& value) const {
  for (const auto& property : m_properties) {
    if (property.hasPrefixAndValue(prefix, value)) {
      return true;
    }
  }
  return false;
}

bool EntityProperties::hasNumberedProperty(
  const std::string& prefix, const std::string& value) const {
  for (const auto& property : m_properties) {
    if (property.hasNumberedPrefixAndValue(prefix, value)) {
      return true;
    }
  }
  return false;
}

std::vector<std::string> EntityProperties::keys() const {
  std::vector<std::string> result;
  result.reserve(m_properties.size());

  for (const EntityProperty& property : m_properties) {
    result.push_back(property.key());
  }
  return result;
}

const std::string* EntityProperties::properties(const std::string& key) const {
  auto it = findProperty(key);
  if (it == std::end(m_properties)) {
    return nullptr;
  } else {
    return &it->value();
  }
}

std::vector<EntityProperty> EntityProperties::propertiesWithKey(const std::string& key) const {
  std::vector<EntityProperty> result;
  for (const auto& property : m_properties) {
    if (property.hasKey(key)) {
      result.push_back(property);
    }
  }
  return result;
}

std::vector<EntityProperty> EntityProperties::propertiesWithPrefix(
  const std::string& prefix) const {
  std::vector<EntityProperty> result;
  for (const auto& property : m_properties) {
    if (property.hasPrefix(prefix)) {
      result.push_back(property);
    }
  }
  return result;
}

std::vector<EntityProperty> EntityProperties::numberedProperties(const std::string& prefix) const {
  std::vector<EntityProperty> result;
  for (const auto& property : m_properties) {
    if (property.hasNumberedPrefix(prefix)) {
      result.push_back(property);
    }
  }
  return result;
}

std::vector<EntityProperty>::const_iterator EntityProperties::findProperty(
  const std::string& key) const {
  for (auto it = std::begin(m_properties), end = std::end(m_properties); it != end; ++it) {
    if (it->hasKey(key)) {
      return it;
    }
  }
  return std::end(m_properties);
}

std::vector<EntityProperty>::iterator EntityProperties::findProperty(const std::string& key) {
  for (auto it = std::begin(m_properties), end = std::end(m_properties); it != end; ++it) {
    if (it->hasKey(key)) {
      return it;
    }
  }
  return std::end(m_properties);
}
} // namespace Model
} // namespace TrenchBroom
