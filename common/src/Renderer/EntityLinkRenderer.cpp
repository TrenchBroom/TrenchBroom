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
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/overload.h>

#include <vecmath/vec.h>

#include <cassert>
#include <unordered_set>

namespace TrenchBroom {
namespace Renderer {
EntityLinkRenderer::EntityLinkRenderer(std::weak_ptr<View::MapDocument> document)
  : m_document(document)
  , m_defaultColor(0.5f, 1.0f, 0.5f, 1.0f)
  , m_selectedColor(1.0f, 0.0f, 0.0f, 1.0f) {}

void EntityLinkRenderer::setDefaultColor(const Color& color) {
  if (color == m_defaultColor)
    return;
  m_defaultColor = color;
  invalidate();
}

void EntityLinkRenderer::setSelectedColor(const Color& color) {
  if (color == m_selectedColor)
    return;
  m_selectedColor = color;
  invalidate();
}

namespace {
class CollectLinksVisitor {
protected:
  const Model::EditorContext& m_editorContext;
  const Color m_defaultColor;
  const Color m_selectedColor;
  std::vector<LinkRenderer::LineVertex>& m_links;

protected:
  CollectLinksVisitor(
    const Model::EditorContext& editorContext, const Color& defaultColor,
    const Color& selectedColor, std::vector<LinkRenderer::LineVertex>& links)
    : m_editorContext(editorContext)
    , m_defaultColor(defaultColor)
    , m_selectedColor(selectedColor)
    , m_links(links) {}

public:
  virtual ~CollectLinksVisitor() = default;
  virtual void visit(Model::EntityNodeBase* node) = 0;

protected:
  void addLink(const Model::EntityNodeBase* source, const Model::EntityNodeBase* target) {
    const auto anySelected = source->selected() || source->descendantSelected() ||
                             target->selected() || target->descendantSelected();
    const auto& sourceColor = anySelected ? m_selectedColor : m_defaultColor;
    const auto targetColor = anySelected ? m_selectedColor : m_defaultColor;

    m_links.emplace_back(vm::vec3f(source->linkSourceAnchor()), sourceColor);
    m_links.emplace_back(vm::vec3f(target->linkTargetAnchor()), targetColor);
  }
};

class CollectAllLinksVisitor : public CollectLinksVisitor {
public:
  CollectAllLinksVisitor(
    const Model::EditorContext& editorContext, const Color& defaultColor,
    const Color& selectedColor, std::vector<LinkRenderer::LineVertex>& links)
    : CollectLinksVisitor(editorContext, defaultColor, selectedColor, links) {}

  void visit(Model::EntityNodeBase* node) override {
    if (m_editorContext.visible(node)) {
      addTargets(node, node->linkTargets());
      addTargets(node, node->killTargets());
    }
  }

private:
  void addTargets(
    Model::EntityNodeBase* source, const std::vector<Model::EntityNodeBase*>& targets) {
    for (const Model::EntityNodeBase* target : targets) {
      if (m_editorContext.visible(target))
        addLink(source, target);
    }
  }
};

class CollectTransitiveSelectedLinksVisitor : public CollectLinksVisitor {
private:
  std::unordered_set<Model::Node*> m_visited;

public:
  CollectTransitiveSelectedLinksVisitor(
    const Model::EditorContext& editorContext, const Color& defaultColor,
    const Color& selectedColor, std::vector<LinkRenderer::LineVertex>& links)
    : CollectLinksVisitor(editorContext, defaultColor, selectedColor, links) {}

  void visit(Model::EntityNodeBase* node) override {
    if (m_editorContext.visible(node)) {
      const bool visited = !m_visited.insert(node).second;
      if (!visited) {
        addSources(node->linkSources(), node);
        addSources(node->killSources(), node);
        addTargets(node, node->linkTargets());
        addTargets(node, node->killTargets());
      }
    }
  }

private:
  void addSources(
    const std::vector<Model::EntityNodeBase*>& sources, Model::EntityNodeBase* target) {
    for (Model::EntityNodeBase* source : sources) {
      if (m_editorContext.visible(source)) {
        addLink(source, target);
        visit(source);
      }
    }
  }

  void addTargets(
    Model::EntityNodeBase* source, const std::vector<Model::EntityNodeBase*>& targets) {
    for (Model::EntityNodeBase* target : targets) {
      if (m_editorContext.visible(target)) {
        addLink(source, target);
        visit(target);
      }
    }
  }
};

struct CollectDirectSelectedLinksVisitor : public CollectLinksVisitor {
public:
  CollectDirectSelectedLinksVisitor(
    const Model::EditorContext& editorContext, const Color& defaultColor,
    const Color& selectedColor, std::vector<LinkRenderer::LineVertex>& links)
    : CollectLinksVisitor(editorContext, defaultColor, selectedColor, links) {}

  void visit(Model::EntityNodeBase* node) override {
    if (node->selected() || node->descendantSelected()) {
      addSources(node->linkSources(), node);
      addSources(node->killSources(), node);
      addTargets(node, node->linkTargets());
      addTargets(node, node->killTargets());
    }
  }

private:
  void addSources(
    const std::vector<Model::EntityNodeBase*>& sources, Model::EntityNodeBase* target) {
    for (const Model::EntityNodeBase* source : sources) {
      if (!source->selected() && !source->descendantSelected() && m_editorContext.visible(source))
        addLink(source, target);
    }
  }

  void addTargets(
    Model::EntityNodeBase* source, const std::vector<Model::EntityNodeBase*>& targets) {
    for (const Model::EntityNodeBase* target : targets) {
      if (m_editorContext.visible(target))
        addLink(source, target);
    }
  }
};
} // namespace

static void collectSelectedLinks(
  const Model::NodeCollection& selectedNodes, CollectLinksVisitor& collectLinks) {
  for (auto* node : selectedNodes) {
    node->accept(kdl::overload(
      [](Model::WorldNode*) {}, [](Model::LayerNode*) {}, [](Model::GroupNode*) {},
      [&](Model::EntityNode* entity) {
        collectLinks.visit(entity);
      },
      [](auto&& thisLambda, Model::BrushNode* brush) {
        brush->parent()->accept(thisLambda);
      },
      [](Model::PatchNode*) {}));
  }
}

static void getAllLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor,
  std::vector<LinkRenderer::LineVertex>& links) {
  const Model::EditorContext& editorContext = document.editorContext();

  CollectAllLinksVisitor collectLinks(editorContext, defaultColor, selectedColor, links);

  if (document.world() != nullptr) {
    document.world()->accept(kdl::overload(
      [](auto&& thisLambda, Model::WorldNode* world) {
        world->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, Model::LayerNode* layer) {
        layer->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, Model::GroupNode* group) {
        group->visitChildren(thisLambda);
      },
      [&](Model::EntityNode* entity) {
        collectLinks.visit(entity);
      },
      [](Model::BrushNode*) {}, [](Model::PatchNode*) {}));
  }
}

static void getTransitiveSelectedLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor,
  std::vector<LinkRenderer::LineVertex>& links) {
  const Model::EditorContext& editorContext = document.editorContext();

  CollectTransitiveSelectedLinksVisitor collectLinks(
    editorContext, defaultColor, selectedColor, links);
  collectSelectedLinks(document.selectedNodes(), collectLinks);
}

static void getDirectSelectedLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor,
  std::vector<LinkRenderer::LineVertex>& links) {
  const Model::EditorContext& editorContext = document.editorContext();

  CollectDirectSelectedLinksVisitor collectLinks(editorContext, defaultColor, selectedColor, links);
  collectSelectedLinks(document.selectedNodes(), collectLinks);
}

static void getLinks(
  View::MapDocument& document, const Color& defaultColor, const Color& selectedColor,
  std::vector<LinkRenderer::LineVertex>& links) {
  const QString entityLinkMode = pref(Preferences::EntityLinkMode);

  if (entityLinkMode == Preferences::entityLinkModeAll()) {
    getAllLinks(document, defaultColor, selectedColor, links);
  } else if (entityLinkMode == Preferences::entityLinkModeTransitive()) {
    getTransitiveSelectedLinks(document, defaultColor, selectedColor, links);
  } else if (entityLinkMode == Preferences::entityLinkModeDirect()) {
    getDirectSelectedLinks(document, defaultColor, selectedColor, links);
  }
}

std::vector<LinkRenderer::LineVertex> EntityLinkRenderer::getLinks() {
  auto document = kdl::mem_lock(m_document);
  auto links = std::vector<LineVertex>{};
  Renderer::getLinks(*document, m_defaultColor, m_selectedColor, links);
  return links;
}
} // namespace Renderer
} // namespace TrenchBroom
