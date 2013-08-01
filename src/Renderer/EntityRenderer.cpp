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

#include "TrenchBroom.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinition.h"
#include "Assets/ModelDefinition.h"
#include "Assets/EntityModel.h"
#include "Model/Entity.h"
#include "Model/Filter.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/FontManager.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
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

        EntityRenderer::EntityClassnameFilter::EntityClassnameFilter(const Model::Filter& filter) :
        m_filter(filter) {}

        bool EntityRenderer::EntityClassnameFilter::stringVisible(RenderContext& context, const Key& entity) const {
            return m_filter.visible(entity);
        }

        Color EntityRenderer::EntityClassnameColorProvider::textColor(RenderContext& context, const Key& entity) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            if (entity->selected())
                return prefs.getColor(Preferences::SelectedInfoOverlayTextColor);
            return prefs.getColor(Preferences::InfoOverlayTextColor);
        }
        
        Color EntityRenderer::EntityClassnameColorProvider::backgroundColor(RenderContext& context, const Key& entity) const {
            PreferenceManager& prefs = PreferenceManager::instance();
            if (entity->selected())
                return prefs.getColor(Preferences::SelectedInfoOverlayBackgroundColor);
            return prefs.getColor(Preferences::InfoOverlayBackgroundColor);
        }

        EntityRenderer::EntityRenderer(FontManager& fontManager, const Model::Filter& filter) :
        m_filter(filter),
        m_classnameRenderer(ClassnameRenderer(font(fontManager))),
        m_modelRenderer(m_filter),
        m_boundsValid(false) {
            m_classnameRenderer.setFadeDistance(500.0f);
        }
        
        EntityRenderer::~EntityRenderer() {
            clear();
        }
        
        void EntityRenderer::addEntity(Model::Entity* entity) {
            assert(entity != NULL);

            assert(m_entities.count(entity) == 0);
            m_entities.insert(entity);
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
            assert(m_entities.count(entity) == 1);
            
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
            
            Model::EntitySet::iterator it = m_entities.find(entity);
            assert(it != m_entities.end());
            m_entities.erase(it);
            
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
            m_entities.clear();
            m_classnameRenderer.clear();
            m_modelRenderer.clear();
        }

        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }
        
        void EntityRenderer::render(RenderContext& context) {
            renderBounds(context);
            renderModels(context);
            renderClassnames(context);
        }

        void EntityRenderer::renderBounds(RenderContext& context) {
            if (!m_boundsValid)
                validateBounds();
            
            PreferenceManager& prefs = PreferenceManager::instance();

            glSetEdgeOffset(0.025f);
            m_boundsRenderer.render(context);
            
            glSetEdgeOffset(0.03f);
            glDisable(GL_DEPTH_TEST);
            m_selectedBoundsRenderer.setColor(prefs.getColor(Preferences::OccludedSelectedEdgeColor));
            m_selectedBoundsRenderer.render(context);

            glEnable(GL_DEPTH_TEST);
            m_selectedBoundsRenderer.setColor(prefs.getColor(Preferences::SelectedEdgeColor));
            m_selectedBoundsRenderer.render(context);
            glResetEdgeOffset();
        }
        
        void EntityRenderer::renderClassnames(RenderContext& context) {
            EntityClassnameFilter textFilter(m_filter);
            EntityClassnameColorProvider colorProvider;
            m_classnameRenderer.render(context, textFilter, colorProvider,
                                       Shaders::TextShader, Shaders::TextBackgroundShader);
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

        struct BuildColoredBoundsVertices {
            VertexSpecs::P3C4::Vertex::List& vertices;
            Color color;
            
            BuildColoredBoundsVertices(VertexSpecs::P3C4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            inline void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3C4::Vertex(v1, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v2, color));
            }
        };
        
        struct BuildBoundsVertices {
            VertexSpecs::P3::Vertex::List& vertices;
            
            BuildBoundsVertices(VertexSpecs::P3::Vertex::List& i_vertices) :
            vertices(i_vertices) {}
            
            inline void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3::Vertex(v1));
                vertices.push_back(VertexSpecs::P3::Vertex(v2));
            }
        };
        
        void EntityRenderer::validateBounds() {
            VertexSpecs::P3C4::Vertex::List vertices;
            VertexSpecs::P3::Vertex::List selectedVertices;
            vertices.reserve(24 * m_entities.size());
            selectedVertices.reserve(24 * m_entities.size());
            
            Model::EntitySet::const_iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (m_filter.visible(entity)) {
                    if (entity->selected()) {
                        BuildBoundsVertices builder(selectedVertices);
                        eachBBoxEdge(entity->bounds(), builder);
                    } else {
                        BuildColoredBoundsVertices builder(vertices, boundsColor(*entity));
                        eachBBoxEdge(entity->bounds(), builder);
                    }
                }
            }
            
            m_boundsRenderer = EdgeRenderer(vertices);
            m_selectedBoundsRenderer = EdgeRenderer(selectedVertices);
            m_boundsValid = true;
        }

        const Color& EntityRenderer::boundsColor(const Model::Entity& entity) const {
            const Assets::EntityDefinition* definition = entity.definition();
            if (definition == NULL) {
                PreferenceManager& prefs = PreferenceManager::instance();
                return prefs.getColor(Preferences::UndefinedEntityColor);
            }
            return definition->color();
        }
    }
}
