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

#include "ObjectRenderer.h"

#include "mdl/GroupNode.h"

#include "kdl/overload.h"

namespace tb::render
{

void ObjectRenderer::addNode(mdl::Node* node)
{
  node->accept(kdl::overload(
    [](mdl::WorldNode*) {},
    [](mdl::LayerNode*) {},
    [&](mdl::GroupNode* group) { m_groupRenderer.addGroup(group); },
    [&](mdl::EntityNode* entity) { m_entityRenderer.addEntity(entity); },
    [&](mdl::BrushNode* brush) { m_brushRenderer.addBrush(brush); },
    [&](mdl::PatchNode* patch) { m_patchRenderer.addPatch(patch); }));
}

void ObjectRenderer::removeNode(mdl::Node* node)
{
  node->accept(kdl::overload(
    [](mdl::WorldNode*) {},
    [](mdl::LayerNode*) {},
    [&](mdl::GroupNode* group) { m_groupRenderer.removeGroup(group); },
    [&](mdl::EntityNode* entity) { m_entityRenderer.removeEntity(entity); },
    [&](mdl::BrushNode* brush) { m_brushRenderer.removeBrush(brush); },
    [&](mdl::PatchNode* patch) { m_patchRenderer.removePatch(patch); }));
}

void ObjectRenderer::invalidateMaterials(
  const std::vector<const mdl::Material*>& materials)
{
  m_brushRenderer.invalidateMaterials(materials);
  m_patchRenderer.invalidate();
}

void ObjectRenderer::invalidateEntityModels(
  const std::vector<const mdl::EntityModel*>& entityModels)
{
  m_entityRenderer.invalidateEntityModels(entityModels);
}

void ObjectRenderer::invalidateNode(mdl::Node* node)
{
  node->accept(kdl::overload(
    [](mdl::WorldNode*) {},
    [](mdl::LayerNode*) {},
    [&](mdl::GroupNode* group) { m_groupRenderer.invalidateGroup(group); },
    [&](mdl::EntityNode* entity) { m_entityRenderer.invalidateEntity(entity); },
    [&](mdl::BrushNode* brush) { m_brushRenderer.invalidateBrush(brush); },
    [&](mdl::PatchNode* patch) { m_patchRenderer.invalidatePatch(patch); }));
}

void ObjectRenderer::invalidate()
{
  m_groupRenderer.invalidate();
  m_entityRenderer.invalidate();
  m_brushRenderer.invalidate();
  m_patchRenderer.invalidate();
}

void ObjectRenderer::clear()
{
  m_groupRenderer.clear();
  m_entityRenderer.clear();
  m_brushRenderer.clear();
  m_patchRenderer.clear();
}

void ObjectRenderer::reloadModels()
{
  m_entityRenderer.reloadModels();
}

void ObjectRenderer::setShowOverlays(const bool showOverlays)
{
  m_groupRenderer.setShowOverlays(showOverlays);
  m_entityRenderer.setShowOverlays(showOverlays);
}

void ObjectRenderer::setEntityOverlayTextColor(const Color& overlayTextColor)
{
  m_entityRenderer.setOverlayTextColor(overlayTextColor);
}

void ObjectRenderer::setGroupOverlayTextColor(const Color& overlayTextColor)
{
  m_groupRenderer.setOverlayTextColor(overlayTextColor);
}

void ObjectRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor)
{
  m_groupRenderer.setOverlayBackgroundColor(overlayBackgroundColor);
  m_entityRenderer.setOverlayBackgroundColor(overlayBackgroundColor);
}

void ObjectRenderer::setTint(const bool tint)
{
  m_entityRenderer.setTint(tint);
  m_brushRenderer.setTint(tint);
  m_patchRenderer.setTint(tint);
}

void ObjectRenderer::setTintColor(const Color& tintColor)
{
  m_entityRenderer.setTintColor(tintColor);
  m_brushRenderer.setTintColor(tintColor);
  m_patchRenderer.setTintColor(tintColor);
}

void ObjectRenderer::setShowOccludedObjects(const bool showOccludedObjects)
{
  m_groupRenderer.setShowOccludedBounds(showOccludedObjects);
  m_groupRenderer.setShowOccludedOverlays(showOccludedObjects);
  m_entityRenderer.setShowOccludedBounds(showOccludedObjects);
  m_entityRenderer.setShowOccludedOverlays(showOccludedObjects);
  m_brushRenderer.setShowOccludedEdges(showOccludedObjects);
  m_patchRenderer.setShowOccludedEdges(showOccludedObjects);
}

void ObjectRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor)
{
  m_groupRenderer.setOccludedBoundsColor(occludedEdgeColor);
  m_entityRenderer.setOccludedBoundsColor(occludedEdgeColor);
  m_brushRenderer.setOccludedEdgeColor(occludedEdgeColor);
  m_patchRenderer.setOccludedEdgeColor(occludedEdgeColor);
}

void ObjectRenderer::setTransparencyAlpha(const float transparencyAlpha)
{
  m_brushRenderer.setTransparencyAlpha(transparencyAlpha);
  m_patchRenderer.setTransparencyAlpha(transparencyAlpha);
}

void ObjectRenderer::setShowEntityAngles(const bool showAngles)
{
  m_entityRenderer.setShowAngles(showAngles);
}

void ObjectRenderer::setEntityAngleColor(const Color& color)
{
  m_entityRenderer.setAngleColor(color);
}

void ObjectRenderer::setOverrideGroupColors(const bool overrideGroupColors)
{
  m_groupRenderer.setOverrideColors(overrideGroupColors);
}

void ObjectRenderer::setGroupBoundsColor(const Color& color)
{
  m_groupRenderer.setBoundsColor(color);
}

void ObjectRenderer::setOverrideEntityBoundsColor(const bool overrideEntityBoundsColor)
{
  m_entityRenderer.setOverrideBoundsColor(overrideEntityBoundsColor);
}

void ObjectRenderer::setEntityBoundsColor(const Color& color)
{
  m_entityRenderer.setBoundsColor(color);
}

void ObjectRenderer::setShowBrushEdges(const bool showBrushEdges)
{
  m_brushRenderer.setShowEdges(showBrushEdges);
  m_patchRenderer.setShowEdges(showBrushEdges);
}

void ObjectRenderer::setBrushFaceColor(const Color& brushFaceColor)
{
  m_brushRenderer.setFaceColor(brushFaceColor);
  m_patchRenderer.setDefaultColor(brushFaceColor);
}

void ObjectRenderer::setBrushEdgeColor(const Color& brushEdgeColor)
{
  m_brushRenderer.setEdgeColor(brushEdgeColor);
  m_patchRenderer.setEdgeColor(brushEdgeColor);
}

void ObjectRenderer::setShowHiddenObjects(const bool showHiddenObjects)
{
  m_entityRenderer.setShowHiddenEntities(showHiddenObjects);
  m_brushRenderer.setShowHiddenBrushes(showHiddenObjects);
}

void ObjectRenderer::renderOpaque(RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_brushRenderer.renderOpaque(renderContext, renderBatch);
  m_patchRenderer.render(renderContext, renderBatch);
  m_entityRenderer.render(renderContext, renderBatch);
  m_groupRenderer.render(renderContext, renderBatch);
}

void ObjectRenderer::renderTransparent(
  RenderContext& renderContext, RenderBatch& renderBatch)
{
  m_brushRenderer.renderTransparent(renderContext, renderBatch);
}

} // namespace tb::render
