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

#ifndef TrenchBroom_TestUtils_h
#define TrenchBroom_TestUtils_h

#include <gtest/gtest.h>

#include "StringUtils.h"
#include "VecMath.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    bool texCoordsEqual(const Vec2f& tc1, const Vec2f& tc2);
    bool pointExactlyIntegral(const Vec3d &point);

    namespace Model {
        void assertTexture(const String& expected, const Brush* brush, const Vec3d& faceNormal);
        
        void assertTexture(const String& expected, const Brush* brush, const Vec3d& v1, const Vec3d& v2, const Vec3d& v3);
        void assertTexture(const String& expected, const Brush* brush, const Vec3d& v1, const Vec3d& v2, const Vec3d& v3, const Vec3d& v4);
        void assertTexture(const String& expected, const Brush* brush, const Vec3d::List& vertices);
        void assertTexture(const String& expected, const Brush* brush, const Polygon3d& vertices);
    }
}

template <typename T, size_t S>
void ASSERT_VEC_EQ(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    ASSERT_TRUE(equal(lhs, rhs, static_cast<T>(0.001)));
}

template <typename T, size_t S>
void EXPECT_VEC_EQ(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    EXPECT_TRUE(equal(lhs, rhs, static_cast<T>(0.001)));
}

template <typename T, size_t S>
void ASSERT_VEC_NE(const Vec<T,S>& lhs, const Vec<T,S>& rhs) {
    ASSERT_FALSE(equal(lhs, rhs, static_cast<T>(0.001)));
}

#define ASSERT_MAT_EQ(mat1, mat2) ASSERT_TRUE((mat1).equals((mat2)))
#define ASSERT_MAT_NE(mat1, mat2) ASSERT_FALSE((mat1).equals((mat2)))
#define ASSERT_WXSTR_EQ(str1, str2) ASSERT_TRUE((str1).IsSameAs((str2)))

#define ASSERT_TC_EQ(tc1, tc2) ASSERT_TRUE(texCoordsEqual(tc1, tc2));
#define EXPECT_TC_EQ(tc1, tc2) EXPECT_TRUE(texCoordsEqual(tc1, tc2));

#define ASSERT_POINT_INTEGRAL(vec) ASSERT_TRUE(pointExactlyIntegral(vec))

#endif
