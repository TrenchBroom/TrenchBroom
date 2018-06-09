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

#include "AllocationTracker.h"

#include <exception>
#include <cassert>

//#define EXPENSIVE_CHECKS

namespace TrenchBroom {
    namespace Renderer {
        AllocationTracker::Range::Range(Index p, Index s)
                : pos(p), size(s) {}

        bool AllocationTracker::Range::operator==(const Range &other) const {
            return pos == other.pos
                   && size == other.size;
        }

        bool AllocationTracker::Range::operator<(const Range &other) const {
            if (pos < other.pos) return true;
            if (pos > other.pos) return false;

            return size < other.size;
        }

        void AllocationTracker::unlinkFromBinList(Block* block) {
            assert(block->free);

            // special case: when we are the head of the list
            // (m_sizeToFreeBlock has a pointer to us)
            if (block->prevOfSameSize == nullptr) {
                // this means we must be in m_sizeToFreeBlock
                auto it = m_sizeToFreeBlock.find(block->size);
                assert(it != m_sizeToFreeBlock.end());
                assert(it->second == block);

                // make sure we prune empty lists from the map!
                if (block->nextOfSameSize == nullptr) {
                    m_sizeToFreeBlock.erase(it);
                } else {
                    it->second = block->nextOfSameSize;
                    block->nextOfSameSize->prevOfSameSize = nullptr;
                    block->nextOfSameSize = nullptr;
                }
                return;
            }

            // handle the "previous" side
            assert(block->prevOfSameSize != nullptr);
            assert(block->size == block->prevOfSameSize->size);
            block->prevOfSameSize->nextOfSameSize = block->nextOfSameSize;

            // handle the "next" side
            if (block->nextOfSameSize) {
                assert(block->size == block->nextOfSameSize->size);
                block->nextOfSameSize->prevOfSameSize = block->prevOfSameSize;
            }

            // clear the nextOfSameSize/prevOfSameSize pointers to mark the block as unlinked from the bin list
            block->nextOfSameSize = nullptr;
            block->prevOfSameSize = nullptr;
        }

        void AllocationTracker::linkToBinList(Block* block) {
            assert(block->size > 0);
            assert(block->prevOfSameSize == nullptr);
            assert(block->nextOfSameSize == nullptr);

            Block*& dest = m_sizeToFreeBlock[block->size];
            if (dest == nullptr) {
                // NOTE: inserts into the map
                dest = block;
            } else {
                Block* previousListHead = dest;

                assert(previousListHead->size == block->size);
                assert(previousListHead->prevOfSameSize == nullptr);

                // connect block to previousListHead
                block->nextOfSameSize = previousListHead;
                previousListHead->prevOfSameSize = block;

                // NOTE: inserts into the map
                dest = block;
            }
        }

        AllocationTracker::Block* AllocationTracker::allocate(size_t needed) {
            checkInvariants();

            if (needed == 0)
                throw std::runtime_error("allocate() requires positive nonzero size");

            // find the smallest free block that will fit the allocation
            auto it = m_sizeToFreeBlock.lower_bound(needed);
            if (it == m_sizeToFreeBlock.end()) {
                checkInvariants();
                return nullptr;
            }

            // unlink it from the size bin
            Block* block = it->second;
            assert(block != nullptr);
            assert(block->free);
            assert(block->prevOfSameSize == nullptr);
            assert(block->size == it->first);
            {
                Block *blockAfter = block->nextOfSameSize;
                if (blockAfter == nullptr) {
                    m_sizeToFreeBlock.erase(it);
                } else {
                    assert(blockAfter->size == it->first);
                    it->second = blockAfter;
                    blockAfter->prevOfSameSize = nullptr;
                }
            }

            block->nextOfSameSize = nullptr;
            block->prevOfSameSize = nullptr;

            if (block->size == needed) {
                // lucky case: exact size. we're done
                block->free = false;

                checkInvariants();
                return block;
            }

            // common case:
            // the block is too large, split off the part we need
            assert(block->size > needed);

            // this will be the left section of `block`
            Block* newBlock = new Block();
            newBlock->pos= block->pos;
            newBlock->size = needed;
            newBlock->prevOfSameSize = nullptr;
            newBlock->nextOfSameSize = nullptr;
            newBlock->left = block->left;
            newBlock->right = block;
            newBlock->free = false;

            // update the block that was to the left of block
            if (block->left == nullptr) {
                // update m_leftmostBlock
                assert(m_leftmostBlock == block);
                m_leftmostBlock = newBlock;
            } else {
                block->left->right = newBlock;
            }

            // update block
            block->left = newBlock;
            block->pos += needed;
            block->size -= needed;
            linkToBinList(block);

            checkInvariants();
            return newBlock;
        }

        void AllocationTracker::free(Block* block) {
            checkInvariants();

            assert(!block->free);
            assert(block->prevOfSameSize == nullptr);
            assert(block->nextOfSameSize == nullptr);

            Block* left = block->left;
            Block* right = block->right;

            // 3 cases:
            // a) merge left, block, and right
            if (left != nullptr && left->free
                && right != nullptr && right->free) {

                // keep left, delete block and right

                unlinkFromBinList(left);
                unlinkFromBinList(right);

                left->size += (block->size + right->size);

                Block* newRightNeighbour = right->right;
                left->right = newRightNeighbour;
                if (newRightNeighbour) {
                    newRightNeighbour->left = left;
                }

                delete block;
                delete right;

                linkToBinList(left);

                checkInvariants();
                return;
            }

            // b) merge left and block
            if (left != nullptr && left->free) {
                // keep left, delete block

                unlinkFromBinList(left);

                left->size += block->size;
                left->right = right;
                if (right) {
                    right->left = left;
                }

                delete block;

                linkToBinList(left);

                checkInvariants();
                return;
            }

            // c) merge block and right
            if (right != nullptr && right->free) {
                // keep block, delete right

                unlinkFromBinList(right);

                block->size += right->size;
                Block* newRightNeighbour = right->right;
                block->right = newRightNeighbour;
                if (newRightNeighbour != nullptr) {
                    newRightNeighbour->left = block;
                }

                delete right;

                linkToBinList(block);
                block->free = true;

                checkInvariants();
                return;
            }

            // no merging possible.

            linkToBinList(block);
            block->free = true;

            checkInvariants();
        }


        //old

        AllocationTracker::AllocationTracker(Index initial_capacity)
                : m_capacity(0),
                  m_leftmostBlock(nullptr) {
            if (initial_capacity > 0) {
                expand(initial_capacity);
                checkInvariants();
            }
        }

        AllocationTracker::AllocationTracker()
                : m_capacity(0),
                  m_leftmostBlock(nullptr) {}

        AllocationTracker::~AllocationTracker() {
            checkInvariants();

            Block* next;
            for (Block* block = m_leftmostBlock; block != nullptr; block = next) {
                next = block->right;
                delete block;
            }
        }

        size_t AllocationTracker::capacity() const {
            return static_cast<size_t>(m_capacity);
        }

        void AllocationTracker::expand(Index newcap) {
            checkInvariants();

            // special case: empty
            if (m_capacity == 0) {
                assert(newcap > 0);
                m_capacity = newcap;

                Block* newBlock = new Block();
                newBlock->pos = 0;
                newBlock->size = m_capacity;
                newBlock->prevOfSameSize = nullptr;
                newBlock->nextOfSameSize = nullptr;
                newBlock->left = nullptr;
                newBlock->right = nullptr;
                newBlock->free = true;

                m_leftmostBlock = newBlock;

                linkToBinList(newBlock);

                checkInvariants();
                return;
            }

            const Index increase = newcap - m_capacity;
            assert(increase > 0);

            // find the last block
            Block* lastBlock = nullptr;
            for (Block* block = m_leftmostBlock; block != nullptr; block = block->right) {
                lastBlock = block;
            }
            assert(lastBlock != nullptr);

            // 2 cases:
            if (lastBlock->free) {
                // the current buffer ends in a free block. we can just expand it.
                unlinkFromBinList(lastBlock);

                lastBlock->size += increase;

                linkToBinList(lastBlock);
            } else {
                // the current buffer ends in a used block.
                // create a new free block

                Block* newBlock = new Block();
                newBlock->pos = lastBlock->pos + lastBlock->size;
                newBlock->size = increase;
                newBlock->prevOfSameSize = nullptr;
                newBlock->nextOfSameSize = nullptr;
                newBlock->left = lastBlock;
                newBlock->right = nullptr;
                newBlock->free = true;

                linkToBinList(newBlock);

                lastBlock->right = newBlock;
            }

            m_capacity += increase;

            checkInvariants();
        }

// Testing / debugging

        std::set<AllocationTracker::Range> AllocationTracker::freeBlocks() const {
            std::set<Range> res;
            for (Block* block = m_leftmostBlock; block != nullptr; block = block->right) {
                if (block->free) {
                    res.insert(Range{block->pos, block->size});
                }
            }
            return res;
        }

        std::set<AllocationTracker::Range> AllocationTracker::usedBlocks() const {
            std::set<Range> res;
            for (Block* block = m_leftmostBlock; block != nullptr; block = block->right) {
                if (!block->free) {
                    res.insert(Range{block->pos, block->size});
                }
            }
            return res;
        }

        AllocationTracker::Index AllocationTracker::largestPossibleAllocation() const {
            auto it = m_sizeToFreeBlock.crbegin();
            if (it == m_sizeToFreeBlock.crend())
                return 0;

            return it->first;
        }

        void AllocationTracker::checkInvariants() const {
#ifndef EXPENSIVE_CHECKS
            return;
#endif

            if (m_capacity == 0) {
                assert(m_leftmostBlock == nullptr);
                assert(m_sizeToFreeBlock.empty());
                return;
            }

            assert(m_leftmostBlock != nullptr);
            assert(m_leftmostBlock->left == nullptr);
            assert(m_leftmostBlock->pos == 0);

            // check the left/right pointers, size, pos
            size_t totalSize = 0;
            for (Block* block = m_leftmostBlock; block != nullptr; block = block->right) {
                assert(block->size != 0);
                totalSize += block->size;

                if (block->right != nullptr) {
                    assert(block->right->left == block);
                    assert(block->right->pos == block->pos + block->size);
                }

                // used blocks aren't in the nextOfSameSize linked list
                if (!block->free) {
                    assert(block->prevOfSameSize == nullptr);
                    assert(block->nextOfSameSize == nullptr);
                }
            }
            assert(m_capacity == totalSize);

            // check the size map
            for (const auto& [size, headBlock] : m_sizeToFreeBlock) {
                assert(headBlock != nullptr);
                assert(headBlock->prevOfSameSize == nullptr);

                // check they all have the correct size
                for (Block* block = headBlock; block != nullptr; block = block->nextOfSameSize) {
                    assert(block->size == size);
                    assert(block->free);

                    if (block->nextOfSameSize != nullptr) {
                        assert(block->nextOfSameSize->prevOfSameSize == block);
                    }
                }
            }
        }
    }
}
