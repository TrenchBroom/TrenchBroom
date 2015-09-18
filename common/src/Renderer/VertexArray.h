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
        class BaseHolder {
        protected:
            typedef std::vector<GLint> IndexArray;
            typedef std::vector<GLsizei> CountArray;
        public:
            typedef std::tr1::shared_ptr<BaseHolder> Ptr;
            virtual ~BaseHolder() {}
            
            virtual size_t vertexCount() const = 0;
            virtual size_t size() const = 0;

            virtual const IndexArray& indices() const = 0;
            virtual const CountArray& counts() const = 0;
            
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
            
            size_t size() const {
                return VertexSpec::Size * m_vertexCount;
            }
            
            virtual void prepare(Vbo& vbo) {
                assert(m_vertexCount > 0);
                assert(m_block == NULL);

                SetVboState mapVbo(vbo);
                mapVbo.mapped();
                
                const VertexList& vertices = doGetVertices();
                m_block = vbo.allocateBlock(size());
                m_block->writeBuffer(0, vertices);
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
            BaseHolder::IndexArray m_indices;
            BaseHolder::CountArray m_counts;
        public:
            CopyHolder(const VertexList& vertices, const BaseHolder::IndexArray& indices, const BaseHolder::CountArray& counts) :
            Holder<VertexSpec>(vertices.size()),
            m_vertices(vertices),
            m_indices(indices),
            m_counts(counts) {}
            
            const BaseHolder::IndexArray& indices() const {
                return m_indices;
            }
            
            const BaseHolder::CountArray& counts() const {
                return m_counts;
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
        class SwapHolder : public Holder<VertexSpec> {
        public:
            typedef typename VertexSpec::Vertex::List VertexList;
        private:
            VertexList m_vertices;
            BaseHolder::IndexArray m_indices;
            BaseHolder::CountArray m_counts;
        public:
            SwapHolder(VertexList& vertices, BaseHolder::IndexArray& indices, BaseHolder::CountArray& counts) :
            Holder<VertexSpec>(vertices.size()),
            m_vertices(0),
            m_indices(0),
            m_counts(0) {
                using std::swap;
                swap(m_vertices, vertices);
                swap(m_indices, indices);
                swap(m_counts, counts);
            }
            
            const BaseHolder::IndexArray& indices() const {
                return m_indices;
            }
            
            const BaseHolder::CountArray& counts() const {
                return m_counts;
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
            const BaseHolder::IndexArray& m_indices;
            const BaseHolder::CountArray& m_counts;
        public:
            RefHolder(const VertexList& vertices, const BaseHolder::IndexArray& indices, const BaseHolder::CountArray& counts) :
            Holder<VertexSpec>(vertices.size()),
            m_vertices(vertices),
            m_indices(indices),
            m_counts(counts) {}

            const BaseHolder::IndexArray& indices() const {
                return m_indices;
            }
            
            const BaseHolder::CountArray& counts() const {
                return m_counts;
            }
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
            static IndexArray SwappableIndexArray;
            static CountArray SwappableCountArray;
        private:
            GLenum m_primType;
            BaseHolder::Ptr m_holder;
            bool m_prepared;
        public:
            explicit VertexArray();
            
            template <typename A1>
            static VertexArray copy(const GLenum primType, const std::vector<Vertex1<A1> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex1<A1>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1>
            static VertexArray swap(const GLenum primType, std::vector<Vertex1<A1> >& vertices, IndexArray& indices = SwappableIndexArray, CountArray& counts = SwappableCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex1<A1>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }

            template <typename A1>
            static VertexArray ref(const GLenum primType, const std::vector<Vertex1<A1> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex1<A1>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }

            template <typename A1, typename A2>
            static VertexArray copy(const GLenum primType, const std::vector<Vertex2<A1, A2> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex2<A1, A2>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }

            template <typename A1, typename A2>
            static VertexArray swap(const GLenum primType, std::vector<Vertex2<A1, A2> >& vertices, IndexArray& indices = SwappableIndexArray, CountArray& counts = SwappableCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex2<A1, A2>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2>
            static VertexArray ref(const GLenum primType, const std::vector<Vertex2<A1, A2> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex2<A1, A2>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray copy(const GLenum primType, const std::vector<Vertex3<A1, A2, A3> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyIndexArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }

            template <typename A1, typename A2, typename A3>
            static VertexArray swap(const GLenum primType, std::vector<Vertex3<A1, A2, A3> >& vertices, IndexArray& indices = SwappableIndexArray, CountArray& counts = SwappableCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2, typename A3>
            static VertexArray ref(const GLenum primType, const std::vector<Vertex3<A1, A2, A3> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray copy(const GLenum primType, const std::vector<Vertex4<A1, A2, A3, A4> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }

            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray swap(const GLenum primType, std::vector<Vertex4<A1, A2, A3, A4> >& vertices, IndexArray& indices = SwappableIndexArray, CountArray& counts = SwappableCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray ref(const GLenum primType, const std::vector<Vertex4<A1, A2, A3, A4> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray copy(const GLenum primType, const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray swap(const GLenum primType, std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, IndexArray& indices = SwappableIndexArray, CountArray& counts = SwappableCountArray) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }
            
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray ref(const GLenum primType, const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices, const IndexArray& indices = EmptyIndexArray, const CountArray& counts = EmptyCountArray) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices, indices, counts));
                return VertexArray(primType, holder);
            }

            VertexArray& operator= (VertexArray other);
            friend void swap(VertexArray& left, VertexArray& right);
            
            size_t size() const;
            size_t vertexCount() const;
            
            bool prepared() const;
            void prepare(Vbo& vbo);
            void render();
        private:
            VertexArray(const GLenum primType, BaseHolder::Ptr holder);
        };
    }
}

#endif /* defined(TrenchBroom_VertexArrayRenderer) */
