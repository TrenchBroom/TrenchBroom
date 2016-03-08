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

#ifndef VertexListBuilder_h
#define VertexListBuilder_h

#include "Renderer/GL.h"
#include "Renderer/Vertex.h"

namespace TrenchBroom {
    namespace Renderer {
        template <typename VertexSpec>
        class VertexListBuilder {
        public:
            struct IndexData {
                size_t index;
                size_t count;
                
                IndexData(const size_t i_index, const size_t i_count) :
                index(i_index),
                count(i_count) {}
            };

            typedef typename VertexSpec::Vertex Vertex;
            typedef typename Vertex::List VertexList;
        private:
            VertexList m_vertices;
            bool m_dynamicGrowth;
        public:
            VertexListBuilder(const size_t capacity) :
            m_vertices(0),
            m_dynamicGrowth(false) {
                m_vertices.reserve(capacity);
            }
            
            VertexListBuilder() :
            m_dynamicGrowth(true) {}
            
            size_t vertexCount() const {
                return m_vertices.size();
            }
            
            const VertexList& vertices() const {
                return m_vertices;
            }
            
            VertexList& vertices() {
                return m_vertices;
            }
            
            IndexData addPoint(const Vertex& v1) {
                assert(checkCapacity(1));
                
                const size_t index = currentIndex();
                m_vertices.push_back(v1);
                
                return IndexData(index, 1);
            }
            
            IndexData addPoints(const VertexList& vertices) {
                return addVertices(vertices);
            }
            
            IndexData addLine(const Vertex& v1, const Vertex& v2) {
                assert(checkCapacity(2));
                
                const size_t index = currentIndex();
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
                
                const size_t index = currentIndex();
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
                
                const size_t index = currentIndex();
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
            IndexData addVertices(const VertexList& vertices) {
                assert(checkCapacity(vertices.size()));
                
                const size_t index = currentIndex();
                const size_t count = vertices.size();
                VectorUtils::append(m_vertices, vertices);
                
                return IndexData(index, count);
            }

            bool checkCapacity(const size_t toAdd) const {
                return m_dynamicGrowth || m_vertices.capacity() - m_vertices.size() >= toAdd;
            }
            
            size_t currentIndex() const {
                return static_cast<size_t>(vertexCount());
            }
        };
    }
}

#endif /* VertexListBuilder_h */
