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

#include "MemoryPoolTracker.h"

#include <stdexcept>
#include <cassert>

namespace TrenchBroom {
	namespace Renderer {
        MemoryPoolTracker::Block* MemoryPoolTracker::allocate() {
            if (m_freeHead != nullptr) {
                Block *block = m_freeHead;
                m_freeHead = block->next;
                return block;
            }

            // maybe allocate a new block
            if (m_blocksAllocated < m_capacity) {
                Block *newBlock = new Block();
                newBlock->offset = m_blocksAllocated;
                newBlock->next = nullptr;

                // chain to the list of all blocks, for deleting them in the destructor
                newBlock->allBlocksChain = m_allBlocksHead;
                m_allBlocksHead = newBlock;

                m_blocksAllocated++;
                return newBlock;
            }
            // no free blocks
            return nullptr;
        }

        void MemoryPoolTracker::free(Block *block) {
            block->next = m_freeHead;
            m_freeHead = block;
        }

        int64_t MemoryPoolTracker::capacity() {
            return m_capacity;
        }

        void MemoryPoolTracker::expand(int64_t newSize) {
            if (newSize <= m_capacity) {
                throw std::invalid_argument("new size must be larger");
            }
            m_capacity = newSize;
        }

        MemoryPoolTracker::MemoryPoolTracker() :
                m_freeHead(nullptr),
                m_allBlocksHead(nullptr),
                m_capacity(0),
                m_blocksAllocated(0) {}

        MemoryPoolTracker::MemoryPoolTracker(int64_t size) :
                m_freeHead(nullptr),
                m_allBlocksHead(nullptr),
                m_capacity(size),
                m_blocksAllocated(0) {}

        MemoryPoolTracker::~MemoryPoolTracker() {
            int64_t numBlocksFreed = 0;

            while (m_allBlocksHead != nullptr) {
                Block *blockToDelete = m_allBlocksHead;
                m_allBlocksHead = blockToDelete->allBlocksChain;
                delete blockToDelete;
                numBlocksFreed++;
            }

            assert(numBlocksFreed == m_blocksAllocated);
        }
    }
}
