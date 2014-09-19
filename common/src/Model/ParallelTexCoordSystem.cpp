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

#include "ParallelTexCoordSystem.h"
#include "Assets/Texture.h"
#include "Model/ParaxialTexCoordSystem.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        ParallelTexCoordSystemSnapshot::ParallelTexCoordSystemSnapshot(ParallelTexCoordSystem* coordSystem) :
        m_coordSystem(coordSystem),
        m_xAxis(m_coordSystem->xAxis()),
        m_yAxis(m_coordSystem->yAxis()) {}
        
        void ParallelTexCoordSystemSnapshot::doRestore() {
            m_coordSystem->m_xAxis = m_xAxis;
            m_coordSystem->m_yAxis = m_yAxis;
        }

        ParallelTexCoordSystem::ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis) :
        m_xAxis(xAxis),
        m_yAxis(yAxis) {}
        
        ParallelTexCoordSystem::ParallelTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            const Vec3 normal = crossed(point2 - point0, point1 - point0).normalized();
            computeInitialAxes(normal, m_xAxis, m_yAxis);
        }
        
        TexCoordSystem* ParallelTexCoordSystem::doClone() const {
            return new ParallelTexCoordSystem(*this);
        }
        
        TexCoordSystemSnapshot* ParallelTexCoordSystem::doTakeSnapshot() {
            return new ParallelTexCoordSystemSnapshot(this);
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
        
        bool ParallelTexCoordSystem::isRotationInverted(const Vec3& normal) const {
            return false;
        }
        
        Vec2f ParallelTexCoordSystem::doGetTexCoords(const Vec3& point, const BrushFaceAttribs& attribs) const {
            return (computeTexCoords(point, attribs.scale()) + attribs.offset()) / attribs.textureSize();
        }
        
        void ParallelTexCoordSystem::doSetRotation(const Vec3& normal, const float oldAngle, const float newAngle) {
            const float angleDelta = newAngle - oldAngle;
            if (angleDelta == 0.0f)
                return;
            
            const FloatType angle = static_cast<FloatType>(Math::radians(-angleDelta));
            const Quat3 rot(normal, angle);
            
            m_xAxis = rot * m_xAxis;
            m_yAxis = rot * m_yAxis;
        }
        
        void ParallelTexCoordSystem::doTransform(const Plane3& oldBoundary, const Mat4x4& transformation, BrushFaceAttribs& attribs, bool lockTexture, const Vec3& oldInvariant) {
            // calculate the current texture coordinates of the face's center
            const Vec2f oldInvariantTechCoords = computeTexCoords(oldInvariant, attribs.scale()) + attribs.offset();
            
            const Vec3 offset = transformation * Vec3::Null;
            m_xAxis           = transformation * m_xAxis - offset;
            m_yAxis           = transformation * m_yAxis - offset;

            // determine the new texture coordinates of the transformed center of the face, sans offsets
            const Vec3 newInvariant = transformation * oldInvariant;
            const Vec2f newInvariantTexCoords = computeTexCoords(newInvariant, attribs.scale());
            
            // since the center should be invariant, the offsets are determined by the difference of the current and
            // the original texture coordinates of the center
            const Vec2f newOffset = attribs.modOffset(oldInvariantTechCoords - newInvariantTexCoords).corrected(4);
            attribs.setOffset(newOffset);
        }

        void ParallelTexCoordSystem::doUpdateNormal(const Vec3& oldNormal, const Vec3& newNormal, const BrushFaceAttribs& attribs) {
            Quat3 rotation;
            const Vec3 cross = crossed(oldNormal, newNormal);
            if (cross.null()) {
                rotation = Quat3(oldNormal.makePerpendicular(), Math::C::pi());
            } else {
                const Vec3 axis = cross.normalized();
                const FloatType angle = angleBetween(newNormal, oldNormal, axis);
                rotation = Quat3(axis, angle);
            }
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
