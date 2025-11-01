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

#include "EntityLinkRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityLinkManager.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"

#include "vm/vec.h"

#include <cassert>
#include <ranges>
#include <unordered_set>

namespace tb::render
{

namespace
{

auto getLinkEnds(const auto& entityLinks)
{
  return entityLinks
         | std::views::transform(
           [](const auto& nameAndTargetNodes) -> const mdl::EntityLinkManager::LinkEnds& {
             return nameAndTargetNodes.second;
           })
         | std::views::join;
}

void addLink(
  const mdl::EntityNodeBase& sourceNode,
  const mdl::EntityNodeBase& targetNode,
  const Color& defaultColor,
  const Color& selectedColor,
  std::vector<LinkRenderer::LineVertex>& links)
{
  const auto anySelected = sourceNode.selected() || sourceNode.descendantSelected()
                           || targetNode.selected() || targetNode.descendantSelected();
  const auto& sourceColor = anySelected ? selectedColor : defaultColor;
  const auto& targetColor = anySelected ? selectedColor : defaultColor;

  links.emplace_back(vm::vec3f{sourceNode.linkSourceAnchor()}, sourceColor.toRgbaF());
  links.emplace_back(vm::vec3f{targetNode.linkTargetAnchor()}, targetColor.toRgbaF());
}

struct CollectAllLinksVisitor
{
  const mdl::EntityLinkManager& entityLinkManager;
  const mdl::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  void visit(
    const mdl::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    if (editorContext.visible(node))
    {
      addLinks(node, entityLinkManager.linksFrom(node), linkVertices);
    }
  }

  void addLinks(
    const mdl::EntityNodeBase& sourceNode,
    const mdl::EntityLinkManager::LinkEndsForPropertyKey& entityLinks,
    std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    for (const auto& linkEnd : getLinkEnds(entityLinks))
    {
      const auto& targetNode = *linkEnd.node;
      if (editorContext.visible(targetNode))
      {
        addLink(sourceNode, targetNode, defaultColor, selectedColor, linkVertices);
      }
    }
  }
};

struct CollectTransitiveSelectedLinksVisitor
{
  const mdl::EntityLinkManager& entityLinkManager;
  const mdl::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  std::unordered_set<const mdl::Node*> visited;

  void visit(
    const mdl::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    if (visited.insert(&node).second && editorContext.visible(node))
    {
      addLinksFrom(node, entityLinkManager.linksFrom(node), linkVertices);
      addLinksTo(node, entityLinkManager.linksTo(node), linkVertices);
    }
  }

  void addLinksFrom(
    const mdl::EntityNodeBase& sourceNode,
    const mdl::EntityLinkManager::LinkEndsForPropertyKey& entityLinks,
    std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    for (const auto& linkEnd : getLinkEnds(entityLinks))
    {
      const auto& targetNode = *linkEnd.node;
      if (editorContext.visible(targetNode))
      {
        addLink(sourceNode, targetNode, defaultColor, selectedColor, linkVertices);
        visit(targetNode, linkVertices);
      }
    }
  }

  void addLinksTo(
    const mdl::EntityNodeBase& targetNode,
    const mdl::EntityLinkManager::LinkEndsForPropertyKey& entityLinks,
    std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    for (const auto& linkEnd : getLinkEnds(entityLinks))
    {
      const auto& sourceNode = *linkEnd.node;
      if (editorContext.visible(sourceNode))
      {
        addLink(sourceNode, targetNode, defaultColor, selectedColor, linkVertices);
        visit(sourceNode, linkVertices);
      }
    }
  }
};

struct CollectDirectSelectedLinksVisitor
{
  const mdl::EntityLinkManager& entityLinkManager;
  const mdl::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  void visit(
    const mdl::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    if (node.selected() || node.descendantSelected())
    {
      addLinksFrom(node, entityLinkManager.linksFrom(node), linkVertices);
      addLinksTo(node, entityLinkManager.linksTo(node), linkVertices);
    }
  }

  void addLinksFrom(
    const mdl::EntityNodeBase& sourceNode,
    const mdl::EntityLinkManager::LinkEndsForPropertyKey& entityLinks,
    std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    for (const auto& linkEnd : getLinkEnds(entityLinks))
    {
      const auto& targetNode = *linkEnd.node;
      if (editorContext.visible(targetNode))
      {
        addLink(sourceNode, targetNode, defaultColor, selectedColor, linkVertices);
      }
    }
  }

  void addLinksTo(
    const mdl::EntityNodeBase& targetNode,
    const mdl::EntityLinkManager::LinkEndsForPropertyKey& entityLinks,
    std::vector<LinkRenderer::LineVertex>& linkVertices)
  {
    for (const auto& linkEnd : getLinkEnds(entityLinks))
    {
      const auto& sourceNode = *linkEnd.node;
      if (
        !sourceNode.selected() && !sourceNode.descendantSelected()
        && editorContext.visible(sourceNode))
      {
        addLink(sourceNode, targetNode, defaultColor, selectedColor, linkVertices);
      }
    }
  }
};

template <typename Visitor>
auto collectSelectedLinks(const mdl::Selection& selection, Visitor visitor)
{
  auto links = std::vector<LinkRenderer::LineVertex>{};

  for (auto* node : selection.nodes)
  {
    node->accept(kdl::overload(
      [](const mdl::WorldNode*) {},
      [](const mdl::LayerNode*) {},
      [](const mdl::GroupNode*) {},
      [&](const mdl::EntityNode* entityNode) { visitor.visit(*entityNode, links); },
      [](auto&& thisLambda, const mdl::BrushNode* brushNode) {
        brushNode->visitParent(thisLambda);
      },
      [](auto&& thisLambda, const mdl::PatchNode* patchNode) {
        patchNode->visitParent(thisLambda);
      }));
  }

  return links;
}

auto getAllLinks(
  const mdl::Map& map, const Color& defaultColor, const Color& selectedColor)
{
  auto links = std::vector<LinkRenderer::LineVertex>{};

  if (map.world())
  {
    auto visitor = CollectAllLinksVisitor{
      map.entityLinkManager(), map.editorContext(), defaultColor, selectedColor};

    map.world()->accept(kdl::overload(
      [](auto&& thisLambda, const mdl::WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const mdl::LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const mdl::GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [&](const mdl::EntityNode* entityNode) { visitor.visit(*entityNode, links); },
      [](const mdl::BrushNode*) {},
      [](const mdl::PatchNode*) {}));
  }

  return links;
}

auto getTransitiveSelectedLinks(
  const mdl::Map& map, const Color& defaultColor, const Color& selectedColor)
{
  auto visitor = CollectTransitiveSelectedLinksVisitor{
    map.entityLinkManager(), map.editorContext(), defaultColor, selectedColor, {}};
  return collectSelectedLinks(map.selection(), visitor);
}

auto getDirectSelectedLinks(
  const mdl::Map& map, const Color& defaultColor, const Color& selectedColor)
{
  auto visitor = CollectDirectSelectedLinksVisitor{
    map.entityLinkManager(), map.editorContext(), defaultColor, selectedColor};
  return collectSelectedLinks(map.selection(), visitor);
}

auto getLinks(const mdl::Map& map, const Color& defaultColor, const Color& selectedColor)
{
  const auto entityLinkMode = pref(Preferences::EntityLinkMode);
  if (entityLinkMode == Preferences::entityLinkModeAll())
  {
    return getAllLinks(map, defaultColor, selectedColor);
  }
  if (entityLinkMode == Preferences::entityLinkModeTransitive())
  {
    return getTransitiveSelectedLinks(map, defaultColor, selectedColor);
  }
  if (entityLinkMode == Preferences::entityLinkModeDirect())
  {
    return getDirectSelectedLinks(map, defaultColor, selectedColor);
  }

  return std::vector<LinkRenderer::LineVertex>{};
}

} // namespace

EntityLinkRenderer::EntityLinkRenderer(mdl::Map& map)
  : m_map{map}
{
}

void EntityLinkRenderer::setDefaultColor(const Color& defaultColor)
{
  if (defaultColor != m_defaultColor)
  {
    m_defaultColor = defaultColor;
    invalidate();
  }
}

void EntityLinkRenderer::setSelectedColor(const Color& selectedColor)
{
  if (selectedColor != m_selectedColor)
  {
    m_selectedColor = selectedColor;
    invalidate();
  }
}

std::vector<LinkRenderer::LineVertex> EntityLinkRenderer::getLinks()
{
  return render::getLinks(m_map, m_defaultColor, m_selectedColor);
}

} // namespace tb::render
