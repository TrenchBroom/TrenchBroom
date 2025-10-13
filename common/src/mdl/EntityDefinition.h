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

#include "Color.h"
#include "mdl/DecalDefinition.h"
#include "mdl/ModelDefinition.h"
#include "mdl/PropertyDefinition.h"

#include "kdl/reflection_decl.h"

#include "vm/bbox.h"

#include <atomic>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tb::mdl
{

struct PointEntityDefinition
{
  vm::bbox3d bounds;
  ModelDefinition modelDefinition;
  DecalDefinition decalDefinition;

  kdl_reflect_decl(PointEntityDefinition, bounds, modelDefinition, decalDefinition);
};

struct EntityDefinition
{
  std::string name;
  Color color;
  std::string description;
  std::vector<PropertyDefinition> propertyDefinitions;
  std::optional<PointEntityDefinition> pointEntityDefinition = std::nullopt;
  size_t index = 0;

  size_t usageCount() const;
  void incUsageCount() const;
  void decUsageCount() const;

  kdl_reflect_decl(
    EntityDefinition,
    index,
    name,
    color,
    description,
    propertyDefinitions,
    pointEntityDefinition);

  // Use a shared pointer to enable copying
  std::shared_ptr<std::atomic<size_t>> m_usageCount =
    std::make_shared<std::atomic<size_t>>(0);
};

PropertyDefinition* getPropertyDefinition(
  EntityDefinition& entityDefinition, const std::string& key);

const PropertyDefinition* getPropertyDefinition(
  const EntityDefinition& entityDefinition, const std::string& key);

const PropertyDefinition* getPropertyDefinition(
  const EntityDefinition* entityDefinition, const std::string& key);

std::vector<const PropertyDefinition*> getLinkSourcePropertyDefinitions(
  const EntityDefinition* entityDefinition);

std::vector<const PropertyDefinition*> getLinkTargetPropertyDefinitions(
  const EntityDefinition* entityDefinition);

enum class EntityDefinitionType
{
  Point,
  Brush,
};

std::string_view getShortName(const EntityDefinition& entityDefinition);

std::string_view getGroupName(const EntityDefinition& entityDefinition);

EntityDefinitionType getType(const EntityDefinition& entityDefinition);

const PointEntityDefinition* getPointEntityDefinition(
  const EntityDefinition* entityDefinition);

} // namespace tb::mdl
