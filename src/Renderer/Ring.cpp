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

#include "Ring.h"

#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        Ring::Ring(const float radius, const float width, const float startAngle, const float angleLength, const size_t segments) {
            typedef VertexSpecs::P2::Vertex Vertex;
            Vertex::List vertices(2 * segments + 2);
            
            const float inner = radius;
            const float outer = radius + width;
            const float d = angleLength / segments;
            float a = startAngle;
            for (size_t i = 0; i <= segments; ++i) {
                const float s = std::sin(a);
                const float c = std::cos(a);
                vertices[2 * i + 0] = Vertex(Vec2f(outer * s, outer * c));
                vertices[2 * i + 1] = Vertex(Vec2f(inner * s, inner * c));
                a += d;
            }
            
            m_array = VertexArray::swap(GL_TRIANGLE_STRIP, vertices);
        }

        void Ring::prepare(Vbo& vbo) {
            m_array.prepare(vbo);
        }

        void Ring::render() {
            m_array.render();
        }
    }
}
