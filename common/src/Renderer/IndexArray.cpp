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

#include "IndexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        IndexArray::IndicesAndCounts::IndicesAndCounts(const size_t capacity) :
        indices(0),
        counts(0) {
            indices.reserve(capacity);
            counts.reserve(capacity);
        }
        
        IndexArray::IndicesAndCounts::IndicesAndCounts(const GLint index, const GLsizei count) :
        indices(1, index),
        counts(1, count) {}

        size_t IndexArray::IndicesAndCounts::size() const {
            return indices.size();
        }
        
        void IndexArray::IndicesAndCounts::add(const PrimType primType, const GLint index, const GLsizei count) {
            switch (primType) {
                case PT_Points:
                case PT_Lines:
                case PT_Triangles:
                case PT_Quads: {
                    if (size() == 1) {
                        const GLint myIndex = indices.front();
                        GLsizei& myCount = counts.front();
                        
                        if (index == myIndex + myCount) {
                            myCount += count;
                            break;
                        }
                    }
                }
                case PT_LineStrips:
                case PT_LineLoops:
                case PT_TriangleFans:
                case PT_TriangleStrips:
                case PT_QuadStrips:
                case PT_Polygons:
                    assert(indices.capacity() > indices.size());
                    indices.push_back(index);
                    counts.push_back(count);
                    break;
            }
        }
        
        void IndexArray::Size::inc(const PrimType primType, const size_t count) {
            PrimTypeToSize::iterator primIt = MapUtils::findOrInsert(m_sizes, primType, 0);
            primIt->second += count;
        }
        
        void IndexArray::Size::initialize(PrimTypeToIndexData& data) const {
            PrimTypeToSize::const_iterator primIt, primEnd;
            for (primIt = m_sizes.begin(), primEnd = m_sizes.end(); primIt != primEnd; ++primIt) {
                const PrimType primType = primIt->first;
                const size_t size = primIt->second;
                data.insert(std::make_pair(primType, IndicesAndCounts(size)));
            }
        }
        
        IndexArray::IndexArray() {}

        IndexArray::IndexArray(const Size& size) {
            size.initialize(*m_data);
        }

        IndexArray::IndexArray(const PrimType primType, const GLint index, const GLsizei count) {
            m_data->insert(std::make_pair(primType, IndicesAndCounts(index, count)));
        }

        IndexArray::IndexArray(const PrimType primType, const GLint index, const size_t count) {
            m_data->insert(std::make_pair(primType, IndicesAndCounts(index, static_cast<GLsizei>(count))));
        }
        
        void IndexArray::add(const PrimType primType, const GLint index, const GLsizei count) {
            PrimTypeToIndexData::iterator it = m_data->find(primType);
            assert(it != m_data->end());
            
            IndicesAndCounts& indicesAndCounts = it->second;
            indicesAndCounts.add(primType, index, count);
        }

        void IndexArray::render(const VertexArray& vertexArray) const {
            typename PrimTypeToIndexData::const_iterator primIt, primEnd;
            for (primIt = m_data->begin(), primEnd = m_data->end(); primIt != primEnd; ++primIt) {
                const PrimType primType = primIt->first;
                const IndicesAndCounts& indicesAndCounts = primIt->second;
                const GLsizei primCount = static_cast<GLsizei>(indicesAndCounts.size());
                vertexArray.render(primType, indicesAndCounts.indices, indicesAndCounts.counts, primCount);
            }
        }
    }
}
