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

#include "EntityLinkRenderer.h"

#include "Macros.h"
#include "Model/AttributableNode.h"
#include "Model/CollectMatchingNodesVisitor.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        EntityLinkRenderer::EntityLinkRenderer(View::MapDocumentWPtr document) :
        m_document(document),
        m_defaultColor(0.5f, 1.0f, 0.5f, 1.0f),
        m_selectedColor(1.0f, 0.0f, 0.0f, 1.0f),
        m_valid(false) {}
        
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
        
        void EntityLinkRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            renderBatch.add(this);
        }

        void EntityLinkRenderer::invalidate() {
            m_valid = false;
        }

        void EntityLinkRenderer::doPrepare(Vbo& vbo) {
            if (!m_valid) {
                validate();
                m_entityLinks.prepare(vbo);
            }
        }

        void EntityLinkRenderer::doRender(RenderContext& renderContext) {
            assert(m_valid);
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::EntityLinkShader);
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("MaxDistance", 6000.0f);

            glDisable(GL_DEPTH_TEST);
            shader.set("Alpha", 0.4f);
            m_entityLinks.render();
            
            glEnable(GL_DEPTH_TEST);
            shader.set("Alpha", 1.0f);
            m_entityLinks.render();
        }

        void EntityLinkRenderer::validate() {
            m_entityLinks = VertexArray::copy(GL_LINES, links());
            m_valid = true;
        }
        
        class EntityLinkRenderer::MatchEntities {
        public:
            bool operator()(const Model::Entity* entity) { return true; }
            bool operator()(const Model::Node* node) { return false; }
        };
        
        class EntityLinkRenderer::CollectEntitiesVisitor : public Model::CollectMatchingNodesVisitor<MatchEntities, Model::UniqueNodeCollectionStrategy> {};

        class EntityLinkRenderer::CollectLinksVisitor : public Model::NodeVisitor {
        protected:
            const Model::EditorContext& m_editorContext;
        private:
            const Color& m_defaultColor;
            const Color& m_selectedColor;
            Vertex::List m_vertices;
        protected:
            CollectLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor) :
            m_editorContext(editorContext),
            m_defaultColor(defaultColor),
            m_selectedColor(selectedColor) {}
        public:
            const Vertex::List& vertices() const {
                return m_vertices;
            }
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Brush* brush)   {}
            void doVisit(Model::Entity* entity) {
                if (m_editorContext.visible(entity))
                    visitEntity(entity);
                stopRecursion();
            }
            
            virtual void visitEntity(Model::Entity* entity) = 0;
        protected:
            void addLink(const Model::AttributableNode* source, const Model::AttributableNode* target) {
                const bool anySelected = source->selected() || source->descendantSelected() || target->selected() || target->descendantSelected();
                const Color& sourceColor = anySelected ? m_selectedColor : m_defaultColor;
                Color targetColor = anySelected ? m_selectedColor : m_defaultColor;
                
                m_vertices.push_back(Vertex(source->linkSourceAnchor(), sourceColor));
                m_vertices.push_back(Vertex(target->linkTargetAnchor(), targetColor));
            }
        };
        
        class EntityLinkRenderer::CollectAllLinksVisitor : public CollectLinksVisitor {
        public:
            CollectAllLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor) :
            CollectLinksVisitor(editorContext, defaultColor, selectedColor) {}
        private:
            void visitEntity(Model::Entity* entity) {
                if (m_editorContext.visible(entity)) {
                    addTargets(entity, entity->linkTargets());
                    addTargets(entity, entity->killTargets());
                }
            }
            
            void addTargets(Model::Entity* source, const Model::AttributableNodeList& targets) {
                Model::AttributableNodeList::const_iterator it, end;
                for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                    const Model::AttributableNode* target = *it;
                    if (m_editorContext.visible(target))
                        addLink(source, target);
                }
            }
        };
        
        class EntityLinkRenderer::CollectTransitiveSelectedLinksVisitor : public CollectLinksVisitor {
        private:
            Model::NodeSet m_visited;
        public:
            CollectTransitiveSelectedLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor) :
            CollectLinksVisitor(editorContext, defaultColor, selectedColor) {}
        private:
            void visitEntity(Model::Entity* entity) {
                if (m_editorContext.visible(entity)) {
                    const bool visited = !m_visited.insert(entity).second;
                    if (!visited) {
                        addSources(entity->linkSources(), entity);
                        addSources(entity->killSources(), entity);
                        addTargets(entity, entity->linkTargets());
                        addTargets(entity, entity->killTargets());
                    }
                }
            }

            void addSources(const Model::AttributableNodeList& sources, Model::Entity* target) {
                Model::AttributableNodeList::const_iterator it, end;
                for (it = sources.begin(), end = sources.end(); it != end; ++it) {
                    Model::AttributableNode* source = *it;
                    if (m_editorContext.visible(source)) {
                        addLink(source, target);
                        source->accept(*this);
                    }
                }
            }
            
            void addTargets(Model::Entity* source, const Model::AttributableNodeList& targets) {
                Model::AttributableNodeList::const_iterator it, end;
                for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                    Model::AttributableNode* target = *it;
                    if (m_editorContext.visible(target)) {
                        addLink(source, target);
                        target->accept(*this);
                    }
                }
            }
        };
        
        class EntityLinkRenderer::CollectDirectSelectedLinksVisitor : public CollectLinksVisitor {
        public:
            CollectDirectSelectedLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor) :
            CollectLinksVisitor(editorContext, defaultColor, selectedColor) {}
        private:
            void visitEntity(Model::Entity* entity) {
                if ((entity->selected() || entity->descendantSelected())) {
                    addSources(entity->linkSources(), entity);
                    addSources(entity->killSources(), entity);
                    addTargets(entity, entity->linkTargets());
                    addTargets(entity, entity->killTargets());
                }
            }
            
            void addSources(const Model::AttributableNodeList& sources, Model::Entity* target) {
                Model::AttributableNodeList::const_iterator it, end;
                for (it = sources.begin(), end = sources.end(); it != end; ++it) {
                    const Model::AttributableNode* source = *it;
                    if (!source->selected() && !source->descendantSelected() && m_editorContext.visible(source))
                        addLink(source, target);
                }
            }
            
            void addTargets(Model::Entity* source, const Model::AttributableNodeList& targets) {
                Model::AttributableNodeList::const_iterator it, end;
                for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                    const Model::AttributableNode* target = *it;
                    if (m_editorContext.visible(target))
                        addLink(source, target);
                }
            }
        };
        
        EntityLinkRenderer::Vertex::List EntityLinkRenderer::links() const {
            View::MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            switch (editorContext.entityLinkMode()) {
                case Model::EditorContext::EntityLinkMode_All:
                    return allLinks();
                case Model::EditorContext::EntityLinkMode_Transitive:
                    return transitiveSelectedLinks();
                case Model::EditorContext::EntityLinkMode_Direct:
                    return directSelectedLinks();
                case Model::EditorContext::EntityLinkMode_None:
                    return Vertex::List(0);
                DEFAULT_SWITCH()
            }
        }
        
        EntityLinkRenderer::Vertex::List EntityLinkRenderer::allLinks() const {
            View::MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            
            CollectAllLinksVisitor collectLinks(editorContext, m_defaultColor, m_selectedColor);
            
            Model::World* world = document->world();
            if (world != NULL)
                world->acceptAndRecurse(collectLinks);
            return collectLinks.vertices();
        }
        
        EntityLinkRenderer::Vertex::List EntityLinkRenderer::transitiveSelectedLinks() const {
            View::MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            
            CollectTransitiveSelectedLinksVisitor visitor(editorContext, m_defaultColor, m_selectedColor);
            return collectSelectedLinks(visitor);
        }
        
        EntityLinkRenderer::Vertex::List EntityLinkRenderer::directSelectedLinks() const {
            View::MapDocumentSPtr document = lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();
            
            CollectDirectSelectedLinksVisitor visitor(editorContext, m_defaultColor, m_selectedColor);
            return collectSelectedLinks(visitor);
        }

        EntityLinkRenderer::Vertex::List EntityLinkRenderer::collectSelectedLinks(CollectLinksVisitor& collectLinks) const {
            View::MapDocumentSPtr document = lock(m_document);
            
            const Model::NodeList& selectedNodes = document->selectedNodes().nodes();
            CollectEntitiesVisitor collectEntities;
            Model::Node::acceptAndEscalate(selectedNodes.begin(), selectedNodes.end(), collectEntities);
            
            const Model::NodeList& selectedEntities = collectEntities.nodes();
            Model::Node::accept(selectedEntities.begin(), selectedEntities.end(), collectLinks);
            
            return collectLinks.vertices();
        }
    }
}
