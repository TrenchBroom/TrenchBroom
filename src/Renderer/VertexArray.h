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
        class BaseHolder {
        public:
            virtual ~BaseHolder() {}
            
            virtual VboBlock* upload() = 0;
        };
        
        template <typename T>
        class Holder : public BaseHolder {
        private:
            Vbo& m_vbo;
            std::vector<T> m_data;
        public:
            Holder(Vbo& vbo, const std::vector<T> data) :
            m_vbo(vbo),
            m_data(data) {}
            
            inline VboBlock* upload() {
                SetVboState mapVbo(m_vbo);
                mapVbo.mapped();
                
                VboBlock* block = m_vbo.allocateBlock(m_data.size() * sizeof(T));
                block->writeBuffer(0, m_data);
                return block;
            }
        };
        
        class VertexArray {
        private:
            BaseHolder* m_holder;
            VboBlock* m_block;
            size_t m_vertexCount;
        public:
            template <typename T>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex1<T> >& data) :
            m_holder(new Holder<Vertex1<T> >(vbo, data)),
            m_block(NULL),
            m_vertexCount(data.size()) {}
            
            template <typename T1, typename T2>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex2<T1, T2> >& data) :
            m_holder(new Holder<Vertex2<T1, T2> >(vbo, data)),
            m_block(NULL),
            m_vertexCount(data.size()) {}
            
            template <typename T1, typename T2, typename T3>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex3<T1, T2, T3> >& data) :
            m_holder(new Holder<Vertex3<T1, T2, T3> >(vbo, data)),
            m_block(NULL),
            m_vertexCount(data.size()) {}
            
            template <typename T1, typename T2, typename T3, typename T4>
            explicit VertexArray(Vbo& vbo, const typename std::vector<Vertex4<T1, T2, T3, T4> >& data) :
            m_holder(new Holder<Vertex4<T1, T2, T3, T4> >(vbo, data)),
            m_block(NULL),
            m_vertexCount(data.size()) {}
            
            VertexArray() :
            m_holder(NULL),
            m_block(0),
            m_vertexCount(0) {}
            
            VertexArray(VertexArray& other) :
            m_holder(NULL),
            m_block(NULL),
            m_vertexCount(0) {
                using std::swap;
                swap(*this, other);
            }
            
            inline VertexArray& operator= (VertexArray other) {
                using std::swap;
                swap(*this, other);
                return *this;
            }
            
            inline void prepare() {
                if (m_holder != NULL) {
                    assert(m_block == NULL);
                    m_block = m_holder->upload();
                    delete m_holder;
                    m_holder = NULL;
                }
            }
            
            inline size_t blockOffset() const {
                assert(m_block != NULL);
                return m_block->offset();
            }
            
            inline size_t vertexCount() const {
                return m_vertexCount;
            }
            
            ~VertexArray() {
                if (m_holder != NULL) {
                    delete m_holder;
                    m_holder = NULL;
                }
                if (m_block != NULL) {
                    m_block->free();
                    m_block = NULL;
                }
            }

            inline friend void swap(VertexArray& left, VertexArray& right) {
                using std::swap;
                swap(left.m_holder, right.m_holder);
                swap(left.m_block, right.m_block);
                swap(left.m_vertexCount, right.m_vertexCount);
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
