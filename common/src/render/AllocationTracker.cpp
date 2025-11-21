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

#include "kd/contracts.h"
#include "kd/vector_set.h"

#include <algorithm>

// #define EXPENSIVE_CHECKS

namespace tb::render
{

AllocationTracker::Range::Range(Index p, Index s)
  : pos{p}
  , size{s}
{
}

static std::vector<AllocationTracker::Block*>::iterator findFirstLargerOrEqualBin(
  std::vector<AllocationTracker::Block*>& bins, const size_t desiredSize)
{
  return std::ranges::lower_bound(
    bins, desiredSize, std::less<size_t>{}, [](const auto* block) {
      return block->size;
    });
}

void AllocationTracker::unlinkFromBinList(Block* block)
{
  contract_pre(block->free);

  if (block->prevOfSameSize == nullptr)
  {
    // slow case: when we are the head of the list
    // (m_sizeToFreeBlock has a pointer to us)

    // this means we must be in m_sizeToFreeBlock
    auto it = findFirstLargerOrEqualBin(m_freeBlockSizeBins, block->size);
    contract_assert(it != m_freeBlockSizeBins.end());
    contract_assert(*it == block);

    // make sure we prune empty lists from the map!
    if (block->nextOfSameSize == nullptr)
    {
      // NOTE: O(n) in the number of bins
      m_freeBlockSizeBins.erase(it);
    }
    else
    {
      *it = block->nextOfSameSize;
      block->nextOfSameSize->prevOfSameSize = nullptr;
      block->nextOfSameSize = nullptr;
    }
  }
  else
  {
    // "regular" case, not the head of a size bin list.

    // handle the "previous" side
    contract_assert(block->prevOfSameSize != nullptr);
    contract_assert(block->size == block->prevOfSameSize->size);
    block->prevOfSameSize->nextOfSameSize = block->nextOfSameSize;

    // handle the "next" side
    if (block->nextOfSameSize)
    {
      contract_assert(block->size == block->nextOfSameSize->size);
      block->nextOfSameSize->prevOfSameSize = block->prevOfSameSize;
    }

    // clear the nextOfSameSize/prevOfSameSize pointers to mark the block as unlinked from
    // the bin list
    block->nextOfSameSize = nullptr;
    block->prevOfSameSize = nullptr;
  }
}

void AllocationTracker::linkToBinList(Block* block)
{
  contract_pre(block->free);
  contract_pre(block->size > 0);
  contract_pre(block->prevOfSameSize == nullptr);
  contract_pre(block->nextOfSameSize == nullptr);

  auto it = findFirstLargerOrEqualBin(m_freeBlockSizeBins, block->size);

  if (it == m_freeBlockSizeBins.end())
  {
    // All existing bins too small; insert at end.
    m_freeBlockSizeBins.insert(it, block);
  }
  else if ((*it)->size == block->size)
  {
    // There is an existing exact match for the bin size, so we don't need to resize the
    // vector.
    Block* previousListHead = *it;

    contract_assert(previousListHead->size == block->size);
    contract_assert(previousListHead->prevOfSameSize == nullptr);

    // connect block to previousListHead
    block->nextOfSameSize = previousListHead;
    previousListHead->prevOfSameSize = block;

    // NOTE: inserts into the map
    *it = block;
  }
  else
  {
    // Slow case: insert a new bin, before `it`.
    m_freeBlockSizeBins.insert(it, block);
  }
}

void AllocationTracker::recycle(Block* block)
{
  block->nextRecycledBlock = m_recycledBlockList;
  m_recycledBlockList = block;
}

AllocationTracker::Block* AllocationTracker::obtainBlock()
{
  if (m_recycledBlockList != nullptr)
  {
    Block* newBlock = m_recycledBlockList;
    m_recycledBlockList = newBlock->nextRecycledBlock;
    return newBlock;
  }
  return new Block();
}

AllocationTracker::Block* AllocationTracker::allocate(const size_t needed)
{
  contract_pre(needed > 0);

  checkInvariants();

  // find the smallest free block that will fit the allocation
  auto it = findFirstLargerOrEqualBin(m_freeBlockSizeBins, needed);
  if (it == m_freeBlockSizeBins.end())
  {
    checkInvariants();
    return nullptr;
  }

  // unlink it from the size bin
  // (this is a special case of unlinkFromBinList(), duplicated here
  // to avoid doing a redundant binary search)
  Block* block = *it;
  contract_assert(block != nullptr);
  contract_assert(block->free);
  contract_assert(block->prevOfSameSize == nullptr);
  {
    Block* blockAfter = block->nextOfSameSize;
    if (blockAfter == nullptr)
    {
      m_freeBlockSizeBins.erase(it);
    }
    else
    {
      *it = blockAfter;
      blockAfter->prevOfSameSize = nullptr;
    }
  }

  block->nextOfSameSize = nullptr;
  block->prevOfSameSize = nullptr;

  if (block->size == needed)
  {
    // lucky case: exact size. we're done
    block->free = false;

    checkInvariants();
    return block;
  }

  // common case:
  // the block is too large, split off the part we need
  contract_assert(block->size > needed);

  // this will be the left section of `block`
  Block* newBlock = obtainBlock();
  newBlock->pos = block->pos;
  newBlock->size = needed;
  newBlock->prevOfSameSize = nullptr;
  newBlock->nextOfSameSize = nullptr;
  newBlock->left = block->left;
  newBlock->right = block;
  newBlock->free = false;

  // update the block that was to the left of block
  if (block->left == nullptr)
  {
    // update m_leftmostBlock
    contract_assert(m_leftmostBlock == block);
    m_leftmostBlock = newBlock;
  }
  else
  {
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

void AllocationTracker::free(Block* block)
{
  contract_pre(!block->free);
  contract_pre(block->prevOfSameSize == nullptr);
  contract_pre(block->nextOfSameSize == nullptr);

  checkInvariants();

  Block* left = block->left;
  Block* right = block->right;

  // 3 possible cases for merging blocks:
  // a) merge left, block, and right
  if (left != nullptr && left->free && right != nullptr && right->free)
  {

    // keep left, delete block and right

    unlinkFromBinList(left);
    unlinkFromBinList(right);

    left->size += (block->size + right->size);

    Block* newRightNeighbour = right->right;
    left->right = newRightNeighbour;
    if (newRightNeighbour)
    {
      newRightNeighbour->left = left;
    }

    recycle(block);
    recycle(right);

    linkToBinList(left);

    // update rightmost block
    if (m_rightmostBlock == right)
    {
      m_rightmostBlock = left;
    }

    checkInvariants();
    return;
  }

  // b) merge left and block
  if (left != nullptr && left->free)
  {
    // keep left, delete block

    unlinkFromBinList(left);

    left->size += block->size;
    left->right = right;
    if (right)
    {
      right->left = left;
    }

    recycle(block);

    linkToBinList(left);

    // update rightmost block
    if (m_rightmostBlock == block)
    {
      m_rightmostBlock = left;
    }

    checkInvariants();
    return;
  }

  // c) merge block and right
  if (right != nullptr && right->free)
  {
    // keep block, delete right

    unlinkFromBinList(right);

    block->size += right->size;
    Block* newRightNeighbour = right->right;
    block->right = newRightNeighbour;
    if (newRightNeighbour != nullptr)
    {
      newRightNeighbour->left = block;
    }

    recycle(right);

    block->free = true;
    linkToBinList(block);

    // update rightmost block
    if (m_rightmostBlock == right)
    {
      m_rightmostBlock = block;
    }

    checkInvariants();
    return;
  }

  // no merging possible.

  block->free = true;
  linkToBinList(block);

  checkInvariants();
}

AllocationTracker::AllocationTracker(const Index initial_capacity)
  : m_capacity(0)
  , m_leftmostBlock(nullptr)
  , m_rightmostBlock(nullptr)
  , m_recycledBlockList(nullptr)
{
  if (initial_capacity > 0)
  {
    expand(initial_capacity);
    checkInvariants();
  }
}

AllocationTracker::AllocationTracker()
  : m_capacity(0)
  , m_leftmostBlock(nullptr)
  , m_rightmostBlock(nullptr)
  , m_recycledBlockList(nullptr)
{
}

AllocationTracker::~AllocationTracker()
{
  checkInvariants();

  Block* next;
  for (Block* block = m_leftmostBlock; block != nullptr; block = next)
  {
    next = block->right;
    delete block;
  }

  for (Block* block = m_recycledBlockList; block != nullptr; block = next)
  {
    next = block->nextRecycledBlock;
    delete block;
  }
}

size_t AllocationTracker::capacity() const
{
  return static_cast<size_t>(m_capacity);
}

void AllocationTracker::expand(const Index newCapacity)
{
  contract_pre(newCapacity > 0);

  checkInvariants();

  // special case: empty
  if (m_capacity == 0)
  {
    m_capacity = newCapacity;

    Block* newBlock = obtainBlock();
    newBlock->pos = 0;
    newBlock->size = m_capacity;
    newBlock->prevOfSameSize = nullptr;
    newBlock->nextOfSameSize = nullptr;
    newBlock->left = nullptr;
    newBlock->right = nullptr;
    newBlock->free = true;

    m_leftmostBlock = newBlock;
    m_rightmostBlock = newBlock;

    linkToBinList(newBlock);

    checkInvariants();
    return;
  }

  const Index increase = newCapacity - m_capacity;

  // 2 cases:
  Block* lastBlock = m_rightmostBlock;
  if (lastBlock->free)
  {
    // the current buffer ends in a free block. we can just expand it.
    unlinkFromBinList(lastBlock);

    lastBlock->size += increase;

    linkToBinList(lastBlock);
  }
  else
  {
    // the current buffer ends in a used block.
    // create a new free block

    Block* newBlock = obtainBlock();
    newBlock->pos = lastBlock->pos + lastBlock->size;
    newBlock->size = increase;
    newBlock->prevOfSameSize = nullptr;
    newBlock->nextOfSameSize = nullptr;
    newBlock->left = lastBlock;
    newBlock->right = nullptr;
    newBlock->free = true;

    linkToBinList(newBlock);

    lastBlock->right = newBlock;

    m_rightmostBlock = newBlock;
  }

  m_capacity += increase;

  checkInvariants();
}

bool AllocationTracker::hasAllocations() const
{
  // NOTE: this loop should execute at most 2 iterations, because adjacent free blocks are
  // always merged
  for (Block* block = m_leftmostBlock; block != nullptr; block = block->right)
  {
    if (!block->free)
    {
      return true;
    }
  }
  return false;
}

// Testing / debugging

std::vector<AllocationTracker::Range> AllocationTracker::freeBlocks() const
{
  kdl::vector_set<Range> res;
  for (Block* block = m_leftmostBlock; block != nullptr; block = block->right)
  {
    if (block->free)
    {
      res.insert(Range{block->pos, block->size});
    }
  }
  return res.release_data();
}

std::vector<AllocationTracker::Range> AllocationTracker::usedBlocks() const
{
  kdl::vector_set<Range> res;
  for (Block* block = m_leftmostBlock; block != nullptr; block = block->right)
  {
    if (!block->free)
    {
      res.insert(Range{block->pos, block->size});
    }
  }
  return res.release_data();
}

AllocationTracker::Index AllocationTracker::largestPossibleAllocation() const
{
  auto it = m_freeBlockSizeBins.crbegin();
  return it != m_freeBlockSizeBins.crend() ? (*it)->size : 0;
}

void AllocationTracker::checkInvariants() const
{
#ifdef EXPENSIVE_CHECKS
  if (m_capacity == 0)
  {
    contract_assert(m_leftmostBlock == nullptr);
    contract_assert(m_rightmostBlock == nullptr);
    contract_assert(m_freeBlockSizeBins.empty());
    return;
  }

  contract_assert(m_leftmostBlock != nullptr);
  contract_assert(m_leftmostBlock->left == nullptr);
  contract_assert(m_leftmostBlock->pos == 0);

  contract_assert(m_rightmostBlock != nullptr);
  contract_assert(m_rightmostBlock->right == nullptr);

  // check the left/right pointers, size, pos
  size_t totalSize = 0;
  for (Block* block = m_leftmostBlock; block != nullptr; block = block->right)
  {
    contract_assert(block->size != 0);
    totalSize += block->size;

    if (block->right != nullptr)
    {
      contract_assert(block->right->left == block);
      contract_assert(block->right->pos == block->pos + block->size);
    }
    else
    {
      // rightmost block
      contract_assert(block == m_rightmostBlock);
    }

    // used blocks aren't in the nextOfSameSize linked list
    if (!block->free)
    {
      contract_assert(block->prevOfSameSize == nullptr);
      contract_assert(block->nextOfSameSize == nullptr);
    }
  }
  contract_assert(m_capacity == totalSize);

  // check the size map
  for (const auto& headBlock : m_freeBlockSizeBins)
  {
    contract_assert(headBlock != nullptr);
    contract_assert(headBlock->prevOfSameSize == nullptr);

    // check they all have the correct size
    for (Block* block = headBlock; block != nullptr; block = block->nextOfSameSize)
    {
      contract_assert(block->free);
      contract_assert(block->size == headBlock->size);

      if (block->nextOfSameSize != nullptr)
      {
        contract_assert(block->nextOfSameSize->prevOfSameSize == block);
      }
    }
  }

  // ensure the size bins are sorted
  for (size_t i = 0; (i + 1) < m_freeBlockSizeBins.size(); ++i)
  {
    const auto& a = m_freeBlockSizeBins.at(i);
    const auto& b = m_freeBlockSizeBins.at(i + 1);
    contract_assert(a->size < b->size);
  }
#endif
}

} // namespace tb::render
