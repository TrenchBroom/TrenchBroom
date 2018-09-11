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

#include "TrenchBroom.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"

#include <vecmath/vec.h>
#include <vecmath/plane.h>

namespace TrenchBroom {
    namespace Model {
        const vm::vec3 ParaxialTexCoordSystem::BaseAxes[] = {
            vm::vec3( 0.0,  0.0,  1.0), vm::vec3( 1.0,  0.0,  0.0), vm::vec3( 0.0, -1.0,  0.0),
            vm::vec3( 0.0,  0.0, -1.0), vm::vec3( 1.0,  0.0,  0.0), vm::vec3( 0.0, -1.0,  0.0),
            vm::vec3( 1.0,  0.0,  0.0), vm::vec3( 0.0,  1.0,  0.0), vm::vec3( 0.0,  0.0, -1.0),
            vm::vec3(-1.0,  0.0,  0.0), vm::vec3( 0.0,  1.0,  0.0), vm::vec3( 0.0,  0.0, -1.0),
            vm::vec3( 0.0,  1.0,  0.0), vm::vec3( 1.0,  0.0,  0.0), vm::vec3( 0.0,  0.0, -1.0),
            vm::vec3( 0.0, -1.0,  0.0), vm::vec3( 1.0,  0.0,  0.0), vm::vec3( 0.0,  0.0, -1.0),
        };

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const BrushFaceAttributes& attribs) :
        m_index(0) {
            resetCache(point0, point1, point2, attribs);
        }

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const vm::vec3& normal, const BrushFaceAttributes& attribs) :
        m_index(0) {
            setRotation(normal, 0.0f, attribs.rotation());
        }
        
        size_t ParaxialTexCoordSystem::planeNormalIndex(const vm::vec3& normal) {
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
        
        void ParaxialTexCoordSystem::axes(const size_t index, vm::vec3& xAxis, vm::vec3& yAxis) {
            vm::vec3 temp;
            axes(index, xAxis, yAxis, temp);
        }
        
        void ParaxialTexCoordSystem::axes(size_t index, vm::vec3& xAxis, vm::vec3& yAxis, vm::vec3& projectionAxis) {
            xAxis = BaseAxes[index * 3 + 1];
            yAxis = BaseAxes[index * 3 + 2];
            projectionAxis = BaseAxes[(index / 2) * 6];
        }
        
        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const size_t index, const vm::vec3& xAxis, const vm::vec3& yAxis) :
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

        vm::vec3 ParaxialTexCoordSystem::getXAxis() const {
            return m_xAxis;
        }
        
        vm::vec3 ParaxialTexCoordSystem::getYAxis() const {
            return m_yAxis;
        }
        
        vm::vec3 ParaxialTexCoordSystem::getZAxis() const {
            return BaseAxes[m_index * 3 + 0];
        }

        void ParaxialTexCoordSystem::doResetCache(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const BrushFaceAttributes& attribs) {
            const vm::vec3 normal = normalize(cross(point2 - point0, point1 - point0));
            setRotation(normal, 0.0f, attribs.rotation());
        }

        void ParaxialTexCoordSystem::doResetTextureAxes(const vm::vec3& normal) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParaxial(const vm::vec3& normal, const float angle) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParallel(const vm::vec3& normal, const float angle) {}

        bool ParaxialTexCoordSystem::isRotationInverted(const vm::vec3& normal) const {
            const size_t index = planeNormalIndex(normal);
            return index % 2 == 0;
        }

        vm::vec2f ParaxialTexCoordSystem::doGetTexCoords(const vm::vec3& point, const BrushFaceAttributes& attribs) const {
            return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / attribs.textureSize();
        }
        
        void ParaxialTexCoordSystem::doSetRotation(const vm::vec3& normal, const float oldAngle, const float newAngle) {
            m_index = planeNormalIndex(normal);
            axes(m_index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, vm::radians(newAngle), m_index);
        }

        void ParaxialTexCoordSystem::doTransform(const vm::plane3& oldBoundary, const vm::plane3& newBoundary, const vm::mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vm::vec3& oldInvariant) {
            const vm::vec3 offset     = transformation * vm::vec3::zero;
            const vm::vec3& oldNormal = oldBoundary.normal;
                  vm::vec3 newNormal  = newBoundary.normal;
            assert(isUnit(newNormal));
            
            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (isEqual(newNormal, oldNormal, 0.01)) {
                newNormal = oldNormal;
            }

            if (!lockTexture || attribs.xScale() == 0.0f || attribs.yScale() == 0.0f) {
                setRotation(newNormal, attribs.rotation(), attribs.rotation());
                return;
            }
            
            // calculate the current texture coordinates of the origin
            const vm::vec2f oldInvariantTexCoords = computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();

            // project the texture axes onto the boundary plane along the texture Z axis
            const vm::vec2 scale(attribs.scale());
            const vm::vec3 boundaryOffset     = oldBoundary.projectPoint(vm::vec3::zero, getZAxis());
            const vm::vec3 oldXAxisOnBoundary = oldBoundary.projectPoint(m_xAxis * scale.x(), getZAxis()) - boundaryOffset;
            const vm::vec3 oldYAxisOnBoundary = oldBoundary.projectPoint(m_yAxis * scale.y(), getZAxis()) - boundaryOffset;

            // transform the projected texture axes and compensate the translational component
            const vm::vec3 transformedXAxis = transformation * oldXAxisOnBoundary - offset;
            const vm::vec3 transformedYAxis = transformation * oldYAxisOnBoundary - offset;
            
            const vm::vec2f textureSize = attribs.textureSize();
            const bool preferX = textureSize.x() >= textureSize.y();

            // obtain the new texture plane norm and the new base texture axes
            vm::vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
            const size_t newIndex = planeNormalIndex(newNormal);
            axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);

            const vm::plane3 newTexturePlane(0.0, newProjectionAxis);
            
            // project the transformed texture axes onto the new texture projection plane
            const vm::vec3 projectedTransformedXAxis = newTexturePlane.projectPoint(transformedXAxis);
            const vm::vec3 projectedTransformedYAxis = newTexturePlane.projectPoint(transformedYAxis);
            assert(!isNaN(projectedTransformedXAxis) &&
                   !isNaN(projectedTransformedYAxis));

            const vm::vec3 normalizedXAxis = normalize(projectedTransformedXAxis);
            const vm::vec3 normalizedYAxis = normalize(projectedTransformedYAxis);
            
            // determine the rotation angle from the dot product of the new base axes and the transformed, projected and normalized texture axes
            float cosX = static_cast<float>(dot(newBaseXAxis, normalizedXAxis));
            float cosY = static_cast<float>(dot(newBaseYAxis, normalizedYAxis));
            assert(!vm::isNan(cosX));
            assert(!vm::isNan(cosY));

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
            
            const float newRotation = vm::correct(vm::normalizeDegrees(vm::degrees(rad)), 4);
            doSetRotation(newNormal, newRotation, newRotation);
            
            // finally compute the scaling factors
            vm::vec2f newScale = correct(vm::vec2f(length(projectedTransformedXAxis), length(projectedTransformedYAxis)), 4);

            // the sign of the scaling factors depends on the angle between the new texture axis and the projected transformed axis
            if (dot(m_xAxis, normalizedXAxis) < 0.0)
                newScale[0] *= -1.0f;
            if (dot(m_yAxis, normalizedYAxis) < 0.0)
                newScale[1] *= -1.0f;
            
            // compute the parameters of the transformed texture coordinate system
            const vm::vec3 newInvariant = transformation * oldInvariant;

            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const vm::vec2f newInvariantTexCoords = computeTexCoords(newInvariant, newScale);
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordiknates of the center
            const vm::vec2f newOffset = correct(attribs.modOffset(oldInvariantTexCoords - newInvariantTexCoords), 4);
            
            assert(!isNaN(newOffset));
            assert(!isNaN(newScale));
            assert(!vm::isNan(newRotation));
            assert(!vm::isZero(newScale.x()));
            assert(!vm::isZero(newScale.y()));
            
            attribs.setOffset(newOffset);
            attribs.setScale(newScale);
            attribs.setRotation(newRotation);
        }

        void ParaxialTexCoordSystem::doUpdateNormalWithProjection(const vm::vec3& oldNormal, const vm::vec3& newNormal, const BrushFaceAttributes& attribs) {
            setRotation(newNormal, attribs.rotation(), attribs.rotation());
        }

        void ParaxialTexCoordSystem::doUpdateNormalWithRotation(const vm::vec3& oldNormal, const vm::vec3& newNormal, const BrushFaceAttributes& attribs) {
            // not supported; fall back to doUpdateNormalWithProjection
            doUpdateNormalWithProjection(oldNormal, newNormal, attribs);
        }
        
        void ParaxialTexCoordSystem::doShearTexture(const vm::vec3& normal, const vm::vec2f& factors) {
            // not supported
        }

        float ParaxialTexCoordSystem::doMeasureAngle(const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const {
            const vm::vec3& zAxis = vm::vec3::pos_z; //m_index == 5 ? vm::vec3::neg_z : 	vm::vec3::pos_z;
            const vm::quat3 rot(zAxis, -vm::radians(currentAngle));
            const vm::vec3 vec = rot * vm::vec3(point - center);

            const FloatType angleInRadians = vm::C::twoPi() - angleBetween(normalize(vec), vm::vec3::pos_x, zAxis);
            return static_cast<float>(vm::degrees(angleInRadians));
        }

        void ParaxialTexCoordSystem::rotateAxes(vm::vec3& xAxis, vm::vec3& yAxis, const FloatType angleInRadians, const size_t planeNormIndex) const {
            const vm::vec3 rotAxis = cross(BaseAxes[planeNormIndex * 3 + 2], BaseAxes[planeNormIndex * 3 + 1]);
            const vm::quat3 rot(rotAxis, angleInRadians);
            xAxis = correct(rot * xAxis);
            yAxis = correct(rot * yAxis);
        }
    }
}
