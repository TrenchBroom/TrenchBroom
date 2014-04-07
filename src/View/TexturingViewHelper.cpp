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
#include "Model/TexCoordSystemHelper.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/OrthographicCamera.h"
#include "View/TexturingView.h"

namespace TrenchBroom {
    namespace View {
        TexturingViewHelper::TexturingViewHelper() :
        m_face(NULL),
        m_cameraZoom(1.0f),
        m_subDivisions(1, 1) {}
        
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
        
        Vec3 TexturingViewHelper::computeTexPoint(const Ray3& ray) const {
            assert(valid());
            
            const Plane3& boundary = m_face->boundary();
            const FloatType facePointDist = boundary.intersectWithRay(ray);
            const Vec3 facePoint = ray.pointAtDistance(facePointDist);
            return transformToTex(facePoint);
        }
        
        Vec3 TexturingViewHelper::transformToTex(const Vec3& worldPoint, const bool withOffset) const {
            assert(valid());
            
            Model::TexCoordSystemHelper helper(m_face);
            helper.setScale();
            helper.setTranslate(withOffset);
            helper.setProject();
            return helper.worldToTex(worldPoint);
        }
        
        Vec3::List TexturingViewHelper::transformToTex(const Vec3::List& worldPoints, const bool withOffset) const {
            assert(valid());
            
            Model::TexCoordSystemHelper helper(m_face);
            helper.setScale();
            helper.setTranslate(withOffset);
            helper.setProject();
            return helper.worldToTex(worldPoints);
        }
        
        Vec2f computeDistance(const Vec3& position, const Assets::Texture* texture, const Vec2i& subDivisions) {
            const FloatType width  = static_cast<FloatType>(texture->width() / subDivisions.x());
            const FloatType height = static_cast<FloatType>(texture->height() / subDivisions.y());
            const FloatType x = Math::remainder(position.x(), width);
            const FloatType y = Math::remainder(position.y(), height);
            
            return Vec2f(x, y);
        }
        
        Vec2f combineDistances(const Vec2f& r1, const Vec2f& r2) {
            if (r1.squaredLength() < r2.squaredLength())
                return r1;
            return r2;
        }
        
        Vec2f snap(const Vec2f& delta, const Vec2f& distance, const float cameraZoom) {
            if (distance.length() < 4.0f / cameraZoom)
                return delta - distance;
            return delta.rounded();
        }
        
        Vec2f TexturingViewHelper::snapOffset(const Vec2f& delta) const {
            assert(valid());
            
            if (delta.null())
                return delta;
            
            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return delta;
            
            Model::TexCoordSystemHelper helper(m_face);
            helper.setOverrideTranslate(m_face->offset() - delta); // I don't know why this has to be subracted, but it works :-(
            helper.setScale();
            helper.setProject();
            
            const Model::BrushVertexList& vertices = m_face->vertices();
            Vec2f distance = computeDistance(helper.worldToTex(vertices[0]->position), texture, m_subDivisions);
            
            for (size_t i = 1; i < vertices.size(); ++i)
                distance = combineDistances(distance, computeDistance(helper.worldToTex(vertices[i]->position), texture, m_subDivisions));
            
            return snap(delta, -distance, m_cameraZoom);
        }
        
        Vec2f computeDistance(const Vec3& point, const Vec2f& newHandlePosition) {
            return Vec2f(newHandlePosition.x() - point.x(),
                         newHandlePosition.y() - point.y());
        }
        
        Vec2f TexturingViewHelper::snapScaleOrigin(const Vec2f& deltaInFaceCoords) const {
            assert(valid());
            
            if (deltaInFaceCoords.null())
                return deltaInFaceCoords;

            Model::TexCoordSystemHelper faceCoordSystem(m_face);
            faceCoordSystem.setProject();

            Model::TexCoordSystemHelper texCoordSystem(m_face);
            texCoordSystem.setTranslate();
            texCoordSystem.setScale();
            texCoordSystem.setProject();

            const Vec2f newOriginInFaceCoords = m_scaleOrigin + deltaInFaceCoords;
            const Vec2f newOriginInTexCoords = faceCoordSystem.texToTex(newOriginInFaceCoords, texCoordSystem);
            
            // now snap to the vertices
            const Model::BrushVertexList& vertices = m_face->vertices();
            Vec2f distanceInTexCoords = computeDistance(texCoordSystem.worldToTex(vertices[0]->position), newOriginInTexCoords);
            
            for (size_t i = 1; i < vertices.size(); ++i)
                distanceInTexCoords = combineDistances(distanceInTexCoords, computeDistance(texCoordSystem.worldToTex(vertices[i]->position), newOriginInTexCoords));
            
            // and to the texture grid
            const Assets::Texture* texture = m_face->texture();
            if (texture != NULL)
                distanceInTexCoords = combineDistances(distanceInTexCoords, computeDistance(Vec3(newOriginInTexCoords), texture, m_subDivisions));
            
            // now we have a distance in the scaled and translated texture coordinate system
            // so we transform the new position plus distance back to the unscaled and untranslated texture coordinate system
            // and take the actual distance
            const Vec2f distanceInFaceCoords = texCoordSystem.texToTex(newOriginInTexCoords + distanceInTexCoords, faceCoordSystem) - newOriginInFaceCoords;
            
            return snap(deltaInFaceCoords, distanceInFaceCoords, m_cameraZoom);
        }
        
        Vec2f TexturingViewHelper::snapToVertices(const Vec2f& pointInFaceCoords) const {
            return snapToPoints(pointInFaceCoords, Model::vertexPositions(m_face->vertices()));
        }

        Vec2f TexturingViewHelper::snapToPoints(const Vec2f& pointInFaceCoords, const Vec3::List& points) const {
            assert(valid());
            
            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(m_face);
            Vec2f distanceInFaceCoords = computeDistance(faceCoordSystem.worldToTex(points[0]), pointInFaceCoords);
            
            for (size_t i = 1; i < points.size(); ++i)
                distanceInFaceCoords = combineDistances(distanceInFaceCoords, computeDistance(faceCoordSystem.worldToTex(points[i]), pointInFaceCoords));
            
            if (distanceInFaceCoords.length() > 4.0f / m_cameraZoom)
                distanceInFaceCoords = Vec2f::Null;
            return pointInFaceCoords - distanceInFaceCoords;
        }

        float TexturingViewHelper::measureRotationAngle(const Vec2f& pointInFaceCoords) const {
            const Vec3 direction((pointInFaceCoords - m_rotationCenter).normalized());
            const FloatType angleInRadians = angleBetween(direction, Vec3::PosX, Vec3::PosZ);
            return static_cast<float>(Math::degrees(Math::Constants<FloatType>::TwoPi - angleInRadians));
        }
        
        void TexturingViewHelper::computeScaleOriginHandles(Line3& xHandle, Line3& yHandle) const {
            assert(valid());

            Model::TexCoordSystemHelper helper(m_face);
            // helper.setTranslate();
            // helper.setScale();
            helper.setProject();

            const Vec3 scaleOriginInFaceCoords(m_scaleOrigin.x(), m_scaleOrigin.y(), 0.0);
            xHandle.point = yHandle.point = helper.texToWorld(scaleOriginInFaceCoords);
            
            const Vec3 xHandlePoint2 = helper.texToWorld(scaleOriginInFaceCoords + Vec3::PosY);
            const Vec3 yHandlePoint2 = helper.texToWorld(scaleOriginInFaceCoords + Vec3::PosX);
            
            xHandle.direction = (xHandlePoint2 - xHandle.point).normalized();
            yHandle.direction = (yHandlePoint2 - yHandle.point).normalized();
        }
        
        void TexturingViewHelper::computeScaleOriginHandleVertices(const Renderer::OrthographicCamera& camera, Vec3& x1, Vec3& x2, Vec3& y1, Vec3& y2) const {
            assert(valid());
            
            Model::TexCoordSystemHelper helper(m_face);
            helper.setProject();

            const Vec3::List viewportVertices = helper.worldToTex(camera.viewportVertices());
            const BBox3 viewportBounds(viewportVertices);
            const Vec3& min = viewportBounds.min;
            const Vec3& max = viewportBounds.max;
            
            x1 = Vec3(m_scaleOrigin.x(), min.y(), 0.0);
            x2 = Vec3(m_scaleOrigin.x(), max.y(), 0.0);
            
            y1 = Vec3(min.x(), m_scaleOrigin.y(), 0.0);
            y2 = Vec3(max.x(), m_scaleOrigin.y(), 0.0);
            
            x1 = helper.texToWorld(x1);
            x2 = helper.texToWorld(x2);
            y1 = helper.texToWorld(y1);
            y2 = helper.texToWorld(y2);
        }
        
        void TexturingViewHelper::computeHLineVertices(const Renderer::OrthographicCamera& camera, const FloatType y, Vec3& v1, Vec3& v2) const {
            Model::TexCoordSystemHelper helper(m_face);
            helper.setTranslate();
            helper.setScale();
            helper.setProject();

            const Vec3::List viewportVertices = helper.worldToTex(camera.viewportVertices());
            const BBox3 viewportBounds(viewportVertices);
            const Vec3& min = viewportBounds.min;
            const Vec3& max = viewportBounds.max;
            
            v1 = Vec3(min.x(), y, 0.0);
            v2 = Vec3(max.x(), y, 0.0);

            v1 = helper.texToWorld(v1);
            v2 = helper.texToWorld(v2);
        }
        
        void TexturingViewHelper::computeVLineVertices(const Renderer::OrthographicCamera& camera, const FloatType x, Vec3& v1, Vec3& v2) const {
            Model::TexCoordSystemHelper helper(m_face);
            helper.setTranslate();
            helper.setScale();
            helper.setProject();
            
            const Vec3::List viewportVertices = helper.worldToTex(camera.viewportVertices());
            const BBox3 viewportBounds(viewportVertices);
            const Vec3& min = viewportBounds.min;
            const Vec3& max = viewportBounds.max;
            
            v1 = Vec3(x, min.y(), 0.0);
            v2 = Vec3(x, max.y(), 0.0);
            
            v1 = helper.texToWorld(v1);
            v2 = helper.texToWorld(v2);
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
            if (face != m_face) {
                m_face = face;
                if (m_face != NULL) {
                    validate();
                    resetScaleOrigin();
                    resetRotationCenter();
                }
            }
        }
        
        void TexturingViewHelper::faceDidChange() {
            if (m_face != NULL) {
                validate();
            }
        }
        
        void TexturingViewHelper::setCameraZoom(const float cameraZoom) {
            m_cameraZoom = cameraZoom;
        }
        
        const Vec2i& TexturingViewHelper::subDivisions() const {
            return m_subDivisions;
        }
        
        void TexturingViewHelper::setSubDivisions(const Vec2i& subDivisions) {
            m_subDivisions = subDivisions;
        }
        
        const Vec2f TexturingViewHelper::scaleOriginInFaceCoords() const {
            return m_scaleOrigin;
        }
        
        const Vec2f TexturingViewHelper::scaleOriginInTexCoords() const {
            Model::TexCoordSystemHelper faceCoordSystem(m_face);
            faceCoordSystem.setProject();
            
            Model::TexCoordSystemHelper texCoordSystem(m_face);
            texCoordSystem.setTranslate();
            texCoordSystem.setScale();
            texCoordSystem.setProject();
            
            return faceCoordSystem.texToTex(m_scaleOrigin, texCoordSystem);
        }
        
        void TexturingViewHelper::setScaleOrigin(const Vec2f& scaleOriginInFaceCoords) {
            m_scaleOrigin = scaleOriginInFaceCoords;
        }
        
        const Vec2f TexturingViewHelper::rotationCenterInFaceCoords() const {
            return m_rotationCenter;
        }
        
        const Vec2f TexturingViewHelper::angleHandleInFaceCoords(const float distance) const {
            return m_rotationCenter + distance * Vec2f::PosX;
        }

        void TexturingViewHelper::setRotationCenter(const Vec2f& rotationCenterInFaceCoords) {
            m_rotationCenter = rotationCenterInFaceCoords;
        }
        
        void TexturingViewHelper::validate() {
            assert(m_face != NULL);
            
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
        
        void TexturingViewHelper::resetScaleOrigin() {
            assert(m_face != NULL);
            const Model::BrushVertexList& vertices = m_face->vertices();
            const size_t vertexCount = vertices.size();
            
            Vec3::List positions(vertexCount);
            for (size_t i = 0; i < vertexCount; ++i)
                positions[i] = vertices[i]->position;
            
            Model::TexCoordSystemHelper helper(m_face);
            helper.setProject();

            const BBox3 bounds(helper.worldToTex(positions));
            m_scaleOrigin = Vec2f(bounds.min);
        }
        
        void TexturingViewHelper::resetRotationCenter() {
            assert(m_face != NULL);
            const Vec3 center = m_face->center();
            
            Model::TexCoordSystemHelper helper(m_face);
            helper.setProject();
            
            const Vec3 centerInFaceCoords = helper.worldToTex(center);
            m_rotationCenter = Vec2f(centerInFaceCoords);
        }
    }
}
