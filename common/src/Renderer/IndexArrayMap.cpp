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

#include "IndexArrayMap.h"

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        void IndexArrayMap::Size::inc(const PrimType primType, const size_t count) {
            PrimTypeToSize::iterator primIt = MapUtils::findOrInsert(m_sizes, primType, 0);
            primIt->second += count;
        }

        void IndexArrayMap::Size::initialize(PrimTypeToIndexData& data) const {
            PrimTypeToSize::const_iterator primIt, primEnd;
            for (primIt = m_sizes.begin(), primEnd = m_sizes.end(); primIt != primEnd; ++primIt) {
                const PrimType primType = primIt->first;
                const size_t size = primIt->second;
                data[primType].reserve(size);
            }
        }

        IndexArrayMap::IndexArrayMap() :
        m_data(new PrimTypeToIndexData()),
        m_dynamicGrowth(true) {}
        
        IndexArrayMap::IndexArrayMap(const Size& size) :
        m_data(new PrimTypeToIndexData()),
        m_dynamicGrowth(false) {
            size.initialize(*m_data);
        }

        void IndexArrayMap::add(PrimType primType, const size_t index) {
            IndexList& indices = findIndices(primType, 1);
            indices.push_back(static_cast<Index>(index));
        }
        
        void IndexArrayMap::addPolygon(PrimType primType, const size_t index, const size_t count) {
            IndexList& indices = findIndices(primType, 3 * (count - 2));
            
            for (size_t i = 0; i < count - 2; ++i) {
                indices.push_back(static_cast<Index>(index + 0 + 0));
                indices.push_back(static_cast<Index>(index + i + 1));
                indices.push_back(static_cast<Index>(index + i + 2));
            }
        }

        size_t IndexArrayMap::countIndices() const {
            size_t result = 0;
            
            PrimTypeToIndexData::const_iterator it, end;
            for (it = m_data->begin(), end = m_data->end(); it != end; ++it) {
                const IndexList& indices = it->second;
                result += indices.size();
            }
            
            return result;
        }

        void IndexArrayMap::getIndices(IndexList& allIndices, PrimTypeToRangeMap& ranges) const {
            
            PrimTypeToIndexData::const_iterator it, end;
            for (it = m_data->begin(), end = m_data->end(); it != end; ++it) {
                const PrimType primType = it->first;
                const IndexList& indices = it->second;
                
                const size_t offset = allIndices.size();
                const size_t count = indices.size();
                
                assert(allIndices.capacity() - allIndices.size() >= count);
                VectorUtils::append(allIndices, indices);
                ranges[primType] = IndexArrayRange(offset, count);
            }
        }

        IndexArrayMap::IndexList& IndexArrayMap::findIndices(const PrimType primType, const size_t toAdd) {
            
            PrimTypeToIndexData::iterator it = m_data->end();
            if (m_dynamicGrowth)
                it = MapUtils::findOrInsert(*m_data, primType);
            else
                it = m_data->find(primType);
            assert(it != m_data->end());
            
            IndexList& indices = it->second;
            assert(m_dynamicGrowth || indices.capacity() - indices.size() >= toAdd);
            return indices;
        }
    }
}
