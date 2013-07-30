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
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/ModelDefinition.h"
#include "Assets/EntityModel.h"
#include "Model/Entity.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/SingleEntityRenderer.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        EntityRenderer::EntityClassnameAnchor::EntityClassnameAnchor(const Model::Entity* entity) :
        m_entity(entity) {}
        
        const Vec3f EntityRenderer::EntityClassnameAnchor::basePosition() const {
            const Assets::EntityModel* model = m_entity->model();
            Vec3f position = m_entity->bounds().center();
            position[2] = m_entity->bounds().max.z();
            if (model != NULL) {
                const Assets::ModelSpecification spec = m_entity->modelSpecification();
                const BBox3f modelBounds = model->bounds(spec.skinIndex, spec.frameIndex);
                const Vec3f origin = m_entity->origin();
                position[2] = std::max(position[2], modelBounds.max.z() + origin.z());
            }
            position[2] += 2.0f;
            return position;
        }
        
        const Alignment::Type EntityRenderer::EntityClassnameAnchor::alignment() const {
            return Alignment::Bottom;
        }

        bool EntityRenderer::EntityClassnameFilter::stringVisible(RenderContext& context, const Key& entity) const {
            return true;
        }

        EntityRenderer::EntityRenderer(FontManager& fontManager) :
        m_classnameRenderer(ClassnameRenderer(font(fontManager))),
        m_boundsValid(false) {
            m_classnameRenderer.setFadeDistance(500.0f);
        }
        
        EntityRenderer::~EntityRenderer() {
            clear();
        }
        
        void EntityRenderer::addEntity(Model::Entity* entity) {
            assert(entity != NULL);

            assert(m_renderers.count(entity) == 0);
            SingleEntityRenderer* renderer = createRenderer(entity);
            m_renderers[entity] = renderer;
            m_classnameRenderer.addString(entity, entity->classname(), TextAnchor::Ptr(new EntityClassnameAnchor(entity)));
            m_modelRenderer.addEntity(entity);
            
            invalidateBounds();
        }
        
        void EntityRenderer::addEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                addEntity(*it);
            invalidateBounds();
        }
        
        void EntityRenderer::updateEntity(Model::Entity* entity) {
            assert(entity != NULL);
            
            Cache::iterator it = m_renderers.find(entity);
            assert(it != m_renderers.end());
            
            delete it->second;
            it->second = createRenderer(entity);
            
            m_classnameRenderer.updateString(entity, entity->classname());
            m_modelRenderer.updateEntity(entity);
            invalidateBounds();
        }
        
        void EntityRenderer::updateEntities(const Model::EntityList& entities) {
            Model::EntityList::const_iterator it, end;
            for (it = entities.begin(), end = entities.end(); it != end; ++it)
                updateEntity(*it);
        }
        
        void EntityRenderer::removeEntity(Model::Entity* entity) {
            assert(entity != NULL);
            
            Cache::iterator it = m_renderers.find(entity);
            assert(it != m_renderers.end());
            
            delete it->second;
            m_renderers.erase(it);
            
            m_classnameRenderer.removeString(entity);
            m_modelRenderer.removeEntity(entity);
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
            m_classnameRenderer.clear();
            m_modelRenderer.clear();
        }

        void EntityRenderer::render(RenderContext& context) {
            renderBounds(context);
            renderModels(context);
            renderClassnames(context);
        }

        void EntityRenderer::renderBounds(RenderContext& context) {
            if (!m_boundsValid)
                validateBounds();
            
            glSetEdgeOffset(0.025f);
            m_boundsRenderer.render(context);
            glResetEdgeOffset();
        }
        
        void EntityRenderer::renderClassnames(RenderContext& context) {
            EntityClassnameFilter textFilter;
            m_classnameRenderer.render(context, textFilter,
                                       Shaders::TextShader, classnameTextColor(),
                                       Shaders::TextBackgroundShader, classnameBackgroundColor());
        }
        
        void EntityRenderer::renderModels(RenderContext& context) {
            m_modelRenderer.render(context);
        }

        TextureFont& EntityRenderer::font(FontManager& fontManager) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            const size_t fontSize = static_cast<size_t>(prefs.getInt(Preferences::RendererFontSize));
            return fontManager.font(FontDescriptor(fontName, fontSize));
        }

        const Color& EntityRenderer::classnameTextColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::InfoOverlayTextColor);
        }
        
        const Color& EntityRenderer::classnameBackgroundColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::InfoOverlayBackgroundColor);
        }

        SingleEntityRenderer* EntityRenderer::createRenderer(const Model::Entity* entity) const {
            return new SingleEntityRenderer(entity);
        }

        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }

        void EntityRenderer::validateBounds() {
            VertexSpecs::P3C4::Vertex::List vertices;
            vertices.reserve(24 * m_renderers.size());
            
            Cache::const_iterator it, end;
            for (it = m_renderers.begin(), end = m_renderers.end(); it != end; ++it) {
                const SingleEntityRenderer* renderer = it->second;
                renderer->getBoundsVertices(vertices);
            }
            
            m_boundsRenderer = EdgeRenderer(vertices);
            m_boundsValid = true;
        }
    }
}
