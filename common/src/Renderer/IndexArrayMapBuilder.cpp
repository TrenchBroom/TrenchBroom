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

#include "IndexArrayMapBuilder.h"

namespace TrenchBroom {
    namespace Renderer {
        IndexArrayMapBuilder::IndexArrayMapBuilder(const IndexArrayMap::Size& size) :
        m_indices(size.indexCount()),
        m_ranges(size) {}
        
        const IndexArrayMapBuilder::IndexList& IndexArrayMapBuilder::indices() const {
            return m_indices;
        }
        
        IndexArrayMapBuilder::IndexList& IndexArrayMapBuilder::indices() {
            return m_indices;
        }
        
        const IndexArrayMap& IndexArrayMapBuilder::ranges() const {
            return m_ranges;
        }
        
        void IndexArrayMapBuilder::addPoint(const Index i) {
            const size_t offset = m_ranges.add(GL_POINTS, 1);
            m_indices[offset] = i;
        }
        
        void IndexArrayMapBuilder::addPoints(const IndexList& indices) {
            add(GL_POINTS, indices);
        }
        
        void IndexArrayMapBuilder::addLine(const Index i1, const Index i2) {
            const size_t offset = m_ranges.add(GL_LINES, 2);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
        }
        
        void IndexArrayMapBuilder::addLines(const IndexList& indices) {
            assert(indices.size() % 2 == 0);
            add(GL_LINES, indices);
        }
        
        void IndexArrayMapBuilder::addTriangle(const Index i1, const Index i2, const Index i3) {
            const size_t offset = m_ranges.add(GL_TRIANGLES, 3);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
            m_indices[offset + 2] = i3;
        }
        
        void IndexArrayMapBuilder::addTriangles(const IndexList& indices) {
            assert(indices.size() % 3 == 0);
            add(GL_TRIANGLES, indices);
        }
        
        void IndexArrayMapBuilder::addQuad(const Index, const Index i1, const Index i2, const Index i3, const Index i4) {
            const size_t offset = m_ranges.add(GL_QUADS, 4);
            m_indices[offset + 0] = i1;
            m_indices[offset + 1] = i2;
            m_indices[offset + 2] = i3;
            m_indices[offset + 3] = i4;
        }
        
        void IndexArrayMapBuilder::addQuads(const IndexList& indices) {
            assert(indices.size() % 4 == 0);
            add(GL_QUADS, indices);
        }
        
        void IndexArrayMapBuilder::addQuads(const Index baseIndex, const size_t vertexCount) {
            assert(vertexCount % 4 == 0);
            IndexList indices(vertexCount);
            
            for (size_t i = 0; i < vertexCount; ++i)
                indices[i] = baseIndex + static_cast<Index>(i);
            
            add(GL_QUADS, indices);
        }
        
        void IndexArrayMapBuilder::addPolygon(const IndexList& indices) {
            const size_t count = indices.size();
            
            IndexList polyIndices(0);
            polyIndices.reserve(3 * (count - 2));
            
            for (size_t i = 0; i < count - 2; ++i) {
                polyIndices.push_back(indices[0]);
                polyIndices.push_back(indices[i + 1]);
                polyIndices.push_back(indices[i + 2]);
            }
            
            add(GL_TRIANGLES, polyIndices);
        }
        
        void IndexArrayMapBuilder::addPolygon(const Index baseIndex, const size_t vertexCount) {
            IndexList polyIndices(0);
            polyIndices.reserve(3 * (vertexCount - 2));
            
            for (size_t i = 0; i < vertexCount - 2; ++i) {
                polyIndices.push_back(baseIndex);
                polyIndices.push_back(baseIndex + static_cast<Index>(i + 1));
                polyIndices.push_back(baseIndex + static_cast<Index>(i + 2));
            }
            
            add(GL_TRIANGLES, polyIndices);
        }
        
        void IndexArrayMapBuilder::add(const PrimType primType, const IndexList& indices) {
            const size_t offset = m_ranges.add(primType, indices.size());
            IndexList::iterator dest = m_indices.begin();
            std::advance(dest, offset);
            std::copy(indices.begin(), indices.end(), dest);
        }
    }
}
