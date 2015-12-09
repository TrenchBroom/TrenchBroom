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

#include "EntityRenderer.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "CollectionUtils.h"
#include "Preferences.h"
#include "PreferenceManager.h"
#include "Assets/EntityDefinition.h"
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
        class EntityRenderer::EntityClassnameAnchor : public TextAnchor {
        private:
            const Model::Entity* m_entity;
        public:
            EntityClassnameAnchor(const Model::Entity* entity) :
            m_entity(entity) {}
        private:
            Vec3f basePosition() const {
                const Assets::EntityModel* model = m_entity->model();
                Vec3f position = m_entity->bounds().center();
                position[2] = float(m_entity->bounds().max.z());
                if (model != NULL) {
                    const Assets::ModelSpecification spec = m_entity->modelSpecification();
                    const BBox3f modelBounds = model->bounds(spec.skinIndex, spec.frameIndex);
                    const Vec3f origin = m_entity->origin();
                    position[2] = std::max(position[2], modelBounds.max.z() + origin.z());
                }
                position[2] += 2.0f;
                return position;
            }
            
            TextAlignment::Type alignment() const {
                return TextAlignment::Bottom;
            }
        };
        
        EntityRenderer::EntityRenderer(Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext) :
        m_editorContext(editorContext),
        m_modelRenderer(entityModelManager, m_editorContext),
        m_boundsValid(false),
        m_showOverlays(true),
        m_showOccludedOverlays(false),
        m_tint(false),
        m_overrideBoundsColor(false),
        m_showOccludedBounds(false),
        m_showAngles(false),
        m_showHiddenEntities(false),
        m_vbo(0xFFF) {}
        
        void EntityRenderer::setEntities(const Model::EntityList& entities) {
            m_entities = entities;
            reloadModels();
            invalidate();
        }

        void EntityRenderer::invalidate() {
            invalidateBounds();
        }

        void EntityRenderer::clear() {
            m_entities.clear();
            m_wireframeBoundsRenderer = DirectEdgeRenderer();
            m_solidBoundsRenderer = TriangleRenderer();
            m_modelRenderer.clear();
        }

        void EntityRenderer::reloadModels() {
            m_modelRenderer.setEntities(m_entities.begin(), m_entities.end());
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

            if (renderContext.showEntityBounds())
                renderWireframeBounds(renderBatch);
            if (m_showHiddenEntities || renderContext.showPointEntities())
                renderSolidBounds(renderBatch);
        }
        
        void EntityRenderer::renderWireframeBounds(RenderBatch& renderBatch) {
            if (m_showOccludedBounds)
                m_wireframeBoundsRenderer.renderOnTop(renderBatch, m_overrideBoundsColor, m_boundsColor);
            m_wireframeBoundsRenderer.render(renderBatch, m_overrideBoundsColor, m_boundsColor);
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
                
                Model::EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    const Model::Entity* entity = *it;
                    if (m_showHiddenEntities || m_editorContext.visible(entity)) {
                        const EntityClassnameAnchor anchor(entity);
                        if (m_showOccludedOverlays)
                            renderService.renderStringOnTop(entityString(entity), anchor);
                        else
                            renderService.renderString(entityString(entity), anchor);
                    }
                }
            }
        }
        
        void EntityRenderer::renderAngles(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (!m_showAngles)
                return;
            
            static const float maxDistance2 = 500.0f * 500.0f;
            typedef VertexSpecs::P3::Vertex Vertex;
            const Vec3f::List arrow = arrowHead(9.0f, 6.0f);
            
            Vertex::List vertices;
            Model::EntityList::const_iterator it, end;
            for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                const Model::Entity* entity = *it;
                if (!m_showHiddenEntities && !m_editorContext.visible(entity))
                    continue;
                
                const Mat4x4f rotation(entity->rotation());
                const Vec3f direction = rotation * Vec3f::PosX;
                const Vec3f center(entity->bounds().center());
                
                const Vec3f toCam = renderContext.camera().position() - center;
                if (toCam.squaredLength() > maxDistance2)
                    continue;

                Vec3f onPlane = toCam - toCam.dot(direction) * direction;
                if (onPlane.null())
                    continue;
                
                onPlane.normalize();
                
                const Vec3f rotZ = rotation * Vec3f::PosZ;
                const float angle = -angleBetween(rotZ, onPlane, direction);
                const Mat4x4f matrix = translationMatrix(center) * rotationMatrix(direction, angle) * rotation * translationMatrix(16.0f * Vec3f::PosX);
                
                for (size_t i = 0; i < 3; ++i)
                    vertices.push_back(Vertex(matrix * arrow[i]));
            }
            
            const size_t vertexCount = vertices.size();
            if (vertexCount == 0)
                return;
            
            VertexArray array = VertexArray::swap(vertices);
            
            ActivateVbo activate(m_vbo);
            array.prepare(m_vbo);
            
            ActiveShader shader(renderContext.shaderManager(), Shaders::HandleShader);

            glAssert(glDepthMask(GL_FALSE));

            glAssert(glDisable(GL_DEPTH_TEST));
            glAssert(glPolygonMode(GL_FRONT, GL_LINE));
            shader.set("Color", m_angleColor);
            array.render(GL_TRIANGLES);

            glAssert(glPolygonMode(GL_FRONT, GL_FILL));
            glAssert(glDepthMask(GL_TRUE));
            glAssert(glEnable(GL_DEPTH_TEST));
        }

        Vec3f::List EntityRenderer::arrowHead(const float length, const float width) const {
            // clockwise winding
            Vec3f::List result(3);
            result[0] = Vec3f(0.0f,    width / 2.0f, 0.0f);
            result[1] = Vec3f(length,          0.0f, 0.0f);
            result[2] = Vec3f(0.0f,   -width / 2.0f, 0.0f);
            return result;
        }

        struct EntityRenderer::BuildColoredSolidBoundsVertices {
            VertexSpecs::P3NC4::Vertex::List& vertices;
            Color color;
            
            BuildColoredSolidBoundsVertices(VertexSpecs::P3NC4::Vertex::List& i_vertices, const Color& i_color) :
            vertices(i_vertices),
            color(i_color) {}
            
            void operator()(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4, const Vec3& n) {
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v1, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v2, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v3, n, color));
                vertices.push_back(VertexSpecs::P3NC4::Vertex(v4, n, color));
            }
        };

        struct EntityRenderer::BuildColoredWireframeBoundsVertices {
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
        
        struct EntityRenderer::BuildWireframeBoundsVertices {
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
            VertexSpecs::P3NC4::Vertex::List solidVertices;
            solidVertices.reserve(36 * m_entities.size());
            
            if (m_overrideBoundsColor) {
                VertexSpecs::P3::Vertex::List wireframeVertices;
                wireframeVertices.reserve(24 * m_entities.size());

                BuildWireframeBoundsVertices wireframeBoundsBuilder(wireframeVertices);
                Model::EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    const Model::Entity* entity = *it;
                    if (m_editorContext.visible(entity)) {
                        eachBBoxEdge(entity->bounds(), wireframeBoundsBuilder);
                        if (!entity->hasChildren() && entity->model() == NULL) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(entity));
                            eachBBoxFace(entity->bounds(), solidBoundsBuilder);
                        }
                    }
                }
                
                m_wireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::swap(wireframeVertices), GL_LINES);
            } else {
                VertexSpecs::P3C4::Vertex::List wireframeVertices;
                wireframeVertices.reserve(24 * m_entities.size());

                Model::EntityList::const_iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it) {
                    const Model::Entity* entity = *it;
                    if (m_editorContext.visible(entity)) {
                        if (!entity->hasChildren() && entity->model() == NULL) {
                            BuildColoredSolidBoundsVertices solidBoundsBuilder(solidVertices, boundsColor(entity));
                            eachBBoxFace(entity->bounds(), solidBoundsBuilder);
                        } else {
                            BuildColoredWireframeBoundsVertices wireframeBoundsBuilder(wireframeVertices, boundsColor(entity));
                            eachBBoxEdge(entity->bounds(), wireframeBoundsBuilder);
                        }
                    }
                }

                m_wireframeBoundsRenderer = DirectEdgeRenderer(VertexArray::swap(wireframeVertices), GL_LINES);
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
            if (definition == NULL)
                return m_boundsColor;
            return definition->color();
        }
    }
}
