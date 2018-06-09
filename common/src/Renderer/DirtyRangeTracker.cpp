/*
 Copyright (C) 2018 Eric Wasylishen

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

#include "DirtyRangeTracker.h"

#include <exception>
#include <cassert>
#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {

        FastDirtyRange::FastDirtyRange(size_t initial_capacity)
            : m_dirtyPos(0), m_dirtySize(0), m_capacity(initial_capacity) {}

        FastDirtyRange::FastDirtyRange()
            : m_dirtyPos(0), m_dirtySize(0), m_capacity(0) {}

        void FastDirtyRange::expand(size_t newcap) {
            if (newcap <= m_capacity) {
                throw std::invalid_argument("new capacity must be greater");
            }

            const size_t oldcap = m_capacity;
            m_capacity = newcap;
            markDirty(oldcap, newcap - oldcap);
        }

        size_t FastDirtyRange::capacity() const {
            return m_capacity;
        }

        void FastDirtyRange::markDirty(size_t pos, size_t size) {
            // bounds check
            if (pos + size > m_capacity) {
                throw std::invalid_argument("markDirty provided range out of bounds");
            }

            const size_t newPos = std::min(pos, m_dirtyPos);
            const size_t newEnd = std::max(pos + size, m_dirtyPos + m_dirtySize);

            m_dirtyPos = newPos;
            m_dirtySize = newEnd - newPos;
        }

        bool FastDirtyRange::clean() const {
            return m_dirtySize == 0;
        }

        // DirtyRangeTracker

        DirtyRangeTracker::Range::Range(size_t p, size_t s) : pos(p), size(s) {}

        DirtyRangeTracker::Range::Range() : pos(0), size(0) {}
        DirtyRangeTracker::Range::Range(const Range& other) = default;
        DirtyRangeTracker::Range& DirtyRangeTracker::Range::operator=(const DirtyRangeTracker::Range& other) = default;

        DirtyRangeTracker::Range DirtyRangeTracker::Range::unionWith(const DirtyRangeTracker::Range &other) const {
            const size_t newPos = std::min(pos, other.pos);
            const size_t newEnd = std::max(pos + size, other.pos + other.size);
            return Range(newPos, newEnd - newPos);
        }

        bool DirtyRangeTracker::Range::operator==(const Range& other) const {
            return pos == other.pos
                    && size == other.size;
        }

        DirtyRangeTracker::DirtyRangeTracker(size_t initial_capacity)
                : m_capacity(initial_capacity) {}

        DirtyRangeTracker::DirtyRangeTracker()
                : m_capacity(0) {}

        void DirtyRangeTracker::expand(const size_t newcap) {
            if (newcap <= m_capacity) {
                throw std::invalid_argument("new capacity must be greater");
            }

            const size_t oldcap = m_capacity;
            m_capacity = newcap;
            markDirty(oldcap, newcap - oldcap);
        }

        size_t DirtyRangeTracker::capacity() const {
            return m_capacity;
        }

        void DirtyRangeTracker::markDirty(const size_t pos, const size_t size) {
            if (size == 0) {
                return;
            }

            // bounds check
            if (pos + size > m_capacity) {
                throw std::invalid_argument("markDirty provided range out of bounds");
            }

            Range newRange = Range(pos, size);

            // check for all existing ranges whose end is >= pos and start is <= (pos + size)
            while (true) {
                auto it = m_endPosToRange.lower_bound(pos); // existing block whose end is >= pos
                if (it == m_endPosToRange.end()) {
                    break;
                }
                const Range& existingRange = it->second;
                if (existingRange.pos > pos + size) {
                    // for the lowest end position >= pos, the start position is after (pos + start), so we can stop searching
                    break;
                }
                // the existing block touches the one we're adding, delete it and merge the ranges.
                newRange = newRange.unionWith(existingRange);
                m_endPosToRange.erase(it);
            }

            // insert the new range
            m_endPosToRange[newRange.pos + newRange.size] = newRange;
        }

        bool DirtyRangeTracker::clean() const {
            return m_endPosToRange.empty();
        }
    }
}

