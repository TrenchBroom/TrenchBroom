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
        
        void TexturedIndexArrayBuilder::addPoint(const Texture* texture, const Index i) {
            const size_t offset = m_ranges.add(texture, GL_POINTS, 1);
            m_indices[offset] = i;
        }
        
        void TexturedIndexArrayBuilder::addPoints(const Texture* texture, const IndexList& indices) {
            Index* dest = add(texture, GL_POINTS, indices.size());
            for (Index index : indices) {
                *(dest++) = index;
            }
        }

#if 0
        void TexturedIndexArrayBuilder::addLine(const Texture* texture, const Index i1, const Index i2) {
            const size_t offset = m_ranges.add(texture, GL_LINES, 2);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
        }
        
        void TexturedIndexArrayBuilder::addLines(const Texture* texture, const IndexList& indices) {
            assert(indices.size() % 2 == 0);
            add(texture, GL_LINES, indices);
        }
        
        void TexturedIndexArrayBuilder::addTriangle(const Texture* texture, const Index i1, const Index i2, const Index i3) {
            const size_t offset = m_ranges.add(texture, GL_TRIANGLES, 3);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
            m_indices[offset + 2] = i3;
        }
        
        void TexturedIndexArrayBuilder::addTriangles(const Texture* texture, const IndexList& indices) {
            assert(indices.size() % 3 == 0);
            add(texture, GL_TRIANGLES, indices);
        }
        
        void TexturedIndexArrayBuilder::addQuad(const Texture* texture, const Index, const Index i1, const Index i2, const Index i3, const Index i4) {
            const size_t offset = m_ranges.add(texture, GL_QUADS, 4);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
            m_indices[offset + 2] = i3;
            m_indices[offset + 3] = i4;
        }
        
        void TexturedIndexArrayBuilder::addQuads(const Texture* texture, const IndexList& indices) {
            assert(indices.size() % 4 == 0);
            add(texture, GL_QUADS, indices);
        }
#endif
        void TexturedIndexArrayBuilder::addQuads(const Texture* texture, const Index baseIndex, const size_t vertexCount) {
            assert(vertexCount % 4 == 0);
            Index* dest = add(texture, GL_QUADS, vertexCount);
            
            for (size_t i = 0; i < vertexCount; ++i) {
                dest[i] = baseIndex + static_cast<Index>(i);
            }
        }
#if 0
        void TexturedIndexArrayBuilder::addPolygon(const Texture* texture, const IndexList& indices) {
            const size_t count = indices.size();

            const size_t indexCount = 3 * (count - 2);
            Index* dest = add(texture, GL_TRIANGLES, indexCount);
            
            for (size_t i = 0; i < count - 2; ++i) {
                *(dest++) = indices[0];
                *(dest++) = indices[i + 1];
                *(dest++) = indices[i + 2];
            }
        }
#endif
        void TexturedIndexArrayBuilder::addPolygon(const Texture* texture, const Index baseIndex, const size_t vertexCount) {
            const size_t indexCount = 3 * (vertexCount - 2);
            Index* dest = add(texture, GL_TRIANGLES, indexCount);

            for (size_t i = 0; i < vertexCount - 2; ++i) {
                *(dest++) = baseIndex;
                *(dest++) = baseIndex + static_cast<Index>(i + 1);
                *(dest++) = baseIndex + static_cast<Index>(i + 2);
            }
        }

        TexturedIndexArrayBuilder::Index* TexturedIndexArrayBuilder::add(const Texture* texture, PrimType primType, size_t indexCount) {
            const size_t offset = m_ranges.add(texture, primType, indexCount);

            return m_indices.data() + offset;
        }
    }
}
