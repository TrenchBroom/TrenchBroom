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

#ifndef BrushRendererArray_h
#define BrushRendererArray_h

#include "Renderer/VertexSpec.h"
#include "Renderer/Vbo.h"
#include "Renderer/AllocationTracker.h"
#include "Renderer/GL.h"
#include "Renderer/VboBlock.h"

#include <algorithm>
#include <vector>
#include <cassert>
#include <unordered_map>

namespace TrenchBroom {
    namespace Model {
        class Brush;
    }
    namespace Renderer {
        struct DirtyRangeTracker {
            size_t m_dirtyPos;
            size_t m_dirtySize;
            size_t m_capacity;

            /**
             * New trackers are initially clean.
             */
            explicit DirtyRangeTracker(size_t initial_capacity);
            DirtyRangeTracker();

            /**
             * Expanding marks the new range as dirty.
             */
            void expand(size_t newcap);
            size_t capacity() const;
            void markDirty(size_t pos, size_t size);
            bool clean() const;
        };

        /**
         * Wrapper around a std::vector<T> and VboBlock.
         *
         * Non-copyable; meant to be held in a std::shared_ptr.
         * Able to be resized, and handles copying edits made in the local std::vector to the VBO.
         *
         * Currently uses a single range to track the modified region which might upload much more than necessary;
         * it might be worth mapping the VBO and editing it directly.
         */
        template<typename T>
        class VboBlockHolder {
        protected:
            std::vector<T> m_snapshot;
            DirtyRangeTracker m_dirtyRange;
            VboBlock *m_block;

        private:
            void freeBlock() {
                if (m_block != nullptr) {
                    m_block->free();
                    m_block = nullptr;
                }
            }

            void allocateBlock(Vbo &vbo) {
                assert(m_block == nullptr);

                ActivateVbo activate(vbo);
                m_block = vbo.allocateBlock(m_snapshot.size() * sizeof(T));
                assert(m_block != nullptr);

                MapVboBlock map(m_block);
                m_block->writeElements(0, m_snapshot);

                m_dirtyRange = DirtyRangeTracker(m_snapshot.size());
                assert(m_dirtyRange.clean());
                assert((m_block->capacity() / sizeof(T)) == m_dirtyRange.capacity());
            }

        public:
            VboBlockHolder() : m_snapshot(),
                               m_dirtyRange(0),
                               m_block(nullptr) {}

            /**
             * NOTE: This destructively moves the contents of `elements` into the Holder.
             */
            VboBlockHolder(std::vector<T> &elements)
                    : m_snapshot(),
                      m_dirtyRange(elements.size()),
                      m_block(nullptr) {

                const size_t elementsCount = elements.size();
                m_dirtyRange.markDirty(0, elementsCount);

                elements.swap(m_snapshot);

                // we allow zero elements.
                if (!empty()) {
                    assert(!prepared());
                }
            }

            VboBlockHolder(const VboBlockHolder& other) = delete;

            virtual ~VboBlockHolder() {
                freeBlock();
            }

            void resize(size_t newSize) {
                m_snapshot.resize(newSize);
                m_dirtyRange.expand(newSize);
            }

            T* getPointerToWriteElementsTo(const size_t offsetWithinBlock, const size_t elementCount) {
                assert(offsetWithinBlock + elementCount <= m_snapshot.size());

                // mark dirty range
                m_dirtyRange.markDirty(offsetWithinBlock, elementCount);

                return m_snapshot.data() + offsetWithinBlock;
            }

            bool prepared() const {
                // NOTE: this returns true if the capacity is 0
                return m_dirtyRange.clean();
            }

            void prepare(Vbo& vbo) {
                if (empty()) {
                    assert(prepared());
                    return;
                }
                if (prepared()) {
                    return;
                }

                // first ever upload?
                if (m_block == nullptr) {
                    allocateBlock(vbo);
                    assert(prepared());
                    return;
                }

                // resize?
                if (m_dirtyRange.capacity() != (m_block->capacity() / sizeof(T))) {
                    freeBlock();
                    allocateBlock(vbo);
                    assert(prepared());
                    return;
                }

                // otherwise, it's an incremental update of the dirty ranges.
                ActivateVbo activate(vbo);
                MapVboBlock map(m_block);

                if (!m_dirtyRange.clean()) {
                    const size_t pos = m_dirtyRange.m_dirtyPos;
                    const size_t size = m_dirtyRange.m_dirtySize;

                    const size_t bytesFromStart = pos * sizeof(T);
                    m_block->writeArray(bytesFromStart,
                                        m_snapshot.data() + pos,
                                        size);
                }

                m_dirtyRange = DirtyRangeTracker(m_snapshot.size());
                assert(prepared());
            }

            bool empty() const {
                return m_snapshot.empty();
            }

            size_t size() const {
                return m_snapshot.size();
            }
        };

        class IndexHolder : public VboBlockHolder<GLuint> {
        public:
            using Index = GLuint;

            IndexHolder();
            /**
             * NOTE: This destructively moves the contents of `elements` into the Holder.
             */
            IndexHolder(std::vector<Index>& elements);
            void zeroRange(const size_t offsetWithinBlock, const size_t count);
            void render(const PrimType primType, const size_t offset, size_t count) const;

            static std::shared_ptr<IndexHolder> swap(std::vector<Index>& elements);
        };

        /**
         * VboBlock handle that supports dynamically allocating ranges of indices, grows as needed, and also
         * supports freeing allocations and zeroing the corresponding indicies so they become degenerate primitives.
         */
        class BrushIndexArray {
        private:
            IndexHolder m_indexHolder;
            AllocationTracker m_allocationTracker;
        public:
            BrushIndexArray();

            bool empty() const;

            /**
             * Call this to request writing the given number of indices.
             *
             * The VboBlock will be expanded if needed to accommodate the allocation.
             *
             * Returns a AllocationTracker::Block pointer which can be used later in a call to zeroElementsWithKey(),
             * and also a GLuint pointer where the caller should write `elementCount` GLuint's.
             */
            std::pair<AllocationTracker::Block*, GLuint*> getPointerToInsertElementsAt(size_t elementCount);

            /**
             * Deletes indices for the given brush and marks the allocation as free.
             */
            void zeroElementsWithKey(AllocationTracker::Block* key);

            void render(const PrimType primType) const;
            bool prepared() const;
            void prepare(Vbo& vbo);
        };

        class VertexArrayInterface {
        public:
            virtual ~VertexArrayInterface() = 0;
            virtual bool setupVertices() = 0;
            virtual void prepareVertices(Vbo& vbo) = 0;
            virtual void cleanupVertices() = 0;
        };

        template<typename V>
        class VertexHolder : public VboBlockHolder<V>, public VertexArrayInterface {
        public:
            VertexHolder()
                    : VboBlockHolder<V>() {}

            VertexHolder(std::vector<V>& elements)
                    : VboBlockHolder<V>(elements) {}

            bool setupVertices() override {
                ensure(VboBlockHolder<V>::m_block != nullptr, "block is null");
                V::Spec::setup(VboBlockHolder<V>::m_block->offset());
                return true;
            }
            void prepareVertices(Vbo& vbo) override {
                VboBlockHolder<V>::prepare(vbo);
            }
            void cleanupVertices() override {
                V::Spec::cleanup();
            }

            static std::shared_ptr<VertexHolder<V>> swap(std::vector<V>& elements) {
                return std::make_shared<VertexHolder<V>>(elements);
            }
        };

        /**
         * Same as BrushIndexArray but for vertices instead of indices.
         * The only difference is deleteVerticesWithKey() doesn't need to zero out
         * the deleted memory in the VBO, while BrushIndexArray's does.
         */
        class BrushVertexArray {
        private:
            using Vertex = Renderer::VertexSpecs::P3NT2::Vertex;

            VertexHolder<Vertex> m_vertexHolder;
            AllocationTracker m_allocationTracker;
        public:
            BrushVertexArray();

            /**
             * Call this to request writing the given number of vertices.
             *
             * The VboBlock will be expanded if needed to accommodate the allocation.
             *
             * Returns a AllocationTracker::Block pointer which can be used later in a call to deleteVerticesWithKey(),
             * and also a Vertex pointer where the caller should write `elementCount` Vertex objects.
             */
            std::pair<AllocationTracker::Block*, Vertex*> getPointerToInsertVerticesAt(size_t vertexCount);

            void deleteVerticesWithKey(AllocationTracker::Block* key);

            // setting up GL attributes
            bool setupVertices();
            void cleanupVertices();

            // uploading the VBO
            bool prepared() const;
            void prepare(Vbo& vbo);
        };
    }
}

#endif /* BrushRenderer_h */
