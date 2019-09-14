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
#include <vecmath/vec.h>
#include <vecmath/convex_hull.h>

#include "test_utils.h"

namespace vm {
    TEST(ConvexHullTest, convexHull2dSimple) {
        const vm::vec3d p1(0.0, 0.0, 0.0);
        const vm::vec3d p2(8.0, 8.0, 0.0);
        const vm::vec3d p3(8.0, 0.0, 0.0);
        const vm::vec3d p4(0.0, 8.0, 0.0);

        std::vector<vm::vec3d> points;
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);
        points.push_back(p4);

        const std::vector<vm::vec3d> hull = vm::convexHull2D<double>(points);
        ASSERT_EQ(4u, hull.size());
        ASSERT_VEC_EQ(p3, hull[0]);
        ASSERT_VEC_EQ(p2, hull[1]);
        ASSERT_VEC_EQ(p4, hull[2]);
        ASSERT_VEC_EQ(p1, hull[3]);
    }

    TEST(ConvexHullTest, convexHull2dSimpleWithInternalPoint) {
        const vm::vec3d p1(0.0, 0.0, 0.0);
        const vm::vec3d p2(8.0, 8.0, 0.0);
        const vm::vec3d p3(8.0, 0.0, 0.0);
        const vm::vec3d p4(0.0, 8.0, 0.0);
        const vm::vec3d p5(4.0, 4.0, 0.0);

        std::vector<vm::vec3d> points;
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);
        points.push_back(p4);
        points.push_back(p5);

        const std::vector<vm::vec3d> hull = vm::convexHull2D<double>(points);
        ASSERT_EQ(4u, hull.size());
        ASSERT_VEC_EQ(p3, hull[0]);
        ASSERT_VEC_EQ(p2, hull[1]);
        ASSERT_VEC_EQ(p4, hull[2]);
        ASSERT_VEC_EQ(p1, hull[3]);
    }

    TEST(ConvexHullTest, convexHull2dSimpleWithPointOnLine) {
        const vm::vec3d p1(0.0, 0.0, 0.0);
        const vm::vec3d p2(8.0, 8.0, 0.0);
        const vm::vec3d p3(8.0, 0.0, 0.0);
        const vm::vec3d p4(0.0, 8.0, 0.0);
        const vm::vec3d p5(4.0, 0.0, 0.0);

        std::vector<vm::vec3d> points;
        points.push_back(p1);
        points.push_back(p2);
        points.push_back(p3);
        points.push_back(p4);
        points.push_back(p5);

        const std::vector<vm::vec3d> hull = vm::convexHull2D<double>(points);
        ASSERT_EQ(4u, hull.size());
        ASSERT_VEC_EQ(p3, hull[0]);
        ASSERT_VEC_EQ(p2, hull[1]);
        ASSERT_VEC_EQ(p4, hull[2]);
        ASSERT_VEC_EQ(p1, hull[3]);
    }
}
