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
#include "String.h"
#include "VecMath.h"

#include "GL/GL.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T>
        class Vertex1 {
        public:
            typedef std::vector<Vertex1<T> > List;
        private:
            T m_value;
        public:
            Vertex1(const T& value) :
            m_value(value) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif
    
#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T1, typename T2>
        class Vertex2 {
        public:
            typedef std::vector<Vertex2<T1, T2> > List;
        private:
            T1 m_value1;
            T2 m_value2;
        public:
            Vertex2(const T1& value1, const T2& value2) :
            m_value1(value1),
            m_value2(value2) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif

#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T1, typename T2, typename T3>
        class Vertex3 {
        public:
            typedef std::vector<Vertex3<T1, T2, T3> > List;
        private:
            T1 m_value1;
            T2 m_value2;
            T3 m_value3;
        public:
            Vertex3(const T1& value1, const T2& value2, const T3& value3) :
            m_value1(value1),
            m_value2(value2),
            m_value3(value3) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif

#if defined _WIN32
#pragma pack(push,1)
#endif
        template <typename T1, typename T2, typename T3, typename T4>
        class Vertex4 {
        public:
            typedef std::vector<Vertex4<T1, T2, T3, T4> > List;
        private:
            T1 m_value1;
            T2 m_value2;
            T3 m_value3;
            T4 m_value4;
        public:
            Vertex4(const T1& value1, const T2& value2, const T3& value3, const T4& value4) :
            m_value1(value1),
            m_value2(value2),
            m_value3(value3),
            m_value4(value4) {}
#if defined _WIN32
        };
#pragma pack(pop)
#else
        } __attribute__((packed));
#endif

        typedef Vertex1<Vec3f> VP3;
        typedef Vertex2<Vec3f,Vec2f> VP3T2;
        typedef Vertex3<Vec3f,Vec3f,Vec2f> VP3N3T2;

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
            
            inline const size_t blockOffset() const {
                assert(m_block != NULL);
                return m_block->offset();
            }
            
            inline const size_t vertexCount() const {
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
