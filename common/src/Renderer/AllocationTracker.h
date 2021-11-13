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

#pragma once

#include <vector>

namespace TrenchBroom {
namespace Renderer {
/**
 * Implements bookkeeping for dynamic memory allocation (like malloc).
 */
class AllocationTracker {
public:
  using Index = size_t;

  struct Block {
  public:
    Index pos;
    Index size;

  private:
    friend class AllocationTracker;
    /**
     * If this is null, it means we're the head of the list in the
     * m_freeBlockSizeBins map.
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
    /**
     * Null iff m_rightmostBlock points to this block.
     * Links to the Block at (pos + size).
     */
    Block* right;

    bool free;

    Block* nextRecycledBlock;
  };

private:
  /**
   * Size of memory managed by this AllocationTracker.
   * Always equal to the sum of `size` of all Blocks.
   */
  Index m_capacity;

  /**
   * Points to the Block with pos 0. Used to free all of the blocks in the destructor
   */
  Block* m_leftmostBlock;
  /**
   * Points to the Block with the highest pos. Used when expanding.
   */
  Block* m_rightmostBlock;

  /**
   * Instead of always calling new/delete on Blocks when splitting/merging,
   * we keep a singly-linked list of "recycled" blocks (using the nextRecycledBlock pointer).
   * If the list is empty, then fall back to calling `new Block`.
   */
  Block* m_recycledBlockList;

  /**
   * A map from Block size to a linked list of Blocks of that exact size
   * (the linked list is stored in the prevOfSameSize/nextOfSameSize pointers)
   *
   * Sorted vector, benchmarks faster than std::map for this use case.
   */
  std::vector<Block*> m_freeBlockSizeBins;

  /**
   * Unlinks a Block from m_freeBlockSizeBins. Must be called before modifying Block::size.
   */
  void unlinkFromBinList(Block* block);
  void linkToBinList(Block* block);

  void recycle(Block* block);
  Block* obtainBlock();

public:
  explicit AllocationTracker(Index initial_capacity);
  AllocationTracker();
  ~AllocationTracker();

  /**
   * Tries to make an allocation. Returns nullptr if there is no room for the requested allocation.
   *
   * If the allocation is successful, the returned block->size is guaranteed to equal `size`.
   * The caller can read block->pos to find out where in the buffer the allocation was made.
   *
   * The AllocationTracker owns the Block object itself.
   */
  Block* allocate(size_t size);
  void free(Block* block);
  size_t capacity() const;
  void expand(Index newCapacity);
  /**
   * @return whether there are any allocations. i.e. returns false iff the whole range managed by
   * the allocation tracker is free. Returns false if `capacity() == 0`. Constant time.
   */
  bool hasAllocations() const;

  // Testing / debugging

  class Range {
  public:
    Index pos;
    Index size;

    Range(Index p, Index s);

    bool operator==(const Range& other) const;
    bool operator<(const Range& other) const;
  };

  std::vector<Range> freeBlocks() const;
  std::vector<Range> usedBlocks() const;
  Index largestPossibleAllocation() const;
  void checkInvariants() const;
};
} // namespace Renderer
} // namespace TrenchBroom
