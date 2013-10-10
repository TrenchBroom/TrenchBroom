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

#ifndef TrenchBroom_IndexedVertexList_h
#define TrenchBroom_IndexedVertexList_h

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
            
            void endPrimitive() {
                m_indices.push_back(static_cast<GLint>(m_primStart));
                m_counts.push_back(static_cast<GLsizei>(m_vertices.size() - m_primStart));
                m_primStart = m_vertices.size();
            }
            
            const IndexArray& indices() const {
                return m_indices;
            }
            
            const CountArray& counts() const {
                return m_counts;
            }
            
            const typename T::Vertex::List& vertices() const {
                return m_vertices;
            }
        };
    }
}

#endif
