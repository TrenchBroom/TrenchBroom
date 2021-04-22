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

#include "TexturedIndexArrayMapBuilder.h"

#include "Renderer/PrimType.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArrayMapBuilder::TexturedIndexArrayMapBuilder(const TexturedIndexArrayMap::Size& size) :
        m_ranges{size} {
            m_indices.resize(size.indexCount());
        }

        TexturedIndexArrayMapBuilder::IndexList& TexturedIndexArrayMapBuilder::indices() {
            return m_indices;
        }

        TexturedIndexArrayMap& TexturedIndexArrayMapBuilder::ranges() {
            return m_ranges;
        }

        void TexturedIndexArrayMapBuilder::addPoint(const Texture* texture, const Index i) {
            const size_t offset = m_ranges.add(texture, PrimType::Points, 1);
            m_indices[offset] = i;
        }

        void TexturedIndexArrayMapBuilder::addPoints(const Texture* texture, const IndexList& indices) {
            add(texture, PrimType::Points, indices);
        }

        void TexturedIndexArrayMapBuilder::addLine(const Texture* texture, const Index i1, const Index i2) {
            const size_t offset = m_ranges.add(texture, PrimType::Lines, 2);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
        }

        void TexturedIndexArrayMapBuilder::addLines(const Texture* texture, const IndexList& indices) {
            assert(indices.size() % 2 == 0);
            add(texture, PrimType::Lines, indices);
        }

        void TexturedIndexArrayMapBuilder::addTriangle(const Texture* texture, const Index i1, const Index i2, const Index i3) {
            const size_t offset = m_ranges.add(texture, PrimType::Triangles, 3);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
            m_indices[offset + 2] = i3;
        }

        void TexturedIndexArrayMapBuilder::addTriangles(const Texture* texture, const IndexList& indices) {
            assert(indices.size() % 3 == 0);
            add(texture, PrimType::Triangles, indices);
        }

        void TexturedIndexArrayMapBuilder::addQuad(const Texture* texture, const Index, const Index i1, const Index i2, const Index i3, const Index i4) {
            const size_t offset = m_ranges.add(texture, PrimType::Quads, 4);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
            m_indices[offset + 2] = i3;
            m_indices[offset + 3] = i4;
        }

        void TexturedIndexArrayMapBuilder::addQuads(const Texture* texture, const IndexList& indices) {
            assert(indices.size() % 4 == 0);
            add(texture, PrimType::Quads, indices);
        }

        void TexturedIndexArrayMapBuilder::addQuads(const Texture* texture, const Index baseIndex, const size_t vertexCount) {
            assert(vertexCount % 4 == 0);
            IndexList indices(vertexCount);

            for (size_t i = 0; i < vertexCount; ++i) {
                indices[i] = baseIndex + static_cast<Index>(i);
            }

            add(texture, PrimType::Quads, indices);
        }

        void TexturedIndexArrayMapBuilder::addPolygon(const Texture* texture, const IndexList& indices) {
            const size_t count = indices.size();

            auto polyIndices = IndexList{};
            polyIndices.reserve(3 * (count - 2));

            for (size_t i = 0; i < count - 2; ++i) {
                polyIndices.push_back(indices[0]);
                polyIndices.push_back(indices[i + 1]);
                polyIndices.push_back(indices[i + 2]);
            }

            add(texture, PrimType::Triangles, polyIndices);
        }

        void TexturedIndexArrayMapBuilder::addPolygon(const Texture* texture, const Index baseIndex, const size_t vertexCount) {
            auto polyIndices = IndexList{};
            polyIndices.reserve(3 * (vertexCount - 2));

            for (size_t i = 0; i < vertexCount - 2; ++i) {
                polyIndices.push_back(baseIndex);
                polyIndices.push_back(baseIndex + static_cast<Index>(i + 1));
                polyIndices.push_back(baseIndex + static_cast<Index>(i + 2));
            }

            add(texture, PrimType::Triangles, polyIndices);
        }

        void TexturedIndexArrayMapBuilder::add(const Texture* texture, const PrimType primType, const IndexList& indices) {
            const size_t offset = m_ranges.add(texture, primType, indices.size());
            auto dest = std::begin(m_indices);
            std::advance(dest, static_cast<IndexList::iterator::difference_type>(offset));
            std::copy(std::begin(indices), std::end(indices), dest);
        }
    }
}
