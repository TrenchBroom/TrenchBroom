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
        ParallelTexCoordSystem::ParallelTexCoordSystem(const Vec3& xAxis, const Vec3& yAxis, const Vec3& normal, const float rotation) :
        m_xAxis(xAxis),
        m_yAxis(yAxis) {
            const FloatType angle = static_cast<FloatType>(Math::radians(rotation));
            const Quat3 rot(normal, -angle);
            m_initialXAxis = rot * m_xAxis;
            m_initialYAxis = rot * m_yAxis;
        }
        
        ParallelTexCoordSystem::ParallelTexCoordSystem(const Vec3& point0, const Vec3& point1, const Vec3& point2) {
            const Vec3 normal = crossed(point2 - point0, point1 - point0).normalized();
            
            const size_t index = ParaxialTexCoordSystem::planeNormalIndex(normal);
            ParaxialTexCoordSystem::axes(index, m_initialXAxis, m_initialYAxis);

            m_xAxis = m_initialXAxis;
            m_yAxis = m_initialYAxis;
        }
        
        TexCoordSystem* ParallelTexCoordSystem::doClone() const {
            return new ParallelTexCoordSystem(*this);
        }
        
        const Vec3& ParallelTexCoordSystem::getXAxis() const {
            return m_xAxis;
        }
        
        const Vec3& ParallelTexCoordSystem::getYAxis() const {
            return m_yAxis;
        }
        
        const Vec3& ParallelTexCoordSystem::getZAxis() const {
            return m_zAxis;
        }
        
        bool ParallelTexCoordSystem::isRotationInverted(const Vec3& normal) const {
            return false;
        }
        
        Vec2f ParallelTexCoordSystem::doGetTexCoords(const Vec3& point, const BrushFaceAttribs& attribs) const {
            const Assets::Texture* texture = attribs.texture();
            const size_t textureWidth = texture == NULL ? 1 : texture->width();
            const size_t textureHeight = texture == NULL ? 1 : texture->height();
            const float safeXScale = attribs.xScale() == 0.0f ? 1.0f : attribs.xScale();
            const float safeYScale = attribs.yScale() == 0.0f ? 1.0f : attribs.yScale();
            const float x = static_cast<float>((point.dot(m_xAxis / safeXScale) + attribs.xOffset()) / textureWidth);
            const float y = static_cast<float>((point.dot(m_yAxis / safeYScale) + attribs.yOffset()) / textureHeight);
            return Vec2f(x, y);
        }
        
        void ParallelTexCoordSystem::doUpdate(const Vec3& normal, const BrushFaceAttribs& attribs) {
            const FloatType angle = static_cast<FloatType>(Math::radians(attribs.rotation()));
            const Quat3 rot(normal, angle);
            m_xAxis = rot * m_initialXAxis;
            m_yAxis = rot * m_initialYAxis;
            m_zAxis = crossed(m_xAxis, m_yAxis).normalized();
        }
        
        void ParallelTexCoordSystem::doCompensate(const Vec3& normal, const Vec3& center, const Mat4x4& transformation, BrushFaceAttribs& attribs) {
            // todo
        }

        float ParallelTexCoordSystem::doMeasureAngle(const float currentAngle, const Vec2f& center, const Vec2f& point) const {
            const Quat3 rot(Vec3::PosZ, -currentAngle);
            const Vec3 vec = rot * (point - center);
            const FloatType angleInRadians = Math::C::TwoPi - angleBetween(vec.normalized(), Vec3::PosX, Vec3::PosZ);
            return static_cast<float>(Math::degrees(angleInRadians));
        }
    }
}
