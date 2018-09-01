/*
 Copyright (C) 2010-2017 Kristian Duske
 
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
#include "Assets/EntityModelManager.h"
#include "Assets/ModelDefinition.h"
#include "Assets/EntityModel.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Renderer/Camera.h"
#include "Renderer/IndexRangeMap.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/TextAnchor.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class EntityRenderer::EntityClassnameAnchor : public TextAnchor3D {
        private:
            const Model::Entity* m_entity;
        public:
            EntityClassnameAnchor(const Model::Entity* entity) :
            m_entity(entity) {}
        private:
            vec3f basePosition() const override {
                auto position = vec3f(m_entity->bounds().center());
                position[2] = float(m_entity->bounds().max.z());
                position[2] += 2.0f;
                return position;
            }
            
            TextAlignment::Type alignment() const override {
                return TextAlignment::Bottom;
            }
        };
        
        EntityRenderer::EntityRenderer(Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext) :
        m_entityModelManager(entityModelManager),
        m_editorContext(editorContext),
        m_modelRenderer(m_entityModelManager, m_editorContext),
        m_boundsValid(false),
        m_showOverlays(true),
        m_showOccludedOverlays(false),
        m_tint(false),
        m_overrideBoundsColor(false),
        m_showOccludedBounds(false),
        m_showAngles(false),
        m_showHiddenEntities(false) {}
        
        void EntityRenderer::setEntities(const Model::EntityList& entities) {
            m_entities = entities;
            m_modelRenderer.setEntities(std::begin(m_entities), std::end(m_entities));
            invalidate();
        }

        void EntityRenderer::invalidate() {
            invalidateBounds();
            reloadModels();
        }

        void EntityRenderer::clear() {
            m_entities.clear();
            m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer();
            m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer();
            m_solidBoundsRenderer = TriangleRenderer();
            m_modelRenderer.clear();
        }

        void EntityRenderer::reloadModels() {
            m_modelRenderer.updateEntities(std::begin(m_entities), std::end(m_entities));
        }

        void EntityRenderer::setShowOverlays(const bool showOverlays) {
            m_showOverlays = showOverlays;
        }

        void EntityRenderer::setOverlayTextColor(const Color& overlayTextColor) {
            m_overlayTextColor = overlayTextColor;
        }
        
        void EntityRenderer::setOverlayBackgroundColor(const Color& overlayBackgroundColor) {
            m_overlayBackgroundColor = overlayBackgroundColor;
        }
        
        void EntityRenderer::setShowOccludedOverlays(const bool showOccludedOverlays) {
            m_showOccludedOverlays = showOccludedOverlays;
        }
        
        void EntityRenderer::setTint(const bool tint) {
            m_tint = tint;
        }
        
        void EntityRenderer::setTintColor(const Color& tintColor) {
            m_tintColor = tintColor;
        }
        
        void EntityRenderer::setOverrideBoundsColor(const bool overrideBoundsColor) {
            m_overrideBoundsColor = overrideBoundsColor;
        }
        
        void EntityRenderer::setBoundsColor(const Color& boundsColor) {
            m_boundsColor = boundsColor;
        }
        
        void EntityRenderer::setShowOccludedBounds(const bool showOccludedBounds) {
            m_showOccludedBounds = showOccludedBounds;
        }
        
        void EntityRenderer::setOccludedBoundsColor(const Color& occludedBoundsColor) {
            m_occludedBoundsColor = occludedBoundsColor;
        }
        
        void EntityRenderer::setShowAngles(const bool showAngles) {
            m_showAngles = showAngles;
        }
        
        void EntityRenderer::setAngleColor(const Color& angleColor) {
            m_angleColor = angleColor;
        }
        
        void EntityRenderer::setShowHiddenEntities(const bool showHiddenEntities) {
            m_showHiddenEntities = showHiddenEntities;
        }

        void EntityRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_entities.empty()) {
                renderBounds(renderContext, renderBatch);
                renderModels(renderContext, renderBatch);
                renderClassnames(renderContext, renderBatch);
                renderAngles(renderContext, renderBatch);
            }
        }
        
        void EntityRenderer::renderBounds(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_boundsValid)
                validateBounds();

            if (renderContext.showPointEntityBounds()) {
                renderPointEntityWireframeBounds(renderBatch);
            }
            if (renderContext.showBrushEntityBounds()) {
                renderBrushEntityWireframeBounds(renderBatch);
            }

            if (m_showHiddenEntities || renderContext.showPointEntities())
                renderSolidBounds(renderBatch);
        }

        void EntityRenderer::renderPointEntityWireframeBounds(RenderBatch& renderBatch) {
            if (m_showOccludedBounds) {
                m_pointEntityWireframeBoundsRenderer.renderOnTop(renderBatch, m_overrideBoundsColor, m_occludedBoundsColor);
            }
            m_pointEntityWireframeBoundsRenderer.render(renderBatch, m_overrideBoundsColor, m_boundsColor);
        }

        void EntityRenderer::renderBrushEntityWireframeBounds(RenderBatch& renderBatch) {
            if (m_showOccludedBounds) {
                m_brushEntityWireframeBoundsRenderer.renderOnTop(renderBatch, m_overrideBoundsColor, m_occludedBoundsColor);
            }
            m_brushEntityWireframeBoundsRenderer.render(renderBatch, m_overrideBoundsColor, m_boundsColor);
        }

        void EntityRenderer::renderSolidBounds(RenderBatch& renderBatch) {
            m_solidBoundsRenderer.setApplyTinting(m_tint);
            m_solidBoundsRenderer.setTintColor(m_tintColor);
            renderBatch.add(&m_solidBoundsRenderer);
        }
        
        void EntityRenderer::renderModels(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (m_showHiddenEntities || (renderContext.showPointEntities() &&
                                         renderContext.showPointEntityModels())) {
                m_modelRenderer.setApplyTinting(m_tint);
                m_modelRenderer.setTintColor(m_tintColor);
                m_modelRenderer.setShowHiddenEntities(m_showHiddenEntities);
                m_modelRenderer.render(renderBatch);
            }
        }
        
        void EntityRenderer::renderClassnames(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (m_showOverlays && renderContext.showEntityClassnames()) {
                Renderer::RenderService renderService(renderContext, renderBatch);
                renderService.setForegroundColor(m_overlayTextColor);
                renderService.setBackgroundColor(m_overlayBackgroundColor);
                
                for (const Model::Entity* entity : m_entities) {
                    if (m_showHiddenEntities || m_editorContext.visible(entity)) {
                        if (entity->group() == nullptr || entity->group() == m_editorContext.currentGroup()) {
                            if (m_showOccludedOverlays)
                                renderService.setShowOccludedObjects();
                            else
                                renderService.setHideOccludedObjects();
                            renderService.renderString(entityString(entity), EntityClassnameAnchor(entity));
                        }
                    }
                }
            }
        }
        
        void EntityRenderer::renderAngles(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_showAngles) {
                return;
            }

            static const auto maxDistance2 = 500.0f * 500.0f;
            const auto arrow = arrowHead(9.0f, 6.0f);
            
            RenderService renderService(renderContext, renderBatch);
            renderService.setShowOccludedObjectsTransparent();
            renderService.setForegroundColor(m_angleColor);
            
            vec3f::List vertices(3);
            for (const auto* entity : m_entities) {
                if (!m_showHiddenEntities && !m_editorContext.visible(entity)) {
                    continue;
                }

                const auto rotation = mat4x4f(entity->rotation());
                const auto direction = rotation * vec3f::pos_x;
                const auto center = vec3f(entity->bounds().center());
                
                const auto toCam = renderContext.camera().position() - center;
                if (squaredLength(toCam) > maxDistance2) {
                    continue;
                }

                auto onPlane = toCam - dot(toCam, direction) * direction;
                if (isZero(onPlane)) {
                    continue;
                }

                onPlane = normalize(onPlane);

                const auto rotZ = rotation * vec3f::pos_z;
                const auto angle = -angleBetween(rotZ, onPlane, direction);
                const auto matrix = translationMatrix(center) * rotationMatrix(direction, angle) * rotation * translationMatrix(16.0f * vec3f::pos_x);
                
                for (size_t i = 0; i < 3; ++i) {
                    vertices[i] = matrix * arrow[i];
                }
                renderService.renderPolygonOutline(vertices);
            }
        }

        vec3f::List EntityRenderer::arrowHead(const float length, const float width) const {
            // clockwise winding
            vec3f::List result(3);
            result[0] = vec3f(0.0f,    width / 2.0f, 0.0f);
            result[1] = vec3f(length,          0.0f, 0.0f);
            result[2] = vec3f(0.0f,   -width / 2.0f, 0.0f);
            return result;
        }

        struct EntityRenderer::BuildColoredSolidBoundsVertices {
            VertexSpecs::P3NC4::Vertex::List& vertices;
            Color color;
            
            BuildColoredSolidBoundsVertices(VertexSpecs::P3NC4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, const vec3& n) {
                vertices.push_back(VertexSpecs::P3NC4::Vertex(vec3f(v1), vec3f(n), color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(vec3f(v2), vec3f(n), color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(vec3f(v3), vec3f(n), color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(vec3f(v4), vec3f(n), color));
            }
        };

        struct EntityRenderer::BuildColoredWireframeBoundsVertices {
            VertexSpecs::P3C4::Vertex::List& vertices;
            Color color;
            
            BuildColoredWireframeBoundsVertices(VertexSpecs::P3C4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const vec3& v1, const vec3& v2) {
                vertices.push_back(VertexSpecs::P3C4::Vertex(vec3f(v1), color));
                vertices.push_back(VertexSpecs::P3C4::Vertex(vec3f(v2), color));
            }
        };
        
        struct EntityRenderer::BuildWireframeBoundsVertices {
            VertexSpecs::P3::Vertex::List& vertices;
            
            BuildWireframeBoundsVertices(VertexSpecs::P3::Vertex::List& i_vertices) :
            vertices(i_vertices) {}
            
            void operator()(const vec3& v1, const vec3& v2) {
                vertices.push_back(VertexSpecs::P3::Vertex(vec3f(v1)));
                vertices.push_back(VertexSpecs::P3::Vertex(vec3f(v2)));
            }
        };
        
        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }
        
        void EntityRenderer::validateBounds() {
            VertexSpecs::P3NC4::Vertex::List solidVertices;
            solidVertices.reserve(36 * m_entities.size());
            
            if (m_overrideBoundsColor) {
                VertexSpecs::P3::Vertex::List pointEntityWireframeVertices;
                VertexSpecs::P3::Vertex::List brushEntityWireframeVertices;

                pointEntityWireframeVertices.reserve(24 * m_entities.size());
                brushEntityWireframeVertices.reserve(24 * m_entities.size());

                BuildWireframeBoundsVertices pointEntityWireframeBoundsBuilder(pointEntityWireframeVertices);
                BuildWireframeBoundsVertices brushEntityWireframeBoundsBuilder(brushEntityWireframeVertices);

                for (const Model::Entity* entity : m_entities) {
                    if (m_editorContext.visible(entity)) {
                        const bool pointEntity = !entity->hasChildren();

                        entity->bounds().forEachEdge(pointEntity ? pointEntityWireframeBoundsBuilder : brushEntityWireframeBoundsBuilder);

                        if (!entity->hasChildren() && !m_entityModelManager.hasModel(entity)) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(entity));
                            entity->bounds().forEachFace(solidBoundsBuilder);
                        }
                    }
                }
                
                m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::swap(pointEntityWireframeVertices), GL_LINES);
                m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::swap(brushEntityWireframeVertices), GL_LINES);
            } else {
                VertexSpecs::P3C4::Vertex::List pointEntityWireframeVertices;
                VertexSpecs::P3C4::Vertex::List brushEntityWireframeVertices;

                pointEntityWireframeVertices.reserve(24 * m_entities.size());
                brushEntityWireframeVertices.reserve(24 * m_entities.size());

                for (const Model::Entity* entity : m_entities) {
                    if (m_editorContext.visible(entity)) {
                        const bool pointEntity = !entity->hasChildren();

                        if (!entity->hasChildren() && !m_entityModelManager.hasModel(entity)) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(entity));
                            entity->bounds().forEachFace(solidBoundsBuilder);
                        } else {
                            BuildColoredWireframeBoundsVertices pointEntityWireframeBoundsBuilder(pointEntityWireframeVertices, boundsColor(entity));
                            BuildColoredWireframeBoundsVertices brushEntityWireframeBoundsBuilder(brushEntityWireframeVertices, boundsColor(entity));

                            entity->bounds().forEachEdge(pointEntity ? pointEntityWireframeBoundsBuilder : brushEntityWireframeBoundsBuilder);
                        }
                    }
                }

                m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::swap(pointEntityWireframeVertices), GL_LINES);
                m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::swap(brushEntityWireframeVertices), GL_LINES);
            }
            
            m_solidBoundsRenderer = TriangleRenderer(VertexArray::swap(solidVertices), GL_QUADS);
            m_boundsValid = true;
        }

        AttrString EntityRenderer::entityString(const Model::Entity* entity) const {
            const Model::AttributeValue& classname = entity->classname();
            // const Model::AttributeValue& targetname = entity->attribute(Model::AttributeNames::Targetname);
            
            AttrString str;
            str.appendCentered(classname);
            // if (!targetname.empty())
               // str.appendCentered(targetname);
            return str;
        }

        const Color& EntityRenderer::boundsColor(const Model::Entity* entity) const {
            const Assets::EntityDefinition* definition = entity->definition();
            if (definition == nullptr) {
                return m_boundsColor;
            } else {
                return definition->color();
            }
        }
    }
}
