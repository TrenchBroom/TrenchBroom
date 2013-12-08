/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Circle.h"

#include "Renderer/RenderUtils.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Circle::Circle(const float radius, const size_t segments, const bool filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init2D(radius, segments, filled, 0.0f, Math::Constants<float>::TwoPi);
        }
        
        Circle::Circle(const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init2D(radius, segments, filled, startAngle, angleLength);
        }
        
        Circle::Circle(const float radius, const size_t segments, const bool filled, const Math::Axis::Type axis, const Vec3f& startAxis, const Vec3f& endAxis) {
            assert(radius > 0.0f);
            assert(segments > 0);

            float angle1, angle2, angleLength;
            switch (axis) {
                case Math::Axis::AX:
                    angle1 = angleBetween(startAxis, Vec3f::PosY, Vec3f::PosX);
                    angle2 = angleBetween(endAxis, Vec3f::PosY, Vec3f::PosX);
                    angleLength = std::min(angleBetween(startAxis, endAxis, Vec3f::PosX), angleBetween(endAxis, startAxis, Vec3f::PosX));
                    break;
                case Math::Axis::AY:
                    angle1 = angleBetween(startAxis, Vec3f::PosZ, Vec3f::PosY);
                    angle2 = angleBetween(endAxis, Vec3f::PosZ, Vec3f::PosY);
                    angleLength = std::min(angleBetween(startAxis, endAxis, Vec3f::PosY), angleBetween(endAxis, startAxis, Vec3f::PosY));
                    break;
                default:
                    angle1 = angleBetween(startAxis, Vec3f::PosX, Vec3f::PosZ);
                    angle2 = angleBetween(endAxis, Vec3f::PosX, Vec3f::PosZ);
                    angleLength = std::min(angleBetween(startAxis, endAxis, Vec3f::PosZ), angleBetween(endAxis, startAxis, Vec3f::PosZ));
                    break;
            }
            const float minAngle = std::min(angle1, angle2);
            const float maxAngle = std::max(angle1, angle2);
            const float startAngle = (maxAngle - minAngle <= Math::Constants<float>::Pi ? minAngle : maxAngle);

            init3D(radius, segments, filled, axis, startAngle, angleLength);
        }
        
        Circle::Circle(const float radius, const size_t segments, const bool filled, const Math::Axis::Type axis, const float startAngle, const float angleLength) {
            assert(radius > 0.0f);
            assert(segments > 0);
            assert(angleLength > 0.0);
            init3D(radius, segments, filled, axis, startAngle, angleLength);
        }

        void Circle::prepare(Vbo& vbo) {
            m_array.prepare(vbo);
        }
        
        void Circle::render() {
            m_array.render();
        }
        
        void Circle::init2D(const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength) {
            typedef VertexSpecs::P2::Vertex Vertex;

            Vec2f::List positions = circle2D(radius, startAngle, angleLength, segments);
            if (filled)
                positions.push_back(Vec2f::Null);
            Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            m_array = VertexArray::swap(filled ? GL_TRIANGLE_FAN : GL_LINE_STRIP, vertices);
        }
        
        void Circle::init3D(const float radius, const size_t segments, const bool filled, const Math::Axis::Type axis, const float startAngle, const float angleLength) {
            typedef VertexSpecs::P3::Vertex Vertex;
            
            Vec3f::List positions = circle2D(radius, axis, startAngle, angleLength, segments);
            if (filled)
                positions.push_back(Vec3f::Null);
            Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            m_array = VertexArray::swap(filled ? GL_TRIANGLE_FAN : GL_LINE_STRIP, vertices);
        }
    }
}
