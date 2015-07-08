/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Face.h"

#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Texture.h"

namespace TrenchBroom {
    namespace Model {
        inline void FindFacePoints::operator()(const Face& face, FacePoints& points) const {
            size_t numPoints = selectInitialPoints(face, points);
            findPoints(face.boundary(), points, numPoints);
        }

        const FindFacePoints& FindFacePoints::instance(bool forceIntegerCoordinates) {
            if (forceIntegerCoordinates)
                return FindIntegerFacePoints::Instance;
            return FindFloatFacePoints::Instance;
        }

        inline size_t FindIntegerFacePoints::selectInitialPoints(const Face& face, FacePoints& points) const {
            size_t numPoints = 0;
            bool integer[3];
            for (size_t i = 0; i < 3; i++) {
                integer[i] = points[i].isInteger();
                if  (integer[i])
                    numPoints++;
            }

            if (numPoints < 3)
                return 0;
            return 3;
            
            /*
            
            if (numPoints == 0)
                return 1;
            if (numPoints == 1) {
                if (integer[1])
                    std::swap(points[0], points[1]);
                else if (integer[2])
                    std::swap(points[0], points[2]);
                return 1;
            }
            if (numPoints == 2) {
                if (!integer[0])
                    std::swap(points[0], points[2]);
                else if (!integer[1])
                    std::swap(points[1], points[2]);
                return 2;
            }
            return 3;
             */
        }
        
        static float CheckPlaneError(const FacePoints& testPoints, const FacePoints& referencePoints) {
            Planef testPlane;
            if (!testPlane.setPoints(testPoints[0], testPoints[1], testPoints[2])) {
                return std::numeric_limits<float>::max();
            }
            
            float error = 0;
            for (size_t i=0; i<3; i++)
                error = std::max(error, std::abs(testPlane.pointDistance(referencePoints[i])));
            return error;
        }
        
        inline void FindIntegerFacePoints::findPoints(const Planef& plane, FacePoints& points, size_t numPoints) const {
            // Sometimes simply rounding each plane point seems to
            // give better results in practice than FindIntegerPlanePoints,
            // see https://github.com/kduske/TrenchBroom/issues/1033

            // These are some of the face verts
            const FacePoints refPoints = {points[0], points[1], points[2]};
            
            if (refPoints[0].isInteger()
                && refPoints[1].isInteger()
                && refPoints[2].isInteger())
                return;
            
            // Run the search algorithm
            FacePoints searchAlgoPoints = {refPoints[0], refPoints[1], refPoints[2]};
            m_findPoints(plane, searchAlgoPoints, numPoints);
            const float searchAlgoError = CheckPlaneError(searchAlgoPoints, refPoints);
            
            // Do the rounding approach
            const FacePoints roundingAlgoPoints = {refPoints[0].rounded(), refPoints[1].rounded(), refPoints[2].rounded()};
            const float roundingAlgoError = CheckPlaneError(roundingAlgoPoints, refPoints);
            
            // Return whichever has less error
            for (size_t i=0; i<3; i++) {
                if (searchAlgoError < roundingAlgoError) {
                    points[i] = searchAlgoPoints[i];
                } else {
                    points[i] = roundingAlgoPoints[i];
                }
            }
        }

        const FindIntegerFacePoints FindIntegerFacePoints::Instance = FindIntegerFacePoints();

        inline size_t FindFloatFacePoints::selectInitialPoints(const Face& face, FacePoints& points) const {
            face.getPoints(points[0], points[1], points[2]);
            return 3;
        }
        
        inline void FindFloatFacePoints::findPoints(const Planef& plane, FacePoints& points, size_t numPoints) const {
            m_findPoints(plane, points, numPoints);
        }
        
        const FindFloatFacePoints FindFloatFacePoints::Instance = FindFloatFacePoints();

        const Vec3f Face::BaseAxes[18] = {
            Vec3f::PosZ, Vec3f::PosX, Vec3f::NegY,
            Vec3f::NegZ, Vec3f::PosX, Vec3f::NegY,
            Vec3f::PosX, Vec3f::PosY, Vec3f::NegZ,
            Vec3f::NegX, Vec3f::PosY, Vec3f::NegZ,
            Vec3f::PosY, Vec3f::PosX, Vec3f::NegZ,
            Vec3f::NegY, Vec3f::PosX, Vec3f::NegZ
        };
        
        void Face::init() {
            static unsigned int currentId = 1;
            m_faceId = currentId++;
            for (size_t i = 0; i < 3; i++)
                m_points[i] = Vec3f::Null;
            m_xOffset = 0.0f;
            m_yOffset = 0.0f;
            m_rotation = 0.0f;
            m_xScale = 1.0f;
            m_yScale = 1.0f;
            m_brush = NULL;
            m_texture = NULL;
            m_filePosition = 0;
            m_selected = false;
            m_texAxesValid = false;
            m_vertexCacheValid = false;
            m_contentType = CTDefault;
        }
        
        void Face::texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, unsigned int& planeNormIndex, unsigned int& faceNormIndex) const {
            unsigned int bestIndex = 0;
            float bestDot = 0.0f;
            for (unsigned int i = 0; i < 6; i++) {
                const float dot = faceNormal.dot(BaseAxes[i * 3]);
                if (dot > bestDot) { // no need to use -altaxis for qbsp
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            
            xAxis = BaseAxes[bestIndex * 3 + 1];
            yAxis = BaseAxes[bestIndex * 3 + 2];
            planeNormIndex = (bestIndex / 2) * 6;
            faceNormIndex = bestIndex * 3;
        }
        
        void Face::validateTexAxes(const Vec3f& faceNormal) const {
            texAxesAndIndices(faceNormal, m_texAxisX, m_texAxisY, m_texPlanefNormIndex, m_texFaceNormIndex);
            rotateTexAxes(m_texAxisX, m_texAxisY, Math<float>::radians(m_rotation), m_texPlanefNormIndex);
            m_scaledTexAxisX = m_texAxisX / (m_xScale == 0.0f ? 1.0f : m_xScale);
            m_scaledTexAxisY = m_texAxisY / (m_yScale == 0.0f ? 1.0f : m_yScale);
            
            m_texAxesValid = true;
        }
        
        void Face::projectOntoTexturePlane(Vec3f& xAxis, Vec3f& yAxis) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);

            Planef plane(m_boundary.normal, 0.0f);
            if (BaseAxes[m_texPlanefNormIndex].x() != 0.0f) {
                xAxis[0] = plane.x(xAxis.y(), xAxis.z());
                yAxis[0] = plane.x(yAxis.y(), yAxis.z());
            } else if (BaseAxes[m_texPlanefNormIndex].y() != 0.0f) {
                xAxis[1] = plane.y(xAxis.x(), xAxis.z());
                yAxis[1] = plane.y(yAxis.x(), yAxis.z());
            } else {
                xAxis[2] = plane.z(xAxis.x(), xAxis.y());
                yAxis[2] = plane.z(yAxis.x(), yAxis.y());
            }
        }

        void Face::validateVertexCache() const {
            assert(m_side != NULL);
            
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            unsigned int width = m_texture != NULL ? m_texture->width() : 1;
            unsigned int height = m_texture != NULL ? m_texture->height() : 1;
            
            size_t vertexCount = m_side->vertices.size();
            m_vertexCache.resize(3 * (vertexCount - 2));
            
            Vec2f texCoords;
            size_t j = 0;
            for (size_t i = 1; i < vertexCount - 1; i++) {
                m_vertexCache[j++] = Renderer::FaceVertex(m_side->vertices[0]->position,
                                                          m_boundary.normal,
                                                          Vec2f((m_side->vertices[0]->position.dot(m_scaledTexAxisX) + m_xOffset) / width,
                                                                (m_side->vertices[0]->position.dot(m_scaledTexAxisY) + m_yOffset) / height)
                                                          );
                m_vertexCache[j++] = Renderer::FaceVertex(m_side->vertices[i]->position,
                                                          m_boundary.normal,
                                                          Vec2f((m_side->vertices[i]->position.dot(m_scaledTexAxisX) + m_xOffset) / width,
                                                                (m_side->vertices[i]->position.dot(m_scaledTexAxisY) + m_yOffset) / height)
                                                          );
                m_vertexCache[j++] = Renderer::FaceVertex(m_side->vertices[i+1]->position,
                                                          m_boundary.normal,
                                                          Vec2f((m_side->vertices[i+1]->position.dot(m_scaledTexAxisX) + m_xOffset) / width,
                                                                (m_side->vertices[i+1]->position.dot(m_scaledTexAxisY) + m_yOffset) / height)
                                                          );
            }
            
            m_vertexCacheValid = true;
        }
        
        void Face::compensateTransformation(const Mat4f& transformation) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            // calculate the current texture coordinates of the face's center
            const Vec3f curCenter = centerOfVertices(m_side->vertices);
            const Vec2f curCenterTexCoords(curCenter.dot(m_scaledTexAxisX) + m_xOffset,
                                           curCenter.dot(m_scaledTexAxisY) + m_yOffset);
            
            // invert the scale of the current texture axes
            Vec3f newTexAxisX = m_texAxisX * m_xScale;
            Vec3f newTexAxisY = m_texAxisY * m_yScale;
            
            // project the inversely scaled texture axes onto the boundary plane
            projectOntoTexturePlane(newTexAxisX, newTexAxisY);
            
            // apply the transformation
            newTexAxisX = transformation * newTexAxisX;
            newTexAxisY = transformation * newTexAxisY;
            
            Vec3f newFaceNorm = transformation * m_boundary.normal;
            const Vec3f offset = transformation * Vec3f::Null;
            const Vec3f newCenter = transformation * curCenter;
            
            // correct the directional vectors by the translational part of the transformation
            newTexAxisX -= offset;
            newTexAxisY -= offset;
            newFaceNorm -= offset;
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (newFaceNorm.equals(m_boundary.normal, 0.01f))
                newFaceNorm = m_boundary.normal;
            
            // obtain the new texture plane norm and the new base texture axes
            Vec3f newBaseAxisX, newBaseAxisY;
            unsigned int newPlanefNormIndex, newFaceNormIndex;
            texAxesAndIndices(newFaceNorm, newBaseAxisX, newBaseAxisY, newPlanefNormIndex, newFaceNormIndex);
            
            /*
             float tpnDot = dotV3f(texPlanefNorm, newTexPlanefNorm);
             if (tpnDot == 1 || tpnDot == -1) {
             Vec3f transformedTexPlanefNorm;
             transformM4fV3f(transformation, texPlanefNorm, &transformedTexPlanefNorm);
             subV3f(&transformedTexPlanefNorm, &offset, &transformedTexPlanefNorm);
             
             if (dotV3f(texPlanefNorm, &transformedTexPlanefNorm) == 0) {
             crossV3f(texPlanefNorm, &transformedTexPlanefNorm, &temp);
             const Vec3f* rotAxis = closestAxisV3f(&temp);
             
             float angle = Math<float>::Pi_2;
             if (tpnDot == 1)
             angle *= -1;
             
             TQuaternion rot;
             setAngleAndAxisQ(&rot, angle, rotAxis);
             
             rotateQ(&rot, &newTexAxisX, &newTexAxisX);
             rotateQ(&rot, &newTexAxisY, &newTexAxisY);
             }
             }
             */
            
            // project the transformed texture axes onto the new texture plane
            if (BaseAxes[newPlanefNormIndex].x() != 0.0f) {
                newTexAxisX[0] = 0.0f;
                newTexAxisY[0] = 0.0f;
            } else if (BaseAxes[newPlanefNormIndex].y() != 0.0f) {
                newTexAxisX[1] = 0.0f;
                newTexAxisY[1] = 0.0f;
            } else {
                newTexAxisX[2] = 0.0f;
                newTexAxisY[2] = 0.0f;
            }
            
            // the new scaling factors are the lengths of the transformed texture axes
            m_xScale = newTexAxisX.length();
            m_yScale = newTexAxisY.length();
            
            // normalize the transformed texture axes
            newTexAxisX /= m_xScale;
            newTexAxisY /= m_yScale;
            
            // WARNING: the texture plane norm is not the rotation axis of the texture (it's always the absolute axis)
            
            // determine the rotation angle from the dot product of the new base axes and the transformed texture axes
            float radX = acosf(newBaseAxisX.dot(newTexAxisX));
            if (crossed(newBaseAxisX, newTexAxisX).dot(BaseAxes[newPlanefNormIndex]) < 0.0f)
                radX *= -1.0f;
            
            /*
            float radY = acosf(newBaseAxisY.dot(newTexAxisY));
            if (crossed(newBaseAxisY, newTexAxisY).dot(BaseAxes[newPlanefNormIndex]) < 0.0f)
                radY *= -1.0f;
             */
            
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            float rad = radX;
            if (newPlanefNormIndex == 12)
                rad *= -1.0f;

            m_rotation = Math<float>::degrees(rad);
            
            // apply the rotation to the new base axes
            rotateTexAxes(newBaseAxisX, newBaseAxisY, rad, newPlanefNormIndex);
            
            // the sign of the scaling factors depends on the angle between the new base axis and the new texture axis
            if (newBaseAxisX.dot(newTexAxisX) < 0.0f)
                m_xScale *= -1.0f;
            if (newBaseAxisY.dot(newTexAxisY) < 0.0f)
                m_yScale *= -1.0f;
            
            // correct rounding errors
            m_xScale = Math<float>::correct(m_xScale);
            m_yScale = Math<float>::correct(m_yScale);
            m_rotation = Math<float>::correct(m_rotation);
            
            validateTexAxes(newFaceNorm);
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const Vec2f newCenterTexCoords(newCenter.dot(m_scaledTexAxisX),
                                           newCenter.dot(m_scaledTexAxisY));
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            m_xOffset = curCenterTexCoords.x() - newCenterTexCoords.x();
            m_yOffset = curCenterTexCoords.y() - newCenterTexCoords.y();
            
            if (m_texture != NULL) {
                m_xOffset -= static_cast<int>(Math<float>::round(m_xOffset / static_cast<float>(m_texture->width()))) * static_cast<int>(m_texture->width());
                m_yOffset -= static_cast<int>(Math<float>::round(m_yOffset / static_cast<float>(m_texture->height()))) * static_cast<int>(m_texture->height());
            }
            
            // correct rounding errors
            m_xOffset = Math<float>::correct(m_xOffset);
            m_yOffset = Math<float>::correct(m_yOffset);
        }
        
        void Face::updateContentType() {
            if (!m_textureName.empty()) {
                if (m_textureName[0] == '*')
                    m_contentType = CTLiquid;
                else if (Utility::containsString(m_textureName, "clip", false))
                    m_contentType = CTClip;
                else if (Utility::containsString(m_textureName, "skip", false))
                    m_contentType = CTSkip;
                else if (Utility::containsString(m_textureName, "hint", false))
                    m_contentType = CTHint;
                else if (Utility::containsString(m_textureName, "trigger", false))
                    m_contentType = CTTrigger;
                else
                    m_contentType = CTDefault;
            } else {
                m_contentType = CTDefault;
            }
        }

        Face::Face(const BBoxf& worldBounds, bool forceIntegerFacePoints, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const String& textureName) : m_worldBounds(worldBounds), m_textureName(textureName) {
            init();
            m_worldBounds = worldBounds;
            m_forceIntegerFacePoints = forceIntegerFacePoints;
            m_points[0] = point1;
            m_points[1] = point2;
            m_points[2] = point3;
            correctFacePoints();
            m_boundary.setPoints(m_points[0], m_points[1], m_points[2]);
            updatePointsFromBoundary();
            setTextureName(textureName);
        }
        
        Face::Face(const BBoxf& worldBounds, bool forceIntegerFacePoints, const Face& faceTemplate) : m_worldBounds(worldBounds) {
            init();
            m_worldBounds = worldBounds;
            m_forceIntegerFacePoints = forceIntegerFacePoints;
            restore(faceTemplate);
        }
        
        Face::Face(const Face& face) :
        m_side(NULL),
        m_faceId(face.faceId()),
        m_boundary(face.boundary()),
        m_worldBounds(face.worldBounds()),
        m_forceIntegerFacePoints(face.forceIntegerFacePoints()),
        m_textureName(face.textureName()),
        m_texture(face.texture()),
        m_xOffset(face.xOffset()),
        m_yOffset(face.yOffset()),
        m_rotation(face.rotation()),
        m_xScale(face.xScale()),
        m_yScale(face.yScale()),
        m_texAxesValid(false),
        m_vertexCacheValid(false),
        m_filePosition(face.filePosition()),
        m_selected(false),
        m_contentType(face.contentType()) {
            face.getPoints(m_points[0], m_points[1], m_points[2]);
            updatePointsFromBoundary();
        }
        
		Face::~Face() {
			m_texPlanefNormIndex = 0;
			m_texFaceNormIndex = 0;
			m_texAxisX = Vec3f::NaN;
			m_texAxisY = Vec3f::NaN;
			m_scaledTexAxisX = Vec3f::NaN;
			m_scaledTexAxisY = Vec3f::NaN;
            
			m_faceId = 0;
            setBrush(NULL);
			
			for (unsigned int i = 0; i < 3; i++)
				m_points[i] = Vec3f::NaN;
			m_boundary.normal = Vec3f::NaN;
			m_boundary.distance = -1.0f;

            setTexture(NULL);
			m_xOffset = 0;
			m_yOffset = 0;
			m_rotation = 0.0f;
			m_xScale = 0.0f;
			m_yScale = 0.0f;
			m_side = NULL;
			m_filePosition = 0;
			m_selected = false;
			m_vertexCacheValid = false;
			m_texAxesValid = false;
		}
        
        void Face::restore(const Face& faceTemplate) {
            faceTemplate.getPoints(m_points[0], m_points[1], m_points[2]);
            m_boundary = faceTemplate.boundary();
            m_xOffset = faceTemplate.xOffset();
            m_yOffset = faceTemplate.yOffset();
            m_rotation = faceTemplate.rotation();
            m_xScale = faceTemplate.xScale();
            m_yScale = faceTemplate.yScale();
            setTexture(faceTemplate.texture());
            m_texAxesValid = false;
            m_vertexCacheValid = false;
			m_selected = faceTemplate.selected();
            m_contentType = faceTemplate.contentType();
        }
        
        void Face::setBrush(Brush* brush) {
            if (brush == m_brush)
                return;
            
            if (m_brush != NULL && m_selected)
                m_brush->decSelectedFaceCount();
            m_brush = brush;
            if (m_brush != NULL && m_selected)
                m_brush->incSelectedFaceCount();
        }
        
        void Face::updatePointsFromVertices() {
            Vec3f v1, v2;
            
            const size_t vertexCount = m_side->vertices.size();
            assert(vertexCount >= 3);

            float bestDot = 1.0f;
            size_t best = vertexCount;
            for (unsigned int i = 0; i < vertexCount && bestDot > 0; i++) {
                m_points[2] = m_side->vertices[pred(i, vertexCount)]->position;
                m_points[0] = m_side->vertices[i]->position;
                m_points[1] = m_side->vertices[succ(i, vertexCount)]->position;
                
                v1 = (m_points[2] - m_points[0]).normalized();
                v2 = (m_points[1] - m_points[0]).normalized();
                const float dot = std::abs(v1.dot(v2));
                if (dot < bestDot) {
                    bestDot = dot;
                    best = i;
                }
            }
            
            m_points[2] = m_side->vertices[pred(best, vertexCount)]->position;
            m_points[0] = m_side->vertices[best]->position;
            m_points[1] = m_side->vertices[succ(best, vertexCount)]->position;
            correctFacePoints();
            
            if (!m_boundary.setPoints(m_points[0], m_points[1], m_points[2])) {
                std::stringstream msg;
                msg << "Invalid face points "
                << m_points[0].asString() << "; "
                << m_points[1].asString() << "; "
                << m_points[2].asString()
                << " for face with ID " << m_faceId;
                throw GeometryException(msg);
            }
        }
        
        void Face::updatePointsFromBoundary() {
            const FindFacePoints& findPoints = FindFacePoints::instance(m_forceIntegerFacePoints);
            findPoints(*this, m_points);
            correctFacePoints();
            
            if (!m_boundary.setPoints(m_points[0], m_points[1], m_points[2])) {
                std::stringstream msg;
                msg << "Invalid face points "
                << m_points[0].asString() << "; "
                << m_points[1].asString() << "; "
                << m_points[2].asString()
                << " for face with ID " << m_faceId;
                throw GeometryException(msg);
            }
        }

        void Face::correctFacePoints() {
            for (size_t i = 0; i < 3; i++)
                m_points[i].correct();
        }
        
        void Face::setForceIntegerFacePoints(bool forceIntegerFacePoints) {
            m_forceIntegerFacePoints = forceIntegerFacePoints;
            updatePointsFromBoundary();
        }

        void Face::setTexture(Texture* texture) {
            if (texture == m_texture)
                return;
            
            if (m_texture != NULL)
                m_texture->decUsageCount();
            
            m_texture = texture;
            if (m_texture != NULL)
                m_textureName = texture->name();
            
            if (m_texture != NULL)
                m_texture->incUsageCount();
            m_vertexCacheValid = false;
            updateContentType();
        }
        
        void Face::moveTexture(const Vec3f& up, const Vec3f& right, Direction direction, float distance) {
            assert(direction != DForward && direction != DBackward);
            
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);

            Vec3f texX = m_texAxisX;
            Vec3f texY = m_texAxisY;
            projectOntoTexturePlane(texX, texY);
            texX.normalize();
            texY.normalize();

            Vec3f vAxis, hAxis;
            float* xOffset = NULL;
            float* yOffset = NULL;
            
            // we prefer to use the texture axis which is closer to the XY plane for horizontal movement
            if (Math<float>::lt(std::abs(texX.z()), std::abs(texY.z()))) {
                hAxis = texX;
                vAxis = texY;
                xOffset = &m_xOffset;
                yOffset = &m_yOffset;
            } else if (Math<float>::lt(std::abs(texY.z()), std::abs(texX.z()))) {
                hAxis = texY;
                vAxis = texX;
                xOffset = &m_yOffset;
                yOffset = &m_xOffset;
            } else {
                // both texture axes have the same absolute angle towards the XY plane, prefer the one that is closer
                // to the right view axis for horizontal movement

                if (Math<float>::gt(std::abs(right.dot(texX)), std::abs(right.dot(texY)))) {
                    // the right view axis is closer to the X texture axis
                    hAxis = texX;
                    vAxis = texY;
                    xOffset = &m_xOffset;
                    yOffset = &m_yOffset;
                } else if (Math<float>::gt(std::abs(right.dot(texY)), std::abs(right.dot(texX)))) {
                    // the right view axis is closer to the Y texture axis
                    hAxis = texY;
                    vAxis = texX;
                    xOffset = &m_yOffset;
                    yOffset = &m_xOffset;
                } else {
                    // the right axis is as close to the X texture axis as to the Y texture axis
                    // test the up axis
                    if (Math<float>::gt(std::abs(up.dot(texY)), std::abs(up.dot(texX)))) {
                        // the up view axis is closer to the Y texture axis
                        hAxis = texX;
                        vAxis = texY;
                        xOffset = &m_xOffset;
                        yOffset = &m_yOffset;
                    } else if (Math<float>::gt(std::abs(up.dot(texX)), std::abs(up.dot(texY)))) {
                        // the up view axis is closer to the X texture axis
                        hAxis = texY;
                        vAxis = texX;
                        xOffset = &m_yOffset;
                        yOffset = &m_xOffset;
                    } else {
                        // this is just bad, better to do nothing
                        return;
                    }
                }
            }

            assert(xOffset != NULL && yOffset != NULL &&
                   !hAxis.null() && !vAxis.null());
            
            switch (direction) {
                case DUp:
                    if (up.dot(vAxis) >= 0.0f)
                        *yOffset -= distance;
                    else
                        *yOffset += distance;
                    break;
                case DRight:
                    if (right.dot(hAxis) >= 0.0f)
                        *xOffset -= distance;
                    else
                        *xOffset += distance;
                    break;
                case DDown:
                    if (up.dot(vAxis) >= 0.0f)
                        *yOffset += distance;
                    else
                        *yOffset -= distance;
                    break;
                case DLeft:
                    if (right.dot(hAxis) >= 0.0f)
                        *xOffset += distance;
                    else
                        *xOffset -= distance;
                    break;
                    
                default:
                    return;
            }
            
            m_vertexCacheValid = false;
        }
        
        void Face::rotateTexture(float angle) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            if (m_texPlanefNormIndex == m_texFaceNormIndex)
                m_rotation += angle;
            else
                m_rotation -= angle;
            m_texAxesValid = false;
            m_vertexCacheValid = false;
        }
        
        void Face::setSelected(bool selected) {
            if (selected == m_selected)
                return;
            
            m_selected = selected;
            if (m_brush != NULL) {
                if (m_selected)
                    m_brush->incSelectedFaceCount();
                else
                    m_brush->decSelectedFaceCount();
            }
        }
        
        void Face::transform(const Mat4f& pointTransform, const Mat4f& vectorTransform, const bool lockTexture, const bool invertOrientation) {
            if (lockTexture)
                compensateTransformation(pointTransform);
            
            m_boundary.transform(pointTransform, vectorTransform);
            for (size_t i = 0; i < 3; i++)
                m_points[i] = pointTransform * m_points[i];
            if (invertOrientation)
                std::swap(m_points[1], m_points[2]);
            if (m_forceIntegerFacePoints)
                updatePointsFromBoundary();
            else
                correctFacePoints();

            m_texAxesValid = false;
            m_vertexCacheValid = false;
        }
    }
}
