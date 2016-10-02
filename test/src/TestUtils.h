/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef TrenchBroom_TestUtils_h
#define TrenchBroom_TestUtils_h

#include <gtest/gtest.h>

#include "VecMath.h"

namespace TrenchBroom {
    bool texCoordsEqual(const Vec2f& tc1, const Vec2f& tc2);
}

#define ASSERT_VEC_EQ(vec1, vec2) ASSERT_TRUE((vec1).equals((vec2)))
#define EXPECT_VEC_EQ(vec1, vec2) EXPECT_TRUE((vec1).equals((vec2)))
#define ASSERT_VEC_NE(vec1, vec2) ASSERT_FALSE((vec1).equals((vec2)))
#define ASSERT_MAT_EQ(mat1, mat2) ASSERT_TRUE((mat1).equals((mat2)))
#define ASSERT_MAT_NE(mat1, mat2) ASSERT_FALSE((mat1).equals((mat2)))
#define ASSERT_WXSTR_EQ(str1, str2) ASSERT_TRUE((str1).IsSameAs((str2)))

#define ASSERT_TC_EQ(tc1, tc2) ASSERT_TRUE(texCoordsEqual(tc1, tc2));
#define EXPECT_TC_EQ(tc1, tc2) EXPECT_TRUE(texCoordsEqual(tc1, tc2));

#endif
