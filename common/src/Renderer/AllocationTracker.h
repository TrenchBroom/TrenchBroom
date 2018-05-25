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

            class Block {
            public:
                Index pos;
                Index size;

                Block(Index p, Index s);

                bool operator==(const Block &other) const;

                bool operator<(const Block &other) const;
            };

        private:
            Index m_capacity;

            // track free space by 3 indices
            std::map<Index, std::set<Index>> m_sizeToFreePositions;
            std::map<Index, Index> m_posToFreeSize;
            std::map<Index, Index> m_endPosToFreePos;

            // track used space
            std::map<Index, Index> m_posToUsedSize;

            void eraseFree(Block b);
            void insertFree(Block b);

        public:
            explicit AllocationTracker(Index initial_capacity);
            AllocationTracker();

            /**
             * Tries to make an allocation. Returns {true, index} on success,
             * and {false, ?} on failure (the Index is meaningless if the
             * first element is false.)
             */
            std::pair<bool, Index> allocate(size_t bytes);
            /**
             * Returns the block that was freed.
             */
            Block free(Index pos);
            size_t capacity() const;
            void expand(Index newcap);

            // Testing / debugging

            std::set<Block> freeBlocks() const;
            std::set<Block> usedBlocks() const;
            Index largestPossibleAllocation() const;
        };
    }
}

#endif
