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
        static const Vec3f* BaseAxes[18] = {
            &Vec3f::PosZ, &Vec3f::PosX, &Vec3f::NegY,
            &Vec3f::NegZ, &Vec3f::PosX, &Vec3f::NegY,
            &Vec3f::PosX, &Vec3f::PosY, &Vec3f::NegZ,
            &Vec3f::NegX, &Vec3f::PosY, &Vec3f::NegZ,
            &Vec3f::PosY, &Vec3f::PosX, &Vec3f::NegZ,
            &Vec3f::NegY, &Vec3f::PosX, &Vec3f::NegZ
        };
        
        void Face::init() {
            static int currentId = 1;
            m_faceId = currentId++;
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
            m_coordsValid = false;
        }
        
        void Face::texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, int& planeNormIndex, int& faceNormIndex) const {
            int bestIndex = 0;
            float bestDot = -1;
            for (unsigned int i = 0; i < 6; i++) {
                float dot = faceNormal.dot(*BaseAxes[i * 3]);
                if (dot >= bestDot) {
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            
            xAxis = *BaseAxes[bestIndex * 3 + 1];
            yAxis = *BaseAxes[bestIndex * 3 + 2];
            faceNormIndex = bestIndex * 3;
            planeNormIndex = (bestIndex / 2) * 6;
        }
        
        void Face::validateTexAxes(const Vec3f& faceNormal) const {
            texAxesAndIndices(faceNormal, m_texAxisX, m_texAxisY, m_texPlaneNormIndex, m_texFaceNormIndex);
            
            Quat rot(Math::radians(m_rotation), *BaseAxes[m_texPlaneNormIndex]);
            m_texAxisX = rot * m_texAxisX;
            m_texAxisY = rot * m_texAxisY;
            m_scaledTexAxisX = m_texAxisX / m_xScale;
            m_scaledTexAxisY = m_texAxisY / m_yScale;
            
            m_texAxesValid = true;
        }
        
        void Face::validateCoords() const {
            assert(m_side != NULL);
            
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            int width = m_texture != NULL ? m_texture->width() : 1;
            int height = m_texture != NULL ? m_texture->height() : 1;
            
            size_t vertexCount = m_side->vertices.size();
            m_texCoords.resize(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) {
                const Vec3f& vertex = m_side->vertices[i]->position;
                
                m_texCoords[i].x = (vertex.dot(m_scaledTexAxisX) + m_xOffset) / width,
                m_texCoords[i].y = (vertex.dot(m_scaledTexAxisY) + m_yOffset) / height;
            }
            
            m_coordsValid = true;
        }
        
        void Face::compensateTransformation(const Mat4f& transformation) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            Vec3f newTexAxisX, newTexAxisY, newFaceNorm, newCenter, newBaseAxisX, newBaseAxisY, offset, cross;
            Vec2f curCenterTexCoords, newCenterTexCoords;
            Plane plane;
            Vec3f curCenter;
            int newPlaneNormIndex, newFaceNormIndex;
            float radX, radY, rad;
            
            // calculate the current texture coordinates of the face's center
            curCenter = centerOfVertices(m_side->vertices);
            curCenterTexCoords.x = (curCenter.dot(m_scaledTexAxisX)) + m_xOffset;
            curCenterTexCoords.y = (curCenter.dot(m_scaledTexAxisY)) + m_yOffset;
            
            // invert the scale of the current texture axes
            newTexAxisX = m_texAxisX * m_xScale;
            newTexAxisY = m_texAxisY * m_yScale;
            
            // project the inversely scaled texture axes onto the boundary plane
            plane.distance = 0.0f;
            plane.normal = m_boundary.normal;
            if (BaseAxes[m_texPlaneNormIndex]->x != 0.0f) {
                newTexAxisX.x = plane.x(newTexAxisX.y, newTexAxisX.z);
                newTexAxisY.x = plane.x(newTexAxisY.y, newTexAxisY.z);
            } else if (BaseAxes[m_texPlaneNormIndex]->y != 0.0f) {
                newTexAxisX.y = plane.y(newTexAxisX.x, newTexAxisX.z);
                newTexAxisY.y = plane.y(newTexAxisY.x, newTexAxisY.z);
            } else {
                newTexAxisX.z = plane.z(newTexAxisX.x, newTexAxisX.y);
                newTexAxisY.z = plane.z(newTexAxisY.x, newTexAxisY.y);
            }
            
            // apply the transformation
            newTexAxisX = transformation * newTexAxisX;
            newTexAxisY = transformation * newTexAxisY;
            newFaceNorm = transformation * m_boundary.normal;
            offset = transformation * Vec3f::Null;
            newCenter = transformation * curCenter;
            
            // correct the directional vectors by the translational part of the transformation
            newTexAxisX -= offset;
            newTexAxisY -= offset;
            newFaceNorm -= offset;
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (newFaceNorm.equals(m_boundary.normal, 0.001f))
                newFaceNorm = m_boundary.normal;
            
            // obtain the new texture plane norm and the new base texture axes
            texAxesAndIndices(newFaceNorm, newBaseAxisX, newBaseAxisY, newPlaneNormIndex, newFaceNormIndex);
            
            /*
             float tpnDot = dotV3f(texPlaneNorm, newTexPlaneNorm);
             if (tpnDot == 1 || tpnDot == -1) {
             Vec3f transformedTexPlaneNorm;
             transformM4fV3f(transformation, texPlaneNorm, &transformedTexPlaneNorm);
             subV3f(&transformedTexPlaneNorm, &offset, &transformedTexPlaneNorm);
             
             if (dotV3f(texPlaneNorm, &transformedTexPlaneNorm) == 0) {
             crossV3f(texPlaneNorm, &transformedTexPlaneNorm, &temp);
             const Vec3f* rotAxis = closestAxisV3f(&temp);
             
             float angle = Math::Pi_2;
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
            if (BaseAxes[newPlaneNormIndex]->x != 0.0f) {
                newTexAxisX.x = 0.0f;
                newTexAxisY.x = 0.0f;
            } else if (BaseAxes[newPlaneNormIndex]->y != 0.0f) {
                newTexAxisX.y = 0.0f;
                newTexAxisY.y = 0.0f;
            } else {
                newTexAxisX.z = 0.0f;
                newTexAxisY.z = 0.0f;
            }
            
            // the new scaling factors are the lengths of the transformed texture axes
            m_xScale = newTexAxisX.length();
            m_yScale = newTexAxisY.length();
            
            // normalize the transformed texture axes
            newTexAxisX /= m_xScale;
            newTexAxisY /= m_yScale;
            
            // WARNING: the texture plane norm is not the rotation axis of the texture (it's always the absolute axis)
            
            // determine the rotation angle from the dot product of the new base axes and the transformed texture axes
            radX = acosf(newBaseAxisX.dot(newTexAxisX));
            cross = newBaseAxisX.crossed(newTexAxisX);
            if ((cross.dot(*BaseAxes[newPlaneNormIndex])) < 0.0f)
                radX *= -1.0f;
            
            radY = acosf(newBaseAxisY.dot(newTexAxisY));
            cross = newBaseAxisY.crossed(newTexAxisY);
            if ((cross.dot(*BaseAxes[newPlaneNormIndex])) < 0.0f)
                radY *= -1.0f;
            
            rad = radX;
            m_rotation = Math::degrees(rad);
            
            // apply the rotation to the new base axes
            Quat rot(rad, *BaseAxes[newPlaneNormIndex]);
            newBaseAxisX = rot * newBaseAxisX;
            newBaseAxisY = rot * newBaseAxisY;
            
            // the sign of the scaling factors depends on the angle between the new base axis and the new texture axis
            if (newBaseAxisX.dot(newTexAxisX) < 0.0f)
                m_xScale *= -1.0f;
            if (newBaseAxisY.dot(newTexAxisY) < 0.0f)
                m_yScale *= -1.0f;
            
            // correct rounding errors
            m_xScale = Math::correct(m_xScale);
            m_yScale = Math::correct(m_yScale);
            m_rotation = Math::correct(m_rotation);
            
            validateTexAxes(newFaceNorm);
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            newCenterTexCoords.x = newCenter.dot(m_scaledTexAxisX);
            newCenterTexCoords.y = newCenter.dot(m_scaledTexAxisY);
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            m_xOffset = curCenterTexCoords.x - newCenterTexCoords.x;
            m_yOffset = curCenterTexCoords.y - newCenterTexCoords.y;
            
            if (m_texture != NULL) {
                m_xOffset -= static_cast<int>(Math::round(m_xOffset / static_cast<float>(m_texture->width()))) * static_cast<int>(m_texture->width());
                m_yOffset -= static_cast<int>(Math::round(m_yOffset / static_cast<float>(m_texture->height()))) * static_cast<int>(m_texture->height());
            }
            
            // correct rounding errors
            m_xOffset = Math::correct(m_xOffset);
            m_yOffset = Math::correct(m_yOffset);
        }

        Face::Face(const BBox& worldBounds, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const String& textureName) : m_worldBounds(worldBounds), m_textureName(textureName) {
            init();
            m_points[0] = point1;
            m_points[1] = point2;
            m_points[2] = point3;
            m_boundary.setPoints(m_points[0], m_points[1], m_points[2]);
        }
        
        Face::Face(const BBox& worldBounds, const Face& faceTemplate) : m_worldBounds(worldBounds) {
            init();
            restore(faceTemplate);
        }
        
        Face::Face(const Face& face) :
        m_side(NULL),
        m_faceId(face.faceId()),
        m_boundary(face.boundary()),
        m_worldBounds(face.worldBounds()),
        m_textureName(face.textureName()),
        m_texture(face.texture()),
        m_xOffset(face.xOffset()),
        m_yOffset(face.yOffset()),
        m_rotation(face.rotation()),
        m_xScale(face.xScale()),
        m_yScale(face.yScale()),
        m_texAxesValid(false),
        m_coordsValid(false),
        m_filePosition(face.filePosition()),
        m_selected(false) {
            face.getPoints(m_points[0], m_points[1], m_points[2]);
        }
        
		Face::~Face() {
			m_texPlaneNormIndex = -1;
			m_texFaceNormIndex = -1;
			m_texAxisX = Vec3f::NaN;
			m_texAxisY = Vec3f::NaN;
			m_scaledTexAxisX = Vec3f::NaN;
			m_scaledTexAxisY = Vec3f::NaN;
            
			m_texCoords.clear();
            
			m_faceId *= -1;
			m_brush = NULL;
			
			for (unsigned int i = 0; i < 3; i++)
				m_points[i] = Vec3f::NaN;
			m_boundary.normal = Vec3f::NaN;
			m_boundary.distance = -1.0f;
            
			m_textureName = "___deleted";
			m_texture = NULL;
			m_xOffset = 0;
			m_yOffset = 0;
			m_rotation = 0.0f;
			m_xScale = 0.0f;
			m_yScale = 0.0f;
			m_side = NULL;
			m_filePosition = -1;
			m_selected = false;
			m_coordsValid = false;
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
            m_coordsValid = false;
			m_selected = faceTemplate.selected();
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
        
        void Face::updatePoints() {
            Vec3f v1, v2;
            
            float bestDot = 1;
            int best = -1;
            size_t vertexCount = m_side->vertices.size();
            for (unsigned int i = 0; i < vertexCount && bestDot > 0; i++) {
                m_points[2] = m_side->vertices[pred(i, vertexCount)]->position;
                m_points[0] = m_side->vertices[i]->position;
                m_points[1] = m_side->vertices[succ(i, vertexCount)]->position;
                
                v1 = (m_points[2] - m_points[0]).normalized();
                v2 = (m_points[1] - m_points[0]).normalized();
                float dot = v1.dot(v2);
                if (dot < bestDot) {
                    bestDot = dot;
                    best = i;
                }
            }
            
            assert(best != -1);
            
            m_points[2] = m_side->vertices[pred(best, vertexCount)]->position;
            m_points[0] = m_side->vertices[best]->position;
            m_points[1] = m_side->vertices[succ(best, vertexCount)]->position;
            
            if (!m_boundary.setPoints(m_points[0], m_points[1], m_points[2])) {
                std::stringstream msg;
                msg << "Invalid face m_points "
                << m_points[0].x << " " << m_points[0].y << " " << m_points[0].z << "; "
                << m_points[1].x << " " << m_points[1].y << " " << m_points[1].z << "; "
                << m_points[2].x << " " << m_points[2].y << " " << m_points[2].z
                << " for face with ID " << m_faceId;
                throw GeometryException(msg);
            }
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
            m_coordsValid = false;
        }

        void Face::moveTexture(float distance, const Vec3f& direction) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            float dotX = direction.dot(m_scaledTexAxisX);
            float dotY = direction.dot(m_scaledTexAxisY);
            
            if (fabsf(dotX) >= fabsf(dotY)) {
                if (dotX >= 0.0f)
                    m_xOffset -= distance;
                else
                    m_xOffset += distance;
            } else {
                if (dotY >= 0.0f)
                    m_yOffset -= distance;
                else
                    m_yOffset += distance;
            }
            
            m_coordsValid = false;
        }
        
        void Face::rotateTexture(float angle) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            if (m_texPlaneNormIndex == m_texFaceNormIndex)
                m_rotation += angle;
            else
                m_rotation -= angle;
            m_texAxesValid = false;
            m_coordsValid = false;
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

        void Face::move(float dist, bool lockTexture) {
        }
        
        void Face::translate(const Vec3f& delta, bool lockTexture) {
            if (lockTexture)
                compensateTransformation(Mat4f::Identity.translated(delta));
            
            m_boundary.translate(delta);
            for (unsigned int i = 0; i < 3; i++)
                m_points[i] += delta;
            
            m_texAxesValid = false;
            m_coordsValid = false;
        }

        void Face::rotate90(Axis::Type axis, const Vec3f& center, bool clockwise, bool lockTexture) {
            if (lockTexture) {
                Mat4f t = Mat4f::Identity.translated(center);
                if (axis == Axis::AX)
                    t *= clockwise ? Mat4f::Rot90XCW : Mat4f::Rot90XCCW;
                else if (axis == Axis::AY)
                    t *= clockwise ? Mat4f::Rot90YCW : Mat4f::Rot90YCCW;
                else
                    t *= clockwise ? Mat4f::Rot90ZCW : Mat4f::Rot90ZCCW;
                t.translate(-center);
                compensateTransformation(t);
            }
            
            m_boundary.rotate90(axis, center, clockwise);
            for (unsigned int i = 0; i < 3; i++)
                m_points[i].rotate90(axis, center, clockwise);
            
            m_texAxesValid = false;
            m_coordsValid = false;
        }

        void Face::rotate(const Quat& rotation, const Vec3f& center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t = Mat4f::Identity.translated(center);
                t.rotate(rotation);
                t.translate(-1.0f * center);
                compensateTransformation(t);
            }
            
            m_boundary = m_boundary.rotate(rotation, center);
            
            for (unsigned int i = 0; i < 3; i++)
                m_points[i] = rotation * (m_points[i] - center) + center;
            
            m_texAxesValid = false;
            m_coordsValid = false;
        }

        void Face::flip(Axis::Type axis, const Vec3f& center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t;
                Vec3f d;
                switch (axis) {
                    case Axis::AX:
                        d = Vec3f(center.x, 0.0f, 0.0f);
                        t.setIdentity();
                        t *= Mat4f::MirX;
                        t.translate(-1.0f * d);
                        break;
                    case Axis::AY:
                        d = Vec3f(0.0f, center.y, 0.0f);
                        t.setIdentity();
                        t *= Mat4f::MirY;
                        t.translate(-1.0f * d);
                        break;
                    case Axis::AZ:
                        d = Vec3f(0.0f, 0.0f, center.z);
                        t.setIdentity();
                        t *= Mat4f::MirZ;
                        t.translate(-1.0f * d);
                        break;
                }
                compensateTransformation(t);
            }
            
            m_boundary.flip(axis, center);
            for (unsigned int i = 0; i < 3; i++)
                m_points[i].flip(axis, center);
            
            Vec3f t = m_points[1];
            m_points[1] = m_points[2];
            m_points[2] = t;
            m_texAxesValid = false;
            m_coordsValid = false;        }
    }
}
