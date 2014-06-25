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
            projectionAxis = BaseAxes[(index / 2) * 6];
        }
        
        TexCoordSystem* ParaxialTexCoordSystem::doClone() const {
            return new ParaxialTexCoordSystem(*this);
        }

        Vec3 ParaxialTexCoordSystem::getXAxis() const {
            return m_xAxis;
        }
        
        Vec3 ParaxialTexCoordSystem::getYAxis() const {
            return m_yAxis;
        }
        
        Vec3 ParaxialTexCoordSystem::getZAxis() const {
            return BaseAxes[m_index * 3 + 0];
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

        Vec2f ParaxialTexCoordSystem::doGetTexCoords(const Vec3& point, const BrushFaceAttribs& attribs) const {
            const Assets::Texture* texture = attribs.texture();
            const size_t textureWidth = texture == NULL ? 1 : texture->width();
            const size_t textureHeight = texture == NULL ? 1 : texture->height();
            const float x = static_cast<float>((point.dot(xAxis() / safeScale(attribs.xScale())) + attribs.xOffset()) / textureWidth);
            const float y = static_cast<float>((point.dot(xAxis() / safeScale(attribs.xScale())) + attribs.yOffset()) / textureHeight);
            return Vec2f(x, y);
        }
        
        void ParaxialTexCoordSystem::doUpdate(const Vec3& normal, const BrushFaceAttribs& attribs) {
            doUpdate(normal, attribs.rotation());
        }
        
        void ParaxialTexCoordSystem::doUpdate(const Vec3& normal, float rotation) {
            m_index = planeNormalIndex(normal);
            axes(m_index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, Math::radians(rotation), m_index);
        }
        
        void ParaxialTexCoordSystem::doCompensate(const Vec3& oldNormal, const Vec3& oldCenter, const Mat4x4& transformation, BrushFaceAttribs& attribs) {
            // calculate the current texture coordinates of the face's center
            const Vec2f curCenterTexCoords = Vec2f(static_cast<float>(oldCenter.dot(safeScaleAxis(m_xAxis, attribs.xScale()))),
                                                   static_cast<float>(oldCenter.dot(safeScaleAxis(m_yAxis, attribs.yScale())))) +
                                             attribs.offset();
            
            // compute the parameters of the transformed texture coordinate system
            const Vec3 offset = transformation * Vec3::Null;
            const Vec3 newCenter = transformation * oldCenter;
            
            const Mat4x4 toPlane = planeProjectionMatrix(0.0, oldNormal, crossed(m_xAxis, m_yAxis).normalized());
            const Mat4x4 fromPlane = invertedMatrix(toPlane);
            const Mat4x4 projectToPlane = fromPlane * Mat4x4::ZerZ * toPlane;
            
            // compensate the translational part of the transformation for the directional vectors
            const Vec3 newXAxisOnPlane = transformation * projectToPlane * safeScaleAxis(m_xAxis, attribs.xScale()) - offset;
            const Vec3 newYAxisOnPlane = transformation * projectToPlane * safeScaleAxis(m_yAxis, attribs.yScale()) - offset;
                  Vec3 newNormal       = transformation * oldNormal - offset;
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (newNormal.equals(oldNormal, 0.01))
                newNormal = oldNormal;
            
            // obtain the new texture plane norm and the new base texture axes
            Vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
            const size_t newIndex = planeNormalIndex(newNormal);
            axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);
            
            // project the transformed texture axes onto the new texture projection plane
            const Mat4x4 toTexPlane = planeProjectionMatrix(0.0, newProjectionAxis);
            const Mat4x4 fromTexPlane = invertedMatrix(toTexPlane);
            const Mat4x4 projectToTexPlane = fromTexPlane * Mat4x4::ZerZ * toTexPlane;
            Vec3 newXAxis = projectToTexPlane * newXAxisOnPlane;
            Vec3 newYAxis = projectToTexPlane * newYAxisOnPlane;

            assert(!newXAxis.nan() && !newYAxis.nan());
            
            // the new scaling factors are the lengths of the transformed texture axes
            float newXScale = static_cast<float>(newXAxis.length());
            float newYScale = static_cast<float>(newYAxis.length());
            
            // normalize the transformed texture axes
            newXAxis /= newXScale;
            newYAxis /= newYScale;
            
            // WARNING: the texture plane norm is not the rotation axis of the texture (it's always the absolute axis)
            
            // determine the rotation angle from the dot product of the new base axes and the transformed texture axes
            float cosX = static_cast<float>(newBaseXAxis.dot(newXAxis));
            assert(!Math::isnan(cosX));
            float radX = std::acos(cosX);
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
            const Vec2f newCenterTexCoords(static_cast<float>(newCenter.dot(safeScaleAxis(m_xAxis, newXScale))),
                                           static_cast<float>(newCenter.dot(safeScaleAxis(m_yAxis, newYScale))));
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            float newXOffset = curCenterTexCoords.x() - newCenterTexCoords.x();
            float newYOffset = curCenterTexCoords.y() - newCenterTexCoords.y();
            
            const Assets::Texture* texture = attribs.texture();
            if (texture != NULL) {
                newXOffset -= Math::round(newXOffset / static_cast<float>(texture->width())) * static_cast<float>(texture->width());
                newYOffset -= Math::round(newYOffset / static_cast<float>(texture->height())) * static_cast<float>(texture->height());
            }
            
            // correct rounding errors
            newXOffset = Math::correct(newXOffset);
            newYOffset = Math::correct(newYOffset);
            
            assert(!Math::isnan(newXOffset));
            assert(!Math::isnan(newYOffset));
            assert(!Math::isnan(newRotation));
            assert(!Math::isnan(newXScale));
            assert(!Math::isnan(newYScale));
            assert(!Math::zero(newXScale));
            assert(!Math::zero(newYScale));
            
            attribs.setXOffset(newXOffset);
            attribs.setYOffset(newYOffset);
            attribs.setRotation(newRotation);
            attribs.setXScale(newXScale);
            attribs.setYScale(newYScale);
        }

        float ParaxialTexCoordSystem::doMeasureAngle(const float currentAngle, const Vec2f& center, const Vec2f& point) const {
            const Vec3& zAxis = m_index % 2 == 0 ? Vec3::PosZ : Vec3::NegZ;
            const Quat3 rot(zAxis, -Math::radians(currentAngle));
            const Vec3 vec = rot * (point - center);

            const FloatType angleInRadians = Math::C::twoPi() - angleBetween(vec.normalized(), Vec3::PosX, zAxis);
            return static_cast<float>(Math::degrees(angleInRadians));
        }

        void ParaxialTexCoordSystem::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angleInRadians, const size_t planeNormIndex) const {
            const Quat3 rot(BaseAxes[(planeNormIndex / 2) * 6], planeNormIndex == 4 ? -angleInRadians : angleInRadians);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }
    }
}
