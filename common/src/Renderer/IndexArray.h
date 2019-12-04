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

#ifndef IndexArray_h
#define IndexArray_h

#include "Ensure.h"
#include "Renderer/GL.h"
#include "Renderer/Vbo.h"
#include "Renderer/VboBlock.h"

#include <kdl/vector_utils.h>

#include <memory>

namespace TrenchBroom {
    namespace Renderer {
        /**
         * Represents an array of indices. Optionally, multiple instances of this class can share the same data.
         * Index arrays can be copied around without incurring the cost of copying the actual data.
         *
         * An index array can be uploaded into a vertex buffer object by calling the prepare method. Furthermore, an
         * index array can be rendered by calling the provided render method.
         */
        class IndexArray {
        private:
            class BaseHolder {
            public:
                using Ptr = std::shared_ptr<BaseHolder>;
                virtual ~BaseHolder() {}

                virtual size_t indexCount() const = 0;
                virtual size_t sizeInBytes() const = 0;

                virtual void prepare(Vbo& vbo) = 0;
            public:
                void render(PrimType primType, size_t offset, size_t count) const;

                virtual size_t indexOffset() const = 0;
            private:
                virtual void doRender(PrimType primType, size_t offset, size_t count) const = 0;
            };

            template <typename Index>
            class Holder : public BaseHolder {
            protected:
                using IndexList = std::vector<Index>;
            private:
                VboBlock* m_block;
                size_t m_indexCount;
            public:
                size_t indexCount() const override {
                    return m_indexCount;
                }

                size_t sizeInBytes() const override {
                    return sizeof(Index) * m_indexCount;
                }

                virtual void prepare(Vbo& vbo) override {
                    if (m_indexCount > 0 && m_block == nullptr) {
                        ActivateVbo activate(vbo);
                        m_block = vbo.allocateBlock(sizeInBytes());

                        MapVboBlock map(m_block);
                        m_block->writeBuffer(0, doGetIndices());
                    }
                }
            protected:
                Holder(const size_t indexCount) :
                m_block(nullptr),
                m_indexCount(indexCount) {}

                virtual ~Holder() override {
                    if (m_block != nullptr) {
                        m_block->free();
                        m_block = nullptr;
                    }
                }
            private:
                size_t indexOffset() const override {
                    if (m_indexCount == 0)
                        return 0;
                    ensure(m_block != nullptr, "block is null");
                    return m_block->offset();

                }

                void doRender(PrimType primType, size_t offset, size_t count) const override {
                    const auto renderCount  = static_cast<GLsizei>(count);
                    const auto indexType     = glType<Index>();
                    const auto* renderOffset = reinterpret_cast<GLvoid*>(indexOffset() + sizeof(Index) * offset);

                    glAssert(glDrawElements(primType, renderCount, indexType, renderOffset));
                }
            private:
                virtual const IndexList& doGetIndices() const = 0;
            };

            template <typename Index>
            class CopyHolder : public Holder<Index> {
            public:
                using IndexList = typename Holder<Index>::IndexList;
            private:
                IndexList m_indices;
            public:
                CopyHolder(const IndexList& indices) :
                Holder<Index>(indices.size()),
                m_indices(indices) {}

                void prepare(Vbo& vbo) {
                    Holder<Index>::prepare(vbo);
                    kdl::clear_to_zero(m_indices);
                }
            private:
                const IndexList& doGetIndices() const {
                    return m_indices;
                }
            };

            template <typename Index>
            class SwapHolder : public Holder<Index> {
            public:
                using IndexList = typename Holder<Index>::IndexList;
            private:
                IndexList m_indices;
            public:
                SwapHolder(IndexList& indices) :
                Holder<Index>(indices.size()),
                m_indices(0) {
                    using std::swap;
                    swap(m_indices, indices);
                }

                void prepare(Vbo& vbo) override {
                    Holder<Index>::prepare(vbo);
                    kdl::clear_to_zero(m_indices);
                }
            private:
                const IndexList& doGetIndices() const override {
                    return m_indices;
                }
            };

            template <typename Index>
            class RefHolder : public Holder<Index> {
            public:
                using IndexList = typename Holder<Index>::IndexList;
            private:
                const IndexList& m_indices;
            public:
                RefHolder(const IndexList& indices) :
                Holder<Index>(indices.size()),
                m_indices(indices) {}
            private:
                const IndexList& doGetIndices() const {
                    return m_indices;
                }
            };
        private:
            BaseHolder::Ptr m_holder;
            bool m_prepared;
        public:
            /**
             * Creates a new empty index array.
             */
            IndexArray();

            /**
             * Creates a new index array by copying the given indices. After this operation, the given vector of
             * indices is left unchanged.
             *
             * @tparam Index the index type
             * @param indices the indices to copy
             * @return the index array
             */
            template <typename Index>
            static IndexArray copy(const std::vector<Index>& indices) {
                return IndexArray(BaseHolder::Ptr(new CopyHolder<Index>(indices)));
            }

            /**
             * Creates a new index array by swapping the contents of the given indices. After this operation, the given
             * vector of indices is empty.
             *
             * @tparam Index the index type
             * @param indices the indices to swap
             * @return the index array
             */
            template <typename Index>
            static IndexArray swap(std::vector<Index>& indices) {
                return IndexArray(BaseHolder::Ptr(new SwapHolder<Index>(indices)));
            }

            /**
             * Creates a new index array by referencing the contents of the given indices. After this operation, the
             * given vector of indices is left unchanged. Since this index array will only store a reference to the
             * given vector, changes to the given vector are reflected in this array.
             *
             * A caller must ensure that this index array does not outlive the given vector of indices.
             *
             * @tparam Index the index type
             * @param indices the indices to copy
             * @return the index array
             */
            template <typename Index>
            static IndexArray ref(const std::vector<Index>& indices) {
                return IndexArray(BaseHolder::Ptr(new RefHolder<Index>(indices)));
            }

            /**
             * Indicates whether this index array is empty.
             *
             * @return true if this index array is empty and false otherwise
             */
            bool empty() const;

            /**
             * Returns the size of this index array in bytes.
             *
             * @return the size of this index array in bytes
             */
            size_t sizeInBytes() const;

            /**
             * Returns the number of indices in this index array.
             *
             * @return the number of indices in this index array
             */
            size_t indexCount() const;

            /**
             * Indicates whether this index array was prepared.
             *
             * @return true if this index array was prepared
             */
            bool prepared() const;

            /**
             * Prepares this index array by uploading its contents into the given vertex buffer object.
             *
             * @param vbo the vertex buffer object to upload to
             */
            void prepare(Vbo& vbo);

            /**
             * Renders a range of primitives of the given type using the indices stored in this index array. Assumes that
             * an appropriate vertex array has been set up that contains the actual vertex data.
             *
             * @param primType the type of primitive to render
             * @param offset the offset of the range of indices to render
             * @param count the number of indices to render
             */
            void render(PrimType primType, size_t offset, size_t count) const;
        private:
            explicit IndexArray(BaseHolder::Ptr holder);
        };
    }
}

#endif /* IndexArray_h */
