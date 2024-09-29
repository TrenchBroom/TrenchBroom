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

#include "EntityNodeBase.h"

#include "Assets/EntityDefinition.h"
#include "Assets/PropertyDefinition.h"

#include "kdl/collection_utils.h"
#include "kdl/invoke.h"
#include "kdl/vector_utils.h"

#include <string>
#include <vector>

namespace TrenchBroom::Model
{

namespace
{
template <typename T, typename Eq = std::equal_to<T*>>
const auto* select(const std::vector<T*> ts, const Eq& eq = Eq{})
{
  if (ts.empty())
  {
    return static_cast<T*>(nullptr);
  }

  const auto* result = ts.front();
  for (auto it = std::next(ts.begin()); it != ts.end(); ++it)
  {
    const auto* t = *it;
    if (!eq(result, t))
    {
      return static_cast<T*>(nullptr);
    }
  }
  return result;
}

template <typename T>
void add_ref(std::vector<std::tuple<T*, int>>& vec, T* elem)
{
  for (auto& [existingElem, ref] : vec)
  {
    if (existingElem == elem)
    {
      ++ref;
      return;
    }
  }

  vec.emplace_back(elem, 1);
}

template <typename T>
void remove_ref(std::vector<std::tuple<T*, int>>& vec, T* elem)
{
  for (auto& [existingElem, ref] : vec)
  {
    if (existingElem == elem)
    {
      if (--ref == 0)
      {
        vec = kdl::vec_erase(std::move(vec), std::tuple{existingElem, ref});
      }

      return;
    }
  }
}
} // namespace

const Assets::EntityDefinition* selectEntityDefinition(
  const std::vector<EntityNodeBase*>& nodes)
{
  return select(kdl::vec_transform(
    nodes, [](const auto* node) { return node->entity().definition(); }));
}

const Assets::PropertyDefinition* propertyDefinition(
  const EntityNodeBase* node, const std::string& key)
{
  const auto* definition = node->entity().definition();
  return definition ? definition->propertyDefinition(key) : nullptr;
}

const Assets::PropertyDefinition* selectPropertyDefinition(
  const std::string& key, const std::vector<EntityNodeBase*>& nodes)
{
  return select(kdl::vec_transform(
    nodes, [&](const auto* node) { return propertyDefinition(node, key); }));
}

std::string selectPropertyValue(
  const std::string& key, const std::vector<EntityNodeBase*>& nodes)
{
  const auto* value = select(
    kdl::vec_transform(
      nodes, [&](const auto* node) { return node->entity().property(key); }),
    [](const auto* lhs, const auto* rhs) {
      return (lhs == nullptr && rhs == nullptr)
             || (lhs != nullptr && rhs != nullptr && *lhs == *rhs);
    });

  return value ? *value : "";
}

EntityNodeBase::EntityNodeBase(Entity entity)
  : m_entity{std::move(entity)}
{
}

EntityNodeBase::~EntityNodeBase() = default;

const Entity& EntityNodeBase::entity() const
{
  return m_entity;
}

Entity EntityNodeBase::setEntity(Entity entity)
{
  const auto notifyChange = NotifyPropertyChange{*this};

  auto oldEntity = std::exchange(m_entity, std::move(entity));
  m_entity.setDefinition(oldEntity.definition());
  updateIndexAndLinks(oldEntity.properties());
  return oldEntity;
}

void EntityNodeBase::setDefinition(Assets::EntityDefinition* definition)
{
  if (m_entity.definition() == definition)
  {
    return;
  }

  const auto notifyChange = NotifyPropertyChange{*this};
  m_entity.setDefinition(definition);

  //update links based on new entity definition
  removeAllLinks();
  addAllLinks();
}

EntityNodeBase::NotifyPropertyChange::NotifyPropertyChange(EntityNodeBase& node)
  : m_nodeChange{node}
, m_node{node}
, m_oldPhysicalBounds{node.physicalBounds()}
{
  m_node.propertiesWillChange();
}

EntityNodeBase::NotifyPropertyChange::~NotifyPropertyChange()
{
  m_node.propertiesDidChange(m_oldPhysicalBounds);
}

void EntityNodeBase::propertiesWillChange() {}

void EntityNodeBase::propertiesDidChange(const vm::bbox3& oldPhysicalBounds)
{
  doPropertiesDidChange(oldPhysicalBounds);
}

void EntityNodeBase::updateIndexAndLinks(const std::vector<EntityProperty>& oldProperties)
{
  const auto oldSorted = kdl::vec_sort(oldProperties);
  const auto newSorted = kdl::vec_sort(m_entity.properties());

  updatePropertyIndex(oldSorted, newSorted);
  updateLinks(oldSorted, newSorted);
}

void EntityNodeBase::updatePropertyIndex(
  const std::vector<EntityProperty>& oldProperties,
  const std::vector<EntityProperty>& newProperties)
{
  auto oldIt = std::begin(oldProperties);
  auto oldEnd = std::end(oldProperties);
  auto newIt = std::begin(newProperties);
  auto newEnd = std::end(newProperties);

  while (oldIt != oldEnd && newIt != newEnd)
  {
    const auto& oldProp = *oldIt;
    const auto& newProp = *newIt;

    if (oldProp < newProp)
    {
      removePropertyFromIndex(oldProp.key(), oldProp.value());
      ++oldIt;
    }
    else if (oldProp > newProp)
    {
      addPropertyToIndex(newProp.key(), newProp.value());
      ++newIt;
    }
    else
    {
      updatePropertyIndex(oldProp.key(), oldProp.value(), newProp.key(), newProp.value());
      ++oldIt;
      ++newIt;
    }
  }

  while (oldIt != oldEnd)
  {
    const auto& oldProp = *oldIt;
    removePropertyFromIndex(oldProp.key(), oldProp.value());
    ++oldIt;
  }

  while (newIt != newEnd)
  {
    const auto& newProp = *newIt;
    addPropertyToIndex(newProp.key(), newProp.value());
    ++newIt;
  }
}

void EntityNodeBase::updateLinks(
  const std::vector<EntityProperty>& oldProperties,
  const std::vector<EntityProperty>& newProperties)
{
  auto oldIt = std::begin(oldProperties);
  auto oldEnd = std::end(oldProperties);
  auto newIt = std::begin(newProperties);
  auto newEnd = std::end(newProperties);

  while (oldIt != oldEnd && newIt != newEnd)
  {
    const auto& oldProp = *oldIt;
    const auto& newProp = *newIt;

    if (oldProp.key() < newProp.key())
    {
      removeLinks(oldProp.key(), oldProp.value());
      ++oldIt;
    }
    else if (oldProp.key() > newProp.key())
    {
      addLinks(newProp.key(), newProp.value());
      ++newIt;
    }
    else
    {
      updateLinks(oldProp.key(), oldProp.value(), newProp.key(), newProp.value());
      ++oldIt;
      ++newIt;
    }
  }

  while (oldIt != oldEnd)
  {
    const auto& oldProp = *oldIt;
    removeLinks(oldProp.key(), oldProp.value());
    ++oldIt;
  }

  while (newIt != newEnd)
  {
    const auto& newProp = *newIt;
    addLinks(newProp.key(), newProp.value());
    ++newIt;
  }
}

void EntityNodeBase::addPropertiesToIndex()
{
  for (const auto& property : m_entity.properties())
  {
    addPropertyToIndex(property.key(), property.value());
  }
}

void EntityNodeBase::removePropertiesFromIndex()
{
  for (const auto& property : m_entity.properties())
  {
    removePropertyFromIndex(property.key(), property.value());
  }
}

void EntityNodeBase::addPropertyToIndex(const std::string& key, const std::string& value)
{
  addToIndex(this, key, value);
}

void EntityNodeBase::removePropertyFromIndex(
  const std::string& key, const std::string& value)
{
  removeFromIndex(this, key, value);
}

void EntityNodeBase::updatePropertyIndex(
  const std::string& oldKey,
  const std::string& oldValue,
  const std::string& newKey,
  const std::string& newValue)
{
  if (oldKey == newKey && oldValue == newValue)
  {
    return;
  }
  removeFromIndex(this, oldKey, oldValue);
  addToIndex(this, newKey, newValue);
}

void EntityNodeBase::getAllTargetSourcePropertyNames(std::vector<std::string>& result) const
{
  if (!entity().definition())
  {
    return;
  }

  auto targetSourceType = Assets::PropertyDefinitionType::TargetSourceProperty;
  result = kdl::vec_concat(result,
     kdl::vec_transform(
       kdl::vec_filter(entity().definition()->propertyDefinitions(),
          [&](const auto& definition) { return definition->type() == targetSourceType; }),
     [&](const auto& definition) { return definition->key(); }));
}

void EntityNodeBase::getAllTargetDestinationPropertyNames(std::vector<std::string>& result) const
{
  if (!entity().definition())
  {
    return;
  }

  auto targetDestType = Assets::PropertyDefinitionType::TargetDestinationProperty;
  result = kdl::vec_concat(result,
     kdl::vec_transform(
       kdl::vec_filter(entity().definition()->propertyDefinitions(),
          [&](const auto& definition) { return definition->type() == targetDestType; }),
     [&](const auto& definition) { return definition->key(); }));
}


std::vector<EntityNodeBase*> EntityNodeBase::linkSources() const
{
  return kdl::vec_transform(m_linkSources, [&](const auto& link) {
    return std::get<EntityNodeBase*>(link);
  });
}

std::vector<EntityNodeBase*> EntityNodeBase::linkTargets() const
{
  return kdl::vec_transform(m_linkTargets, [&](const auto& link) {
      return std::get<EntityNodeBase*>(link);
    });
}

vm::vec3 EntityNodeBase::linkSourceAnchor() const
{
  return doGetLinkSourceAnchor();
}

vm::vec3 EntityNodeBase::linkTargetAnchor() const
{
  return doGetLinkTargetAnchor();
}

bool EntityNodeBase::hasMissingSources() const
{
  bool hasTargetname = false;

  auto targetPropertyNames = std::vector<std::string>{};
  getAllTargetSourcePropertyNames(targetPropertyNames);

  for (const auto& sourceProperty : targetPropertyNames)
  {
    if (m_entity.hasProperty(sourceProperty))
    {
      hasTargetname = true;
      break;
    }
  }

  return m_linkTargets.empty() && hasTargetname;
}

std::vector<std::string> EntityNodeBase::findMissingLinkTargets() const
{
  auto result = std::vector<std::string>{};

  std::vector<std::string> linkTargetPropertyNames;
  getAllTargetDestinationPropertyNames(linkTargetPropertyNames);

  for (const auto& linkTargetPropertyName : linkTargetPropertyNames)
  {
    findMissingTargets(linkTargetPropertyName, result);
  }

  return result;
}


void EntityNodeBase::findMissingTargets(
  const std::string& prefix, std::vector<std::string>& result) const
{
  for (const auto& property : m_entity.numberedProperties(prefix))
  {
    const auto& targetname = property.value();
    if (targetname.empty())
    {
      result.push_back(property.key());
    }
    else
    {
      auto linkTargets = std::vector<EntityNodeBase*>{};
      findEntityNodesWithTargetSourceProperty(targetname, linkTargets);
      if (linkTargets.empty())
      {
        result.push_back(property.key());
      }
    }
  }
}

void EntityNodeBase::addLinks(const std::string& name, const std::string& value)
{
  std::vector<std::string> targetSourceNames;
  getAllTargetSourcePropertyNames(targetSourceNames);

  std::vector<std::string> targetDestinationNames;
  getAllTargetDestinationPropertyNames(targetDestinationNames);

  if (kdl::vec_contains(targetSourceNames, name))
  {
    addLinkSourcesIncludingNumbered(value);
  }
  else
  {
    for (const auto& targetDestinationName : targetDestinationNames)
    {
      if (isNumberedProperty(targetDestinationName, name))
      {
        addLinkTargets(value);
      }
    }
  }
}

void EntityNodeBase::removeLinks(const std::string& name, const std::string& value)
{
  std::vector<std::string> targetSourceNames;
  getAllTargetSourcePropertyNames(targetSourceNames);

  if (kdl::vec_contains(targetSourceNames, name))
  {
    removeAllLinkSources();
  }
  else
  {
    std::vector<std::string> linkTargetPropertyNames;
    getAllTargetDestinationPropertyNames(linkTargetPropertyNames);

    for (const auto& linkTargetPropertyName : linkTargetPropertyNames)
    {
      if (isNumberedProperty(linkTargetPropertyName, name))
      {
        removeLinkTargets(value);
      }
    }
  }
}

void EntityNodeBase::updateLinks(
  const std::string& oldName,
  const std::string& oldValue,
  const std::string& newName,
  const std::string& newValue)
{
  if (oldName == newName && oldValue == newValue)
  {
    return;
  }
  removeLinks(oldName, oldValue);
  addLinks(newName, newValue);
}

void EntityNodeBase::addLinkTargets(const std::string& targetname)
{
  if (!targetname.empty())
  {
    auto targets = std::vector<EntityNodeBase*>{};
    findEntityNodesWithTargetSourceProperty(targetname, targets);
    addLinkTargets(targets);
  }
}


void EntityNodeBase::removeLinkTargets(const std::string& targetname)
{
  if (targetname.empty())
  {
    return;
  }

  for (auto& [target, _] : m_linkTargets)
  {
    auto targetSourcePropertyNames = std::vector<std::string>{};
    target->getAllTargetSourcePropertyNames(targetSourcePropertyNames);

    for (const auto& targetSourcePropertyName : targetSourcePropertyNames)
    {
      const auto* targetTargetname = target->entity().property(targetSourcePropertyName);
      if (targetTargetname && *targetTargetname == targetname)
      {
        removeLinkTarget(target);
        target->removeLinkSource(this);
        break;
      }
    }
  }
}


void EntityNodeBase::addLinkSourcesIncludingNumbered(const std::string& targetname)
{
  if (!targetname.empty())
  {
    auto linkSources = std::vector<EntityNodeBase*>{};
    findEntityNodesWithTargetDestinationProperty(targetname, linkSources);
    addLinkSources(linkSources);
  }
}

void EntityNodeBase::addLinkTargetsIncludingNumbered(const std::string& propertyName)
{
  for (const auto& property : m_entity.numberedProperties(propertyName))
  {
    const auto& targetname = property.value();
    if (!targetname.empty())
    {
      auto linkTargets = std::vector<EntityNodeBase*>{};
      findEntityNodesWithTargetSourceProperty(targetname, linkTargets);
      addLinkTargets(linkTargets);
    }
  }
}



void EntityNodeBase::addLinkTargets(const std::vector<EntityNodeBase*>& targets)
{
  m_linkTargets.reserve(m_linkTargets.size() + targets.size());
  for (auto* target : targets)
  {
    target->addLinkSource(this);
    addLinkTarget(target);
  }
  invalidateIssues();
}

void EntityNodeBase::addLinkSources(const std::vector<EntityNodeBase*>& sources)
{
  m_linkSources.reserve(m_linkSources.size() + sources.size());
  for (auto* linkSource : sources)
  {
    linkSource->addLinkTarget(this);
    addLinkSource(linkSource);
  }
  invalidateIssues();
}

void EntityNodeBase::removeAllLinkSources()
{
  for (auto& [linkSource, ref] : m_linkSources)
  {
    linkSource->removeLinkTarget(this);
  }
  m_linkSources.clear();
  invalidateIssues();
}

void EntityNodeBase::removeAllLinkTargets()
{
  for (auto& [linkTarget, ref] : m_linkTargets)
  {
    linkTarget->removeLinkSource(this);
  }
  m_linkTargets.clear();
  invalidateIssues();
}

void EntityNodeBase::removeAllLinks()
{
  removeAllLinkSources();
  removeAllLinkTargets();
}

void EntityNodeBase::addAllLinks()
{
  std::vector<std::string> linkTargetPropertyNames;
  getAllTargetDestinationPropertyNames(linkTargetPropertyNames);

  for (const auto& linkTargetPropertyName : linkTargetPropertyNames)
  {
    addLinkTargetsIncludingNumbered(linkTargetPropertyName);
  }

  // get all targetnames
  std::vector<std::string> linkSourcePropertyNames;
  getAllTargetSourcePropertyNames(linkSourcePropertyNames);

  for (const auto& linkSourcePropertyName : linkSourcePropertyNames)
  {
    // foreach name we could be called by
    const auto* targetname = m_entity.property(linkSourcePropertyName);
    if (targetname && !targetname->empty())
    {
      addLinkSourcesIncludingNumbered(*targetname);
    }
  }
}

void EntityNodeBase::doAncestorWillChange()
{
  removeAllLinks();
  removePropertiesFromIndex();
}

void EntityNodeBase::doAncestorDidChange()
{
  addPropertiesToIndex();
  addAllLinks();
}

void EntityNodeBase::addLinkSource(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  add_ref(m_linkSources, node);
  invalidateIssues();
}

void EntityNodeBase::addLinkTarget(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  add_ref(m_linkTargets, node);
  invalidateIssues();
}

void EntityNodeBase::removeLinkSource(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  remove_ref(m_linkSources, node);
  invalidateIssues();
}

void EntityNodeBase::removeLinkTarget(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  remove_ref(m_linkTargets, node);
  invalidateIssues();
}
EntityNodeBase::EntityNodeBase() = default;

const std::string& EntityNodeBase::doGetName() const
{
  return m_entity.classname();
}

bool operator==(const EntityNodeBase& lhs, const EntityNodeBase& rhs)
{
  return lhs.entity() == rhs.entity();
}

bool operator!=(const EntityNodeBase& lhs, const EntityNodeBase& rhs)
{
  return !(lhs == rhs);
}

} // namespace TrenchBroom::Model
