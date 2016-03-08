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
#include "Renderer/IndexArray.h"

namespace TrenchBroom {
    namespace Renderer {
        IndexArrayMap::IndexArrayRange::IndexArrayRange(const size_t i_offset, const size_t i_capacity) :
        offset(i_offset),
        capacity(i_capacity),
        count(0) {}

        size_t IndexArrayMap::IndexArrayRange::add(const size_t i_count) {
            assert(capacity - count >= i_count);
            const size_t result = offset + count;
            count += i_count;
            return result;
        }

        IndexArrayMap::Size::Size() :
        m_indexCount(0) {}
        
        void IndexArrayMap::Size::inc(const PrimType primType, const size_t count) {
            PrimTypeToSize::iterator primIt = MapUtils::findOrInsert(m_sizes, primType, 0);
            primIt->second += count;
            m_indexCount += count;
        }

        size_t IndexArrayMap::Size::indexCount() const {
            return m_indexCount;
        }

        void IndexArrayMap::Size::initialize(PrimTypeToRangeMap& data, const size_t baseOffset) const {
            size_t offset = baseOffset;
            PrimTypeToSize::const_iterator primIt, primEnd;
            for (primIt = m_sizes.begin(), primEnd = m_sizes.end(); primIt != primEnd; ++primIt) {
                const PrimType primType = primIt->first;
                const size_t size = primIt->second;
                data.insert(std::make_pair(primType, IndexArrayRange(offset, size)));
                offset += size;
            }
        }

        IndexArrayMap::IndexArrayMap() {}

        IndexArrayMap::IndexArrayMap(const Size& size) :
        m_ranges(new PrimTypeToRangeMap()) {
            size.initialize(*m_ranges, 0);
        }

        IndexArrayMap::IndexArrayMap(const Size& size, const size_t baseOffset) :
        m_ranges(new PrimTypeToRangeMap()) {
            size.initialize(*m_ranges, baseOffset);
        }

        size_t IndexArrayMap::add(const PrimType primType, const size_t count) {
            IndexArrayRange& range = findRange(primType);
            return range.add(count);
        }

        void IndexArrayMap::render(IndexArray& indexArray) const {
            PrimTypeToRangeMap::const_iterator primIt, primEnd;
            for (primIt = m_ranges->begin(), primEnd = m_ranges->end(); primIt != primEnd; ++primIt) {
                const PrimType primType = primIt->first;
                const IndexArrayRange& range = primIt->second;
                indexArray.render(primType, range.offset, range.count);
            }
        }

        IndexArrayMap::IndexArrayRange& IndexArrayMap::findRange(const PrimType primType) {
            PrimTypeToRangeMap::iterator it = m_ranges->find(primType);
            assert(it != m_ranges->end());
            return it->second;
        }
    }
}
