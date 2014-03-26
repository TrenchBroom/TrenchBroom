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
#include "Model/BrushFace.h"
#include "Model/BrushVertex.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/OrthographicCamera.h"
#include "View/TexturingView.h"

namespace TrenchBroom {
    namespace View {
        TexturingViewHelper::TexturingViewHelper() :
        m_face(NULL) {}
        
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
        
        const Vec3& TexturingViewHelper::origin() const {
            assert(valid());
            return m_origin;
        }
        
        const Vec3& TexturingViewHelper::xAxis() const {
            assert(valid());
            return m_xAxis;
        }
        
        const Vec3& TexturingViewHelper::yAxis() const {
            assert(valid());
            return m_yAxis;
        }
        
        const Vec3& TexturingViewHelper::zAxis() const {
            assert(valid());
            return m_zAxis;
        }
        
        BBox3 TexturingViewHelper::computeBounds() const {
            assert(valid());
            
            BBox3 result;
            const Model::BrushVertexList& vertices = m_face->vertices();
            result.min = result.max = transformToFace(vertices[0]->position);
            for (size_t i = 1; i < vertices.size(); ++i)
                result.mergeWith(transformToFace(vertices[i]->position));
            return result;
        }
        
        Vec3 TexturingViewHelper::transformToFace(const Vec3& point) const {
            assert(valid());
            return m_toFaceTransform * point;
        }
        
        Vec3 TexturingViewHelper::transformFromFace(const Vec3& point) const {
            assert(valid());
            return m_fromFaceTransform * point;
        }
        
        Vec2f TexturingViewHelper::textureCoords(const Vec3f& point) const {
            assert(valid());
            return m_face->textureCoords(Vec3(point));
        }
        
        Vec3::List TexturingViewHelper::textureSeamVertices(const Renderer::OrthographicCamera& camera) const {
            assert(valid());
            
            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return Vec3::EmptyList;
            
            // We compute the texture seam lines as follows:
            // 1. Transform the viewport vertices to the texture coordinate system.
            // 2. Project the transformed viewport onto the XY plane (of the texture coordinate system).
            // 3. Compute the seam vertices (by walking along the X and Y axes).
            // 4. Transform the seam vertices to the world coordinate system.
            // 5. Transform the seam vertices to some coordinate system on the boundary plane, but
            //    with the Z axis parallel to the texture coordinates system Z axis (skewed).
            // 6. Project the seam vertices onto the boundary plane in parallel to the Z axis of
            //    the texture coordinate system.
            // 7. Transform the seam vertices back to the world coordinate system.
            
            
            // This matrix performs steps 1 and 2:
            const Vec2f offset(m_face->xOffset(), m_face->yOffset());
            const Vec2f scale(m_face->xScale(), m_face->yScale());
            const Mat4x4 worldToTex = Mat4x4::ZerZ * m_face->toTexCoordSystemMatrix(offset, scale);
            
            // This matrix performs step 4:
            const Mat4x4 texToWorld = m_face->fromTexCoordSystemMatrix(offset, scale);
            
            // This matrix performs step 7:
            const Vec3 texZAxis = m_face->fromTexCoordSystemMatrix() * Vec3::PosZ;
            const Plane3& boundary = m_face->boundary();
            const Mat4x4 planeToWorld = planeProjectionMatrix(boundary.distance, boundary.normal, texZAxis);
            
            // This matrix performs steps 5 and 6:
            const Mat4x4 worldToPlane = Mat4x4::ZerZ * invertedMatrix(planeToWorld);
            
            // This matrix performs steps 4 through 7:
            const Mat4x4 texToWorldWithProjection = planeToWorld * worldToPlane * texToWorld;
            
            // Steps 1 and 2:
            const Vec3::List viewportVertices = worldToTex * camera.viewportVertices();
            
            // Step 3: Compute the seam vertices using the bounding box of the transformed viewport.
            const BBox3 viewportBounds(viewportVertices);
            const Vec3& min = viewportBounds.min;
            const Vec3& max = viewportBounds.max;
            
            Vec3::List seamVertices;
            
            const FloatType dx = min.x() / texture->width();
            FloatType x = (dx < 0.0 ? Math::ceil(dx) : Math::floor(dx)) * texture->width();
            while (Math::lte(x, max.x())) {
                seamVertices.push_back(Vec3(x, min.y(), 0.0));
                seamVertices.push_back(Vec3(x, max.y(), 0.0));
                x += texture->width();
            }
            
            const FloatType dy = min.y() / texture->height();
            FloatType y = (dy < 0.0 ? Math::ceil(dy) : Math::floor(dy)) * texture->height();
            while (Math::lte(y, max.y())) {
                seamVertices.push_back(Vec3(min.x(), y, 0.0));
                seamVertices.push_back(Vec3(max.x(), y, 0.0));
                y += texture->height();
            }
            
            // Steps 4 through 7:
            return texToWorldWithProjection * seamVertices;
        }
        
        Mat4x4 TexturingViewHelper::worldToTexMatrix() const {
            assert(valid());
            const Vec2f offset(m_face->xOffset(), m_face->yOffset());
            const Vec2f scale(m_face->xScale(), m_face->yScale());
            return Mat4x4::ZerZ * m_face->toTexCoordSystemMatrix(offset, scale);
        }
        
        void TexturingViewHelper::activateTexture(Renderer::ActiveShader& shader) {
            assert(valid());
            Assets::Texture* texture = m_face->texture();
            if (texture != NULL) {
                shader.set("ApplyTexture", true);
                shader.set("Color", texture->averageColor());
                texture->activate();
            } else {
                PreferenceManager& prefs = PreferenceManager::instance();
                shader.set("ApplyTexture", false);
                shader.set("Color", prefs.get(Preferences::FaceColor));
            }
        }
        
        void TexturingViewHelper::deactivateTexture() {
            assert(valid());
            Assets::Texture* texture = m_face->texture();
            if (texture != NULL)
                texture->deactivate();
        }
        
        Hits TexturingViewHelper::pick(const Ray3& pickRay) const {
            assert(valid());
            
            Hits hits;
            const FloatType distance = m_face->intersectWithRay(pickRay);
            if (!Math::isnan(distance)) {
                const Vec3 hitPoint = pickRay.pointAtDistance(distance);
                hits.addHit(Hit(TexturingView::FaceHit, distance, hitPoint, m_face));
            }
            return hits;
        }
        
        void TexturingViewHelper::setFace(Model::BrushFace* face) {
            m_face = face;
            validate();
        }
        
        void TexturingViewHelper::validate() {
            if (m_face != NULL) {
                m_origin = m_face->center();
                m_zAxis = m_face->boundary().normal;
                
                if (Math::lt(Math::abs(Vec3::PosZ.dot(m_zAxis)), 1.0))
                    m_xAxis = crossed(Vec3::PosZ, m_zAxis).normalized();
                else
                    m_xAxis = Vec3::PosX;
                m_yAxis = crossed(m_zAxis, m_xAxis).normalized();
                
                m_fromFaceTransform = coordinateSystemMatrix(m_xAxis, m_yAxis, m_zAxis, m_origin);
                bool invertible = true;
                m_toFaceTransform = invertedMatrix(m_fromFaceTransform, invertible);
                assert(invertible);
            }
        }
    }
}
