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

#ifndef __TrenchBroom__VertexArrayRenderer__
#define __TrenchBroom__VertexArrayRenderer__

#include "GL/GL.h"
#include "SharedPointer.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class VertexArray {
        public:
            typedef std::tr1::shared_ptr<VertexArray> Ptr;
            virtual ~VertexArray() {}
            
            virtual size_t size() const = 0;
            virtual void setup() = 0;
            virtual void cleanup() = 0;
        };
        
        template <typename VertexSpec>
        class VertexArrayT : public VertexArray {
        public:
            typedef typename VertexSpec::VertexType::List VertexList;
        private:
            Vbo& m_vbo;
            VboBlock* m_block;
            
            VertexList m_vertices;
            size_t m_size;
        public:
            VertexArrayT(Vbo& vbo, const VertexList& vertices) :
            m_vbo(vbo),
            m_block(NULL),
            m_vertices(vertices),
            m_size(m_vertices.size()) {}
            
            ~VertexArrayT() {
                if (m_block != NULL) {
                    m_block->free();
                    m_block = NULL;
                }
            }
            
            inline size_t size() const {
                return m_size;
            }
            
            inline void setup() {
                assert(m_size > 0);
                if (m_block == NULL) {
                    const size_t capacity = VertexSpec::Size * m_vertices.size();
                    m_block = m_vbo.allocateBlock(capacity);
                    
                    SetVboState mapVbo(m_vbo);
                    mapVbo.mapped();
                    m_block->writeBuffer(0, m_vertices);
                    m_vertices.resize(0);
                }
                
                VertexSpec::setup(m_block->offset());
            }
            
            inline void cleanup() {
                VertexSpec::cleanup();
            }
        };
        
        class VertexArrayRenderer {
        public:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;
        private:
            GLenum m_primType;
            VertexArray::Ptr m_vertexArray;
            IndexArray m_indices;
            CountArray m_counts;
        public:
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            explicit VertexArrayRenderer(Vbo& vbo, const GLenum primType, const std::vector<Vertex<A1, A2, A3, A4, A5> >& vertices) :
            m_primType(primType),
            m_vertexArray(VertexArray::Ptr(new VertexArrayT<typename Vertex<A1, A2, A3, A4, A5>::Spec>(vbo, vertices))) {}
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            explicit VertexArrayRenderer(Vbo& vbo, const GLenum primType, const std::vector<Vertex<A1, A2, A3, A4, A5> >& vertices, const IndexArray& indices, const CountArray& counts) :
            m_primType(primType),
            m_vertexArray(VertexArray::Ptr(new VertexArrayT<typename Vertex<A1, A2, A3, A4, A5>::Spec>(vbo, vertices))),
            m_indices(indices),
            m_counts(counts) {}

            void render();
        };
    }
}

#endif /* defined(__TrenchBroom__VertexArrayRenderer__) */
