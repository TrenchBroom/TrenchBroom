/*
 Copyright (C) 2010-2019 Kristian Duske

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

#include "StepIterator.h"

#include <vector>

TEST(StepIteratorTest, emptySequence) {
    std::vector<size_t> vec;
    
    ASSERT_EQ(std::begin(vec), stepIterator(std::begin(vec), std::end(vec)));
    ASSERT_EQ(std::end(vec), stepIterator(std::begin(vec), std::end(vec)));
    ASSERT_EQ(std::begin(vec), stepIterator(std::begin(vec), std::end(vec), 1));
    ASSERT_EQ(std::end(vec), stepIterator(std::begin(vec), std::end(vec), 1));
}

TEST(StepIteratorTest, zeroStride) {
    std::vector<size_t> vec({ 1 });

    ASSERT_EQ(std::begin(vec), stepIterator(std::begin(vec), std::end(vec), 0, 0));
    ASSERT_EQ(std::begin(vec), std::next(stepIterator(std::begin(vec), std::end(vec), 0, 0)));
}

TEST(StepIteratorTest, oneElementSequence) {
    std::vector<size_t> vec({ 1 });

    ASSERT_EQ(std::begin(vec), stepIterator(std::begin(vec), std::end(vec)));
    ASSERT_EQ(std::end(vec), stepIterator(std::begin(vec), std::end(vec), 1));
    ASSERT_EQ(std::end(vec), stepIterator(std::begin(vec), std::end(vec), 2));

    ASSERT_EQ(std::begin(vec), stepIterator(std::begin(vec), std::end(vec), 0, 2));
    ASSERT_EQ(std::end(vec), stepIterator(std::begin(vec), std::end(vec), 1, 2));
    ASSERT_EQ(std::end(vec), stepIterator(std::begin(vec), std::end(vec), 2, 2));

    ASSERT_EQ(std::end(vec), std::next(stepIterator(std::begin(vec), std::end(vec), 0)));
    ASSERT_EQ(std::end(vec), std::next(stepIterator(std::begin(vec), std::end(vec), 1)));
    ASSERT_EQ(std::end(vec), std::next(stepIterator(std::begin(vec), std::end(vec), 2)));

    ASSERT_EQ(std::end(vec), std::next(stepIterator(std::begin(vec), std::end(vec), 0, 2)));
    ASSERT_EQ(std::end(vec), std::next(stepIterator(std::begin(vec), std::end(vec), 1, 2)));
    ASSERT_EQ(std::end(vec), std::next(stepIterator(std::begin(vec), std::end(vec), 2, 2)));
}
