/*
 Copyright (C) 2025 Kristian Duske

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

#include "EntityLinkManager.h"

#include "mdl/EntityNodeBase.h"
#include "mdl/NodeIndex.h"

#include "kdl/ranges/to.h"

#include <ranges>

namespace tb::mdl
{
namespace
{
const EntityLinkManager::LinkEndsForName EmptyLinkEnds;
}

EntityLinkManager::EntityLinkManager(const NodeIndex& nodeIndex)
  : m_nodeIndex{nodeIndex}
{
}

const EntityLinkManager::LinkEndsForName& EntityLinkManager::linksFrom(
  const EntityNodeBase& sourceNode) const
{
  const auto i = m_linkSources.find(&sourceNode);
  return i != m_linkSources.end() ? i->second : EmptyLinkEnds;
}

const EntityLinkManager::LinkEndsForName& EntityLinkManager::linksTo(
  const EntityNodeBase& targetNode) const
{
  const auto i = m_linkTargets.find(&targetNode);
  return i != m_linkTargets.end() ? i->second : EmptyLinkEnds;
}

bool EntityLinkManager::hasLink(
  const EntityNodeBase& sourceNode,
  const EntityNodeBase& targetNode,
  const std::string& name) const
{
  const auto i = m_linkSources.find(&sourceNode);
  if (i != m_linkSources.end())
  {
    const auto j = i->second.find(name);
    return j != i->second.end() && j->second.contains(&targetNode);
  }
  return false;
}

bool EntityLinkManager::hasMissingTarget(
  const EntityNodeBase& sourceNode, const std::string& name) const
{
  const auto i = m_linkSources.find(&sourceNode);
  if (i != m_linkSources.end())
  {
    const auto j = i->second.find(name);
    return j != i->second.end() && j->second.empty();
  }
  return false;
}

std::vector<std::string> EntityLinkManager::getLinksWithMissingTarget(
  const EntityNodeBase& sourceNode) const
{
  const auto i = m_linkSources.find(&sourceNode);
  return i != m_linkSources.end()
           ? i->second | std::views::filter([](const auto& nameToTargetMap) {
               return nameToTargetMap.second.empty();
             }) | std::views::transform([](const auto& nameToTargetMap) {
               return nameToTargetMap.first;
             }) | kdl::ranges::to<std::vector>()
           : std::vector<std::string>{};
}


bool EntityLinkManager::hasMissingSource(const EntityNodeBase& targetNode) const
{
  const auto i = m_linkTargets.find(&targetNode);
  return i != m_linkTargets.end() && i->second.empty();
}


void EntityLinkManager::addEntityNode(EntityNodeBase& entityNode)
{
  addLinksFrom(entityNode);
  addLinksTo(entityNode);
}

void EntityLinkManager::removeEntityNode(EntityNodeBase& entityNode)
{
  removeLinksFrom(entityNode);
  removeLinksTo(entityNode);
}

void EntityLinkManager::clear()
{
  m_linkSources.clear();
  m_linkTargets.clear();
}

void EntityLinkManager::addLinksFrom(EntityNodeBase& sourceNode)
{
  using namespace EntityPropertyKeys;

  const auto& entity = sourceNode.entity();
  for (const auto& propertyPrefix : {Target, Killtarget})
  {
    for (const auto& property : entity.numberedProperties(propertyPrefix))
    {
      // The node has a target or killtarget property. We create an entry for the node
      // and the property even if we don't know if there are any target nodes. This way,
      // we can detect link sources with missing targets during validation.
      auto& linkTargetsForName = m_linkSources[&sourceNode][propertyPrefix];

      auto targetNodes =
        m_nodeIndex.findNodes<EntityNodeBase>(property.value())
        | std::views::filter([&](const auto& entityNode) {
            return entityNode->entity().hasProperty(Targetname, property.value());
          });

      for (auto* targetNode : targetNodes)
      {
        linkTargetsForName.emplace(targetNode);
        m_linkTargets[targetNode][propertyPrefix].emplace(&sourceNode);
      }
    }
  }
}

void EntityLinkManager::addLinksTo(EntityNodeBase& targetNode)
{
  using namespace EntityPropertyKeys;

  const auto& entity = targetNode.entity();
  if (const auto* targetname = entity.property(Targetname))
  {
    // The entity has a targetname property, so we will create an entry for it even though
    // it might not have any link sources. We use the empty entry to identify that is is
    // missing a link source during validation.
    auto& linksToTarget = m_linkTargets[&targetNode];

    for (const auto& propertyPrefix : {Target, Killtarget})
    {
      auto sourceNodes =
        m_nodeIndex.findNodes<EntityNodeBase>(*targetname)
        | std::views::filter([&](const auto& entityNode) {
            return entityNode->entity().hasNumberedProperty(propertyPrefix, *targetname);
          });

      for (auto* sourceNode : sourceNodes)
      {
        m_linkSources[sourceNode][propertyPrefix].emplace(&targetNode);
        linksToTarget[propertyPrefix].emplace(sourceNode);
      }
    }
  }
}

void EntityLinkManager::removeLinksFrom(const EntityNodeBase& sourceNode)
{
  if (auto i = m_linkSources.find(&sourceNode); i != m_linkSources.end())
  {
    std::ranges::for_each(i->second, [&](const auto& nameToTargetsEntry) {
      const auto& [name, targetNodes] = nameToTargetsEntry;

      std::ranges::for_each(targetNodes, [&](const auto* targetNode) {
        removeLinkFromTarget(sourceNode, *targetNode, name);
      });
    });

    m_linkSources.erase(i);
  }
}

void EntityLinkManager::removeLinksTo(const EntityNodeBase& targetNode)
{
  if (auto i = m_linkTargets.find(&targetNode); i != m_linkTargets.end())
  {
    std::ranges::for_each(i->second, [&](const auto& nameToSourcesEntry) {
      const auto& [name, sourceNodes] = nameToSourcesEntry;

      std::ranges::for_each(sourceNodes, [&](const auto* sourceNode) {
        removeLinkFromSource(*sourceNode, targetNode, name);
      });
    });

    m_linkTargets.erase(i);
  }
}


void EntityLinkManager::removeLinkFromTarget(
  const EntityNodeBase& sourceNode,
  const EntityNodeBase& targetNode,
  const std::string& name)
{
  if (auto i = m_linkTargets.find(&targetNode); i != m_linkTargets.end())
  {
    auto& nameToSourcesMap = i->second;
    if (auto j = nameToSourcesMap.find(name); j != nameToSourcesMap.end())
    {
      auto& sourcesForName = j->second;
      sourcesForName.erase(&sourceNode);

      if (sourcesForName.empty())
      {
        nameToSourcesMap.erase(j);
      }
    }

    // Don't erase i from m_linkTargets even if it becomes empty! The entry will still be
    // used to find target nodes with missing sources during validation.
  }
}

void EntityLinkManager::removeLinkFromSource(
  const EntityNodeBase& sourceNode,
  const EntityNodeBase& targetNode,
  const std::string& name)
{
  if (auto i = m_linkSources.find(&sourceNode); i != m_linkSources.end())
  {
    auto& nameToTargetsMap = i->second;
    if (auto j = nameToTargetsMap.find(name); j != nameToTargetsMap.end())
    {
      auto& targetsForName = j->second;
      targetsForName.erase(&targetNode);

      // Don't erase j from nameToTargetsMap even if it becomes empty! The entry will
      // still be used to find target nodes with missing targets during validation.
    }
  }
}

} // namespace tb::mdl
