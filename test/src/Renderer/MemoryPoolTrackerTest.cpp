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

#include "Renderer/MemoryPoolTracker.h"

namespace TrenchBroom {
    namespace Renderer {
        TEST(MemoryPoolTrackerTest, constructor) {
            MemoryPoolTracker t(100);
            EXPECT_EQ(100, t.capacity());
        }
        
        TEST(MemoryPoolTrackerTest, emptyConstructor) {
            MemoryPoolTracker t;
            EXPECT_EQ(0, t.capacity());
            EXPECT_EQ(nullptr, t.allocate());
        }

        TEST(MemoryPoolTrackerTest, constructWithZeroCapacity) {
            MemoryPoolTracker t(0);
            EXPECT_EQ(0, t.capacity());
            EXPECT_EQ(nullptr, t.allocate());
        }

        TEST(MemoryPoolTrackerTest, fiveAllocations) {
            MemoryPoolTracker t(5);

            // allocate all the memory
            MemoryPoolTracker::Block* allocations[5];
            for (int i=0; i<5; ++i) {
                allocations[i] = t.allocate();
                EXPECT_NE(nullptr, allocations[i]);
            }

            EXPECT_EQ(nullptr, t.allocate());

            // now start freeing
            t.free(allocations[1]);
            t.free(allocations[3]);
            t.free(allocations[2]);

            // 0 and 4 still used, 3 free blocks
            for (int i=1; i<=3; ++i) {
                allocations[i] = t.allocate();
                EXPECT_NE(nullptr, allocations[i]);
            }

            EXPECT_EQ(nullptr, t.allocate());

            // avoid leaking memory
            for (int i=0; i<5; ++i) {
                t.free(allocations[i]);
            }
        }
        
        TEST(MemoryPoolTrackerTest, expandEmpty) {
            MemoryPoolTracker t;
            
            t.expand(100);
            EXPECT_EQ(100, t.capacity());
        }
        
        TEST(MemoryPoolTrackerTest, expandWithFreeSpaceAtEnd) {
            MemoryPoolTracker t(2);
            MemoryPoolTracker::Block* a = t.allocate();
            
            t.expand(5);
            EXPECT_EQ(5, t.capacity());

            t.free(a);
        }

        static constexpr size_t NumBrushes = 64'000;

        TEST(MemoryPoolTrackerTest, benchmark) {
            MemoryPoolTracker t(NumBrushes);
            MemoryPoolTracker::Block** allocations = new MemoryPoolTracker::Block*[NumBrushes];

            for (size_t i = 0; i < NumBrushes; ++i) {
                allocations[i] = t.allocate();
                EXPECT_NE(nullptr, allocations[i]);
            }

            for (size_t i = 0; i < NumBrushes; ++i) {
                t.free(allocations[i]);
            }

            delete[] allocations;
        }
    }
}

