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

#ifndef TrenchBroom_Mesh
#define TrenchBroom_Mesh

#include "CollectionUtils.h"
#include "Renderer/IndexedVertexList.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/VertexArray.h"

#include <cassert>
#include <map>
#include <vector>

// disable warnings about truncated names in MSVC:
#ifdef _MSC_VER
#pragma warning(disable:4503)
#endif

namespace TrenchBroom {
    namespace Renderer {
        template <class VertexSpec, typename Key = int>
        class Mesh {
        public:
            typedef typename VertexSpec::Vertex Vertex;
            typedef typename Vertex::List VertexList;
        private:
            VertexList m_vertices;
        public:
            Mesh() :
            m_vertices(0) {}

            Mesh(const size_t vertexCount) :
            m_vertices() {
                m_vertices.reserve(vertexCount);
            }

            size_t vertexCount() const {
                return m_vertices.size();
            }
            
            VertexArray::RenderSpec addPoint(const Vertex& v1) {
                assert(checkCapacity(1));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                
                return VertexArray::RenderSpec(VertexArray::PT_Points, index, 1);
            }
            
            VertexArray::RenderSpec addPoints(const VertexList& vertices) {
                return addVertices(VertexArray::PT_Points, vertices);
            }
            
            VertexArray::RenderSpec addLine(const Vertex& v1, const Vertex& v2) {
                assert(checkCapacity(2));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                m_vertices.push_back(v2);
                
                return VertexArray::RenderSpec(VertexArray::PT_Lines, index, 2);
            }
            
            VertexArray::RenderSpec addLines(const VertexList& vertices) {
                assert(vertices.size() % 2 == 0);
                return addVertices(VertexArray::PT_Lines, vertices);
            }
            
            VertexArray::RenderSpec addLineStrip(const VertexList& vertices) {
                assert(vertices.size() >= 2);
                return addVertices(VertexArray::PT_LineStrips, vertices);
            }
            
            VertexArray::RenderSpec addLineLoop(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(VertexArray::PT_LineLoops, vertices);
            }

            VertexArray::RenderSpec addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
                assert(checkCapacity(3));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                m_vertices.push_back(v2);
                m_vertices.push_back(v3);
                
                return VertexArray::RenderSpec(VertexArray::PT_Triangles, index, 3);
            }

            VertexArray::RenderSpec addTriangles(const VertexList& vertices) {
                assert(vertices.size() % 3 == 0);
                return addVertices(VertexArray::PT_Triangles, vertices);
            }

            VertexArray::RenderSpec addTriangleFan(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(VertexArray::PT_TriangleFans, vertices);
            }
            
            VertexArray::RenderSpec addTriangleStrip(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(VertexArray::PT_TriangleStrips, vertices);
            }
            
            VertexArray::RenderSpec addQuad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4) {
                assert(checkCapacity(4));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                m_vertices.push_back(v2);
                m_vertices.push_back(v3);
                m_vertices.push_back(v4);
                
                return VertexArray::RenderSpec(VertexArray::PT_Triangles, index, 4);
            }
            
            VertexArray::RenderSpec addQuads(const VertexList& vertices) {
                assert(vertices.size() % 4 == 0);
                return addVertices(VertexArray::PT_Quads, vertices);
            }
            
            VertexArray::RenderSpec addQuadStrip(const VertexList& vertices) {
                assert(vertices.size() >= 4);
                assert(vertices.size() % 2 == 0);
                return addVertices(VertexArray::PT_QuadStrips, vertices);
            }
            
            VertexArray::RenderSpec addPolygon(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(VertexArray::PT_Polygons, vertices);
            }
        private:
            bool checkCapacity(const size_t toAdd) const {
                return m_vertices.capacity() - m_vertices.size() >= toAdd;
            }

            GLint currentIndex() const {
                return static_cast<GLint>(vertexCount());
            }
            
            VertexArray::RenderSpec addVertices(const VertexArray::PrimType primType, const VertexList& vertices) {
                assert(checkCapacity(vertices.size()));
                
                const GLint index = currentIndex();
                VectorUtils::append(m_vertices, vertices);
                
                return VertexArray::RenderSpec(primType, index, vertices.size());
            }
        };
    }
}

#endif /* defined(TrenchBroom_Mesh) */
