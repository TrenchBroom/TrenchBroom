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
#include <vecmath/utils.h>
#include "TestUtils.h"

// TODO 2201: write more tests

namespace vm {
    TEST(VecTest, parseVec3fWithValidString) {
        ASSERT_EQ(vec3f(1.0f, 3.0f, 3.5f), vec3f::parse("1.0 3 3.5"));
    }
    
    TEST(VecTest, parseVec3fWithShortString) {
        ASSERT_EQ(vec3f(1.0f, 3.0f, 0.0f), vec3f::parse("1.0 3"));
    }
    
    TEST(VecTest, constructVec3fWithInvalidString) {
        ASSERT_EQ(vec3f::zero, vec3f::parse("asdf"));
    }
    
    TEST(VecTest, constructVec3fFrom2Floats) {
        ASSERT_EQ(vec3f(1.0f, 2.0f, 0.0f), vec3f(1.0f, 2.0f));
    }
    
    TEST(VecTest, constructVec3fFrom4Floats) {
        ASSERT_EQ(vec3f(1.0f, 2.0f, 3.0f), vec3f(1.0f, 2.0f, 3.0f, 4.0f));
    }
    
    TEST(VecTest, constructVec4fFrom3Floats) {
        ASSERT_EQ(vec4f(1.0f, 2.0f, 3.0f, 0.0f), vec4f(1.0f, 2.0f, 3.0f));
    }
    
    TEST(VecTest, constructvec2fFromvec2f) {
        const vec2f v(2.0f, 3.0f);
        ASSERT_EQ(v, vec2f(v));
    }
    
    TEST(VecTest, constructvec2fFromVec3f) {
        const vec3f v(3.0f, 5.0f, 78.0f);
        ASSERT_EQ(vec2f(v[0], v[1]), vec2f(v));
    }
    
    TEST(VecTest, constructvec2fFromVec4f) {
        const vec4f v(3.0f, 5.0f, 2.0f, 7.0f);
        ASSERT_EQ(vec2f(v[0], v[1]), vec2f(v));
    }
    
    TEST(VecTest, constructVec3fFromvec2f) {
        const vec2f v(2.0f, 3.0f);
        ASSERT_EQ(vec3f(v[0], v[1], 0.0f), vec3f(v));
    }
    
    TEST(VecTest, constructVec4fFromvec2f) {
        const vec2f v(2.0f, 3.0f);
        ASSERT_EQ(vec4f(v[0], v[1], 0.0f, 0.0f), vec4f(v));
    }
    
    TEST(VecTest, constructVec4fFromvec2fWithLast1) {
        const vec2f v(3.0f, 5.0f);
        ASSERT_EQ(vec4f(v[0], v[1], 0.0f, 2.0f), vec4f(v, 2.0f));
    }
    
    TEST(VecTest, constructVec4fFromvec2fWithLast2) {
        const vec2f v(3.0f, 5.0f);
        ASSERT_EQ(vec4f(v[0], v[1], 3.0f, 2.0f), vec4f(v, 3.0f, 2.0f));
    }
    
    TEST(VecTest, constructVec3fFromVec3fWithLast1) {
        const vec3f v(3.0f, 5.0f, 8.0f);
        ASSERT_EQ(vec3f(v[0], v[1], 2.0f), vec3f(v, 2.0f));
    }
    
    TEST(VecTest, constructVec3fFromVec3fWithLast2) {
        const vec3f v(3.0f, 5.0f, 8.0f);
        ASSERT_EQ(vec3f(v[0], 2.0f, 4.0f), vec3f(v, 2.0f, 4.0f));
    }
    
    TEST(VecTest, assignVec3fToVec3f) {
        const vec3f t(2.0f, 3.0f, 5.0f);
        vec3f v;
        ASSERT_EQ(vec3f(t), (v = t));
    }
    
    TEST(VecTest, invertVec3f) {
        ASSERT_EQ( vec3f(-1.0f, -2.0f, -3.0f),
                  -vec3f( 1.0f,  2.0f,  3.0f));
    }
    
    TEST(VecTest, addVec3f) {
        ASSERT_EQ(vec3f(4.0f, 4.0f, 4.0f),
                  vec3f(1.0f, 2.0f, 3.0f) +
                  vec3f(3.0f, 2.0f, 1.0f));
    }
    
    TEST(VecTest, subtractVec3f) {
        ASSERT_EQ(vec3f(1.0f, 1.0f, -1.0f),
                  vec3f(2.0f, 3.0f, 1.0f) -
                  vec3f(1.0f, 2.0f, 2.0f));
    }
    
    TEST(VecTest, multiplyVec3fWithScalar) {
        ASSERT_EQ(vec3f(6.0f, 9.0f, 3.0f),
                  vec3f(2.0f, 3.0f, 1.0f) * 3.0f);
    }
    
    TEST(VecTest, divideVec3fByScalar) {
        ASSERT_EQ(vec3f(1.0f, 18.0f, 2.0f),
                  vec3f(2.0f, 36.0f, 4.0f) / 2.0f);
    }
    
    TEST(VecTest, subscriptAccess) {
        vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
        ASSERT_EQ(1.0f, v[0]);
        ASSERT_EQ(2.0f, v[1]);
        ASSERT_EQ(3.0f, v[2]);
        ASSERT_EQ(4.0f, v[3]);
    }
    
    TEST(VecTest, accessors) {
        vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
        ASSERT_EQ(v[0], v.x());
        ASSERT_EQ(v[1], v.y());
        ASSERT_EQ(v[2], v.z());
        ASSERT_EQ(v[3], v.w());
        ASSERT_EQ(vec2f(1.0f, 2.0f), v.xy());
        ASSERT_EQ(vec3f(1.0f, 2.0f, 3.0f), v.xyz());
        ASSERT_EQ(v, v.xyzw());
    }
    
    TEST(VecTest, vec4fOverLast) {
        vec4f v(2.0f, 4.0f, 8.0f, 2.0f);
        ASSERT_EQ(vec3f(1.0f, 2.0f, 4.0f), toCartesianCoords(v));
    }
    
    TEST(VecTest, snap) {
        ASSERT_EQ(vec2f( 8.0f,  0.0f), snap(vec2f( 7.0f, -3.0f), vec2f( 4.0f, 12.0f)));
        ASSERT_EQ(vec2f( 8.0f, -6.0f), snap(vec2f( 7.0f, -5.0f), vec2f(-4.0f, -2.0f)));
        ASSERT_EQ(vec2f(-8.0f,  6.0f), snap(vec2f(-7.0f,  5.0f), vec2f(-4.0f, -2.0f)));
    }
    
    TEST(VecTest, vec3fDot) {
        ASSERT_FLOAT_EQ(-748013.6097f, dot(vec3f(2.3f, 8.7878f, -2323.0f), vec3f(4.333f, -2.0f, 322.0f)));
    }
    
    TEST(VecTest, vec3fDotNull) {
        ASSERT_FLOAT_EQ(0.0f, dot(vec3f(2.3f, 8.7878f, -2323.0f), vec3f::zero));
    }
    
    TEST(VecTest, vec3fLength) {
        ASSERT_FLOAT_EQ(0.0f, length(vec3f::zero));
        ASSERT_FLOAT_EQ(1.0f, length(vec3f::pos_x));
        ASSERT_FLOAT_EQ(std::sqrt(5396411.51542884f), length(vec3f(2.3f, 8.7878f, -2323.0f)));
    }
    
    TEST(VecTest, vec3fLengthSquared) {
        ASSERT_FLOAT_EQ(0.0f, squaredLength(vec3f::zero));
        ASSERT_FLOAT_EQ(1.0f, squaredLength(vec3f::pos_x));
        ASSERT_FLOAT_EQ(5396411.51542884f, squaredLength(vec3f(2.3f, 8.7878f, -2323.0f)));
    }
    
    TEST(VecTest, vec3fDistanceTo) {
        const vec3f v1(2.3f, 8.7878f, -2323.0f);
        const vec3f v2(4.333f, -2.0f, 322.0f);
        ASSERT_FLOAT_EQ(0.0f, distance(v1, v1));
        ASSERT_FLOAT_EQ(length(v1), distance(v1, vec3f::zero));
        ASSERT_FLOAT_EQ(length(v1 - v2), distance(v1, v2));
    }
    
    TEST(VecTest, vec3fSquaredDistanceTo) {
        const vec3f v1(2.3f, 8.7878f, -2323.0f);
        const vec3f v2(4.333f, -2.0f, 322.0f);
        ASSERT_FLOAT_EQ(0.0f, squaredDistance(v1, v1));
        ASSERT_FLOAT_EQ(squaredLength(v1), squaredDistance(v1, vec3f::zero));
        ASSERT_FLOAT_EQ(squaredLength(v1 - v2), squaredDistance(v1, v2));
    }
    
    TEST(VecTest, vec3fNormalize) {
        ASSERT_EQ(vec3f::pos_x, normalize(vec3f::pos_x));
        ASSERT_EQ(vec3f::neg_x, normalize(vec3f::neg_x));
        
        const vec3f v1(2.3f, 8.7878f, -2323.0f);
        const vec3f v2(4.333f, -2.0f, 322.0f);
        ASSERT_VEC_EQ((v1 / length(v1)), normalize(v1));
        ASSERT_VEC_EQ((v2 / length(v2)), normalize(v2));
    }
    
    TEST(VecTest, vec3fNull) {
        ASSERT_TRUE(isZero(vec3f::zero));
        ASSERT_FALSE(isZero(vec3f::pos_x));
    }
    
    TEST(VecTest, vec3fFill) {
        ASSERT_EQ(vec3f(2.0f, 2.0, 2.0f), vec3f::fill(2.0f));
    }
    
    TEST(VecTest, vec3fMajorComponent) {
        ASSERT_EQ(axis::x, majorComponent(vec3f::pos_x, 0));
        ASSERT_EQ(axis::x, majorComponent(vec3f::neg_x, 0));
        ASSERT_EQ(axis::y, majorComponent(vec3f::pos_y, 0));
        ASSERT_EQ(axis::y, majorComponent(vec3f::neg_y, 0));
        ASSERT_EQ(axis::z, majorComponent(vec3f::pos_z, 0));
        ASSERT_EQ(axis::z, majorComponent(vec3f::neg_z, 0));
        
        ASSERT_EQ(axis::x, majorComponent(vec3f(3.0f, -1.0f, 2.0f), 0));
        ASSERT_EQ(axis::z, majorComponent(vec3f(3.0f, -1.0f, 2.0f), 1));
        ASSERT_EQ(axis::y, majorComponent(vec3f(3.0f, -1.0f, 2.0f), 2));
    }
    
    TEST(VecTest, vec3fMajorAxis) {
        ASSERT_EQ(vec3f::pos_x, majorAxis(vec3f::pos_x, 0));
        ASSERT_EQ(vec3f::neg_x, majorAxis(vec3f::neg_x, 0));
        ASSERT_EQ(vec3f::pos_y, majorAxis(vec3f::pos_y, 0));
        ASSERT_EQ(vec3f::neg_y, majorAxis(vec3f::neg_y, 0));
        ASSERT_EQ(vec3f::pos_z, majorAxis(vec3f::pos_z, 0));
        ASSERT_EQ(vec3f::neg_z, majorAxis(vec3f::neg_z, 0));
    
        ASSERT_EQ(vec3f::pos_x, majorAxis(vec3f(3.0f, -1.0f, 2.0f), 0));
        ASSERT_EQ(vec3f::pos_z, majorAxis(vec3f(3.0f, -1.0f, 2.0f), 1));
        ASSERT_EQ(vec3f::neg_y, majorAxis(vec3f(3.0f, -1.0f, 2.0f), 2));
    }
    
    TEST(VecTest, vec3fAbsMajorAxis) {
        ASSERT_EQ(vec3f::pos_x, absMajorAxis(vec3f::pos_x, 0));
        ASSERT_EQ(vec3f::pos_x, absMajorAxis(vec3f::neg_x, 0));
        ASSERT_EQ(vec3f::pos_y, absMajorAxis(vec3f::pos_y, 0));
        ASSERT_EQ(vec3f::pos_y, absMajorAxis(vec3f::neg_y, 0));
        ASSERT_EQ(vec3f::pos_z, absMajorAxis(vec3f::pos_z, 0));
        ASSERT_EQ(vec3f::pos_z, absMajorAxis(vec3f::neg_z, 0));
        
        ASSERT_EQ(vec3f::pos_x, absMajorAxis(vec3f(3.0f, -1.0f, 2.0f), 0));
        ASSERT_EQ(vec3f::pos_z, absMajorAxis(vec3f(3.0f, -1.0f, 2.0f), 1));
        ASSERT_EQ(vec3f::pos_y, absMajorAxis(vec3f(3.0f, -1.0f, 2.0f), 2));
    }
    
    TEST(VecTest, multiplyScalarWithVec3f) {
        ASSERT_EQ(       vec3f(6.0f, 9.0f, 3.0f),
                  3.0f * vec3f(2.0f, 3.0f, 1.0f));
    }
    
    TEST(VecTest, vec3fCrossProduct) {
        ASSERT_EQ(vec3f::zero, cross(vec3f::zero, vec3f::zero));
        ASSERT_EQ(vec3f::zero, cross(vec3f::zero, vec3f(2.0f, 34.233f, -10003.0002f)));
        ASSERT_EQ(vec3f::pos_z, cross(vec3f::pos_x, vec3f::pos_y));
        ASSERT_VEC_EQ(vec3f(-2735141.499f, 282853.508f, 421.138f), cross(vec3f(12.302f, -0.0017f, 79898.3f),
                                                                         vec3f(2.0f, 34.233f, -10003.0002f)));
    
        const vec3f t1(7.0f, 4.0f, 0.0f);
        const vec3f t2(-2.0f, 22.0f, 0.0f);
    
        const vec3f c1 = normalize(cross(t1, t2));
        const vec3f c2 = normalize(cross(normalize(t1), normalize(t2)));
        ASSERT_VEC_EQ(c1, c2);
    }
    
    TEST(VecTest, measureAngle) {
        ASSERT_FLOAT_EQ(measureAngle(vec3f::pos_x, vec3f::pos_x, vec3f::pos_z), 0.0f);
        ASSERT_FLOAT_EQ(measureAngle(vec3f::pos_y, vec3f::pos_x, vec3f::pos_z), Cf::piOverTwo());
        ASSERT_FLOAT_EQ(measureAngle(vec3f::neg_x, vec3f::pos_x, vec3f::pos_z), Cf::pi());
        ASSERT_FLOAT_EQ(measureAngle(vec3f::neg_y, vec3f::pos_x, vec3f::pos_z), 3.0f * Cf::piOverTwo());
    }
    
    TEST(VecTest, colinear) {
        ASSERT_TRUE(colinear(vec3d::zero, vec3d::zero, vec3d::zero));
        ASSERT_TRUE(colinear(vec3d::one,  vec3d::one,  vec3d::one));
        ASSERT_TRUE(colinear(vec3d(0.0, 0.0, 0.0), vec3d(0.0, 0.0, 1.0), vec3d(0.0, 0.0, 2.0)));
        ASSERT_FALSE(colinear(vec3d(0.0, 0.0, 0.0), vec3d(1.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0)));
        ASSERT_FALSE(colinear(vec3d(0.0, 0.0, 0.0), vec3d(10.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0)));
    }
    
    TEST(VecTest, mix) {
        ASSERT_EQ(vec3d::zero, mix(vec3d::zero, vec3d::one, vec3d::zero));
        ASSERT_EQ(vec3d::one, mix(vec3d::zero, vec3d::one, vec3d::one));
        ASSERT_EQ(vec3d::one / 2.0, mix(vec3d::zero, vec3d::one, vec3d::one / 2.0));
    }
    
    TEST(VecTest, swizzle) {
        ASSERT_EQ(vec3d(2, 3, 1), swizzle(vec3d(1, 2, 3), 0));
        ASSERT_EQ(vec3d(3, 1, 2), swizzle(vec3d(1, 2, 3), 1));
        ASSERT_EQ(vec3d(1, 2, 3), swizzle(vec3d(1, 2, 3), 2));
    }
    
    TEST(VecTest, unswizzle) {
        for (size_t i = 0; i < 3; ++i) {
            ASSERT_EQ(vec3d(1, 2, 3), unswizzle(swizzle(vec3d(1, 2, 3), i), i));
        }
    }
}
