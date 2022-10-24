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

#include "EntityNodeIndex.h"

#include "Macros.h"
#include "Model/Entity.h"
#include "Model/EntityNodeBase.h"
#include "Model/EntityProperties.h"

#include <kdl/compact_trie.h>
#include <kdl/vector_utils.h>

#include <iterator>
#include <list>
#include <string>
#include <vector>

namespace TrenchBroom
{
namespace Model
{
EntityNodeIndexQuery EntityNodeIndexQuery::exact(const std::string& pattern)
{
  return EntityNodeIndexQuery(Type_Exact, pattern);
}

EntityNodeIndexQuery EntityNodeIndexQuery::prefix(const std::string& pattern)
{
  return EntityNodeIndexQuery(Type_Prefix, pattern);
}

EntityNodeIndexQuery EntityNodeIndexQuery::numbered(const std::string& pattern)
{
  return EntityNodeIndexQuery(Type_Numbered, pattern);
}

EntityNodeIndexQuery EntityNodeIndexQuery::any()
{
  return EntityNodeIndexQuery(Type_Any);
}

std::set<EntityNodeBase*> EntityNodeIndexQuery::execute(
  const EntityNodeStringIndex& index) const
{
  std::set<EntityNodeBase*> result;
  switch (m_type)
  {
  case Type_Exact:
    index.find_matches(m_pattern, std::inserter(result, std::end(result)));
    break;
  case Type_Prefix:
    index.find_matches(m_pattern + "*", std::inserter(result, std::end(result)));
    break;
  case Type_Numbered:
    index.find_matches(m_pattern + "%*", std::inserter(result, std::end(result)));
    break;
  case Type_Any:
    break;
    switchDefault();
  }
  return result;
}

bool EntityNodeIndexQuery::execute(
  const EntityNodeBase* node, const std::string& value) const
{
  switch (m_type)
  {
  case Type_Exact:
    return node->entity().hasProperty(m_pattern, value);
  case Type_Prefix:
    return node->entity().hasPropertyWithPrefix(m_pattern, value);
  case Type_Numbered:
    return node->entity().hasNumberedProperty(m_pattern, value);
  case Type_Any:
    return true;
    switchDefault();
  }
}

std::vector<Model::EntityProperty> EntityNodeIndexQuery::execute(
  const EntityNodeBase* node) const
{
  const auto& entity = node->entity();
  switch (m_type)
  {
  case Type_Exact:
    return entity.propertiesWithKey(m_pattern);
  case Type_Prefix:
    return entity.propertiesWithPrefix(m_pattern);
  case Type_Numbered:
    return entity.numberedProperties(m_pattern);
  case Type_Any:
    return entity.properties();
    switchDefault();
  }
}

EntityNodeIndexQuery::EntityNodeIndexQuery(const Type type, const std::string& pattern)
  : m_type(type)
  , m_pattern(pattern)
{
}

EntityNodeIndex::EntityNodeIndex()
  : m_keyIndex(std::make_unique<EntityNodeStringIndex>())
  , m_valueIndex(std::make_unique<EntityNodeStringIndex>())
{
}

EntityNodeIndex::~EntityNodeIndex() = default;

void EntityNodeIndex::addEntityNode(EntityNodeBase* node)
{
  for (const EntityProperty& property : node->entity().properties())
    addProperty(node, property.key(), property.value());
}

void EntityNodeIndex::removeEntityNode(EntityNodeBase* node)
{
  for (const EntityProperty& property : node->entity().properties())
    removeProperty(node, property.key(), property.value());
}

void EntityNodeIndex::addProperty(
  EntityNodeBase* node, const std::string& key, const std::string& value)
{
  m_keyIndex->insert(key, node);
  m_valueIndex->insert(value, node);
}

void EntityNodeIndex::removeProperty(
  EntityNodeBase* node, const std::string& key, const std::string& value)
{
  m_keyIndex->remove(key, node);
  m_valueIndex->remove(value, node);
}

std::vector<EntityNodeBase*> EntityNodeIndex::findEntityNodes(
  const EntityNodeIndexQuery& keyQuery, const std::string& value) const
{
  // first, find Nodes which have `value` as the value for any key
  std::vector<EntityNodeBase*> result;
  m_valueIndex->find_matches(value, std::back_inserter(result));
  if (result.empty())
  {
    return {};
  }

  result = kdl::vec_sort_and_remove_duplicates(std::move(result));

  // next, remove results from the result set that don't match `keyQuery`
  auto it = std::begin(result);
  while (it != std::end(result))
  {
    const EntityNodeBase* node = *it;
    if (!keyQuery.execute(node, value))
      it = result.erase(it);
    else
      ++it;
  }

  return result;
}

std::vector<std::string> EntityNodeIndex::allKeys() const
{
  std::vector<std::string> result;
  m_keyIndex->get_keys(std::back_inserter(result));
  return result;
}

std::vector<std::string> EntityNodeIndex::allValuesForKeys(
  const EntityNodeIndexQuery& keyQuery) const
{
  std::vector<std::string> result;

  const std::set<EntityNodeBase*> nameResult = keyQuery.execute(*m_keyIndex);
  for (const auto node : nameResult)
  {
    const auto matchingProperties = keyQuery.execute(node);
    for (const auto& property : matchingProperties)
    {
      result.push_back(property.value());
    }
  }

  return result;
}
} // namespace Model
} // namespace TrenchBroom
