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

#include "ParallelTexCoordSystem.h"
#include "Assets/Texture.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/BrushFace.h"

#include <cstddef>

namespace TrenchBroom {
    namespace Model {
        ParallelTexCoordSystemSnapshot::ParallelTexCoordSystemSnapshot(const Vec3& xAxis, const Vec3& yAxis) :
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
        
        ParallelTexCoordSystem::ParallelTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs) {
            const Vec3 normal = crossed(point2 - point0, point1 - point0).normalized();
            computeInitialAxes(normal, m_xAxis, m_yAxis);
            applyRotation(normal, attribs.rotation());
        }

        ParallelTexCoordSystem::ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis) :
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

        Vec3 ParallelTexCoordSystem::getXAxis() const {
            return m_xAxis;
        }
        
        Vec3 ParallelTexCoordSystem::getYAxis() const {
            return m_yAxis;
        }
        
        Vec3 ParallelTexCoordSystem::getZAxis() const {
            return crossed(m_xAxis, m_yAxis).normalized();
        }

        void ParallelTexCoordSystem::doResetCache(const Vec3& point0, const Vec3& point1, const Vec3& point2, const BrushFaceAttributes& attribs) {
            // no-op
        }

        void ParallelTexCoordSystem::doResetTextureAxes(const Vec3& normal) {
            computeInitialAxes(normal, m_xAxis, m_yAxis);
        }
        
        void ParallelTexCoordSystem::doResetTextureAxesToParaxial(const Vec3& normal, float angle) {
            const size_t index = ParaxialTexCoordSystem::planeNormalIndex(normal);
            ParaxialTexCoordSystem::axes(index, m_xAxis, m_yAxis);
            applyRotation(normal, static_cast<FloatType>(angle));
        }
        
        void ParallelTexCoordSystem::doResetTextureAxesToParallel(const Vec3& normal, float angle) {
            computeInitialAxes(normal, m_xAxis, m_yAxis);
            applyRotation(normal, static_cast<FloatType>(angle));
        }

        bool ParallelTexCoordSystem::isRotationInverted(const Vec3& normal) const {
            return false;
        }
        
        Vec2f ParallelTexCoordSystem::doGetTexCoords(const Vec3& point, const BrushFaceAttributes& attribs) const {
            return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / attribs.textureSize();
        }
        
        void ParallelTexCoordSystem::doSetRotation(const Vec3& normal, const float oldAngle, const float newAngle) {
            const float angleDelta = newAngle - oldAngle;
            if (angleDelta == 0.0f)
                return;
            
            const FloatType angle = static_cast<FloatType>(Math::radians(-angleDelta));
            applyRotation(normal, angle);
        }
        
        void ParallelTexCoordSystem::applyRotation(const Vec3& normal, const FloatType angle) {
            const Quat3 rot(normal, angle);
            m_xAxis = rot * m_xAxis;
            m_yAxis = rot * m_yAxis;
        }

        void ParallelTexCoordSystem::doTransform(const Plane3& oldBoundary, const Plane3& newBoundary, const Mat4x4& transformation, BrushFaceAttributes& attribs, bool lockTexture, const Vec3& oldInvariant) {

            if (attribs.xScale() == 0.0f || attribs.yScale() == 0.0f)
                return;
            
            // when texture lock is off, strip off the translation and flip part of the transformation
            Mat4x4 effectiveTransformation;
            if (lockTexture) {
                effectiveTransformation = transformation;
            } else {
                effectiveTransformation = stripTranslation(transformation);
                
                // we also shouldn't compensate for flips
                if (effectiveTransformation.equals(Mat4x4::MirX)
                    || effectiveTransformation.equals(Mat4x4::MirY)
                    || effectiveTransformation.equals(Mat4x4::MirZ)) {
                    effectiveTransformation = Mat4x4::Identity;
                }
            }
            
            // determine the rotation by which the texture coordinate system will be rotated about its normal
            const float angleDelta = computeTextureAngle(oldBoundary, effectiveTransformation);
            const float newAngle = Math::correct(Math::normalizeDegrees(attribs.rotation() + angleDelta), 4);
            assert(!Math::isnan(newAngle));
            attribs.setRotation(newAngle);

            // calculate the current texture coordinates of the face's center
            const Vec2f oldInvariantTechCoords = computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();
            assert(!oldInvariantTechCoords.nan());
            
            // compute the new texture axes
            const Vec3 offset = effectiveTransformation * Vec3::Null;
            m_xAxis           = effectiveTransformation * m_xAxis - offset;
            m_yAxis           = effectiveTransformation * m_yAxis - offset;
            assert(!m_xAxis.nan());
            assert(!m_yAxis.nan());
            
            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const Vec3 newInvariant = effectiveTransformation * oldInvariant;
            const Vec2f newInvariantTexCoords = computeTexCoords(newInvariant, attribs.scale());
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            const Vec2f newOffset = attribs.modOffset(oldInvariantTechCoords - newInvariantTexCoords).corrected(4);
            assert(!newOffset.nan());
            attribs.setOffset(newOffset);
        }

        float ParallelTexCoordSystem::computeTextureAngle(const Plane3& oldBoundary, const Mat4x4& transformation) const {
            const Mat4x4& rotation = stripTranslation(transformation);
            const Vec3& oldNormal = oldBoundary.normal;
            const Vec3  newNormal = rotation * oldNormal;

            const Mat4x4 nonRotation = computeNonTextureRotation(oldNormal, newNormal, rotation);
            const Vec3 newXAxis = (rotation * m_xAxis).normalized();
            const Vec3 nonXAxis = (nonRotation * m_xAxis).normalized();
            const FloatType angle = Math::degrees(angleBetween(nonXAxis, newXAxis, newNormal));
            return static_cast<float>(angle);
        }

        Mat4x4 ParallelTexCoordSystem::computeNonTextureRotation(const Vec3& oldNormal, const Vec3& newNormal, const Mat4x4& rotation) const {
            if (oldNormal.equals(newNormal))
                return Mat4x4::Identity;
            
            if (oldNormal.equals(-newNormal)) {
                const Vec3 minorAxis = oldNormal.majorAxis(2);
                const Vec3 axis = crossed(oldNormal, minorAxis).normalized();
                return rotationMatrix(axis, Math::C::pi());
            }
            
            const Vec3 axis = crossed(newNormal, oldNormal).normalized();
            const FloatType angle = angleBetween(newNormal, oldNormal, axis);
            return rotationMatrix(axis, angle);
        }

        void ParallelTexCoordSystem::doUpdateNormalWithProjection(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) {
            // Goal: (m_xAxis, m_yAxis) define the texture projection that was used for a face with oldNormal.
            // We want to update (m_xAxis, m_yAxis) to be usable on a face with newNormal.
            // Since this is the "projection" method (attempts to emulate ParaxialTexCoordSystem),
            // we want to modify (m_xAxis, m_yAxis) as little as possible
            // and only make 90 degree rotations if necessary.
            
            // Method: build a cube where the front face is the old texture projection (m_xAxis, m_yAxis)
            // and the other 5 faces are 90 degree rotations from that.
            // Use the "face" whose texture normal (cross product of the x and y axis) is closest to newNormal (the new face normal).
            
            std::vector<std::pair<Vec3, Vec3>> possibleTexAxes;
            possibleTexAxes.push_back({m_xAxis, m_yAxis}); // possibleTexAxes[0] = front
            possibleTexAxes.push_back({m_yAxis, m_xAxis}); // possibleTexAxes[1] = back
            const std::vector<Quat3> rotations {
                Quat3(m_xAxis, Math::radians(90.0)),  // possibleTexAxes[2]= bottom (90 degrees CCW about m_xAxis)
                Quat3(m_xAxis, Math::radians(-90.0)), // possibleTexAxes[3] = top
                Quat3(m_yAxis, Math::radians(90.0)),  // possibleTexAxes[4] = left
                Quat3(m_yAxis, Math::radians(-90.0)), // possibleTexAxes[5] = right
            };
            for (const Quat3& rotation : rotations) {
                possibleTexAxes.push_back({rotation * m_xAxis, rotation * m_yAxis});
            }
            assert(possibleTexAxes.size() == 6);
            
            std::vector<Vec3> possibleTexAxesNormals;
            for (const auto& axes : possibleTexAxes) {
                const Vec3 texNormal = crossed(axes.first, axes.second).normalized();
                possibleTexAxesNormals.push_back(texNormal);
            }
            assert(possibleTexAxesNormals.size() == 6);
            
            // Find the index in possibleTexAxesNormals of the normal closest to the newNormal (face normal)
            std::vector<FloatType> cosAngles;
			for (const auto& texNormal : possibleTexAxesNormals) {
                const FloatType cosAngle = texNormal.dot(newNormal);
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
        
        void ParallelTexCoordSystem::doUpdateNormalWithRotation(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttributes& attribs) {
            Quat3 rotation;
            const Vec3 cross = crossed(oldNormal, newNormal);
            Vec3 axis;
            if (cross.null()) {
                // oldNormal and newNormal are either the same or opposite.
                // in this case, no need to update the texture axes.
                return;
            } else {
                axis = cross.normalized();
            }
            
            const FloatType angle = angleBetween(newNormal, oldNormal, axis);
            rotation = Quat3(axis, angle);

            m_xAxis = rotation * m_xAxis;
            m_yAxis = rotation * m_yAxis;
        }

        void ParallelTexCoordSystem::doShearTexture(const Vec3& normal, const Vec2f& f) {
            const Mat4x4 shear( 1.0, f[0], 0.0, 0.0,
                               f[1],  1.0, 0.0, 0.0,
                                0.0,  0.0, 1.0, 0.0,
                                0.0,  0.0, 0.0, 1.0);
            
            const Mat4x4 toMatrix = coordinateSystemMatrix(m_xAxis, m_yAxis, getZAxis(), Vec3::Null);
            const Mat4x4 fromMatrix = invertedMatrix(toMatrix);

            const Mat4x4 transform = fromMatrix * shear * toMatrix;
            m_xAxis = transform * m_xAxis;
            m_yAxis = transform * m_yAxis;
        }

        float ParallelTexCoordSystem::doMeasureAngle(const float currentAngle, const Vec2f& center, const Vec2f& point) const {
            const Vec3 vec(point - center);
            const FloatType angleInRadians = angleBetween(vec.normalized(), Vec3::PosX, Vec3::PosZ);
            return static_cast<float>(currentAngle + Math::degrees(angleInRadians));
        }

        void ParallelTexCoordSystem::computeInitialAxes(const Vec3& normal, Vec3& xAxis, Vec3& yAxis) const {
            const Math::Axis::Type first = normal.firstComponent();
            
            switch (first) {
                case Math::Axis::AX:
                case Math::Axis::AY:
                    xAxis = crossed(Vec3::PosZ, normal).normalized();
                    break;
                case Math::Axis::AZ:
                    xAxis = crossed(Vec3::PosY, normal).normalized();
                    break;
            }
            
            yAxis = crossed(m_xAxis, normal).normalized();
        }
    }
}
