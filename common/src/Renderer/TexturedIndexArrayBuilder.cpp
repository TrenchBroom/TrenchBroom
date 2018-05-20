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

#include "TexturedIndexArrayBuilder.h"

namespace TrenchBroom {
    namespace Renderer {
        TexturedIndexArrayBuilder::TexturedIndexArrayBuilder(const TexturedIndexArrayMap::Size& size) :
        m_indices(size.indexCount()),
        m_ranges(size) {}
        
        const TexturedIndexArrayBuilder::IndexList& TexturedIndexArrayBuilder::indices() const {
            return m_indices;
        }
        
        TexturedIndexArrayBuilder::IndexList& TexturedIndexArrayBuilder::indices() {
            return m_indices;
        }
        
        const TexturedIndexArrayMap& TexturedIndexArrayBuilder::ranges() const {
            return m_ranges;
        }

        void TexturedIndexArrayBuilder::addPolygon(const Texture* texture, const Index baseIndex, const size_t vertexCount) {
            const size_t indexCount = 3 * (vertexCount - 2);
            Index* dest = addTriangles(texture, indexCount);

            for (size_t i = 0; i < vertexCount - 2; ++i) {
                *(dest++) = baseIndex;
                *(dest++) = baseIndex + static_cast<Index>(i + 1);
                *(dest++) = baseIndex + static_cast<Index>(i + 2);
            }
        }

        TexturedIndexArrayBuilder::Index* TexturedIndexArrayBuilder::addTriangles(const Texture* texture, size_t indexCount) {
            const size_t offset = m_ranges.addTriangles(texture, indexCount);

            return m_indices.data() + offset;
        }
    }
}
