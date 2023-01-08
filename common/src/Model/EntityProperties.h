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

#pragma once

#include "EL/Expression.h"

#include <kdl/reflection_decl.h>

#include <optional>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
namespace EntityPropertyKeys
{
extern const std::string Classname;
extern const std::string Origin;
extern const std::string Wad;
extern const std::string Textures;
extern const std::string Mods;
extern const std::string Spawnflags;
extern const std::string EntityDefinitions;
extern const std::string Angle;
extern const std::string Angles;
extern const std::string Mangle;
extern const std::string Target;
extern const std::string Targetname;
extern const std::string Killtarget;
extern const std::string ProtectedEntityProperties;
extern const std::string GroupType;
extern const std::string LayerId;
extern const std::string LayerName;
extern const std::string LayerSortIndex;
extern const std::string LayerColor;
extern const std::string LayerLocked;
extern const std::string LayerHidden;
extern const std::string LayerOmitFromExport;
extern const std::string Layer;
extern const std::string GroupId;
extern const std::string GroupName;
extern const std::string Group;
extern const std::string GroupTransformation;
extern const std::string LinkedGroupId;
extern const std::string Message;
extern const std::string ValveVersion;
extern const std::string SoftMapBounds;
} // namespace EntityPropertyKeys

namespace EntityPropertyValues
{
extern const std::string WorldspawnClassname;
extern const std::string NoClassname;
extern const std::string LayerClassname;
extern const std::string GroupClassname;
extern const std::string GroupTypeLayer;
extern const std::string GroupTypeGroup;
extern const std::string DefaultValue;
extern const std::string NoSoftMapBounds;
extern const std::string LayerLockedValue;
extern const std::string LayerHiddenValue;
extern const std::string LayerOmitFromExportValue;
} // namespace EntityPropertyValues

struct EntityPropertyConfig
{
  std::optional<EL::Expression> defaultModelScaleExpression;
  bool setDefaultProperties{false};
  bool updateAnglePropertyAfterTransform{true};

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

} // namespace Model
} // namespace TrenchBroom
