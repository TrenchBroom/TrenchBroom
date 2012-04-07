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
#include <cassert>
#include <cmath>
#include "BrushGeometry.h"

namespace TrenchBroom {
    namespace Model {
        static const Vec3f* BaseAxes[18] = { &ZAxisPos, &XAxisPos, &YAxisNeg,
            &ZAxisNeg, &XAxisPos, &YAxisNeg,
            &XAxisPos, &YAxisPos, &ZAxisNeg,
            &XAxisNeg, &YAxisPos, &ZAxisNeg,
            &YAxisPos, &XAxisPos, &ZAxisNeg,
            &YAxisNeg, &XAxisPos, &ZAxisNeg};
        
        void Face::texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, int& planeNormIndex, int& faceNormIndex) const {
            int bestIndex = 0;
            float bestDot = -1;
            for (int i = 0; i < 6; i++) {
                float dot = faceNormal | *BaseAxes[i * 3];
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
        
        void Face::validateTexAxes(const Vec3f& faceNormal) {
            texAxesAndIndices(faceNormal, m_texAxisX, m_texAxisY, m_texPlaneNormIndex, m_texFaceNormIndex);
            
            Quat rot(m_rotation * M_PI / 180, *BaseAxes[m_texPlaneNormIndex]);
            m_texAxisX = rot * m_texAxisX;
            m_texAxisY = rot * m_texAxisY;
            m_texAxisX /= m_xScale;
            m_texAxisY /= m_yScale;
            
            m_texAxesValid = true;
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
            curCenterTexCoords.x = (curCenter | m_scaledTexAxisX) + m_xOffset;
            curCenterTexCoords.y = (curCenter | m_scaledTexAxisY) + m_yOffset;
            
            // invert the scale of the current texture axes
            newTexAxisX = m_texAxisX * m_xScale;
            newTexAxisY = m_texAxisY * m_yScale;
            
            // project the inversely scaled texture axes onto the boundary plane
            plane.distance = 0;
            plane.normal = m_boundary.normal;
            if (BaseAxes[m_texPlaneNormIndex]->x != 0) {
                newTexAxisX.x = plane.x(newTexAxisX.y, newTexAxisX.z);
                newTexAxisY.x = plane.x(newTexAxisY.y, newTexAxisY.z);
            } else if (BaseAxes[m_texPlaneNormIndex]->y != 0) {
                newTexAxisX.x = plane.y(newTexAxisX.x, newTexAxisX.z);
                newTexAxisY.x = plane.y(newTexAxisY.x, newTexAxisY.z);
            } else {
                newTexAxisX.x = plane.z(newTexAxisX.x, newTexAxisX.y);
                newTexAxisY.x = plane.z(newTexAxisY.x, newTexAxisY.y);
            }
            
            // apply the transformation
            newTexAxisX = transformation * newTexAxisX;
            newTexAxisY = transformation * newTexAxisY;
            newFaceNorm = transformation * m_boundary.normal;
            offset = transformation * Null3f;
            newCenter = transformation * curCenter;
            
            // correct the directional vectors by the translational part of the transformation
            newTexAxisX -= offset;
            newTexAxisY -= offset;
            newFaceNorm -= offset;
            
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
             
             float angle = M_PI_2;
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
            if (BaseAxes[newPlaneNormIndex]->x != 0) {
                newTexAxisX.x = 0;
                newTexAxisY.x = 0;
            } else if (BaseAxes[newPlaneNormIndex]->y != 0) {
                newTexAxisX.y = 0;
                newTexAxisY.y = 0;
            } else {
                newTexAxisX.z = 0;
                newTexAxisY.z = 0;
            }
            
            // the new scaling factors are the lengths of the transformed texture axes
            m_xScale = newTexAxisX.length();
            m_yScale = newTexAxisY.length();
            
            // normalize the transformed texture axes
            newTexAxisX /= m_xScale;
            newTexAxisY /= m_yScale;
            
            // WARNING: the texture plane norm is not the rotation axis of the texture (it's always the absolute axis)
            
            // determine the rotation angle from the dot product of the new base axes and the transformed texture axes
            radX = acosf(newBaseAxisX | newTexAxisX);
            cross = newBaseAxisX % newTexAxisX;
            if ((cross | *BaseAxes[newPlaneNormIndex]) < 0)
                radX *= -1;
            
            radY = acosf(newBaseAxisY | newTexAxisY);
            cross = newBaseAxisY % newTexAxisY;
            if ((cross | *BaseAxes[newPlaneNormIndex]) < 0)
                radY *= -1;
            
            rad = radX;
            m_rotation = rad * 180 / M_PI;
            
            // apply the rotation to the new base axes
            Quat rot(rad, *BaseAxes[newPlaneNormIndex]);
            newBaseAxisX = rot * newBaseAxisX;
            newBaseAxisY = rot * newBaseAxisY;
            
            // the sign of the scaling factors depends on the angle between the new base axis and the new texture axis
            if ((newBaseAxisX | newTexAxisX) < 0)
                m_xScale *= -1;
            if ((newBaseAxisY | newTexAxisY) < 0)
                m_yScale *= -1;
            
            validateTexAxes(newFaceNorm);
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            newCenterTexCoords.x = (newCenter | m_scaledTexAxisX);
            newCenterTexCoords.y = (newCenter | m_scaledTexAxisY);
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            m_xOffset = curCenterTexCoords.x - newCenterTexCoords.x;
            m_yOffset = curCenterTexCoords.y - newCenterTexCoords.y;
            
            m_xOffset -= ((int)m_xOffset / m_texture->width) * m_texture->width;
            m_yOffset -= ((int)m_yOffset / m_texture->height) * m_texture->height;
        }
        
        void Face::init() {
            static int currentId = 1;
            m_faceId = currentId++;
            m_brush = NULL;
            m_texture = NULL;
            m_vboBlock = NULL;
            m_filePosition = -1;
            m_selected = false;
            m_texAxesValid = false;
        }
        
        Face::Face(const BBox& worldBounds, Vec3f point1, Vec3f point2, Vec3f point3) : m_worldBounds(worldBounds) {
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
        
        Face::~Face() {
            if (m_vboBlock != NULL)
                m_vboBlock->freeBlock();
        }
        
        void Face::restore(const Face& faceTemplate) {
            faceTemplate.points(m_points[0], m_points[1], m_points[2]);
            m_boundary = faceTemplate.boundary();
            m_xOffset = faceTemplate.xOffset();
            m_yOffset = faceTemplate.yOffset();
            m_rotation = faceTemplate.rotation();
            m_xScale = faceTemplate.xScale();
            m_yScale = faceTemplate.yScale();
            setTexture(faceTemplate.texture());
            m_texAxesValid = false;
        }
        
        int Face::faceId() const {
            return m_faceId;
        }
        
        Brush* Face::brush() const {
            return m_brush;
        }
        
        void Face::setBrush(Brush* brush) {
            m_brush = brush;
        }
        
        void Face::setSide(Side* side) {
            m_side = side;
        }
        
        void Face::points(Vec3f& point1, Vec3f& point2, Vec3f& point3) const {
            point1 = m_points[0];
            point2 = m_points[1];
            point3 = m_points[2];
        }
        
        void Face::updatePoints() {
            Vec3f v1, v2;
            
            float bestDot = 1;
            int best = -1;
            vector<Vertex*> vertices = this->vertices();
            size_t vertexCount = vertices.size();
            for (int i = 0; i < vertexCount && bestDot > 0; i++) {
                m_points[2] = vertices[(i - 1 + vertexCount) % vertexCount]->position;
                m_points[0] = vertices[i]->position;
                m_points[1] = vertices[(i + 1) % vertexCount]->position;
                
                v1 = (m_points[2] - m_points[0]).normalize();
                v2 = (m_points[1] - m_points[0]).normalize();
                float dot = v1 | v2;
                if (dot < bestDot) {
                    bestDot = dot;
                    best = i;
                }
            }
            
            assert(best != -1);
            
            m_points[2] = vertices[(best - 1 + vertexCount) % vertexCount]->position;
            m_points[0] = vertices[best]->position;
            m_points[1] = vertices[(best + 1) % vertexCount]->position;
            
            assert(m_boundary.setPoints(m_points[0], m_points[1], m_points[2]));
        }
        
        Vec3f Face::normal() const {
            return m_boundary.normal;
        }
        
        Plane Face::boundary() const {
            return m_boundary;
        }
        
        Vec3f Face::center() const {
            return centerOfVertices(vertices());
        }
        
        const BBox& Face::worldBounds() const {
            return m_worldBounds;
        }
        
        const vector<Vertex*>& Face::vertices() const {
            return m_side->vertices;
        }
        
        const vector<Edge*>& Face::edges() const {
            return m_side->edges;
        }
        
        Assets::Texture* Face::texture() const {
            return m_texture;
        }
        
        void Face::setTexture(Assets::Texture* texture) {
            if (m_texture != NULL)
                m_texture->usageCount--;
            
            m_texture = texture;
            
            if (m_texture != NULL)
                m_texture->usageCount++;
        }
        
        int Face::xOffset() const {
            return m_xOffset;
        }
        
        void Face::setXOffset(int xOffset) {
            m_xOffset = xOffset;
            m_texAxesValid = false;
        }
        
        int Face::yOffset() const {
            return m_yOffset;
        }
        
        void Face::setYOffset(int yOffset) {
            m_yOffset = yOffset;
            m_texAxesValid = false;
        }
        
        float Face::rotation() const {
            return m_rotation;
        }
        
        void Face::setRotation(float rotation) {
            m_rotation = rotation;
            m_texAxesValid = false;
        }
        
        float Face::xScale() const {
            return m_xScale;
        }
        
        void Face::setXScale(float xScale) {
            m_xScale = xScale;
            m_texAxesValid = false;
        }
        
        float Face::yScale() const {
            return m_yScale;
        }
        
        void Face::setYScale(float yScale) {
            m_yScale = yScale;
            m_texAxesValid = false;
        }
        
        void Face::translateOffsets(float delta, Vec3f dir) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            float dotX = dir | m_texAxisX;
            float dotY = dir | m_texAxisY;
            
            if (fabsf(dotX) >= fabsf(dotY)) {
                if (dotX >= 0)
                    m_xOffset -= delta;
                else
                    m_xOffset += delta;
            } else {
                if (dotY >= 0)
                    m_yOffset -= delta;
                else
                    m_yOffset += delta;
            }
        }
        
        void Face::rotateTexture(float angle) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            if (m_texPlaneNormIndex == m_texFaceNormIndex)
                m_rotation += angle;
            else
                m_rotation -= angle;
            m_texAxesValid = false;
        }
        
        void Face::translate(Vec3f delta, bool lockTexture) {
            if (lockTexture)
                compensateTransformation(IdentityM4f.translate(delta));
            
            m_boundary = m_boundary.translate(delta);
            for (int i = 0; i < 3; i++)
                m_points[i] += delta;
            
            m_texAxesValid = false;
        }
        
        void Face::rotate90CW(EAxis axis, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t = IdentityM4f.translate(center);
                if (axis == A_X) t *= RotX90CWM4f;
                else if (axis == A_Y) t *= RotY90CWM4f;
                else t *= RotZ90CWM4f;
                t.translate(center * -1);
                compensateTransformation(t);
            }
            
            m_boundary = m_boundary.rotate90(axis, center, true);
            for (int i = 0; i < 3; i++)
                m_points[i] = m_points[i].rotate90(axis, center, true);
            
            m_texAxesValid = false;
        }
        
        void Face::rotate90CCW(EAxis axis, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t = IdentityM4f.translate(center);
                if (axis == A_X) t *= RotX90CCWM4f;
                else if (axis == A_Y) t *= RotY90CCWM4f;
                else t *= RotZ90CCWM4f;
                t.translate(center * -1);
                compensateTransformation(t);
            }
            
            m_boundary = m_boundary.rotate90(axis, center, false);
            for (int i = 0; i < 3; i++)
                m_points[i] = m_points[i].rotate90(axis, center, false);
            
            m_texAxesValid = false;
        }
        
        void Face::rotate(Quat rotation, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t = IdentityM4f.translate(center).rotate(rotation).translate(center * -1);
                compensateTransformation(t);
            }
            
            m_boundary = m_boundary.rotate(rotation, center);
            
            for (int i = 0; i < 3; i++)
                m_points[i] = rotation * (m_points[i] - center) + center;
            
            m_texAxesValid = false;
        }
        
        void Face::flip(EAxis axis, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t;
                Vec3f d;
                switch (axis) {
                    case A_X:
                        d = Vec3f(center.x, 0, 0);
                        t = IdentityM4f.translate(d) * MirXM4f * IdentityM4f.translate(d * -1);
                        break;
                    case A_Y:
                        d = Vec3f(0, center.y, 0);
                        t = IdentityM4f.translate(d) * MirYM4f * IdentityM4f.translate(d * -1);
                        break;
                    case A_Z:
                        d = Vec3f(0, 0, center.z);
                        t = IdentityM4f.translate(d) * MirZM4f * IdentityM4f.translate(d * -1);
                        break;
                }
                compensateTransformation(t);
            }
            
            m_boundary = m_boundary.flip(axis, center);
            for (int i = 0; i < 3; i++)
                m_points[i] = m_points[i].flip(axis, center);
            
            Vec3f t = m_points[1];
            m_points[1] = m_points[2];
            m_points[2] = t;
            m_texAxesValid = false;
        }
        
        void Face::move(float dist, bool lockTexture) {
            m_boundary.distance += dist;
            Vec3f delta = m_boundary.normal * dist;
            for (int i = 0; i < 3; i++)
                m_points[i] += delta;
            
            m_texAxesValid = false; 
        }
        
        Vec2f Face::textureCoords(Vec3f vertex) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            Vec2f texCoords;
            texCoords.x = (vertex | m_scaledTexAxisX) + m_xOffset;
            texCoords.y = (vertex | m_scaledTexAxisY) + m_yOffset;
            return texCoords;
        }
        
        Vec2f Face::gridCoords(Vec3f vertex) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.normal);
            
            Vec2f gridCoords;
            switch (m_texFaceNormIndex) {
                case 2:
                case 3:
                    gridCoords.x = (vertex.y + 0.5f) / 256;
                    gridCoords.y = (vertex.z + 0.5f) / 256;
                    break;
                case 4:
                case 5:
                    gridCoords.x = (vertex.x + 0.5f) / 256;
                    gridCoords.y = (vertex.z + 0.5f) / 256;
                    break;
                default:
                    gridCoords.x = (vertex.x + 0.5f) / 256;
                    gridCoords.y = (vertex.y + 0.5f) / 256;
                    break;
            }
            return gridCoords;
        }
        
        int Face::filePosition() const {
            return m_filePosition;
        }
        
        void Face::setFilePosition(int filePosition) {
            m_filePosition = filePosition;
        }
        
        bool Face::selected() const {
            return m_selected;
        }
        
        void Face::setSelected(bool selected) {
            m_selected = selected;
        }
        
        Renderer::VboBlock* Face::vboBlock() const {
            return m_vboBlock;
        }
        
        void Face::setVboBlock(Renderer::VboBlock* vboBlock) {
            if (m_vboBlock != NULL)
                m_vboBlock->freeBlock();
            m_vboBlock = vboBlock;
        }
    }
}