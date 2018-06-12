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
        struct FastDirtyRange {
            size_t m_dirtyPos;
            size_t m_dirtySize;
            size_t m_capacity;

            /**
             * New trackers are initially clean.
             */
            explicit FastDirtyRange(size_t initial_capacity);
            FastDirtyRange();

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
         */
        template<typename T>
        class VboBlockHolder {
        protected:
            std::vector<T> m_snapshot;
            FastDirtyRange m_dirtyRanges;
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

                m_dirtyRanges = FastDirtyRange(m_snapshot.size());
                assert(m_dirtyRanges.clean());
                assert((m_block->capacity() / sizeof(T)) == m_dirtyRanges.capacity());
            }

        public:
            VboBlockHolder() : m_snapshot(),
                               m_dirtyRanges(0),
                               m_block(nullptr) {}

            /**
             * NOTE: This destructively moves the contents of `elements` into the Holder.
             */
            VboBlockHolder(std::vector<T> &elements)
                    : m_snapshot(),
                      m_dirtyRanges(elements.size()),
                      m_block(nullptr) {

                const size_t elementsCount = elements.size();
                m_dirtyRanges.markDirty(0, elementsCount);

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
                m_dirtyRanges.expand(newSize);
            }

            T* getPointerToWriteElementsTo(const size_t offsetWithinBlock, const size_t elementCount) {
                assert(offsetWithinBlock + elementCount <= m_snapshot.size());

                // mark dirty range
                m_dirtyRanges.markDirty(offsetWithinBlock, elementCount);

                return m_snapshot.data() + offsetWithinBlock;
            }

            bool prepared() const {
                // NOTE: this returns true if the capacity is 0
                return m_dirtyRanges.clean();
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
                if (m_dirtyRanges.capacity() != (m_block->capacity() / sizeof(T))) {
                    freeBlock();
                    allocateBlock(vbo);
                    assert(prepared());
                    return;
                }

                // otherwise, it's an incremental update of the dirty ranges.
                ActivateVbo activate(vbo);
                MapVboBlock map(m_block);

                if (!m_dirtyRanges.clean()) {
                    const size_t pos = m_dirtyRanges.m_dirtyPos;
                    const size_t size = m_dirtyRanges.m_dirtySize;

                    const size_t bytesFromStart = pos * sizeof(T);
                    m_block->writeArray(bytesFromStart,
                                        m_snapshot.data() + pos,
                                        size);
                }

                m_dirtyRanges = FastDirtyRange(m_snapshot.size());
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

        class BrushIndexHolder {
        private:
            IndexHolder m_indexHolder;
            AllocationTracker m_allocationTracker;
        public:
            BrushIndexHolder();

            bool empty() const;

            std::pair<AllocationTracker::Block*, GLuint*> getPointerToInsertElementsAt(size_t elementCount);

            /**
             * Deletes indices for the given brush. No-op if the brush is not used.
             */
            void zeroElementsWithKey(AllocationTracker::Block* key);

            void render(const PrimType primType) const;
            bool prepared() const;
            void prepare(Vbo& vbo);
        };


        /**
         * A reference-counted handle to a VboBlock (which is a subset of a VBO).
         *
         * It maintains an in-memory copy of the data that was uploaded to the VBO.
         * Support resizing.
         *
         * Copying the IndexArray just increments the reference count,
         * the same underlying buffer is shared between the copies.
         */
        //using IndexArrayPtr = std::shared_ptr<IndexHolder>;


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

        // TODO: This is mostly copied/pasted from BrushIndexHolder.
        // The only difference is deleteVerticesWithKey() doesn't need to zero out
        // the deleted memory in the VBO, while BrushIndexHolder's does.
        class BrushVertexHolder {
        private:
            using Vertex = Renderer::VertexSpecs::P3NT2::Vertex;

            VertexHolder<Vertex> m_vertexHolder;
            AllocationTracker m_allocationTracker;
        public:
            BrushVertexHolder();

            std::pair<AllocationTracker::Block*, Vertex*> getPointerToInsertVerticesAt(size_t vertexCount);

            void deleteVerticesWithKey(AllocationTracker::Block* key);

            // setting up GL attributes
            bool setupVertices();
            void cleanupVertices();

            // uploading the VBO
            bool prepared() const;
            void prepare(Vbo& vbo);
        };

        //using VertexArrayPtr = std::shared_ptr<VertexArrayInterface>;
    }
}

#endif /* BrushRenderer_h */
