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
#include "Renderer/GroupLinkRenderer.h"
#include "Renderer/ObjectRenderer.h"
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

namespace TrenchBroom
{
namespace Renderer
{
class MapRenderer::SelectedBrushRendererFilter : public BrushRenderer::DefaultFilter
{
public:
  SelectedBrushRendererFilter(const Model::EditorContext& context)
    : DefaultFilter(context)
  {
  }

  RenderSettings markFaces(const Model::BrushNode* brushNode) const override
  {
    if (!(visible(brushNode) && editable(brushNode)))
    {
      return renderNothing();
    }

    const bool brushSelected = selected(brushNode);
    const Model::Brush& brush = brushNode->brush();
    for (const Model::BrushFace& face : brush.faces())
    {
      face.setMarked(brushSelected || selected(brushNode, face));
    }
    return std::make_tuple(
      FaceRenderPolicy::RenderMarked, EdgeRenderPolicy::RenderIfEitherFaceMarked);
  }
};

class MapRenderer::LockedBrushRendererFilter : public BrushRenderer::DefaultFilter
{
public:
  LockedBrushRendererFilter(const Model::EditorContext& context)
    : DefaultFilter(context)
  {
  }

  RenderSettings markFaces(const Model::BrushNode* brushNode) const override
  {
    if (!visible(brushNode))
    {
      return renderNothing();
    }

    const Model::Brush& brush = brushNode->brush();
    for (const Model::BrushFace& face : brush.faces())
    {
      face.setMarked(true);
    }

    return std::make_tuple(FaceRenderPolicy::RenderMarked, EdgeRenderPolicy::RenderAll);
  }
};

class MapRenderer::UnselectedBrushRendererFilter : public BrushRenderer::DefaultFilter
{
public:
  UnselectedBrushRendererFilter(const Model::EditorContext& context)
    : DefaultFilter(context)
  {
  }

  RenderSettings markFaces(const Model::BrushNode* brushNode) const override
  {
    const bool brushVisible = visible(brushNode);
    const bool brushEditable = editable(brushNode);

    const bool renderFaces = (brushVisible && brushEditable);
    bool renderEdges = (brushVisible && !selected(brushNode));

    if (!renderFaces && !renderEdges)
    {
      return renderNothing();
    }

    const Model::Brush& brush = brushNode->brush();

    bool anyFaceVisible = false;
    for (const Model::BrushFace& face : brush.faces())
    {
      const bool faceVisible = !selected(brushNode, face) && visible(brushNode, face);
      face.setMarked(faceVisible);
      anyFaceVisible |= faceVisible;
    }

    if (!anyFaceVisible)
    {
      return renderNothing();
    }

    // Render all edges if only one face is visible.
    renderEdges |= anyFaceVisible;

    return std::make_tuple(
      renderFaces ? FaceRenderPolicy::RenderMarked : FaceRenderPolicy::RenderNone,
      renderEdges ? EdgeRenderPolicy::RenderAll : EdgeRenderPolicy::RenderNone);
  }
};

MapRenderer::MapRenderer(std::weak_ptr<View::MapDocument> document)
  : m_document(document)
  , m_defaultRenderer(createDefaultRenderer(m_document))
  , m_selectionRenderer(createSelectionRenderer(m_document))
  , m_lockedRenderer(createLockRenderer(m_document))
  , m_entityLinkRenderer(std::make_unique<EntityLinkRenderer>(m_document))
  , m_groupLinkRenderer(std::make_unique<GroupLinkRenderer>(m_document))
{
  connectObservers();
  setupRenderers();
}

MapRenderer::~MapRenderer()
{
  clear();
}

std::unique_ptr<ObjectRenderer> MapRenderer::createDefaultRenderer(
  std::weak_ptr<View::MapDocument> document)
{
  return std::make_unique<ObjectRenderer>(
    *kdl::mem_lock(document),
    kdl::mem_lock(document)->entityModelManager(),
    kdl::mem_lock(document)->editorContext(),
    UnselectedBrushRendererFilter(kdl::mem_lock(document)->editorContext()));
}

std::unique_ptr<ObjectRenderer> MapRenderer::createSelectionRenderer(
  std::weak_ptr<View::MapDocument> document)
{
  return std::make_unique<ObjectRenderer>(
    *kdl::mem_lock(document),
    kdl::mem_lock(document)->entityModelManager(),
    kdl::mem_lock(document)->editorContext(),
    SelectedBrushRendererFilter(kdl::mem_lock(document)->editorContext()));
}

std::unique_ptr<ObjectRenderer> MapRenderer::createLockRenderer(
  std::weak_ptr<View::MapDocument> document)
{
  return std::make_unique<ObjectRenderer>(
    *kdl::mem_lock(document),
    kdl::mem_lock(document)->entityModelManager(),
    kdl::mem_lock(document)->editorContext(),
    LockedBrushRendererFilter(kdl::mem_lock(document)->editorContext()));
}

void MapRenderer::clear()
{
  m_defaultRenderer->clear();
  m_selectionRenderer->clear();
  m_lockedRenderer->clear();
  m_entityLinkRenderer->invalidate();
  m_groupLinkRenderer->invalidate();
  m_trackedNodes.clear();
}

void MapRenderer::overrideSelectionColors(const Color& color, const float mix)
{
  const Color edgeColor = pref(Preferences::SelectedEdgeColor).mixed(color, mix);
  const Color occludedEdgeColor = pref(Preferences::SelectedFaceColor).mixed(color, mix);
  const Color tintColor = pref(Preferences::SelectedFaceColor).mixed(color, mix);

  m_selectionRenderer->setEntityBoundsColor(edgeColor);
  m_selectionRenderer->setBrushEdgeColor(edgeColor);
  m_selectionRenderer->setOccludedEdgeColor(occludedEdgeColor);
  m_selectionRenderer->setTintColor(tintColor);
}

void MapRenderer::restoreSelectionColors()
{
  setupSelectionRenderer(*m_selectionRenderer);
}

void MapRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch)
{
  commitPendingChanges();
  setupGL(renderBatch);
  renderDefaultOpaque(renderContext, renderBatch);
  renderLockedOpaque(renderContext, renderBatch);
  renderSelectionOpaque(renderContext, renderBatch);

  renderDefaultTransparent(renderContext, renderBatch);
  renderLockedTransparent(renderContext, renderBatch);
  renderSelectionTransparent(renderContext, renderBatch);

  renderEntityLinks(renderContext, renderBatch);
  renderGroupLinks(renderContext, renderBatch);
}

void MapRenderer::commitPendingChanges()
{
  auto document = kdl::mem_lock(m_document);
  document->commitPendingAssets();
}

class SetupGL : public Renderable
{
private:
  void doRender(RenderContext&) override
  {
    glAssert(glFrontFace(GL_CW));
    glAssert(glEnable(GL_CULL_FACE));
    glAssert(glEnable(GL_DEPTH_TEST));
    glAssert(glDepthFunc(GL_LEQUAL));
    glResetEdgeOffset();
  }
};

void MapRenderer::setupGL(RenderBatch& renderBatch)
{
  renderBatch.addOneShot(new SetupGL());
}

void MapRenderer::renderDefaultOpaque(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_defaultRenderer->setShowOverlays(renderContext.render3D());
  m_defaultRenderer->renderOpaque(renderContext, renderBatch);
}

void MapRenderer::renderDefaultTransparent(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_defaultRenderer->setShowOverlays(renderContext.render3D());
  m_defaultRenderer->renderTransparent(renderContext, renderBatch);
}

void MapRenderer::renderSelectionOpaque(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (!renderContext.hideSelection())
  {
    m_selectionRenderer->renderOpaque(renderContext, renderBatch);
  }
}

void MapRenderer::renderSelectionTransparent(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  if (!renderContext.hideSelection())
  {
    m_selectionRenderer->renderTransparent(renderContext, renderBatch);
  }
}

void MapRenderer::renderLockedOpaque(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_lockedRenderer->setShowOverlays(renderContext.render3D());
  m_lockedRenderer->renderOpaque(renderContext, renderBatch);
}

void MapRenderer::renderLockedTransparent(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_lockedRenderer->setShowOverlays(renderContext.render3D());
  m_lockedRenderer->renderTransparent(renderContext, renderBatch);
}

void MapRenderer::renderEntityLinks(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_entityLinkRenderer->render(renderContext, renderBatch);
}

void MapRenderer::renderGroupLinks(RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_groupLinkRenderer->render(renderContext, renderBatch);
}

void MapRenderer::setupRenderers()
{
  setupDefaultRenderer(*m_defaultRenderer);
  setupSelectionRenderer(*m_selectionRenderer);
  setupLockedRenderer(*m_lockedRenderer);
}

void MapRenderer::setupDefaultRenderer(ObjectRenderer& renderer)
{
  renderer.setEntityOverlayTextColor(pref(Preferences::InfoOverlayTextColor));
  renderer.setGroupOverlayTextColor(pref(Preferences::GroupInfoOverlayTextColor));
  renderer.setOverlayBackgroundColor(pref(Preferences::InfoOverlayBackgroundColor));
  renderer.setTint(false);
  renderer.setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));

  renderer.setGroupBoundsColor(pref(Preferences::DefaultGroupColor));
  renderer.setEntityBoundsColor(pref(Preferences::UndefinedEntityColor));

  renderer.setBrushFaceColor(pref(Preferences::FaceColor));
  renderer.setBrushEdgeColor(pref(Preferences::EdgeColor));
}

void MapRenderer::setupSelectionRenderer(ObjectRenderer& renderer)
{
  renderer.setEntityOverlayTextColor(pref(Preferences::SelectedInfoOverlayTextColor));
  renderer.setGroupOverlayTextColor(pref(Preferences::SelectedInfoOverlayTextColor));
  renderer.setOverlayBackgroundColor(
    pref(Preferences::SelectedInfoOverlayBackgroundColor));
  renderer.setShowBrushEdges(true);
  renderer.setShowOccludedObjects(true);
  renderer.setOccludedEdgeColor(Color(
    pref(Preferences::SelectedEdgeColor), pref(Preferences::OccludedSelectedEdgeAlpha)));
  renderer.setTint(true);
  renderer.setTintColor(pref(Preferences::SelectedFaceColor));

  renderer.setOverrideGroupColors(true);
  renderer.setGroupBoundsColor(pref(Preferences::SelectedEdgeColor));

  renderer.setOverrideEntityBoundsColor(true);
  renderer.setEntityBoundsColor(pref(Preferences::SelectedEdgeColor));
  renderer.setShowEntityAngles(true);
  renderer.setEntityAngleColor(pref(Preferences::AngleIndicatorColor));

  renderer.setBrushFaceColor(pref(Preferences::FaceColor));
  renderer.setBrushEdgeColor(pref(Preferences::SelectedEdgeColor));
}

void MapRenderer::setupLockedRenderer(ObjectRenderer& renderer)
{
  renderer.setEntityOverlayTextColor(pref(Preferences::LockedInfoOverlayTextColor));
  renderer.setGroupOverlayTextColor(pref(Preferences::LockedInfoOverlayTextColor));
  renderer.setOverlayBackgroundColor(pref(Preferences::LockedInfoOverlayBackgroundColor));
  renderer.setShowOccludedObjects(false);
  renderer.setTint(true);
  renderer.setTintColor(pref(Preferences::LockedFaceColor));
  renderer.setTransparencyAlpha(pref(Preferences::TransparentFaceAlpha));

  renderer.setOverrideGroupColors(true);
  renderer.setGroupBoundsColor(pref(Preferences::LockedEdgeColor));

  renderer.setOverrideEntityBoundsColor(true);
  renderer.setEntityBoundsColor(pref(Preferences::LockedEdgeColor));
  renderer.setShowEntityAngles(false);

  renderer.setBrushFaceColor(pref(Preferences::FaceColor));
  renderer.setBrushEdgeColor(pref(Preferences::LockedEdgeColor));
}

static bool selected(const Model::Node* node)
{
  return node->selected() || node->descendantSelected() || node->parentSelected();
}

MapRenderer::Renderer MapRenderer::determineDesiredRenderers(Model::Node* node)
{
  int result = 0;

  node->accept(kdl::overload(
    [](Model::WorldNode*) {},
    [](Model::LayerNode*) {},
    [&](Model::GroupNode* group) {
      if (group->locked())
      {
        result = Renderer_Locked;
      }
      else if (selected(group) || group->opened())
      {
        result = Renderer_Selection;
      }
      else
      {
        result = Renderer_Default;
      }
    },
    [&](Model::EntityNode* entity) {
      if (entity->locked())
      {
        result = Renderer_Locked;
      }
      else if (selected(entity))
      {
        result = Renderer_Selection;
      }
      else
      {
        result = Renderer_Default;
      }
    },
    [&](Model::BrushNode* brush) {
      if (brush->locked())
      {
        result = Renderer_Locked;
      }
      else if (selected(brush) || brush->hasSelectedFaces())
      {
        result = Renderer_Selection;
      }
      if (!brush->selected() && !brush->parentSelected() && !brush->locked())
      {
        result |= Renderer_Default;
      }
    },
    [&](Model::PatchNode* patchNode) {
      if (patchNode->locked())
      {
        result = Renderer_Locked;
      }
      else if (selected(patchNode))
      {
        result = Renderer_Selection;
      }
      if (!patchNode->selected() && !patchNode->parentSelected() && !patchNode->locked())
      {
        result |= Renderer_Default;
      }
    }));
  return static_cast<Renderer>(result);
}

/**
 * - Determine which renderers the given node should be in
 * - Remove from any renderers the node shouldn't be in
 * - Add to desired renderers, if not already present
 * - Invalidate, for any renderers it was already present in
 */
void MapRenderer::updateAndInvalidateNode(Model::Node* node)
{
  const Renderer desiredRenderers = determineDesiredRenderers(node);
  Renderer currentRenderers;

  if (auto it = m_trackedNodes.find(node); it != m_trackedNodes.end())
  {
    currentRenderers = it->second;
  }
  else
  {
    currentRenderers = static_cast<Renderer>(0);
  }

  auto updateForRenderer = [&](const Renderer r, ObjectRenderer* o) {
    const bool isRDesired = (desiredRenderers & r) != 0;
    const bool isRCurrent = (currentRenderers & r) != 0;

    if (isRCurrent && !isRDesired)
    {
      o->removeNode(node);
    }
    else if (!isRCurrent && isRDesired)
    {
      o->addNode(node);
    }
    else if (isRCurrent && isRDesired)
    {
      o->invalidateNode(node);
    }
  };

  updateForRenderer(Renderer_Default, m_defaultRenderer.get());
  updateForRenderer(Renderer_Selection, m_selectionRenderer.get());
  updateForRenderer(Renderer_Locked, m_lockedRenderer.get());

  // Update the metadata to reflect the changes that we made above
  m_trackedNodes[node] = desiredRenderers;
}

void MapRenderer::updateAndInvalidateNodeRecursive(Model::Node* node)
{
  node->accept(kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [&](auto&& thisLambda, Model::GroupNode* group) {
      updateAndInvalidateNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* entity) {
      updateAndInvalidateNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) { updateAndInvalidateNode(brush); },
    [&](Model::PatchNode* patchNode) { updateAndInvalidateNode(patchNode); }));

  // Due to the definition of `selected()` above, we also need to update the parent.
  // (not recursively, though, so this has little performance impact.)
  // This handles clicking on a brush in a brush entity -> the entity label needs to
  // render as selected.
  if (node->parent())
  {
    updateAndInvalidateNode(node->parent());
  }
}

void MapRenderer::removeNode(Model::Node* node)
{
  if (auto it = m_trackedNodes.find(node); it != m_trackedNodes.end())
  {
    const Renderer renderers = it->second;

    if (renderers & Renderer_Default)
    {
      m_defaultRenderer->removeNode(node);
    }
    if (renderers & Renderer_Selection)
    {
      m_selectionRenderer->removeNode(node);
    }
    if (renderers & Renderer_Locked)
    {
      m_lockedRenderer->removeNode(node);
    }

    m_trackedNodes.erase(it);

    // At this point, none of the default/selection/locked renderers,
    // or their underlying node-type specific renderers, have a reference
    // to `node` anymore, and they won't render it.
  }
}

void MapRenderer::removeNodeRecursive(Model::Node* node)
{
  node->accept(kdl::overload(
    [](auto&& thisLambda, Model::WorldNode* world) { world->visitChildren(thisLambda); },
    [](auto&& thisLambda, Model::LayerNode* layer) { layer->visitChildren(thisLambda); },
    [&](auto&& thisLambda, Model::GroupNode* group) {
      removeNode(group);
      group->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* entity) {
      removeNode(entity);
      entity->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* brush) { removeNode(brush); },
    [&](Model::PatchNode* patchNode) { removeNode(patchNode); }));
}

/**
 * Calls updateAndInvalidateNode() on every node in the world.
 */
void MapRenderer::updateAllNodes()
{
  auto document = kdl::mem_lock(m_document);
  updateAndInvalidateNodeRecursive(document->world());
}

/**
 * Marks the nodes that are already tracked in the given renderers as invalid, i.e.
 * needing to be re-rendered.
 */
void MapRenderer::invalidateRenderers(Renderer renderers)
{
  if ((renderers & Renderer_Default) != 0)
    m_defaultRenderer->invalidate();
  if ((renderers & Renderer_Selection) != 0)
    m_selectionRenderer->invalidate();
  if ((renderers & Renderer_Locked) != 0)
    m_lockedRenderer->invalidate();
}

void MapRenderer::invalidateEntityLinkRenderer()
{
  m_entityLinkRenderer->invalidate();
}

void MapRenderer::invalidateGroupLinkRenderer()
{
  m_groupLinkRenderer->invalidate();
}

void MapRenderer::reloadEntityModels()
{
  m_defaultRenderer->reloadModels();
  m_selectionRenderer->reloadModels();
  m_lockedRenderer->reloadModels();
}

void MapRenderer::connectObservers()
{
  assert(!kdl::mem_expired(m_document));
  auto document = kdl::mem_lock(m_document);

  m_notifierConnection +=
    document->documentWasClearedNotifier.connect(this, &MapRenderer::documentWasCleared);
  m_notifierConnection += document->documentWasNewedNotifier.connect(
    this, &MapRenderer::documentWasNewedOrLoaded);
  m_notifierConnection += document->documentWasLoadedNotifier.connect(
    this, &MapRenderer::documentWasNewedOrLoaded);
  m_notifierConnection +=
    document->nodesWereAddedNotifier.connect(this, &MapRenderer::nodesWereAdded);
  m_notifierConnection +=
    document->nodesWereRemovedNotifier.connect(this, &MapRenderer::nodesWereRemoved);
  m_notifierConnection +=
    document->nodesDidChangeNotifier.connect(this, &MapRenderer::nodesDidChange);
  m_notifierConnection += document->nodeVisibilityDidChangeNotifier.connect(
    this, &MapRenderer::nodeVisibilityDidChange);
  m_notifierConnection += document->nodeLockingDidChangeNotifier.connect(
    this, &MapRenderer::nodeLockingDidChange);
  m_notifierConnection +=
    document->groupWasOpenedNotifier.connect(this, &MapRenderer::groupWasOpened);
  m_notifierConnection +=
    document->groupWasClosedNotifier.connect(this, &MapRenderer::groupWasClosed);
  m_notifierConnection += document->brushFacesDidChangeNotifier.connect(
    this, &MapRenderer::brushFacesDidChange);
  m_notifierConnection +=
    document->selectionDidChangeNotifier.connect(this, &MapRenderer::selectionDidChange);
  m_notifierConnection += document->textureCollectionsWillChangeNotifier.connect(
    this, &MapRenderer::textureCollectionsWillChange);
  m_notifierConnection += document->entityDefinitionsDidChangeNotifier.connect(
    this, &MapRenderer::entityDefinitionsDidChange);
  m_notifierConnection +=
    document->modsDidChangeNotifier.connect(this, &MapRenderer::modsDidChange);
  m_notifierConnection += document->editorContextDidChangeNotifier.connect(
    this, &MapRenderer::editorContextDidChange);

  PreferenceManager& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapRenderer::preferenceDidChange);
}

void MapRenderer::documentWasCleared(View::MapDocument*)
{
  clear();
}

void MapRenderer::documentWasNewedOrLoaded(View::MapDocument*)
{
  clear();
  updateAllNodes();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesWereAdded(const std::vector<Model::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    // The nodes passed in don't include recursive children, so we need to visit them
    // ourselves.
    updateAndInvalidateNodeRecursive(node);
  }
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesWereRemoved(const std::vector<Model::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    // The nodes passed in don't include recursive children, so we need to visit them
    // ourselves. Otherwise deleting a group doesn't delete the brushes within.
    removeNodeRecursive(node);
  }
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesDidChange(const std::vector<Model::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    // nodesDidChange() will report ancestors changing, e.g. the world and layer are
    // reported as changing when a brush is dragged. So, don't update recursively here as
    // it would cause the entire map to be invalidated on every change.
    updateAndInvalidateNode(node);
  }
  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::nodeVisibilityDidChange(const std::vector<Model::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    updateAndInvalidateNodeRecursive(node);
  }
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodeLockingDidChange(const std::vector<Model::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    updateAndInvalidateNodeRecursive(node);
  }
  invalidateEntityLinkRenderer();
}

void MapRenderer::groupWasOpened(Model::GroupNode*)
{
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::groupWasClosed(Model::GroupNode*)
{
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::brushFacesDidChange(const std::vector<Model::BrushFaceHandle>& faces)
{
  for (const auto& face : faces)
  {
    updateAndInvalidateNode(face.node());
  }
}

void MapRenderer::selectionDidChange(const View::Selection& selection)
{
  for (const auto& face : selection.deselectedBrushFaces())
  {
    updateAndInvalidateNode(face.node());
  }
  for (const auto& face : selection.selectedBrushFaces())
  {
    updateAndInvalidateNode(face.node());
  }
  // These need to be recursive otherwise selecting a Group doesn't render the contents
  // selected
  for (auto* node : selection.deselectedNodes())
  {
    updateAndInvalidateNodeRecursive(node);
  }
  for (auto* node : selection.selectedNodes())
  {
    updateAndInvalidateNodeRecursive(node);
  }

  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::textureCollectionsWillChange()
{
  invalidateRenderers(Renderer_All);
}

void MapRenderer::entityDefinitionsDidChange()
{
  reloadEntityModels();
  invalidateRenderers(Renderer_All);
  invalidateEntityLinkRenderer();
}

void MapRenderer::modsDidChange()
{
  reloadEntityModels();
  invalidateRenderers(Renderer_All);
  invalidateEntityLinkRenderer();
}

void MapRenderer::editorContextDidChange()
{
  invalidateRenderers(Renderer_All);
  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::preferenceDidChange(const IO::Path& path)
{
  setupRenderers();

  auto document = kdl::mem_lock(m_document);
  if (document->isGamePathPreference(path))
  {
    reloadEntityModels();
    invalidateRenderers(Renderer_All);
    invalidateEntityLinkRenderer();
    invalidateGroupLinkRenderer();
  }

  if (path.hasPrefix(IO::Path("Map view"), true))
  {
    invalidateRenderers(Renderer_All);
    invalidateEntityLinkRenderer();
    invalidateGroupLinkRenderer();
  }
}
} // namespace Renderer
} // namespace TrenchBroom
