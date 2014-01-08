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

#include "Model/Entity.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/ShaderProgram.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        bool EntityLinkRenderer::Filter::showLink(const Model::Entity* source, const Model::Entity* target, const bool isConnectedToSelected) const {
            return doGetShowLink(source, target, isConnectedToSelected);
        }
        
        const Color& EntityLinkRenderer::Filter::linkColor(const Model::Entity* source, const Model::Entity* target, const bool isConnectedToSelected) const {
            return doGetLinkColor(source, target, isConnectedToSelected);
        }
        
        const Color& EntityLinkRenderer::Filter::killColor(const Model::Entity* source, const Model::Entity* target, const bool isConnectedToSelected) const {
            return doGetKillColor(source, target, isConnectedToSelected);
        }

        EntityLinkRenderer::EntityLinkRenderer() :
        m_vbo(0xFFFF),
        m_valid(false) {}
        
        bool EntityLinkRenderer::valid() const {
            return m_valid;
        }

        void EntityLinkRenderer::invalidate() {
            m_valid = false;
        }
        
        void EntityLinkRenderer::validate(const Filter& filter, const Model::EntityList& entities) {
            assert(!m_valid);
            
            if (entities.empty()) {
                m_entityLinks = VertexArray();
                m_valid = true;
                return;
            }
            
            Model::EntitySet visitedEntities;
            Vertex::List vertices;
            Model::EntityList::const_iterator it, end;
            
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                if (entity->selected() || entity->partiallySelected())
                    buildLinks(filter, visitedEntities, entity, true, vertices);
            }
            
            for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                Model::Entity* entity = *it;
                if (!entity->selected() && !entity->partiallySelected())
                    buildLinks(filter, visitedEntities, entity, false, vertices);
            }
            
            SetVboState mapVbo(m_vbo);
            mapVbo.mapped();
            
            m_entityLinks = VertexArray::swap(GL_LINES, vertices);
            m_valid = true;
        }
        
        void EntityLinkRenderer::render(RenderContext& renderContext) {
            assert(m_valid);
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::EntityLinkShader);
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("MaxDistance", 1000.0f);

            SetVboState activateVbo(m_vbo);
            activateVbo.active();
            if (!m_entityLinks.prepared()) {
                SetVboState mapVbo(m_vbo);
                mapVbo.mapped();
                m_entityLinks.prepare(m_vbo);
            }

            glDisable(GL_DEPTH_TEST);
            shader.set("Alpha", 0.4f);
            m_entityLinks.render();
            
            glEnable(GL_DEPTH_TEST);
            shader.set("Alpha", 1.0f);
            m_entityLinks.render();
        }

        void EntityLinkRenderer::buildLinks(const Filter& filter, Model::EntitySet& visitedEntities, Model::Entity* entity, const bool isConnectedToSelected, Vertex::List& vertices) const {
            const bool visited = visitedEntities.insert(entity).second;
            if (!visited)
                return;
            
            Model::EntityList::const_iterator it, end;

            const Model::EntityList& linkTargets = entity->linkTargets();
            for (it = linkTargets.begin(), end = linkTargets.end(); it != end; ++it) {
                Model::Entity* target = *it;
                if (filter.showLink(entity, target, isConnectedToSelected)) {
                    const Color& color = filter.linkColor(entity, target, isConnectedToSelected);
                    addLink(entity, target, color, vertices);
                    buildLinks(filter, visitedEntities, target, isConnectedToSelected, vertices);
                }
            }
            
            const Model::EntityList& linkSources = entity->linkSources();
            for (it = linkSources.begin(), end = linkSources.end(); it != end; ++it) {
                Model::Entity* source = *it;
                if (filter.showLink(source, entity, isConnectedToSelected))
                    buildLinks(filter, visitedEntities, source, isConnectedToSelected, vertices);
            }
            
            const Model::EntityList& killTargets = entity->killTargets();
            for (it = killTargets.begin(), end = killTargets.end(); it != end; ++it) {
                Model::Entity* target = *it;
                if (filter.showLink(entity, target, isConnectedToSelected)) {
                    const Color& color = filter.killColor(entity, target, isConnectedToSelected);
                    addLink(entity, target, color, vertices);
                    buildLinks(filter, visitedEntities, target, isConnectedToSelected, vertices);
                }
            }

            const Model::EntityList& killSources = entity->killSources();
            for (it = killSources.begin(), end = killSources.end(); it != end; ++it) {
                Model::Entity* source = *it;
                if (filter.showLink(source, entity, isConnectedToSelected))
                    buildLinks(filter, visitedEntities, source, isConnectedToSelected, vertices);
            }
        }

        void EntityLinkRenderer::addLink(const Model::Entity* source, const Model::Entity* target, const Color& color, Vertex::List& vertices) const {
            vertices.push_back(Vertex(source->bounds().center(), color));
            vertices.push_back(Vertex(target->bounds().center(), color));
        }
    }
}
