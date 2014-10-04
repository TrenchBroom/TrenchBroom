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

#include "Model/Node.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Renderer {
        class AddNodeToObjectRenderer : public Model::NodeVisitor {
        private:
            EntityRenderer& m_entityRenderer;
            BrushRenderer& m_brushRenderer;
        public:
            AddNodeToObjectRenderer(EntityRenderer& entityRenderer, BrushRenderer& brushRenderer) :
            m_entityRenderer(entityRenderer),
            m_brushRenderer(brushRenderer) {}
        private:
            virtual void doVisit(Model::World* world)   {}
            virtual void doVisit(Model::Layer* layer)   {}
            virtual void doVisit(Model::Group* group)   {}
            virtual void doVisit(Model::Entity* entity) { m_entityRenderer.addEntity(entity); }
            virtual void doVisit(Model::Brush* brush)   { m_brushRenderer.addBrush(brush); }
        };
        
        void ObjectRenderer::addObjects(const Model::NodeList& nodes) {
            AddNodeToObjectRenderer visitor(m_entityRenderer, m_brushRenderer);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
        }

        void ObjectRenderer::addObject(Model::Node* object) {
            assert(object != NULL);
            AddNodeToObjectRenderer visitor(m_entityRenderer, m_brushRenderer);
            object->acceptAndRecurse(visitor);
        }
        
        class RemoveNodeFromObjectRenderer : public Model::NodeVisitor {
        private:
            EntityRenderer& m_entityRenderer;
            BrushRenderer& m_brushRenderer;
        public:
            RemoveNodeFromObjectRenderer(EntityRenderer& entityRenderer, BrushRenderer& brushRenderer) :
            m_entityRenderer(entityRenderer),
            m_brushRenderer(brushRenderer) {}
        private:
            virtual void doVisit(Model::World* world)   {}
            virtual void doVisit(Model::Layer* layer)   {}
            virtual void doVisit(Model::Group* group)   {}
            virtual void doVisit(Model::Entity* entity) { m_entityRenderer.removeEntity(entity); }
            virtual void doVisit(Model::Brush* brush)   { m_brushRenderer.removeBrush(brush); }
        };

        void ObjectRenderer::removeObjects(const Model::NodeList& nodes) {
            RemoveNodeFromObjectRenderer visitor(m_entityRenderer, m_brushRenderer);
            Model::Node::acceptAndRecurse(nodes.begin(), nodes.end(), visitor);
        }

        void ObjectRenderer::removeObject(Model::Node* object) {
            assert(object != NULL);
            RemoveNodeFromObjectRenderer visitor(m_entityRenderer, m_brushRenderer);
            object->acceptAndRecurse(visitor);
        }

        class UpdateNodesInObjectRenderer : public Model::NodeVisitor {
        private:
            EntityRenderer& m_entityRenderer;
            BrushRenderer& m_brushRenderer;
        public:
            UpdateNodesInObjectRenderer(EntityRenderer& entityRenderer, BrushRenderer& brushRenderer) :
            m_entityRenderer(entityRenderer),
            m_brushRenderer(brushRenderer) {}
        private:
            virtual void doVisit(Model::World* world)   {}
            virtual void doVisit(Model::Layer* layer)   {}
            virtual void doVisit(Model::Group* group)   {}
            virtual void doVisit(Model::Entity* entity) { m_entityRenderer.updateEntity(entity); }
            virtual void doVisit(Model::Brush* brush)   { m_brushRenderer.updateBrush(brush); }
        };
        
        void ObjectRenderer::updateObjects(const Model::NodeList& nodes) {
            UpdateNodesInObjectRenderer visitor(m_entityRenderer, m_brushRenderer);
            Model::Node::accept(nodes.begin(), nodes.end(), visitor);
        }
        
        void ObjectRenderer::updateObject(Model::Node* object) {
            assert(object != NULL);
            UpdateNodesInObjectRenderer visitor(m_entityRenderer, m_brushRenderer);
            object->accept(visitor);
        }

        void ObjectRenderer::clear() {
            m_brushRenderer.clear();
            m_entityRenderer.clear();
        }
        
        void ObjectRenderer::setOverlayTextColor(const Color& overlayTextColor) {
            m_entityRenderer.setOverlayTextColor(overlayTextColor);
        }
    
        void ObjectRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor) {
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
            m_entityRenderer.setShowOccludedBounds(showOccludedObjects);
            m_entityRenderer.setShowOccludedOverlays(true);
            m_brushRenderer.setShowOccludedEdges(true);
        }
        
        void ObjectRenderer::setOccludedEdgeColor(const Color& occludedEdgeColor) {
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
        }
    }
}
