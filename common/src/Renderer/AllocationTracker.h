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

#ifndef TrenchBroom_AllocationTracker
#define TrenchBroom_AllocationTracker

#include <map>
#include <set>
#include <utility>

namespace TrenchBroom {
    namespace Renderer {
        /**
         * Implements the bookkeeping part of a malloc()-like interface.
         */
        class AllocationTracker {
        public:
            using Index = int64_t;

            // TODO: remove
            class Range {
            public:
                Index pos;
                Index size;

                Range(Index p, Index s);

                bool operator==(const Range &other) const;

                bool operator<(const Range &other) const;
            };

            struct Block {
            public:
                Index pos;
                Index size;

            private:
                friend class AllocationTracker;
                /**
                 * If this is null it means we're the head of the list in the
                 * m_sizeToFreeBlock map.
                 */
                Block* prevOfSameSize;
                Block* nextOfSameSize;
                /**
                 * If this is null it means m_leftmostBlock points to us.
                 * These are used for:
                 *  - merging adjacent free blocks when an allocation is freed.
                 *  - freeing the actual memory used for the Block objects
                 *    in the AllocationTracker destructor.
                 */
                Block* left;
                Block* right;

                bool free;
            };

        private:
            Index m_capacity;

            /**
             * Only used to free all of the blocks in the destructor
             */
            Block* m_leftmostBlock;

            // TODO: benchmark against a vector + binary search
            std::map<Index, Block*> m_sizeToFreeBlock;

            void unlinkFromBinList(Block* block);
            void linkToBinList(Block* block);

        public:
            explicit AllocationTracker(Index initial_capacity);
            AllocationTracker();
            ~AllocationTracker();

            /**
             * Tries to make an allocation. Returns {true, index} on success,
             * and {false, ?} on failure (the Index is meaningless if the
             * first element is false.)
             */
            Block* allocate(size_t bytes);
            /**
             * Returns the block that was freed.
             */
            void free(Block* block);
            size_t capacity() const;
            void expand(Index newcap);

            // Testing / debugging

            std::set<Range> freeBlocks() const;
            std::set<Range> usedBlocks() const;
            Index largestPossibleAllocation() const;
            void checkInvariants() const;
        };
    }
}

#endif
