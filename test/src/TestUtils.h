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
#include "Model/ModelTypes.h"
#include "IO/Path.h"

#include <vecmath/vec.h>
#include <vecmath/mat.h>

#include <cctype>

namespace TrenchBroom {
    bool texCoordsEqual(const vm::vec2f& tc1, const vm::vec2f& tc2);
    bool pointExactlyIntegral(const vm::vec3d &point);

    namespace Model {
        void assertTexture(const String& expected, const Brush* brush, const vm::vec3d& faceNormal);
        
        void assertTexture(const String& expected, const Brush* brush, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3);
        void assertTexture(const String& expected, const Brush* brush, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3, const vm::vec3d& v4);
        void assertTexture(const String& expected, const Brush* brush, const std::vector<vm::vec3d>& vertices);
        void assertTexture(const String& expected, const Brush* brush, const vm::polygon3d& vertices);
    }
}

std::string sanitizeTestName(const std::string& name);

template <typename T, size_t S>
void ASSERT_VEC_EQ(const vm::vec<T,S>& lhs, const vm::vec<T,S>& rhs) {
    ASSERT_TRUE(isEqual(lhs, rhs, static_cast<T>(0.001)));
}

template <typename T, size_t S>
void EXPECT_VEC_EQ(const vm::vec<T,S>& lhs, const vm::vec<T,S>& rhs) {
    EXPECT_TRUE(isEqual(lhs, rhs, static_cast<T>(0.001)));
}

template <typename T, size_t S>
void ASSERT_VEC_NE(const vm::vec<T,S>& lhs, const vm::vec<T,S>& rhs) {
    ASSERT_FALSE(isEqual(lhs, rhs, static_cast<T>(0.001)));
}

template <typename T, size_t C, size_t R>
void ASSERT_MAT_EQ(const vm::mat<T,R,C>& lhs, const vm::mat<T,R,C>& rhs) {
    ASSERT_TRUE(isEqual(lhs, rhs, static_cast<T>(0.001)));
}

template <typename T, size_t C, size_t R>
void ASSERT_MAT_NE(const vm::mat<T,R,C>& lhs, const vm::mat<T,R,C>& rhs) {
    ASSERT_FALSE(isEqual(lhs, rhs, static_cast<T>(0.001)));
}

#define ASSERT_WXSTR_EQ(str1, str2) ASSERT_TRUE((str1).IsSameAs((str2)))

#define ASSERT_TC_EQ(tc1, tc2) ASSERT_TRUE(texCoordsEqual(tc1, tc2));
#define EXPECT_TC_EQ(tc1, tc2) EXPECT_TRUE(texCoordsEqual(tc1, tc2));

#define ASSERT_POINT_INTEGRAL(vec) ASSERT_TRUE(pointExactlyIntegral(vec))

#endif
