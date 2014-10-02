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

        void ObjectRenderer::clear() {
            m_brushRenderer.clear();
            m_entityRenderer.clear();
        }
        
        void ObjectRenderer::setTint(const bool tint) {
            m_brushRenderer.setTintFaces(tint);
            m_entityRenderer.setApplyTinting(tint);
        }
        
        void ObjectRenderer::setTintColor(const Color& tintColor) {
            m_brushRenderer.setTintColor(tintColor);
            m_entityRenderer.setTintColor(tintColor);
        }
        
        void ObjectRenderer::setRenderOccludedEdges(const bool renderOccludedEdges) {
            m_brushRenderer.setRenderOccludedEdges(renderOccludedEdges);
            m_entityRenderer.setRenderOccludedBounds(renderOccludedEdges);
        }
        
        void ObjectRenderer::setTransparencyAlpha(const float transparencyAlpha) {
            m_brushRenderer.setTransparencyAlpha(transparencyAlpha);
        }
        
        void ObjectRenderer::setBrushFaceColor(const Color& brushFaceColor) {
            m_brushRenderer.setFaceColor(brushFaceColor);
        }
        
        void ObjectRenderer::setBrushEdgeColor(const Color& brushEdgeColor) {
            m_brushRenderer.setEdgeColor(brushEdgeColor);
        }
        
        void ObjectRenderer::setOccludedBrushEdgeColor(const Color& occludedEdgeColor) {
            m_brushRenderer.setOccludedEdgeColor(occludedEdgeColor);
        }
        
        void ObjectRenderer::setShowHiddenBrushes(const bool showHiddenBrushes) {
            m_brushRenderer.setShowHiddenBrushes(showHiddenBrushes);
        }

        void ObjectRenderer::render(RenderContext& renderContext) {
            m_brushRenderer.render(renderContext);
            m_entityRenderer.render(renderContext);
        }
    }
}
