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
#include <exception>
#include <cassert>

/**
 * Implements the bookkeeping part of a malloc() like interface.
 * 
 */
class AllocationTracker {
public:
    using Index = int64_t;
    
    class Block {
    public:
        Index pos;
        Index size;
        
        Block(Index p, Index s)
        : pos(p), size(s) {}
        
        bool operator==(const Block& other) const {
            return pos == other.pos
                && size == other.size;
        }
        
        bool operator<(const Block& other) const {
            if (pos < other.pos) return true;
            if (pos > other.pos) return false;
            
            return size < other.size;
        }
    };
    
private:

    Index m_capacity;

    // track free space by 3 indices
    std::map<Index, std::set<Index>> m_sizeToFreePositions;
    std::map<Index, Index> m_posToFreeSize;
    std::map<Index, Index> m_endPosToFreePos;

    // track used space
    std::map<Index, Index> m_posToUsedSize;

private:
    void eraseFree(Block b) {
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

    void insertFree(Block b) {
        assert(b.size > 0);

        m_sizeToFreePositions[b.size].insert(b.pos);
        m_posToFreeSize[b.pos] = b.size;
        m_endPosToFreePos[b.pos + b.size] = b.pos;
    }

public:
    explicit AllocationTracker(Index initial_capacity)
    : m_capacity(initial_capacity) {
        insertFree(Block{0, initial_capacity});
    }
    
    AllocationTracker()
    : m_capacity(0) {}

public:

    Index allocate(Index bytes) {
        if (bytes <= 0)
            throw std::runtime_error("allocate() requires positive nonzero size");
        
        // find the smallest free block that will fit the allocation
        auto it = m_sizeToFreePositions.lower_bound(bytes);
        if (it == m_sizeToFreePositions.end()) {
            throw std::runtime_error("out of space");
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

        return oldFree.pos;
    }

    void free(Index pos) {
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
    }

    Index capacity() const {
        return m_capacity;
    }

    void expand(Index newcap) {
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
    
public:
    // Testing / debugging
    
    std::set<Block> freeBlocks() const {
        std::set<Block> res;
        for (const auto& pr : m_posToFreeSize) {
            res.insert(Block{pr.first, pr.second});
        }
        return res;
    }
    
    std::set<Block> usedBlocks() const {
        std::set<Block> res;
        for (const auto& pr : m_posToUsedSize) {
            res.insert(Block{pr.first, pr.second});
        }
        return res;
    }
    
    Index largestPossibleAllocation() const {
        auto it = m_sizeToFreePositions.crbegin();
        if (it == m_sizeToFreePositions.crend())
            return 0;
        
        return it->first;
    }
};

#endif
