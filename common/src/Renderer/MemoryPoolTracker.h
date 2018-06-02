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


#include <cstdint>

namespace TrenchBroom {
	namespace Renderer {
        /**
         * Tracks allocation metadata for fixed-size blocks.
         *
         * Note: you need to return all allocated blocks before
         * the destructor runs, otherwise it leaks memory.
         */
        class MemoryPoolTracker {
        public:
            struct Block;
        private:
            Block* m_freeHead;

            int64_t m_capacity;
            int64_t m_blocksAllocated;
        public:
            struct Block {
            public:
                int64_t offset;
            private:
                friend class MemoryPoolTracker;
                Block* next;
            };

            Block* allocate();
            void free(Block* block);
            int64_t capacity();
            void expand(int64_t newSize);

            MemoryPoolTracker();
            explicit MemoryPoolTracker(int64_t size);
            ~MemoryPoolTracker();
        };
	}
}
