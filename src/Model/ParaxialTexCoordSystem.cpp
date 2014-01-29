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
            doUpdate(normal, 0.0f);
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
        
        TexCoordSystem* ParaxialTexCoordSystem::doClone() const {
            return new ParaxialTexCoordSystem(*this);
        }

        const Vec3& ParaxialTexCoordSystem::getXAxis() const {
            return m_xAxis;
        }
        
        const Vec3& ParaxialTexCoordSystem::getYAxis() const {
            return m_yAxis;
        }
        
        bool ParaxialTexCoordSystem::isRotationInverted(const Vec3& normal) const {
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

        Vec2f ParaxialTexCoordSystem::doGetTexCoords(const Vec3& point, const BrushFaceAttribs& attribs, const Assets::Texture* texture) const {
            const size_t textureWidth = texture == NULL ? 1 : texture->width();
            const size_t textureHeight = texture == NULL ? 1 : texture->height();
            const float safeXScale = attribs.xScale() == 0.0f ? 1.0f : attribs.xScale();
            const float safeYScale = attribs.yScale() == 0.0f ? 1.0f : attribs.yScale();
            const float x = static_cast<float>((point.dot(m_xAxis / safeXScale) + attribs.xOffset()) / textureWidth);
            const float y = static_cast<float>((point.dot(m_yAxis / safeYScale) + attribs.yOffset()) / textureHeight);
            return Vec2f(x, y);
        }
        
        void ParaxialTexCoordSystem::doUpdate(const Vec3& normal, const BrushFaceAttribs& attribs) {
            doUpdate(normal, attribs.rotation());
        }
        
        void ParaxialTexCoordSystem::doUpdate(const Vec3& normal, float rotation) {
            const size_t index = planeNormalIndex(normal);
            axes(index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, Math::radians(rotation), index);
        }
        
        void ParaxialTexCoordSystem::doCompensate(const Vec3& normal, const Vec3& center, const Mat4x4& transformation, BrushFaceAttribs& attribs) {
            // calculate the current texture coordinates of the face's center
            const Vec2f curCenterTexCoords(center.dot(safeScaleAxis(m_xAxis, attribs.xScale())) + attribs.xOffset(),
                                           center.dot(safeScaleAxis(m_yAxis, attribs.yScale())) + attribs.yOffset());
            
            // compute the parameters of the transformed texture coordinate system
            const Vec3 offset = transformation * Vec3::Null;
            const Vec3 newCenter = transformation * center;
            
            // compensate the translational part of the transformation for the directional vectors
            Vec3 newXAxis = transformAxis(normal, m_xAxis * attribs.xScale(), transformation) - offset;
            Vec3 newYAxis = transformAxis(normal, m_yAxis * attribs.yScale(), transformation) - offset;
            Vec3 newNormal = transformation * normal - offset;
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (newNormal.equals(normal, 0.01))
                newNormal = normal;
            
            // obtain the new texture plane norm and the new base texture axes
            Vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
            const size_t newIndex = planeNormalIndex(newNormal);
            axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);
            
            // project the transformed texture axes onto the new texture projection plane
            newXAxis = project(newProjectionAxis, newXAxis);
            newYAxis = project(newProjectionAxis, newYAxis);
            
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
            
            doUpdate(newNormal, newRotation);
            
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

        Vec3 ParaxialTexCoordSystem::transformAxis(const Vec3& normal, const Vec3& axis, const Mat4x4& transformation) const {
            return transformation * project(normal, axis);
        }

        void ParaxialTexCoordSystem::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angle, const size_t planeNormIndex) const {
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            const Quat3 rot(BaseAxes[(planeNormIndex / 2) * 6], planeNormIndex == 4 ? -angle : angle);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }

        Vec3 ParaxialTexCoordSystem::safeScaleAxis(const Vec3& axis, const float factor) const {
            return axis / (factor == 0 ? 1.0f : factor);
        }
    }
}
