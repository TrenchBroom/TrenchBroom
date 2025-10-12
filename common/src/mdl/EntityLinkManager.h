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

#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tb::mdl
{
class EntityNodeBase;
class NodeIndex;

class EntityLinkManager
{
public:
  using LinkEnds = std::unordered_set<const EntityNodeBase*>;
  using LinkEndsForName = std::unordered_map<std::string, LinkEnds>;

private:
  const NodeIndex& m_nodeIndex;

  std::unordered_map<const EntityNodeBase*, LinkEndsForName> m_linkSources;
  std::unordered_map<const EntityNodeBase*, LinkEndsForName> m_linkTargets;

public:
  explicit EntityLinkManager(const NodeIndex& nodeIndex);

  const LinkEndsForName& linksFrom(const EntityNodeBase& sourceNode) const;
  const LinkEndsForName& linksTo(const EntityNodeBase& targetNode) const;

  bool hasLink(
    const EntityNodeBase& sourceNode,
    const EntityNodeBase& targetNode,
    const std::string& name) const;

  bool hasMissingTarget(const EntityNodeBase& sourceNode, const std::string& name) const;
  bool hasMissingSource(const EntityNodeBase& targetNode) const;

  std::vector<std::string> getLinksWithMissingTarget(
    const EntityNodeBase& sourceNode) const;

  void addEntityNode(EntityNodeBase& entityNode);
  void removeEntityNode(EntityNodeBase& entityNode);

  void clear();

private:
  void addLinksFrom(EntityNodeBase& sourceNode);
  void addLinksTo(EntityNodeBase& targetNode);

  void removeLinksFrom(const EntityNodeBase& sourceNode);
  void removeLinksTo(const EntityNodeBase& targetNode);

  void removeLinkFromTarget(
    const EntityNodeBase& sourceNode,
    const EntityNodeBase& targetNode,
    const std::string& name);
  void removeLinkFromSource(
    const EntityNodeBase& sourceNode,
    const EntityNodeBase& targetNode,
    const std::string& name);
};
} // namespace tb::mdl
