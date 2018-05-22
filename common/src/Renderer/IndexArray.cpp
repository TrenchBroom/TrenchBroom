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

namespace TrenchBroom {
    namespace Renderer {

        // IndexHolder

        IndexHolder::IndexHolder(std::vector<Index> &elements)
                : VboBlockHolder<Index>(elements) {}

        void IndexHolder::zeroRange(const size_t offsetWithinBlock, const size_t count) {
            // TODO: It's wasteful to allocate a buffer of zeros. Try glMapBuffer and memset to 0?
            std::vector<Index> zeros;
            zeros.resize(count);

            writeElements(offsetWithinBlock, zeros);
        }

        void IndexHolder::render(const PrimType primType, const size_t offset, size_t count) const {
            const GLsizei renderCount = static_cast<GLsizei>(count);
            const GLvoid *renderOffset = reinterpret_cast<GLvoid *>(m_block->offset() + sizeof(Index) * offset);

            glAssert(glDrawElements(primType, renderCount, glType<Index>(), renderOffset));
        }

        // IndexArray

        IndexArray::IndexArray(std::vector<Index>& indices)
        : m_holder(new IndexHolder(indices)) {}

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
