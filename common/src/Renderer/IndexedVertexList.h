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

#ifndef TrenchBroom_IndexedVertexList_h
#define TrenchBroom_IndexedVertexList_h

#include "CollectionUtils.h"
#include "Renderer/GL.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        template <typename T>
        class IndexedVertexList {
        private:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;
            
            size_t m_primStart;
            typename T::Vertex::List m_vertices;
            IndexArray m_indices;
            CountArray m_counts;
        public:
            IndexedVertexList() :
            m_primStart(0) {}
            
            IndexedVertexList(const size_t vertexCount, const size_t primCount) :
            m_primStart(0),
            m_vertices(vertexCount),
            m_indices(primCount),
            m_counts(primCount) {}

            void reserve(const size_t vertexCount, const size_t primitiveCount) {
                m_vertices.reserve(vertexCount);
                m_indices.reserve(primitiveCount);
                m_counts.reserve(primitiveCount);
            }
            
            void addVertex(const typename T::Vertex& vertex) {
                m_vertices.push_back(vertex);
            }
            
            void addVertices(const typename T::Vertex::List& vertices) {
                m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
            }
            
            void addPrimitive(const typename T::Vertex::List& vertices) {
                addVertices(vertices);
                endPrimitive();
            }
            
            void addPrimitives(const IndexedVertexList& primitives) {
                VectorUtils::append(m_vertices, primitives.vertices());
                VectorUtils::append(m_indices, primitives.indices());
                VectorUtils::append(m_counts, primitives.counts());
                m_primStart = m_vertices.size();
            }
            
            void endPrimitive() {
                if (m_primStart < m_vertices.size()) {
                    m_indices.push_back(static_cast<GLint>(m_primStart));
                    m_counts.push_back(static_cast<GLsizei>(m_vertices.size() - m_primStart));
                    m_primStart = m_vertices.size();
                }
            }
            
            bool empty() const {
                return m_vertices.empty();
            }
            
            size_t vertexCount() const {
                return m_vertices.size();
            }
            
            size_t primCount() const {
                assert(m_indices.size() == m_counts.size());
                return m_indices.size();
            }
            
            IndexArray& indices() {
                return m_indices;
            }
            
            const IndexArray& indices() const {
                return m_indices;
            }
            
            CountArray& counts() {
                return m_counts;
            }
            
            const CountArray& counts() const {
                return m_counts;
            }
            
            typename T::Vertex::List& vertices() {
                return m_vertices;
            }
            
            const typename T::Vertex::List& vertices() const {
                return m_vertices;
            }
            
            void clear() {
                m_vertices.clear();
                m_indices.clear();
                m_counts.clear();
            }
        };
    }
}

#endif
