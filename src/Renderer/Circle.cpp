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
        Circle::Circle(Vbo& vbo, const float radius, const size_t segments, const bool filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init(vbo, radius, segments, filled, 0.0f, Math::Constants<float>::TwoPi);
        }
        
        Circle::Circle(Vbo& vbo, const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init(vbo, radius, segments, filled, startAngle, angleLength);
        }
        
        Circle::Circle(Vbo& vbo, const float radius, const size_t segments, const bool filled, const Vec3f& startAxis, const Vec3f& endAxis) {
            assert(radius > 0.0f);
            assert(segments > 0);

            const float angle1 = angleBetween(startAxis, Vec3f::PosY, Vec3f::PosZ);
            const float angle2 = angleBetween(endAxis, Vec3f::PosY, Vec3f::PosZ);
            const float minAngle = std::min(angle1, angle2);
            const float maxAngle = std::max(angle1, angle2);
            const float startAngle = (maxAngle - minAngle <= Math::Constants<float>::Pi ? minAngle : maxAngle);
            const float angleLength = std::min(angleBetween(startAxis, endAxis, Vec3f::PosZ), angleBetween(endAxis, startAxis, Vec3f::PosZ));

            init(vbo, radius, segments, filled, startAngle, angleLength);
        }
        
        void Circle::prepare() {
            m_array.prepare();
        }
        
        void Circle::render() {
            m_array.render();
        }
        
        void Circle::init(Vbo& vbo, const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength) {
            typedef VertexSpecs::P2::Vertex Vertex;

            const Vec2f::List positions = circle(radius, startAngle, angleLength, segments);
            const Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            m_array = VertexArray(vbo, filled ? GL_TRIANGLE_FAN : GL_LINE_STRIP, vertices);
        }
    }
}
