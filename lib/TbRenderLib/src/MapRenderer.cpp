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

#include "render/MapRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "gl/MaterialManager.h"
#include "gl/ResourceManager.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityModelManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GameInfo.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Node.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/SelectionChange.h"
#include "mdl/WorldNode.h"
#include "render/BrushRenderer.h"
#include "render/EntityDecalRenderer.h"
#include "render/EntityLinkRenderer.h"
#include "render/GroupLinkRenderer.h"
#include "render/ObjectRenderer.h"
#include "render/RenderBatch.h"
#include "render/RenderContext.h"

#include "kd/overload.h"
#include "kd/path_utils.h"

#include <vector>

namespace tb::render
{
namespace
{

class SelectedBrushRendererFilter : public BrushRenderer::DefaultFilter
{
public:
  explicit SelectedBrushRendererFilter(const mdl::EditorContext& context)
    : DefaultFilter{context}
  {
  }

  RenderSettings markFaces(const mdl::BrushNode& brushNode) const override
  {
    if (!(visible(brushNode) && editable(brushNode)))
    {
      return renderNothing();
    }

    const auto brushSelected = selected(brushNode);
    const auto& brush = brushNode.brush();
    for (const auto& face : brush.faces())
    {
      face.setMarked(brushSelected || selected(brushNode, face));
    }
    return {FaceRenderPolicy::RenderMarked, EdgeRenderPolicy::RenderIfEitherFaceMarked};
  }
};

class LockedBrushRendererFilter : public BrushRenderer::DefaultFilter
{
public:
  explicit LockedBrushRendererFilter(const mdl::EditorContext& context)
    : DefaultFilter{context}
  {
  }

  RenderSettings markFaces(const mdl::BrushNode& brushNode) const override
  {
    if (!visible(brushNode))
    {
      return renderNothing();
    }

    const auto& brush = brushNode.brush();
    bool hasMarkedFaces = false;
    for (const auto& face : brush.faces())
    {
      const bool mark = visible(brushNode, face);
      face.setMarked(mark);
      hasMarkedFaces |= mark;
    }

    if (!hasMarkedFaces)
    {
      return renderNothing();
    }

    return {FaceRenderPolicy::RenderMarked, EdgeRenderPolicy::RenderAll};
  }
};

class UnselectedBrushRendererFilter : public BrushRenderer::DefaultFilter
{
public:
  explicit UnselectedBrushRendererFilter(const mdl::EditorContext& context)
    : DefaultFilter{context}
  {
  }

  RenderSettings markFaces(const mdl::BrushNode& brushNode) const override
  {
    const auto brushVisible = visible(brushNode);
    const auto brushEditable = editable(brushNode);

    const auto renderFaces = (brushVisible && brushEditable);
    auto renderEdges = (brushVisible && !selected(brushNode));

    if (!renderFaces && !renderEdges)
    {
      return renderNothing();
    }

    const auto& brush = brushNode.brush();

    auto anyFaceVisible = false;
    for (const auto& face : brush.faces())
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

    return {
      renderFaces ? FaceRenderPolicy::RenderMarked : FaceRenderPolicy::RenderNone,
      renderEdges ? EdgeRenderPolicy::RenderAll : EdgeRenderPolicy::RenderNone};
  }
};

std::unique_ptr<ObjectRenderer> createDefaultRenderer(mdl::Map& map)
{
  return std::make_unique<ObjectRenderer>(
    map.logger(),
    map.entityModelManager(),
    map.editorContext(),
    UnselectedBrushRendererFilter{map.editorContext()});
}

std::unique_ptr<ObjectRenderer> createSelectionRenderer(mdl::Map& map)
{
  return std::make_unique<ObjectRenderer>(
    map.logger(),
    map.entityModelManager(),
    map.editorContext(),
    SelectedBrushRendererFilter{map.editorContext()});
}

std::unique_ptr<ObjectRenderer> createLockRenderer(mdl::Map& map)
{
  return std::make_unique<ObjectRenderer>(
    map.logger(),
    map.entityModelManager(),
    map.editorContext(),
    LockedBrushRendererFilter{map.editorContext()});
}

std::unique_ptr<EntityDecalRenderer> createEntityDecalRenderer(mdl::Map& map)
{
  return std::make_unique<EntityDecalRenderer>(map);
}

} // namespace

MapRenderer::MapRenderer(mdl::Map& map)
  : m_map{map}
  , m_defaultRenderer{createDefaultRenderer(m_map)}
  , m_selectionRenderer{createSelectionRenderer(m_map)}
  , m_lockedRenderer{createLockRenderer(m_map)}
  , m_entityDecalRenderer{createEntityDecalRenderer(m_map)}
  , m_entityLinkRenderer{std::make_unique<EntityLinkRenderer>(m_map)}
  , m_groupLinkRenderer{std::make_unique<GroupLinkRenderer>(m_map)}
{
  connectObservers();
  setupRenderers();
  updateAllNodes();
}

MapRenderer::~MapRenderer() = default;

void MapRenderer::overrideSelectionColors(const Color& color, const float mix)
{
  const auto edgeColor =
    mixColors(pref(Preferences::SelectedEdgeColor).to<RgbaF>(), color.to<RgbaF>(), mix);
  const auto occludedEdgeColor =
    mixColors(pref(Preferences::SelectedFaceColor).to<RgbaF>(), color.to<RgbaF>(), mix);
  const auto tintColor =
    mixColors(pref(Preferences::SelectedFaceColor).to<RgbaF>(), color.to<RgbaF>(), mix);

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
  setupGL(renderBatch);
  renderEntityDecals(renderContext, renderBatch);
  renderEntityLinks(renderContext, renderBatch);
  renderGroupLinks(renderContext, renderBatch);

  renderDefaultOpaque(renderContext, renderBatch);
  renderLockedOpaque(renderContext, renderBatch);
  renderSelectionOpaque(renderContext, renderBatch);

  renderDefaultTransparent(renderContext, renderBatch);
  renderLockedTransparent(renderContext, renderBatch);
  renderSelectionTransparent(renderContext, renderBatch);
}

class SetupGL : public Renderable
{
  void render(RenderContext&) override
  {
    glAssert(glFrontFace(GL_CW));
    glAssert(glEnable(GL_CULL_FACE));
    glAssert(glEnable(GL_DEPTH_TEST));
    glAssert(glDepthFunc(GL_LEQUAL));
    gl::glResetEdgeOffset();
  }
};

void MapRenderer::setupGL(RenderBatch& renderBatch)
{
  renderBatch.addOneShot(new SetupGL{});
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

void MapRenderer::renderEntityDecals(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  // only render decals in the 3D view
  if (renderContext.render3D())
  {
    m_entityDecalRenderer->render(renderContext, renderBatch);
  }
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
  renderer.setOccludedEdgeColor(RgbaF{
    pref(Preferences::SelectedEdgeColor).to<RgbF>(),
    pref(Preferences::OccludedSelectedEdgeAlpha)});
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

static bool selected(const mdl::Node* node)
{
  return node->selected() || node->descendantSelected() || node->parentSelected();
}

int MapRenderer::determineDesiredRenderers(mdl::Node& node)
{
  int result = 0;

  node.accept(kdl::overload(
    [](mdl::WorldNode*) {},
    [](mdl::LayerNode*) {},
    [&](mdl::GroupNode* group) {
      if (group->locked())
      {
        result = int(Renderer::Locked);
      }
      else if (selected(group) || group->opened())
      {
        result = int(Renderer::Selection);
      }
      else
      {
        result = int(Renderer::Default);
      }
    },
    [&](mdl::EntityNode* entity) {
      if (entity->locked())
      {
        result = int(Renderer::Locked);
      }
      else if (selected(entity))
      {
        result = int(Renderer::Selection);
      }
      else
      {
        result = int(Renderer::Default);
      }
    },
    [&](mdl::BrushNode* brush) {
      if (brush->locked())
      {
        result = int(Renderer::Locked);
      }
      else if (selected(brush) || brush->hasSelectedFaces())
      {
        result = int(Renderer::Selection);
      }
      if (!brush->selected() && !brush->parentSelected() && !brush->locked())
      {
        result |= int(Renderer::Default);
      }
    },
    [&](mdl::PatchNode* patchNode) {
      if (patchNode->locked())
      {
        result = int(Renderer::Locked);
      }
      else if (selected(patchNode))
      {
        result = int(Renderer::Selection);
      }
      if (!patchNode->selected() && !patchNode->parentSelected() && !patchNode->locked())
      {
        result |= int(Renderer::Default);
      }
    }));
  return result;
}

/**
 * - Determine which renderers the given node should be in
 * - Remove from any renderers the node shouldn't be in
 * - Add to desired renderers, if not already present
 * - Invalidate, for any renderers it was already present in
 */
void MapRenderer::updateAndInvalidateNode(mdl::Node& node)
{
  const auto desiredRenderers = determineDesiredRenderers(node);
  int currentRenderers = 0;

  if (auto it = m_trackedNodes.find(&node); it != m_trackedNodes.end())
  {
    currentRenderers = it->second;
  }

  auto updateForRenderer = [&](const Renderer r, ObjectRenderer* o) {
    const auto isRDesired = (desiredRenderers & int(r)) != 0;
    const auto isRCurrent = (currentRenderers & int(r)) != 0;

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

  updateForRenderer(Renderer::Default, m_defaultRenderer.get());
  updateForRenderer(Renderer::Selection, m_selectionRenderer.get());
  updateForRenderer(Renderer::Locked, m_lockedRenderer.get());

  // Update the metadata to reflect the changes that we made above
  m_trackedNodes[&node] = desiredRenderers;

  m_entityDecalRenderer->updateNode(node);
}

void MapRenderer::updateAndInvalidateNodeRecursive(mdl::Node& node)
{
  node.accept(kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
      updateAndInvalidateNode(*groupNode);
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode* entityNode) {
      updateAndInvalidateNode(*entityNode);
      entityNode->visitChildren(thisLambda);
    },
    [&](mdl::BrushNode* brushNode) { updateAndInvalidateNode(*brushNode); },
    [&](mdl::PatchNode* patchNode) { updateAndInvalidateNode(*patchNode); }));

  // Due to the definition of `selected()` above, we also need to update the parent.
  // (not recursively, though, so this has little performance impact.)
  // This handles clicking on a brush in a brush entity -> the entity label needs to
  // render as selected.
  if (node.parent())
  {
    updateAndInvalidateNode(*node.parent());
  }
}

void MapRenderer::removeNode(mdl::Node& node)
{
  if (auto it = m_trackedNodes.find(&node); it != m_trackedNodes.end())
  {
    const auto renderers = it->second;

    if (renderers & int(Renderer::Default))
    {
      m_defaultRenderer->removeNode(node);
    }
    if (renderers & int(Renderer::Selection))
    {
      m_selectionRenderer->removeNode(node);
    }
    if (renderers & int(Renderer::Locked))
    {
      m_lockedRenderer->removeNode(node);
    }

    m_trackedNodes.erase(it);

    // At this point, none of the default/selection/locked renderers,
    // or their underlying node-type specific renderers, have a reference
    // to `node` anymore, and they won't render it.

    m_entityDecalRenderer->removeNode(node);
  }
}

void MapRenderer::removeNodeRecursive(mdl::Node& node)
{
  node.accept(kdl::overload(
    [](auto&& thisLambda, mdl::WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, mdl::LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode* groupNode) {
      removeNode(*groupNode);
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode* entityNode) {
      removeNode(*entityNode);
      entityNode->visitChildren(thisLambda);
    },
    [&](mdl::BrushNode* brushNode) { removeNode(*brushNode); },
    [&](mdl::PatchNode* patchNode) { removeNode(*patchNode); }));
}

/**
 * Calls updateAndInvalidateNode() on every node in the world.
 */
void MapRenderer::updateAllNodes()
{
  updateAndInvalidateNodeRecursive(m_map.worldNode());
}

/**
 * Marks the nodes that are already tracked in the given renderers as invalid, i.e.
 * needing to be re-rendered.
 */
void MapRenderer::invalidateRenderers(const Renderer renderers)
{
  if (int(renderers) & int(Renderer::Default))
  {
    m_defaultRenderer->invalidate();
  }
  if (int(renderers) & int(Renderer::Selection))
  {
    m_selectionRenderer->invalidate();
  }
  if (int(renderers) & int(Renderer::Locked))
  {
    m_lockedRenderer->invalidate();
  }
}

void MapRenderer::invalidateEntityDecalRenderer()
{
  m_entityDecalRenderer->invalidate();
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
  m_notifierConnection +=
    m_map.nodesWereAddedNotifier.connect(this, &MapRenderer::nodesWereAdded);
  m_notifierConnection +=
    m_map.nodesWereRemovedNotifier.connect(this, &MapRenderer::nodesWereRemoved);
  m_notifierConnection +=
    m_map.nodesDidChangeNotifier.connect(this, &MapRenderer::nodesDidChange);
  m_notifierConnection += m_map.nodeVisibilityDidChangeNotifier.connect(
    this, &MapRenderer::nodeVisibilityDidChange);
  m_notifierConnection +=
    m_map.nodeLockingDidChangeNotifier.connect(this, &MapRenderer::nodeLockingDidChange);
  m_notifierConnection +=
    m_map.groupWasOpenedNotifier.connect(this, &MapRenderer::groupWasOpened);
  m_notifierConnection +=
    m_map.groupWasClosedNotifier.connect(this, &MapRenderer::groupWasClosed);
  m_notifierConnection +=
    m_map.selectionDidChangeNotifier.connect(this, &MapRenderer::selectionDidChange);
  m_notifierConnection += m_map.materialCollectionsWillChangeNotifier.connect(
    this, &MapRenderer::materialCollectionsWillChange);
  m_notifierConnection += m_map.entityDefinitionsDidChangeNotifier.connect(
    this, &MapRenderer::entityDefinitionsDidChange);
  m_notifierConnection +=
    m_map.modsDidChangeNotifier.connect(this, &MapRenderer::modsDidChange);
  m_notifierConnection += m_map.editorContextDidChangeNotifier.connect(
    this, &MapRenderer::editorContextDidChange);

  m_notifierConnection += m_map.resourceManager().resourcesWereProcessedNotifier.connect(
    this, &MapRenderer::resourcesWereProcessed);

  auto& prefs = PreferenceManager::instance();
  m_notifierConnection +=
    prefs.preferenceDidChangeNotifier.connect(this, &MapRenderer::preferenceDidChange);
}

void MapRenderer::nodesWereAdded(const std::vector<mdl::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    // The nodes passed in don't include recursive children, so we need to visit them
    // ourselves.
    updateAndInvalidateNodeRecursive(*node);
  }
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesWereRemoved(const std::vector<mdl::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    // The nodes passed in don't include recursive children, so we need to visit them
    // ourselves. Otherwise deleting a group doesn't delete the brushes within.
    removeNodeRecursive(*node);
  }
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodesDidChange(const std::vector<mdl::Node*>& nodes)
{
  for (auto* node : mdl::collectNodesAndAncestors(nodes))
  {
    // We update the ancestors along with the nodes, i.e. the world node. So, don't update
    // recursively here as it would cause the entire map to be invalidated on every
    // change.
    updateAndInvalidateNode(*node);
  }
  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::nodeVisibilityDidChange(const std::vector<mdl::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    updateAndInvalidateNodeRecursive(*node);
  }
  invalidateEntityLinkRenderer();
}

void MapRenderer::nodeLockingDidChange(const std::vector<mdl::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    updateAndInvalidateNodeRecursive(*node);
  }
  invalidateEntityLinkRenderer();
}

void MapRenderer::groupWasOpened()
{
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::groupWasClosed()
{
  invalidateGroupLinkRenderer();
  invalidateEntityLinkRenderer();
}

void MapRenderer::selectionDidChange(const mdl::SelectionChange& selectionChange)
{
  for (const auto& face : selectionChange.deselectedBrushFaces)
  {
    updateAndInvalidateNode(*face.node());
  }
  for (const auto& face : selectionChange.selectedBrushFaces)
  {
    updateAndInvalidateNode(*face.node());
  }
  // These need to be recursive otherwise selecting a Group doesn't render the contents
  // selected
  for (auto* node : selectionChange.deselectedNodes)
  {
    updateAndInvalidateNodeRecursive(*node);
  }
  for (auto* node : selectionChange.selectedNodes)
  {
    updateAndInvalidateNodeRecursive(*node);
  }

  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::resourcesWereProcessed(const std::vector<gl::ResourceId>& resourceIds)
{
  const auto& materialManager = m_map.materialManager();
  const auto materials = materialManager.findMaterialsByTextureResourceId(resourceIds);

  m_defaultRenderer->invalidateMaterials(materials);
  m_selectionRenderer->invalidateMaterials(materials);
  m_lockedRenderer->invalidateMaterials(materials);

  const auto& entityModelManager = m_map.entityModelManager();
  const auto entityModels =
    entityModelManager.findEntityModelsByTextureResourceId(resourceIds);
  m_defaultRenderer->invalidateEntityModels(entityModels);
  m_selectionRenderer->invalidateEntityModels(entityModels);
  m_lockedRenderer->invalidateEntityModels(entityModels);
}

void MapRenderer::materialCollectionsWillChange()
{
  invalidateRenderers(Renderer::All);
}

void MapRenderer::entityDefinitionsDidChange()
{
  reloadEntityModels();
  invalidateRenderers(Renderer::All);
  invalidateEntityLinkRenderer();
}

void MapRenderer::modsDidChange()
{
  reloadEntityModels();
  invalidateRenderers(Renderer::All);
  invalidateEntityLinkRenderer();
}

void MapRenderer::editorContextDidChange()
{
  invalidateRenderers(Renderer::All);
  invalidateEntityLinkRenderer();
  invalidateGroupLinkRenderer();
}

void MapRenderer::preferenceDidChange(const std::filesystem::path& path)
{
  setupRenderers();

  if (path == m_map.gameInfo().gamePathPreference.path)
  {
    reloadEntityModels();
    invalidateRenderers(Renderer::All);
    invalidateEntityLinkRenderer();
    invalidateGroupLinkRenderer();
  }

  if (kdl::path_has_prefix(path, "Map view"))
  {
    invalidateRenderers(Renderer::All);
    invalidateEntityLinkRenderer();
    invalidateGroupLinkRenderer();
  }
}

} // namespace tb::render
