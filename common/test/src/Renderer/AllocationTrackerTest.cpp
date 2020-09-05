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

#include <catch2/catch.hpp>

#include "GTestCompat.h"
#include <random>
#include <algorithm>
#include <vector>

#include "Renderer/AllocationTracker.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST_CASE("AllocationTrackerTest.constructor", "[AllocationTrackerTest]") {
            AllocationTracker t(100);
            EXPECT_EQ(100u, t.capacity());
            EXPECT_EQ(100u, t.largestPossibleAllocation());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}}), t.freeBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.usedBlocks());
            EXPECT_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.emptyConstructor", "[AllocationTrackerTest]") {
            AllocationTracker t;
            EXPECT_EQ(0u, t.capacity());
            EXPECT_EQ(0u, t.largestPossibleAllocation());
            EXPECT_EQ(nullptr, t.allocate(1));
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.freeBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.usedBlocks());
            EXPECT_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.constructWithZeroCapacity", "[AllocationTrackerTest]") {
            AllocationTracker t(0);
            EXPECT_EQ(0u, t.capacity());
            EXPECT_EQ(0u, t.largestPossibleAllocation());
            EXPECT_EQ(nullptr, t.allocate(1));
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.freeBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.usedBlocks());
            EXPECT_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.invalidAllocate", "[AllocationTrackerTest]") {
            AllocationTracker t(100);

            EXPECT_ANY_THROW(t.allocate(0));

            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}}), t.freeBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.usedBlocks());
            EXPECT_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.fiveAllocations", "[AllocationTrackerTest]") {
            AllocationTracker t(500);

            // allocate all the memory
            AllocationTracker::Block* blocks[5];

            blocks[0] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[0]);
            EXPECT_EQ(0u, blocks[0]->pos);
            EXPECT_EQ(100u, blocks[0]->size);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 400}}), t.freeBlocks());
            EXPECT_TRUE(t.hasAllocations());

            blocks[1] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[1]);
            EXPECT_EQ(100u, blocks[1]->pos);
            EXPECT_EQ(100u, blocks[1]->size);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{200, 300}}), t.freeBlocks());

            blocks[2] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[2]);
            EXPECT_EQ(200u, blocks[2]->pos);
            EXPECT_EQ(100u, blocks[2]->size);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{300, 200}}), t.freeBlocks());

            blocks[3] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[3]);
            EXPECT_EQ(300u, blocks[3]->pos);
            EXPECT_EQ(100u, blocks[3]->size);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{400, 100}}), t.freeBlocks());

            blocks[4] = t.allocate(100);
            ASSERT_NE(nullptr, blocks[4]);
            EXPECT_EQ(400u, blocks[4]->pos);
            EXPECT_EQ(100u, blocks[4]->size);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}, {300, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.freeBlocks());

            // further allocations throw
            EXPECT_EQ(nullptr, t.allocate(1));

            // now start freeing
            t.free(blocks[1]);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {200, 100}, {300, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 100}}), t.freeBlocks());

            t.free(blocks[3]);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {200, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 100}, {300, 100}}), t.freeBlocks());
            EXPECT_EQ(100u, t.largestPossibleAllocation());

            // this will cause a merge with the left and right free blocks
            t.free(blocks[2]);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 300}}), t.freeBlocks());
            EXPECT_EQ(300u, t.largestPossibleAllocation());

            // allocate the free block of 300 in the middle
            EXPECT_EQ(nullptr, t.allocate(301));
            AllocationTracker::Block* newBlock = t.allocate(300);
            ASSERT_NE(nullptr, newBlock);
            EXPECT_EQ(100u, newBlock->pos);
            EXPECT_EQ(300u, newBlock->size);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {100, 300}, {400, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.freeBlocks());
        }

        TEST_CASE("AllocationTrackerTest.freeMergeRight", "[AllocationTrackerTest]") {
            AllocationTracker t(400);

            // allocate all the memory
            AllocationTracker::Block* blocks[4];

            blocks[0] = t.allocate(100);
            blocks[1] = t.allocate(100);
            blocks[2] = t.allocate(100);
            blocks[3] = t.allocate(100);
            EXPECT_EQ(0u, t.largestPossibleAllocation());

            // now start freeing
            t.free(blocks[2]);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{200, 100}}), t.freeBlocks());

            // this will merge with the right free block
            t.free(blocks[1]);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 200}}), t.freeBlocks());

            EXPECT_EQ(200u, t.largestPossibleAllocation());
        }

        TEST_CASE("AllocationTrackerTest.freeMergeLeft", "[AllocationTrackerTest]") {
            AllocationTracker t(400);

            // allocate all the memory
            AllocationTracker::Block* blocks[4];

            blocks[0] = t.allocate(100);
            blocks[1] = t.allocate(100);
            blocks[2] = t.allocate(100);
            blocks[3] = t.allocate(100);
            EXPECT_EQ(0u, t.largestPossibleAllocation());

            // now start freeing
            t.free(blocks[1]);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {200, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 100}}), t.freeBlocks());

            // this will merge with the left free block
            t.free(blocks[2]);
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}, {300, 100}}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 200}}), t.freeBlocks());

            EXPECT_EQ(200u, t.largestPossibleAllocation());
        }

        TEST_CASE("AllocationTrackerTest.expandEmpty", "[AllocationTrackerTest]") {
            AllocationTracker t;

            t.expand(100);
            EXPECT_EQ(100u, t.capacity());
            EXPECT_EQ(100u, t.largestPossibleAllocation());

            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}}), t.freeBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.usedBlocks());

            EXPECT_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.expandWithFreeSpaceAtEnd", "[AllocationTrackerTest]") {
            AllocationTracker t(200);

            AllocationTracker::Block* newBlock = t.allocate(100);
            ASSERT_NE(nullptr, newBlock);
            EXPECT_EQ(0u, newBlock->pos);
            EXPECT_EQ(100u, newBlock->size);

            EXPECT_EQ(100u, t.largestPossibleAllocation());

            t.expand(500);
            EXPECT_EQ(500u, t.capacity());
            EXPECT_EQ(400u, t.largestPossibleAllocation());

            EXPECT_EQ((std::vector<AllocationTracker::Range>{{100, 400}}), t.freeBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 100}}), t.usedBlocks());
        }

        TEST_CASE("AllocationTrackerTest.expandWithUsedSpaceAtEnd", "[AllocationTrackerTest]") {
            AllocationTracker t(200);

            {
                AllocationTracker::Block *newBlock = t.allocate(200);
                ASSERT_NE(nullptr, newBlock);
                EXPECT_EQ(0u, newBlock->pos);
                EXPECT_EQ(0u, t.largestPossibleAllocation());
                EXPECT_EQ(nullptr, t.allocate(1));
            }

            t.expand(500);
            EXPECT_EQ(500u, t.capacity());
            EXPECT_EQ(300u, t.largestPossibleAllocation());

            EXPECT_EQ((std::vector<AllocationTracker::Range>{{200, 300}}), t.freeBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 200}}), t.usedBlocks());

            EXPECT_EQ(nullptr, t.allocate(301));

            {
                AllocationTracker::Block *newBlock2 = t.allocate(300);
                ASSERT_NE(nullptr, newBlock2);
                EXPECT_EQ(200u, newBlock2->pos);
            }
        }

        static constexpr size_t NumBrushes = 64'000;

        // between 12 and 140, inclusive.
        static size_t getBrushSizeFromRandEngine(std::mt19937& engine) {
            return 12 + (4 * (engine() % 33));
        }

        /**
         * This doesn't use std::uniform_int_distribution or
         * std::shuffle because we want the same results on all C++ implementations.
         */
        template <typename T>
        static void shuffle(std::vector<T>& vec, std::mt19937& engine) {
            // https://en.wikipedia.org/wiki/Fisher–Yates_shuffle
            const size_t vecSize = vec.size();
            if (vecSize < 2) {
                return;
            }

            for (size_t i=0; i <= (vecSize - 2); ++i) {
                // pick j to be a random integer in [i, vecSize)
                const size_t rangeExclusive = (vecSize - i);
                // Note, this has modulo bias, but it's good enough for generating test cases
                const size_t j = (engine() % rangeExclusive);

                std::swap(vec[i], vec[j]);
            }

        }

        TEST_CASE("AllocationTrackerTest.testShuffle", "[AllocationTrackerTest]") {
            std::vector<int> ints;
            for (int i = 0; i < 10; ++i) {
                ints.push_back(i);
            }

            std::mt19937 randEngine;
            shuffle(ints, randEngine);

            ASSERT_EQ((std::vector<int>{8, 0, 7, 6, 4, 3, 5, 1, 2, 9}), ints);
        }

        TEST_CASE("AllocationTrackerTest.benchmarkAllocOnly", "[AllocationTrackerTest]") {
            std::mt19937 randEngine;

            AllocationTracker t(140 * NumBrushes);
            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                EXPECT_NE(nullptr, t.allocate(brushSize));
            }
        }

        TEST_CASE("AllocationTrackerTest.benchmarkAllocFreeAlloc", "[AllocationTrackerTest]") {
            std::mt19937 randEngine;

            AllocationTracker t(140 * NumBrushes);

            std::vector<AllocationTracker::Block*> allocations;
            allocations.resize(NumBrushes);

            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                allocations[i] = t.allocate(brushSize);
                EXPECT_NE(nullptr, allocations[i]);
            }

            shuffle(allocations, randEngine);

            for (size_t i = 0; i < NumBrushes; ++i) {
                t.free(allocations[i]);
            }

            EXPECT_EQ((std::vector<AllocationTracker::Range>{}), t.usedBlocks());
            EXPECT_EQ((std::vector<AllocationTracker::Range>{{0, 140 * NumBrushes}}), t.freeBlocks());
            EXPECT_FALSE(t.hasAllocations());

            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                allocations[i] = t.allocate(brushSize);
                EXPECT_NE(nullptr, allocations[i]);
            }
        }

        TEST_CASE("AllocationTrackerTest.benchmarkAllocAndExpand", "[AllocationTrackerTest]") {
            std::mt19937 randEngine;

            AllocationTracker t;
            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                auto key = t.allocate(brushSize);
                if (key == nullptr) {
                    const size_t newSize = t.capacity() + brushSize;
                    t.expand(newSize);

                    //std::cout << "expand to " << newSize << "\n";

                    key = t.allocate(brushSize);
                }
                EXPECT_NE(nullptr, key);
            }
        }
    }
}

