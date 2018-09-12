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

#include <vecmath/forward.h>
#include <vecmath/polygon.h>

namespace vm {
    TEST(PolygonTest, testBackwardCompareEmptyPolygon) {
        polygon3d p1{};
        ASSERT_EQ(compareUnoriented(p1, polygon3d{}), 0);
        ASSERT_EQ(compareUnoriented(p1, polygon3d{vec3d::zero}), -1);
    
        polygon3d p2{vec3d::zero};
        ASSERT_EQ(compareUnoriented(p2, p1), +1);
        ASSERT_EQ(compareUnoriented(p2, polygon3d{vec3d::zero}), 0);
    }
    
    TEST(PolygonTest, testBackwardComparePolygonWithOneVertex) {
        polygon3d p2{vec3d::zero};
        ASSERT_EQ(compareUnoriented(p2, polygon3d{vec3d::zero}), 0);
        ASSERT_EQ(compareUnoriented(p2, polygon3d{vec3d::zero, vec3d::zero}), -1);
    }
    
    TEST(PolygonTest, testBackwardCompare) {
        polygon3d p1{
                vec3d(-1.0, -1.0, 0.0),
                vec3d(+1.0, -1.0, 0.0),
                vec3d(+1.0, +1.0, 0.0),
                vec3d(-1.0, +1.0, 0.0),
        };
        polygon3d p2{
                vec3d(-1.0, +1.0, 0.0),
                vec3d(+1.0, +1.0, 0.0),
                vec3d(+1.0, -1.0, 0.0),
                vec3d(-1.0, -1.0, 0.0),
        };
        ASSERT_EQ(compareUnoriented(p1, p1), 0);
        ASSERT_EQ(compareUnoriented(p1, p2), 0);
        ASSERT_EQ(compareUnoriented(p2, p1), 0);
        ASSERT_EQ(compareUnoriented(p2, p2), 0);
    }
}
