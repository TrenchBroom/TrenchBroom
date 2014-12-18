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

#include "UVViewHelper.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/Texture.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
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
        m_face(NULL),
        m_subDivisions(1, 1) {}
        
        bool UVViewHelper::valid() const {
            return m_face != NULL;
        }
        
        Model::BrushFace* UVViewHelper::face() const {
            return m_face;
        }
        
        const Assets::Texture* UVViewHelper::texture() const {
            if (!valid())
                return NULL;
            return m_face->texture();
        }
        
        void UVViewHelper::setFace(Model::BrushFace* face) {
            if (face != m_face) {
                m_face = face;
                if (m_face != NULL) {
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
        
        const Vec2i& UVViewHelper::subDivisions() const {
            return m_subDivisions;
        }

        Vec2 UVViewHelper::stripeSize() const {
            assert(valid());
            
            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return Vec2::Null;
            const FloatType width  = static_cast<FloatType>(texture->width())  / static_cast<FloatType>(m_subDivisions.x());
            const FloatType height = static_cast<FloatType>(texture->height()) / static_cast<FloatType>(m_subDivisions.y());
            return Vec2(width, height);
        }

        void UVViewHelper::setSubDivisions(const Vec2i& subDivisions) {
            m_subDivisions = subDivisions;
        }
        
        const Vec3 UVViewHelper::origin() const {
            assert(valid());

            const Mat4x4 fromTex = m_face->fromTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            return fromTex * Vec3(m_origin);
        }

        const Vec2f UVViewHelper::originInFaceCoords() const {
            return m_origin;
        }
        
        const Vec2f UVViewHelper::originInTexCoords() const {
            assert(valid());
            
            const Mat4x4 fromTex = m_face->fromTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            const Mat4x4 toFace  = m_face->toTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            return Vec2f(toFace * fromTex * Vec3(m_origin));
        }
        
        void UVViewHelper::setOrigin(const Vec2f& originInFaceCoords) {
            m_origin = originInFaceCoords;
        }
        
        const Renderer::Camera& UVViewHelper::camera() const {
            return m_camera;
        }

        float UVViewHelper::cameraZoom() const {
            const Vec2f& zoom = m_camera.zoom();
            assert(zoom.x() == zoom.y());
            return zoom.x();
        }
        
        void UVViewHelper::pickTextureGrid(const Ray3& ray, const Hit::HitType hitTypes[2], Hits& hits) const {
            assert(valid());
            
            const Assets::Texture* texture = m_face->texture();
            if (texture != NULL) {
                
                const Plane3& boundary = m_face->boundary();
                const FloatType rayDistance = ray.intersectWithPlane(boundary.normal, boundary.anchor());
                const Vec3 hitPointInWorldCoords = ray.pointAtDistance(rayDistance);
                const Vec3 hitPointInTexCoords = m_face->toTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true) * hitPointInWorldCoords;
                
                const FloatType maxDistance = 5.0 / cameraZoom();
                const Vec2 stripeSize = UVViewHelper::stripeSize();
                
                for (size_t i = 0; i < 2; ++i) {
                    const FloatType error = Math::abs(Math::remainder(hitPointInTexCoords[i], stripeSize[i]));
                    if (error <= maxDistance) {
                        const int index = static_cast<int>(Math::round(hitPointInTexCoords[i] / stripeSize[i]));
                        hits.addHit(Hit(hitTypes[i], rayDistance, hitPointInWorldCoords, index, error));
                    }
                }
            }
        }

        Vec2f UVViewHelper::snapDelta(const Vec2f& delta, const Vec2f& distance) const {
            const float zoom = cameraZoom();
            
            Vec2f result;
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(distance[i]) < 4.0f / zoom)
                    result[i] = delta[i] + distance[i];
                else
                    result[i] = Math::round(delta[i]);
            }
            return result;
        }
        
        Vec2f UVViewHelper::computeDistanceFromTextureGrid(const Vec3& position) const {
            const Vec2 stripe = stripeSize();
            assert(stripe.x() != 0.0 && stripe.y() != 0);
            
            const FloatType x = Math::remainder(position.x(), stripe.x());
            const FloatType y = Math::remainder(position.y(), stripe.y());
            
            return Vec2f(x, y);
        }
        
        void UVViewHelper::computeOriginHandleVertices(Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const {
            assert(valid());
            
            const Mat4x4 toTex   = m_face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            const Mat4x4 toWorld = m_face->fromTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            computeLineVertices(m_origin, x1, x2, y1, y2, toTex, toWorld);
        }

        void UVViewHelper::computeScaleHandleVertices(const Vec2& pos, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const {
            assert(valid());
            
            const Mat4x4 toTex   = m_face->toTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            const Mat4x4 toWorld = m_face->fromTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            computeLineVertices(pos, x1, x2, y1, y2, toTex, toWorld);
        }
        
        void UVViewHelper::computeLineVertices(const Vec2& pos, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2, const Mat4x4& toTex, const Mat4x4& toWorld) const {
            const Vec3::List viewportVertices = toTex * m_camera.viewportVertices();
            const BBox3 viewportBounds(viewportVertices);
            const Vec3& min = viewportBounds.min;
            const Vec3& max = viewportBounds.max;
            
            x1 = toWorld * Vec3(pos.x(), min.y(), 0.0);
            x2 = toWorld * Vec3(pos.x(), max.y(), 0.0);
            y1 = toWorld * Vec3(min.x(), pos.y(), 0.0);
            y2 = toWorld * Vec3(max.x(), pos.y(), 0.0);
        }

        void UVViewHelper::resetOrigin() {
            assert(valid());
            
            const Mat4x4 toTex = m_face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            const BBox3 bounds(toTex * vertexPositions(m_face->vertices()));
            
            const Vec3 vertices[] = {
                bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Min, BBox3::Corner_Min),
                bounds.vertex(BBox3::Corner_Min, BBox3::Corner_Max, BBox3::Corner_Min),
                bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Max, BBox3::Corner_Min),
                bounds.vertex(BBox3::Corner_Max, BBox3::Corner_Min, BBox3::Corner_Min)
            };
            
            const Mat4x4 fromTex = m_face->fromTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            const Mat4x4 toCam(m_camera.viewMatrix());
            
            for (size_t i = 0; i < 4; ++i) {
                const Vec3 vertex = toCam * fromTex * vertices[i];
                if (vertex.x() < 0.0 && vertex.y() < 0.0) {
                    m_origin = Vec2f(vertices[i]);
                    break;
                }
            }
        }
        
        void UVViewHelper::resetCamera() {
            assert(valid());

            const Vec3& normal = m_face->boundary().normal;
            Vec3 right;
            
            if (Math::lt(Math::abs(Vec3::PosZ.dot(normal)), 1.0))
                right = crossed(Vec3::PosZ, normal).normalized();
            else
                right = Vec3::PosX;
            const Vec3 up = crossed(normal, right).normalized();
            
            m_camera.setNearPlane(-1.0);
            m_camera.setFarPlane(1.0);
            m_camera.setDirection(-normal, up);

            const Vec3 position = m_face->boundsCenter();
            m_camera.moveTo(position);
            resetZoom();
        }
        
        void UVViewHelper::resetZoom() {
            assert(valid());
            
            const BBox3 bounds = computeFaceBoundsInCameraCoords();
            const Vec3f size(bounds.size());
            const float w = static_cast<float>(m_camera.unzoomedViewport().width - 80);
            const float h = static_cast<float>(m_camera.unzoomedViewport().height - 80);
            
            float zoom = 3.0f;
            zoom = Math::min(zoom, w / size.x());
            zoom = Math::min(zoom, h / size.y());
            if (zoom > 0.0f) {
                m_camera.setZoom(zoom);
                m_zoomValid = true;
            }
        }

        BBox3 UVViewHelper::computeFaceBoundsInCameraCoords() const {
            assert(valid());
            
            const Mat4x4 transform = coordinateSystemMatrix(m_camera.right(), m_camera.up(), -m_camera.direction(), m_camera.position());

            BBox3 result;
            const Model::BrushVertexList& vertices = m_face->vertices();
            result.min = result.max = transform * vertices[0]->position;
            for (size_t i = 1; i < vertices.size(); ++i)
                result.mergeWith(transform * vertices[i]->position);
            return result;
        }
    }
}
