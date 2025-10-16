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

#include "mdl/EntityDefinitionUtils.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/NodeIndex.h"

#include "kdl/hash_utils.h"
#include "kdl/ranges/to.h"
#include "kdl/reflection_impl.h"

#include <ranges>

namespace tb::mdl
{
namespace
{
const EntityLinkManager::LinkEndsForPropertyKey EmptyLinkEndsForPropertyKey;
const EntityLinkManager::LinkEnds EmptyLinkEnds;

const auto& getLinks(
  const EntityLinkManager::LinksForEntityNode& links, const EntityNodeBase& node)
{
  const auto iLinkEndsForPropertyKey = links.find(&node);
  return iLinkEndsForPropertyKey != links.end() ? iLinkEndsForPropertyKey->second
                                                : EmptyLinkEndsForPropertyKey;
}

const auto& getLinks(
  const EntityLinkManager::LinksForEntityNode& links,
  const EntityNodeBase& node,
  const std::string& propertyKey)
{
  const auto& linksForNode = getLinks(links, node);
  if (const auto iLinkEnds = linksForNode.find(propertyKey);
      iLinkEnds != linksForNode.end())
  {
    return iLinkEnds->second;
  }
  return EmptyLinkEnds;
}

auto hasMissingLinkEnd(
  const EntityLinkManager::LinksForEntityNode& links,
  const EntityNodeBase& node,
  const std::string& propertyKey)
{
  if (const auto iLinkEndsForPropertyKey = links.find(&node);
      iLinkEndsForPropertyKey != links.end())
  {
    const auto& linkEndsForPropertyKey = iLinkEndsForPropertyKey->second;
    const auto iLinkEnds = linkEndsForPropertyKey.find(propertyKey);
    return iLinkEnds != linkEndsForPropertyKey.end() && iLinkEnds->second.empty();
  }
  return false;
}

auto getPropertyKeysWithMissingLinkEnd(
  const EntityLinkManager::LinksForEntityNode& links, const EntityNodeBase& node)
{
  if (const auto iLinkEndsForPropertyKey = links.find(&node);
      iLinkEndsForPropertyKey != links.end())
  {
    const auto& linkEndsForPropertyKey = iLinkEndsForPropertyKey->second;
    return linkEndsForPropertyKey
           | std::views::filter([](const auto& entry) { return entry.second.empty(); })
           | std::views::keys | kdl::ranges::to<std::vector>();
  }

  return std::vector<std::string>{};
}

auto getLinkSourcePropertyKeys(const EntityNodeBase& sourceNode)
{
  return getLinkSourcePropertyDefinitions(sourceNode.entity().definition())
         | std::views::transform(
           [](const auto* propertyDefinition) { return propertyDefinition->key; })
         | kdl::ranges::to<std::vector>();
}

auto getLinkTargetPropertyKeys(const EntityNodeBase& targetNode)
{
  return getLinkTargetPropertyDefinitions(targetNode.entity().definition())
         | std::views::transform(
           [](const auto* propertyDefinition) { return propertyDefinition->key; })
         | kdl::ranges::to<std::vector>();
}

template <std::ranges::range R>
auto getPropertiesForKeys(const EntityNodeBase& entityNode, R keys)
{
  return std::move(keys) | std::views::transform([&](const auto& key) {
           return entityNode.entity().numberedProperties(key)
                  | std::views::transform(
                    [&](auto property) { return std::pair{key, std::move(property)}; });
         })
         | std::views::join;
}

void removeLinkEnd(
  EntityLinkManager::LinksForEntityNode& links,
  const LinkEnd& linkEndToKeep,
  const LinkEnd& linkEndToRemove)
{
  if (auto iLinkEndsForPropertyKey = links.find(linkEndToKeep.node);
      iLinkEndsForPropertyKey != links.end())
  {
    auto& linkEndsForPropertyKey = iLinkEndsForPropertyKey->second;
    if (auto iLinkEnds = linkEndsForPropertyKey.find(linkEndToKeep.propertyKey);
        iLinkEnds != linkEndsForPropertyKey.end())
    {
      auto& linkEnds = iLinkEnds->second;
      linkEnds.erase(linkEndToRemove);

      // Don't erase iLinkEnds from linkEndsForPropertyKey even if it becomes empty! The
      // entry will still be used to find links with missing opposite ends during
      // validation.
    }
  }
}

void removeLinks(
  EntityLinkManager::LinksForEntityNode& links,
  EntityLinkManager::LinksForEntityNode& oppositeLinks,
  const EntityNodeBase& node)
{
  if (auto iLinkEndsForPropertyKey = links.find(&node);
      iLinkEndsForPropertyKey != links.end())
  {
    auto& linkEndsForPropertyKey = iLinkEndsForPropertyKey->second;
    std::ranges::for_each(
      linkEndsForPropertyKey, [&](const auto& propertyKeyAndLinkEnds) {
        const auto& [propertyKey, oppositeLinkEnds] = propertyKeyAndLinkEnds;

        const auto linkEnd = LinkEnd{&node, propertyKey};
        std::ranges::for_each(oppositeLinkEnds, [&](const auto& oppositeLinkEnd) {
          removeLinkEnd(oppositeLinks, oppositeLinkEnd, linkEnd);
        });
      });

    links.erase(iLinkEndsForPropertyKey);
  }
}

} // namespace

kdl_reflect_impl(LinkEnd);

EntityLinkManager::EntityLinkManager(const NodeIndex& nodeIndex)
  : m_nodeIndex{nodeIndex}
{
}

const EntityLinkManager::LinkEndsForPropertyKey& EntityLinkManager::linksFrom(
  const EntityNodeBase& sourceNode) const
{
  return getLinks(m_linkSources, sourceNode);
}

const EntityLinkManager::LinkEnds& EntityLinkManager::linksFrom(
  const EntityNodeBase& sourceNode, const std::string& sourcePropertyKey) const
{
  return getLinks(m_linkSources, sourceNode, sourcePropertyKey);
}

const EntityLinkManager::LinkEndsForPropertyKey& EntityLinkManager::linksTo(
  const EntityNodeBase& targetNode) const
{
  return getLinks(m_linkTargets, targetNode);
}

const EntityLinkManager::LinkEnds& EntityLinkManager::linksTo(
  const EntityNodeBase& targetNode, const std::string& targetPropertyKey) const
{
  return getLinks(m_linkTargets, targetNode, targetPropertyKey);
}

bool EntityLinkManager::hasLink(
  const EntityNodeBase& sourceNode,
  const EntityNodeBase& targetNode,
  const std::string& sourcePropertyKey) const
{
  return std::ranges::any_of(
    linksFrom(sourceNode, sourcePropertyKey),
    [&](const auto& linkEnd) { return linkEnd.node == &targetNode; });
}

bool EntityLinkManager::hasLink(
  const EntityNodeBase& sourceNode,
  const EntityNodeBase& targetNode,
  const std::string& sourcePropertyKey,
  const std::string& targetPropertyKey) const
{
  return linksFrom(sourceNode, sourcePropertyKey)
    .contains(LinkEnd{&targetNode, targetPropertyKey});
}

bool EntityLinkManager::hasMissingTarget(
  const EntityNodeBase& sourceNode, const std::string& sourcePropertyKey) const
{
  return hasMissingLinkEnd(m_linkSources, sourceNode, sourcePropertyKey);
}

std::vector<std::string> EntityLinkManager::getSourcePropertyKeysWithMissingTarget(
  const EntityNodeBase& sourceNode) const
{
  return getPropertyKeysWithMissingLinkEnd(m_linkSources, sourceNode);
}

bool EntityLinkManager::hasMissingSource(
  const EntityNodeBase& targetNode, const std::string& targetPropertyKey) const
{
  return hasMissingLinkEnd(m_linkTargets, targetNode, targetPropertyKey);
}

std::vector<std::string> EntityLinkManager::getTargetPropertyKeysWithMissingSource(
  const EntityNodeBase& targetNode) const
{
  return getPropertyKeysWithMissingLinkEnd(m_linkTargets, targetNode);
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
  for (const auto& [sourcePropertyKey, sourceProperty] :
       getPropertiesForKeys(sourceNode, getLinkSourcePropertyKeys(sourceNode)))
  {
    const auto& sourcePropertyValue = sourceProperty.value();

    // The node has some source property. We create an entry for the node and the property
    // even if we don't know if there are any target nodes. This way, we can detect link
    // sources with missing targets during validation.
    auto& linkTargetsForKey = m_linkSources[&sourceNode][sourcePropertyKey];

    for (const auto* targetNode :
         m_nodeIndex.findNodes<EntityNodeBase>(sourcePropertyValue))
    {
      for (const auto& targetPropertyKey : getLinkTargetPropertyKeys(*targetNode))
      {
        const auto& targetEntity = targetNode->entity();
        if (targetEntity.hasProperty(targetPropertyKey, sourcePropertyValue))
        {
          linkTargetsForKey.emplace(LinkEnd{targetNode, targetPropertyKey});
          m_linkTargets[targetNode][targetPropertyKey].emplace(
            LinkEnd{&sourceNode, sourcePropertyKey});
        }
      }
    }
  }
}

void EntityLinkManager::addLinksTo(EntityNodeBase& targetNode)
{
  for (const auto& [targetPropertyKey, targetProperty] :
       getPropertiesForKeys(targetNode, getLinkTargetPropertyKeys(targetNode)))
  {
    const auto& targetPropertyValue = targetProperty.value();

    // The node has some target property. We create an entry for the node and the property
    // even if we don't know if there are any source nodes. This way, we can detect link
    // targets with missing sources during validation.
    auto& linkSourcesForKey = m_linkTargets[&targetNode][targetPropertyKey];

    for (const auto* sourceNode :
         m_nodeIndex.findNodes<EntityNodeBase>(targetPropertyValue))
    {
      for (const auto& sourcePropertyKey : getLinkSourcePropertyKeys(*sourceNode))
      {
        const auto& sourceEntity = sourceNode->entity();
        if (sourceEntity.hasNumberedProperty(sourcePropertyKey, targetPropertyValue))
        {
          linkSourcesForKey.emplace(LinkEnd{sourceNode, sourcePropertyKey});
          m_linkSources[sourceNode][sourcePropertyKey].emplace(
            LinkEnd{&targetNode, targetPropertyKey});
        }
      }
    }
  }
}

void EntityLinkManager::removeLinksFrom(const EntityNodeBase& sourceNode)
{
  removeLinks(m_linkSources, m_linkTargets, sourceNode);
}

void EntityLinkManager::removeLinksTo(const EntityNodeBase& targetNode)
{
  removeLinks(m_linkTargets, m_linkSources, targetNode);
}

} // namespace tb::mdl

std::size_t std::hash<tb::mdl::LinkEnd>::operator()(
  const tb::mdl::LinkEnd& linkEnd) const noexcept
{
  return kdl::hash(linkEnd.node, linkEnd.propertyKey);
}
