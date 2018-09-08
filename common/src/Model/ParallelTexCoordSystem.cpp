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

#include "VecMath.h"
#include "ParallelTexCoordSystem.h"
#include "Assets/Texture.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/BrushFace.h"

#include <cstddef>

namespace TrenchBroom {
    namespace Model {
        ParallelTexCoordSystemSnapshot::ParallelTexCoordSystemSnapshot(const vm::vec3& xAxis, const vm::vec3& yAxis) :
        m_xAxis(xAxis),
        m_yAxis(yAxis) {}
        
        ParallelTexCoordSystemSnapshot::ParallelTexCoordSystemSnapshot(ParallelTexCoordSystem* coordSystem) :
        m_xAxis(coordSystem->xAxis()),
        m_yAxis(coordSystem->yAxis()) {}
        
        TexCoordSystemSnapshot* ParallelTexCoordSystemSnapshot::doClone() const {
            return new ParallelTexCoordSystemSnapshot(m_xAxis, m_yAxis);
        }
        
        void ParallelTexCoordSystemSnapshot::doRestore(ParallelTexCoordSystem* coordSystem) const {
            coordSystem->m_xAxis = m_xAxis;
            coordSystem->m_yAxis = m_yAxis;
        }
        
        void ParallelTexCoordSystemSnapshot::doRestore(ParaxialTexCoordSystem* coordSystem) const {
            ensure(false, "wrong coord system type");
        }
        
        ParallelTexCoordSystem::ParallelTexCoordSystem(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const BrushFaceAttributes& attribs) {
            const vm::vec3 normal = normalize(cross(point2 - point0, point1 - point0));
            computeInitialAxes(normal, m_xAxis, m_yAxis);
            applyRotation(normal, attribs.rotation());
        }

        ParallelTexCoordSystem::ParallelTexCoordSystem(const vm::vec3& xAxis, const vm::vec3& yAxis) :
        m_xAxis(xAxis),
        m_yAxis(yAxis) {}
        
        TexCoordSystem* ParallelTexCoordSystem::doClone() const {
            return new ParallelTexCoordSystem(m_xAxis, m_yAxis);
        }
        
        TexCoordSystemSnapshot* ParallelTexCoordSystem::doTakeSnapshot() {
            return new ParallelTexCoordSystemSnapshot(this);
        }
        
        void ParallelTexCoordSystem::doRestoreSnapshot(const TexCoordSystemSnapshot& snapshot) {
            snapshot.doRestore(this);
        }

        vm::vec3 ParallelTexCoordSystem::getXAxis() const {
            return m_xAxis;
        }
        
        vm::vec3 ParallelTexCoordSystem::getYAxis() const {
            return m_yAxis;
        }
        
        vm::vec3 ParallelTexCoordSystem::getZAxis() const {
            return normalize(cross(m_xAxis, m_yAxis));
        }

        void ParallelTexCoordSystem::doResetCache(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const BrushFaceAttributes& attribs) {
            // no-op
        }

        void ParallelTexCoordSystem::doResetTextureAxes(const vm::vec3& normal) {
            computeInitialAxes(normal, m_xAxis, m_yAxis);
        }
        
        void ParallelTexCoordSystem::doResetTextureAxesToParaxial(const vm::vec3& normal, float angle) {
            const size_t index = ParaxialTexCoordSystem::planeNormalIndex(normal);
            ParaxialTexCoordSystem::axes(index, m_xAxis, m_yAxis);
            applyRotation(normal, static_cast<FloatType>(angle));
        }
        
        void ParallelTexCoordSystem::doResetTextureAxesToParallel(const vm::vec3& normal, float angle) {
            computeInitialAxes(normal, m_xAxis, m_yAxis);
            applyRotation(normal, static_cast<FloatType>(angle));
        }

        bool ParallelTexCoordSystem::isRotationInverted(const vm::vec3& normal) const {
            return false;
        }
        
        vm::vec2f ParallelTexCoordSystem::doGetTexCoords(const vm::vec3& point, const BrushFaceAttributes& attribs) const {
            return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / attribs.textureSize();
        }
        
        /**
         * Rotates from `oldAngle` to `newAngle`. Both of these are in CCW degrees about
         * the texture normal (`getZAxis()`). The provided `normal` is ignored.
         */
        void ParallelTexCoordSystem::doSetRotation(const vm::vec3& normal, const float oldAngle, const float newAngle) {
            const float angleDelta = newAngle - oldAngle;
            if (angleDelta == 0.0f)
                return;
            
            const FloatType angle = static_cast<FloatType>(vm::radians(angleDelta));
            applyRotation(getZAxis(), angle);
        }
        
        /**
         * Rotate CCW by `angle` radians about `normal`.
         */
        void ParallelTexCoordSystem::applyRotation(const vm::vec3& normal, const FloatType angle) {
            const vm::quat3 rot(normal, angle);
            m_xAxis = rot * m_xAxis;
            m_yAxis = rot * m_yAxis;
        }

        void ParallelTexCoordSystem::doTransform(const vm::plane3& oldBoundary, const vm::plane3& newBoundary, const vm::mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const vm::vec3& oldInvariant) {

            if (attribs.xScale() == 0.0f || attribs.yScale() == 0.0f) {
                return;
            }

            // when texture lock is off, just project the current texturing
            if (!lockTexture) {
                doUpdateNormalWithProjection(oldBoundary.normal, newBoundary.normal, attribs);
                return;
            }
            
            const auto effectiveTransformation = transformation;
            
            // determine the rotation by which the texture coordinate system will be rotated about its normal
            const auto angleDelta = computeTextureAngle(oldBoundary, effectiveTransformation);
            const auto newAngle = vm::correct(vm::normalizeDegrees(attribs.rotation() + angleDelta), 4);
            assert(!vm::isnan(newAngle));
            attribs.setRotation(newAngle);

            // calculate the current texture coordinates of the face's center
            const auto oldInvariantTechCoords = computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();
            assert(!isNaN(oldInvariantTechCoords));
            
            // compute the new texture axes
            const auto worldToTexSpace = toMatrix(vm::vec2f(0, 0), vm::vec2f(1, 1));
            
            // The formula for texturing is:
            //
            //     uv = worldToTexSpace * point
            //
            // We want to find a new worldToTexSpace matrix, ?, such that
            // transformed points have the same uv coords as they did
            // without the transform, with the old worldToTexSpace matrix:
            //
            //     uv = ? * transform * point
            //
            // The solution for ? is (worldToTexSpace * transform_inverse)
            const auto [invertible, inverseTransform] = invert(effectiveTransformation);
            assert(invertible); unused(invertible);
            const auto newWorldToTexSpace = worldToTexSpace * inverseTransform;
            
            // extract the new m_xAxis and m_yAxis from newWorldToTexSpace.
            // note, the matrix is in column major format.
            for (size_t i=0; i<3; i++) {
                m_xAxis[i] = newWorldToTexSpace[i][0];
                m_yAxis[i] = newWorldToTexSpace[i][1];
            }
            assert(!isNaN(m_xAxis));
            assert(!isNaN(m_yAxis));
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const auto newInvariant = effectiveTransformation * oldInvariant;
            const auto newInvariantTexCoords = computeTexCoords(newInvariant, attribs.scale());

            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            const auto newOffset = correct(attribs.modOffset(oldInvariantTechCoords - newInvariantTexCoords), 4);
            assert(!isNaN(newOffset));
            attribs.setOffset(newOffset);
        }

        float ParallelTexCoordSystem::computeTextureAngle(const vm::plane3& oldBoundary, const vm::mat4x4& transformation) const {
            const vm::mat4x4& rotationScale = stripTranslation(transformation);
            const vm::vec3& oldNormal = oldBoundary.normal;
            const vm::vec3  newNormal = normalize(rotationScale * oldNormal);

            const vm::mat4x4 nonRotation = computeNonTextureRotation(oldNormal, newNormal, rotationScale);
            const vm::vec3 newXAxis = normalize(rotationScale * m_xAxis);
            const vm::vec3 nonXAxis = normalize(nonRotation * m_xAxis);
            const FloatType angle = vm::degrees(angleBetween(nonXAxis, newXAxis, newNormal));
            return static_cast<float>(angle);
        }

        vm::mat4x4 ParallelTexCoordSystem::computeNonTextureRotation(const vm::vec3& oldNormal, const vm::vec3& newNormal, const vm::mat4x4& rotation) const {
            if (oldNormal == newNormal) {
                return vm::mat4x4::identity;
            } else if (oldNormal == -newNormal) {
                const vm::vec3 minorAxis = majorAxis(oldNormal, 2);
                const vm::vec3 axis = normalize(cross(oldNormal, minorAxis));
                return rotationMatrix(axis, vm::C::pi());
            } else {
                const vm::vec3 axis = normalize(cross(newNormal, oldNormal));
                const FloatType angle = angleBetween(newNormal, oldNormal, axis);
                return rotationMatrix(axis, angle);
            }
        }

        void ParallelTexCoordSystem::doUpdateNormalWithProjection(const vm::vec3& oldNormal, const vm::vec3& newNormal, const BrushFaceAttributes& attribs) {
            // Goal: (m_xAxis, m_yAxis) define the texture projection that was used for a face with oldNormal.
            // We want to update (m_xAxis, m_yAxis) to be usable on a face with newNormal.
            // Since this is the "projection" method (attempts to emulate ParaxialTexCoordSystem),
            // we want to modify (m_xAxis, m_yAxis) as little as possible
            // and only make 90 degree rotations if necessary.
            
            // Method: build a cube where the front face is the old texture projection (m_xAxis, m_yAxis)
            // and the other 5 faces are 90 degree rotations from that.
            // Use the "face" whose texture normal (cross product of the x and y axis) is closest to newNormal (the new face normal).
            
            std::vector<std::pair<vm::vec3, vm::vec3>> possibleTexAxes;
            possibleTexAxes.push_back({m_xAxis, m_yAxis}); // possibleTexAxes[0] = front
            possibleTexAxes.push_back({m_yAxis, m_xAxis}); // possibleTexAxes[1] = back
            const std::vector<vm::quat3> rotations {
                vm::quat3(normalize(m_xAxis), vm::radians(90.0)),  // possibleTexAxes[2]= bottom (90 degrees CCW about m_xAxis)
                vm::quat3(normalize(m_xAxis), vm::radians(-90.0)), // possibleTexAxes[3] = top
                vm::quat3(normalize(m_yAxis), vm::radians(90.0)),  // possibleTexAxes[4] = left
                vm::quat3(normalize(m_yAxis), vm::radians(-90.0)), // possibleTexAxes[5] = right
            };
            for (const vm::quat3& rotation : rotations) {
                possibleTexAxes.push_back({rotation * m_xAxis, rotation * m_yAxis});
            }
            assert(possibleTexAxes.size() == 6);
            
            std::vector<vm::vec3> possibleTexAxesNormals;
            for (const auto& axes : possibleTexAxes) {
                const vm::vec3 texNormal = normalize(cross(axes.first, axes.second));
                possibleTexAxesNormals.push_back(texNormal);
            }
            assert(possibleTexAxesNormals.size() == 6);
            
            // Find the index in possibleTexAxesNormals of the normal closest to the newNormal (face normal)
            std::vector<FloatType> cosAngles;
            for (const auto& texNormal : possibleTexAxesNormals) {
                const FloatType cosAngle = dot(texNormal, newNormal);
                cosAngles.push_back(cosAngle);
            }
            assert(cosAngles.size() == 6);
            
            const ptrdiff_t index = std::distance(cosAngles.begin(), std::max_element(cosAngles.begin(), cosAngles.end()));
            assert(index >= 0);
            assert(index < 6);
            
            // Skip 0 because it is "no change".
            // Skip 1 becaues it's a 180 degree flip, we prefer to just project the "front" texture axes.
            if (index >= 2) {
                const auto& axes = possibleTexAxes[static_cast<size_t>(index)];
                m_xAxis = axes.first;
                m_yAxis = axes.second;
            }
        }
        
        void ParallelTexCoordSystem::doUpdateNormalWithRotation(const vm::vec3& oldNormal, const vm::vec3& newNormal, const BrushFaceAttributes& attribs) {
            vm::quat3 rotation;
            auto axis = vm::cross(oldNormal, newNormal);
            if (axis == vm::vec3::zero) {
                // oldNormal and newNormal are either the same or opposite.
                // in this case, no need to update the texture axes.
                return;
            } else {
                axis = normalize(axis);
            }
            
            const auto angle = angleBetween(newNormal, oldNormal, axis);
            rotation = vm::quat3(axis, angle);

            m_xAxis = rotation * m_xAxis;
            m_yAxis = rotation * m_yAxis;
        }

        void ParallelTexCoordSystem::doShearTexture(const vm::vec3& normal, const vm::vec2f& f) {
            const vm::mat4x4 shear( 1.0, f[0], 0.0, 0.0,
                               f[1],  1.0, 0.0, 0.0,
                                0.0,  0.0, 1.0, 0.0,
                                0.0,  0.0, 0.0, 1.0);
            
            const auto toMatrix = coordinateSystemMatrix(m_xAxis, m_yAxis, getZAxis(), vm::vec3::zero);
            const auto [invertible, fromMatrix] = invert(toMatrix);
            assert(invertible); unused(invertible);

            const auto transform = fromMatrix * shear * toMatrix;
            m_xAxis = transform * m_xAxis;
            m_yAxis = transform * m_yAxis;
        }

        /**
         * Measures the angle between the line from `center` to `point` and the texture space X axis,
         * in CCW degrees about the texture normal.
         * Returns this, added to `currentAngle` (also in CCW degrees).
         */
        float ParallelTexCoordSystem::doMeasureAngle(const float currentAngle, const vm::vec2f& center, const vm::vec2f& point) const {
            const vm::vec3 vec(point - center);
            const auto angleInRadians = angleBetween(normalize(vec), vm::vec3::pos_x, vm::vec3::pos_z);
            return static_cast<float>(currentAngle + vm::degrees(angleInRadians));
        }

        void ParallelTexCoordSystem::computeInitialAxes(const vm::vec3& normal, vm::vec3& xAxis, vm::vec3& yAxis) const {
            switch (firstComponent(normal)) {
                case vm::Axis::AX:
                case vm::Axis::AY:
                    xAxis = normalize(cross(vm::vec3::pos_z, normal));
                    break;
                case vm::Axis::AZ:
                    xAxis = normalize(cross(vm::vec3::pos_y, normal));
                    break;
            }
            
            yAxis = normalize(cross(m_xAxis, normal));
        }
    }
}
