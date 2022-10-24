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
  const size_t index = m_name.find_first_of('_');
  if (index == std::string::npos)
    return m_name;
  return m_name.substr(index + 1);
}

std::string EntityDefinition::groupName() const
{
  const size_t index = m_name.find_first_of('_');
  if (index == std::string::npos)
    return m_name;
  return m_name.substr(0, index);
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
  const size_t previous = m_usageCount--;
  assert(previous > 0);
  unused(previous);
}

const FlagsPropertyDefinition* EntityDefinition::spawnflags() const
{
  for (const auto& propertyDefinition : m_propertyDefinitions)
  {
    if (
      propertyDefinition->type() == PropertyDefinitionType::FlagsProperty
      && propertyDefinition->key() == Model::EntityPropertyKeys::Spawnflags)
    {
      return static_cast<FlagsPropertyDefinition*>(propertyDefinition.get());
    }
  }
  return nullptr;
}

const EntityDefinition::PropertyDefinitionList& EntityDefinition::propertyDefinitions()
  const
{
  return m_propertyDefinitions;
}

const PropertyDefinition* EntityDefinition::propertyDefinition(
  const std::string& propertyKey) const
{
  for (const auto& propertyDefinition : m_propertyDefinitions)
  {
    if (propertyDefinition->key() == propertyKey)
    {
      return propertyDefinition.get();
    }
  }
  return nullptr;
}

const PropertyDefinition* EntityDefinition::safeGetPropertyDefinition(
  const EntityDefinition* entityDefinition, const std::string& propertyKey)
{
  if (entityDefinition == nullptr)
    return nullptr;
  return entityDefinition->propertyDefinition(propertyKey);
}

const FlagsPropertyDefinition* EntityDefinition::safeGetFlagsPropertyDefinition(
  const EntityDefinition* entityDefinition, const std::string& propertyKey)
{
  if (entityDefinition == nullptr)
    return nullptr;
  const PropertyDefinition* propertyDefinition =
    entityDefinition->propertyDefinition(propertyKey);
  if (
    propertyDefinition == nullptr
    || propertyDefinition->type() != PropertyDefinitionType::FlagsProperty)
  {
    return nullptr;
  }
  return static_cast<const FlagsPropertyDefinition*>(propertyDefinition);
}

std::vector<EntityDefinition*> EntityDefinition::filterAndSort(
  const std::vector<EntityDefinition*>& definitions,
  const EntityDefinitionType type,
  const EntityDefinitionSortOrder order)
{
  std::vector<EntityDefinition*> result;
  for (const auto& definition : definitions)
  {
    if (definition->type() == type)
    {
      result.push_back(definition);
    }
  }

  if (order == EntityDefinitionSortOrder::Usage)
  {
    std::sort(
      std::begin(result),
      std::end(result),
      [](const EntityDefinition* lhs, const EntityDefinition* rhs) {
        if (lhs->usageCount() == rhs->usageCount())
        {
          return lhs->name() < rhs->name();
        }
        else
        {
          return lhs->usageCount() > rhs->usageCount();
        }
      });
  }
  else
  {
    std::sort(
      std::begin(result),
      std::end(result),
      [](const EntityDefinition* lhs, const EntityDefinition* rhs) {
        const int strCmp = kdl::ci::str_compare(lhs->name(), rhs->name());
        if (strCmp == 0)
        {
          return lhs->usageCount() > rhs->usageCount();
        }
        else
        {
          return strCmp < 0;
        }
      });
  }

  return result;
}

EntityDefinition::EntityDefinition(
  const std::string& name,
  const Color& color,
  const std::string& description,
  const PropertyDefinitionList& propertyDefinitions)
  : m_index(0)
  , m_name(name)
  , m_color(color)
  , m_description(description)
  , m_usageCount(0u)
  , m_propertyDefinitions(propertyDefinitions)
{
}

PointEntityDefinition::PointEntityDefinition(
  const std::string& name,
  const Color& color,
  const vm::bbox3& bounds,
  const std::string& description,
  const PropertyDefinitionList& propertyDefinitions,
  const ModelDefinition& modelDefinition)
  : EntityDefinition(name, color, description, propertyDefinitions)
  , m_bounds(bounds)
  , m_modelDefinition(modelDefinition)
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
  const std::string& name,
  const Color& color,
  const std::string& description,
  const PropertyDefinitionList& propertyDefinitions)
  : EntityDefinition(name, color, description, propertyDefinitions)
{
}

EntityDefinitionType BrushEntityDefinition::type() const
{
  return EntityDefinitionType::BrushEntity;
}
} // namespace Assets
} // namespace TrenchBroom
