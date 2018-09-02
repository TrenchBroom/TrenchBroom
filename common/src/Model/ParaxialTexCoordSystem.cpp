/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#include "VecMath.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        const vec3 ParaxialTexCoordSystem::BaseAxes[] = {
            vec3( 0.0,  0.0,  1.0), vec3( 1.0,  0.0,  0.0), vec3( 0.0, -1.0,  0.0),
            vec3( 0.0,  0.0, -1.0), vec3( 1.0,  0.0,  0.0), vec3( 0.0, -1.0,  0.0),
            vec3( 1.0,  0.0,  0.0), vec3( 0.0,  1.0,  0.0), vec3( 0.0,  0.0, -1.0),
            vec3(-1.0,  0.0,  0.0), vec3( 0.0,  1.0,  0.0), vec3( 0.0,  0.0, -1.0),
            vec3( 0.0,  1.0,  0.0), vec3( 1.0,  0.0,  0.0), vec3( 0.0,  0.0, -1.0),
            vec3( 0.0, -1.0,  0.0), vec3( 1.0,  0.0,  0.0), vec3( 0.0,  0.0, -1.0),
        };

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs) :
        m_index(0) {
            resetCache(point0, point1, point2, attribs);
        }

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const vec3& normal, const BrushFaceAttributes& attribs) :
        m_index(0) {
            setRotation(normal, 0.0f, attribs.rotation());
        }
        
        size_t ParaxialTexCoordSystem::planeNormalIndex(const vec3& normal) {
            size_t bestIndex = 0;
            FloatType bestDot = static_cast<FloatType>(0.0);
            for (size_t i = 0; i < 6; ++i) {
                const FloatType curDot = dot(normal, BaseAxes[i * 3]);
                if (curDot > bestDot) { // no need to use -altaxis for qbsp, but -oldaxis is necessary
                    bestDot = curDot;
                    bestIndex = i;
                }
            }
            return bestIndex;
        }
        
        void ParaxialTexCoordSystem::axes(const size_t index, vec3& xAxis, vec3& yAxis) {
            vec3 temp;
            axes(index, xAxis, yAxis, temp);
        }
        
        void ParaxialTexCoordSystem::axes(size_t index, vec3& xAxis, vec3& yAxis, vec3& projectionAxis) {
            xAxis = BaseAxes[index * 3 + 1];
            yAxis = BaseAxes[index * 3 + 2];
            projectionAxis = BaseAxes[(index / 2) * 6];
        }
        
        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const size_t index, const vec3& xAxis, const vec3& yAxis) :
        m_index(index),
        m_xAxis(xAxis),
        m_yAxis(yAxis) {}

        TexCoordSystem* ParaxialTexCoordSystem::doClone() const {
            return new ParaxialTexCoordSystem(m_index, m_xAxis, m_yAxis);
        }

        TexCoordSystemSnapshot* ParaxialTexCoordSystem::doTakeSnapshot() {
            return nullptr;
        }
        
        void ParaxialTexCoordSystem::doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) {
            ensure(false, "unsupported");
        }

        vec3 ParaxialTexCoordSystem::getXAxis() const {
            return m_xAxis;
        }
        
        vec3 ParaxialTexCoordSystem::getYAxis() const {
            return m_yAxis;
        }
        
        vec3 ParaxialTexCoordSystem::getZAxis() const {
            return BaseAxes[m_index * 3 + 0];
        }

        void ParaxialTexCoordSystem::doResetCache(const vec3& point0, const vec3& point1, const vec3& point2, const BrushFaceAttributes& attribs) {
            const vec3 normal = normalize(cross(point2 - point0, point1 - point0));
            setRotation(normal, 0.0f, attribs.rotation());
        }

        void ParaxialTexCoordSystem::doResetTextureAxes(const vec3& normal) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParaxial(const vec3& normal, const float angle) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParallel(const vec3& normal, const float angle) {}

        bool ParaxialTexCoordSystem::isRotationInverted(const vec3& normal) const {
            const size_t index = planeNormalIndex(normal);
            return index % 2 == 0;
        }

        vec2f ParaxialTexCoordSystem::doGetTexCoords(const vec3& point, const BrushFaceAttributes& attribs) const {
            return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / attribs.textureSize();
        }
        
        void ParaxialTexCoordSystem::doSetRotation(const vec3& normal, const float oldAngle, const float newAngle) {
            m_index = planeNormalIndex(normal);
            axes(m_index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, Math::radians(newAngle), m_index);
        }

        void ParaxialTexCoordSystem::doTransform(const plane3& oldBoundary, const plane3& newBoundary, const mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vec3& oldInvariant) {
            const vec3 offset     = transformation * vec3::zero;
            const vec3& oldNormal = oldBoundary.normal;
                  vec3 newNormal  = newBoundary.normal;
            assert(isUnit(newNormal));
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (equal(newNormal, oldNormal, 0.01)) {
                newNormal = oldNormal;
            }

            if (!lockTexture || attribs.xScale() == 0.0f || attribs.yScale() == 0.0f) {
                setRotation(newNormal, attribs.rotation(), attribs.rotation());
                return;
            }
            
            // calculate the current texture coordinates of the origin
            const vec2f oldInvariantTexCoords = computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();

            // project the texture axes onto the boundary plane along the texture Z axis
            const vec2 scale(attribs.scale());
            const vec3 boundaryOffset     = oldBoundary.projectPoint(vec3::zero, getZAxis());
            const vec3 oldXAxisOnBoundary = oldBoundary.projectPoint(m_xAxis * scale.x(), getZAxis()) - boundaryOffset;
            const vec3 oldYAxisOnBoundary = oldBoundary.projectPoint(m_yAxis * scale.y(), getZAxis()) - boundaryOffset;

            // transform the projected texture axes and compensate the translational component
            const vec3 transformedXAxis = transformation * oldXAxisOnBoundary - offset;
            const vec3 transformedYAxis = transformation * oldYAxisOnBoundary - offset;
            
            const vec2f textureSize = attribs.textureSize();
            const bool preferX = textureSize.x() >= textureSize.y();

            // obtain the new texture plane norm and the new base texture axes
            vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
            const size_t newIndex = planeNormalIndex(newNormal);
            axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);

            const plane3 newTexturePlane(0.0, newProjectionAxis);
            
            // project the transformed texture axes onto the new texture projection plane
            const vec3 projectedTransformedXAxis = newTexturePlane.projectPoint(transformedXAxis);
            const vec3 projectedTransformedYAxis = newTexturePlane.projectPoint(transformedYAxis);
            assert(!isNaN(projectedTransformedXAxis) &&
                   !isNaN(projectedTransformedYAxis));

            const vec3 normalizedXAxis = normalize(projectedTransformedXAxis);
            const vec3 normalizedYAxis = normalize(projectedTransformedYAxis);
            
            // determine the rotation angle from the dot product of the new base axes and the transformed, projected and normalized texture axes
            float cosX = static_cast<float>(dot(newBaseXAxis, normalizedXAxis));
            float cosY = static_cast<float>(dot(newBaseYAxis, normalizedYAxis));
            assert(!Math::isnan(cosX));
            assert(!Math::isnan(cosY));

            float radX = std::acos(cosX);
            if (dot(cross(newBaseXAxis, normalizedXAxis), newProjectionAxis) < 0.0)
                radX *= -1.0f;
            
            float radY = std::acos(cosY);
            if (dot(cross(newBaseYAxis, normalizedYAxis), newProjectionAxis) < 0.0)
                radY *= -1.0f;
            
            // TODO: be smarter about choosing between the X and Y axis rotations - sometimes either
            // one can be better
            float rad = preferX ? radX : radY;
            
            // for some reason, when the texture plane normal is the Y axis, we must rotation clockwise
            const size_t planeNormIndex = (newIndex / 2) * 6;
            if (planeNormIndex == 12)
                rad *= -1.0f;
            
            const float newRotation = Math::correct(Math::normalizeDegrees(Math::degrees(rad)), 4);
            doSetRotation(newNormal, newRotation, newRotation);
            
            // finally compute the scaling factors
            vec2f newScale = correct(vec2f(length(projectedTransformedXAxis), length(projectedTransformedYAxis)), 4);

            // the sign of the scaling factors depends on the angle between the new texture axis and the projected transformed axis
            if (dot(m_xAxis, normalizedXAxis) < 0.0)
                newScale[0] *= -1.0f;
            if (dot(m_yAxis, normalizedYAxis) < 0.0)
                newScale[1] *= -1.0f;
            
            // compute the parameters of the transformed texture coordinate system
            const vec3 newInvariant = transformation * oldInvariant;

            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const vec2f newInvariantTexCoords = computeTexCoords(newInvariant, newScale);
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordiknates of the center
            const vec2f newOffset = correct(attribs.modOffset(oldInvariantTexCoords - newInvariantTexCoords), 4);
            
            assert(!isNaN(newOffset));
            assert(!isNaN(newScale));
            assert(!Math::isnan(newRotation));
            assert(!Math::zero(newScale.x()));
            assert(!Math::zero(newScale.y()));
            
            attribs.setOffset(newOffset);
            attribs.setScale(newScale);
            attribs.setRotation(newRotation);
        }

        void ParaxialTexCoordSystem::doUpdateNormalWithProjection(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) {
            setRotation(newNormal, attribs.rotation(), attribs.rotation());
        }

        void ParaxialTexCoordSystem::doUpdateNormalWithRotation(const vec3& oldNormal, const vec3& newNormal, const BrushFaceAttributes& attribs) {
            // not supported; fall back to doUpdateNormalWithProjection
            doUpdateNormalWithProjection(oldNormal, newNormal, attribs);
        }
        
        void ParaxialTexCoordSystem::doShearTexture(const vec3& normal, const vec2f& factors) {
            // not supported
        }

        float ParaxialTexCoordSystem::doMeasureAngle(const float currentAngle, const vec2f& center, const vec2f& point) const {
            const vec3& zAxis = vec3::pos_z; //m_index == 5 ? vec3::neg_z : 	vec3::pos_z;
            const quat3 rot(zAxis, -Math::radians(currentAngle));
            const vec3 vec = rot * vec3(point - center);

            const FloatType angleInRadians = Math::C::twoPi() - angleBetween(normalize(vec), vec3::pos_x, zAxis);
            return static_cast<float>(Math::degrees(angleInRadians));
        }

        void ParaxialTexCoordSystem::rotateAxes(vec3& xAxis, vec3& yAxis, const FloatType angleInRadians, const size_t planeNormIndex) const {
            const vec3 rotAxis = cross(BaseAxes[planeNormIndex * 3 + 2], BaseAxes[planeNormIndex * 3 + 1]);
            const quat3 rot(rotAxis, angleInRadians);
            xAxis = correct(rot * xAxis);
            yAxis = correct(rot * yAxis);
        }
    }
}
