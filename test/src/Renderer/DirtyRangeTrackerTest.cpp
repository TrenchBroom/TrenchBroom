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

#include "Renderer/DirtyRangeTracker.h"

namespace TrenchBroom {
    namespace Renderer {
        using Range = DirtyRangeTracker::Range;

        static std::vector<Range> getDirtyRanges(const DirtyRangeTracker& tracker) {
            std::vector<Range> result;

            tracker.visitRanges([&](const Range& range) {
                result.push_back(range);
            });

            return result;
        }

        TEST(DirtyRangeTrackerTest, constructor) {
            DirtyRangeTracker t(100);
            EXPECT_EQ(100, t.capacity());
            EXPECT_EQ((std::vector<Range>{}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, emptyConstructor) {
            DirtyRangeTracker t;
            EXPECT_EQ(0, t.capacity());
            EXPECT_EQ((std::vector<Range>{}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, invalidExpand) {
            DirtyRangeTracker t(100);

            EXPECT_ANY_THROW(t.expand(0));
            EXPECT_ANY_THROW(t.expand(99));
            EXPECT_ANY_THROW(t.expand(100));
        }

        TEST(DirtyRangeTrackerTest, expandEmpty) {
            DirtyRangeTracker t(100);
            // expanding marks the new range as dirty
            t.expand(150);
            EXPECT_EQ((std::vector<Range>{{100,50}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, expandDirty) {
            DirtyRangeTracker t(100);
            t.markDirty(0, 100);
            t.expand(150);
            EXPECT_EQ((std::vector<Range>{{0,150}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, markPastEnd) {
            DirtyRangeTracker t(100);
            EXPECT_ANY_THROW(t.markDirty(0, 101));
        }

        TEST(DirtyRangeTrackerTest, mergeOverlappingStart) {
            DirtyRangeTracker t(100);
            t.markDirty(0, 10);
            t.markDirty(5, 10);
            EXPECT_EQ((std::vector<Range>{{0,15}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, mergeOverlappingEnd) {
            DirtyRangeTracker t(100);
            t.markDirty(10, 10);
            t.markDirty(5, 10);
            EXPECT_EQ((std::vector<Range>{{5,15}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, mergeTouchingStart) {
            DirtyRangeTracker t(100);
            t.markDirty(10, 10);
            t.markDirty(20, 10);
            EXPECT_EQ((std::vector<Range>{{10,20}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, mergeTouchingEnd) {
            DirtyRangeTracker t(100);
            t.markDirty(10, 10);
            t.markDirty(0, 10);
            EXPECT_EQ((std::vector<Range>{{0,20}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, markZeroRange) {
            DirtyRangeTracker t(100);
            t.markDirty(0, 0);
            EXPECT_EQ((std::vector<Range>{}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, mergeEqual) {
            DirtyRangeTracker t(100);
            t.markDirty(0, 50);
            t.markDirty(0, 50);
            EXPECT_EQ((std::vector<Range>{{0, 50}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, mergeSubset) {
            DirtyRangeTracker t(100);
            t.markDirty(0, 50);
            t.markDirty(10, 30);
            EXPECT_EQ((std::vector<Range>{{0, 50}}), getDirtyRanges(t));
        }

        TEST(DirtyRangeTrackerTest, markDisjoint) {
            DirtyRangeTracker t(100);
            t.markDirty(0, 10);
            t.markDirty(20, 10);
            t.markDirty(40, 10);
            EXPECT_EQ((std::vector<Range>{{0, 10}, {20, 10}, {40, 10}}), getDirtyRanges(t));
        }
    }
}

