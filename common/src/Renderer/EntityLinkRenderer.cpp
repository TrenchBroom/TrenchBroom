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
#include "Model/Entity.h"
#include "Model/Map.h"
#include "Model/ModelFilter.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"
#include "View/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        EntityLinkRenderer::EntityLinkRenderer(View::MapDocumentWPtr document) :
        m_document(document),
        m_defaultColor(0.5f, 1.0f, 0.5f, 1.0f),
        m_selectedColor(1.0f, 0.0f, 0.0f, 1.0f),
        m_vbo(0xFFFF),
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
        
        void EntityLinkRenderer::invalidate() {
            m_valid = false;
        }

        void EntityLinkRenderer::render(RenderContext& renderContext) {
            if (!m_valid)
                validate();
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::EntityLinkShader);
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("MaxDistance", 6000.0f);

            SetVboState activateVbo(m_vbo);
            activateVbo.active();
            if (!m_valid)
                validate();

            glDisable(GL_DEPTH_TEST);
            shader.set("Alpha", 0.4f);
            m_entityLinks.render();
            
            glEnable(GL_DEPTH_TEST);
            shader.set("Alpha", 1.0f);
            m_entityLinks.render();
        }

        void EntityLinkRenderer::validate() {
            Vertex::List vertices;

            View::MapDocumentSPtr document = lock(m_document);
            const Model::ModelFilter& filter = document->filter();
            switch (filter.entityLinkMode()) {
                case Model::ModelFilter::EntityLinkMode_All:
                    addAllLinks(document, vertices);
                    break;
                case Model::ModelFilter::EntityLinkMode_Transitive:
                    addTransitiveSelectedLinks(document, vertices);
                    break;
                case Model::ModelFilter::EntityLinkMode_Direct:
                    addDirectSelectedLinks(document, vertices);
                    break;
                case Model::ModelFilter::EntityLinkMode_None:
                    break;
                DEFAULT_SWITCH()
            }

            SetVboState mapVbo(m_vbo);
            mapVbo.mapped();
            m_entityLinks = VertexArray::swap(GL_LINES, vertices);
            m_entityLinks.prepare(m_vbo);
            m_valid = true;
        }
        
        void EntityLinkRenderer::addTransitiveSelectedLinks(View::MapDocumentSPtr document, Vertex::List& vertices) const {
            const Model::EntityList& entities = document->map()->entities();
            Model::EntitySet visitedEntities;
            Model::EntityList::const_iterator it, end;
            
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                if (entity->selected() || entity->partiallySelected())
                    buildLinks(document->filter(), visitedEntities, entity, true, vertices);
            }
            
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                if (!entity->selected() && !entity->partiallySelected())
                    buildLinks(document->filter(), visitedEntities, entity, false, vertices);
            }
        }
        
        void EntityLinkRenderer::buildLinks(const Model::ModelFilter& filter, Model::EntitySet& visitedEntities, Model::Entity* entity, const bool isConnectedToSelected, Vertex::List& vertices) const {
            if (!filter.visible(entity))
                return;
            
            const bool visited = visitedEntities.insert(entity).second;
            if (!visited)
                return;
            
            Model::EntityList::const_iterator it, end;
            
            const Model::EntityList& linkTargets = entity->linkTargets();
            for (it = linkTargets.begin(), end = linkTargets.end(); it != end; ++it) {
                Model::Entity* target = *it;
                if (filter.visible(target) &&
                    (entity->selected() || entity->partiallySelected() ||
                     target->selected() || target->partiallySelected() ||
                     isConnectedToSelected)) {
                    addLink(entity, target, vertices);
                    buildLinks(filter, visitedEntities, target, isConnectedToSelected, vertices);
                }
            }
            
            const Model::EntityList& linkSources = entity->linkSources();
            for (it = linkSources.begin(), end = linkSources.end(); it != end; ++it) {
                Model::Entity* source = *it;
                if (filter.visible(source) &&
                    (source->selected() || source->partiallySelected() ||
                     entity->selected() || entity->partiallySelected() ||
                     isConnectedToSelected))
                    buildLinks(filter, visitedEntities, source, isConnectedToSelected, vertices);
            }
            
            const Model::EntityList& killTargets = entity->killTargets();
            for (it = killTargets.begin(), end = killTargets.end(); it != end; ++it) {
                Model::Entity* target = *it;
                if (filter.visible(target) &&
                    (entity->selected() || entity->partiallySelected() ||
                     target->selected() || target->partiallySelected() ||
                     isConnectedToSelected)) {
                    addLink(entity, target, vertices);
                    buildLinks(filter, visitedEntities, target, isConnectedToSelected, vertices);
                }
            }
            
            const Model::EntityList& killSources = entity->killSources();
            for (it = killSources.begin(), end = killSources.end(); it != end; ++it) {
                Model::Entity* source = *it;
                if (filter.visible(source) &&
                    (source->selected() || source->partiallySelected() ||
                     entity->selected() || entity->partiallySelected() ||
                     isConnectedToSelected))
                    buildLinks(filter, visitedEntities, source, isConnectedToSelected, vertices);
            }
        }
        
        void EntityLinkRenderer::addAllLinks(View::MapDocumentSPtr document, Vertex::List& vertices) const {
            addSourceLinks(document->filter(), document->map()->entities(), vertices);
        }
        
        void EntityLinkRenderer::addDirectSelectedLinks(View::MapDocumentSPtr document, Vertex::List& vertices) const {
            addSourceLinks(document->filter(), document->allSelectedEntities(), vertices);
            addTargetLinks(document->filter(), document->allSelectedEntities(), vertices);
        }
        
        void EntityLinkRenderer::addSourceLinks(const Model::ModelFilter& filter, const Model::EntityList& entities, Vertex::List& vertices) const {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (filter.visible(entity)) {
                    addLinks(filter, entity, entity->linkTargets(), vertices);
                    addLinks(filter, entity, entity->killTargets(), vertices);
                }
            }
        }

        void EntityLinkRenderer::addTargetLinks(const Model::ModelFilter& filter, const Model::EntityList& entities, Vertex::List& vertices) const {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (filter.visible(entity)) {
                    addLinks(filter, entity, entity->linkSources(), vertices);
                    addLinks(filter, entity, entity->killSources(), vertices);
                }
            }
        }
        
        void EntityLinkRenderer::addLinks(const Model::ModelFilter& filter, const Model::Entity* source, const Model::EntityList& targets, Vertex::List& vertices) const {
            Model::EntityList::const_iterator it, end;
            for (it = targets.begin(), end = targets.end(); it != end; ++it) {
                const Model::Entity* target = *it;
                if (filter.visible(target))
                    addLink(source, target, vertices);
            }
        }

        void EntityLinkRenderer::addLink(const Model::Entity* source, const Model::Entity* target, Vertex::List& vertices) const {
            const bool anySelected = source->selected() || source->partiallySelected() || target->selected() || target->partiallySelected();
            const Color& sourceColor = anySelected ? m_selectedColor : m_defaultColor;
            Color targetColor = anySelected ? m_selectedColor : m_defaultColor;
            for (size_t i = 0; i < 3; ++i)
                targetColor[i] *= 0.5f;
            
            vertices.push_back(Vertex(source->bounds().center(), sourceColor));
            vertices.push_back(Vertex(target->bounds().center(), targetColor));
        }
    }
}
