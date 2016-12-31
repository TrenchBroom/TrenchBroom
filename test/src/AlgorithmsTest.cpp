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

#include <gtest/gtest.h>

#include "Algorithms.h"
#include "VecMath.h"

bool containsPoint(const Vec3d::List& vertices, const Vec3d& point);

Vec3d::List square();
Vec3d::List triangle();

TEST(AlgorithmsTest, squareContainsPointInCenter) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(0.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsLeftTopVertex) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(-1.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsRightTopVertex) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(+1.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsRightBottomVertex) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(+1.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsLeftBottomVertex) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(-1.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfLeftEdge) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(-1.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfTopEdge) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(0.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfRightEdge) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(+1.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfBottomEdge) {
    ASSERT_TRUE(containsPoint(square(), Vec3d(0.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsOrigin) {
    ASSERT_TRUE(containsPoint(triangle(), Vec3d(0.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsTopPoint) {
    ASSERT_TRUE(containsPoint(triangle(), Vec3d(-1.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsLeftBottomPoint) {
    ASSERT_TRUE(containsPoint(triangle(), Vec3d(-1.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsRightBottomPoint) {
    ASSERT_TRUE(containsPoint(triangle(), Vec3d(+1.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsCenterOfTopToLeftBottomEdge) {
    ASSERT_TRUE(containsPoint(triangle(), (triangle()[0] + triangle()[1]) / 2.0));
}

TEST(AlgorithmsTest, triangleContainsCenterOfLeftBottomToRightBottomEdge) {
    ASSERT_TRUE(containsPoint(triangle(), (triangle()[1] + triangle()[2]) / 2.0));
}

TEST(AlgorithmsTest, triangleContainsCenterOfRightBottomToTopEdge) {
    ASSERT_TRUE(containsPoint(triangle(), (triangle()[2] + triangle()[0]) / 2.0));
}

TEST(AlgorithmsTest, triangleContainsOuterPoint) {
    ASSERT_FALSE(containsPoint(triangle(), Vec3d(+1.0, +1.0, 0.0)));
}

bool containsPoint(const Vec3d::List& vertices, const Vec3d& point) {
    return polygonContainsPoint(point, std::begin(vertices), std::end(vertices),
                                [](const Vec3d& vertex) { return vertex; });
}

Vec3d::List square() {
    return Vec3d::List {
        Vec3d(-1.0, -1.0, 0.0),
        Vec3d(-1.0, +1.0, 0.0),
        Vec3d(+1.0, +1.0, 0.0),
        Vec3d(+1.0, -1.0, 0.0)
    };
}

Vec3d::List triangle() {
    return Vec3d::List {
        Vec3d(-1.0, +1.0, 0.0), // top
        Vec3d(-1.0, -1.0, 0.0), // left bottom
        Vec3d(+1.0, -1.0, 0.0), // right bottom
    };
}
