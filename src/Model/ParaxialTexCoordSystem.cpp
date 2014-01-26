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

#include "ParaxialTexCoordSystem.h"

#include "Assets/Texture.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        const Vec3 ParaxialTexCoordSystem::BaseAxes[] = {
            Vec3( 0.0,  0.0,  1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 0.0,  0.0, -1.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0, -1.0,  0.0),
            Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3(-1.0,  0.0,  0.0), Vec3( 0.0,  1.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0,  1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
            Vec3( 0.0, -1.0,  0.0), Vec3( 1.0,  0.0,  0.0), Vec3( 0.0,  0.0, -1.0),
        };

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            const Vec3 normal = crossed(point2 - point0, point1 - point0).normalized();
            update(normal, 0.0f);
        }

        const Vec3& ParaxialTexCoordSystem::xAxis() const {
            return m_xAxis;
        }
        
        const Vec3& ParaxialTexCoordSystem::yAxis() const {
            return m_yAxis;
        }
        
        Vec3 ParaxialTexCoordSystem::projectedXAxis(const Vec3& normal) const {
            return projectAxis(normal, xAxis());
        }
        
        Vec3 ParaxialTexCoordSystem::projectedYAxis(const Vec3& normal) const {
            return projectAxis(normal, yAxis());
        }

        void ParaxialTexCoordSystem::update(const Vec3& normal, const float rotation) {
            const size_t index = planeNormalIndex(normal);
            axes(index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, Math::radians(rotation), index);
        }

        size_t ParaxialTexCoordSystem::planeNormalIndex(const Vec3& normal) {
            size_t bestIndex = 0;
            FloatType bestDot = static_cast<FloatType>(0.0);
            for (size_t i = 0; i < 6; ++i) {
                const FloatType dot = normal.dot(BaseAxes[i * 3]);
                if (dot > bestDot) { // no need to use -altaxis for qbsp
                    bestDot = dot;
                    bestIndex = i;
                }
            }
            return bestIndex;
//            return (bestIndex / 2) * 6;
        }

        void ParaxialTexCoordSystem::axes(const size_t index, Vec3& xAxis, Vec3& yAxis) {
            Vec3 temp;
            axes(index, xAxis, yAxis, temp);
        }
        
        void ParaxialTexCoordSystem::axes(size_t index, Vec3& xAxis, Vec3& yAxis, Vec3& projectionAxis) {
            xAxis = BaseAxes[index * 3 + 1];
            yAxis = BaseAxes[index * 3 + 2];
            projectionAxis = BaseAxes[index / 2 * 6];
        }

        void ParaxialTexCoordSystem::compensateTransformation(const Vec3& faceNormal, const Vec3& curCenter, const Mat4x4& transformation, BrushFaceAttribs& attribs) {
            // calculate the current texture coordinates of the face's center
            const Vec2f curCenterTexCoords(curCenter.dot(safeScaleAxis(m_xAxis, attribs.xScale())) + attribs.xOffset(),
                                           curCenter.dot(safeScaleAxis(m_yAxis, attribs.yScale())) + attribs.yOffset());
            
            // compute the parameters of the transformed texture coordinate system
            const Vec3 offset = transformation * Vec3::Null;
            const Vec3 newCenter = transformation * curCenter;
            
            // compensate the translational part of the transformation for the directional vectors
            Vec3 newXAxis = transformAxis(faceNormal, m_xAxis * attribs.xScale(), transformation) - offset;
            Vec3 newYAxis = transformAxis(faceNormal, m_yAxis * attribs.yScale(), transformation) - offset;
            Vec3 newFaceNormal = transformation * faceNormal - offset;
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (newFaceNormal.equals(faceNormal, 0.01))
                newFaceNormal = faceNormal;
            
            // obtain the new texture plane norm and the new base texture axes
            Vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
            const size_t newIndex = planeNormalIndex(newFaceNormal);
            axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);
            
            // project the transformed texture axes onto the new texture projection plane
            newXAxis = projectAxis(newProjectionAxis, newXAxis);
            newYAxis = projectAxis(newProjectionAxis, newYAxis);
            
            // the new scaling factors are the lengths of the transformed texture axes
            float newXScale = newXAxis.length();
            float newYScale = newYAxis.length();
            
            // normalize the transformed texture axes
            newXAxis /= newXScale;
            newYAxis /= newYScale;
            
            // WARNING: the texture plane norm is not the rotation axis of the texture (it's always the absolute axis)
            
            // determine the rotation angle from the dot product of the new base axes and the transformed texture axes
            float radX = acosf(newBaseXAxis.dot(newXAxis));
            if (crossed(newBaseXAxis, newXAxis).dot(newProjectionAxis) < 0.0)
                radX *= -1.0f;
            
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            float rad = radX;
            if (newIndex == 4)
                rad *= -1.0f;
            
            float newRotation = Math::degrees(rad);
            newRotation = Math::correct(newRotation);
            
            // apply the rotation to the new base axes
            rotateAxes(newBaseXAxis, newBaseYAxis, rad, newIndex);
            
            // the sign of the scaling factors depends on the angle between the new base axis and the new texture axis
            if (newBaseXAxis.dot(newXAxis) < 0.0)
                newXScale *= -1.0f;
            if (newBaseYAxis.dot(newYAxis) < 0.0)
                newYScale *= -1.0f;
            
            // correct rounding errors
            newXScale = Math::correct(newXScale);
            newYScale = Math::correct(newYScale);
            
            update(newFaceNormal, newRotation);
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const Vec2f newCenterTexCoords(newCenter.dot(safeScaleAxis(m_xAxis, newXScale)),
                                           newCenter.dot(safeScaleAxis(m_yAxis, newYScale)));
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            float newXOffset = curCenterTexCoords.x() - newCenterTexCoords.x();
            float newYOffset = curCenterTexCoords.y() - newCenterTexCoords.y();
            
            const Assets::Texture* texture = attribs.texture();
            if (texture != NULL) {
                newXOffset -= static_cast<int>(Math::round(newXOffset / static_cast<float>(texture->width()))) * static_cast<int>(texture->width());
                newYOffset -= static_cast<int>(Math::round(newYOffset / static_cast<float>(texture->height()))) * static_cast<int>(texture->height());
            }
            
            // correct rounding errors
            newXOffset = Math::correct(newXOffset);
            newYOffset = Math::correct(newYOffset);
            
            attribs.setXOffset(newXOffset);
            attribs.setYOffset(newYOffset);
            attribs.setRotation(newRotation);
            attribs.setXScale(newXScale);
            attribs.setYScale(newYScale);
        }
        
        bool ParaxialTexCoordSystem::invertRotation(const Vec3& normal) {
            const size_t index = planeNormalIndex(normal);
            switch (index) {
                case 0:
                    return true;
                case 1:
                    return false;
                case 2:
                    return true;
                case 3:
                    return false;
                case 4: // Y axis rotation is the other way around (see rotateAxes method, too)
                    return false;
                default:
                    return true;
            }
        }
        
        Vec3 ParaxialTexCoordSystem::transformAxis(const Vec3& normal, const Vec3& axis, const Mat4x4& transformation) {
            return transformation * projectAxis(normal, axis);
        }

        Vec3 ParaxialTexCoordSystem::projectAxis(const Vec3& normal, const Vec3& axis) {
            const Plane3 plane(0.0, normal);
            const size_t index = planeNormalIndex(normal);
            switch (index) {
                case 0:
                case 1: // z != 0
                    return Vec3(axis.x(), axis.y(), plane.zAt(Vec2(axis.x(), axis.y())));
                case 2:
                case 3: // x != 0
                    return Vec3(plane.xAt(Vec2(axis.y(), axis.z())), axis.y(), axis.z());
                default: // y != 0
                    return Vec3(axis.x(), plane.yAt(Vec2(axis.x(), axis.z())), axis.z());
            }
        }

        void ParaxialTexCoordSystem::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angle, const size_t planeNormIndex) {
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            const Quat3 rot(BaseAxes[(planeNormIndex / 2) * 6], planeNormIndex == 4 ? -angle : angle);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }

        Vec3 ParaxialTexCoordSystem::safeScaleAxis(const Vec3& axis, const float factor) {
            return axis / (factor == 0 ? 1.0f : factor);
        }
    }
}
