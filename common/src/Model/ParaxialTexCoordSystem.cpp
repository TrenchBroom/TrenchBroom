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

#include "Ensure.h"
#include "FloatType.h"
#include "Assets/Texture.h"
#include "Model/BrushFace.h"

#include <vecmath/vec.h>
#include <vecmath/plane.h>
#include <vecmath/quat.h>

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

        ParaxialTexCoordSystem::ParaxialTexCoordSystem(const size_t index, const vm::vec3& xAxis, const vm::vec3& yAxis) :
        m_index(index),
        m_xAxis(xAxis),
        m_yAxis(yAxis) {}

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

        std::unique_ptr<TexCoordSystem> ParaxialTexCoordSystem::doClone() const {
            return std::make_unique<ParaxialTexCoordSystem>(m_index, m_xAxis, m_yAxis);
        }

        std::unique_ptr<TexCoordSystemSnapshot> ParaxialTexCoordSystem::doTakeSnapshot() const {
            return std::unique_ptr<TexCoordSystemSnapshot>();
        }

        void ParaxialTexCoordSystem::doRestoreSnapshot(const TexCoordSystemSnapshot& /* snapshot */) {
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

        void ParaxialTexCoordSystem::doResetTextureAxes(const vm::vec3& /* normal */) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParaxial(const vm::vec3& /* normal */, const float /* angle */) {}
        void ParaxialTexCoordSystem::doResetTextureAxesToParallel(const vm::vec3& /* normal */, const float /* angle */) {}

        bool ParaxialTexCoordSystem::isRotationInverted(const vm::vec3& normal) const {
            const size_t index = planeNormalIndex(normal);
            return index % 2 == 0;
        }

        vm::vec2f ParaxialTexCoordSystem::doGetTexCoords(const vm::vec3& point, const BrushFaceAttributes& attribs, const vm::vec2f& textureSize) const {
            return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / textureSize;
        }

        void ParaxialTexCoordSystem::doSetRotation(const vm::vec3& normal, const float /* oldAngle */, const float newAngle) {
            m_index = planeNormalIndex(normal);
            axes(m_index, m_xAxis, m_yAxis);
            rotateAxes(m_xAxis, m_yAxis, vm::to_radians(static_cast<FloatType>(newAngle)), m_index);
        }

        void ParaxialTexCoordSystem::doTransform(const vm::plane3& oldBoundary, const vm::plane3& newBoundary, const vm::mat4x4& transformation, BrushFaceAttributes& attribs, const vm::vec2f& textureSize, bool lockTexture, const vm::vec3& oldInvariant) {
            const vm::vec3 offset     = transformation * vm::vec3::zero();
            const vm::vec3& oldNormal = oldBoundary.normal;
                  vm::vec3 newNormal  = newBoundary.normal;
            assert(vm::is_unit(newNormal, vm::C::almost_zero()));

            // fix some rounding errors - if the old and new texture axes are almost the same, use the old axis
            if (vm::is_equal(newNormal, oldNormal, 0.01)) {
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
            const vm::vec3 boundaryOffset     = oldBoundary.project_point(vm::vec3::zero(), getZAxis());
            const vm::vec3 oldXAxisOnBoundary = oldBoundary.project_point(m_xAxis * scale.x(), getZAxis()) - boundaryOffset;
            const vm::vec3 oldYAxisOnBoundary = oldBoundary.project_point(m_yAxis * scale.y(), getZAxis()) - boundaryOffset;

            // transform the projected texture axes and compensate the translational component
            const vm::vec3 transformedXAxis = transformation * oldXAxisOnBoundary - offset;
            const vm::vec3 transformedYAxis = transformation * oldYAxisOnBoundary - offset;

            const bool preferX = textureSize.x() >= textureSize.y();

            // obtain the new texture plane norm and the new base texture axes
            vm::vec3 newBaseXAxis, newBaseYAxis, newProjectionAxis;
            const size_t newIndex = planeNormalIndex(newNormal);
            axes(newIndex, newBaseXAxis, newBaseYAxis, newProjectionAxis);

            const vm::plane3 newTexturePlane(0.0, newProjectionAxis);

            // project the transformed texture axes onto the new texture projection plane
            const vm::vec3 projectedTransformedXAxis = newTexturePlane.project_point(transformedXAxis);
            const vm::vec3 projectedTransformedYAxis = newTexturePlane.project_point(transformedYAxis);
            assert(!vm::is_nan(projectedTransformedXAxis) &&
                   !vm::is_nan(projectedTransformedYAxis));

            const vm::vec3 normalizedXAxis = vm::normalize(projectedTransformedXAxis);
            const vm::vec3 normalizedYAxis = vm::normalize(projectedTransformedYAxis);

            // determine the rotation angle from the dot product of the new base axes and the transformed, projected and normalized texture axes
            float cosX = static_cast<float>(vm::dot(newBaseXAxis, normalizedXAxis));
            float cosY = static_cast<float>(vm::dot(newBaseYAxis, normalizedYAxis));
            assert(!vm::is_nan(cosX));
            assert(!vm::is_nan(cosY));

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

            const float newRotation = vm::correct(vm::normalize_degrees(vm::to_degrees(rad)), 4);
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
            const vm::vec2f newOffset = correct(attribs.modOffset(oldInvariantTexCoords - newInvariantTexCoords, textureSize), 4);

            assert(!vm::is_nan(newOffset));
            assert(!vm::is_nan(newScale));
            assert(!vm::is_nan(newRotation));
            assert(!vm::is_zero(newScale.x(), vm::Cf::almost_zero()));
            assert(!vm::is_zero(newScale.y(), vm::Cf::almost_zero()));

            attribs.setOffset(newOffset);
            attribs.setScale(newScale);
            attribs.setRotation(newRotation);
        }

        void ParaxialTexCoordSystem::doUpdateNormalWithProjection(const vm::vec3& newNormal, const BrushFaceAttributes& attribs) {
            setRotation(newNormal, attribs.rotation(), attribs.rotation());
        }

        void ParaxialTexCoordSystem::doUpdateNormalWithRotation(const vm::vec3& /* oldNormal */, const vm::vec3& newNormal, const BrushFaceAttributes& attribs) {
            // not supported; fall back to doUpdateNormalWithProjection
            doUpdateNormalWithProjection(newNormal, attribs);
        }

        void ParaxialTexCoordSystem::doShearTexture(const vm::vec3& /* normal */, const vm::vec2f& /* factors */) {
            // not supported
        }

        float ParaxialTexCoordSystem::doMeasureAngle(const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const {
            const auto rot = vm::quatf(vm::vec3f::pos_z(), -vm::to_radians(currentAngle));
            const auto vec = rot * vm::vec3f(point - center);

            const auto angleInRadians =
                vm::Cf::two_pi() - vm::measure_angle(vm::normalize(vec), vm::vec3f::pos_x(), vm::vec3f::pos_z());
            return vm::to_degrees(angleInRadians);
        }

        void ParaxialTexCoordSystem::rotateAxes(vm::vec3& xAxis, vm::vec3& yAxis, const FloatType angleInRadians, const size_t planeNormIndex) const {
            const vm::vec3 rotAxis = vm::cross(BaseAxes[planeNormIndex * 3 + 2], BaseAxes[planeNormIndex * 3 + 1]);
            const vm::quat3 rot(rotAxis, angleInRadians);
            xAxis = vm::correct(rot * xAxis);
            yAxis = vm::correct(rot * yAxis);
        }
    }
}
