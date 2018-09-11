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

#include <vecmath/polygon.h>

TEST(PolygonTest, testBackwardCompareEmptyPolygon) {
    vm::polygon3d p1{};
    ASSERT_EQ(vm::compareUnoriented(p1, vm::polygon3d{}), 0);
    ASSERT_EQ(vm::compareUnoriented(p1, vm::polygon3d{vm::vec3d::zero}), -1);

    vm::polygon3d p2{vm::vec3d::zero};
    ASSERT_EQ(vm::compareUnoriented(p2, p1), +1);
    ASSERT_EQ(vm::compareUnoriented(p2, vm::polygon3d{vm::vec3d::zero}), 0);
}

TEST(PolygonTest, testBackwardComparePolygonWithOneVertex) {
    vm::polygon3d p2{vm::vec3d::zero};
    ASSERT_EQ(vm::compareUnoriented(p2, vm::polygon3d{vm::vec3d::zero}), 0);
    ASSERT_EQ(vm::compareUnoriented(p2, vm::polygon3d{vm::vec3d::zero, vm::vec3d::zero}), -1);
}

TEST(PolygonTest, testBackwardCompare) {
    vm::polygon3d p1{
            vm::vec3d(-1.0, -1.0, 0.0),
            vm::vec3d(+1.0, -1.0, 0.0),
            vm::vec3d(+1.0, +1.0, 0.0),
            vm::vec3d(-1.0, +1.0, 0.0),
    };
    vm::polygon3d p2{
            vm::vec3d(-1.0, +1.0, 0.0),
            vm::vec3d(+1.0, +1.0, 0.0),
            vm::vec3d(+1.0, -1.0, 0.0),
            vm::vec3d(-1.0, -1.0, 0.0),
    };
    ASSERT_EQ(vm::compareUnoriented(p1, p1), 0);
    ASSERT_EQ(vm::compareUnoriented(p1, p2), 0);
    ASSERT_EQ(vm::compareUnoriented(p2, p1), 0);
    ASSERT_EQ(vm::compareUnoriented(p2, p2), 0);
}
