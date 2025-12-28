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

#include "el/Expression.h"

#include "kd/reflection_decl.h"

#include <optional>
#include <string>
#include <vector>

namespace tb::mdl
{
namespace EntityPropertyKeys
{
inline const std::string Classname = "classname";
inline const std::string Origin = "origin";
inline const std::string Wad = "wad";
inline const std::string Mods = "_tb_mod";
inline const std::string EnabledMaterialCollections = "_tb_textures";
inline const std::string Spawnflags = "spawnflags";
inline const std::string EntityDefinitions = "_tb_def";
inline const std::string Angle = "angle";
inline const std::string Angles = "angles";
inline const std::string Mangle = "mangle";
inline const std::string Target = "target";
inline const std::string Targetname = "targetname";
inline const std::string Killtarget = "killtarget";
inline const std::string ProtectedEntityProperties = "_tb_protected_properties";
inline const std::string GroupType = "_tb_type";
inline const std::string LayerId = "_tb_id";
inline const std::string LayerName = "_tb_name";
inline const std::string LayerSortIndex = "_tb_layer_sort_index";
inline const std::string LayerColor = "_tb_layer_color";
inline const std::string LayerLocked = "_tb_layer_locked";
inline const std::string LayerHidden = "_tb_layer_hidden";
inline const std::string LayerOmitFromExport = "_tb_layer_omit_from_export";
inline const std::string Layer = "_tb_layer";
inline const std::string GroupId = "_tb_id";
inline const std::string GroupName = "_tb_name";
inline const std::string Group = "_tb_group";
inline const std::string GroupTransformation = "_tb_transformation";
inline const std::string LinkId = "_tb_linked_group_id";
inline const std::string Message = "_tb_message";
inline const std::string ValveVersion = "mapversion";
inline const std::string SoftMapBounds = "_tb_soft_map_bounds";
} // namespace EntityPropertyKeys

namespace EntityPropertyValues
{
inline const std::string WorldspawnClassname = "worldspawn";
inline const std::string NoClassname = "undefined";
inline const std::string LayerClassname = "func_group";
inline const std::string GroupClassname = "func_group";
inline const std::string GroupTypeLayer = "_tb_layer";
inline const std::string GroupTypeGroup = "_tb_group";
inline const std::string DefaultValue = std::string{};
inline const std::string NoSoftMapBounds = "none";
inline const std::string LayerLockedValue = "1";
inline const std::string LayerHiddenValue = "1";
inline const std::string LayerOmitFromExportValue = "1";
} // namespace EntityPropertyValues

struct EntityPropertyConfig
{
  std::optional<el::ExpressionNode> defaultModelScaleExpression;
  bool setDefaultProperties = false;
  bool updateAnglePropertyAfterTransform = true;

  kdl_reflect_decl(
    EntityPropertyConfig,
    defaultModelScaleExpression,
    setDefaultProperties,
    updateAnglePropertyAfterTransform);
};

bool isNumberedProperty(std::string_view prefix, std::string_view key);

class EntityProperty
{
private:
  std::string m_key;
  std::string m_value;

public:
  EntityProperty();
  EntityProperty(std::string key, std::string value);

  kdl_reflect_decl(EntityProperty, m_key, m_value);

  const std::string& key() const;
  const std::string& value() const;

  bool hasKey(std::string_view key) const;
  bool hasValue(std::string_view value) const;
  bool hasKeyAndValue(std::string_view key, std::string_view value) const;
  bool hasPrefix(std::string_view prefix) const;
  bool hasPrefixAndValue(std::string_view prefix, std::string_view value) const;
  bool hasNumberedPrefix(std::string_view prefix) const;
  bool hasNumberedPrefixAndValue(std::string_view prefix, std::string_view value) const;

  void setKey(std::string key);
  void setValue(std::string value);
};

bool isLayer(const std::string& classname, const std::vector<EntityProperty>& properties);
bool isGroup(const std::string& classname, const std::vector<EntityProperty>& properties);
bool isWorldspawn(const std::string& classname);

std::vector<EntityProperty>::const_iterator findEntityProperty(
  const std::vector<EntityProperty>& properties, const std::string& key);
std::vector<EntityProperty>::iterator findEntityProperty(
  std::vector<EntityProperty>& properties, const std::string& key);

const std::string& findEntityPropertyOrDefault(
  const std::vector<EntityProperty>& properties,
  const std::string& key,
  const std::string& defaultValue = EntityPropertyValues::DefaultValue);

} // namespace tb::mdl
