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
#include <vecmath/scalar.h>
#include <vecmath/util.h>

#include "Macros.h"
#include "TestUtils.h"

#include <array>

namespace vm {
    TEST(PlaneTest, defaultConstructor) {
        const plane3f p;
        ASSERT_EQ(0.0f, p.distance);
        ASSERT_EQ(vec3f::zero, p.normal);
    }
    
    TEST(PlaneTest, constructWithDistanceAndNormal) {
        const float d = 123.0f;
        const vec3f n = normalize(vec3f(1.0f, 2.0f, 3.0f));
        const plane3f p(d, n);
        ASSERT_FLOAT_EQ(d, p.distance);
        ASSERT_VEC_EQ(n, p.normal);
    }
    
    TEST(PlaneTest, constructWithAnchorAndNormal) {
        const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
        const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
        const plane3f p(a, n);
        ASSERT_FLOAT_EQ(dot(a, n), p.distance);
        ASSERT_VEC_EQ(n, p.normal);
    }
    
    TEST(PlaneTest, anchor) {
        const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
        const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
        const plane3f p(a, n);
        ASSERT_VEC_EQ(p.distance * n, p.anchor());
    }

    TEST(PlaneTest, at) {
        const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
        const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
        const plane3f p(a, n);
        const vec2f point1(27.022f, -12.0123223f);

        ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.y() - point1.y() * p.normal.z()) / p.normal[axis::x],
                        p.at(point1, axis::x));
        ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.z()) / p.normal[axis::y],
                        p.at(point1, axis::y));
        ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.y()) / p.normal[axis::z],
                        p.at(point1, axis::z));
    }

    TEST(PlaneTest, at_parallelPlanes) {
        const plane3f p1(10.0f, vec3f::pos_x);

        ASSERT_FLOAT_EQ(p1.distance, p1.at(vec2f(2.0f, 1.0f), axis::x));
        ASSERT_FLOAT_EQ(p1.distance, p1.at(vec2f(22.0f, -34322.0232f), axis::x));
        ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(2.0f, 1.0f), axis::y));
        ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(22.0f, -34322.0232f), axis::y));
        ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(2.0f, 1.0f), axis::z));
        ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(22.0f, -34322.0232f), axis::z));
    }

    TEST(PlaneTest, xyzAt) {
        const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
        const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
        const plane3f p(a, n);
        const vec2f point1(27.022f, -12.0123223f);

        ASSERT_FLOAT_EQ(p.at(point1, axis::x), p.xAt(point1));
        ASSERT_FLOAT_EQ(p.at(point1, axis::y), p.yAt(point1));
        ASSERT_FLOAT_EQ(p.at(point1, axis::z), p.zAt(point1));
    }

    TEST(PlaneTest, pointDistance) {
        const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
        const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
        const plane3f p(a, n);
        const vec3f point(1.0f, -32.37873f, 32.0f);
        ASSERT_EQ(dot(point, p.normal) - p.distance, p.pointDistance(point));
    }

    TEST(PlaneTest, pointStatus) {
        const plane3f p(10.0f, vec3f::pos_z);
        ASSERT_EQ(point_status::above, p.pointStatus(vec3f(0.0f, 0.0f, 11.0f)));
        ASSERT_EQ(point_status::below, p.pointStatus(vec3f(0.0f, 0.0f, 9.0f)));
        ASSERT_EQ(point_status::inside, p.pointStatus(vec3f(0.0f, 0.0f, 10.0f)));
    }

    TEST(PlaneTest, transform) {
        const auto p = plane3d(vec3d::one, vec3d::pos_z);
        const auto rm = rotationMatrix(toRadians(15.0), toRadians(20.0), toRadians(-12.0));
        const auto tm = translationMatrix(vec3d::one);

        const auto pt = p.transform(rm * tm);
        ASSERT_TRUE(isUnit(p.normal));
        ASSERT_EQ(point_status::inside, pt.pointStatus(rm * tm * p.anchor()));
        ASSERT_VEC_EQ(rm * p.normal, pt.normal);
    }

    TEST(PlaneTest, projectPoint) {
        ASSERT_VEC_EQ(vec3d(0, 0, 0), plane3d(0.0, vec3d::pos_z).projectPoint(vec3d(0, 0, 10)));
        ASSERT_VEC_EQ(vec3d(1, 2, 0), plane3d(0.0, vec3d::pos_z).projectPoint(vec3d(1, 2, 10)));
        ASSERT_VEC_EQ(vec3d(0, 0, 0), plane3d(0.0, normalize(vec3d(1, 1, 1))).projectPoint(vec3d(10, 10, 10)));
    }

    TEST(PlaneTest, projectPoint_direction) {
        ASSERT_VEC_EQ(vec3d(0, 0, 0), plane3d(0.0, vec3d::pos_z).projectPoint(vec3d(0, 0, 10), vec3d::pos_z));
        ASSERT_VEC_EQ(vec3d(1, 2, 0), plane3d(0.0, vec3d::pos_z).projectPoint(vec3d(1, 2, 10), vec3d::pos_z));
        ASSERT_VEC_EQ(vec3d(0, 0, 0), plane3d(0.0, vec3d::pos_z).projectPoint(vec3d(10, 10, 10), normalize(vec3d(1, 1, 1))));
    }

    TEST(PlaneTest, projectVector) {
        ASSERT_VEC_EQ(vec3d(1, 1, 0), plane3d(0.0, vec3d::pos_z).projectVector(vec3d(1, 1, 1)));
        ASSERT_VEC_EQ(vec3d(1, 1, 0), plane3d(1.0, vec3d::pos_z).projectVector(vec3d(1, 1, 1)));
    }

    TEST(PlaneTest, projectVector_direction) {
        ASSERT_VEC_EQ(vec3d(1, 1, 0), plane3d(0.0, vec3d::pos_z).projectVector(vec3d(1, 1, 1), vec3d::pos_z));
        ASSERT_VEC_EQ(vec3d(1, 1, 0), plane3d(1.0, vec3d::pos_z).projectVector(vec3d(1, 1, 1), vec3d::pos_z));
        ASSERT_VEC_EQ(vec3d(2, 2, 0), plane3d(0.0, vec3d::pos_z).projectVector(vec3d(1, 1, 1), normalize(vec3d(1, 1, -1))));
    }

    TEST(PlaneTest, isEqual) {
        ASSERT_TRUE(isEqual(plane3f(0.0f, vec3f::pos_x), plane3f(0.0f, vec3f::pos_x), constants<float>::almostZero()));
        ASSERT_TRUE(isEqual(plane3f(0.0f, vec3f::pos_y), plane3f(0.0f, vec3f::pos_y), constants<float>::almostZero()));
        ASSERT_TRUE(isEqual(plane3f(0.0f, vec3f::pos_z), plane3f(0.0f, vec3f::pos_z), constants<float>::almostZero()));
        ASSERT_FALSE(isEqual(plane3f(0.0f, vec3f::pos_x), plane3f(0.0f, vec3f::neg_x), constants<float>::almostZero()));
        ASSERT_FALSE(isEqual(plane3f(0.0f, vec3f::pos_x), plane3f(0.0f, vec3f::pos_y), constants<float>::almostZero()));
    }

    TEST(PlaneTest, equal) {
        ASSERT_TRUE (plane3d() == plane3d());
        ASSERT_TRUE (plane3d(10.0, vec3d::pos_z) == plane3d(10.0, vec3d::pos_z));
        ASSERT_FALSE(plane3d(20.0, vec3d::pos_z) == plane3d(10.0, vec3d::pos_z));
        ASSERT_FALSE(plane3d(10.0, vec3d::neg_z) == plane3d(10.0, vec3d::pos_z));
        ASSERT_FALSE(plane3d(10.0, normalize(vec3d::one)) == plane3d(10.0, vec3d::pos_z));
    }

    TEST(PlaneTest, notEqual) {
        ASSERT_FALSE(plane3d() != plane3d());
        ASSERT_FALSE(plane3d(10.0, vec3d::pos_z) != plane3d(10.0, vec3d::pos_z));
        ASSERT_TRUE (plane3d(20.0, vec3d::pos_z) != plane3d(10.0, vec3d::pos_z));
        ASSERT_TRUE (plane3d(10.0, vec3d::neg_z) != plane3d(10.0, vec3d::pos_z));
        ASSERT_TRUE (plane3d(10.0, normalize(vec3d::one)) != plane3d(10.0, vec3d::pos_z));
    }

    template <typename T>
    void assertValidPlaneNormal(const vec<T,3>& expected, const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3) {
        const auto [valid, normal] = planeNormal(p1, p2, p3);
        ASSERT_TRUE(valid);
        ASSERT_VEC_EQ(expected, normal);
    }

    template <typename T>
    void assertInvalidPlaneNormal(const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3) {
        const auto [valid, normal] = planeNormal(p1, p2, p3);
        ASSERT_FALSE(valid);
        unused(normal);
    }

    TEST(PlaneTest, planeNormal) {
        assertValidPlaneNormal(vec3d::pos_z, vec3d::zero, vec3d::pos_y, vec3d::pos_x);
        assertValidPlaneNormal(vec3d::pos_z, vec3d::zero, normalize(vec3d(1, 1, 0)), vec3d::pos_x);
        assertInvalidPlaneNormal(vec3d::zero, vec3d::zero, vec3d::pos_x);
        assertInvalidPlaneNormal(vec3d::zero, vec3d::pos_x, vec3d::pos_x);
        assertInvalidPlaneNormal(vec3d::zero, vec3d::neg_x, vec3d::pos_x);
        assertInvalidPlaneNormal(vec3d::zero, vec3d::zero, vec3d::pos_x);
    }

    TEST(PlaneTest, fromPoints) {
        bool valid;
        plane3f plane;
        std::array<vec3f, 3> points;
        const float epsilon = constants<float>::pointStatusEpsilon();
    
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(0.0f, 1.0f, 0.0f);
        points[2] = vec3f(1.0f, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_TRUE(valid);
        ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
        ASSERT_FLOAT_EQ(0.0f, plane.distance);
    
        // right angle, short vectors
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(0.0f, epsilon, 0.0f);
        points[2] = vec3f(epsilon, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_TRUE(valid);
        ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
        ASSERT_FLOAT_EQ(0.0f, plane.distance);
    
        // plane point vectors at a 45 degree angle, short vectors
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(epsilon, epsilon, 0.0f);
        points[2] = vec3f(epsilon, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_TRUE(valid);
        ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
        ASSERT_FLOAT_EQ(0.0f, plane.distance);
        
        // horizontal plane at z=length units above the origin
        points[0] = vec3f(0.0f, 0.0f, epsilon);
        points[1] = vec3f(0.0f, epsilon, epsilon);
        points[2] = vec3f(epsilon, 0.0f, epsilon);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_TRUE(valid);
        ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
        ASSERT_FLOAT_EQ(epsilon, plane.distance);
        
        // small angle (triangle 1000 units wide, length units tall)
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(1000.0f, epsilon, 0.0f);
        points[2] = vec3f(1000.0f, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_TRUE(valid);
        ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
        ASSERT_FLOAT_EQ(0.0f, plane.distance);
        
        // small angle
        points[0] = vec3f(224.0f, -400.0f, 1648.0f);
        points[1] = vec3f(304.0f, -432.0f, 1248.0f + epsilon);
        points[2] = vec3f(304.0f, -432.0f, 1248.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_TRUE(valid);
        ASSERT_FLOAT_EQ(1.0f, length(plane.normal));
        
        // too-small angle (triangle 1000 units wide, length/100 units tall)
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(1000.0f, epsilon/100.0f, 0.0f);
        points[2] = vec3f(1000.0f, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_FALSE(valid);
    
        // all zero
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(0.0f, 0.0f, 0.0f);
        points[2] = vec3f(0.0f, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_FALSE(valid);
        
        // same direction, short vectors
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(2*epsilon, 0.0f, 0.0f);
        points[2] = vec3f(epsilon, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_FALSE(valid);
        
        // opposite, short vectors
        points[0] = vec3f(0.0f, 0.0f, 0.0f);
        points[1] = vec3f(-epsilon, 0.0f, 0.0f);
        points[2] = vec3f(epsilon, 0.0f, 0.0f);
    
        std::tie(valid, plane) = fromPoints(std::begin(points), std::end(points));
        ASSERT_FALSE(valid);
    }
    
    TEST(PlaneTest, horizontalPlane) {
        const vec3f position(322.0f, -122.2392f, 34.0f);
        const plane3f p = horizontalPlane(position);
        ASSERT_TRUE(p.pointStatus(position) == point_status::inside);
        ASSERT_VEC_EQ(vec3f::pos_z, p.normal);
    }
    
    TEST(PlaneTest, orthogonalPlane) {
        const vec3f position(322.0f, -122.2392f, 34.0f);
        const vec3f direction = normalize(vec3f(1.0f, 2.0f, -3.0f));
        const plane3f p = orthogonalPlane(position, direction);
        ASSERT_TRUE(p.pointStatus(position) == point_status::inside);
        ASSERT_VEC_EQ(direction, p.normal);
    }
    
    TEST(PlaneTest, alignedOrthogonalPlane) {
        const vec3f position(322.0f, -122.2392f, 34.0f);
        const vec3f direction = normalize(vec3f(1.0f, 2.0f, -3.0f));
        const plane3f p = alignedOrthogonalPlane(position, direction);
        ASSERT_TRUE(p.pointStatus(position) == point_status::inside);
        ASSERT_VEC_EQ(firstAxis(direction), p.normal);
    }
}
