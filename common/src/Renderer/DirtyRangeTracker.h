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

#ifndef TrenchBroom_DirtyRangeTracker
#define TrenchBroom_DirtyRangeTracker

#include <map>

namespace TrenchBroom {
    namespace Renderer {
        /**
         * Tracks dirty ranges of an array, automatically unioning overlapping ranges.
         * Used for syncing edits to two arrays.
         */
        class DirtyRangeTracker {
        public:
            struct Range {
                size_t pos;
                size_t size;

                Range();
                Range(size_t p, size_t s);
                Range(const Range& other);
                Range& operator=(const Range& other);

                Range unionWith(const Range& other) const;
                bool operator==(const Range& other) const;
            };

        private:
            size_t m_capacity;
            std::map<size_t, Range> m_endPosToRange;
        public:
            /**
             * New trackers are initially clean.
             */
            explicit DirtyRangeTracker(size_t initial_capacity);
            DirtyRangeTracker();
            /**
             * Expanding marks the new range as dirty.
             */
            void expand(size_t newcap);
            size_t capacity() const;
            void markDirty(size_t pos, size_t size);
            bool clean() const;

            template <typename L>
            void visitRanges(L&& lambda) const {
                for (const auto& [endPos, range] : m_endPosToRange) {
                    lambda(range);
                }
            }
        };
    }
}

#endif
