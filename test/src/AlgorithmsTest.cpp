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

#include "Algorithms.h"
#include "VecMath.h"

bool containsPoint(const vec3d::List& vertices, const vec3d& point);

vec3d::List square();
vec3d::List triangle();

TEST(AlgorithmsTest, squareContainsPointInCenter) {
    ASSERT_TRUE(containsPoint(square(), vec3d(0.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsLeftTopVertex) {
    ASSERT_TRUE(containsPoint(square(), vec3d(-1.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsRightTopVertex) {
    ASSERT_TRUE(containsPoint(square(), vec3d(+1.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsRightBottomVertex) {
    ASSERT_TRUE(containsPoint(square(), vec3d(+1.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsLeftBottomVertex) {
    ASSERT_TRUE(containsPoint(square(), vec3d(-1.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfLeftEdge) {
    ASSERT_TRUE(containsPoint(square(), vec3d(-1.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfTopEdge) {
    ASSERT_TRUE(containsPoint(square(), vec3d(0.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfRightEdge) {
    ASSERT_TRUE(containsPoint(square(), vec3d(+1.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, squareContainsCenterOfBottomEdge) {
    ASSERT_TRUE(containsPoint(square(), vec3d(0.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsOrigin) {
    ASSERT_TRUE(containsPoint(triangle(), vec3d(0.0, 0.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsTopPoint) {
    ASSERT_TRUE(containsPoint(triangle(), vec3d(-1.0, +1.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsLeftBottomPoint) {
    ASSERT_TRUE(containsPoint(triangle(), vec3d(-1.0, -1.0, 0.0)));
}

TEST(AlgorithmsTest, triangleContainsRightBottomPoint) {
    ASSERT_TRUE(containsPoint(triangle(), vec3d(+1.0, -1.0, 0.0)));
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
    ASSERT_FALSE(containsPoint(triangle(), vec3d(+1.0, +1.0, 0.0)));
}

bool containsPoint(const vec3d::List& vertices, const vec3d& point) {
    return polygonContainsPoint(point, std::begin(vertices), std::end(vertices),
                                [](const vec3d& vertex) { return vertex; });
}

vec3d::List square() {
    return vec3d::List {
        vec3d(-1.0, -1.0, 0.0),
        vec3d(-1.0, +1.0, 0.0),
        vec3d(+1.0, +1.0, 0.0),
        vec3d(+1.0, -1.0, 0.0)
    };
}

vec3d::List triangle() {
    return vec3d::List {
        vec3d(-1.0, +1.0, 0.0), // top
        vec3d(-1.0, -1.0, 0.0), // left bottom
        vec3d(+1.0, -1.0, 0.0), // right bottom
    };
}
