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
        
        Vec2f combineIndividualDistances(const Vec2f& r1, const Vec2f& r2);
        Vec2f combineBothDistances(const Vec2f& r1, const Vec2f& r2);
        Vec2f snapIndividual(const Vec2f& delta, const Vec2f& distance, const float cameraZoom);
        Vec2f snapBoth(const Vec2f& delta, const Vec2f& distance, const float cameraZoom);

        Vec2f combineIndividualDistances(const Vec2f& r1, const Vec2f& r2) {
            Vec2f result;
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(r1[i]) < Math::abs(r2[i]))
                    result[i] = r1[i];
                else
                    result[i] = r2[i];
            }
            return result;
        }
        
        Vec2f combineBothDistances(const Vec2f& r1, const Vec2f& r2) {
            if (r1.squaredLength() < r2.squaredLength())
                return r1;
            return r2;
        }
        
        Vec2f snapIndividual(const Vec2f& delta, const Vec2f& distance, const float cameraZoom) {
            Vec2f result;
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(distance[i]) < 4.0f / cameraZoom)
                    result[i] = delta[i] - distance[i];
                else
                    result[i] = Math::round(delta[i]);
            }
            return result;
        }
        
        Vec2f snapBoth(const Vec2f& delta, const Vec2f& distance, const float cameraZoom) {
            if (distance.length() < 4.0f / cameraZoom)
                return delta - distance;
            return delta.rounded();
        }
        
        // Computes the smallest distance between the texture grid and the given point in translated and scaled tex
        // coords.
        // only used in snapOffset!
        
        Vec2f TexturingViewHelper::snapOffset(const Vec2f& delta) const {
            assert(valid());
            
            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return delta;
            
            const Mat4x4 transform = Mat4x4::ZerZ * m_face->toTexCoordSystemMatrix(m_face->offset() - delta, m_face->scale());
            
            const Model::BrushVertexList& vertices = m_face->vertices();
            Vec2f distance = computeDistanceFromTextureGrid(transform * vertices[0]->position);
            
            for (size_t i = 1; i < vertices.size(); ++i)
                distance = combineIndividualDistances(distance, computeDistanceFromTextureGrid(transform * vertices[i]->position));
            
            return snapIndividual(delta, -distance, cameraZoom());
        }
        
        Vec2f computeDistance(const Vec3& point, const Vec2f& newHandlePosition);
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
                distanceInTexCoords = combineIndividualDistances(distanceInTexCoords, computeDistance(texCoordSystem.worldToTex(vertices[i]->position), newOriginInTexCoords));
            
            // and to the texture grid
            const Assets::Texture* texture = m_face->texture();
            if (texture != NULL)
                distanceInTexCoords = combineIndividualDistances(distanceInTexCoords, computeDistanceFromTextureGrid(Vec3(newOriginInTexCoords)));
            
            // now we have a distance in the scaled and translated texture coordinate system
            // so we transform the new position plus distance back to the unscaled and untranslated texture coordinate system
            // and take the actual distance
            const Vec2f distanceInFaceCoords = texCoordSystem.texToTex(newOriginInTexCoords + distanceInTexCoords, faceCoordSystem) - newOriginInFaceCoords;
            
            return snapIndividual(deltaInFaceCoords, distanceInFaceCoords, cameraZoom());
        }
        
        Vec2f TexturingViewHelper::snapScaleHandle(const Vec2f& scaleHandleInFaceCoords) const {
            assert(valid());
            
            const Model::BrushVertexList& vertices = m_face->vertices();
            
            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(m_face);
            Vec2f distanceInFaceCoords = computeDistance(faceCoordSystem.worldToTex(vertices[0]->position), scaleHandleInFaceCoords);
            
            for (size_t i = 1; i < vertices.size(); ++i)
                distanceInFaceCoords = combineIndividualDistances(distanceInFaceCoords, computeDistance(faceCoordSystem.worldToTex(vertices[i]->position), scaleHandleInFaceCoords));
            
            for (size_t i = 0; i < 2; ++i) {
                if (Math::abs(distanceInFaceCoords[i]) > 4.0f / cameraZoom())
                    distanceInFaceCoords[i] = 0.0f;
            }

            return scaleHandleInFaceCoords - distanceInFaceCoords;
        }

        Vec2f TexturingViewHelper::snapRotationCenter(const Vec2f& rotationCenterInFaceCoords) const {
            assert(valid());
            Vec3::List snapPoints = Model::vertexPositions(m_face->vertices());
            snapPoints.push_back(m_face->center());
            return snapToPoints(rotationCenterInFaceCoords, snapPoints);
        }

        Vec2f TexturingViewHelper::snapToPoints(const Vec2f& pointInFaceCoords, const Vec3::List& points) const {
            assert(valid());
            
            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(m_face);
            Vec2f distanceInFaceCoords = computeDistance(faceCoordSystem.worldToTex(points[0]), pointInFaceCoords);
            
            for (size_t i = 1; i < points.size(); ++i)
                distanceInFaceCoords = combineBothDistances(distanceInFaceCoords, computeDistance(faceCoordSystem.worldToTex(points[i]), pointInFaceCoords));
            
            if (distanceInFaceCoords.length() > 4.0f / cameraZoom())
                distanceInFaceCoords = Vec2f::Null;
            return pointInFaceCoords - distanceInFaceCoords;
        }

        float TexturingViewHelper::measureRotationAngle(const Vec2f& point) const {
            assert(valid());
            return m_face->measureTextureAngle(rotationCenterInFaceCoords(), point);
        }

        float TexturingViewHelper::snapRotationAngle(const float angle) const {
            assert(valid());

            const float angles[] = {
                Math::mod(angle +   0.0f, 360.0f),
                Math::mod(angle +  90.0f, 360.0f),
                Math::mod(angle + 180.0f, 360.0f),
                Math::mod(angle + 270.0f, 360.0f),
            };
            float minDelta = std::numeric_limits<float>::max();
            
            const Model::TexCoordSystemHelper faceCoordSystem = Model::TexCoordSystemHelper::faceCoordSystem(m_face);
            const Model::BrushEdgeList& edges = m_face->edges();
            Model::BrushEdgeList::const_iterator it, end;
            for (it = edges.begin(), end = edges.end(); it != end; ++it) {
                const Model::BrushEdge* edge = *it;
                
                const Vec3 startInFaceCoords = faceCoordSystem.worldToTex(edge->start->position);
                const Vec3 endInFaceCoords   = faceCoordSystem.worldToTex(edge->end->position);
                const float edgeAngle        = Math::mod(m_face->measureTextureAngle(startInFaceCoords, endInFaceCoords), 360.0f);
                
                for (size_t i = 0; i < 4; ++i) {
                    if (std::abs(angles[i] - edgeAngle) < std::abs(minDelta))
                        minDelta = angles[i] - edgeAngle;
                }
            }
            
            if (std::abs(minDelta) < 3.0f)
                return angle - minDelta;
            return angle;
        }
        
        Vec2 TexturingViewHelper::computeStripeSize() const {
            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return Vec2::Null;
            const FloatType width  = static_cast<FloatType>(texture->width())  / static_cast<FloatType>(m_subDivisions.x());
            const FloatType height = static_cast<FloatType>(texture->height()) / static_cast<FloatType>(m_subDivisions.y());
            return Vec2(width, height);
        }

        Vec2f TexturingViewHelper::computeDistanceFromTextureGrid(const Vec3& position) const {
            const Vec2 stripeSize = computeStripeSize();
            assert(stripeSize.x() != 0.0 && stripeSize.y() != 0);
            
            const FloatType x = Math::remainder(position.x(), stripeSize.x());
            const FloatType y = Math::remainder(position.y(), stripeSize.y());
            
            return Vec2f(x, y);
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
                    resetScaleOrigin();
                    resetRotationCenter();
                }
            }
        }
        
        const Vec2i& TexturingViewHelper::subDivisions() const {
            return m_subDivisions;
        }
        
        void TexturingViewHelper::setSubDivisions(const Vec2i& subDivisions) {
            m_subDivisions = subDivisions;
        }
        
        Vec2 TexturingViewHelper::stripeSize() const {
            assert(valid());
            return computeStripeSize();
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
        
        void TexturingViewHelper::renderTexture(Renderer::RenderContext& renderContext) {
            assert(valid());

            const Assets::Texture* texture = m_face->texture();
            if (texture == NULL)
                return;

            const Vec3f::List positions = getTextureQuad();
            const Vec3f normal(m_face->boundary().normal);
            
            typedef Renderer::VertexSpecs::P3NT2::Vertex Vertex;
            Vertex::List vertices(positions.size());
            
            for (size_t i = 0; i < positions.size(); ++i)
                vertices[i] = Vertex(positions[i],
                                     normal,
                                     m_face->textureCoords(positions[i]));
            
            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(GL_QUADS, vertices);
            
            Renderer::SetVboState setVboState(m_vbo);
            setVboState.mapped();
            vertexArray.prepare(m_vbo);
            setVboState.active();
            
            PreferenceManager& prefs = PreferenceManager::instance();
            
            Renderer::ActiveShader shader(renderContext.shaderManager(), Renderer::Shaders::TexturingViewShader);
            shader.set("Brightness", prefs.get(Preferences::Brightness));
            shader.set("RenderGrid", true);
            shader.set("GridSizes", Vec2f(texture->width(), texture->height()));
            shader.set("GridColor", Color(1.0f, 1.0f, 0.0f, 1.0f));
            shader.set("GridScales", m_face->scale());
            shader.set("GridMatrix", worldToTexMatrix());
            shader.set("GridDivider", Vec2f(m_subDivisions));
            shader.set("CameraZoom", m_camera.zoom().x());
            shader.set("Texture", 0);

            activateTexture(shader);
            vertexArray.render();
            deactivateTexture();
        }
        
        Vec3f::List TexturingViewHelper::getTextureQuad() const {
            Vec3f::List vertices(4);

            const Renderer::Camera::Viewport& v = m_camera.viewport();
            const Vec2f& z = m_camera.zoom();
            const float w2 = static_cast<float>(v.width)  / z.x() / 2.0f;
            const float h2 = static_cast<float>(v.height) / z.y() / 2.0f;

            const Vec3f& p = m_camera.position();
            const Vec3f& r = m_camera.right();
            const Vec3f& u = m_camera.up();
            
            vertices[0] = -w2 * r +h2 * u + p;
            vertices[1] = +w2 * r +h2 * u + p;
            vertices[2] = +w2 * r -h2 * u + p;
            vertices[3] = -w2 * r -h2 * u + p;
            
            return vertices;
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

        Mat4x4 TexturingViewHelper::worldToTexMatrix() const {
            assert(valid());
            const Vec2f offset(m_face->xOffset(), m_face->yOffset());
            const Vec2f scale(m_face->xScale(), m_face->yScale());
            return Mat4x4::ZerZ * m_face->toTexCoordSystemMatrix(offset, scale);
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

        float TexturingViewHelper::cameraZoom() const {
            const Vec2f& zoom = m_camera.zoom();
            assert(zoom.x() == zoom.y());
            return zoom.x();
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
