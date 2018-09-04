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

#include "UVViewHelper.h"

#include "VecMath.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/PickResult.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/UVView.h"

namespace TrenchBroom {
    namespace View {
        UVViewHelper::UVViewHelper(Renderer::OrthographicCamera& camera) :
        m_camera(camera),
        m_zoomValid(false),
        m_face(nullptr),
        m_subDivisions(1, 1) {}
        
        bool UVViewHelper::valid() const {
            return m_face != nullptr;
        }
        
        Model::BrushFace* UVViewHelper::face() const {
            return m_face;
        }
        
        const Assets::Texture* UVViewHelper::texture() const {
            if (!valid())
                return nullptr;
            return m_face->texture();
        }
        
        void UVViewHelper::setFace(Model::BrushFace* face) {
            if (face != m_face) {
                m_face = face;
                if (m_face != nullptr) {
                    resetCamera();
                    resetOrigin();
                }
            }
        }
        
        void UVViewHelper::cameraViewportChanged() {
            // If the user selects a face before the texturing view was shown for the first time, the size of the view
            // might still have been off, resulting in invalid zoom factors. Therefore we must reset the zoom whenever
            // the viewport changes until a valid zoom factor can be computed.
            if (valid() && !m_zoomValid)
                resetZoom();
        }
        
        const vec2i& UVViewHelper::subDivisions() const {
            return m_subDivisions;
        }

        vec2 UVViewHelper::stripeSize() const {
            assert(valid());
            
            const Assets::Texture* texture = m_face->texture();
            if (texture == nullptr)
                return vec2::zero;
            const FloatType width  = static_cast<FloatType>(texture->width())  / static_cast<FloatType>(m_subDivisions.x());
            const FloatType height = static_cast<FloatType>(texture->height()) / static_cast<FloatType>(m_subDivisions.y());
            return vec2(width, height);
        }

        void UVViewHelper::setSubDivisions(const vec2i& subDivisions) {
            m_subDivisions = subDivisions;
        }
        
        const vec3 UVViewHelper::origin() const {
            assert(valid());

            return m_origin;
        }

        const vec2f UVViewHelper::originInFaceCoords() const {
            const mat4x4 toFace = m_face->toTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            return vec2f(toFace * origin());
        }
        
        const vec2f UVViewHelper::originInTexCoords() const {
            assert(valid());
            
            const mat4x4 toFace  = m_face->toTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            return vec2f(toFace * origin());
        }
        
        void UVViewHelper::setOriginInFaceCoords(const vec2f& originInFaceCoords) {
            const mat4x4 fromFace = m_face->fromTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            m_origin = fromFace * vec3(originInFaceCoords);
        }
        
        const Renderer::Camera& UVViewHelper::camera() const {
            return m_camera;
        }

        float UVViewHelper::cameraZoom() const {
            return m_camera.zoom();
        }
        
        void UVViewHelper::pickTextureGrid(const ray3& ray, const Model::Hit::HitType hitTypes[2], Model::PickResult& pickResult) const {
            assert(valid());
            
            const auto* texture = m_face->texture();
            if (texture != nullptr) {
                
                const auto& boundary = m_face->boundary();
                const auto distance = intersect(ray, boundary);
                const auto hitPointInWorldCoords = ray.pointAtDistance(distance);
                const auto hitPointInTexCoords = m_face->toTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true) * hitPointInWorldCoords;
                
                const auto maxDistance = 5.0 / cameraZoom();
                const auto stripeSize = UVViewHelper::stripeSize();
                
                for (size_t i = 0; i < 2; ++i) {
                    const auto closestStrip = Math::roundToMultiple(hitPointInTexCoords[i], stripeSize[i]);
                    const auto error = Math::abs(hitPointInTexCoords[i] - closestStrip);
                    if (error <= maxDistance) {
                        const auto index = static_cast<int>(Math::round(hitPointInTexCoords[i] / stripeSize[i]));
                        pickResult.addHit(Model::Hit(hitTypes[i], distance, hitPointInWorldCoords, index, error));
                    }
                }
            }
        }

        vec2f UVViewHelper::snapDelta(const vec2f& delta, const vec2f& distance) const {
            const float zoom = cameraZoom();
            
            vec2f result;
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(distance[i]) < 4.0f / zoom)
                    result[i] = delta[i] + distance[i];
                else
                    result[i] = Math::round(delta[i]);
            }
            return result;
        }
        
        vec2f UVViewHelper::computeDistanceFromTextureGrid(const vec3& position) const {
            const vec2 stripe = stripeSize();
            assert(stripe.x() != 0.0 && stripe.y() != 0);
            
            const vec2 closest = roundToMultiple(position.xy(), stripe);
            return vec2f(closest - position.xy());
        }
        
        void UVViewHelper::computeOriginHandleVertices(vec3& x1, vec3& x2, vec3& y1, vec3& y2) const {
            assert(valid());
            
            const auto toTex   = m_face->toTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            const auto toWorld = m_face->fromTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            computeLineVertices(vec2(originInFaceCoords()), x1, x2, y1, y2, toTex, toWorld);
        }

        void UVViewHelper::computeScaleHandleVertices(const vec2& pos, vec3& x1, vec3& x2, vec3& y1, vec3& y2) const {
            assert(valid());
            
            const mat4x4 toTex   = m_face->toTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            const mat4x4 toWorld = m_face->fromTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            computeLineVertices(pos, x1, x2, y1, y2, toTex, toWorld);
        }
        
        void UVViewHelper::computeLineVertices(const vec2& pos, vec3& x1, vec3& x2, vec3& y1, vec3& y2, const mat4x4& toTex, const mat4x4& toWorld) const {
            const auto viewportVertices = toTex * m_camera.viewportVertices();
            const auto viewportBounds = bbox3::mergeAll(std::begin(viewportVertices), std::end(viewportVertices));
            const auto& min = viewportBounds.min;
            const auto& max = viewportBounds.max;
            
            x1 = toWorld * vec3(pos.x(), min.y(), 0.0);
            x2 = toWorld * vec3(pos.x(), max.y(), 0.0);
            y1 = toWorld * vec3(min.x(), pos.y(), 0.0);
            y2 = toWorld * vec3(max.x(), pos.y(), 0.0);
        }

        void UVViewHelper::resetOrigin() {
            assert(valid());

            const auto toTex = m_face->toTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            const auto transformedVertices = toTex * m_face->vertexPositions();
            const auto bounds = bbox3::mergeAll(std::begin(transformedVertices), std::end(transformedVertices));
            
            const vec3 vertices[] = {
                bounds.corner(bbox3::Corner::min, bbox3::Corner::min, bbox3::Corner::min),
                bounds.corner(bbox3::Corner::min, bbox3::Corner::max, bbox3::Corner::min),
                bounds.corner(bbox3::Corner::max, bbox3::Corner::max, bbox3::Corner::min),
                bounds.corner(bbox3::Corner::max, bbox3::Corner::min, bbox3::Corner::min)
            };
            
            const auto fromTex = m_face->fromTexCoordSystemMatrix(vec2f::zero, vec2f::one, true);
            const auto toCam = mat4x4(m_camera.viewMatrix());
            
            for (size_t i = 0; i < 4; ++i) {
                const auto vertex = toCam * fromTex * vertices[i];
                if (vertex.x() < 0.0 && vertex.y() < 0.0) {
                    setOriginInFaceCoords(vec2f(vertices[i]));
                    break;
                }
            }
        }
        
        void UVViewHelper::resetCamera() {
            assert(valid());

            const auto& normal = m_face->boundary().normal;
            vec3 right;
            
            if (Math::lt(Math::abs(dot(vec3::pos_z, normal)), 1.0)) {
                right = normalize(cross(vec3::pos_z, normal));
            } else {
                right = vec3::pos_x;
            }
            const auto up = normalize(cross(normal, right));
            
            m_camera.setNearPlane(-1.0f);
            m_camera.setFarPlane( +1.0f);
            m_camera.setDirection(vec3f(-normal), vec3f(up));

            const auto position = m_face->boundsCenter();
            m_camera.moveTo(vec3f(position));
            resetZoom();
        }
        
        void UVViewHelper::resetZoom() {
            assert(valid());
            
            auto w = static_cast<float>(m_camera.unzoomedViewport().width);
            auto h = static_cast<float>(m_camera.unzoomedViewport().height);
            
            if (w <= 1.0f || h <= 1.0f) {
                return;
            }

            if (w > 80.0f) {
                w -= 80.0f;
            }
            if (h > 80.0f) {
                h -= 80.0f;
            }

            const auto bounds = computeFaceBoundsInCameraCoords();
            const auto boundsSize = vec3f(bounds.size());

            auto zoom = 3.0f;
            zoom = Math::min(zoom, w / boundsSize.x());
            zoom = Math::min(zoom, h / boundsSize.y());
            if (zoom > 0.0f) {
                m_camera.setZoom(zoom);
                m_zoomValid = true;
            }
        }

        bbox3 UVViewHelper::computeFaceBoundsInCameraCoords() const {
            assert(valid());
            
            const auto transform = coordinateSystemMatrix(vec3(m_camera.right()), vec3(m_camera.up()), vec3(-m_camera.direction()), vec3(m_camera.position()));

            bbox3 result;
            const Model::BrushFace::VertexList vertices = m_face->vertices();
            Model::BrushFace::VertexList::const_iterator it = std::begin(vertices);
            Model::BrushFace::VertexList::const_iterator end = std::end(vertices);
            
            result.min = result.max = transform * (*it++)->position();
            while (it != end) {
                result = merge(result, transform * (*it++)->position());
            }
            return result;
        }
    }
}
