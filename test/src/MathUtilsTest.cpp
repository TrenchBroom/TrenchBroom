/*
Copyright (C) 2010-2017 Kristian Duske

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

#include "MathUtils.h"

#include <cstdint>

TEST(MathUtilsTest, findHighestOrderBit) {
    ASSERT_EQ(sizeof(unsigned)*8, Math::findHighestOrderBit(0u));
    ASSERT_EQ(sizeof(unsigned)*8, Math::findHighestOrderBit(0u, 2));
    
    for (uint64_t i = 0; i < sizeof(uint64_t)*8; ++i) {
        ASSERT_EQ(i, Math::findHighestOrderBit(static_cast<uint64_t>(1u) << i));
    }
    
    for (uint64_t i = 0; i < sizeof(uint64_t)*8; ++i) {
        const uint64_t test = static_cast<uint64_t>(1u) | (static_cast<uint64_t>(1u) << i);
        ASSERT_EQ(i, Math::findHighestOrderBit(test));
    }
    
    for (uint64_t i = 0; i < sizeof(uint64_t)*8-1; ++i) {
        const uint64_t test = (static_cast<uint64_t>(1u) << 63) | (static_cast<uint64_t>(1u) << i);
        ASSERT_EQ(63u, Math::findHighestOrderBit(test));
    }

    for (uint64_t i = 0; i < sizeof(uint64_t)*8; ++i) {
        for (uint64_t j = 0; j < sizeof(uint64_t) * 8; ++j) {
            const uint64_t exp = i <= j ? i : sizeof(uint64_t)*8;
            ASSERT_EQ(exp, Math::findHighestOrderBit(static_cast<uint64_t>(1u) << i, j));
        }
    }
}
