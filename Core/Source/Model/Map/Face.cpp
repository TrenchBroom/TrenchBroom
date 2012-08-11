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
#include "Model/Assets/Texture.h"
#include "Model/Map/Brush.h"
#include "Model/Map/MapExceptions.h"
#include "Renderer/Vbo.h"

namespace TrenchBroom {
    namespace Model {
        static const Vec3f* BaseAxes[18] = {
            &Vec3f::PosZ, &Vec3f::PosX, &Vec3f::NegY,
            &Vec3f::NegZ, &Vec3f::PosX, &Vec3f::NegY,
            &Vec3f::PosX, &Vec3f::PosY, &Vec3f::NegZ,
            &Vec3f::NegX, &Vec3f::PosY, &Vec3f::NegZ,
            &Vec3f::PosY, &Vec3f::PosX, &Vec3f::NegZ,
            &Vec3f::NegY, &Vec3f::PosX, &Vec3f::NegZ};
        
        void Face::texAxesAndIndices(const Vec3f& faceNormal, Vec3f& xAxis, Vec3f& yAxis, int& planeNormIndex, int& faceNormIndex) const {
            int bestIndex = 0;
            float bestDot = -1;
            for (unsigned int i = 0; i < 6; i++) {
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
            
            Quat rot(rotation * Math::Pi / 180, *BaseAxes[m_texPlaneNormIndex]);
            m_texAxisX = rot * m_texAxisX;
            m_texAxisY = rot * m_texAxisY;
            m_scaledTexAxisX = m_texAxisX / xScale;
            m_scaledTexAxisY = m_texAxisY / yScale;
            
            texAxesValid = true;
        }
        
        
        void Face::compensateTransformation(const Mat4f& transformation) {
            if (!texAxesValid)
                validateTexAxes(boundary.normal);
            
            Vec3f newTexAxisX, newTexAxisY, newFaceNorm, newCenter, newBaseAxisX, newBaseAxisY, offset, cross;
            Vec2f curCenterTexCoords, newCenterTexCoords;
            Plane plane;
            Vec3f curCenter;
            int newPlaneNormIndex, newFaceNormIndex;
            float radX, radY, rad;
            
            // calculate the current texture coordinates of the face's center
            curCenter = centerOfVertices(side->vertices);
            curCenterTexCoords.x = (curCenter | m_scaledTexAxisX) + xOffset;
            curCenterTexCoords.y = (curCenter | m_scaledTexAxisY) + yOffset;
            
            // invert the scale of the current texture axes
            newTexAxisX = m_texAxisX * xScale;
            newTexAxisY = m_texAxisY * yScale;
            
            // project the inversely scaled texture axes onto the boundary plane
            plane.distance = 0;
            plane.normal = boundary.normal;
            if (BaseAxes[m_texPlaneNormIndex]->x != 0) {
                newTexAxisX.x = plane.x(newTexAxisX.y, newTexAxisX.z);
                newTexAxisY.x = plane.x(newTexAxisY.y, newTexAxisY.z);
            } else if (BaseAxes[m_texPlaneNormIndex]->y != 0) {
                newTexAxisX.y = plane.y(newTexAxisX.x, newTexAxisX.z);
                newTexAxisY.y = plane.y(newTexAxisY.x, newTexAxisY.z);
            } else {
                newTexAxisX.z = plane.z(newTexAxisX.x, newTexAxisX.y);
                newTexAxisY.z = plane.z(newTexAxisY.x, newTexAxisY.y);
            }
            
            // apply the transformation
            newTexAxisX = transformation * newTexAxisX;
            newTexAxisY = transformation * newTexAxisY;
            newFaceNorm = transformation * boundary.normal;
            offset = transformation * Vec3f::Null;
            newCenter = transformation * curCenter;
            
            // correct the directional vectors by the translational part of the transformation
            newTexAxisX -= offset;
            newTexAxisY -= offset;
            newFaceNorm -= offset;
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (newFaceNorm.equals(boundary.normal, 0.001f))
                newFaceNorm = boundary.normal;
            
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
            xScale = newTexAxisX.length();
            yScale = newTexAxisY.length();
            
            // normalize the transformed texture axes
            newTexAxisX /= xScale;
            newTexAxisY /= yScale;
            
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
            rotation = rad * 180 / Math::Pi;
            
            // apply the rotation to the new base axes
            Quat rot(rad, *BaseAxes[newPlaneNormIndex]);
            newBaseAxisX = rot * newBaseAxisX;
            newBaseAxisY = rot * newBaseAxisY;
            
            // the sign of the scaling factors depends on the angle between the new base axis and the new texture axis
            if ((newBaseAxisX | newTexAxisX) < 0)
                xScale *= -1;
            if ((newBaseAxisY | newTexAxisY) < 0)
                yScale *= -1;
            
            // correct rounding errors
            xScale = Math::fcorrect(xScale);
            yScale = Math::fcorrect(yScale);
            rotation = Math::fcorrect(rotation);

            validateTexAxes(newFaceNorm);
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            newCenterTexCoords.x = (newCenter | m_scaledTexAxisX);
            newCenterTexCoords.y = (newCenter | m_scaledTexAxisY);
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            xOffset = curCenterTexCoords.x - newCenterTexCoords.x;
            yOffset = curCenterTexCoords.y - newCenterTexCoords.y;
            
            if (texture != NULL) {
                xOffset -= static_cast<int>(Math::fround(xOffset / static_cast<float>(texture->width))) * static_cast<int>(texture->width);
                yOffset -= static_cast<int>(Math::fround(yOffset / static_cast<float>(texture->height))) * static_cast<int>(texture->height);
            }

            // correct rounding errors
            xOffset = Math::fcorrect(xOffset);
            yOffset = Math::fcorrect(yOffset);
        }
        
        void Face::validateCoords() {
            assert(side != NULL);
            
            if (!texAxesValid)
                validateTexAxes(boundary.normal);

            EAxis axis = boundary.normal.firstComponent();
            int width = texture != NULL ? texture->width : 1;
            int height = texture != NULL ? texture->height : 1;
            
            size_t vertexCount = side->vertices.size();
            m_gridCoords.resize(vertexCount);
            m_texCoords.resize(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) {
                const Vec3f& vertex = side->vertices[i]->position;

                m_texCoords[i].x = ((vertex | m_scaledTexAxisX) + xOffset) / width,
                m_texCoords[i].y = ((vertex | m_scaledTexAxisY) + yOffset) / height;

                switch (axis) {
                    case TB_AX_X:
                        m_gridCoords[i].x = (vertex.y + 0.5f) / 256.0f;
                        m_gridCoords[i].y = (vertex.z + 0.5f) / 256.0f;
                        break;
                    case TB_AX_Y:
                        m_gridCoords[i].x = (vertex.x + 0.5f) / 256.0f;
                        m_gridCoords[i].y = (vertex.z + 0.5f) / 256.0f;
                        break;
                    default:
                        m_gridCoords[i].x = (vertex.x + 0.5f) / 256.0f;
                        m_gridCoords[i].y = (vertex.y + 0.5f) / 256.0f;
                        break;
                }
            }
            
            coordsValid = true;
        }

        void Face::init() {
            static int currentId = 1;
            faceId = currentId++;
            xOffset = 0.0f;
            yOffset = 0.0f;
            rotation = 0.0f;
            xScale = 1.0f;
            yScale = 1.0f;
            m_brush = NULL;
            texture = NULL;
            filePosition = -1;
            m_selected = false;
            texAxesValid = false;
            coordsValid = false;
            
        }
        
        void Face::setBrush(Brush* brush) {
            if (brush == m_brush)
                return;
            
            if (m_brush != NULL && m_selected)
                m_brush->selectedFaceCount--;
            m_brush = brush;
            if (m_brush != NULL && m_selected)
                m_brush->selectedFaceCount++;
        }

        void Face::setSelected(bool selected) {
            if (m_brush) {
                if (selected)
                    m_brush->selectedFaceCount++;
                else
                    m_brush->selectedFaceCount--;
            }
            
            m_selected = selected;
        }

        Face::Face(const BBox& worldBounds, const Vec3f& point1, const Vec3f& point2, const Vec3f& point3, const std::string& textureName) : worldBounds(worldBounds), textureName(textureName) {
            init();
            points[0] = point1;
            points[1] = point2;
            points[2] = point3;
            boundary.setPoints(points[0], points[1], points[2]);
        }
        
        Face::Face(const BBox& worldBounds, const Face& faceTemplate) : worldBounds(worldBounds) {
            init();
            restore(faceTemplate);
        }
        
        Face::Face(const Face& face) : 
            faceId(face.faceId), 
            boundary(face.boundary), 
            worldBounds(face.worldBounds),
            texture(face.texture),
            textureName(face.textureName),
            xOffset(static_cast<float>(face.xOffset)),
            yOffset(static_cast<float>(face.yOffset)),
            xScale(face.xScale),
            yScale(face.yScale),
            rotation(face.rotation),
            side(NULL),
            texAxesValid(false),
            coordsValid(false),
            filePosition(face.filePosition),
            m_selected(false) {
            face.getPoints(points[0], points[1], points[2]);
        }

		Face::~Face() {
			m_texPlaneNormIndex = -1;
			m_texFaceNormIndex = -1;
			m_texAxisX = Vec3f::NaN;
			m_texAxisY = Vec3f::NaN;
			m_scaledTexAxisX = Vec3f::NaN;
			m_scaledTexAxisY = Vec3f::NaN;

			m_gridCoords.clear();
			m_texCoords.clear();

			faceId *= -1;
			m_brush = NULL;
			
			for (unsigned int i = 0; i < 3; i++)
				points[i] = Vec3f::NaN;
			boundary.normal = Vec3f::NaN;
			boundary.distance = -1.0f;

			textureName = "___deleted";
			texture = NULL;
			xOffset = 0;
			yOffset = 0;
			rotation = 0.0f;
			xScale = 0.0f;
			yScale = 0.0f;
			side = NULL;
			filePosition = -1;
			m_selected = false;
			coordsValid = false;
			texAxesValid = false;
		}

        void Face::restore(const Face& faceTemplate) {
            faceTemplate.getPoints(points[0], points[1], points[2]);
            boundary = faceTemplate.boundary;
            xOffset = faceTemplate.xOffset;
            yOffset = faceTemplate.yOffset;
            rotation = faceTemplate.rotation;
            xScale = faceTemplate.xScale;
            yScale = faceTemplate.yScale;
            setTexture(faceTemplate.texture);
            texAxesValid = false;
            coordsValid = false;
			m_selected = faceTemplate.selected();
        }
        
        void Face::getPoints(Vec3f& point1, Vec3f& point2, Vec3f& point3) const {
            point1 = points[0];
            point2 = points[1];
            point3 = points[2];
        }

        void Face::updatePoints() {
            Vec3f v1, v2;
            
            float bestDot = 1;
            int best = -1;
            size_t vertexCount = side->vertices.size();
            for (unsigned int i = 0; i < vertexCount && bestDot > 0; i++) {
                points[2] = side->vertices[pred(i, vertexCount)]->position;
                points[0] = side->vertices[i]->position;
                points[1] = side->vertices[succ(i, vertexCount)]->position;
                
                v1 = (points[2] - points[0]).normalize();
                v2 = (points[1] - points[0]).normalize();
                float dot = v1 | v2;
                if (dot < bestDot) {
                    bestDot = dot;
                    best = i;
                }
            }
            
            assert(best != -1);
            
            points[2] = side->vertices[pred(best, vertexCount)]->position;
            points[0] = side->vertices[best]->position;
            points[1] = side->vertices[succ(best, vertexCount)]->position;
            
            if (!boundary.setPoints(points[0], points[1], points[2])) {
                std::stringstream msg;
                msg << "Invalid face points "
                    << points[0].x << " " << points[0].y << " " << points[0].z << "; "
                    << points[1].x << " " << points[1].y << " " << points[1].z << "; "
                    << points[2].x << " " << points[2].y << " " << points[2].z
                    << " for face with ID " << faceId;
                throw GeometryException(msg);
            }
        }
        
        Vec3f Face::center() const {
            return centerOfVertices(side->vertices);
        }
        
        const std::vector<Vec2f>& Face::gridCoords() {
            if (!coordsValid)
                validateCoords();
            return m_gridCoords;
        }
        
        const std::vector<Vec2f>& Face::texCoords() {
            if (!coordsValid)
                validateCoords();
            return m_texCoords;
        }

        void Face::setTexture(Assets::Texture* aTexture) {
            if (texture == aTexture)
                return;
            
            if (texture != NULL)
                texture->usageCount--;
            
            texture = aTexture;
            if (texture != NULL)
                textureName = texture->name;
            
            if (texture != NULL)
                texture->usageCount++;
            coordsValid = false;
        }

        void Face::setXOffset(float aXOffset) {
            if (xOffset == aXOffset)
                return;
            
            xOffset = aXOffset;
            coordsValid = false;
        }
        
        void Face::setYOffset(float aYOffset) {
            if (yOffset == aYOffset)
                return;
            
            yOffset = aYOffset;
            coordsValid = false;
        }
        
        void Face::setRotation(float aRotation) {
            if (rotation == aRotation)
                return;
            
            rotation = aRotation;
            texAxesValid = false;
            coordsValid = false;
        }
        
        void Face::setXScale(float aXScale) {
            if (xScale == aXScale)
                return;
            
            xScale = aXScale;
            texAxesValid = false;
            coordsValid = false;
        }
        
        void Face::setYScale(float aYScale) {
            if (yScale == aYScale)
                return;
            
            yScale = aYScale;
            texAxesValid = false;
            coordsValid = false;
        }

        void Face::translateOffsets(float delta, Vec3f dir) {
            if (!texAxesValid)
                validateTexAxes(boundary.normal);
            
            float dotX = dir | m_scaledTexAxisX;
            float dotY = dir | m_scaledTexAxisY;
            
            if (fabsf(dotX) >= fabsf(dotY)) {
                if (dotX >= 0)
                    xOffset -= delta;
                else
                    xOffset += delta;
            } else {
                if (dotY >= 0)
                    yOffset -= delta;
                else
                    yOffset += delta;
            }
            
            coordsValid = false;
        }
        
        void Face::rotateTexture(float angle) {
            if (!texAxesValid)
                validateTexAxes(boundary.normal);
            
            if (m_texPlaneNormIndex == m_texFaceNormIndex)
                rotation += angle;
            else
                rotation -= angle;
            texAxesValid = false;
            coordsValid = false;
        }
        
        void Face::translate(Vec3f delta, bool lockTexture) {
            if (lockTexture)
                compensateTransformation(Mat4f::Identity.translate(delta));
            
            boundary = boundary.translate(delta);
            for (unsigned int i = 0; i < 3; i++)
                points[i] += delta;
            
            texAxesValid = false;
            coordsValid = false;
        }
        
        void Face::rotate90(EAxis axis, Vec3f center, bool clockwise, bool lockTexture) {
            if (lockTexture) {
                Mat4f t = Mat4f::Identity.translate(center);
                if (axis == TB_AX_X) t *= clockwise ? Mat4f::Rot90XCW : Mat4f::Rot90XCCW;
                else if (axis == TB_AX_Y) t *= clockwise ? Mat4f::Rot90YCW : Mat4f::Rot90YCCW;
                else t *= clockwise ? Mat4f::Rot90ZCW : Mat4f::Rot90ZCCW;
                t.translate(center * -1);
                compensateTransformation(t);
            }
            
            boundary = boundary.rotate90(axis, center, clockwise);
            for (unsigned int i = 0; i < 3; i++)
                points[i] = points[i].rotate90(axis, center, clockwise);
            
            texAxesValid = false;
            coordsValid = false;
        }
        
        void Face::rotate(Quat rotation, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t = Mat4f::Identity.translate(center).rotate(rotation).translate(center * -1);
                compensateTransformation(t);
            }
            
            boundary = boundary.rotate(rotation, center);
            
            for (unsigned int i = 0; i < 3; i++)
                points[i] = rotation * (points[i] - center) + center;
            
            texAxesValid = false;
            coordsValid = false;
        }
        
        void Face::flip(EAxis axis, Vec3f center, bool lockTexture) {
            if (lockTexture) {
                Mat4f t;
                Vec3f d;
                switch (axis) {
                    case TB_AX_X:
                        d = Vec3f(center.x, 0, 0);
                        t = Mat4f::Identity.translate(d) * Mat4f::MirX * Mat4f::Identity.translate(d * -1);
                        break;
                    case TB_AX_Y:
                        d = Vec3f(0, center.y, 0);
                        t = Mat4f::Identity.translate(d) * Mat4f::MirY * Mat4f::Identity.translate(d * -1);
                        break;
                    case TB_AX_Z:
                        d = Vec3f(0, 0, center.z);
                        t = Mat4f::Identity.translate(d) * Mat4f::MirZ * Mat4f::Identity.translate(d * -1);
                        break;
                }
                compensateTransformation(t);
            }
            
            boundary = boundary.flip(axis, center);
            for (unsigned int i = 0; i < 3; i++)
                points[i] = points[i].flip(axis, center);
            
            Vec3f t = points[1];
            points[1] = points[2];
            points[2] = t;
            texAxesValid = false;
            coordsValid = false;
        }
        
        void Face::move(float dist, bool lockTexture) {
            boundary.distance += dist;
            Vec3f delta = boundary.normal * dist;
            for (unsigned int i = 0; i < 3; i++)
                points[i] += delta;
            
            texAxesValid = false; 
            coordsValid = false;
        }
    }
}