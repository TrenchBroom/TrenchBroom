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

#include <cassert>
#include <algorithm>
#include <cstring>

namespace TrenchBroom {
    // BrushIndexHolder

    namespace Renderer {

        // IndexHolder

        IndexHolder::IndexHolder() : VboBlockHolder<Index>() {}

        IndexHolder::IndexHolder(std::vector<Index> &elements)
                : VboBlockHolder<Index>(elements) {}

        void IndexHolder::zeroRange(const size_t offsetWithinBlock, const size_t count) {
            Index* dest = getPointerToWriteElementsTo(offsetWithinBlock, count);
            std::memset(dest, 0, count * sizeof(Index));
        }

        void IndexHolder::render(const PrimType primType, const size_t offset, size_t count) const {
            const GLsizei renderCount = static_cast<GLsizei>(count);
            const GLvoid *renderOffset = reinterpret_cast<GLvoid *>(m_block->offset() + sizeof(Index) * offset);

            glAssert(glDrawElements(primType, renderCount, glType<Index>(), renderOffset));
        }

        std::shared_ptr<IndexHolder> IndexHolder::swap(std::vector<IndexHolder::Index> &elements) {
            return std::make_shared<IndexHolder>(elements);
        }

        VertexArrayInterface::~VertexArrayInterface() {}

        // BrushIndexHolder

        BrushIndexHolder::BrushIndexHolder() : m_indexHolder(),
                                               m_allocationTracker(0) {}

        bool BrushIndexHolder::empty() const {
            return m_indexHolder.empty();
        }

        std::pair<AllocationTracker::Block*, GLuint*> BrushIndexHolder::getPointerToInsertElementsAt(const size_t elementCount) {
            if (auto block = m_allocationTracker.allocate(elementCount); block != nullptr) {
                GLuint* dest = m_indexHolder.getPointerToWriteElementsTo(block->pos, elementCount);
                return {block, dest};
            }

            // retry
            const size_t newSize = std::max(2 * m_allocationTracker.capacity(),
                                            m_allocationTracker.capacity() + elementCount);
            m_allocationTracker.expand(newSize);
            m_indexHolder.resize(newSize);

            // insert again
            auto block = m_allocationTracker.allocate(elementCount);
            assert(block != nullptr);
            GLuint* dest = m_indexHolder.getPointerToWriteElementsTo(block->pos, elementCount);
            return {block, dest};
        }

        void BrushIndexHolder::zeroElementsWithKey(AllocationTracker::Block* key) {
            const auto pos = key->pos;
            const auto size = key->size;
            m_allocationTracker.free(key);

            m_indexHolder.zeroRange(pos, size);
        }

        void BrushIndexHolder::render(const PrimType primType) const {
            assert(m_indexHolder.prepared());
            m_indexHolder.render(primType, 0, m_indexHolder.size());
        }

        bool BrushIndexHolder::prepared() const {
            return m_indexHolder.prepared();
        }

        void BrushIndexHolder::prepare(Vbo& vbo) {
            m_indexHolder.prepare(vbo);
            assert(m_indexHolder.prepared());
        }

        // BrushVertexHolder

        BrushVertexHolder::BrushVertexHolder() : m_vertexHolder(),
                                               m_allocationTracker(0) {}

        std::pair<AllocationTracker::Block*, BrushVertexHolder::Vertex*> BrushVertexHolder::getPointerToInsertVerticesAt(const size_t vertexCount) {
            if (auto block = m_allocationTracker.allocate(vertexCount); block != nullptr) {
                Vertex* dest = m_vertexHolder.getPointerToWriteElementsTo(block->pos, vertexCount);
                return {block, dest};
            }

            // retry
            const size_t newSize = std::max(2 * m_allocationTracker.capacity(),
                                            m_allocationTracker.capacity() + vertexCount);
            m_allocationTracker.expand(newSize);
            m_vertexHolder.resize(newSize);

            // insert again
            auto block = m_allocationTracker.allocate(vertexCount);
            assert(block != nullptr);

            Vertex* dest = m_vertexHolder.getPointerToWriteElementsTo(block->pos, vertexCount);
            return {block, dest};
        }

        void BrushVertexHolder::deleteVerticesWithKey(AllocationTracker::Block* key) {
            m_allocationTracker.free(key);

            // there's no need to actually delete the vertices from the VBO.
            // because we only ever do indexed drawing from it.
            // Marking the space free in m_allocationTracker will allow
            // us to re-use the space later
        }

        bool BrushVertexHolder::setupVertices() {
            return m_vertexHolder.setupVertices();
        }

        void BrushVertexHolder::cleanupVertices() {
            m_vertexHolder.cleanupVertices();
        }

        bool BrushVertexHolder::prepared() const {
            return m_vertexHolder.prepared();
        }

        void BrushVertexHolder::prepare(Vbo& vbo) {
            m_vertexHolder.prepare(vbo);
            assert(m_vertexHolder.prepared());
        }
    }
}
