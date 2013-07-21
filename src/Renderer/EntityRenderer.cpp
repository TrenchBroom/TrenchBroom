/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityRenderer.h"

#include "CollectionUtils.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/SingleEntityRenderer.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        EntityRenderer::EntityRenderer() :
        m_boundsValid(false) {}
        
        EntityRenderer::~EntityRenderer() {
            clear();
        }
        
        void EntityRenderer::addEntity(const Model::Entity* entity) {
            assert(entity != NULL);
            assert(m_renderers.count(entity) == 0);
            SingleEntityRenderer* renderer = createRenderer(entity);
            m_renderers[entity] = renderer;
            invalidateBounds();
        }
        
        void EntityRenderer::addEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                addEntity(*it);
            invalidateBounds();
        }
        
        void EntityRenderer::updateEntity(const Model::Entity* entity) {
            assert(entity != NULL);
            
            Cache::iterator it = m_renderers.find(entity);
            assert(it != m_renderers.end());
            
            delete it->second;
            it->second = createRenderer(entity);
            invalidateBounds();
        }
        
        void EntityRenderer::updateEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                updateEntity(*it);
        }
        
        void EntityRenderer::removeEntity(const Model::Entity* entity) {
            assert(entity != NULL);
            
            Cache::iterator it = m_renderers.find(entity);
            assert(it != m_renderers.end());
            
            delete it->second;
            m_renderers.erase(it);
            invalidateBounds();
        }
        
        void EntityRenderer::removeEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                removeEntity(*it);
            invalidateBounds();
        }

        void EntityRenderer::clear() {
            MapUtils::clearAndDelete(m_renderers);
        }

        bool EntityRenderer::boundsValid() const {
            return m_boundsValid;
        }

        void EntityRenderer::validateBounds(Vbo& boundsVbo) {
            VertexSpecs::P3C4::Vertex::List vertices;
            vertices.reserve(24 * m_renderers.size());
            
            Cache::const_iterator it, end;
            for (it = m_renderers.begin(), end = m_renderers.end(); it != end; ++it) {
                const SingleEntityRenderer* renderer = it->second;
                renderer->getBoundsVertices(vertices);
            }
            
            m_boundsRenderer = EdgeRenderer(boundsVbo, vertices);
            m_boundsValid = true;
        }

        void EntityRenderer::render(RenderContext& context) {
            assert(m_boundsValid);
            glSetEdgeOffset(0.025f);
            m_boundsRenderer.render(context);
            glResetEdgeOffset();
        }

        SingleEntityRenderer* EntityRenderer::createRenderer(const Model::Entity* entity) const {
            return new SingleEntityRenderer(entity);
        }

        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }
    }
}
