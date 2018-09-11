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

#include <vecmath/vec.h>
#include <vecmath/plane.h>
#include <vecmath/utils.h>
#include "TestUtils.h"

#include <array>

TEST(PlaneTest, constructDefault) {
    const vm::plane3f p;
    ASSERT_EQ(0.0f, p.distance);
    ASSERT_EQ(vm::vec3f::zero, p.normal);
}

TEST(PlaneTest, constructWithDistanceAndNormal) {
    const float d = 123.0f;
    const vm::vec3f n = normalize(vm::vec3f(1.0f, 2.0f, 3.0f));
    const vm::plane3f p(d, n);
    ASSERT_FLOAT_EQ(d, p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, constructWithAnchorAndNormal) {
    const vm::vec3f a = vm::vec3f(-2038.034f, 0.0023f, 32.0f);
    const vm::vec3f n = normalize(vm::vec3f(9.734f, -3.393f, 2.033f));
    const vm::plane3f p(a, n);
    ASSERT_FLOAT_EQ(dot(a, n), p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, constructPlaneContainingVector) {
}

TEST(PlaneTest, anchor) {
    const vm::vec3f a = vm::vec3f(-2038.034f, 0.0023f, 32.0f);
    const vm::vec3f n = normalize(vm::vec3f(9.734f, -3.393f, 2.033f));
    const vm::plane3f p(a, n);
    ASSERT_VEC_EQ(p.distance * n, p.anchor());
}

TEST(PlaneTest, intersectWithRay) {
}


TEST(PlaneTest, pointStatus) {
    const vm::plane3f p(10.0f, vm::vec3f::pos_z);
    ASSERT_EQ(vm::point_status::above, p.pointStatus(vm::vec3f(0.0f, 0.0f, 11.0f)));
    ASSERT_EQ(vm::point_status::below, p.pointStatus(vm::vec3f(0.0f, 0.0f, 9.0f)));
    ASSERT_EQ(vm::point_status::inside, p.pointStatus(vm::vec3f(0.0f, 0.0f, 10.0f)));
}

TEST(PlaneTest, pointDistance) {
    const vm::vec3f a = vm::vec3f(-2038.034f, 0.0023f, 32.0f);
    const vm::vec3f n = normalize(vm::vec3f(9.734f, -3.393f, 2.033f));
    const vm::plane3f p(a, n);
    const vm::vec3f point(1.0f, -32.37873f, 32.0f);
    ASSERT_EQ(dot(point, p.normal) - p.distance, p.pointDistance(point));
}

TEST(PlaneTest, valueAtParallelPlanes) {
    const vm::plane3f p1(10.0f, vm::vec3f::pos_x);
    
    ASSERT_FLOAT_EQ(p1.distance, p1.at(vm::vec2f(2.0f, 1.0f), vm::axis::x));
    ASSERT_FLOAT_EQ(p1.distance, p1.at(vm::vec2f(22.0f, -34322.0232f), vm::axis::x));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vm::vec2f(2.0f, 1.0f), vm::axis::y));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vm::vec2f(22.0f, -34322.0232f), vm::axis::y));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vm::vec2f(2.0f, 1.0f), vm::axis::z));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vm::vec2f(22.0f, -34322.0232f), vm::axis::z));
}

TEST(PlaneTest, valueAt) {
    const vm::vec3f a = vm::vec3f(-2038.034f, 0.0023f, 32.0f);
    const vm::vec3f n = normalize(vm::vec3f(9.734f, -3.393f, 2.033f));
    const vm::plane3f p(a, n);
    const vm::vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.y() - point1.y() * p.normal.z()) / p.normal[vm::axis::x],
                    p.at(point1, vm::axis::x));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.z()) / p.normal[vm::axis::y],
                    p.at(point1, vm::axis::y));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.y()) / p.normal[vm::axis::z],
                    p.at(point1, vm::axis::z));
}

TEST(PlaneTest, xYZValueAt) {
    const vm::vec3f a = vm::vec3f(-2038.034f, 0.0023f, 32.0f);
    const vm::vec3f n = normalize(vm::vec3f(9.734f, -3.393f, 2.033f));
    const vm::plane3f p(a, n);
    const vm::vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ(p.at(point1, vm::axis::x), p.xAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, vm::axis::y), p.yAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, vm::axis::z), p.zAt(point1));
}

TEST(PlaneTest, isEqual) {
    ASSERT_TRUE(isEqual(vm::plane3f(0.0f, vm::vec3f::pos_x), vm::plane3f(0.0f, vm::vec3f::pos_x), vm::constants<float>::almostZero()));
    ASSERT_TRUE(isEqual(vm::plane3f(0.0f, vm::vec3f::pos_y), vm::plane3f(0.0f, vm::vec3f::pos_y), vm::constants<float>::almostZero()));
    ASSERT_TRUE(isEqual(vm::plane3f(0.0f, vm::vec3f::pos_z), vm::plane3f(0.0f, vm::vec3f::pos_z), vm::constants<float>::almostZero()));
    ASSERT_FALSE(isEqual(vm::plane3f(0.0f, vm::vec3f::pos_x), vm::plane3f(0.0f, vm::vec3f::neg_x), vm::constants<float>::almostZero()));
    ASSERT_FALSE(isEqual(vm::plane3f(0.0f, vm::vec3f::pos_x), vm::plane3f(0.0f, vm::vec3f::pos_y), vm::constants<float>::almostZero()));
}

TEST(PlaneTest, transform) {
}

TEST(PlaneTest, transformed) {
}

TEST(PlaneTest, project) {
    ASSERT_VEC_EQ(vm::vec3f(1.0f, 2.0f, 0.0f), vm::plane3f(0.0f, vm::vec3f::pos_z).projectPoint(vm::vec3f(1.0f, 2.0f, 3.0f)));
    ASSERT_VEC_EQ(vm::vec3f(1.0f, 2.0f, 2.0f), vm::plane3f(2.0f, vm::vec3f::pos_z).projectPoint(vm::vec3f(1.0f, 2.0f, 3.0f)));
}

TEST(PlaneTest, fromPoints) {
    bool valid;
    vm::plane3f plane;
    std::array<vm::vec3f, 3> points;
    const float epsilon = vm::constants<float>::pointStatusEpsilon();

    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(0.0f, 1.0f, 0.0f);
    points[2] = vm::vec3f(1.0f, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_TRUE(valid);
    ASSERT_VEC_EQ(vm::vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);

    // right angle, short vectors
    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(0.0f, epsilon, 0.0f);
    points[2] = vm::vec3f(epsilon, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_TRUE(valid);
    ASSERT_VEC_EQ(vm::vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);

    // plane point vectors at a 45 degree angle, short vectors
    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(epsilon, epsilon, 0.0f);
    points[2] = vm::vec3f(epsilon, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_TRUE(valid);
    ASSERT_VEC_EQ(vm::vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);
    
    // horizontal plane at z=length units above the origin
    points[0] = vm::vec3f(0.0f, 0.0f, epsilon);
    points[1] = vm::vec3f(0.0f, epsilon, epsilon);
    points[2] = vm::vec3f(epsilon, 0.0f, epsilon);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_TRUE(valid);
    ASSERT_VEC_EQ(vm::vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(epsilon, plane.distance);
    
    // small angle (triangle 1000 units wide, length units tall)
    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(1000.0f, epsilon, 0.0f);
    points[2] = vm::vec3f(1000.0f, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_TRUE(valid);
    ASSERT_VEC_EQ(vm::vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);
    
    // small angle
    points[0] = vm::vec3f(224.0f, -400.0f, 1648.0f);
    points[1] = vm::vec3f(304.0f, -432.0f, 1248.0f + epsilon);
    points[2] = vm::vec3f(304.0f, -432.0f, 1248.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_TRUE(valid);
    ASSERT_FLOAT_EQ(1.0f, length(plane.normal));
    
    // too-small angle (triangle 1000 units wide, length/100 units tall)
    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(1000.0f, epsilon/100.0f, 0.0f);
    points[2] = vm::vec3f(1000.0f, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_FALSE(valid);

    // all zero
    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[2] = vm::vec3f(0.0f, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_FALSE(valid);
    
    // same direction, short vectors
    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(2*epsilon, 0.0f, 0.0f);
    points[2] = vm::vec3f(epsilon, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_FALSE(valid);
    
    // opposite, short vectors
    points[0] = vm::vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vm::vec3f(-epsilon, 0.0f, 0.0f);
    points[2] = vm::vec3f(epsilon, 0.0f, 0.0f);

    std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
    ASSERT_FALSE(valid);
}

TEST(PlaneTest, horizontalPlane) {
    const vm::vec3f position(322.0f, -122.2392f, 34.0f);
    const vm::plane3f p = horizontalPlane(position);
    ASSERT_TRUE(p.pointStatus(position) == vm::point_status::inside);
    ASSERT_VEC_EQ(vm::vec3f::pos_z, p.normal);
}

TEST(PlaneTest, orthogonalPlane) {
    const vm::vec3f position(322.0f, -122.2392f, 34.0f);
    const vm::vec3f direction = normalize(vm::vec3f(1.0f, 2.0f, -3.0f));
    const vm::plane3f p = orthogonalPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == vm::point_status::inside);
    ASSERT_VEC_EQ(direction, p.normal);
}

TEST(PlaneTest, alignedOrthogonalPlane) {
    const vm::vec3f position(322.0f, -122.2392f, 34.0f);
    const vm::vec3f direction = normalize(vm::vec3f(1.0f, 2.0f, -3.0f));
    const vm::plane3f p = alignedOrthogonalPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == vm::point_status::inside);
    ASSERT_VEC_EQ(firstAxis(direction), p.normal);
}
