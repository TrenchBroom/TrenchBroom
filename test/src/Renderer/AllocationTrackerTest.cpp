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

#include <gtest/gtest.h>
#include <random>

#include "Renderer/AllocationTracker.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(AllocationTrackerTest, constructor) {
            AllocationTracker t(100);
            EXPECT_EQ(100, t.capacity());
            EXPECT_EQ(100, t.largestPossibleAllocation());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, emptyConstructor) {
            AllocationTracker t;
            EXPECT_EQ(0, t.capacity());
            EXPECT_EQ(0, t.largestPossibleAllocation());
            EXPECT_EQ(nullptr, t.allocate(1));
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.usedBlocks());
        }

        TEST(AllocationTrackerTest, constructWithZeroCapacity) {
            AllocationTracker t(0);
            EXPECT_EQ(0, t.capacity());
            EXPECT_EQ(0, t.largestPossibleAllocation());
            EXPECT_EQ(nullptr, t.allocate(1));
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, invalidAllocate) {
            AllocationTracker t(100);

            EXPECT_ANY_THROW(t.allocate(0));
        }
        
        TEST(AllocationTrackerTest, fiveAllocations) {
            AllocationTracker t(500);

            // allocate all the memory
            AllocationTracker::Block* blocks[5];

            blocks[0] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[0]);
            EXPECT_EQ(0, blocks[0]->pos);
            EXPECT_EQ(100, blocks[0]->size);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 400}}), t.freeBlocks());

            blocks[1] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[1]);
            EXPECT_EQ(100, blocks[1]->pos);
            EXPECT_EQ(100, blocks[1]->size);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {100, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{200, 300}}), t.freeBlocks());

            blocks[2] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[2]);
            EXPECT_EQ(200, blocks[2]->pos);
            EXPECT_EQ(100, blocks[2]->size);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{300, 200}}), t.freeBlocks());

            blocks[3] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[3]);
            EXPECT_EQ(300, blocks[3]->pos);
            EXPECT_EQ(100, blocks[3]->size);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{400, 100}}), t.freeBlocks());

            blocks[4] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[4]);
            EXPECT_EQ(400, blocks[4]->pos);
            EXPECT_EQ(100, blocks[4]->size);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}, {300, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.freeBlocks());
            
            // further allocations throw
            EXPECT_EQ(nullptr, t.allocate(1));
            
            // now start freeing
            t.free(blocks[1]);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {200, 100}, {300, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 100}}), t.freeBlocks());

            t.free(blocks[3]);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {200, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 100}, {300, 100}}), t.freeBlocks());
            EXPECT_EQ(100, t.largestPossibleAllocation());

            // this will cause a merge with the left and right free blocks
            t.free(blocks[2]);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 300}}), t.freeBlocks());
            EXPECT_EQ(300, t.largestPossibleAllocation());
            
            // allocate the free block of 300 in the middle
            EXPECT_EQ(nullptr, t.allocate(301));
            AllocationTracker::Block* newBlock = t.allocate(300);
            ASSERT_NE(nullptr, newBlock);
            EXPECT_EQ(100, newBlock->pos);
            EXPECT_EQ(300, newBlock->size);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {100, 300}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.freeBlocks());
        }

        TEST(AllocationTrackerTest, freeMergeRight) {
            AllocationTracker t(400);

            // allocate all the memory
            AllocationTracker::Block* blocks[4];

            blocks[0] = t.allocate(100);
            blocks[1] = t.allocate(100);
            blocks[2] = t.allocate(100);
            blocks[3] = t.allocate(100);
            EXPECT_EQ(0, t.largestPossibleAllocation());

            // now start freeing
            t.free(blocks[2]);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {100, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{200, 100}}), t.freeBlocks());

            // this will merge with the right free block
            t.free(blocks[1]);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 200}}), t.freeBlocks());

            EXPECT_EQ(200, t.largestPossibleAllocation());
        }

        TEST(AllocationTrackerTest, freeMergeLeft) {
            AllocationTracker t(400);

            // allocate all the memory
            AllocationTracker::Block* blocks[4];

            blocks[0] = t.allocate(100);
            blocks[1] = t.allocate(100);
            blocks[2] = t.allocate(100);
            blocks[3] = t.allocate(100);
            EXPECT_EQ(0, t.largestPossibleAllocation());

            // now start freeing
            t.free(blocks[1]);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {200, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 100}}), t.freeBlocks());

            // this will merge with the left free block
            t.free(blocks[2]);
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 200}}), t.freeBlocks());

            EXPECT_EQ(200, t.largestPossibleAllocation());
        }
        
        TEST(AllocationTrackerTest, expandEmpty) {
            AllocationTracker t;
            
            t.expand(100);
            EXPECT_EQ(100, t.capacity());
            EXPECT_EQ(100, t.largestPossibleAllocation());
            
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, expandWithFreeSpaceAtEnd) {
            AllocationTracker t(200);

            AllocationTracker::Block* newBlock = t.allocate(100);
            ASSERT_NE(nullptr, newBlock);
            EXPECT_EQ(0, newBlock->pos);
            EXPECT_EQ(100, newBlock->size);

            EXPECT_EQ(100, t.largestPossibleAllocation());
            
            t.expand(500);
            EXPECT_EQ(500, t.capacity());
            EXPECT_EQ(400, t.largestPossibleAllocation());
            
            EXPECT_EQ((std::set<AllocationTracker::Range>{{100, 400}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 100}}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, expandWithUsedSpaceAtEnd) {
            AllocationTracker t(200);

            {
                AllocationTracker::Block *newBlock = t.allocate(200);
                ASSERT_NE(nullptr, newBlock);
                EXPECT_EQ(0, newBlock->pos);
                EXPECT_EQ(0, t.largestPossibleAllocation());
                EXPECT_EQ(nullptr, t.allocate(1));
            }
            
            t.expand(500);
            EXPECT_EQ(500, t.capacity());
            EXPECT_EQ(300, t.largestPossibleAllocation());

            EXPECT_EQ((std::set<AllocationTracker::Range>{{200, 300}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Range>{{0, 200}}), t.usedBlocks());
            
            EXPECT_EQ(nullptr, t.allocate(301));

            {
                AllocationTracker::Block *newBlock2 = t.allocate(300);
                ASSERT_NE(nullptr, newBlock2);
                EXPECT_EQ(200, newBlock2->pos);
            }
        }

        static constexpr size_t NumBrushes = 64'000;

        // between 12 and 140, inclusive.
        static size_t getBrushSizeFromRandEngine(std::mt19937& engine) {
            return 12 + (4 * (engine() % 33));
        }

        TEST(AllocationTrackerTest, benchmarkAllocOnly) {
            std::mt19937 randEngine;

            AllocationTracker t(140 * NumBrushes);
            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                EXPECT_NE(nullptr, t.allocate(brushSize));
            }
        }

        TEST(AllocationTrackerTest, benchmarkAllocFreeAlloc) {
            std::mt19937 randEngine;

            AllocationTracker t(140 * NumBrushes);
            AllocationTracker::Block **allocations = new AllocationTracker::Block*[NumBrushes];

            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                allocations[i] = t.allocate(brushSize);
                EXPECT_NE(nullptr, allocations[i]);
            }
            for (size_t i = 0; i < NumBrushes; ++i) {
                t.free(allocations[i]);
            }
            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                allocations[i] = t.allocate(brushSize);
                EXPECT_NE(nullptr, allocations[i]);
            }

            delete[] allocations;
        }
    }
}

