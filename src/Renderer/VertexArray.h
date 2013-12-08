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

#ifndef __TrenchBroom__VertexArrayRenderer__
#define __TrenchBroom__VertexArrayRenderer__

#include "Renderer/GL.h"
#include "SharedPointer.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace Renderer {
        class BaseHolder {
        public:
            typedef std::tr1::shared_ptr<BaseHolder> Ptr;
            virtual ~BaseHolder() {}
            
            virtual size_t size() const = 0;
            virtual void setup() = 0;
            virtual void cleanup() = 0;
        };
        
        template <typename VertexSpec>
        class Holder : public BaseHolder {
        private:
            typedef typename VertexSpec::Vertex::List VertexList;
        private:
            Vbo& m_vbo;
            VboBlock* m_block;
            size_t m_size;
        public:
            size_t size() const {
                return m_size;
            }
            
            virtual void setup() {
                assert(m_size > 0);
                if (m_block == NULL) {
                    SetVboState mapVbo(m_vbo);
                    mapVbo.mapped();

                    const VertexList& vertices = doGetVertices();
                    const size_t capacity = VertexSpec::Size * vertices.size();
                    m_block = m_vbo.allocateBlock(capacity);
                    m_block->writeBuffer(0, vertices);
                }
                
                VertexSpec::setup(m_block->offset());
            }
            
            virtual void cleanup() {
                VertexSpec::cleanup();
            }
        protected:
            Holder(Vbo& vbo, const size_t vertexCount) :
            m_vbo(vbo),
            m_block(NULL),
            m_size(vertexCount) {}
            
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
            CopyHolder(Vbo& vbo, const VertexList& vertices) :
            Holder<VertexSpec>(vbo, vertices.size()),
            m_vertices(vertices) {}
            
            void setup() {
                Holder<VertexSpec>::setup();
                m_vertices.resize(0);
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
            SwapHolder(Vbo& vbo, VertexList& vertices) :
            Holder<VertexSpec>(vbo, vertices.size()) {
                std::swap(m_vertices, vertices);
            }
            
            void setup() {
                Holder<VertexSpec>::setup();
                m_vertices.resize(0);
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
            RefHolder(Vbo& vbo, const VertexList& vertices) :
            Holder<VertexSpec>(vbo, vertices.size()),
            m_vertices(vertices) {}
        private:
            const VertexList& doGetVertices() const {
                return m_vertices;
            }
        };

        class VertexArray {
        public:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;
            static const IndexArray EmptyIndexArray;
            static const CountArray EmptyCountArray;
        private:
            GLenum m_primType;
            size_t m_vertexCount;
            BaseHolder::Ptr m_holder;
            IndexArray m_indices;
            CountArray m_counts;
        public:
            explicit VertexArray();
            
            template <typename A1>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex1<A1> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex1<A1>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex1<A1> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex1<A1>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }

            template <typename A1>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex1<A1> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex1<A1>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex1<A1> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex1<A1>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }

            template <typename A1>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex1<A1> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex1<A1>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex1<A1> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex1<A1>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }

            template <typename A1, typename A2>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex2<A1, A2> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex2<A1, A2>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex2<A1, A2> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex2<A1, A2>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }

            template <typename A1, typename A2>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex2<A1, A2> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex2<A1, A2>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex2<A1, A2> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex2<A1, A2>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }

            template <typename A1, typename A2>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex2<A1, A2> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex2<A1, A2>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex2<A1, A2> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex2<A1, A2>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }

            template <typename A1, typename A2, typename A3>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex3<A1, A2, A3> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex3<A1, A2, A3>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex3<A1, A2, A3> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex3<A1, A2, A3>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }

            template <typename A1, typename A2, typename A3>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex3<A1, A2, A3> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex3<A1, A2, A3>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex3<A1, A2, A3> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex3<A1, A2, A3>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex3<A1, A2, A3> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex3<A1, A2, A3>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex3<A1, A2, A3> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex3<A1, A2, A3>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex4<A1, A2, A3, A4> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex4<A1, A2, A3, A4> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex4<A1, A2, A3, A4> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex4<A1, A2, A3, A4> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex4<A1, A2, A3, A4> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex4<A1, A2, A3, A4> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray copy(Vbo& vbo, const GLenum primType, const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray swap(Vbo& vbo, const GLenum primType, std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray ref(Vbo& vbo, const GLenum primType, const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, IndexArray& indices, CountArray& counts) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vbo, vertices));
                return VertexArray(primType, holder->size(), holder, indices, counts);
            }
            
            size_t vertexCount() const;
            
            void prepare();
            void render();
        private:
            VertexArray(const GLenum primType, const size_t vertexCount, BaseHolder::Ptr holder, const IndexArray& indices, const CountArray& counts);
            VertexArray(const GLenum primType, const size_t vertexCount, BaseHolder::Ptr holder, IndexArray& indices, CountArray& counts);
        };
    }
}

#endif /* defined(__TrenchBroom__VertexArrayRenderer__) */
