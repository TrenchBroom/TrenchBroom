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

namespace TrenchBroom {
    namespace Renderer {
        AllocationTracker::Block::Block(Index p, Index s)
                : pos(p), size(s) {}

        bool AllocationTracker::Block::operator==(const Block &other) const {
            return pos == other.pos
                   && size == other.size;
        }

        bool AllocationTracker::Block::operator<(const Block &other) const {
            if (pos < other.pos) return true;
            if (pos > other.pos) return false;

            return size < other.size;
        }

        void AllocationTracker::eraseFree(Block b) {
            // update m_sizeToFreePositions
            auto size_it = m_sizeToFreePositions.find(b.size);
            assert(size_it != m_sizeToFreePositions.end());

            std::set<Index> &pos_set = size_it->second;
            auto pos_set_it = pos_set.find(b.pos);
            assert(pos_set_it != pos_set.end());
            pos_set.erase(pos_set_it);

            // it is important to prune empty std::set's from the map,
            // otherwise the m_sizeToFreePositions.lower_bound() call in allocate() would find them
            if (pos_set.empty()) {
                m_sizeToFreePositions.erase(size_it);
            }

            // update m_posToFreeSize
            auto pos_it = m_posToFreeSize.find(b.pos);
            assert(pos_it != m_posToFreeSize.end());
            m_posToFreeSize.erase(pos_it);

            // update m_endPosToFreePos
            auto endpos_it = m_endPosToFreePos.find(b.pos + b.size);
            assert(endpos_it != m_endPosToFreePos.end());
            m_endPosToFreePos.erase(endpos_it);
        }

        void AllocationTracker::insertFree(Block b) {
            assert(b.size > 0);

            m_sizeToFreePositions[b.size].insert(b.pos);
            m_posToFreeSize[b.pos] = b.size;
            m_endPosToFreePos[b.pos + b.size] = b.pos;
        }

        AllocationTracker::AllocationTracker(Index initial_capacity)
                : m_capacity(initial_capacity) {
            if (initial_capacity > 0) {
                insertFree(Block{0, initial_capacity});
            }
        }

        AllocationTracker::AllocationTracker()
                : m_capacity(0) {}

        std::pair<bool, AllocationTracker::Index> AllocationTracker::allocate(const size_t bytesUnsigned) {
            const Index bytes = static_cast<Index>(bytesUnsigned);

            if (bytes <= 0)
                throw std::runtime_error("allocate() requires positive nonzero size");

            // find the smallest free block that will fit the allocation
            auto it = m_sizeToFreePositions.lower_bound(bytes);
            if (it == m_sizeToFreePositions.end()) {
                return {false, 0};
            }
            const Block oldFree{*it->second.begin(), it->first};

            // chop off the start of the free block that will be in use now
            Block newFree = oldFree;
            newFree.pos += bytes;
            newFree.size -= bytes;
            assert(newFree.size >= 0);

            // update free block maps
            eraseFree(oldFree);
            if (newFree.size > 0)
                insertFree(newFree);

            // insert the used block
            m_posToUsedSize[oldFree.pos] = bytes;

            return {true, oldFree.pos};
        }

        AllocationTracker::Block AllocationTracker::free(Index pos) {
            // remove the used block
            auto it = m_posToUsedSize.find(pos);
            if (it == m_posToUsedSize.end())
                throw std::runtime_error("free(): invalid address");
            const Block oldUsedBlock{pos, it->second};
            m_posToUsedSize.erase(it);

            Block newFree = oldUsedBlock;

            // check for an immediately following free block
            const Index oldUsedBlockEnd = oldUsedBlock.pos + oldUsedBlock.size;
            auto it2 = m_posToFreeSize.find(oldUsedBlockEnd);
            if (it2 != m_posToFreeSize.end()) {
                const Block followingFree{oldUsedBlockEnd, it2->second};
                eraseFree(followingFree);

                newFree.size += followingFree.size;
            }

            // check for an immediately preceeding free block
            auto it3 = m_endPosToFreePos.find(oldUsedBlock.pos);
            if (it3 != m_endPosToFreePos.end()) {
                const Block preceedingFree{it3->second, oldUsedBlock.pos - it3->second};
                eraseFree(preceedingFree);

                newFree.pos -= preceedingFree.size;
                newFree.size += preceedingFree.size;
            }

            insertFree(newFree);

            return oldUsedBlock;
        }

        size_t AllocationTracker::capacity() const {
            return static_cast<size_t>(m_capacity);
        }

        void AllocationTracker::expand(Index newcap) {
            // special case: empty
            if (m_capacity == 0) {
                assert(newcap > 0);
                m_capacity = newcap;
                insertFree({0, newcap});
                return;
            }

            const Index increase = newcap - m_capacity;
            assert(increase > 0);

            // 2 cases:
            auto it = m_endPosToFreePos.find(m_capacity);
            if (it != m_endPosToFreePos.end()) {
                // the current buffer ends in a free block. we can just expand it.

                const Block oldFree{it->second, m_capacity - it->second};
                eraseFree(oldFree);

                const Block newFree{oldFree.pos, oldFree.size + increase};
                insertFree(newFree);
            } else {
                // the current buffer ends in a used block.
                // create a new free block

                const Block newFree{m_capacity, increase};
                insertFree(newFree);
            }

            m_capacity += increase;
        }

// Testing / debugging

        std::set<AllocationTracker::Block> AllocationTracker::freeBlocks() const {
            std::set<Block> res;
            for (const auto &pr : m_posToFreeSize) {
                res.insert(Block{pr.first, pr.second});
            }
            return res;
        }

        std::set<AllocationTracker::Block> AllocationTracker::usedBlocks() const {
            std::set<Block> res;
            for (const auto &pr : m_posToUsedSize) {
                res.insert(Block{pr.first, pr.second});
            }
            return res;
        }

        AllocationTracker::Index AllocationTracker::largestPossibleAllocation() const {
            auto it = m_sizeToFreePositions.crbegin();
            if (it == m_sizeToFreePositions.crend())
                return 0;

            return it->first;
        }
    }
}

