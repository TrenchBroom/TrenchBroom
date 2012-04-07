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
#include <assert.h>
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
                float dot = dotV3f(&faceNormal, BaseAxes[i * 3]);
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
            TQuaternion rot;
            
            texAxesAndIndices(faceNormal, m_texAxisX, m_texAxisY, m_texPlaneNormIndex, m_texFaceNormIndex);
            
            setAngleAndAxisQ(&rot, m_rotation * M_PI / 180, BaseAxes[m_texPlaneNormIndex]);
            rotateQ(&rot, &m_texAxisX, &m_texAxisX);
            rotateQ(&rot, &m_texAxisY, &m_texAxisY);
            
            scaleV3f(&m_texAxisX, 1 / m_xScale, &m_scaledTexAxisX);
            scaleV3f(&m_texAxisY, 1 / m_yScale, &m_scaledTexAxisY);
            
            m_texAxesValid = true;
        }
        
        
        void Face::compensateTransformation(const TMatrix4f& transformation) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.norm);
            
            Vec3f newTexAxisX, newTexAxisY, newFaceNorm, newCenter, newBaseAxisX, newBaseAxisY, offset, cross;
            Vec2f curCenterTexCoords, newCenterTexCoords;
            TPlane plane;
            Vec3f curCenter;
            int newPlaneNormIndex, newFaceNormIndex;
            float radX, radY, rad;
            
            // calculate the current texture coordinates of the face's center
            curCenter = centerOfVertices(m_side->vertices);
            curCenterTexCoords.x = dotV3f(&curCenter, &m_scaledTexAxisX) + m_xOffset;
            curCenterTexCoords.y = dotV3f(&curCenter, &m_scaledTexAxisY) + m_yOffset;
            
            // invert the scale of the current texture axes
            scaleV3f(&m_texAxisX, m_xScale, &newTexAxisX);
            scaleV3f(&m_texAxisY, m_yScale, &newTexAxisY);
            
            // project the inversely scaled texture axes onto the boundary plane
            plane.point = NullVector;
            plane.norm = m_boundary.norm;
            if (BaseAxes[m_texPlaneNormIndex]->x != 0) {
                newTexAxisX.x = planeX(&plane, newTexAxisX.y, newTexAxisX.z);
                newTexAxisY.x = planeX(&plane, newTexAxisY.y, newTexAxisY.z);
            } else if (BaseAxes[m_texPlaneNormIndex]->y != 0) {
                newTexAxisX.y = planeY(&plane, newTexAxisX.x, newTexAxisX.z);
                newTexAxisY.y = planeY(&plane, newTexAxisY.x, newTexAxisY.z);
            } else {
                newTexAxisX.z = planeZ(&plane, newTexAxisX.x, newTexAxisX.y);
                newTexAxisY.z = planeZ(&plane, newTexAxisY.x, newTexAxisY.y);
            }
            
            // apply the transformation
            transformM4fV3f(&transformation, &newTexAxisX, &newTexAxisX);
            transformM4fV3f(&transformation, &newTexAxisY, &newTexAxisY);
            transformM4fV3f(&transformation, &m_boundary.norm, &newFaceNorm);
            transformM4fV3f(&transformation, &NullVector, &offset);
            transformM4fV3f(&transformation, &curCenter, &newCenter);
            
            // correct the directional vectors by the translational part of the transformation
            subV3f(&newTexAxisX, &offset, &newTexAxisX);
            subV3f(&newTexAxisY, &offset, &newTexAxisY);
            subV3f(&newFaceNorm, &offset, &newFaceNorm);
            
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
            m_xScale = lengthV3f(&newTexAxisX);
            m_yScale = lengthV3f(&newTexAxisY);
            
            // normalize the transformed texture axes
            scaleV3f(&newTexAxisX, 1 / m_xScale, &newTexAxisX);
            scaleV3f(&newTexAxisY, 1 / m_yScale, &newTexAxisY);
            
            // WARNING: the texture plane norm is not the rotation axis of the texture (it's always the absolute axis)
            
            // determine the rotation angle from the dot product of the new base axes and the transformed texture axes
            radX = acosf(dotV3f(&newBaseAxisX, &newTexAxisX));
            crossV3f(&newBaseAxisX, &newTexAxisX, &cross);
            if (dotV3f(&cross, BaseAxes[newPlaneNormIndex]) < 0)
                radX *= -1;
            
            radY = acosf(dotV3f(&newBaseAxisY, &newTexAxisY));
            crossV3f(&newBaseAxisY, &newTexAxisY, &cross);
            if (dotV3f(&cross, BaseAxes[newPlaneNormIndex]) < 0)
                radY *= -1;
            
            rad = radX;
            m_rotation = rad * 180 / M_PI;
            
            // apply the rotation to the new base axes
            TQuaternion rot;
            //    Vec3f rotAxis;
            //    rotAxis = *newTexPlaneNorm;
            //    absV3f(newTexPlaneNorm, &rotAxis);
            setAngleAndAxisQ(&rot, rad, BaseAxes[newPlaneNormIndex]);
            rotateQ(&rot, &newBaseAxisX, &newBaseAxisX);
            rotateQ(&rot, &newBaseAxisY, &newBaseAxisY);
            
            // the sign of the scaling factors depends on the angle between the new base axis and the new texture axis
            if (dotV3f(&newBaseAxisX, &newTexAxisX) < 0)
                m_xScale *= -1;
            if (dotV3f(&newBaseAxisY, &newTexAxisY) < 0)
                m_yScale *= -1;
            
            validateTexAxes(newFaceNorm);
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            newCenterTexCoords.x = dotV3f(&newCenter, &m_scaledTexAxisX);
            newCenterTexCoords.y = dotV3f(&newCenter, &m_scaledTexAxisY);
            
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
        
        Face::Face(const BBox& worldBounds) : m_worldBounds(worldBounds) {
            init();
        }
        
        Face::Face(const BBox& worldBounds, Vec3f point1, Vec3f point2, Vec3f point3) : m_worldBounds(worldBounds) {
            init();
            m_points[0] = point1;
            m_points[1] = point2;
            m_points[3] = point3;
        }
        
        Face::Face(const BBox& worldBounds, const Face& faceTemplate) : m_worldBounds(worldBounds) {
            init();
            restore(faceTemplate);
        }
        
        Face::~Face() {
            if (m_vboBlock != NULL)
                freeVboBlock(m_vboBlock);
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
            int vertexCount = vertices.size();
            for (int i = 0; i < vertexCount && bestDot > 0; i++) {
                m_points[2] = vertices[(i - 1 + vertexCount) % vertexCount]->position;
                m_points[0] = vertices[i]->position;
                m_points[1] = vertices[(i + 1) % vertexCount]->position;
                
                subV3f(&m_points[2], &m_points[0], &v1);
                normalizeV3f(&v1, &v1);
                subV3f(&m_points[1], &m_points[0], &v2);
                normalizeV3f(&v2, &v2);
                
                float dot = fabsf(dotV3f(&v1, &v2));
                if (dot < bestDot) {
                    bestDot = dot;
                    best = i;
                }
            }
            
            assert(best != -1);
            
            m_points[2] = vertices[(best - 1 + vertexCount) % vertexCount]->position;
            m_points[0] = vertices[best]->position;
            m_points[1] = vertices[(best + 1) % vertexCount]->position;
            
            setPlanePointsV3f(&m_boundary, &m_points[0], &m_points[1], &m_points[2]);
        }
        
        Vec3f Face::normal() const {
            return m_boundary.norm;
        }
        
        TPlane Face::boundary() const {
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
        
        Texture* Face::texture() const {
            return m_texture;
        }
        
        void Face::setTexture(Texture* texture) {
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
                validateTexAxes(m_boundary.norm);
            
            float dotX = dotV3f(&dir, &m_texAxisX);
            float dotY = dotV3f(&dir, &m_texAxisY);
            
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
                validateTexAxes(m_boundary.norm);
            
            if (m_texPlaneNormIndex == m_texFaceNormIndex)
                m_rotation += angle;
            else
                m_rotation -= angle;
            m_texAxesValid = false;
        }
        
        void Face::translate(Vec3f delta, bool lockTexture) {
            if (lockTexture) {
                TMatrix4f t;
                translateM4f(&IdentityM4f, &delta, &t);
                compensateTransformation(t);
            }
            
            addV3f(&m_boundary.point, &delta, &m_boundary.point);
            for (int i = 0; i < 3; i++)
                addV3f(&m_points[i], &delta, &m_points[i]);
            
            m_texAxesValid = false;
        }
        
        void Face::rotate90CW(EAxis axis, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                TMatrix4f t;
                Vec3f invCenter;
                
                translateM4f(&IdentityM4f, &center, &t);
                if (axis == A_X)
                    mulM4f(&t, &RotX90CWM4f, &t);
                else if (axis == A_Y)
                    mulM4f(&t, &RotY90CWM4f, &t);
                else
                    mulM4f(&t, &RotZ90CWM4f, &t);
                
                scaleV3f(&center, -1, &invCenter);
                translateM4f(&t, &invCenter, &t);
                compensateTransformation(t);
            }
            
            subV3f(&m_boundary.point, &center, &m_boundary.point);
            rotate90CWV3f(&m_boundary.point, axis, &m_boundary.point);
            rotate90CWV3f(&m_boundary.norm, axis, &m_boundary.norm);
            addV3f(&m_boundary.point, &center, &m_boundary.point);
            
            for (int i = 0; i < 3; i++) {
                subV3f(&m_points[i], &center, &m_points[i]);
                rotate90CWV3f(&m_points[i], axis, &m_points[i]);
                addV3f(&m_points[i], &center, &m_points[i]);
            }
            
            m_texAxesValid = false;
        }
        
        void Face::rotate90CCW(EAxis axis, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                TMatrix4f t;
                Vec3f invCenter;
                
                translateM4f(&IdentityM4f, &center, &t);
                if (axis == A_X)
                    mulM4f(&t, &RotX90CCWM4f, &t);
                else if (axis == A_Y)
                    mulM4f(&t, &RotY90CCWM4f, &t);
                else
                    mulM4f(&t, &RotZ90CCWM4f, &t);
                
                scaleV3f(&center, -1, &invCenter);
                translateM4f(&t, &invCenter, &t);
                compensateTransformation(t);
            }
            
            subV3f(&m_boundary.point, &center, &m_boundary.point);
            rotate90CCWV3f(&m_boundary.point, axis, &m_boundary.point);
            rotate90CCWV3f(&m_boundary.norm, axis, &m_boundary.norm);
            addV3f(&m_boundary.point, &center, &m_boundary.point);
            
            for (int i = 0; i < 3; i++) {
                subV3f(&m_points[i], &center, &m_points[i]);
                rotate90CCWV3f(&m_points[i], axis, &m_points[i]);
                addV3f(&m_points[i], &center, &m_points[i]);
            }
            
            m_texAxesValid = false;
        }
        
        void Face::rotate(TQuaternion rotation, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                TMatrix4f t;
                Vec3f invCenter;
                
                translateM4f(&IdentityM4f, &center, &t);
                rotateM4fQ(&t, &rotation, &t);
                scaleV3f(&center, -1, &invCenter);
                translateM4f(&t, &invCenter, &t);
                compensateTransformation(t);
            }
            
            subV3f(&m_boundary.point, &center, &m_boundary.point);
            rotateQ(&rotation, &m_boundary.point, &m_boundary.point);
            rotateQ(&rotation, &m_boundary.norm, &m_boundary.norm);
            addV3f(&m_boundary.point, &center, &m_boundary.point);
            
            for (int i = 0; i < 3; i++) {
                subV3f(&m_points[i], &center, &m_points[i]);
                rotateQ(&rotation, &m_points[i], &m_points[i]);
                addV3f(&m_points[i], &center, &m_points[i]);
            }
            
            m_texAxesValid = false;
        }
        
        void Face::flip(EAxis axis, Vec3f center, bool lockTexture) {
            switch (axis) {
                case A_X: {
                    if (lockTexture) {
                        TMatrix4f t1, t2;
                        Vec3f d;
                        
                        d.x = center.x;
                        d.y = 0;
                        d.z = 0;
                        translateM4f(&IdentityM4f, &d, &t1);
                        mulM4f(&t1, &MirXM4f, &t1);
                        scaleV3f(&d, -1, &d);
                        translateM4f(&IdentityM4f, &d, &t2);
                        mulM4f(&t1, &t2, &t1);
                        compensateTransformation(t1);
                    }
                    
                    m_boundary.point.x -= center.x;
                    m_boundary.point.x *= -1;
                    m_boundary.point.x += center.x;
                    m_boundary.norm.x *= -1;
                    
                    for (int i = 0; i < 3; i++) {
                        m_points[i].x -= center.x;
                        m_points[i].x *= -1;
                        m_points[i].x += center.x;
                    }
                    break;
                }
                case A_Y: {
                    if (lockTexture) {
                        TMatrix4f t1, t2;
                        Vec3f d;
                        
                        d.x = 0;
                        d.y = center.y;
                        d.z = 0;
                        translateM4f(&IdentityM4f, &d, &t1);
                        mulM4f(&t1, &MirYM4f, &t1);
                        scaleV3f(&d, -1, &d);
                        translateM4f(&IdentityM4f, &d, &t2);
                        mulM4f(&t1, &t2, &t1);
                        compensateTransformation(t1);
                    }
                    
                    m_boundary.point.y -= center.y;
                    m_boundary.point.y *= -1;
                    m_boundary.point.y += center.y;
                    m_boundary.norm.y *= -1;
                    
                    for (int i = 0; i < 3; i++) {
                        m_points[i].y -= center.y;
                        m_points[i].y *= -1;
                        m_points[i].y += center.y;
                    }
                    break;
                }
                default: {
                    if (lockTexture) {
                        TMatrix4f t1, t2;
                        Vec3f d;
                        
                        d.x = 0;
                        d.y = 0;
                        d.z = center.z;
                        translateM4f(&IdentityM4f, &d, &t1);
                        mulM4f(&t1, &MirZM4f, &t1);
                        scaleV3f(&d, -1, &d);
                        translateM4f(&IdentityM4f, &d, &t2);
                        mulM4f(&t1, &t2, &t1);
                        compensateTransformation(t1);
                    }
                    
                    m_boundary.point.z -= center.z;
                    m_boundary.point.z *= -1;
                    m_boundary.point.z += center.z;
                    m_boundary.norm.z *= -1;
                    
                    for (int i = 0; i < 3; i++) {
                        m_points[i].z -= center.z;
                        m_points[i].z *= -1;
                        m_points[i].z += center.z;
                    }
                    break;
                }
            }
            
            Vec3f t = m_points[1];
            m_points[1] = m_points[2];
            m_points[2] = t;
            m_texAxesValid = false;
        }
        
        void Face::move(float dist, bool lockTexture) {
            Vec3f deltaf;
            scaleV3f(&m_boundary.norm, dist, &deltaf);
            
            addV3f(&m_boundary.point, &deltaf, &m_boundary.point);
            for (int i = 0; i < 3; i++)
                addV3f(&m_points[i], &deltaf, &m_points[i]);
            
            m_texAxesValid = false; 
        }
        
        Vec2f Face::textureCoords(Vec3f vertex) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.norm);
            
            Vec2f texCoords;
            texCoords.x = dotV3f(&vertex, &m_scaledTexAxisX) + m_xOffset;
            texCoords.y = dotV3f(&vertex, &m_scaledTexAxisY) + m_yOffset;
            return texCoords;
        }
        
        Vec2f Face::gridCoords(Vec3f vertex) {
            if (!m_texAxesValid)
                validateTexAxes(m_boundary.norm);
            
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
        
        VboBlock* Face::vboBlock() const {
            return m_vboBlock;
        }
        
        void Face::setVboBlock(VboBlock* vboBlock) {
            if (m_vboBlock != NULL)
                freeVboBlock(m_vboBlock);
            m_vboBlock = vboBlock;
        }
    }
}