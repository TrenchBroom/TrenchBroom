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

#include "MapRenderer.h"

#include "Assets/EntityDefinitionManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/EntityLinkRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/GroupLinkRenderer.h"
#include "Renderer/GroupRenderer.h"
#include "Renderer/PatchRenderer.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "View/MapDocument.h"
#include "View/Selection.h"

#include <kdl/memory_utils.h>
#include <kdl/overload.h>
#include <kdl/vector_set.h>

#include <set>
#include <vector>

#include <QDebug>

namespace TrenchBroom {
namespace Renderer {
MapRenderer::MapRenderer(std::weak_ptr<View::MapDocument> document)
  : m_document(document)
  , m_groupRenderer(std::make_unique<GroupRenderer>(kdl::mem_lock(document)->editorContext()))
  , m_entityRenderer(std::make_unique<EntityRenderer>(
      *kdl::mem_lock(document), kdl::mem_lock(document)->entityModelManager(),
      kdl::mem_lock(document)->editorContext()))
  , m_entityLinkRenderer(std::make_unique<EntityLinkRenderer>(m_document))
  , m_brushRenderer(std::make_unique<BrushRenderer>(kdl::mem_lock(document)->editorContext()))
  , m_patchRenderer(std::make_unique<PatchRenderer>())
  , m_groupLinkRenderer(std::make_unique<GroupLinkRenderer>(m_document)) {
  connectObservers();
  setupRenderers();
}

MapRenderer::~MapRenderer() {
  clear();
}

void MapRenderer::clear() {
  m_groupRenderer->clear();
  m_entityRenderer->clear();
  m_brushRenderer->clear();
  m_patchRenderer->clear();
  m_entityLinkRenderer->invalidate();
  m_groupLinkRenderer->invalidate();
  m_trackedNodes.clear();
}

/**
 * Used to flash the selection color e.g. when duplicating
 */
void MapRenderer::overrideSelectionColors(const Color& color, const float mix) {
  const Color edgeColor = pref(Preferences::SelectedEdgeColor).mixed(color, mix);
  const Color occludedEdgeColor = pref(Preferences::SelectedFaceColor).mixed(color, mix);
  const Color tintColor = pref(Preferences::SelectedFaceColor).mixed(color, mix);

  // m_selectionRenderer->setEntityBoundsColor(edgeColor);
  // m_selectionRenderer->setBrushEdgeColor(edgeColor);
  //  m_selectionRenderer->setOccludedEdgeColor(occludedEdgeColor);
  //  m_selectionRenderer->setTintColor(tintColor);
}

void MapRenderer::restoreSelectionColors() {
  // setupSelectionRenderer(*m_selectionRenderer);
}

void MapRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
  commitPendingChanges();
  setupGL(renderBatch);
  renderOpaque(renderContext, renderBatch);

  renderTransparent(renderContext, renderBatch);

  renderEntityLinks(renderContext, renderBatch);
  renderGroupLinks(renderContext, renderBatch);
}

void MapRenderer::commitPendingChanges() {
  auto document = kdl::mem_lock(m_document);
  document->commitPendingAssets();
}

class SetupGL : public Renderable {
private:
  void doRender(RenderContext&) override {
    glAssert(glFrontFace(GL_CW));
    glAssert(glEnable(GL_CULL_FACE));
    glAssert(glEnable(GL_DEPTH_TEST));
    glAssert(glDepthFunc(GL_LEQUAL));
    glResetEdgeOffset();
  }
};

void MapRenderer::setupGL(RenderBatch& renderBatch) {
  renderBatch.addOneShot(new SetupGL());
}

void MapRenderer::renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch) {
  m_brushRenderer->renderOpaque(renderContext, renderBatch);

  m_entityRenderer->setShowOverlays(renderContext.render3D());
  m_entityRenderer->render(renderContext, renderBatch);

  m_groupRenderer->setShowOverlays(renderContext.render3D());
  m_groupRenderer->render(renderContext, renderBatch);

  m_patchRenderer->render(renderContext, renderBatch);

  // FIXME: make sure selection doesn't render if !renderContext.hideSelection()
}

void MapRenderer::renderTransparent(RenderContext& renderContext, RenderBatch& renderBatch) {
  m_brushRenderer->renderTransparent(renderContext, renderBatch);
}

void MapRenderer::renderEntityLinks(RenderContext& renderContext, RenderBatch& renderBatch) {
  m_entityLinkRenderer->render(renderContext, renderBatch);
}

void MapRenderer::renderGroupLinks(RenderContext& renderContext, RenderBatch& renderBatch) {
  m_groupLinkRenderer->render(renderContext, renderBatch);
}

void MapRenderer::setupRenderers() {
  setupDefaultRenderer();
  setupSelectionRenderer();
  setupLockedRenderer();
  setupEntityLinkRenderer();
}

void MapRenderer::setupEntityLinkRenderer() {}

void MapRenderer::setupDefaultRenderer() {
  m_entityRenderer->setOverlayTextColor(
    RenderType::Default, pref(Preferences::InfoOverlayTextColor));
  m_groupRenderer->setOverlayTextColor(
    RenderType::Default, pref(Preferences::GroupInfoOverlayTextColor));
  m_entityRenderer->setOverlayBackgroundColor(
    RenderType::Default, pref(Preferences::InfoOverlayBackgroundColor));
  m_groupRenderer->setOverlayBackgroundColor(
    RenderType::Default, pref(Preferences::InfoOverlayBackgroundColor));
#if 0            
            renderer.setTint(false);
            renderer.setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));

            renderer.setGroupBoundsColor(pref(Preferences::DefaultGroupColor));
            renderer.setEntityBoundsColor(pref(Preferences::UndefinedEntityColor));
#endif
  m_brushRenderer->setFaceColor(
    pref(Preferences::FaceColor)); // Selected/Locked also use this color
  m_patchRenderer->setDefaultColor(
    pref(Preferences::FaceColor)); // Selected/Locked also use this color
  m_brushRenderer->setEdgeColor(RenderType::Default, pref(Preferences::EdgeColor));
  m_patchRenderer->setEdgeColor(RenderType::Default, pref(Preferences::EdgeColor));
}

void MapRenderer::setupSelectionRenderer() {
  m_entityRenderer->setOverlayTextColor(
    RenderType::Selected, pref(Preferences::SelectedInfoOverlayTextColor));
  m_groupRenderer->setOverlayTextColor(
    RenderType::Selected, pref(Preferences::SelectedInfoOverlayTextColor));
  m_entityRenderer->setOverlayBackgroundColor(
    RenderType::Selected, pref(Preferences::SelectedInfoOverlayBackgroundColor));
  m_groupRenderer->setOverlayBackgroundColor(
    RenderType::Selected, pref(Preferences::SelectedInfoOverlayBackgroundColor));
#if 0                        
            renderer.setShowBrushEdges(true);
            renderer.setShowOccludedObjects(true);
            renderer.setOccludedEdgeColor(Color(pref(Preferences::SelectedEdgeColor), pref(Preferences::OccludedSelectedEdgeAlpha)));
            renderer.setTint(true);
            renderer.setTintColor(pref(Preferences::SelectedFaceColor));

            renderer.setOverrideGroupColors(true);
            renderer.setGroupBoundsColor(pref(Preferences::SelectedEdgeColor));

            renderer.setOverrideEntityBoundsColor(true);
            renderer.setEntityBoundsColor(pref(Preferences::SelectedEdgeColor));
            renderer.setShowEntityAngles(true);
            renderer.setEntityAngleColor(pref(Preferences::AngleIndicatorColor));
#endif
  m_brushRenderer->setEdgeColor(RenderType::Selected, pref(Preferences::SelectedEdgeColor));
  m_patchRenderer->setEdgeColor(RenderType::Selected, pref(Preferences::SelectedEdgeColor));
}

void MapRenderer::setupLockedRenderer() {
  m_entityRenderer->setOverlayTextColor(
    RenderType::Locked, pref(Preferences::LockedInfoOverlayTextColor));
  m_groupRenderer->setOverlayTextColor(
    RenderType::Locked, pref(Preferences::LockedInfoOverlayTextColor));
  m_entityRenderer->setOverlayBackgroundColor(
    RenderType::Locked, pref(Preferences::LockedInfoOverlayBackgroundColor));
  m_groupRenderer->setOverlayBackgroundColor(
    RenderType::Locked, pref(Preferences::LockedInfoOverlayBackgroundColor));
#if 0                        
            renderer.setShowOccludedObjects(false);
            renderer.setTint(true);
            renderer.setTintColor(pref(Preferences::LockedFaceColor));
            renderer.setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));

            renderer.setOverrideGroupColors(true);
            renderer.setGroupBoundsColor(pref(Preferences::LockedEdgeColor));

            renderer.setOverrideEntityBoundsColor(true);
            renderer.setEntityBoundsColor(pref(Preferences::LockedEdgeColor));
            renderer.setShowEntityAngles(false);
#endif
  m_brushRenderer->setEdgeColor(RenderType::Locked, pref(Preferences::LockedEdgeColor));
  m_patchRenderer->setEdgeColor(RenderType::Locked, pref(Preferences::LockedEdgeColor));
}

// FIXME: make sure BrushRenderer etc. are using this predicate (moved to RenderUtils)
// static bool selected(const Model::Node* node) {
//    return node->selected() || node->descendantSelected() || node->parentSelected();
//}

/**
 * - Determine which renderers the given node should be in
 * - Remove from any renderers the node shouldn't be in
 * - Add to desired renderers, if not already present
 * - Invalidate, for any renderers it was already present in
 */
void MapRenderer::updateAndInvalidateNode(Model::Node* node) {
  bool adding = false;

  if (auto it = m_trackedNodes.find(node); it == m_trackedNodes.end()) {
    adding = true;
    m_trackedNodes.insert(node);
  }

  node->accept(kdl::overload(
    [](Model::WorldNode*) {}, [](Model::LayerNode*) {},
    [&](Model::GroupNode* group) {
      if (adding) {
        m_groupRenderer->addGroup(group);
      } else {
        m_groupRenderer->invalidateGroup(group);
      }
    },
    [&](Model::EntityNode* entity) {
      if (adding) {
        m_entityRenderer->addEntity(entity);
      } else {
        m_entityRenderer->invalidateEntity(entity);
      }
    },
    [&](Model::BrushNode* brush) {
      if (adding) {
        m_brushRenderer->addBrush(brush);
      } else {
        m_brushRenderer->invalidateBrush(brush);
      }
    },
    [&](Model::PatchNode* patchNode) {
      if (adding) {
        m_patchRenderer->addPatch(patchNode);
      } else {
        m_patchRenderer->invalidatePatch(patchNode);
      }
    }));
}

void MapRenderer::updateAndInvalidateNodeRecursive(Model::Node* node) {
  node->accept(kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::GroupNode* group) {
      updateAndInvalidateNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* entity) {
      updateAndInvalidateNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) {
      updateAndInvalidateNode(brush);
    },
    [&](Model::PatchNode* patchNode) {
      updateAndInvalidateNode(patchNode);
    }));

  // Due to the definition of `selected()` above, we also need to update the parent.
  // (not recursively, though, so this has little performance impact.)
  // This handles clicking on a brush in a brush entity -> the entity label needs to render as
  // selected.
  if (node->parent()) {
    updateAndInvalidateNode(node->parent());
  }
}

void MapRenderer::removeNode(Model::Node* node) {
  node->accept(kdl::overload(
    [](Model::WorldNode*) {}, [](Model::LayerNode*) {},
    [&](Model::GroupNode* group) {
      m_groupRenderer->removeGroup(group);
    },
    [&](Model::EntityNode* entity) {
      m_entityRenderer->removeEntity(entity);
    },
    [&](Model::BrushNode* brush) {
      m_brushRenderer->removeBrush(brush);
    },
    [&](Model::PatchNode* patchNode) {
      m_patchRenderer->removePatch(patchNode);
    }));

  m_trackedNodes.erase(node);

  // At this point, none of the node-type specific renderers have a reference
  // to `node` anymore, and they won't render it.
}

void MapRenderer::removeNodeRecursive(Model::Node* node) {
  node->accept(kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) {
      world->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, Model::LayerNode* layer) {
      layer->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::GroupNode* group) {
      removeNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* entity) {
      removeNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) {
      removeNode(brush);
    },
    [&](Model::PatchNode* patchNode) {
      removeNode(patchNode);
    }));
}

/**
 * Calls updateAndInvalidateNode() on every node in the world.
 */
void MapRenderer::updateAllNodes() {
  auto document = kdl::mem_lock(m_document);
  updateAndInvalidateNodeRecursive(document->world());
}

/**
 * Marks the nodes that are already tracked as invalid, i.e.
 * needing to be re-rendered.
 */
void MapRenderer::invalidateRenderers() {
  m_groupRenderer->invalidate();
  m_entityRenderer->invalidate();
  m_brushRenderer->invalidate();
  m_patchRenderer->invalidate();
}

void MapRenderer::invalidateEntityLinkRenderer() {
  m_entityLinkRenderer->invalidate();
}

void MapRenderer::invalidateGroupLinkRenderer() {
  m_groupLinkRenderer->invalidate();
}

void MapRenderer::reloadEntityModels() {
  m_entityRenderer->reloadModels();
}

void MapRenderer::connectObservers() {
  assert(!kdl::mem_expired(m_document));
  auto document = kdl::mem_lock(m_document);

  m_notifierConnection +=
    document->documentWasClearedNotifier.connect(this, &MapRenderer::documentWasCleared);
  m_notifierConnection +=
    document->documentWasNewedNotifier.connect(this, &MapRenderer::documentWasNewedOrLoaded);
  m_notifierConnection +=
    document->documentWasLoadedNotifier.connect(this, &MapRenderer::documentWasNewedOrLoaded);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &MapRenderer::nodesWereAdded);
  m_notifierConnection +=
    document->nodesWereRemovedNotifier.connect(this, &MapRenderer::nodesWereRemoved);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &MapRenderer::nodesDidChange);
  m_notifierConnection +=
    document->nodeVisibilityDidChangeNotifier.connect(this, &MapRenderer::nodeVisibilityDidChange);
  m_notifierConnection +=
    document->nodeLockingDidChangeNotifier.connect(this, &MapRenderer::nodeLockingDidChange);
  m_notifierConnection +=
    document->groupWasOpenedNotifier.connect(this, &MapRenderer::groupWasOpened);
  m_notifierConnection +=
    document->groupWasClosedNotifier.connect(this, &MapRenderer::groupWasClosed);
  m_notifierConnection +=
    document->brushFacesDidChangeNotifier.connect(this, &MapRenderer::brushFacesDidChange);
  m_notifierConnection +=
    document->selectionDidChangeNotifier.connect(this, &MapRenderer::selectionDidChange);
  m_notifierConnection += document->textureCollectionsWillChangeNotifier.connect(
    this, &MapRenderer::textureCollectionsWillChange);
  m_notifierConnection += document->entityDefinitionsDidChangeNotifier.connect(
    this, &MapRenderer::entityDefinitionsDidChange);
  m_notifierConnection +=
    document->modsDidChangeNotifier.connect(this, &MapRenderer::modsDidChange);
  m_notifierConnection +=
    document->editorContextDidChangeNotifier.connect(this, &MapRenderer::editorContextDidChange);

  PreferenceManager& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapRenderer::preferenceDidChange);
}

static void debugLog(const char* functionName) {
  qDebug() << functionName;
}

static void debugLog(const char* functionName, const std::vector<Model::Node*>& nodes) {
  qDebug() << functionName << nodes.size() << "nodes";
}

static void debugLog(const char* functionName, const Model::Node*) {
  qDebug() << functionName << "1 node";
}

static void debugLog(const char* functionName, const std::vector<Model::BrushFaceHandle>& faces) {
  qDebug() << functionName << faces.size() << "face handles";
}

static void debugLog(const char* functionName, const View::Selection& selection) {
  qDebug() << functionName
           << QString::fromLatin1(
                "%1/%2 nodes selected/deselected, %3/%4 faces selected/deselected")
                .arg(selection.selectedNodes().size())
                .arg(selection.deselectedNodes().size())
                .arg(selection.selectedBrushFaces().size())
                .arg(selection.deselectedBrushFaces().size());
}

void MapRenderer::documentWasCleared(View::MapDocument*) {
  debugLog(__func__);
  clear();
}

void MapRenderer::documentWasNewedOrLoaded(View::MapDocument*) {
  debugLog(__func__);
  clear();
  updateAllNodes();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesWereAdded(const std::vector<Model::Node*>& nodes) {
  debugLog(__func__, nodes);
  for (auto* node : nodes) {
    // The nodes passed in don't include recursive children, so we need to visit them ourselves.
    updateAndInvalidateNodeRecursive(node);
  }
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesWereRemoved(const std::vector<Model::Node*>& nodes) {
  debugLog(__func__, nodes);
  for (auto* node : nodes) {
    // The nodes passed in don't include recursive children, so we need to visit them ourselves.
    // Otherwise deleting a group doesn't delete the brushes within.
    removeNodeRecursive(node);
  }
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesDidChange(const std::vector<Model::Node*>& nodes) {
  debugLog(__func__, nodes);
  for (auto* node : nodes) {
    // nodesDidChange() will report ancestors changing, e.g. the world and layer are reported as
    // changing when a brush is dragged. So, don't update recursively here as it would cause
    // the entire map to be invalidated on every change.
    updateAndInvalidateNode(node);
  }
  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::nodeVisibilityDidChange(const std::vector<Model::Node*>& nodes) {
  debugLog(__func__, nodes);
  for (auto* node : nodes) {
    updateAndInvalidateNodeRecursive(node);
  }
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodeLockingDidChange(const std::vector<Model::Node*>& nodes) {
  debugLog(__func__, nodes);
  for (auto* node : nodes) {
    updateAndInvalidateNodeRecursive(node);
  }
  invalidateEntityLinkRenderer();
}

void MapRenderer::groupWasOpened(Model::GroupNode* group) {
  debugLog(__func__, group);
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::groupWasClosed(Model::GroupNode* group) {
  debugLog(__func__, group);
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces) {
  debugLog(__func__, faces);
  for (const auto& face : faces) {
    updateAndInvalidateNode(face.node());
  }
}

void MapRenderer::selectionDidChange(const View::Selection& selection) {
  debugLog(__func__, selection);

  for (const auto& face : selection.deselectedBrushFaces()) {
    updateAndInvalidateNode(face.node());
  }
  for (const auto& face : selection.selectedBrushFaces()) {
    updateAndInvalidateNode(face.node());
  }
  // These need to be recursive otherwise selecting a Group doesn't render the contents selected
  for (auto* node : selection.deselectedNodes()) {
    updateAndInvalidateNodeRecursive(node);
  }
  for (auto* node : selection.selectedNodes()) {
    updateAndInvalidateNodeRecursive(node);
  }

  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::textureCollectionsWillChange() {
  debugLog(__func__);
  invalidateRenderers();
}

void MapRenderer::entityDefinitionsDidChange() {
  reloadEntityModels();
  invalidateRenderers();
  invalidateEntityLinkRenderer();
}

void MapRenderer::modsDidChange() {
  reloadEntityModels();
  invalidateRenderers();
  invalidateEntityLinkRenderer();
}

void MapRenderer::editorContextDidChange() {
  invalidateRenderers();
  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::preferenceDidChange(const IO::Path& path) {
  setupRenderers();

  auto document = kdl::mem_lock(m_document);
  if (document->isGamePathPreference(path)) {
    reloadEntityModels();
    invalidateRenderers();
    invalidateEntityLinkRenderer();
    invalidateGroupLinkRenderer();
  }

  if (path.hasPrefix(IO::Path("Map view"), true)) {
    invalidateRenderers();
    invalidateEntityLinkRenderer();
    invalidateGroupLinkRenderer();
  }
}
} // namespace Renderer
} // namespace TrenchBroom
