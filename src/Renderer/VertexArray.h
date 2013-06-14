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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__VertexArray__
#define __TrenchBroom__VertexArray__

#include "TrenchBroom.h"
#include "StringUtils.h"
#include "VecMath.h"

#include "GL/GL.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/Vertex.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class VertexArray {
        private:
            VboBlock* m_block;
            size_t m_vertexCount;
        public:
            template <typename T>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex1<T> >& data) :
            m_block(NULL),
            m_vertexCount(data.size()) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Vertex1<T>));
                m_block->writeBuffer(0, data);
            }
            
            template <typename T1, typename T2>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex2<T1, T2> >& data) :
            m_block(NULL),
            m_vertexCount(data.size()) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Vertex2<T1, T2>));
                m_block->writeBuffer(0, data);
            }
            
            template <typename T1, typename T2, typename T3>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex3<T1, T2, T3> >& data) :
            m_block(NULL),
            m_vertexCount(data.size()) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Vertex3<T1, T2, T3>));
                m_block->writeBuffer(0, data);
            }
            
            template <typename T1, typename T2, typename T3, typename T4>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex4<T1, T2, T3, T4> >& data) :
            m_block(NULL),
            m_vertexCount(data.size()) {
                m_block = vbo.allocateBlock(data.size() * sizeof(Vertex4<T1, T2, T3, T4>));
                m_block->writeBuffer(0, data);
            }
            
            VertexArray(VertexArray& other) :
            m_block(NULL),
            m_vertexCount(other.m_vertexCount) {
                std::swap(m_block, other.m_block);
            }
            
            inline VertexArray& operator= (VertexArray& other) {
                if (m_block != NULL) {
                    m_block->free();
                    m_block = NULL;
                }
                std::swap(m_block, other.m_block);
                m_vertexCount = other.m_vertexCount;
                return *this;
            }
            
            inline size_t blockOffset() const {
                assert(m_block != NULL);
                return m_block->offset();
            }
            
            inline size_t vertexCount() const {
                return m_vertexCount;
            }
            
            ~VertexArray() {
                if (m_block != NULL) {
                    m_block->free();
                    m_block = NULL;
                }
            }
        };

        template <typename T>
        class IndexedVertexList {
        private:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;

            size_t m_primStart;
            typename T::List m_vertices;
            IndexArray m_indices;
            CountArray m_counts;
        public:
            IndexedVertexList() :
            m_primStart(0) {}
            
            inline void addVertex(const T& vertex) {
                m_vertices.push_back(vertex);
            }
            
            inline void endPrimitive() {
                m_indices.push_back(static_cast<GLint>(m_primStart));
                m_counts.push_back(static_cast<GLsizei>(m_vertices.size() - m_primStart));
                m_primStart = m_vertices.size();
            }
            
            inline const IndexArray& indices() const {
                return m_indices;
            }
            
            inline const CountArray& counts() const {
                return m_counts;
            }
            
            inline const typename T::List& vertices() const {
                return m_vertices;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__VertexArray__) */
