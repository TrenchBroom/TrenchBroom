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

#ifndef __TrenchBroom__VertexSpec__
#define __TrenchBroom__VertexSpec__

#include "Vec.h"
#include "GL/GL.h"
#include "Renderer/AttributeSpec.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        class VertexSpec;
        
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        class Vertex {
        public:
            typedef VertexSpec<A1, A2, A3, A4, A5> Spec;
            typedef std::vector<Vertex<A1, A2, A3, A4, A5> > List;
        private:
            typename A1::ElementType m_v1;
            typename A2::ElementType m_v2;
            typename A3::ElementType m_v3;
            typename A4::ElementType m_v4;
            typename A5::ElementType m_v5;
        public:
            Vertex(const typename A1::ElementType& v1 = typename A1::ElementType(),
                   const typename A2::ElementType& v2 = typename A2::ElementType(),
                   const typename A3::ElementType& v3 = typename A3::ElementType(),
                   const typename A4::ElementType& v4 = typename A4::ElementType(),
                   const typename A5::ElementType& v5 = typename A5::ElementType()) :
            m_v1(v1),
            m_v2(v2),
            m_v3(v3),
            m_v4(v4),
            m_v5(v5) {}
        };
        
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        class VertexSpec {
        public:
            typedef Vertex<A1, A2, A3, A4, A5> VertexType;
            static const size_t Size;
        public:
            static void setup(const size_t baseOffset) {
                size_t offset = baseOffset;
                A1::setup(0, Size, offset);
                offset += A1::Size;
                A2::setup(1, Size, offset);
                offset += A2::Size;
                A3::setup(2, Size, offset);
                offset += A3::Size;
                A4::setup(3, Size, offset);
                offset += A4::Size;
                A5::setup(4, Size, offset);
                offset += A5::Size;
            }
            
            static void cleanup() {
                A5::cleanup(4);
                A4::cleanup(3);
                A3::cleanup(2);
                A2::cleanup(1);
                A1::cleanup(0);
            }
        private:
            VertexSpec();
        };
        
        template <typename A1, typename A2, typename A3, typename A4, typename A5>
        const size_t VertexSpec<A1, A2, A3, A4, A5>::Size = A1::Size + A2::Size + A3::Size + A4::Size + A5::Size;

        namespace VertexSpecs {
            typedef VertexSpec<AttributeSpecs::P3, AttributeSpecs::Empty, AttributeSpecs::Empty, AttributeSpecs::Empty, AttributeSpecs::Empty> P3;
            typedef VertexSpec<AttributeSpecs::P3, AttributeSpecs::C4, AttributeSpecs::Empty, AttributeSpecs::Empty, AttributeSpecs::Empty> P3C4;
            typedef VertexSpec<AttributeSpecs::P3, AttributeSpecs::N, AttributeSpecs::T02, AttributeSpecs::Empty, AttributeSpecs::Empty> P3NT2;
        }
        
        template <typename T>
        class IndexedVertexList {
        private:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;
            
            size_t m_primStart;
            typename T::VertexType::List m_vertices;
            IndexArray m_indices;
            CountArray m_counts;
        public:
            IndexedVertexList() :
            m_primStart(0) {}
            
            inline void addVertex(const typename T::Vertex& vertex) {
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
            
            inline const typename T::VertexType::List& vertices() const {
                return m_vertices;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__VertexSpec__) */
