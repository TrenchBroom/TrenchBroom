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

#include <vecmath/vec_decl.h>
#include <vecmath/vec_impl.h>
#include <vecmath/utils.h>
#include "TestUtils.h"

// TODO 2201: move this and other tests to namespace vm, remove vm:: from names

TEST(VecTest, parseVec3fWithValidString) {
    ASSERT_EQ(vm::vec3f(1.0f, 3.0f, 3.5f), vm::vec3f::parse("1.0 3 3.5"));
}

TEST(VecTest, parseVec3fWithShortString) {
    ASSERT_EQ(vm::vec3f(1.0f, 3.0f, 0.0f), vm::vec3f::parse("1.0 3"));
}

TEST(VecTest, constructVec3fWithInvalidString) {
    ASSERT_EQ(vm::vec3f::zero, vm::vec3f::parse("asdf"));
}

TEST(VecTest, constructVec3fFrom2Floats) {
    ASSERT_EQ(vm::vec3f(1.0f, 2.0f, 0.0f), vm::vec3f(1.0f, 2.0f));
}

TEST(VecTest, constructVec3fFrom4Floats) {
    ASSERT_EQ(vm::vec3f(1.0f, 2.0f, 3.0f), vm::vec3f(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(VecTest, constructVec4fFrom3Floats) {
    ASSERT_EQ(vm::vec4f(1.0f, 2.0f, 3.0f, 0.0f), vm::vec4f(1.0f, 2.0f, 3.0f));
}

TEST(VecTest, constructvec2fFromvec2f) {
    const vm::vec2f v(2.0f, 3.0f);
    ASSERT_EQ(v, vm::vec2f(v));
}

TEST(VecTest, constructvec2fFromVec3f) {
    const vm::vec3f v(3.0f, 5.0f, 78.0f);
    ASSERT_EQ(vm::vec2f(v[0], v[1]), vm::vec2f(v));
}

TEST(VecTest, constructvec2fFromVec4f) {
    const vm::vec4f v(3.0f, 5.0f, 2.0f, 7.0f);
    ASSERT_EQ(vm::vec2f(v[0], v[1]), vm::vec2f(v));
}

TEST(VecTest, constructVec3fFromvec2f) {
    const vm::vec2f v(2.0f, 3.0f);
    ASSERT_EQ(vm::vec3f(v[0], v[1], 0.0f), vm::vec3f(v));
}

TEST(VecTest, constructVec4fFromvec2f) {
    const vm::vec2f v(2.0f, 3.0f);
    ASSERT_EQ(vm::vec4f(v[0], v[1], 0.0f, 0.0f), vm::vec4f(v));
}

TEST(VecTest, constructVec4fFromvec2fWithLast1) {
    const vm::vec2f v(3.0f, 5.0f);
    ASSERT_EQ(vm::vec4f(v[0], v[1], 0.0f, 2.0f), vm::vec4f(v, 2.0f));
}

TEST(VecTest, constructVec4fFromvec2fWithLast2) {
    const vm::vec2f v(3.0f, 5.0f);
    ASSERT_EQ(vm::vec4f(v[0], v[1], 3.0f, 2.0f), vm::vec4f(v, 3.0f, 2.0f));
}

TEST(VecTest, constructVec3fFromVec3fWithLast1) {
    const vm::vec3f v(3.0f, 5.0f, 8.0f);
    ASSERT_EQ(vm::vec3f(v[0], v[1], 2.0f), vm::vec3f(v, 2.0f));
}

TEST(VecTest, constructVec3fFromVec3fWithLast2) {
    const vm::vec3f v(3.0f, 5.0f, 8.0f);
    ASSERT_EQ(vm::vec3f(v[0], 2.0f, 4.0f), vm::vec3f(v, 2.0f, 4.0f));
}

TEST(VecTest, assignVec3fToVec3f) {
    const vm::vec3f t(2.0f, 3.0f, 5.0f);
    vm::vec3f v;
    ASSERT_EQ(vm::vec3f(t), (v = t));
}

TEST(VecTest, invertVec3f) {
    ASSERT_EQ( vm::vec3f(-1.0f, -2.0f, -3.0f),
              -vm::vec3f( 1.0f,  2.0f,  3.0f));
}

TEST(VecTest, addVec3f) {
    ASSERT_EQ(vm::vec3f(4.0f, 4.0f, 4.0f),
              vm::vec3f(1.0f, 2.0f, 3.0f) +
              vm::vec3f(3.0f, 2.0f, 1.0f));
}

TEST(VecTest, subtractVec3f) {
    ASSERT_EQ(vm::vec3f(1.0f, 1.0f, -1.0f),
              vm::vec3f(2.0f, 3.0f, 1.0f) -
              vm::vec3f(1.0f, 2.0f, 2.0f));
}

TEST(VecTest, multiplyVec3fWithScalar) {
    ASSERT_EQ(vm::vec3f(6.0f, 9.0f, 3.0f),
              vm::vec3f(2.0f, 3.0f, 1.0f) * 3.0f);
}

TEST(VecTest, divideVec3fByScalar) {
    ASSERT_EQ(vm::vec3f(1.0f, 18.0f, 2.0f),
              vm::vec3f(2.0f, 36.0f, 4.0f) / 2.0f);
}

TEST(VecTest, subscriptAccess) {
    vm::vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
    ASSERT_EQ(1.0f, v[0]);
    ASSERT_EQ(2.0f, v[1]);
    ASSERT_EQ(3.0f, v[2]);
    ASSERT_EQ(4.0f, v[3]);
}

TEST(VecTest, accessors) {
    vm::vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
    ASSERT_EQ(v[0], v.x());
    ASSERT_EQ(v[1], v.y());
    ASSERT_EQ(v[2], v.z());
    ASSERT_EQ(v[3], v.w());
    ASSERT_EQ(vm::vec2f(1.0f, 2.0f), v.xy());
    ASSERT_EQ(vm::vec3f(1.0f, 2.0f, 3.0f), v.xyz());
    ASSERT_EQ(v, v.xyzw());
}

TEST(VecTest, vec4fOverLast) {
    vm::vec4f v(2.0f, 4.0f, 8.0f, 2.0f);
    ASSERT_EQ(vm::vec3f(1.0f, 2.0f, 4.0f), toCartesianCoords(v));
}

TEST(VecTest, snap) {
    ASSERT_EQ(vm::vec2f( 8.0f,  0.0f), snap(vm::vec2f( 7.0f, -3.0f), vm::vec2f( 4.0f, 12.0f)));
    ASSERT_EQ(vm::vec2f( 8.0f, -6.0f), snap(vm::vec2f( 7.0f, -5.0f), vm::vec2f(-4.0f, -2.0f)));
    ASSERT_EQ(vm::vec2f(-8.0f,  6.0f), snap(vm::vec2f(-7.0f,  5.0f), vm::vec2f(-4.0f, -2.0f)));
}

TEST(VecTest, vec3fDot) {
    ASSERT_FLOAT_EQ(-748013.6097f, dot(vm::vec3f(2.3f, 8.7878f, -2323.0f), vm::vec3f(4.333f, -2.0f, 322.0f)));
}

TEST(VecTest, vec3fDotNull) {
    ASSERT_FLOAT_EQ(0.0f, dot(vm::vec3f(2.3f, 8.7878f, -2323.0f), vm::vec3f::zero));
}

TEST(VecTest, vec3fLength) {
    ASSERT_FLOAT_EQ(0.0f, length(vm::vec3f::zero));
    ASSERT_FLOAT_EQ(1.0f, length(vm::vec3f::pos_x));
    ASSERT_FLOAT_EQ(std::sqrt(5396411.51542884f), length(vm::vec3f(2.3f, 8.7878f, -2323.0f)));
}

TEST(VecTest, vec3fLengthSquared) {
    ASSERT_FLOAT_EQ(0.0f, squaredLength(vm::vec3f::zero));
    ASSERT_FLOAT_EQ(1.0f, squaredLength(vm::vec3f::pos_x));
    ASSERT_FLOAT_EQ(5396411.51542884f, squaredLength(vm::vec3f(2.3f, 8.7878f, -2323.0f)));
}

TEST(VecTest, vec3fDistanceTo) {
    const vm::vec3f v1(2.3f, 8.7878f, -2323.0f);
    const vm::vec3f v2(4.333f, -2.0f, 322.0f);
    ASSERT_FLOAT_EQ(0.0f, distance(v1, v1));
    ASSERT_FLOAT_EQ(length(v1), distance(v1, vm::vec3f::zero));
    ASSERT_FLOAT_EQ(length(v1 - v2), distance(v1, v2));
}

TEST(VecTest, vec3fSquaredDistanceTo) {
    const vm::vec3f v1(2.3f, 8.7878f, -2323.0f);
    const vm::vec3f v2(4.333f, -2.0f, 322.0f);
    ASSERT_FLOAT_EQ(0.0f, squaredDistance(v1, v1));
    ASSERT_FLOAT_EQ(squaredLength(v1), squaredDistance(v1, vm::vec3f::zero));
    ASSERT_FLOAT_EQ(squaredLength(v1 - v2), squaredDistance(v1, v2));
}

TEST(VecTest, vec3fNormalize) {
    ASSERT_EQ(vm::vec3f::pos_x, normalize(vm::vec3f::pos_x));
    ASSERT_EQ(vm::vec3f::neg_x, normalize(vm::vec3f::neg_x));
    
    const vm::vec3f v1(2.3f, 8.7878f, -2323.0f);
    const vm::vec3f v2(4.333f, -2.0f, 322.0f);
    ASSERT_VEC_EQ((v1 / length(v1)), normalize(v1));
    ASSERT_VEC_EQ((v2 / length(v2)), normalize(v2));
}

TEST(VecTest, vec3fNull) {
    ASSERT_TRUE(isZero(vm::vec3f::zero));
    ASSERT_FALSE(isZero(vm::vec3f::pos_x));
}

TEST(VecTest, vec3fFill) {
    ASSERT_EQ(vm::vec3f(2.0f, 2.0, 2.0f), vm::vec3f::fill(2.0f));
}

TEST(VecTest, vec3fMajorComponent) {
    ASSERT_EQ(vm::axis::x, majorComponent(vm::vec3f::pos_x, 0));
    ASSERT_EQ(vm::axis::x, majorComponent(vm::vec3f::neg_x, 0));
    ASSERT_EQ(vm::axis::y, majorComponent(vm::vec3f::pos_y, 0));
    ASSERT_EQ(vm::axis::y, majorComponent(vm::vec3f::neg_y, 0));
    ASSERT_EQ(vm::axis::z, majorComponent(vm::vec3f::pos_z, 0));
    ASSERT_EQ(vm::axis::z, majorComponent(vm::vec3f::neg_z, 0));
    
    ASSERT_EQ(vm::axis::x, majorComponent(vm::vec3f(3.0f, -1.0f, 2.0f), 0));
    ASSERT_EQ(vm::axis::z, majorComponent(vm::vec3f(3.0f, -1.0f, 2.0f), 1));
    ASSERT_EQ(vm::axis::y, majorComponent(vm::vec3f(3.0f, -1.0f, 2.0f), 2));
}

TEST(VecTest, vec3fMajorAxis) {
    ASSERT_EQ(vm::vec3f::pos_x, majorAxis(vm::vec3f::pos_x, 0));
    ASSERT_EQ(vm::vec3f::neg_x, majorAxis(vm::vec3f::neg_x, 0));
    ASSERT_EQ(vm::vec3f::pos_y, majorAxis(vm::vec3f::pos_y, 0));
    ASSERT_EQ(vm::vec3f::neg_y, majorAxis(vm::vec3f::neg_y, 0));
    ASSERT_EQ(vm::vec3f::pos_z, majorAxis(vm::vec3f::pos_z, 0));
    ASSERT_EQ(vm::vec3f::neg_z, majorAxis(vm::vec3f::neg_z, 0));

    ASSERT_EQ(vm::vec3f::pos_x, majorAxis(vm::vec3f(3.0f, -1.0f, 2.0f), 0));
    ASSERT_EQ(vm::vec3f::pos_z, majorAxis(vm::vec3f(3.0f, -1.0f, 2.0f), 1));
    ASSERT_EQ(vm::vec3f::neg_y, majorAxis(vm::vec3f(3.0f, -1.0f, 2.0f), 2));
}

TEST(VecTest, vec3fAbsMajorAxis) {
    ASSERT_EQ(vm::vec3f::pos_x, absMajorAxis(vm::vec3f::pos_x, 0));
    ASSERT_EQ(vm::vec3f::pos_x, absMajorAxis(vm::vec3f::neg_x, 0));
    ASSERT_EQ(vm::vec3f::pos_y, absMajorAxis(vm::vec3f::pos_y, 0));
    ASSERT_EQ(vm::vec3f::pos_y, absMajorAxis(vm::vec3f::neg_y, 0));
    ASSERT_EQ(vm::vec3f::pos_z, absMajorAxis(vm::vec3f::pos_z, 0));
    ASSERT_EQ(vm::vec3f::pos_z, absMajorAxis(vm::vec3f::neg_z, 0));
    
    ASSERT_EQ(vm::vec3f::pos_x, absMajorAxis(vm::vec3f(3.0f, -1.0f, 2.0f), 0));
    ASSERT_EQ(vm::vec3f::pos_z, absMajorAxis(vm::vec3f(3.0f, -1.0f, 2.0f), 1));
    ASSERT_EQ(vm::vec3f::pos_y, absMajorAxis(vm::vec3f(3.0f, -1.0f, 2.0f), 2));
}

TEST(VecTest, multiplyScalarWithVec3f) {
    ASSERT_EQ(       vm::vec3f(6.0f, 9.0f, 3.0f),
              3.0f * vm::vec3f(2.0f, 3.0f, 1.0f));
}

TEST(VecTest, vec3fCrossProduct) {
    ASSERT_EQ(vm::vec3f::zero, cross(vm::vec3f::zero, vm::vec3f::zero));
    ASSERT_EQ(vm::vec3f::zero, cross(vm::vec3f::zero, vm::vec3f(2.0f, 34.233f, -10003.0002f)));
    ASSERT_EQ(vm::vec3f::pos_z, cross(vm::vec3f::pos_x, vm::vec3f::pos_y));
    ASSERT_VEC_EQ(vm::vec3f(-2735141.499f, 282853.508f, 421.138f), cross(vm::vec3f(12.302f, -0.0017f, 79898.3f),
                                                                     vm::vec3f(2.0f, 34.233f, -10003.0002f)));

    const vm::vec3f t1(7.0f, 4.0f, 0.0f);
    const vm::vec3f t2(-2.0f, 22.0f, 0.0f);

    const vm::vec3f c1 = normalize(cross(t1, t2));
    const vm::vec3f c2 = normalize(cross(normalize(t1), normalize(t2)));
    ASSERT_VEC_EQ(c1, c2);
}

TEST(VecTest, angleBetween) {
    ASSERT_FLOAT_EQ(angleBetween(vm::vec3f::pos_x, vm::vec3f::pos_x, vm::vec3f::pos_z), 0.0f);
    ASSERT_FLOAT_EQ(angleBetween(vm::vec3f::pos_y, vm::vec3f::pos_x, vm::vec3f::pos_z), vm::Cf::piOverTwo());
    ASSERT_FLOAT_EQ(angleBetween(vm::vec3f::neg_x, vm::vec3f::pos_x, vm::vec3f::pos_z), vm::Cf::pi());
    ASSERT_FLOAT_EQ(angleBetween(vm::vec3f::neg_y, vm::vec3f::pos_x, vm::vec3f::pos_z), 3.0f * vm::Cf::piOverTwo());
}

TEST(VecTest, convexHull2dSimple) {
    const vm::vec3d p1(0.0, 0.0, 0.0);
    const vm::vec3d p2(8.0, 8.0, 0.0);
    const vm::vec3d p3(8.0, 0.0, 0.0);
    const vm::vec3d p4(0.0, 8.0, 0.0);
    
    vm::vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    
    const vm::vec3d::List hull = vm::convexHull2D<double>(points);
    ASSERT_EQ(4u, hull.size());
    ASSERT_VEC_EQ(p3, hull[0]);
    ASSERT_VEC_EQ(p2, hull[1]);
    ASSERT_VEC_EQ(p4, hull[2]);
    ASSERT_VEC_EQ(p1, hull[3]);
}

TEST(VecTest, convexHull2dSimpleWithInternalPoint) {
    const vm::vec3d p1(0.0, 0.0, 0.0);
    const vm::vec3d p2(8.0, 8.0, 0.0);
    const vm::vec3d p3(8.0, 0.0, 0.0);
    const vm::vec3d p4(0.0, 8.0, 0.0);
    const vm::vec3d p5(4.0, 4.0, 0.0);
    
    vm::vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    
    const vm::vec3d::List hull = vm::convexHull2D<double>(points);
    ASSERT_EQ(4u, hull.size());
    ASSERT_VEC_EQ(p3, hull[0]);
    ASSERT_VEC_EQ(p2, hull[1]);
    ASSERT_VEC_EQ(p4, hull[2]);
    ASSERT_VEC_EQ(p1, hull[3]);
}

TEST(VecTest, convexHull2dSimpleWithPointOnLine) {
    const vm::vec3d p1(0.0, 0.0, 0.0);
    const vm::vec3d p2(8.0, 8.0, 0.0);
    const vm::vec3d p3(8.0, 0.0, 0.0);
    const vm::vec3d p4(0.0, 8.0, 0.0);
    const vm::vec3d p5(4.0, 0.0, 0.0);
    
    vm::vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    
    const vm::vec3d::List hull = vm::convexHull2D<double>(points);
    ASSERT_EQ(4u, hull.size());
    ASSERT_VEC_EQ(p3, hull[0]);
    ASSERT_VEC_EQ(p2, hull[1]);
    ASSERT_VEC_EQ(p4, hull[2]);
    ASSERT_VEC_EQ(p1, hull[3]);
}

TEST(VecTest, colinear) {
    ASSERT_TRUE(colinear(vm::vec3d::zero, vm::vec3d::zero, vm::vec3d::zero));
    ASSERT_TRUE(colinear(vm::vec3d::one,  vm::vec3d::one,  vm::vec3d::one));
    ASSERT_TRUE(colinear(vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(0.0, 0.0, 1.0), vm::vec3d(0.0, 0.0, 2.0)));
    ASSERT_FALSE(colinear(vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(1.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0)));
    ASSERT_FALSE(colinear(vm::vec3d(0.0, 0.0, 0.0), vm::vec3d(10.0, 0.0, 0.0), vm::vec3d(0.0, 1.0, 0.0)));
}

TEST(VecTest, mix) {
    ASSERT_EQ(vm::vec3d::zero, mix(vm::vec3d::zero, vm::vec3d::one, vm::vec3d::zero));
    ASSERT_EQ(vm::vec3d::one, mix(vm::vec3d::zero, vm::vec3d::one, vm::vec3d::one));
    ASSERT_EQ(vm::vec3d::one / 2.0, mix(vm::vec3d::zero, vm::vec3d::one, vm::vec3d::one / 2.0));
}

TEST(VecTest, swizzle) {
    ASSERT_EQ(vm::vec3d(2, 3, 1), swizzle(vm::vec3d(1, 2, 3), 0));
    ASSERT_EQ(vm::vec3d(3, 1, 2), swizzle(vm::vec3d(1, 2, 3), 1));
    ASSERT_EQ(vm::vec3d(1, 2, 3), swizzle(vm::vec3d(1, 2, 3), 2));
}

TEST(VecTest, unswizzle) {
    for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(vm::vec3d(1, 2, 3), unswizzle(swizzle(vm::vec3d(1, 2, 3), i), i));
    }
}
