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

#include "Compass.h"

#include "VecMath.h"

#include "CollectionUtils.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        const size_t Compass::m_segments = 32;
        const float Compass::m_shaftLength = 28.0f;
        const float Compass::m_shaftRadius = 1.2f;
        const float Compass::m_headLength = 7.0f;
        const float Compass::m_headRadius = 3.5f;

        Compass::Compass(Vbo& vbo) {
            const Vec3f offset(0.0f, 0.0f, m_shaftLength / 2.0f);
            
            VertsAndNormals shaft = cylinder3D(m_shaftRadius, m_shaftLength, m_segments);
            for (size_t i = 0; i < shaft.vertices.size(); ++i)
                shaft.vertices[i] -= offset;
            
            VertsAndNormals shaftCap = circle3D(m_shaftRadius, m_segments);
            for (size_t i = 0; i < shaftCap.vertices.size(); ++i)
                shaftCap.vertices[i] -= offset;
            
            VertsAndNormals head = cone3D(m_headRadius, m_headLength, m_segments);
            for (size_t i = 0; i < head.vertices.size(); ++i)
                head.vertices[i] += offset;
            
            VertsAndNormals headCap = circle3D(m_headRadius, m_segments);
            for (size_t i = 0; i < headCap.vertices.size(); ++i)
                headCap.vertices[i] += offset;

            typedef VertexSpecs::P3N::Vertex Vertex;
            const Vertex::List shaftVertices = Vertex::fromLists(shaft.vertices, shaft.normals, shaft.vertices.size());
            const Vertex::List headVertices = Vertex::fromLists(head.vertices, head.normals, head.vertices.size());
            const Vertex::List capVertices = VectorUtils::concatenate(Vertex::fromLists(shaftCap.vertices, shaftCap.normals, shaftCap.vertices.size()),
                                                                      Vertex::fromLists(headCap.vertices, headCap.normals, headCap.vertices.size()));
            VertexArray::IndexArray indices(2);
            indices[0] = 0;
            indices[1] = shaftCap.vertices.size();
            VertexArray::CountArray counts(2);
            counts[0] = shaftCap.vertices.size();
            counts[1] = headCap.vertices.size();
            
            m_strip = VertexArray(vbo, GL_TRIANGLE_STRIP, shaftVertices);
            m_set = VertexArray(vbo, GL_TRIANGLES, headVertices);
            m_fans = VertexArray(vbo, GL_TRIANGLE_FAN, capVertices, indices, counts);
        }

        void Compass::prepare() {
            m_strip.prepare();
            m_set.prepare();
            m_fans.prepare();
        }
        
        void Compass::render(RenderContext& renderContext) {
            
        }
    }
}
