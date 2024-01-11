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

#include "kdl/invoke.h"
#include <kdl/collection_utils.h>
#include <kdl/vector_utils.h>

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
    [](const auto* lhs, const auto* rhs) { return *lhs == *rhs; });

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
  m_entity.setDefinition(entityPropertyConfig(), definition);
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

    if (oldProp < newProp)
    {
      removeLinks(oldProp.key(), oldProp.value());
      ++oldIt;
    }
    else if (oldProp > newProp)
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

const std::vector<EntityNodeBase*>& EntityNodeBase::linkSources() const
{
  return m_linkSources;
}

const std::vector<EntityNodeBase*>& EntityNodeBase::linkTargets() const
{
  return m_linkTargets;
}

const std::vector<EntityNodeBase*>& EntityNodeBase::killSources() const
{
  return m_killSources;
}

const std::vector<EntityNodeBase*>& EntityNodeBase::killTargets() const
{
  return m_killTargets;
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
  return m_linkSources.empty() && m_killSources.empty()
         && m_entity.hasProperty(EntityPropertyKeys::Targetname);
}

std::vector<std::string> EntityNodeBase::findMissingLinkTargets() const
{
  auto result = std::vector<std::string>{};
  findMissingTargets(EntityPropertyKeys::Target, result);
  return result;
}

std::vector<std::string> EntityNodeBase::findMissingKillTargets() const
{
  auto result = std::vector<std::string>{};
  findMissingTargets(EntityPropertyKeys::Killtarget, result);
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
      findEntityNodesWithProperty(
        EntityPropertyKeys::Targetname, targetname, linkTargets);
      if (linkTargets.empty())
      {
        result.push_back(property.key());
      }
    }
  }
}

void EntityNodeBase::addLinks(const std::string& name, const std::string& value)
{
  if (isNumberedProperty(EntityPropertyKeys::Target, name))
  {
    addLinkTargets(value);
  }
  else if (isNumberedProperty(EntityPropertyKeys::Killtarget, name))
  {
    addKillTargets(value);
  }
  else if (name == EntityPropertyKeys::Targetname)
  {
    addAllLinkSources(value);
    addAllKillSources(value);
  }
}

void EntityNodeBase::removeLinks(const std::string& name, const std::string& value)
{
  if (isNumberedProperty(EntityPropertyKeys::Target, name))
  {
    removeLinkTargets(value);
  }
  else if (isNumberedProperty(EntityPropertyKeys::Killtarget, name))
  {
    removeKillTargets(value);
  }
  else if (name == EntityPropertyKeys::Targetname)
  {
    removeAllLinkSources();
    removeAllKillSources();
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
    findEntityNodesWithProperty(EntityPropertyKeys::Targetname, targetname, targets);
    addLinkTargets(targets);
  }
}

void EntityNodeBase::addKillTargets(const std::string& targetname)
{
  if (!targetname.empty())
  {
    auto targets = std::vector<EntityNodeBase*>{};
    findEntityNodesWithProperty(EntityPropertyKeys::Targetname, targetname, targets);
    addKillTargets(targets);
  }
}

void EntityNodeBase::removeLinkTargets(const std::string& targetname)
{
  if (!targetname.empty())
  {
    auto rem = std::end(m_linkTargets);
    auto it = std::begin(m_linkTargets);
    while (it != rem)
    {
      auto* target = *it;
      const auto* targetTargetname =
        target->entity().property(EntityPropertyKeys::Targetname);
      if (targetTargetname && *targetTargetname == targetname)
      {
        target->removeLinkSource(this);
        --rem;
        std::iter_swap(it, rem);
      }
      else
      {
        ++it;
      }
    }
    m_linkTargets.erase(rem, std::end(m_linkTargets));
  }
}

void EntityNodeBase::removeKillTargets(const std::string& targetname)
{
  if (!targetname.empty())
  {
    auto rem = std::end(m_killTargets);
    auto it = std::begin(m_killTargets);
    while (it != rem)
    {
      auto* target = *it;
      const auto* targetTargetname =
        target->entity().property(EntityPropertyKeys::Targetname);
      if (targetTargetname && *targetTargetname == targetname)
      {
        target->removeKillSource(this);
        --rem;
        std::iter_swap(it, rem);
      }
      else
      {
        ++it;
      }
    }
    m_killTargets.erase(rem, std::end(m_killTargets));
  }
}

void EntityNodeBase::addAllLinkSources(const std::string& targetname)
{
  if (!targetname.empty())
  {
    auto linkSources = std::vector<EntityNodeBase*>{};
    findEntityNodesWithNumberedProperty(
      EntityPropertyKeys::Target, targetname, linkSources);
    addLinkSources(linkSources);
  }
}

void EntityNodeBase::addAllLinkTargets()
{
  for (const auto& property : m_entity.numberedProperties(EntityPropertyKeys::Target))
  {
    const auto& targetname = property.value();
    if (!targetname.empty())
    {
      auto linkTargets = std::vector<EntityNodeBase*>{};
      findEntityNodesWithProperty(
        EntityPropertyKeys::Targetname, targetname, linkTargets);
      addLinkTargets(linkTargets);
    }
  }
}

void EntityNodeBase::addAllKillSources(const std::string& targetname)
{
  if (!targetname.empty())
  {
    auto killSources = std::vector<EntityNodeBase*>{};
    findEntityNodesWithNumberedProperty(
      EntityPropertyKeys::Killtarget, targetname, killSources);
    addKillSources(killSources);
  }
}

void EntityNodeBase::addAllKillTargets()
{
  for (const auto& property : m_entity.numberedProperties(EntityPropertyKeys::Killtarget))
  {
    const std::string& targetname = property.value();
    if (!targetname.empty())
    {
      auto killTargets = std::vector<EntityNodeBase*>{};
      findEntityNodesWithProperty(
        EntityPropertyKeys::Targetname, targetname, killTargets);
      addKillTargets(killTargets);
    }
  }
}

void EntityNodeBase::addLinkTargets(const std::vector<EntityNodeBase*>& targets)
{
  m_linkTargets.reserve(m_linkTargets.size() + targets.size());
  for (auto* target : targets)
  {
    target->addLinkSource(this);
    m_linkTargets.push_back(target);
  }
  invalidateIssues();
}

void EntityNodeBase::addKillTargets(const std::vector<EntityNodeBase*>& targets)
{
  m_killTargets.reserve(m_killTargets.size() + targets.size());
  for (auto* target : targets)
  {
    target->addKillSource(this);
    m_killTargets.push_back(target);
  }
  invalidateIssues();
}

void EntityNodeBase::addLinkSources(const std::vector<EntityNodeBase*>& sources)
{
  m_linkSources.reserve(m_linkSources.size() + sources.size());
  for (auto* linkSource : sources)
  {
    linkSource->addLinkTarget(this);
    m_linkSources.push_back(linkSource);
  }
  invalidateIssues();
}

void EntityNodeBase::addKillSources(const std::vector<EntityNodeBase*>& sources)
{
  m_killSources.reserve(m_killSources.size() + sources.size());
  for (auto* killSource : sources)
  {
    killSource->addKillTarget(this);
    m_killSources.push_back(killSource);
  }
  invalidateIssues();
}

void EntityNodeBase::removeAllLinkSources()
{
  for (auto* linkSource : m_linkSources)
  {
    linkSource->removeLinkTarget(this);
  }
  m_linkSources.clear();
  invalidateIssues();
}

void EntityNodeBase::removeAllLinkTargets()
{
  for (auto* linkTarget : m_linkTargets)
  {
    linkTarget->removeLinkSource(this);
  }
  m_linkTargets.clear();
  invalidateIssues();
}

void EntityNodeBase::removeAllKillSources()
{
  for (auto* killSource : m_killSources)
  {
    killSource->removeKillTarget(this);
  }
  m_killSources.clear();
  invalidateIssues();
}

void EntityNodeBase::removeAllKillTargets()
{
  for (auto* killTarget : m_killTargets)
  {
    killTarget->removeKillSource(this);
  }
  m_killTargets.clear();
  invalidateIssues();
}

void EntityNodeBase::removeAllLinks()
{
  removeAllLinkSources();
  removeAllLinkTargets();
  removeAllKillSources();
  removeAllKillTargets();
}

void EntityNodeBase::addAllLinks()
{
  addAllLinkTargets();
  addAllKillTargets();

  const auto* targetname = m_entity.property(EntityPropertyKeys::Targetname);
  if (targetname && !targetname->empty())
  {
    addAllLinkSources(*targetname);
    addAllKillSources(*targetname);
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
  m_linkSources.push_back(node);
  invalidateIssues();
}

void EntityNodeBase::addLinkTarget(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  m_linkTargets.push_back(node);
  invalidateIssues();
}

void EntityNodeBase::addKillSource(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  m_killSources.push_back(node);
  invalidateIssues();
}

void EntityNodeBase::addKillTarget(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  m_killTargets.push_back(node);
  invalidateIssues();
}

void EntityNodeBase::removeLinkSource(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  m_linkSources = kdl::vec_erase(std::move(m_linkSources), node);
  invalidateIssues();
}

void EntityNodeBase::removeLinkTarget(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  m_linkTargets = kdl::vec_erase(std::move(m_linkTargets), node);
  invalidateIssues();
}

void EntityNodeBase::removeKillSource(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  m_killSources = kdl::vec_erase(std::move(m_killSources), node);
  invalidateIssues();
}

EntityNodeBase::EntityNodeBase() = default;

const std::string& EntityNodeBase::doGetName() const
{
  return m_entity.classname();
}

void EntityNodeBase::removeKillTarget(EntityNodeBase* node)
{
  ensure(node, "node is not null");
  m_killTargets = kdl::vec_erase(std::move(m_killTargets), node);
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
