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

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs) :
        m_index(0) {
            const Vec3 normal = crossed(point2 - point0, point1 - point0).normalized();
            setRotation(normal, 0.0f, attribs.rotation());
        }

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const Vec3& normal, const BrushFaceAttributes& attribs) :
        m_index(0) {
            setRotation(normal, 0.0f, attribs.rotation());
        }
        
        size_t ParaxialTexCoordSystem::planeNormalIndex(const Vec3& normal) {
            size_t bestIndex = 0;
            FloatType bestDot = static_cast<FloatType>(0.0);
            for (size_t i = 0; i < 6; ++i) {
                const FloatType dot = normal.dot(BaseAxes[i * 3]);
                if (dot > bestDot) { // no need to use -altaxis for qbsp, but -oldaxis is necessary
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

        TexCoordSystemSnapshot* ParaxialTexCoordSystem::doTakeSnapshot() {
            return NULL;
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
        
        void ParaxialTexCoordSystem::doResetTextureAxes(const Vec3& normal) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParaxial(const Vec3& normal, const float angle) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParallel(const Vec3& normal, const float angle) {}

        bool ParaxialTexCoordSystem::isRotationInverted(const Vec3& normal) const {
            const size_t index = planeNormalIndex(normal);
            return index % 2 == 0;
        }

        Vec2f ParaxialTexCoordSystem::doGetTexCoords(const Vec3& point, const BrushFaceAttributes& attribs) const {
            return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / attribs.textureSize();
        }
        
        void ParaxialTexCoordSystem::doSetRotation(const Vec3& normal, const float oldAngle, const float newAngle) {
            m_index = planeNormalIndex(normal);
            axes(m_index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, Math::radians(newAngle), m_index);
        }

        void ParaxialTexCoordSystem::doTransform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& oldInvariant) {
            const Vec3 offset     = transformation * Vec3::Null;
            const Vec3& oldNormal = oldBoundary.normal;
                  Vec3 newNormal  = transformation * oldNormal - offset;
            assert(Math::eq(newNormal.length(), 1.0));
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (newNormal.equals(oldNormal, 0.01))
                newNormal = oldNormal;
            
            if (!lockTexture || attribs.xScale() == 0.0f || attribs.yScale() == 0.0f) {
                setRotation(newNormal, attribs.rotation(), attribs.rotation());
                return;
            }
            
            // calculate the current texture coordinates of the origin
            const Vec2f oldInvariantTexCoords = computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();

            // project the texture axes onto the boundary plane along the texture Z axis
            const Vec3 boundaryOffset     = oldBoundary.project(Vec3::Null, getZAxis());
            const Vec3 oldXAxisOnBoundary = oldBoundary.project(m_xAxis * attribs.xScale(), getZAxis()) - boundaryOffset;
            const Vec3 oldYAxisOnBoundary = oldBoundary.project(m_yAxis * attribs.yScale(), getZAxis()) - boundaryOffset;

            // transform the projected texture axes and compensate the translational component
            const Vec3 transformedXAxis = transformation * oldXAxisOnBoundary - offset;
            const Vec3 transformedYAxis = transformation * oldYAxisOnBoundary - offset;
            
            const Vec2f textureSize = attribs.textureSize();
            const bool preferX = textureSize.x() >= textureSize.y();

            /*
            const FloatType dotX = transformedXAxis.normalized().dot(oldXAxisOnBoundary.normalized());
            const FloatType dotY = transformedYAxis.normalized().dot(oldYAxisOnBoundary.normalized());
            const bool preferX = Math::abs(dotX) < Math::abs(dotY);
            */
            
            // obtain the new texture plane norm and the new base texture axes
            Vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
            const size_t newIndex = planeNormalIndex(newNormal);
            axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);

            const Plane3 newTexturePlane(0.0, newProjectionAxis);
            
            // project the transformed texture axes onto the new texture projection plane
            const Vec3 projectedTransformedXAxis = newTexturePlane.project(transformedXAxis);
            const Vec3 projectedTransformedYAxis = newTexturePlane.project(transformedYAxis);
            assert(!projectedTransformedXAxis.nan() &&
                   !projectedTransformedYAxis.nan());

            const Vec3 normalizedXAxis = projectedTransformedXAxis.normalized();
            const Vec3 normalizedYAxis = projectedTransformedYAxis.normalized();
            
            // determine the rotation angle from the dot product of the new base axes and the transformed, projected and normalized texture axes
            float cosX = static_cast<float>(newBaseXAxis.dot(normalizedXAxis.normalized()));
            float cosY = static_cast<float>(newBaseYAxis.dot(normalizedYAxis.normalized()));
            assert(!Math::isnan(cosX));
            assert(!Math::isnan(cosY));

            float radX = std::acos(cosX);
            if (crossed(newBaseXAxis, normalizedXAxis).dot(newProjectionAxis) < 0.0)
                radX *= -1.0f;
            
            float radY = std::acos(cosY);
            if (crossed(newBaseYAxis, normalizedYAxis).dot(newProjectionAxis) < 0.0)
                radY *= -1.0f;
            
            // TODO: be smarter about choosing between the X and Y axis rotations - sometimes either
            // one can be better
            float rad = preferX ? radX : radY;
            
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            if (newIndex == 4)
                rad *= -1.0f;
            
            const float newRotation = Math::correct(Math::normalizeDegrees(Math::degrees(rad)), 4);
            doSetRotation(newNormal, newRotation, newRotation);
            
            // finally compute the scaling factors
            Vec2f newScale = Vec2f(projectedTransformedXAxis.length(),
                                   projectedTransformedYAxis.length()).corrected(4);

            // the sign of the scaling factors depends on the angle between the new texture axis and the projected transformed axis
            if (m_xAxis.dot(normalizedXAxis) < 0.0)
                newScale[0] *= -1.0f;
            if (m_yAxis.dot(normalizedYAxis) < 0.0)
                newScale[1] *= -1.0f;
            
            // compute the parameters of the transformed texture coordinate system
            const Vec3 newInvariant = transformation * oldInvariant;

            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const Vec2f newInvariantTexCoords = computeTexCoords(newInvariant, newScale);
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordiknates of the center
            const Vec2f newOffset = attribs.modOffset(oldInvariantTexCoords - newInvariantTexCoords).corrected(4);
            
            assert(!newOffset.nan());
            assert(!newScale.nan());
            assert(!Math::isnan(newRotation));
            assert(!Math::zero(newScale.x()));
            assert(!Math::zero(newScale.y()));
            
            attribs.setOffset(newOffset);
            attribs.setScale(newScale);
            attribs.setRotation(newRotation);
        }

        void ParaxialTexCoordSystem::doUpdateNormal(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) {
            setRotation(newNormal, attribs.rotation(), attribs.rotation());
        }

        void ParaxialTexCoordSystem::doShearTexture(const Vec3& normal, const Vec2f& factors) {
            // not supported
        }

        float ParaxialTexCoordSystem::doMeasureAngle(const float currentAngle, const Vec2f& center, const Vec2f& point) const {
            const Vec3& zAxis = Vec3::PosZ; //m_index == 5 ? Vec3::NegZ : 	Vec3::PosZ;
            const Quat3 rot(zAxis, -Math::radians(currentAngle));
            const Vec3 vec = rot * (point - center);

            const FloatType angleInRadians = Math::C::twoPi() - angleBetween(vec.normalized(), Vec3::PosX, zAxis);
            return static_cast<float>(Math::degrees(angleInRadians));
        }

        void ParaxialTexCoordSystem::rotateAxes(Vec3& xAxis, Vec3& yAxis, const FloatType angleInRadians, const size_t planeNormIndex) const {
            const Vec3 rotAxis = crossed(BaseAxes[planeNormIndex * 3 + 2], BaseAxes[planeNormIndex * 3 + 1]);
            const Quat3 rot(rotAxis, angleInRadians);
            xAxis = rot * xAxis;
            yAxis = rot * yAxis;
        }
    }
}
