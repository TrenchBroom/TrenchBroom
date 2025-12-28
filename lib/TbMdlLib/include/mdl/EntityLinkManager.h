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

#include "kd/reflection_decl.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace tb::mdl
{
class EntityNodeBase;
class NodeIndex;

struct LinkEnd
{
  const EntityNodeBase* node;
  std::string propertyKey;

  kdl_reflect_decl(LinkEnd, node, propertyKey);
};

class EntityLinkManager
{
public:
  using LinkEnds = std::unordered_set<LinkEnd>;
  using LinkEndsForPropertyKey = std::unordered_map<std::string, LinkEnds>;
  using LinksForEntityNode =
    std::unordered_map<const EntityNodeBase*, LinkEndsForPropertyKey>;

private:
  const NodeIndex& m_nodeIndex;

  LinksForEntityNode m_linkSources;
  LinksForEntityNode m_linkTargets;

public:
  explicit EntityLinkManager(const NodeIndex& nodeIndex);

  const LinkEndsForPropertyKey& linksFrom(const EntityNodeBase& sourceNode) const;
  const LinkEnds& linksFrom(
    const EntityNodeBase& sourceNode, const std::string& sourcePropertyKey) const;

  const LinkEndsForPropertyKey& linksTo(const EntityNodeBase& targetNode) const;
  const LinkEnds& linksTo(
    const EntityNodeBase& targetNode, const std::string& targetPropertyKey) const;

  bool hasLink(
    const EntityNodeBase& sourceNode,
    const EntityNodeBase& targetNode,
    const std::string& sourcePropertyKey) const;

  bool hasLink(
    const EntityNodeBase& sourceNode,
    const EntityNodeBase& targetNode,
    const std::string& sourcePropertyKey,
    const std::string& targetPropertyKey) const;

  bool hasMissingTarget(
    const EntityNodeBase& sourceNode, const std::string& sourcePropertyKey) const;
  bool hasMissingSource(
    const EntityNodeBase& targetNode, const std::string& targetPropertyKey) const;

  std::vector<std::string> getSourcePropertyKeysWithMissingTarget(
    const EntityNodeBase& sourceNode) const;

  std::vector<std::string> getTargetPropertyKeysWithMissingSource(
    const EntityNodeBase& targetNode) const;

  void addEntityNode(EntityNodeBase& entityNode);
  void removeEntityNode(EntityNodeBase& entityNode);

  void clear();

private:
  void addLinksFrom(EntityNodeBase& sourceNode);
  void addLinksTo(EntityNodeBase& targetNode);

  void removeLinksFrom(const EntityNodeBase& sourceNode);
  void removeLinksTo(const EntityNodeBase& targetNode);
};

} // namespace tb::mdl

template <>
struct std::hash<tb::mdl::LinkEnd>
{
  std::size_t operator()(const tb::mdl::LinkEnd& linkEnd) const noexcept;
};
