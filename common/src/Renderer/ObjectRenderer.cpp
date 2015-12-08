/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Model/Group.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Renderer {
        void ObjectRenderer::setObjects(const Model::GroupList& groups, const Model::EntityList& entities, const Model::BrushList& brushes) {
            m_groupRenderer.setGroups(groups);
            m_entityRenderer.setEntities(entities);
            m_brushRenderer.setBrushes(brushes);
        }

        void ObjectRenderer::invalidate() {
            m_groupRenderer.invalidate();
            m_entityRenderer.invalidate();
            m_brushRenderer.invalidate();
        }

        void ObjectRenderer::clear() {
            m_groupRenderer.clear();
            m_entityRenderer.clear();
            m_brushRenderer.clear();
        }
        
        void ObjectRenderer::reloadModels() {
            m_entityRenderer.reloadModels();
        }

        void ObjectRenderer::setShowOverlays(const bool showOverlays) {
            m_groupRenderer.setShowOverlays(showOverlays);
            m_entityRenderer.setShowOverlays(showOverlays);
        }

        void ObjectRenderer::setOverlayTextColor(const Color& overlayTextColor) {
            m_groupRenderer.setOverlayTextColor(overlayTextColor);
            m_entityRenderer.setOverlayTextColor(overlayTextColor);
        }
    
        void ObjectRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor) {
            m_groupRenderer.setOverlayBackgroundColor(overlayBackgroundColor);
            m_entityRenderer.setOverlayBackgroundColor(overlayBackgroundColor);
        }
        
        void ObjectRenderer::setTint(const bool tint) {
            m_entityRenderer.setTint(tint);
            m_brushRenderer.setTint(tint);
        }
        
        void ObjectRenderer::setTintColor(const Color& tintColor) {
            m_entityRenderer.setTintColor(tintColor);
            m_brushRenderer.setTintColor(tintColor);
        }
        
        void ObjectRenderer::setShowOccludedObjects(const bool showOccludedObjects) {
            m_groupRenderer.setShowOccludedBounds(showOccludedObjects);
            m_groupRenderer.setShowOccludedOverlays(showOccludedObjects);
            m_entityRenderer.setShowOccludedBounds(showOccludedObjects);
            m_entityRenderer.setShowOccludedOverlays(showOccludedObjects);
            m_brushRenderer.setShowOccludedEdges(showOccludedObjects);
        }
        
        void ObjectRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
            m_groupRenderer.setOccludedBoundsColor(occludedEdgeColor);
            m_entityRenderer.setOccludedBoundsColor(occludedEdgeColor);
            m_brushRenderer.setOccludedEdgeColor(occludedEdgeColor);
        }
        
        void ObjectRenderer::setTransparencyAlpha(const float transparencyAlpha) {
            m_brushRenderer.setTransparencyAlpha(transparencyAlpha);
        }
        
        void ObjectRenderer::setShowEntityAngles(const bool showAngles) {
            m_entityRenderer.setShowAngles(showAngles);
        }
        
        void ObjectRenderer::setEntityAngleColor(const Color& color) {
            m_entityRenderer.setAngleColor(color);
        }
        
        void ObjectRenderer::setOverrideGroupBoundsColor(const bool overrideGroupBoundsColor) {
            m_groupRenderer.setOverrideBoundsColor(overrideGroupBoundsColor);
        }

        void ObjectRenderer::setGroupBoundsColor(const Color& color) {
            m_groupRenderer.setBoundsColor(color);
        }

        void ObjectRenderer::setOverrideEntityBoundsColor(const bool overrideEntityBoundsColor) {
            m_entityRenderer.setOverrideBoundsColor(overrideEntityBoundsColor);
        }
        
        void ObjectRenderer::setEntityBoundsColor(const Color& color) {
            m_entityRenderer.setBoundsColor(color);
        }
        
        void ObjectRenderer::setBrushFaceColor(const Color& brushFaceColor) {
            m_brushRenderer.setFaceColor(brushFaceColor);
        }
        
        void ObjectRenderer::setBrushEdgeColor(const Color& brushEdgeColor) {
            m_brushRenderer.setEdgeColor(brushEdgeColor);
        }
        
        void ObjectRenderer::setShowHiddenObjects(const bool showHiddenObjects) {
            m_entityRenderer.setShowHiddenEntities(showHiddenObjects);
            m_brushRenderer.setShowHiddenBrushes(showHiddenObjects);
        }

        void ObjectRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            m_brushRenderer.render(renderContext, renderBatch);
            m_entityRenderer.render(renderContext, renderBatch);
            m_groupRenderer.render(renderContext, renderBatch);
        }
    }
}
