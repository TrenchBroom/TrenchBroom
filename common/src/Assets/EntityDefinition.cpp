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

#include "EntityDefinition.h"

#include "Assets/PropertyDefinition.h"
#include "Macros.h"
#include "Model/EntityProperties.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Assets
{
EntityDefinition::~EntityDefinition() {}

size_t EntityDefinition::index() const
{
  return m_index;
}

void EntityDefinition::setIndex(const size_t index)
{
  m_index = index;
}

const std::string& EntityDefinition::name() const
{
  return m_name;
}

std::string EntityDefinition::shortName() const
{
  const auto index = m_name.find_first_of('_');
  return index == std::string::npos ? m_name : m_name.substr(index + 1);
}

std::string EntityDefinition::groupName() const
{
  const auto index = m_name.find_first_of('_');
  return index == std::string::npos ? m_name : m_name.substr(0, index);
}

const Color& EntityDefinition::color() const
{
  return m_color;
}

const std::string& EntityDefinition::description() const
{
  return m_description;
}

size_t EntityDefinition::usageCount() const
{
  return static_cast<size_t>(m_usageCount);
}

void EntityDefinition::incUsageCount()
{
  ++m_usageCount;
}

void EntityDefinition::decUsageCount()
{
  assertResult((m_usageCount--) > 0);
}

const FlagsPropertyDefinition* EntityDefinition::spawnflags() const
{
  return safeGetFlagsPropertyDefinition(this, Model::EntityPropertyKeys::Spawnflags);
}

const std::vector<std::shared_ptr<PropertyDefinition>>& EntityDefinition::
  propertyDefinitions() const
{
  return m_propertyDefinitions;
}

const PropertyDefinition* EntityDefinition::propertyDefinition(
  const std::string& propertyKey) const
{
  return safeGetPropertyDefinition(this, propertyKey);
}

const PropertyDefinition* EntityDefinition::safeGetPropertyDefinition(
  const EntityDefinition* entityDefinition, const std::string& propertyKey)
{
  if (entityDefinition == nullptr)
  {
    return nullptr;
  }

  const auto& propertyDefinitions = entityDefinition->propertyDefinitions();
  const auto it = std::find_if(
    propertyDefinitions.begin(),
    propertyDefinitions.end(),
    [&](const auto& propertyDefinition) {
      return propertyDefinition->key() == propertyKey;
    });
  return it != propertyDefinitions.end() ? it->get() : nullptr;
}

const FlagsPropertyDefinition* EntityDefinition::safeGetFlagsPropertyDefinition(
  const EntityDefinition* entityDefinition, const std::string& propertyKey)
{
  if (entityDefinition == nullptr)
  {
    return nullptr;
  }

  const auto& propertyDefinitions = entityDefinition->propertyDefinitions();
  const auto it = std::find_if(
    propertyDefinitions.begin(),
    propertyDefinitions.end(),
    [&](const auto& propertyDefinition) {
      return propertyDefinition->type() == PropertyDefinitionType::FlagsProperty
             && propertyDefinition->key() == propertyKey;
    });
  return it != propertyDefinitions.end()
           ? static_cast<FlagsPropertyDefinition*>(it->get())
           : nullptr;
}

std::vector<EntityDefinition*> EntityDefinition::filterAndSort(
  const std::vector<EntityDefinition*>& definitions,
  const EntityDefinitionType type,
  const EntityDefinitionSortOrder order)
{
  auto result = kdl::vec_filter(
    definitions, [&](const auto& definition) { return definition->type() == type; });

  const auto compareNames = [](const auto& lhs, const auto& rhs) {
    return kdl::ci::str_compare(lhs->name(), rhs->name()) < 0;
  };

  const auto compareUsageCount = [](const auto& lhs, const auto& rhs) {
    return lhs->usageCount() > rhs->usageCount();
  };

  if (order == EntityDefinitionSortOrder::Usage)
  {
    return kdl::vec_sort(
      std::move(result), kdl::combine_cmp(compareUsageCount, compareNames));
  }
  return kdl::vec_sort(
    std::move(result), kdl::combine_cmp(compareNames, compareUsageCount));
}

EntityDefinition::EntityDefinition(
  std::string name,
  const Color& color,
  std::string description,
  std::vector<std::shared_ptr<PropertyDefinition>> propertyDefinitions)
  : m_index{0}
  , m_name{std::move(name)}
  , m_color{color}
  , m_description{std::move(description)}
  , m_usageCount{0}
  , m_propertyDefinitions{std::move(propertyDefinitions)}
{
}

PointEntityDefinition::PointEntityDefinition(
  std::string name,
  const Color& color,
  const vm::bbox3& bounds,
  std::string description,
  std::vector<std::shared_ptr<PropertyDefinition>> propertyDefinitions,
  ModelDefinition modelDefinition)
  : EntityDefinition{std::move(name), color, std::move(description), std::move(propertyDefinitions)}
  , m_bounds{bounds}
  , m_modelDefinition{std::move(modelDefinition)}
{
}

EntityDefinitionType PointEntityDefinition::type() const
{
  return EntityDefinitionType::PointEntity;
}

const vm::bbox3& PointEntityDefinition::bounds() const
{
  return m_bounds;
}

const ModelDefinition& PointEntityDefinition::modelDefinition() const
{
  return m_modelDefinition;
}

BrushEntityDefinition::BrushEntityDefinition(
  std::string name,
  const Color& color,
  std::string description,
  std::vector<std::shared_ptr<PropertyDefinition>> propertyDefinitions)
  : EntityDefinition{
    std::move(name), color, std::move(description), std::move(propertyDefinitions)}
{
}

EntityDefinitionType BrushEntityDefinition::type() const
{
  return EntityDefinitionType::BrushEntity;
}
} // namespace Assets
} // namespace TrenchBroom
