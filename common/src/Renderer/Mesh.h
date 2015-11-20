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
        template <class VertexSpec>
        class Mesh {
        public:
            struct IndexData {
                GLint index;
                GLsizei count;
                
                IndexData(const GLint i_index, const GLsizei i_count) :
                index(i_index),
                count(i_count) {}
            };
            
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
            
            IndexData addPoint(const Vertex& v1) {
                assert(checkCapacity(1));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                
                return IndexData(index, 1);
            }
            
            IndexData addPoints(const VertexList& vertices) {
                return addVertices(vertices);
            }
            
            IndexData addLine(const Vertex& v1, const Vertex& v2) {
                assert(checkCapacity(2));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                m_vertices.push_back(v2);
                
                return IndexData(index, 2);
            }
            
            IndexData addLines(const VertexList& vertices) {
                assert(vertices.size() % 2 == 0);
                return addVertices(vertices);
            }
            
            IndexData addLineStrip(const VertexList& vertices) {
                assert(vertices.size() >= 2);
                return addVertices(vertices);
            }
            
            IndexData addLineLoop(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(vertices);
            }

            IndexData addTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
                assert(checkCapacity(3));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                m_vertices.push_back(v2);
                m_vertices.push_back(v3);
                
                return IndexData(index, 3);
            }

            IndexData addTriangles(const VertexList& vertices) {
                assert(vertices.size() % 3 == 0);
                return addVertices(vertices);
            }

            IndexData addTriangleFan(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(vertices);
            }
            
            IndexData addTriangleStrip(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(vertices);
            }
            
            IndexData addQuad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4) {
                assert(checkCapacity(4));
                
                const GLint index = currentIndex();
                m_vertices.push_back(v1);
                m_vertices.push_back(v2);
                m_vertices.push_back(v3);
                m_vertices.push_back(v4);
                
                return IndexData(index, 4);
            }
            
            IndexData addQuads(const VertexList& vertices) {
                assert(vertices.size() % 4 == 0);
                return addVertices(vertices);
            }
            
            IndexData addQuadStrip(const VertexList& vertices) {
                assert(vertices.size() >= 4);
                assert(vertices.size() % 2 == 0);
                return addVertices(vertices);
            }
            
            IndexData addPolygon(const VertexList& vertices) {
                assert(vertices.size() >= 3);
                return addVertices(vertices);
            }
        private:
            bool checkCapacity(const size_t toAdd) const {
                return m_vertices.capacity() - m_vertices.size() >= toAdd;
            }

            GLint currentIndex() const {
                return static_cast<GLint>(vertexCount());
            }
            
            IndexData addVertices(const VertexList& vertices) {
                assert(checkCapacity(vertices.size()));
                
                const GLint index = currentIndex();
                const GLsizei count = static_cast<GLsizei>(vertices.size());
                VectorUtils::append(m_vertices, vertices);
                
                return IndexData(index, count);
            }
        };
    }
}

#endif /* defined(TrenchBroom_Mesh) */
