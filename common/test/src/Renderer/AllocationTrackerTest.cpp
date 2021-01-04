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

#include "Renderer/AllocationTracker.h"

#include <algorithm>
#include <random>
#include <vector>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST_CASE("AllocationTrackerTest.constructor", "[AllocationTrackerTest]") {
            AllocationTracker t(100);
            CHECK(t.capacity() == 100u);
            CHECK(t.largestPossibleAllocation() == 100u);
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}}));
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{}));
            CHECK_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.emptyConstructor", "[AllocationTrackerTest]") {
            AllocationTracker t;
            CHECK(t.capacity() == 0u);
            CHECK(t.largestPossibleAllocation() == 0u);
            CHECK(t.allocate(1) == nullptr);
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{}));
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{}));
            CHECK_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.constructWithZeroCapacity", "[AllocationTrackerTest]") {
            AllocationTracker t(0);
            CHECK(t.capacity() == 0u);
            CHECK(t.largestPossibleAllocation() == 0u);
            CHECK(t.allocate(1) == nullptr);
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{}));
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{}));
            CHECK_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.invalidAllocate", "[AllocationTrackerTest]") {
            AllocationTracker t(100);

            CHECK_THROWS(t.allocate(0));

            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}}));
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{}));
            CHECK_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.fiveAllocations", "[AllocationTrackerTest]") {
            AllocationTracker t(500);

            // allocate all the memory
            AllocationTracker::Block* blocks[5];

            blocks[0] = t.allocate(100);
            REQUIRE(blocks[0] != nullptr);
            CHECK(blocks[0]->pos == 0u);
            CHECK(blocks[0]->size == 100u);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 400}}));
            CHECK(t.hasAllocations());

            blocks[1] = t.allocate(100);
            REQUIRE(blocks[1] != nullptr);
            CHECK(blocks[1]->pos == 100u);
            CHECK(blocks[1]->size == 100u);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{200, 300}}));

            blocks[2] = t.allocate(100);
            REQUIRE(blocks[2] != nullptr);
            CHECK(blocks[2]->pos == 200u);
            CHECK(blocks[2]->size == 100u);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{300, 200}}));

            blocks[3] = t.allocate(100);
            REQUIRE(blocks[3] != nullptr);
            CHECK(blocks[3]->pos == 300u);
            CHECK(blocks[3]->size == 100u);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}, {300, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{400, 100}}));

            blocks[4] = t.allocate(100);
            REQUIRE(blocks[4] != nullptr);
            CHECK(blocks[4]->pos == 400u);
            CHECK(blocks[4]->size == 100u);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {200, 100}, {300, 100}, {400, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{}));

            // further allocations throw
            CHECK(t.allocate(1) == nullptr);

            // now start freeing
            t.free(blocks[1]);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {200, 100}, {300, 100}, {400, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 100}}));

            t.free(blocks[3]);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {200, 100}, {400, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 100}, {300, 100}}));
            CHECK(t.largestPossibleAllocation() == 100u);

            // this will cause a merge with the left and right free blocks
            t.free(blocks[2]);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {400, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 300}}));
            CHECK(t.largestPossibleAllocation() == 300u);

            // allocate the free block of 300 in the middle
            CHECK(t.allocate(301) == nullptr);
            AllocationTracker::Block* newBlock = t.allocate(300);
            REQUIRE(newBlock != nullptr);
            CHECK(newBlock->pos == 100u);
            CHECK(newBlock->size == 300u);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {100, 300}, {400, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{}));
        }

        TEST_CASE("AllocationTrackerTest.freeMergeRight", "[AllocationTrackerTest]") {
            AllocationTracker t(400);

            // allocate all the memory
            AllocationTracker::Block* blocks[4];

            blocks[0] = t.allocate(100);
            blocks[1] = t.allocate(100);
            blocks[2] = t.allocate(100);
            blocks[3] = t.allocate(100);
            CHECK(t.largestPossibleAllocation() == 0u);

            // now start freeing
            t.free(blocks[2]);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {100, 100}, {300, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{200, 100}}));

            // this will merge with the right free block
            t.free(blocks[1]);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {300, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 200}}));

            CHECK(t.largestPossibleAllocation() == 200u);
        }

        TEST_CASE("AllocationTrackerTest.freeMergeLeft", "[AllocationTrackerTest]") {
            AllocationTracker t(400);

            // allocate all the memory
            AllocationTracker::Block* blocks[4];

            blocks[0] = t.allocate(100);
            blocks[1] = t.allocate(100);
            blocks[2] = t.allocate(100);
            blocks[3] = t.allocate(100);
            CHECK(t.largestPossibleAllocation() == 0u);

            // now start freeing
            t.free(blocks[1]);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {200, 100}, {300, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 100}}));

            // this will merge with the left free block
            t.free(blocks[2]);
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}, {300, 100}}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 200}}));

            CHECK(t.largestPossibleAllocation() == 200u);
        }

        TEST_CASE("AllocationTrackerTest.expandEmpty", "[AllocationTrackerTest]") {
            AllocationTracker t;

            t.expand(100);
            CHECK(t.capacity() == 100u);
            CHECK(t.largestPossibleAllocation() == 100u);

            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}}));
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{}));

            CHECK_FALSE(t.hasAllocations());
        }

        TEST_CASE("AllocationTrackerTest.expandWithFreeSpaceAtEnd", "[AllocationTrackerTest]") {
            AllocationTracker t(200);

            AllocationTracker::Block* newBlock = t.allocate(100);
            REQUIRE(newBlock != nullptr);
            CHECK(newBlock->pos == 0u);
            CHECK(newBlock->size == 100u);

            CHECK(t.largestPossibleAllocation() == 100u);

            t.expand(500);
            CHECK(t.capacity() == 500u);
            CHECK(t.largestPossibleAllocation() == 400u);

            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{100, 400}}));
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 100}}));
        }

        TEST_CASE("AllocationTrackerTest.expandWithUsedSpaceAtEnd", "[AllocationTrackerTest]") {
            AllocationTracker t(200);

            {
                AllocationTracker::Block *newBlock = t.allocate(200);
                REQUIRE(newBlock != nullptr);
                CHECK(newBlock->pos == 0u);
                CHECK(t.largestPossibleAllocation() == 0u);
                CHECK(t.allocate(1) == nullptr);
            }

            t.expand(500);
            CHECK(t.capacity() == 500u);
            CHECK(t.largestPossibleAllocation() == 300u);

            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{200, 300}}));
            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{{0, 200}}));

            CHECK(t.allocate(301) == nullptr);

            {
                AllocationTracker::Block *newBlock2 = t.allocate(300);
                REQUIRE(newBlock2 != nullptr);
                CHECK(newBlock2->pos == 200u);
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

            CHECK(ints == (std::vector<int>{8, 0, 7, 6, 4, 3, 5, 1, 2, 9}));
        }

        TEST_CASE("AllocationTrackerTest.benchmarkAllocOnly", "[AllocationTrackerTest]") {
            std::mt19937 randEngine;

            AllocationTracker t(140 * NumBrushes);
            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                CHECK(t.allocate(brushSize) != nullptr);
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
                CHECK(allocations[i] != nullptr);
            }

            shuffle(allocations, randEngine);

            for (size_t i = 0; i < NumBrushes; ++i) {
                t.free(allocations[i]);
            }

            CHECK(t.usedBlocks() == (std::vector<AllocationTracker::Range>{}));
            CHECK(t.freeBlocks() == (std::vector<AllocationTracker::Range>{{0, 140 * NumBrushes}}));
            CHECK_FALSE(t.hasAllocations());

            for (size_t i = 0; i < NumBrushes; ++i) {
                const size_t brushSize = getBrushSizeFromRandEngine(randEngine);

                allocations[i] = t.allocate(brushSize);
                CHECK(allocations[i] != nullptr);
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
                CHECK(key != nullptr);
            }
        }
    }
}

