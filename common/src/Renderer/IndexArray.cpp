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

namespace TrenchBroom {
    // BrushIndexHolder

    namespace Renderer {

        // IndexHolder

        IndexHolder::IndexHolder() : VboBlockHolder<Index>() {}

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

        VertexArrayInterface::~VertexArrayInterface() {}

        // BrushIndexHolder

        BrushIndexHolder::BrushIndexHolder() : m_indexHolder(),
                                               m_allocationTracker(0),
                                               m_brushToOffset() {}

        bool BrushIndexHolder::empty() const {
            return m_indexHolder.empty();
        }

        void BrushIndexHolder::insertElementsAtIndex(const std::vector<TrenchBroom::GLuint> &elements,
                                                     const TrenchBroom::Renderer::AllocationTracker::Index index,
                                                     const TrenchBroom::Model::Brush *key) {
            m_brushToOffset[key] = index;
            m_indexHolder.writeElements(index, elements);
        }

        size_t BrushIndexHolder::insertElements(const std::vector<GLuint>& elements,
                                                const Model::Brush* key) {
            if (auto [success, index] = m_allocationTracker.allocate(elements.size()); success) {
                insertElementsAtIndex(elements, index, key);
                return index;
            }

            // retry
            const size_t newSize = std::max(2 * m_allocationTracker.capacity(),
                                            m_allocationTracker.capacity() + elements.size());
            m_allocationTracker.expand(newSize);
            m_indexHolder.resize(newSize);

            // insert again
            auto [success, index] = m_allocationTracker.allocate(elements.size());
            assert(success);
            insertElementsAtIndex(elements, index, key);
            return index;
        }

        void BrushIndexHolder::zeroElementsWithKey(const Model::Brush* key) {
            auto it = m_brushToOffset.find(key);
            if (it == m_brushToOffset.end()) {
                throw std::invalid_argument("unknown key");
            }

            const auto offset = it->second;

            auto range = m_allocationTracker.free(offset);
            m_indexHolder.zeroRange(range.pos, range.size);
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
        }

        // BrushVertexHolder

        BrushVertexHolder::BrushVertexHolder() : m_vertexHolder(),
                                               m_allocationTracker(0),
                                               m_brushToOffset() {}

        void BrushVertexHolder::insertVerticesAtIndex(const std::vector<Vertex> &elements,
                                                     const TrenchBroom::Renderer::AllocationTracker::Index index,
                                                     const TrenchBroom::Model::Brush *key) {
            m_brushToOffset[key] = index;
            m_vertexHolder.writeElements(index, elements);
        }

        size_t BrushVertexHolder::insertVertices(const std::vector<Vertex>& elements,
                                                const Model::Brush* key) {
            const size_t insertedElementsCount = elements.size();

            if (auto [success, index] = m_allocationTracker.allocate(insertedElementsCount); success) {
                insertVerticesAtIndex(elements, index, key);
                return index;
            }

            // retry
            const size_t newSize = std::max(2 * m_allocationTracker.capacity(),
                                            m_allocationTracker.capacity() + insertedElementsCount);
            m_allocationTracker.expand(newSize);
            m_vertexHolder.resize(newSize);

            // insert again
            auto [success, index] = m_allocationTracker.allocate(insertedElementsCount);
            assert(success);
            insertVerticesAtIndex(elements, index, key);
            return index;
        }

        void BrushVertexHolder::deleteVerticesWithKey(const Model::Brush* key) {
            auto it = m_brushToOffset.find(key);
            if (it == m_brushToOffset.end()) {
                throw std::invalid_argument("unknown key");
            }

            const auto offset = it->second;

            auto range = m_allocationTracker.free(offset);

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
        }
    }
}
