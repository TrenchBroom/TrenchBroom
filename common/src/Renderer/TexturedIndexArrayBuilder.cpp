/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include <algorithm>

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
            add(texture, GL_POINTS, indices);
        }
        
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
        
        void TexturedIndexArrayBuilder::addQuads(const Texture* texture, const Index baseIndex, const size_t vertexCount) {
            assert(vertexCount % 4 == 0);
            IndexList indices(vertexCount);
            
            for (size_t i = 0; i < vertexCount; ++i)
                indices[i] = baseIndex + static_cast<Index>(i);
            
            add(texture, GL_QUADS, indices);
        }

        void TexturedIndexArrayBuilder::addPolygon(const Texture* texture, const IndexList& indices) {
            const size_t count = indices.size();
            
            IndexList polyIndices(0);
            polyIndices.reserve(3 * (count - 2));
            
            for (size_t i = 0; i < count - 2; ++i) {
                polyIndices.push_back(indices[0]);
                polyIndices.push_back(indices[i + 1]);
                polyIndices.push_back(indices[i + 2]);
            }
            
            add(texture, GL_TRIANGLES, polyIndices);
        }

        void TexturedIndexArrayBuilder::addPolygon(const Texture* texture, const Index baseIndex, const size_t vertexCount) {
            IndexList polyIndices(0);
            polyIndices.reserve(3 * (vertexCount - 2));
            
            for (size_t i = 0; i < vertexCount - 2; ++i) {
                polyIndices.push_back(baseIndex);
                polyIndices.push_back(baseIndex + static_cast<Index>(i + 1));
                polyIndices.push_back(baseIndex + static_cast<Index>(i + 2));
            }
            
            add(texture, GL_TRIANGLES, polyIndices);
        }

        void TexturedIndexArrayBuilder::add(const Texture* texture, const PrimType primType, const IndexList& indices) {
            const size_t offset = m_ranges.add(texture, primType, indices.size());
            IndexList::iterator dest = m_indices.begin();
            std::advance(dest, offset);
            std::copy(indices.begin(), indices.end(), dest);
        }
    }
}
