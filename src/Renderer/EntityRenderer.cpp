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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
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
#include "Model/ModelFilter.h"
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

        EntityRenderer::EntityClassnameFilter::EntityClassnameFilter(const Model::ModelFilter& filter) :
        m_filter(filter) {}

        bool EntityRenderer::EntityClassnameFilter::stringVisible(RenderContext& context, const Key& entity) const {
            return m_filter.visible(entity);
        }

        EntityRenderer::EntityClassnameColorProvider::EntityClassnameColorProvider(const Color& textColor, const Color& backgroundColor) :
        m_textColor(textColor),
        m_backgroundColor(backgroundColor) {}

        Color EntityRenderer::EntityClassnameColorProvider::textColor(RenderContext& context, const Key& entity) const {
            return m_textColor;
        }
        
        Color EntityRenderer::EntityClassnameColorProvider::backgroundColor(RenderContext& context, const Key& entity) const {
            return m_backgroundColor;
        }

        EntityRenderer::EntityRenderer(FontManager& fontManager, const Model::ModelFilter& filter) :
        m_filter(filter),
        m_classnameRenderer(ClassnameRenderer(font(fontManager))),
        m_modelRenderer(m_filter),
        m_boundsValid(false),
        m_overrideBoundsColor(false),
        m_renderOccludedBounds(false),
        m_applyTinting(false) {
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
        
        void EntityRenderer::updateEntity(Model::Entity* entity) {
            assert(entity != NULL);
            assert(m_entities.count(entity) == 1);
            
            m_classnameRenderer.updateString(entity, entity->classname());
            m_modelRenderer.updateEntity(entity);
            invalidateBounds();
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
        
        void EntityRenderer::clear() {
            m_entities.clear();
            m_classnameRenderer.clear();
            m_modelRenderer.clear();
        }

        void EntityRenderer::render(RenderContext& context) {
            renderBounds(context);
            renderModels(context);
            renderClassnames(context);
        }

        const Color& EntityRenderer::overlayTextColor() const {
            return m_overlayTextColor;
        }
        
        void EntityRenderer::setOverlayTextColor(const Color& overlayTextColor) {
            m_overlayTextColor = overlayTextColor;
        }
        
        const Color& EntityRenderer::overlayBackgroundColor() const {
            return m_overlayBackgroundColor;
        }
        
        void EntityRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor) {
            m_overlayBackgroundColor = overlayBackgroundColor;
        }
        
        bool EntityRenderer::overrideBoundsColor() const {
            return m_overrideBoundsColor;
        }
        
        void EntityRenderer::setOverrideBoundsColor(const bool overrideBoundsColor) {
            m_overrideBoundsColor = overrideBoundsColor;
        }
        
        const Color& EntityRenderer::boundsColor() const {
            return m_boundsColor;
        }
        
        void EntityRenderer::setBoundsColor(const Color& boundsColor) {
            m_boundsColor = boundsColor;
        }
        
        bool EntityRenderer::renderOccludedBounds() const {
            return m_renderOccludedBounds;
        }
        
        void EntityRenderer::setRenderOccludedBounds(const bool renderOccludedBounds) {
            if (m_renderOccludedBounds == renderOccludedBounds)
                return;
            m_renderOccludedBounds = renderOccludedBounds;
            invalidateBounds();
        }
        
        const Color& EntityRenderer::occludedBoundsColor() const {
            return m_occludedBoundsColor;
        }
        
        void EntityRenderer::setOccludedBoundsColor(const Color& occludedBoundsColor) {
            if (m_occludedBoundsColor == occludedBoundsColor)
                return;
            m_occludedBoundsColor = occludedBoundsColor;
            invalidateBounds();
        }

        bool EntityRenderer::applyTinting() const {
            return m_applyTinting;
        }
        
        void EntityRenderer::setApplyTinting(const bool applyTinting) {
            m_applyTinting = applyTinting;
        }
        
        const Color& EntityRenderer::tintColor() const {
            return m_tintColor;
        }
        
        void EntityRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }

        void EntityRenderer::renderBounds(RenderContext& context) {
            if (!m_boundsValid)
                validateBounds();

            renderWireframeBounds(context);
            renderSolidBounds(context);
        }
        
        void EntityRenderer::renderWireframeBounds(RenderContext& context) {
            if (renderOccludedBounds()) {
                glDisable(GL_DEPTH_TEST);
                if (overrideBoundsColor())
                    m_wireframeBoundsRenderer.setColor(occludedBoundsColor());
                m_wireframeBoundsRenderer.render(context);
                glEnable(GL_DEPTH_TEST);
            }
            
            glSetEdgeOffset(0.025f);
            if (overrideBoundsColor())
                m_wireframeBoundsRenderer.setColor(boundsColor());
            m_wireframeBoundsRenderer.render(context);
            glResetEdgeOffset();
        }
        
        void EntityRenderer::renderSolidBounds(RenderContext& context) {
            m_solidBoundsRenderer.setApplyTinting(m_applyTinting);
            m_solidBoundsRenderer.setTintColor(m_tintColor);
            m_solidBoundsRenderer.render(context);
        }
        
        void EntityRenderer::renderClassnames(RenderContext& context) {
            EntityClassnameFilter textFilter(m_filter);
            EntityClassnameColorProvider colorProvider(overlayTextColor(), overlayBackgroundColor());
            m_classnameRenderer.render(context, textFilter, colorProvider,
                                       Shaders::ColoredTextShader, Shaders::TextBackgroundShader);
        }
        
        void EntityRenderer::renderModels(RenderContext& context) {
            m_modelRenderer.setApplyTinting(m_applyTinting);
            m_modelRenderer.setTintColor(m_tintColor);
            m_modelRenderer.render(context);
        }

        TextureFont& EntityRenderer::font(FontManager& fontManager) {
            PreferenceManager& prefs = PreferenceManager::instance();
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            const size_t fontSize = static_cast<size_t>(prefs.getInt(Preferences::RendererFontSize));
            return fontManager.font(FontDescriptor(fontName, fontSize));
        }

        struct BuildColoredSolidBoundsVertices {
            VertexSpecs::P3C4::Vertex::List& vertices;
            Color color;
            
            BuildColoredSolidBoundsVertices(VertexSpecs::P3C4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4) {
                vertices.push_back(VertexSpecs::P3C4::Vertex(v1, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v2, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v3, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v3, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v4, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v1, color));
            }
        };

        struct BuildColoredWireframeBoundsVertices {
            VertexSpecs::P3C4::Vertex::List& vertices;
            Color color;
            
            BuildColoredWireframeBoundsVertices(VertexSpecs::P3C4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3C4::Vertex(v1, color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(v2, color));
            }
        };
        
        struct BuildWireframeBoundsVertices {
            VertexSpecs::P3::Vertex::List& vertices;
            
            BuildWireframeBoundsVertices(VertexSpecs::P3::Vertex::List& i_vertices) :
            vertices(i_vertices) {}
            
            void operator()(const Vec3& v1, const Vec3& v2) {
                vertices.push_back(VertexSpecs::P3::Vertex(v1));
                vertices.push_back(VertexSpecs::P3::Vertex(v2));
            }
        };
        
        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }
        
        void EntityRenderer::validateBounds() {
            VertexSpecs::P3C4::Vertex::List solidVertices;
            solidVertices.reserve(36 * m_entities.size());
            
            if (m_overrideBoundsColor) {
                VertexSpecs::P3::Vertex::List wireframeVertices;
                wireframeVertices.reserve(24 * m_entities.size());

                BuildWireframeBoundsVertices wireframeBoundsBuilder(wireframeVertices);
                Model::EntitySet::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    const Model::Entity* entity = *it;
                    if (m_filter.visible(entity)) {
                        eachBBoxEdge(entity->bounds(), wireframeBoundsBuilder);
                        if (entity->model() == NULL) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(*entity));
                            eachBBoxFace(entity->bounds(), solidBoundsBuilder);
                        }
                    }
                }
                
                m_wireframeBoundsRenderer = EdgeRenderer(wireframeVertices);
            } else {
                VertexSpecs::P3C4::Vertex::List wireframeVertices;
                wireframeVertices.reserve(24 * m_entities.size());

                Model::EntitySet::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    const Model::Entity* entity = *it;
                    if (m_filter.visible(entity)) {
                        if (entity->brushes().empty() && entity->model() == NULL) {
                            BuildColoredWireframeBoundsVertices wireframeBoundsBuilder(wireframeVertices, boundsColor());
                            eachBBoxEdge(entity->bounds(), wireframeBoundsBuilder);

                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(*entity));
                            eachBBoxFace(entity->bounds(), solidBoundsBuilder);
                        } else {
                            BuildColoredWireframeBoundsVertices wireframeBoundsBuilder(wireframeVertices, boundsColor(*entity));
                            eachBBoxEdge(entity->bounds(), wireframeBoundsBuilder);
                        }
                    }
                }

                m_wireframeBoundsRenderer = EdgeRenderer(wireframeVertices);
            }
            m_solidBoundsRenderer = TriangleRenderer(solidVertices);
            m_boundsValid = true;
        }

        const Color& EntityRenderer::boundsColor(const Model::Entity& entity) const {
            const Assets::EntityDefinition* definition = entity.definition();
            if (definition == NULL)
                return boundsColor();
            return definition->color();
        }
    }
}
