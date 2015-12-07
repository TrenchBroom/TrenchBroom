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

#include "Sphere.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        Sphere::Sphere(const float radius, const size_t iterations) {
            typedef VertexSpecs::P3::Vertex Vertex;
            
            const Vec3f::List positions = sphere3D(radius, iterations);
            Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            m_array = VertexArray::swap(vertices);
        }
        
        bool Sphere::prepared() const {
            return m_array.prepared();
        }

        void Sphere::prepare(Vbo& vbo) {
            m_array.prepare(vbo);
        }
        
        void Sphere::render() {
            m_array.render(GL_TRIANGLES);
        }
    }
}
