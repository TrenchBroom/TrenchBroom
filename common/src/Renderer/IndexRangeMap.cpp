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

#include "IndexRangeMap.h"
#include "CollectionUtils.h"

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

        void IndexRangeMap::IndicesAndCounts::add(const PrimType primType, const size_t index, const size_t count, const bool dynamicGrowth) {
            switch (primType) {
                case PT_Points:
                case PT_Lines:
                case PT_Triangles:
                case PT_Quads: {
                    if (size() == 1) {
                        const GLint myIndex = indices.front();
                        GLsizei& myCount = counts.front();
                        
                        if (index == static_cast<size_t>(myIndex) + static_cast<size_t>(myCount)) {
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
                    assert(dynamicGrowth || indices.capacity() > indices.size());
                    indices.push_back(static_cast<GLint>(index));
                    counts.push_back(static_cast<GLsizei>(count));
                    break;
            }
        }
        
        void IndexRangeMap::Size::inc(const PrimType primType, const size_t count) {
            PrimTypeToSize::iterator primIt = MapUtils::findOrInsert(m_sizes, primType, 0);
            primIt->second += count;
        }
        
        void IndexRangeMap::Size::initialize(PrimTypeToIndexData& data) const {
            PrimTypeToSize::const_iterator primIt, primEnd;
            for (primIt = m_sizes.begin(), primEnd = m_sizes.end(); primIt != primEnd; ++primIt) {
                const PrimType primType = primIt->first;
                const size_t size = primIt->second;
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
        
        void IndexRangeMap::add(const PrimType primType, const size_t index, const size_t count) {
            PrimTypeToIndexData::iterator it = m_data->end();
            if (m_dynamicGrowth)
                it = MapUtils::findOrInsert(*m_data, primType);
            else
                it = m_data->find(primType);
            assert(it != m_data->end());
            
            IndicesAndCounts& indicesAndCounts = it->second;
            indicesAndCounts.add(primType, index, count, m_dynamicGrowth);
        }

        void IndexRangeMap::render(VertexArray& vertexArray) const {
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
