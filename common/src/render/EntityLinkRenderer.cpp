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
#include "mdl/EntityNode.h"
#include "mdl/EntityNodeBase.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/PatchNode.h"
#include "mdl/Selection.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "kdl/memory_utils.h"
#include "kdl/overload.h"

#include "vm/vec.h"

#include <cassert>
#include <unordered_set>

namespace tb::render
{

EntityLinkRenderer::EntityLinkRenderer(std::weak_ptr<ui::MapDocument> document)
  : m_document{std::move(document)}
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

namespace
{

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

  links.emplace_back(vm::vec3f{sourceNode.linkSourceAnchor()}, sourceColor);
  links.emplace_back(vm::vec3f{targetNode.linkTargetAnchor()}, targetColor);
}

struct CollectAllLinksVisitor
{
  const mdl::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  void visit(
    const mdl::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& links)
  {
    if (editorContext.visible(node))
    {
      addTargets(node, node.linkTargets(), links);
      addTargets(node, node.killTargets(), links);
    }
  }

  void addTargets(
    const mdl::EntityNodeBase& sourceNode,
    const std::vector<mdl::EntityNodeBase*>& targetNodes,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (const auto* targetNode : targetNodes)
    {
      if (editorContext.visible(*targetNode))
      {
        addLink(sourceNode, *targetNode, defaultColor, selectedColor, links);
      }
    }
  }
};

struct CollectTransitiveSelectedLinksVisitor
{
  const mdl::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  std::unordered_set<const mdl::Node*> visited;

  void visit(
    const mdl::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& links)
  {
    if (editorContext.visible(node))
    {
      if (visited.insert(&node).second)
      {
        addSources(node.linkSources(), node, links);
        addSources(node.killSources(), node, links);
        addTargets(node, node.linkTargets(), links);
        addTargets(node, node.killTargets(), links);
      }
    }
  }

  void addSources(
    const std::vector<mdl::EntityNodeBase*>& sourceNodes,
    const mdl::EntityNodeBase& targetNode,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (auto* sourceNode : sourceNodes)
    {
      if (editorContext.visible(*sourceNode))
      {
        addLink(*sourceNode, targetNode, defaultColor, selectedColor, links);
        visit(*sourceNode, links);
      }
    }
  }

  void addTargets(
    const mdl::EntityNodeBase& sourceNode,
    const std::vector<mdl::EntityNodeBase*>& targetNodes,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (auto* targetNode : targetNodes)
    {
      if (editorContext.visible(*targetNode))
      {
        addLink(sourceNode, *targetNode, defaultColor, selectedColor, links);
        visit(*targetNode, links);
      }
    }
  }
};

struct CollectDirectSelectedLinksVisitor
{
  const mdl::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  void visit(
    const mdl::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& links)
  {
    if (node.selected() || node.descendantSelected())
    {
      addSources(node.linkSources(), node, links);
      addSources(node.killSources(), node, links);
      addTargets(node, node.linkTargets(), links);
      addTargets(node, node.killTargets(), links);
    }
  }

  void addSources(
    const std::vector<mdl::EntityNodeBase*>& sourceNodes,
    const mdl::EntityNodeBase& targetNode,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (const auto* sourceNode : sourceNodes)
    {
      if (
        !sourceNode->selected() && !sourceNode->descendantSelected()
        && editorContext.visible(*sourceNode))
      {
        addLink(*sourceNode, targetNode, defaultColor, selectedColor, links);
      }
    }
  }

  void addTargets(
    const mdl::EntityNodeBase& sourceNode,
    const std::vector<mdl::EntityNodeBase*>& targetNodes,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (const auto* targetNode : targetNodes)
    {
      if (editorContext.visible(*targetNode))
      {
        addLink(sourceNode, *targetNode, defaultColor, selectedColor, links);
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
    auto visitor =
      CollectAllLinksVisitor{map.editorContext(), defaultColor, selectedColor};

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
    map.editorContext(), defaultColor, selectedColor, {}};
  return collectSelectedLinks(map.selection(), visitor);
}

auto getDirectSelectedLinks(
  const mdl::Map& map, const Color& defaultColor, const Color& selectedColor)
{
  auto visitor =
    CollectDirectSelectedLinksVisitor{map.editorContext(), defaultColor, selectedColor};
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

std::vector<LinkRenderer::LineVertex> EntityLinkRenderer::getLinks()
{
  return render::getLinks(
    kdl::mem_lock(m_document)->map(), m_defaultColor, m_selectedColor);
}

} // namespace tb::render
