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

#include "Renderer/Vbo.h"
#include "Renderer/DirtyRangeTracker.h"
#include "Renderer/GL.h"
#include "Renderer/VboBlock.h"

#include <algorithm>
#include <vector>
#include <cassert>

namespace TrenchBroom {
    namespace Renderer {

        /**
         * Wrapper around a std::vector<T> and VboBlock.
         */
        template<typename T>
        class VboBlockHolder {
        protected:
            std::vector<T> m_snapshot;
            DirtyRangeTracker m_dirtyRanges;
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

                m_dirtyRanges = DirtyRangeTracker(m_snapshot.size());
                assert(m_dirtyRanges.clean());
                assert((m_block->capacity() / sizeof(T)) == m_dirtyRanges.capacity());
            }

        public:
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

            virtual ~VboBlockHolder() {
                freeBlock();
            }

            void resize(size_t newSize) {
                m_snapshot.resize(newSize);
                m_dirtyRanges.expand(newSize);
            }

            void writeElements(const size_t offsetWithinBlock, const std::vector<T>& elements) {
                assert(m_block != nullptr);
                assert(offsetWithinBlock + m_snapshot.size() <= m_snapshot.size());

                // apply update to memory
                std::copy(elements.begin(), elements.end(), m_snapshot.begin() + offsetWithinBlock);

                // mark dirty range
                m_dirtyRanges.markDirty(offsetWithinBlock, elements.size());
            }

            bool prepared() const {
                // NOTE: this returns true if the capacity is 0
                return m_dirtyRanges.clean();
            }

            void prepare(Vbo& vbo) {
                if (empty()) {
                    return;
                }
                if (prepared()) {
                    return;
                }

                // first ever upload?
                if (m_block == nullptr) {
                    allocateBlock(vbo);
                    return;
                }

                // resize?
                if (m_dirtyRanges.capacity() != (m_block->capacity() / sizeof(T))) {
                    freeBlock();
                    allocateBlock(vbo);
                    return;
                }

                // otherwise, it's an incremental update of the dirty ranges.
                ActivateVbo activate(vbo);
                MapVboBlock map(m_block);

                m_dirtyRanges.visitRanges([&](const DirtyRangeTracker::Range& range){
                    // FIXME: Avoid this unnecessary copy
                    std::vector<T> updatedElements;
                    updatedElements.resize(range.size);

                    std::copy(m_snapshot.cbegin() + range.pos,
                              m_snapshot.cbegin() + range.pos + range.size,
                              updatedElements.begin());

                    m_block->writeElements(range.pos, updatedElements);
                });
            }

            bool empty() const {
                return m_snapshot.empty();
            }
        };

        class IndexHolder : public VboBlockHolder<GLuint> {
        private:
            using Index = GLuint;
        public:
            /**
             * NOTE: This destructively moves the contents of `elements` into the Holder.
             */
            IndexHolder(std::vector<Index> &elements);
            void zeroRange(const size_t offsetWithinBlock, const size_t count);
            void render(const PrimType primType, const size_t offset, size_t count) const;
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
        class IndexArray {
        private:
            using Index = GLuint;

            std::shared_ptr<IndexHolder> m_holder;

            IndexArray(std::vector<Index>& indices);
        public:
            /**
             * creates an empty IndexArray
             */
            IndexArray();

            bool empty() const;

            static IndexArray swap(std::vector<Index>& indices);

            void resize(size_t newSize);

            void writeElements(const size_t offsetWithinBlock, const std::vector<Index> &elements);

            void zeroRange(const size_t offsetWithinBlock, const size_t count);

            void render(const PrimType primType, const size_t offset, size_t count) const;

            /**
             * Returns true if all of the edits have been uploaded to the VBO.
             */
            bool prepared() const;

            /**
             * Uploads pending changes to the VBO, allocating a VboBlock if needed.
             */
            void prepare(Vbo& vbo);
        };
    }
}

#endif /* IndexArray_h */
