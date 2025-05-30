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
#include "mdl/PatchNode.h"
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
  const mdl::EntityNodeBase& source,
  const mdl::EntityNodeBase& target,
  const Color& defaultColor,
  const Color& selectedColor,
  std::vector<LinkRenderer::LineVertex>& links)
{
  const auto anySelected = source.selected() || source.descendantSelected()
                           || target.selected() || target.descendantSelected();
  const auto& sourceColor = anySelected ? selectedColor : defaultColor;
  const auto& targetColor = anySelected ? selectedColor : defaultColor;

  links.emplace_back(vm::vec3f{source.linkSourceAnchor()}, sourceColor);
  links.emplace_back(vm::vec3f{target.linkTargetAnchor()}, targetColor);
}

struct CollectAllLinksVisitor
{
  const mdl::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  void visit(
    const mdl::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& links)
  {
    if (editorContext.visible(&node))
    {
      addTargets(node, node.linkTargets(), links);
      addTargets(node, node.killTargets(), links);
    }
  }

  void addTargets(
    const mdl::EntityNodeBase& source,
    const std::vector<mdl::EntityNodeBase*>& targets,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (const mdl::EntityNodeBase* target : targets)
    {
      if (editorContext.visible(target))
      {
        addLink(source, *target, defaultColor, selectedColor, links);
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
    if (editorContext.visible(&node))
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
    const std::vector<mdl::EntityNodeBase*>& sources,
    const mdl::EntityNodeBase& target,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (auto* source : sources)
    {
      if (editorContext.visible(source))
      {
        addLink(*source, target, defaultColor, selectedColor, links);
        visit(*source, links);
      }
    }
  }

  void addTargets(
    const mdl::EntityNodeBase& source,
    const std::vector<mdl::EntityNodeBase*>& targets,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (auto* target : targets)
    {
      if (editorContext.visible(target))
      {
        addLink(source, *target, defaultColor, selectedColor, links);
        visit(*target, links);
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
    const std::vector<mdl::EntityNodeBase*>& sources,
    const mdl::EntityNodeBase& target,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (const auto* source : sources)
    {
      if (
        !source->selected() && !source->descendantSelected()
        && editorContext.visible(source))
      {
        addLink(*source, target, defaultColor, selectedColor, links);
      }
    }
  }

  void addTargets(
    const mdl::EntityNodeBase& source,
    const std::vector<mdl::EntityNodeBase*>& targets,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (const auto* target : targets)
    {
      if (editorContext.visible(target))
      {
        addLink(source, *target, defaultColor, selectedColor, links);
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
  ui::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
{
  auto links = std::vector<LinkRenderer::LineVertex>{};

  if (document.world())
  {
    auto visitor =
      CollectAllLinksVisitor{document.editorContext(), defaultColor, selectedColor};

    document.world()->accept(kdl::overload(
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
  ui::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
{
  auto visitor = CollectTransitiveSelectedLinksVisitor{
    document.editorContext(), defaultColor, selectedColor, {}};
  return collectSelectedLinks(document.selection(), visitor);
}

auto getDirectSelectedLinks(
  ui::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
{
  auto visitor = CollectDirectSelectedLinksVisitor{
    document.editorContext(), defaultColor, selectedColor};
  return collectSelectedLinks(document.selection(), visitor);
}

auto getLinks(
  ui::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
{
  const auto entityLinkMode = pref(Preferences::EntityLinkMode);
  if (entityLinkMode == Preferences::entityLinkModeAll())
  {
    return getAllLinks(document, defaultColor, selectedColor);
  }
  if (entityLinkMode == Preferences::entityLinkModeTransitive())
  {
    return getTransitiveSelectedLinks(document, defaultColor, selectedColor);
  }
  if (entityLinkMode == Preferences::entityLinkModeDirect())
  {
    return getDirectSelectedLinks(document, defaultColor, selectedColor);
  }

  return std::vector<LinkRenderer::LineVertex>{};
}
} // namespace

std::vector<LinkRenderer::LineVertex> EntityLinkRenderer::getLinks()
{
  return render::getLinks(*kdl::mem_lock(m_document), m_defaultColor, m_selectedColor);
}

} // namespace tb::render
