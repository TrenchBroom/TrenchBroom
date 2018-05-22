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

        std::shared_ptr<IndexHolder> IndexHolder::swap(std::vector<IndexHolder::Index> &elements) {
            return std::make_shared<IndexHolder>(elements);
        }
    }
}
