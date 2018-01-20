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

#include "Polygon.h"

namespace TrenchBroom {
    TEST(PolygonTest, testBackwardCompareEmptyPolygon) {
        Polygon3d p1{};
        ASSERT_EQ(p1.compareUnoriented(Polygon3d{}), 0);
        ASSERT_EQ(p1.compareUnoriented(Polygon3d{Vec3d::Null}), -1);

        Polygon3d p2{Vec3d::Null};
        ASSERT_EQ(p2.compareUnoriented(p1), +1);
        ASSERT_EQ(p2.compareUnoriented(Polygon3d{Vec3d::Null}), 0);
    }

    TEST(PolygonTest, testBackwardComparePolygonWithOneVertex) {
        Polygon3d p2{Vec3d::Null};
        ASSERT_EQ(p2.compareUnoriented(Polygon3d{Vec3d::Null}), 0);
        ASSERT_EQ(p2.compareUnoriented(Polygon3d{Vec3d::Null, Vec3d::Null}), -1);
    }

    TEST(PolygonTest, testBackwardCompare) {
        Polygon3d p1{
                Vec3d(-1.0, -1.0, 0.0),
                Vec3d(+1.0, -1.0, 0.0),
                Vec3d(+1.0, +1.0, 0.0),
                Vec3d(-1.0, +1.0, 0.0),
        };
        Polygon3d p2{
                Vec3d(-1.0, +1.0, 0.0),
                Vec3d(+1.0, +1.0, 0.0),
                Vec3d(+1.0, -1.0, 0.0),
                Vec3d(-1.0, -1.0, 0.0),
        };
        ASSERT_EQ(p1.compareUnoriented(p1), 0);
        ASSERT_EQ(p1.compareUnoriented(p2), 0);
        ASSERT_EQ(p2.compareUnoriented(p1), 0);
        ASSERT_EQ(p2.compareUnoriented(p2), 0);
    }
}
