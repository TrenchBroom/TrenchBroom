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

#include "Circle.h"

#include "Renderer/RenderUtils.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        Circle::Circle(const float radius, const size_t segments, const bool filled) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init2D(radius, segments, 0.0f, Math::Cf::twoPi());
        }
        
        Circle::Circle(const float radius, const size_t segments, const bool filled, const float startAngle, const float angleLength) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            init2D(radius, segments, startAngle, angleLength);
        }
        
        Circle::Circle(const float radius, const size_t segments, const bool filled, const Math::Axis::Type axis, const Vec3f& startAxis, const Vec3f& endAxis) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);

            const std::pair<float, float> angles = startAngleAndLength(axis, startAxis, endAxis);
            init3D(radius, segments, axis, angles.first, angles.second);
        }
        
        Circle::Circle(const float radius, const size_t segments, const bool filled, const Math::Axis::Type axis, const float startAngle, const float angleLength) :
        m_filled(filled) {
            assert(radius > 0.0f);
            assert(segments > 0);
            assert(angleLength > 0.0);
            init3D(radius, segments, axis, startAngle, angleLength);
        }

        bool Circle::prepared() const {
            return m_array.prepared();
        }

        void Circle::prepare(Vbo& vbo) {
            m_array.prepare(vbo);
        }
        
        void Circle::render() {
            m_array.render(m_filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
        }
        
        void Circle::init2D(const float radius, const size_t segments, const float startAngle, const float angleLength) {
            typedef VertexSpecs::P2::Vertex Vertex;

            Vec2f::List positions = circle2D(radius, startAngle, angleLength, segments);
            if (m_filled)
                positions.push_back(Vec2f::Null);
            Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            m_array = VertexArray::swap(vertices);
        }
        
        void Circle::init3D(const float radius, const size_t segments, const Math::Axis::Type axis, const float startAngle, const float angleLength) {
            typedef VertexSpecs::P3::Vertex Vertex;
            
            Vec3f::List positions = circle2D(radius, axis, startAngle, angleLength, segments);
            if (m_filled)
                positions.push_back(Vec3f::Null);
            Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            m_array = VertexArray::swap(vertices);
        }
    }
}
