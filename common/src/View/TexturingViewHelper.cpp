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

#include "TexturingViewHelper.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Hit.h"
#include "Assets/Texture.h"
#include "Model/BrushEdge.h"
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Model/TexCoordSystemHelper.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/VertexArray.h"
#include "Renderer/VertexSpec.h"
#include "View/TexturingView.h"

namespace TrenchBroom {
    namespace View {
        TexturingViewHelper::TexturingViewHelper(Renderer::OrthographicCamera& camera) :
        m_camera(camera),
        m_face(NULL),
        m_subDivisions(1, 1),
        m_vbo(0xFFF) {}
        
        bool TexturingViewHelper::valid() const {
            return m_face != NULL;
        }
        
        Model::BrushFace* TexturingViewHelper::face() const {
            return m_face;
        }
        
        const Assets::Texture* TexturingViewHelper::texture() const {
            if (!valid())
                return NULL;
            return m_face->texture();
        }
        
        void TexturingViewHelper::setFace(Model::BrushFace* face) {
            if (face != m_face) {
                m_face = face;
                if (m_face != NULL) {
                    resetOrigin();
                    resetCamera();
                }
            }
        }
        
        const Vec2i& TexturingViewHelper::subDivisions() const {
            return m_subDivisions;
        }

        Vec2 TexturingViewHelper::stripeSize() const {
            assert(valid());
            
            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return Vec2::Null;
            const FloatType width  = static_cast<FloatType>(texture->width())  / static_cast<FloatType>(m_subDivisions.x());
            const FloatType height = static_cast<FloatType>(texture->height()) / static_cast<FloatType>(m_subDivisions.y());
            return Vec2(width, height);
        }

        void TexturingViewHelper::setSubDivisions(const Vec2i& subDivisions) {
            m_subDivisions = subDivisions;
        }
        
        const Vec2f TexturingViewHelper::originInFaceCoords() const {
            return m_origin;
        }
        
        const Vec2f TexturingViewHelper::originInTexCoords() const {
            Model::TexCoordSystemHelper faceCoordSystem(m_face);
            faceCoordSystem.setProject();
            
            Model::TexCoordSystemHelper texCoordSystem(m_face);
            texCoordSystem.setTranslate();
            texCoordSystem.setScale();
            texCoordSystem.setProject();
            
            return faceCoordSystem.texToTex(m_origin, texCoordSystem);
        }
        
        void TexturingViewHelper::setOrigin(const Vec2f& originInFaceCoords) {
            m_origin = originInFaceCoords;
        }
        
        float TexturingViewHelper::cameraZoom() const {
            const Vec2f& zoom = m_camera.zoom();
            assert(zoom.x() == zoom.y());
            return zoom.x();
        }
        
        Vec2f TexturingViewHelper::snapDelta(const Vec2f& delta, const Vec2f& distance) const {
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
        
        Vec2f TexturingViewHelper::computeDistanceFromTextureGrid(const Vec3& position) const {
            const Vec2 stripe = stripeSize();
            assert(stripe.x() != 0.0 && stripe.y() != 0);
            
            const FloatType x = Math::remainder(position.x(), stripe.x());
            const FloatType y = Math::remainder(position.y(), stripe.y());
            
            return Vec2f(x, y);
        }
        
        void TexturingViewHelper::computeOriginHandleVertices(Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const {
            assert(valid());
            
            const Mat4x4 toTex = m_face->toTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            const Mat4x4 toWorld = m_face->fromTexCoordSystemMatrix(Vec2f::Null, Vec2f::One, true);
            computeLineVertices(m_origin, x1, x2, y1, y2, toTex, toWorld);
        }

        void TexturingViewHelper::computeScaleHandleVertices(const Vec2& pos, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const {
            assert(valid());
            
            const Mat4x4 toTex = m_face->toTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            const Mat4x4 toWorld = m_face->fromTexCoordSystemMatrix(m_face->offset(), m_face->scale(), true);
            computeLineVertices(pos, x1, x2, y1, y2, toTex, toWorld);
        }
        
        void TexturingViewHelper::computeLineVertices(const Vec2& pos, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2, const Mat4x4& toTex, const Mat4x4& toWorld) const {
            const Vec3::List viewportVertices = toTex * m_camera.viewportVertices();
            const BBox3 viewportBounds(viewportVertices);
            const Vec3& min = viewportBounds.min;
            const Vec3& max = viewportBounds.max;
            
            x1 = toWorld * Vec3(pos.x(), min.y(), 0.0);
            x2 = toWorld * Vec3(pos.x(), max.y(), 0.0);
            y1 = toWorld * Vec3(min.x(), pos.y(), 0.0);
            y2 = toWorld * Vec3(max.x(), pos.y(), 0.0);
        }

        void TexturingViewHelper::resetOrigin() {
            assert(m_face != NULL);
            const Model::BrushVertexList& vertices = m_face->vertices();
            const size_t vertexCount = vertices.size();
            
            Vec3::List positions(vertexCount);
            for (size_t i = 0; i < vertexCount; ++i)
                positions[i] = vertices[i]->position;
            
            Model::TexCoordSystemHelper helper(m_face);
            helper.setProject();

            const BBox3 bounds(helper.worldToTex(positions));
            m_origin = Vec2f(bounds.min);
        }
        
        void TexturingViewHelper::resetCamera() {
            assert(valid());
            
            const BBox3 bounds = computeFaceBoundsInCameraCoords();
            const Vec3f size(bounds.size());
            const float w = static_cast<float>(m_camera.viewport().width - 20);
            const float h = static_cast<float>(m_camera.viewport().height - 20);
            
            float zoom = 1.0f;
            if (size.x() > w)
                zoom = Math::min(zoom, w / size.x());
            if (size.y() > h)
                zoom = Math::min(zoom, h / size.y());
            m_camera.setZoom(zoom);
            
            const Vec3  position = m_face->center();
            const Vec3& normal = m_face->boundary().normal;
            Vec3 right;
            
            if (Math::lt(Math::abs(Vec3::PosZ.dot(normal)), 1.0))
                right = crossed(Vec3::PosZ, normal).normalized();
            else
                right = Vec3::PosX;
            const Vec3 up = crossed(normal, right).normalized();
            
            m_camera.moveTo(position);
            m_camera.setNearPlane(-1.0);
            m_camera.setFarPlane(1.0);
            m_camera.setDirection(-normal, up);
        }

        BBox3 TexturingViewHelper::computeFaceBoundsInCameraCoords() const {
            assert(valid());
            
            Mat4x4 transform = coordinateSystemMatrix(m_camera.right(), m_camera.up(), -m_camera.direction(), m_camera.position());
            invertMatrix(transform);

            BBox3 result;
            const Model::BrushVertexList& vertices = m_face->vertices();
            result.min = result.max = transform * vertices[0]->position;
            for (size_t i = 1; i < vertices.size(); ++i)
                result.mergeWith(transform * vertices[i]->position);
            return result;
        }
        
    }
}
