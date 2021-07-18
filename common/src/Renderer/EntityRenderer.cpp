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

#include "AttrString.h"
#include "FloatType.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinition.h"
#include "Assets/EntityModelManager.h"
#include "Model/EditorContext.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Renderer/Camera.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/TextAnchor.h"
#include "Renderer/GLVertexType.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/scalar.h>

#include <algorithm>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class EntityRenderer::EntityClassnameAnchor : public TextAnchor3D {
        private:
            const Model::EntityNode* m_entity;
        public:
            EntityClassnameAnchor(const Model::EntityNode* entity) :
            m_entity(entity) {}
        private:
            vm::vec3f basePosition() const override {
                auto position = vm::vec3f(m_entity->logicalBounds().center());
                position[2] = float(m_entity->logicalBounds().max.z());
                position[2] += 2.0f;
                return position;
            }

            TextAlignment::Type alignment() const override {
                return TextAlignment::Bottom;
            }
        };

        EntityRenderer::EntityRenderer(Logger& logger, Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext) :
        m_entityModelManager(entityModelManager),
        m_editorContext(editorContext),
        m_modelRenderer(logger, m_entityModelManager, m_editorContext),
        m_boundsValid(false),
        m_showOverlays(true),
        m_showOccludedOverlays(false),
        m_tint(false),
        m_overrideBoundsColor(false),
        m_showOccludedBounds(false),
        m_showAngles(false),
        m_showHiddenEntities(false) {}

        void EntityRenderer::setEntities(const std::vector<Model::EntityNode*>& entities) {
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

        void EntityRenderer::addEntity(Model::EntityNode* entity) {
            m_entities.push_back(entity);
            m_modelRenderer.addEntity(entity);
            invalidateBounds();
        }

        void EntityRenderer::removeEntity(Model::EntityNode* entity) {
            auto it = std::find(std::begin(m_entities), std::end(m_entities), entity);
            assert(it != m_entities.end());
            m_entities.erase(it);
            m_modelRenderer.removeEntity(entity);
            invalidateBounds();
        }

        void EntityRenderer::invalidateEntity(Model::EntityNode* entity) {
            m_modelRenderer.updateEntity(entity);
            invalidateBounds();
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

                for (const Model::EntityNode* entity : m_entities) {
                    if (m_showHiddenEntities || m_editorContext.visible(entity)) {
                        if (entity->containingGroup() == nullptr || entity->containingGroup() == m_editorContext.currentGroup()) {
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

            std::vector<vm::vec3f> vertices(3);
            for (const auto* entityNode : m_entities) {
                if (!m_showHiddenEntities && !m_editorContext.visible(entityNode)) {
                    continue;
                }

                const auto rotation = vm::mat4x4f(entityNode->entity().rotation());
                const auto direction = rotation * vm::vec3f::pos_x();
                const auto center = vm::vec3f(entityNode->logicalBounds().center());

                const auto toCam = renderContext.camera().position() - center;
                // only distance cull for perspective camera, since the 2D one is always very far from the level
                if (renderContext.camera().perspectiveProjection() && vm::squared_length(toCam) > maxDistance2) {
                    continue;
                }

                auto onPlane = toCam - dot(toCam, direction) * direction;
                if (vm::is_zero(onPlane, vm::Cf::almost_zero())) {
                    continue;
                }

                onPlane = vm::normalize(onPlane);

                const auto rotZ = rotation * vm::vec3f::pos_z();
                const auto angle = -vm::measure_angle(rotZ, onPlane, direction);
                const auto matrix = vm::translation_matrix(center) * vm::rotation_matrix(direction, angle) * rotation * vm::translation_matrix(16.0f * vm::vec3f::pos_x());

                for (size_t i = 0; i < 3; ++i) {
                    vertices[i] = matrix * arrow[i];
                }
                renderService.renderPolygonOutline(vertices);
            }
        }

        std::vector<vm::vec3f> EntityRenderer::arrowHead(const float length, const float width) const {
            // clockwise winding
            std::vector<vm::vec3f> result(3);
            result[0] = vm::vec3f(0.0f,    width / 2.0f, 0.0f);
            result[1] = vm::vec3f(length,          0.0f, 0.0f);
            result[2] = vm::vec3f(0.0f,   -width / 2.0f, 0.0f);
            return result;
        }

        struct EntityRenderer::BuildColoredSolidBoundsVertices {
            using Vertex = GLVertexTypes::P3NC4::Vertex;

            std::vector<Vertex>& vertices;
            Color color;

            BuildColoredSolidBoundsVertices(std::vector<Vertex>& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}

            void operator()(const vm::vec3& v1, const vm::vec3& v2, const vm::vec3& v3, const vm::vec3& v4, const vm::vec3& n) {
                vertices.emplace_back(vm::vec3f(v1), vm::vec3f(n), color);
                vertices.emplace_back(vm::vec3f(v2), vm::vec3f(n), color);
                vertices.emplace_back(vm::vec3f(v3), vm::vec3f(n), color);
                vertices.emplace_back(vm::vec3f(v4), vm::vec3f(n), color);
            }
        };

        struct EntityRenderer::BuildColoredWireframeBoundsVertices {
            using Vertex = GLVertexTypes::P3C4::Vertex;

            std::vector<Vertex>& vertices;
            Color color;

            BuildColoredWireframeBoundsVertices(std::vector<Vertex>& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}

            void operator()(const vm::vec3& v1, const vm::vec3& v2) {
                vertices.emplace_back(vm::vec3f(v1), color);
                vertices.emplace_back(vm::vec3f(v2), color);
            }
        };

        struct EntityRenderer::BuildWireframeBoundsVertices {
            std::vector<GLVertexTypes::P3::Vertex>& vertices;

            explicit BuildWireframeBoundsVertices(std::vector<GLVertexTypes::P3::Vertex>& i_vertices) :
            vertices(i_vertices) {}

            void operator()(const vm::vec3& v1, const vm::vec3& v2) {
                vertices.emplace_back(vm::vec3f(v1));
                vertices.emplace_back(vm::vec3f(v2));
            }
        };

        void EntityRenderer::invalidateBounds() {
            m_boundsValid = false;
        }

        void EntityRenderer::validateBounds() {
            std::vector<GLVertexTypes::P3NC4::Vertex> solidVertices;
            solidVertices.reserve(36 * m_entities.size());

            if (m_overrideBoundsColor) {
                using Vertex = GLVertexTypes::P3::Vertex;
                std::vector<Vertex> pointEntityWireframeVertices;
                std::vector<Vertex> brushEntityWireframeVertices;

                pointEntityWireframeVertices.reserve(24 * m_entities.size());
                brushEntityWireframeVertices.reserve(24 * m_entities.size());

                BuildWireframeBoundsVertices pointEntityWireframeBoundsBuilder(pointEntityWireframeVertices);
                BuildWireframeBoundsVertices brushEntityWireframeBoundsBuilder(brushEntityWireframeVertices);

                for (const Model::EntityNode* entityNode : m_entities) {
                    if (m_editorContext.visible(entityNode)) {
                        const bool pointEntity = !entityNode->hasChildren();
                        if (pointEntity) {
                            entityNode->logicalBounds().for_each_edge(pointEntityWireframeBoundsBuilder);
                        } else {
                            entityNode->logicalBounds().for_each_edge(brushEntityWireframeBoundsBuilder);
                        }

                        if (pointEntity && entityNode->entity().model() == nullptr) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(entityNode));
                            entityNode->logicalBounds().for_each_face(solidBoundsBuilder);
                        }
                    }
                }

                m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::move(std::move(pointEntityWireframeVertices)), PrimType::Lines);
                m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::move(std::move(brushEntityWireframeVertices)), PrimType::Lines);
            } else {
                using Vertex = GLVertexTypes::P3C4::Vertex;
                std::vector<Vertex> pointEntityWireframeVertices;
                std::vector<Vertex> brushEntityWireframeVertices;

                pointEntityWireframeVertices.reserve(24 * m_entities.size());
                brushEntityWireframeVertices.reserve(24 * m_entities.size());

                for (const Model::EntityNode* entityNode : m_entities) {
                    if (m_editorContext.visible(entityNode)) {
                        const bool pointEntity = !entityNode->hasChildren();

                        if (pointEntity && entityNode->entity().model() == nullptr) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(entityNode));
                            entityNode->logicalBounds().for_each_face(solidBoundsBuilder);
                        } else {
                            BuildColoredWireframeBoundsVertices pointEntityWireframeBoundsBuilder(pointEntityWireframeVertices, boundsColor(entityNode));
                            BuildColoredWireframeBoundsVertices brushEntityWireframeBoundsBuilder(brushEntityWireframeVertices, boundsColor(entityNode));

                            if (pointEntity) {
                                entityNode->logicalBounds().for_each_edge(pointEntityWireframeBoundsBuilder);
                            } else {
                                entityNode->logicalBounds().for_each_edge(brushEntityWireframeBoundsBuilder);
                            }
                        }
                    }
                }

                m_pointEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::move(std::move(pointEntityWireframeVertices)), PrimType::Lines);
                m_brushEntityWireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::move(std::move(brushEntityWireframeVertices)), PrimType::Lines);
            }

            m_solidBoundsRenderer = TriangleRenderer(VertexArray::move(std::move(solidVertices)), PrimType::Quads);
            m_boundsValid = true;
        }

        AttrString EntityRenderer::entityString(const Model::EntityNode* entityNode) const {
            const auto& classname = entityNode->entity().classname();
            // const Model::AttributeValue& targetname = entity->attribute(Model::AttributeNames::Targetname);

            AttrString str;
            str.appendCentered(classname);
            // if (!targetname.empty())
               // str.appendCentered(targetname);
            return str;
        }

        const Color& EntityRenderer::boundsColor(const Model::EntityNode* entityNode) const {
            const Assets::EntityDefinition* definition = entityNode->entity().definition();
            if (definition == nullptr) {
                return m_boundsColor;
            } else {
                return definition->color();
            }
        }
    }
}
