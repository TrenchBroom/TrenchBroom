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

#include "Renderer/AllocationTracker.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(AllocationTrackerTest, constructor) {
            AllocationTracker t(100);
            EXPECT_EQ(100, t.capacity());
            EXPECT_EQ(100, t.largestPossibleAllocation());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, emptyConstructor) {
            AllocationTracker t;
            EXPECT_EQ(0, t.capacity());
            EXPECT_EQ(0, t.largestPossibleAllocation());
            EXPECT_EQ(false, t.allocate(1).first);
            EXPECT_EQ((std::set<AllocationTracker::Block>{}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, invalidFree) {
            AllocationTracker t(100);
            
            EXPECT_ANY_THROW(t.free(-1));
            EXPECT_ANY_THROW(t.free(0));
            EXPECT_ANY_THROW(t.free(1));
            EXPECT_ANY_THROW(t.free(100));
        }
        
        TEST(AllocationTrackerTest, invalidAllocate) {
            AllocationTracker t(100);
            
            EXPECT_ANY_THROW(t.allocate(-1));
            EXPECT_ANY_THROW(t.allocate(0));
        }
        
        TEST(AllocationTrackerTest, fiveAllocations) {
            AllocationTracker t(500);

            // allocate all the memory
            EXPECT_EQ(std::make_pair(true, 0ll), t.allocate(100));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{100, 400}}), t.freeBlocks());
            
            EXPECT_EQ(std::make_pair(true, 100ll), t.allocate(100));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {100, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{200, 300}}), t.freeBlocks());
            
            EXPECT_EQ(std::make_pair(true, 200ll), t.allocate(100));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {100, 100}, {200, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{300, 200}}), t.freeBlocks());
            
            EXPECT_EQ(std::make_pair(true, 300ll), t.allocate(100));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {100, 100}, {200, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{400, 100}}), t.freeBlocks());
            
            EXPECT_EQ(std::make_pair(true, 400ll), t.allocate(100));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {100, 100}, {200, 100}, {300, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{}), t.freeBlocks());
            
            // further allocations throw
            EXPECT_EQ(false, t.allocate(1).first);
            
            // now start freeing
            EXPECT_EQ(AllocationTracker::Block(100, 100), t.free(100));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {200, 100}, {300, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{100, 100}}), t.freeBlocks());

            EXPECT_EQ(AllocationTracker::Block(300, 100), t.free(300));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {200, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{100, 100}, {300, 100}}), t.freeBlocks());
            EXPECT_EQ(100, t.largestPossibleAllocation());

            EXPECT_EQ(AllocationTracker::Block(200, 100), t.free(200));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{100, 300}}), t.freeBlocks());
            EXPECT_EQ(300, t.largestPossibleAllocation());
            
            // allocate the free block of 300 in the middle
            EXPECT_EQ(false, t.allocate(301).first);
            EXPECT_EQ(std::make_pair(true, 100ll), t.allocate(300));
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}, {100, 300}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{}), t.freeBlocks());
        }
        
        TEST(AllocationTrackerTest, expandEmpty) {
            AllocationTracker t;
            
            t.expand(100);
            EXPECT_EQ(100, t.capacity());
            EXPECT_EQ(100, t.largestPossibleAllocation());
            
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, expandWithFreeSpaceAtEnd) {
            AllocationTracker t(200);
            EXPECT_EQ(std::make_pair(true, 0ll), t.allocate(100));
            EXPECT_EQ(100, t.largestPossibleAllocation());
            
            t.expand(500);
            EXPECT_EQ(500, t.capacity());
            EXPECT_EQ(400, t.largestPossibleAllocation());
            
            EXPECT_EQ((std::set<AllocationTracker::Block>{{100, 400}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 100}}), t.usedBlocks());
        }
        
        TEST(AllocationTrackerTest, expandWithUsedSpaceAtEnd) {
            AllocationTracker t(200);
            EXPECT_EQ(std::make_pair(true, 0ll), t.allocate(200));
            EXPECT_EQ(0, t.largestPossibleAllocation());
            EXPECT_EQ(false, t.allocate(1).first);
            
            t.expand(500);
            EXPECT_EQ(500, t.capacity());
            EXPECT_EQ(300, t.largestPossibleAllocation());

            EXPECT_EQ((std::set<AllocationTracker::Block>{{200, 300}}), t.freeBlocks());
            EXPECT_EQ((std::set<AllocationTracker::Block>{{0, 200}}), t.usedBlocks());
            
            EXPECT_EQ(false, t.allocate(301).first);
            EXPECT_EQ(std::make_pair(true, 200ll), t.allocate(300));
        }

        static constexpr size_t NumBrushes = 64'000;

        TEST(AllocationTrackerTest, benchmark) {
            AllocationTracker t(100 * NumBrushes);
            for (size_t i = 0; i < NumBrushes; ++i) {
                t.allocate(100);
            }
            EXPECT_EQ(0, t.largestPossibleAllocation());
        }
    }
}

