/*
 Copyright (C) 2010-2017 Kristian Duske

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
        /**
         * Represents an array of vertices. Optionally, multiple instances of this class can share the same data.
         * Vertex arrays can be copied around without incurring the cost of copying the actual data.
         *
         * A vertex array can be uploaded into a vertex buffer object by calling the prepare method. Furthermore, a
         * vertex array can be rendered by calling one of the provided render methods.
         */
        class VertexArray {
        private:
            class BaseHolder {
            public:
                using Ptr = std::shared_ptr<BaseHolder>;
                virtual ~BaseHolder();

                virtual size_t vertexCount() const = 0;
                virtual size_t sizeInBytes() const = 0;

                virtual void prepare(Vbo& vbo) = 0;
                virtual void setup() = 0;
                virtual void cleanup() = 0;
            };

            template <typename VertexSpec>
            class Holder : public BaseHolder {
            private:
                using VertexList = typename VertexSpec::Vertex::List;
            private:
                VboBlock* m_block;
                size_t m_vertexCount;
            public:
                size_t vertexCount() const override {
                    return m_vertexCount;
                }

                size_t sizeInBytes() const override {
                    return VertexSpec::Size * m_vertexCount;
                }

                void prepare(Vbo& vbo) override {
                    if (m_vertexCount > 0 && m_block == nullptr) {
                        ActivateVbo activate(vbo);
                        m_block = vbo.allocateBlock(sizeInBytes());

                        MapVboBlock map(m_block);
                        m_block->writeBuffer(0, doGetVertices());
                    }
                }

                void setup() override {
                    ensure(m_block != nullptr, "block is null");
                    VertexSpec::setup(m_block->offset());
                }

                void cleanup() override {
                    VertexSpec::cleanup();
                }
            protected:
                explicit Holder(const size_t vertexCount) :
                m_block(nullptr),
                m_vertexCount(vertexCount) {}

                ~Holder() override {
                    if (m_block != nullptr) {
                        m_block->free();
                        m_block = nullptr;
                    }
                }
            private:
                virtual const VertexList& doGetVertices() const = 0;
            };

            template <typename VertexSpec>
            class CopyHolder : public Holder<VertexSpec> {
            public:
                using VertexList = typename VertexSpec::Vertex::List;
            private:
                VertexList m_vertices;
            public:
                explicit CopyHolder(const VertexList& vertices) :
                Holder<VertexSpec>(vertices.size()),
                m_vertices(vertices) {}

                void prepare(Vbo& vbo) override {
                    Holder<VertexSpec>::prepare(vbo);
                    VectorUtils::clearToZero(m_vertices);
                }
            private:
                const VertexList& doGetVertices() const override {
                    return m_vertices;
                }
            };

            template <typename VertexSpec>
            class SwapHolder : public Holder<VertexSpec> {
            public:
                using VertexList = typename VertexSpec::Vertex::List;
            private:
                VertexList m_vertices;
            public:
                explicit SwapHolder(VertexList& vertices) :
                Holder<VertexSpec>(vertices.size()),
                m_vertices(0) {
                    using std::swap;
                    swap(m_vertices, vertices);
                }

                void prepare(Vbo& vbo) override {
                    Holder<VertexSpec>::prepare(vbo);
                    VectorUtils::clearToZero(m_vertices);
                }
            private:
                const VertexList& doGetVertices() const override {
                    return m_vertices;
                }
            };

            template <typename VertexSpec>
            class RefHolder : public Holder<VertexSpec> {
            public:
                using VertexList = typename VertexSpec::Vertex::List;
            private:
                const VertexList& m_vertices;
            public:
                explicit RefHolder(const VertexList& vertices) :
                Holder<VertexSpec>(vertices.size()),
                m_vertices(vertices) {}
            private:
                const VertexList& doGetVertices() const override {
                    return m_vertices;
                }
            };
        private:
            BaseHolder::Ptr m_holder;
            bool m_prepared;
            bool m_setup;
        public:
            /**
             * Creates a new empty vertex array.
             */
            VertexArray();

            /**
             * Creates a new vertex array by copying the given vertices. After this operation, the given vector of
             * vertices is left unchanged.
             *
             * @tparam A1 the vertex attribute type
             * @param vertices the vertices to copy
             * @return the vertex array
             */
            template <typename A1>
            static VertexArray copy(const std::vector<Vertex1<A1>>& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex1<A1>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by swapping the contents of the given vertices. After this operation, the given
             * vector of vertices is empty.
             *
             * @tparam A1 the vertex attribute type
             * @param vertices the vertices to swap
             * @return the vertex array
             */
            template <typename A1>
            static VertexArray swap(std::vector<Vertex1<A1> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex1<A1>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by referencing the contents of the given vertices. After this operation, the
             * given vector of vertices is left unchanged. Since this vertex array will only store a reference to the
             * given vector, changes to the given vector are reflected in this array.
             *
             * A caller must ensure that this vertex array does not outlive the given vector of vertices.
             *
             * @tparam A1 the vertex attribute type
             * @param vertices the vertices to reference
             * @return the vertex array
             */
            template <typename A1>
            static VertexArray ref(const std::vector<Vertex1<A1> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex1<A1>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by copying the given vertices. After this operation, the given vector of
             * vertices is left unchanged.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @param vertices the vertices to copy
             * @return the vertex array
             */
            template <typename A1, typename A2>
            static VertexArray copy(const std::vector<Vertex2<A1, A2> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex2<A1, A2>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by swapping the contents of the given vertices. After this operation, the given
             * vector of vertices is empty.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @param vertices the vertices to swap
             * @return the vertex array
             */
            template <typename A1, typename A2>
            static VertexArray swap(std::vector<Vertex2<A1, A2> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex2<A1, A2>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by referencing the contents of the given vertices. After this operation, the
             * given vector of vertices is left unchanged. Since this vertex array will only store a reference to the
             * given vector, changes to the given vector are reflected in this array.
             *
             * A caller must ensure that this vertex array does not outlive the given vector of vertices.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @param vertices the vertices to reference
             * @return the vertex array
             */
            template <typename A1, typename A2>
            static VertexArray ref(const std::vector<Vertex2<A1, A2> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex2<A1, A2>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by copying the given vertices. After this operation, the given vector of
             * vertices is left unchanged.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @param vertices the vertices to copy
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3>
            static VertexArray copy(const std::vector<Vertex3<A1, A2, A3> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by swapping the contents of the given vertices. After this operation, the given
             * vector of vertices is empty.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @param vertices the vertices to swap
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3>
            static VertexArray swap(std::vector<Vertex3<A1, A2, A3> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by referencing the contents of the given vertices. After this operation, the
             * given vector of vertices is left unchanged. Since this vertex array will only store a reference to the
             * given vector, changes to the given vector are reflected in this array.
             *
             * A caller must ensure that this vertex array does not outlive the given vector of vertices.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @param vertices the vertices to reference
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3>
            static VertexArray ref(const std::vector<Vertex3<A1, A2, A3> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex3<A1, A2, A3>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by copying the given vertices. After this operation, the given vector of
             * vertices is left unchanged.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @tparam A4 the fourth vertex attribute type
             * @param vertices the vertices to copy
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray copy(const std::vector<Vertex4<A1, A2, A3, A4> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by swapping the contents of the given vertices. After this operation, the given
             * vector of vertices is empty.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @tparam A4 the fourth vertex attribute type
             * @param vertices the vertices to swap
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray swap(std::vector<Vertex4<A1, A2, A3, A4> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by referencing the contents of the given vertices. After this operation, the
             * given vector of vertices is left unchanged. Since this vertex array will only store a reference to the
             * given vector, changes to the given vector are reflected in this array.
             *
             * A caller must ensure that this vertex array does not outlive the given vector of vertices.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @tparam A4 the fourth vertex attribute type
             * @param vertices the vertices to reference
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3, typename A4>
            static VertexArray ref(const std::vector<Vertex4<A1, A2, A3, A4> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex4<A1, A2, A3, A4>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by copying the given vertices. After this operation, the given vector of
             * vertices is left unchanged.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @tparam A4 the fourth vertex attribute type
             * @tparam A5 the fifth vertex attribute type
             * @param vertices the vertices to copy
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray copy(const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices) {
                BaseHolder::Ptr holder(new CopyHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by swapping the contents of the given vertices. After this operation, the given
             * vector of vertices is empty.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @tparam A4 the fourth vertex attribute type
             * @tparam A5 the fifth vertex attribute type
             * @param vertices the vertices to swap
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray swap(std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices) {
                BaseHolder::Ptr holder(new SwapHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Creates a new vertex array by referencing the contents of the given vertices. After this operation, the
             * given vector of vertices is left unchanged. Since this vertex array will only store a reference to the
             * given vector, changes to the given vector are reflected in this array.
             *
             * A caller must ensure that this vertex array does not outlive the given vector of vertices.
             *
             * @tparam A1 the first vertex attribute type
             * @tparam A2 the second vertex attribute type
             * @tparam A3 the third vertex attribute type
             * @tparam A4 the fourth vertex attribute type
             * @tparam A5 the fifth vertex attribute type
             * @param vertices the vertices to reference
             * @return the vertex array
             */
            template <typename A1, typename A2, typename A3, typename A4, typename A5>
            static VertexArray ref(const std::vector<Vertex5<A1, A2, A3, A4, A5> >& vertices) {
                BaseHolder::Ptr holder(new RefHolder<typename Vertex5<A1, A2, A3, A4, A5>::Spec>(vertices));
                return VertexArray(holder);
            }

            /**
             * Indicates whether this vertex array is empty.
             *
             * @return true if this vertex array is empty and false otherwise
             */
            bool empty() const;

            /**
             * Returns the size of this vertex array in bytes.
             *
             * @return the size of this vertex array in bytes
             */
            size_t sizeInBytes() const;

            /**
             * Returns the numnber of vertices stored in this vertex array.
             *
             * @return the number of vertices stored in this vertex array
             */
            size_t vertexCount() const;

            /**
             * Indicates whether this vertex array way prepared. Preparing a vertex array uploads its data into a
             * vertex buffer object.
             *
             * @return true if this vertex array was prepared and false otherwise
             */
            bool prepared() const;

            /**
             * Prepares this vertex array by uploading its contents into the given vertex buffer object.
             *
             * @param vbo the vertex buffer object to upload the contents of this vertex array into
             */
            void prepare(Vbo& vbo);

            /**
             * Sets this vertex array up for rendering. If this vertex array is only rendered once, then there is no
             * need to call this method (or the corresponding cleanup method), since the render methods will perform
             * setup and cleanup automatically unless the vertex array is already set up when the render method is called.
             *
             * In the case the the vertex array was already setup before a render method was called, the render method
             * will skip the setup and cleanup, and it is the callers' responsibility to perform proper cleanup.
             *
             * It is only useful to perform setup and cleanup for a caller if the caller intends to issue multiple
             * render calls to this vertex array.
             */
            bool setup();

            /**
             * Renders this vertex array as a range of primitives of the given type.
             *
             * @param primType the primitive type to render
             */
            void render(PrimType primType);

            /**
             * Renders a sub range of this vertex array as a range of primitives of the given type.
             *
             * @param primType the primitive type to render
             * @param index the index of the first vertex in this vertex array to render
             * @param count the number of vertices to render
             */
            void render(PrimType primType, GLint index, GLsizei count);

            /**
             * Renders a number of sub ranges of this vertex array as ranges of primitives of the given type. The given
             * indices array contains the start indices of the ranges to render, while the given counts array contains
             * the length of the range. Both the indices and counts must contain at least primCount elements.
             *
             * @param primType the primitive type to render
             * @param indices the start indices of the ranges to render
             * @param counts the lengths of the ranges to render
             * @param primCount the number of ranges to render
             */
            void render(PrimType primType, const GLIndices& indices, const GLCounts& counts, GLint primCount);

            /**
             * Renders a number of primitives of the given type, the vertices of which are indicates by the given
             * index array.
             *
             * @param primType the primitive type to render
             * @param indices the indices of the vertices to render
             * @param count the number of vertices to render
             */
            void render(PrimType primType, const GLIndices& indices, GLsizei count);
            void cleanup();
        private:
            explicit VertexArray(BaseHolder::Ptr holder);
        };
    }
}

#endif /* defined(TrenchBroom_VertexArrayRenderer) */
