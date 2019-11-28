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

#include "IndexRangeMap.h"

#include "CollectionUtils.h"
#include "Base/VecUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        IndexRangeMap::IndicesAndCounts::IndicesAndCounts() :
        indices(0),
        counts(0) {}

        IndexRangeMap::IndicesAndCounts::IndicesAndCounts(const size_t index, const size_t count) :
        indices(1, static_cast<GLint>(index)),
        counts(1,  static_cast<GLsizei>(count)) {}

        size_t IndexRangeMap::IndicesAndCounts::size() const {
            return indices.size();
        }

        void IndexRangeMap::IndicesAndCounts::reserve(const size_t capacity) {
            indices.reserve(capacity);
            counts.reserve(capacity);
        }

        void IndexRangeMap::IndicesAndCounts::add(const PrimType primType, const size_t index, const size_t count, [[maybe_unused]] const bool dynamicGrowth) {
            switch (primType) {
                case GL_POINTS:
                case GL_LINES:
                case GL_TRIANGLES:
                case GL_QUADS: {
                    if (size() == 1) {
                        const auto myIndex = indices.front();
                        auto& myCount = counts.front();

                        if (index == static_cast<size_t>(myIndex) + static_cast<size_t>(myCount)) {
                            myCount += count;
                            break;
                        }
                    }
                    switchFallthrough();
                }
                case GL_LINE_STRIP:
                case GL_LINE_LOOP:
                case GL_TRIANGLE_FAN:
                case GL_TRIANGLE_STRIP:
                case GL_QUAD_STRIP:
                case GL_POLYGON:
                    assert(dynamicGrowth || indices.capacity() > indices.size());
                    indices.push_back(static_cast<GLint>(index));
                    counts.push_back(static_cast<GLsizei>(count));
                    break;
            }
        }

        void IndexRangeMap::IndicesAndCounts::add(const IndicesAndCounts& other, [[maybe_unused]] const bool dynamicGrowth) {
            assert(dynamicGrowth || indices.capacity() >= indices.size() + other.indices.size());
            VecUtils::append(indices, other.indices);
            VecUtils::append(counts, other.counts);
        }

        void IndexRangeMap::Size::inc(const PrimType primType, const size_t count) {
            auto primIt = MapUtils::findOrInsert(m_sizes, primType, 0);
            primIt->second += count;
        }

        void IndexRangeMap::Size::inc(const IndexRangeMap::Size& other) {
            for (const auto& entry : other.m_sizes) {
                inc(entry.first, entry.second);
            }
        }

        void IndexRangeMap::Size::initialize(PrimTypeToIndexData& data) const {
            for (const auto& entry : m_sizes) {
                const auto primType = entry.first;
                const auto size = entry.second;
                data[primType].reserve(size);
            }
        }

        IndexRangeMap::IndexRangeMap() :
        m_data(new PrimTypeToIndexData()),
        m_dynamicGrowth(true) {}

        IndexRangeMap::IndexRangeMap(const Size& size) :
        m_data(new PrimTypeToIndexData()),
        m_dynamicGrowth(false) {
            size.initialize(*m_data);
        }

        IndexRangeMap::IndexRangeMap(const PrimType primType, const size_t index, const size_t count) :
        m_data(new PrimTypeToIndexData()),
        m_dynamicGrowth(false) {
            m_data->insert(std::make_pair(primType, IndicesAndCounts(index, count)));
        }

        IndexRangeMap::Size IndexRangeMap::size() const {
            Size result;
            for (const auto& entry : *m_data) {
                const auto primType = entry.first;
                const auto& indices = entry.second;

                result.inc(primType, indices.size());
            }
            return result;
        }

        void IndexRangeMap::add(const PrimType primType, const size_t index, const size_t count) {
            auto& indicesAndCounts = find(primType);
            indicesAndCounts.add(primType, index, count, m_dynamicGrowth);
        }

        void IndexRangeMap::add(const IndexRangeMap& other) {
            for (const auto& entry : *other.m_data) {
                const auto primType = entry.first;
                const auto& indicesToAdd = entry.second;

                auto& indicesAndCounts = find(primType);
                indicesAndCounts.add(indicesToAdd, m_dynamicGrowth);
            }
        }

        void IndexRangeMap::render(VertexArray& vertexArray) const {
            for (const auto& entry : *m_data) {
                const auto primType = entry.first;
                const auto& indicesAndCounts = entry.second;
                const auto primCount = static_cast<GLsizei>(indicesAndCounts.size());
                vertexArray.render(primType, indicesAndCounts.indices, indicesAndCounts.counts, primCount);
            }
        }

        void IndexRangeMap::forEachPrimitive(std::function<void(PrimType, size_t, size_t)> func) const {
            for (const auto& entry : *m_data) {
                const auto primType = entry.first;
                const auto& indicesAndCounts = entry.second;
                const auto primCount = indicesAndCounts.size();

                for (size_t i = 0; i < primCount; ++i) {
                    func(primType, static_cast<size_t>(indicesAndCounts.indices[i]), static_cast<size_t>(indicesAndCounts.counts[i]));
                }
            }
        }

        IndexRangeMap::IndicesAndCounts& IndexRangeMap::find(const PrimType primType) {
            auto it = m_data->end();
            if (m_dynamicGrowth) {
                it = MapUtils::findOrInsert(*m_data, primType);
            } else {
                it = m_data->find(primType);
            }
            assert(it != m_data->end());
            return it->second;
        }
    }
}
