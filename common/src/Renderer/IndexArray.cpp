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

#include "Renderer/IndexArray.h"

#include "Renderer/GL.h"
#include "Renderer/VboBlock.h"
#include "Renderer/DirtyRangeTracker.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        class IndexArray::Holder {
            std::vector <Index> m_snapshot;
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
                m_block = vbo.allocateBlock(m_snapshot.size() * sizeof(Index));
                assert(m_block != nullptr);

                MapVboBlock map(m_block);
                m_block->writeElements(0, m_snapshot);

                m_dirtyRanges = DirtyRangeTracker(m_snapshot.size());
                assert(m_dirtyRanges.clean());
                assert((m_block->capacity() / sizeof(Index)) == m_dirtyRanges.capacity());
            }

        public:
            /**
             * NOTE: This destructively moves the contents of `elements` into the Holder.
             */
            Holder(std::vector <Index> &elements)
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

            virtual ~Holder() {
                freeBlock();
            }

            void resize(size_t newSize) {
                m_snapshot.resize(newSize);
                m_dirtyRanges.expand(newSize);
            }

            void writeElements(const size_t offsetWithinBlock, const std::vector <Index> &elements) {
                assert(m_block != nullptr);
                assert(offsetWithinBlock + m_snapshot.size() <= m_snapshot.size());

                // apply update to memory
                std::copy(elements.begin(), elements.end(), m_snapshot.begin() + offsetWithinBlock);

                // mark dirty range
                m_dirtyRanges.markDirty(offsetWithinBlock, elements.size());
            }

            void zeroRange(const size_t offsetWithinBlock, const size_t count) {
                // TODO: It's wasteful to allocate a buffer of zeros. Try glMapBuffer and memset to 0?
                std::vector <Index> zeros;
                zeros.resize(count);

                writeElements(offsetWithinBlock, zeros);
            }

            void render(const PrimType primType, const size_t offset, size_t count) const {
                const GLsizei renderCount = static_cast<GLsizei>(count);
                const GLvoid *renderOffset = reinterpret_cast<GLvoid *>(m_block->offset() + sizeof(Index) * offset);

                glAssert(glDrawElements(primType, renderCount, glType<Index>(), renderOffset));
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
                if (m_dirtyRanges.capacity() != (m_block->capacity() / sizeof(Index))) {
                    freeBlock();
                    allocateBlock(vbo);
                    return;
                }

                // otherwise, it's an incremental update of the dirty ranges.
                ActivateVbo activate(vbo);
                MapVboBlock map(m_block);

                m_dirtyRanges.visitRanges([&](const DirtyRangeTracker::Range& range){
                    // FIXME: Avoid this unnecessary copy
                    std::vector<Index> updatedElements;
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

        IndexArray::IndexArray(std::vector<Index>& indices)
        : m_holder(new IndexArray::Holder(indices)) {}

        IndexArray IndexArray::swap(std::vector<Index>& indices) {
            return IndexArray(indices);
        }

        IndexArray::IndexArray() : m_holder(nullptr) {}

        bool IndexArray::empty() const {
            if (m_holder == nullptr) {
                return true;
            }
            return m_holder->empty();
        }

        void IndexArray::resize(size_t newSize) {
            m_holder->resize(newSize);
        }

        void IndexArray::writeElements(const size_t offsetWithinBlock, const std::vector<Index> &elements) {
            m_holder->writeElements(offsetWithinBlock, elements);
        }

        void IndexArray::zeroRange(const size_t offsetWithinBlock, const size_t count) {
            m_holder->zeroRange(offsetWithinBlock, count);
        }

        void IndexArray::render(const PrimType primType, const size_t offset, size_t count) const {
            m_holder->render(primType, offset, count);
        }

        bool IndexArray::prepared() const {
            return m_holder->prepared();
        }

        void IndexArray::prepare(Vbo& vbo) {
            m_holder->prepare(vbo);
        }
    }
}