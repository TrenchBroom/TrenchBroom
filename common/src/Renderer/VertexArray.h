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

#ifndef TrenchBroom_VertexArrayRenderer
#define TrenchBroom_VertexArrayRenderer

#include "CollectionUtils.h"
#include "Renderer/GL.h"
#include "SharedPointer.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class VertexArray {
        private:
            class BaseHolder {
            public:
                typedef std::tr1::shared_ptr<BaseHolder> Ptr;
                virtual ~BaseHolder() {}
                
                virtual size_t vertexCount() const = 0;
                virtual size_t sizeInBytes() const = 0;
                
                virtual void prepare(Vbo& vbo) = 0;
                virtual void setup() = 0;
                virtual void cleanup() = 0;
            };
            
            template <typename VertexSpec>
            class Holder : public BaseHolder {
            private:
                typedef typename VertexSpec::Vertex::List VertexList;
            private:
                VboBlock* m_block;
                size_t m_vertexCount;
            public:
                size_t vertexCount() const {
                    return m_vertexCount;
                }
                
                size_t sizeInBytes() const {
                    return VertexSpec::Size * m_vertexCount;
                }
                
                virtual void prepare(Vbo& vbo) {
                    if (m_vertexCount > 0 && m_block == NULL) {
                        ActivateVbo activate(vbo);
                        m_block = vbo.allocateBlock(sizeInBytes());
                        
                        MapVboBlock map(m_block);
                        m_block->writeBuffer(0, doGetVertices());
                    }
                }
                
                virtual void setup() {
                    assert(m_block != NULL);
                    VertexSpec::setup(m_block->offset());
                }
                
                virtual void cleanup() {
                    VertexSpec::cleanup();
                }
            protected:
                Holder(const size_t vertexCount) :
                m_block(NULL),
                m_vertexCount(vertexCount) {}
                
                virtual ~Holder() {
                    if (m_block != NULL) {
                        m_block->free();
                        m_block = NULL;
                    }
                }
            private:
                virtual const VertexList& doGetVertices() const = 0;
            };
            
            template <typename VertexSpec>
            class CopyHolder : public Holder<VertexSpec> {
            public:
                typedef typename VertexSpec::Vertex::List VertexList;
            private:
                VertexList m_vertices;
            public:
                CopyHolder(const VertexList& vertices) :
                Holder<VertexSpec>(vertices.size()),
                m_vertices(vertices) {}
                
                void prepare(Vbo& vbo) {
                    Holder<VertexSpec>::prepare(vbo);
                    VectorUtils::clearToZero(m_vertices);
                }
            private:
                const VertexList& doGetVertices() const {
                    return m_vertices;
                }
            };
            
            template <typename VertexSpec>
            class SwapHolder : public Holder<VertexSpec> {
            public:
                typedef typename VertexSpec::Vertex::List VertexList;
            private:
                VertexList m_vertices;
            public:
                SwapHolder(VertexList& vertices) :
                Holder<VertexSpec>(vertices.size()),
                m_vertices(0) {
                    using std::swap;
                    swap(m_vertices, vertices);
                }
                
                void prepare(Vbo& vbo) {
                    Holder<VertexSpec>::prepare(vbo);
                    VectorUtils::clearToZero(m_vertices);
                }
            private:
                const VertexList& doGetVertices() const {
                    return m_vertices;
                }
            };
            
            template <typename VertexSpec>
            class RefHolder : public Holder<VertexSpec> {
            public:
                typedef typename VertexSpec::Vertex::List VertexList;
            private:
                const VertexList& m_vertices;
            public:
                RefHolder(const VertexList& vertices) :
                Holder<VertexSpec>(vertices.size()),
                m_vertices(vertices) {}
            private:
                const VertexList& doGetVertices() const {
                    return m_vertices;
                }
            };
        private:
            BaseHolder::Ptr m_holder;
            bool m_prepared;
            bool m_setup;
        public:
            explicit VertexArray();
            
            template <typename A1>
            static VertexArray copy(const std::vector<Vertex1<A1> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex1<A1>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1>
            static VertexArray swap(std::vector<Vertex1<A1> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex1<A1>::Spec>(vertices));
                return VertexArray(holder);
            }

            template <typename A1>
            static VertexArray ref(const std::vector<Vertex1<A1> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex1<A1>::Spec>(vertices));
                return VertexArray(holder);
            }

            template <typename A1, typename A2>
            static VertexArray copy(const std::vector<Vertex2<A1, A2> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex2<A1, A2>::Spec>(vertices));
                return VertexArray(holder);
            }

            template <typename A1, typename A2>
            static VertexArray swap(std::vector<Vertex2<A1, A2> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex2<A1, A2>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2>
            static VertexArray ref(const std::vector<Vertex2<A1, A2> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex2<A1, A2>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray copy(const std::vector<Vertex3<A1, A2, A3> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices));
                return VertexArray(holder);
            }

            template <typename A1, typename A2, typename A3>
            static VertexArray swap(std::vector<Vertex3<A1, A2, A3> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray ref(const std::vector<Vertex3<A1, A2, A3> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray copy(const std::vector<Vertex4<A1, A2, A3, A4> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices));
                return VertexArray(holder);
            }

            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray swap(std::vector<Vertex4<A1, A2, A3, A4> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray ref(const std::vector<Vertex4<A1, A2, A3, A4> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray copy(const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray swap(std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices));
                return VertexArray(holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray ref(const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices));
                return VertexArray(holder);
            }

            VertexArray& operator=(VertexArray other);
            friend void swap(VertexArray& left, VertexArray& right);
            
            bool empty() const;
            size_t sizeInBytes() const;
            size_t vertexCount() const;
            
            bool prepared() const;
            void prepare(Vbo& vbo);
            
            bool setup();
            void render(PrimType primType);
            void render(PrimType primType, GLint index, GLsizei count);
            void render(PrimType primType, const GLIndices& indices, const GLCounts& counts, GLint primCount);
            void render(PrimType primType, const GLIndices& indices, GLsizei count);
            void cleanup();
        private:
            VertexArray(BaseHolder::Ptr holder);
        };
    }
}

#endif /* defined(TrenchBroom_VertexArrayRenderer) */
