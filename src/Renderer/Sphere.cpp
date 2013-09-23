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

#include "Sphere.h"

#include "TrenchBroom.h"
#include "VecMath.h"

#include "Renderer/RenderUtils.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        Sphere::Sphere(Vbo& vbo, const float radius, const size_t iterations) {
            typedef VertexSpecs::P3::Vertex Vertex;
            
            const Vec3f::List positions = sphere(radius, iterations);
            const Vertex::List vertices = Vertex::fromLists(positions, positions.size());
            m_array = VertexArray(vbo, GL_TRIANGLES, vertices);
        }
        
        void Sphere::prepare() {
            m_array.prepare();
        }
        
        void Sphere::render() {
            m_array.render();
        }
    }
}
