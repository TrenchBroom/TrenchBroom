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

#include "EntityLinkRenderer.h"

#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/EntityNodeBase.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/overload.h>

#include <vecmath/vec.h>

#include <cassert>
#include <unordered_set>

namespace TrenchBroom::Renderer
{

EntityLinkRenderer::EntityLinkRenderer(std::weak_ptr<View::MapDocument> document)
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
  const Model::EntityNodeBase& source,
  const Model::EntityNodeBase& target,
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
  const Model::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  void visit(
    const Model::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& links)
  {
    if (editorContext.visible(&node))
    {
      addTargets(node, node.linkTargets(), links);
      addTargets(node, node.killTargets(), links);
    }
  }

  void addTargets(
    const Model::EntityNodeBase& source,
    const std::vector<Model::EntityNodeBase*>& targets,
    std::vector<LinkRenderer::LineVertex>& links)
  {
    for (const Model::EntityNodeBase* target : targets)
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
  const Model::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  std::unordered_set<const Model::Node*> visited = {};

  void visit(
    const Model::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& links)
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
    const std::vector<Model::EntityNodeBase*>& sources,
    const Model::EntityNodeBase& target,
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
    const Model::EntityNodeBase& source,
    const std::vector<Model::EntityNodeBase*>& targets,
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
  const Model::EditorContext& editorContext;
  Color defaultColor;
  Color selectedColor;

  void visit(
    const Model::EntityNodeBase& node, std::vector<LinkRenderer::LineVertex>& links)
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
    const std::vector<Model::EntityNodeBase*>& sources,
    const Model::EntityNodeBase& target,
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
    const Model::EntityNodeBase& source,
    const std::vector<Model::EntityNodeBase*>& targets,
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
auto collectSelectedLinks(const Model::NodeCollection& selectedNodes, Visitor visitor)
{
  auto links = std::vector<LinkRenderer::LineVertex>{};

  for (auto* node : selectedNodes)
  {
    node->accept(kdl::overload(
      [](const Model::WorldNode*) {},
      [](const Model::LayerNode*) {},
      [](const Model::GroupNode*) {},
      [&](const Model::EntityNode* entityNode) { visitor.visit(*entityNode, links); },
      [](auto&& thisLambda, const Model::BrushNode* brushNode) {
        brushNode->visitParent(thisLambda);
      },
      [](auto&& thisLambda, const Model::PatchNode* patchNode) {
        patchNode->visitParent(thisLambda);
      }));
  }

  return links;
}

auto getAllLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
{
  auto links = std::vector<LinkRenderer::LineVertex>{};

  if (document.world())
  {
    auto visitor =
      CollectAllLinksVisitor{document.editorContext(), defaultColor, selectedColor};

    document.world()->accept(kdl::overload(
      [](auto&& thisLambda, const Model::WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const Model::LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const Model::GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [&](const Model::EntityNode* entityNode) { visitor.visit(*entityNode, links); },
      [](const Model::BrushNode*) {},
      [](const Model::PatchNode*) {}));
  }

  return links;
}

auto getTransitiveSelectedLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
{
  auto visitor = CollectTransitiveSelectedLinksVisitor{
    document.editorContext(), defaultColor, selectedColor};
  return collectSelectedLinks(document.selectedNodes(), visitor);
}

auto getDirectSelectedLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
{
  auto visitor = CollectDirectSelectedLinksVisitor{
    document.editorContext(), defaultColor, selectedColor};
  return collectSelectedLinks(document.selectedNodes(), visitor);
}

auto getLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor)
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
  return Renderer::getLinks(*kdl::mem_lock(m_document), m_defaultColor, m_selectedColor);
}

} // namespace TrenchBroom::Renderer
