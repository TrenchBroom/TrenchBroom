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

#ifndef TRENCHBROOM_TEST_UTILS_H
#define TRENCHBROOM_TEST_UTILS_H

#include <vecmath/forward.h>

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

#endif //TRENCHBROOM_TEST_UTILS_H
