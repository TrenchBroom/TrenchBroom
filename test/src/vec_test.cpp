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
#include <vecmath/vec_ext.h>

#include "TestUtils.h"

#include <limits>
#include <vector>

namespace vm {
    TEST(VecTest, checkStatics) {
        ASSERT_EQ(vec3f(+1,  0,  0), vec3f::pos_x);
        ASSERT_EQ(vec3f( 0, +1,  0), vec3f::pos_y);
        ASSERT_EQ(vec3f( 0,  0, +1), vec3f::pos_z);
        ASSERT_EQ(vec3f(-1,  0,  0), vec3f::neg_x);
        ASSERT_EQ(vec3f( 0, -1,  0), vec3f::neg_y);
        ASSERT_EQ(vec3f( 0,  0, -1), vec3f::neg_z);
        ASSERT_EQ(vec3f( 0,  0,  0), vec3f::zero);
        ASSERT_EQ(vec3f( 1,  1,  1), vec3f::one);

        for (size_t i = 0; i < 3; ++i) {
            ASSERT_FLOAT_EQ(std::numeric_limits<float>::min(), vec3f::min[i]);
            ASSERT_FLOAT_EQ(std::numeric_limits<float>::max(), vec3f::max[i]);
            ASSERT_TRUE(std::isnan(vec3f::NaN[i]));
        }
    }

    TEST(VecTest, axis) {
        ASSERT_EQ(vec3f::pos_x, vec3f::axis(0));
        ASSERT_EQ(vec3f::pos_y, vec3f::axis(1));
        ASSERT_EQ(vec3f::pos_z, vec3f::axis(2));
    }

    TEST(VecTest, fill) {
        ASSERT_EQ(vec3f( 2,  2,  2), vec3f::fill( 2));
        ASSERT_EQ(vec3f( 0,  0,  0), vec3f::fill( 0));
        ASSERT_EQ(vec3f(-2, -2, -2), vec3f::fill(-2));
    }

    TEST(VecTest, parseValidString) {
        ASSERT_EQ(vec3f(1.0f, 3.0f, 3.5f), vec3f::parse("1.0 3 3.5"));
    }
    
    TEST(VecTest, parseShortString) {
        ASSERT_EQ(vec3f(1.0f, 3.0f, 0.0f), vec3f::parse("1.0 3"));
    }

    TEST(VecTest, parseAll) {
        std::vector<vec3f> result;

        vec3f::parseAll("", std::back_inserter(result));
        ASSERT_TRUE(result.empty());

        result.clear();
        vec3f::parseAll("1.0 3 3.5 2.0 2.0 2.0", std::back_inserter(result));
        ASSERT_EQ(std::vector<vec3f>({ vec3f(1, 3, 3.5), vec3f(2, 2, 2) }), result);

        result.clear();
        vec3f::parseAll("(1.0 3 3.5) (2.0 2.0 2.0)", std::back_inserter(result));
        ASSERT_EQ(std::vector<vec3f>({ vec3f(1, 3, 3.5), vec3f(2, 2, 2) }), result);

        result.clear();
        vec3f::parseAll("(1.0 3 3.5), (2.0 2.0 2.0)", std::back_inserter(result));
        ASSERT_EQ(std::vector<vec3f>({ vec3f(1, 3, 3.5), vec3f(2, 2, 2) }), result);

        result.clear();
        vec3f::parseAll("(1.0 3 3.5); (2.0 2.0 2.0)", std::back_inserter(result));
        ASSERT_EQ(std::vector<vec3f>({ vec3f(1, 3, 3.5), vec3f(2, 2, 2) }), result);

        result.clear();
        vec3f::parseAll("1.0 3 3.5, 2.0 2.0 2.0", std::back_inserter(result));
        ASSERT_EQ(std::vector<vec3f>({ vec3f(1, 3, 3.5), vec3f(2, 2, 2) }), result);
    }

    TEST(VecTest, constructDefault) {
        ASSERT_EQ(vec3f::zero, vec3f());
    }

    TEST(VecTest, constructWithInitializerList) {
        ASSERT_EQ(vec3f(1, 2, 3), vec3f({ 1, 2, 3 }));
        ASSERT_EQ(vec3f(1, 2, 3), vec3f({ 1, 2, 3, 4 }));
        ASSERT_EQ(vec3f(1, 2, 0), vec3f({ 1, 2 }));
    }

    TEST(VecTest, constructWithInvalidString) {
        ASSERT_EQ(vec3f::zero, vec3f::parse("asdf"));
    }
    
    TEST(VecTest, constructFrom2Floats) {
        ASSERT_EQ(vec3f(1.0f, 2.0f, 0.0f), vec3f(1.0f, 2.0f));
    }
    
    TEST(VecTest, constructVec3fFrom4Floats) {
        ASSERT_EQ(vec3f(1.0f, 2.0f, 3.0f), vec3f(1.0f, 2.0f, 3.0f, 4.0f));
    }
    
    TEST(VecTest, constructVec4fFrom3Floats) {
        ASSERT_EQ(vec4f(1.0f, 2.0f, 3.0f, 0.0f), vec4f(1.0f, 2.0f, 3.0f));
    }
    
    TEST(VecTest, constructvec2fFromVec2f) {
        const vec2f v(2.0f, 3.0f);
        ASSERT_EQ(v, vec2f(v));
    }
    
    TEST(VecTest, constructVec2fFromVec3f) {
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

    /* ========== comparison operators ========== */

    TEST(VecTest, compare) {
        ASSERT_EQ( 0, compare(vec3f::zero, vec3f::zero));

        ASSERT_EQ(-1, compare(vec3f::zero, vec3f::one));
        ASSERT_EQ(-1, compare(vec3f::one,  vec3f(2, 1, 1)));
        ASSERT_EQ(-1, compare(vec3f::one,  vec3f(1, 2, 1)));
        ASSERT_EQ(-1, compare(vec3f::one,  vec3f(1, 1, 2)));
        ASSERT_EQ(-1, compare(vec3f::one,  vec3f(2, 0, 0)));
        ASSERT_EQ(-1, compare(vec3f::one,  vec3f(1, 2, 0)));

        ASSERT_EQ(+1, compare(vec3f::one,     vec3f::zero));
        ASSERT_EQ(+1, compare(vec3f(2, 1, 1), vec3f::one));
        ASSERT_EQ(+1, compare(vec3f(1, 2, 1), vec3f::one));
        ASSERT_EQ(+1, compare(vec3f(1, 1, 2), vec3f::one));
        ASSERT_EQ(+1, compare(vec3f(2, 0, 0), vec3f::one));
        ASSERT_EQ(+1, compare(vec3f(1, 2, 0), vec3f::one));
    }

    TEST(VecTest, compareRanges) {
        const auto r1 = std::vector<vec3f>{ vec3f(1, 2, 3), vec3f(1, 2, 3) };
        const auto r2 = std::vector<vec3f>{ vec3f(1, 2, 3), vec3f(2, 2, 3) };
        const auto r3 = std::vector<vec3f>{ vec3f(2, 2, 3) };

        // same length
        ASSERT_EQ( 0, compare(std::begin(r1), std::end(r1), std::begin(r1), std::end(r1)));
        ASSERT_EQ(-1, compare(std::begin(r1), std::end(r1), std::begin(r2), std::end(r2)));
        ASSERT_EQ(+1, compare(std::begin(r2), std::end(r2), std::begin(r1), std::end(r1)));

        // prefix
        ASSERT_EQ(-1, compare(std::begin(r1), std::next(std::begin(r1)), std::begin(r1), std::end(r1)));
        ASSERT_EQ(+1, compare(std::begin(r1), std::end(r1), std::begin(r1), std::next(std::begin(r1))));

        // different length and not prefix
        ASSERT_EQ(-1, compare(std::begin(r1), std::end(r1), std::begin(r3), std::end(r3)));
        ASSERT_EQ(+1, compare(std::begin(r3), std::end(r3), std::begin(r1), std::end(r1)));
    }

    TEST(VecTest, isEqual) {
        ASSERT_TRUE(isEqual(vec2f::zero, vec2f::zero, 0.0f));
        ASSERT_FALSE(isEqual(vec2f::zero, vec2f::one, 0.0f));
        ASSERT_TRUE(isEqual(vec2f::zero, vec2f::one, 2.0f));
    }

    TEST(VecTest, equal) {
        ASSERT_FALSE(vec3f(1, 2, 3) == vec3f(2, 2, 2));
        ASSERT_TRUE (vec3f(1, 2, 3) == vec3f(1, 2, 3));
        ASSERT_FALSE(vec3f(1, 2, 4) == vec3f(1, 2, 2));
    }

    TEST(VecTest, notEqual) {
        ASSERT_TRUE (vec3f(1, 2, 3) != vec3f(2, 2, 2));
        ASSERT_FALSE(vec3f(1, 2, 3) != vec3f(1, 2, 3));
        ASSERT_TRUE (vec3f(1, 2, 4) != vec3f(1, 2, 2));
    }

    TEST(VecTest, lessThan) {
        ASSERT_TRUE (vec3f(1, 2, 3) < vec3f(2, 2, 2));
        ASSERT_FALSE(vec3f(1, 2, 3) < vec3f(1, 2, 3));
        ASSERT_FALSE(vec3f(1, 2, 4) < vec3f(1, 2, 2));
    }

    TEST(VecTest, lessThanOrEqual) {
        ASSERT_TRUE (vec3f(1, 2, 3) <= vec3f(2, 2, 2));
        ASSERT_TRUE (vec3f(1, 2, 3) <= vec3f(1, 2, 3));
        ASSERT_FALSE(vec3f(1, 2, 4) <= vec3f(1, 2, 2));
    }

    TEST(VecTest, greaterThan) {
        ASSERT_FALSE(vec3f(1, 2, 3) > vec3f(2, 2, 2));
        ASSERT_FALSE(vec3f(1, 2, 3) > vec3f(1, 2, 3));
        ASSERT_TRUE (vec3f(1, 2, 4) > vec3f(1, 2, 2));
    }

    TEST(VecTest, greaterThanOrEqual) {
        ASSERT_FALSE(vec3f(1, 2, 3) >= vec3f(2, 2, 2));
        ASSERT_TRUE (vec3f(1, 2, 3) >= vec3f(1, 2, 3));
        ASSERT_TRUE (vec3f(1, 2, 4) >= vec3f(1, 2, 2));
    }

    /* ========== accessing major component / axis ========== */

    TEST(VecTest, majorComponent) {
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

    TEST(VecTest, majorAxis) {
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

    TEST(VecTest, absMajorAxis) {
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

    TEST(VecTest, firstComponent) {
        ASSERT_EQ(axis::x, firstComponent(vec3f::pos_x));
        ASSERT_EQ(axis::x, firstComponent(vec3f::neg_x));
        ASSERT_EQ(axis::y, firstComponent(vec3f::pos_y));
        ASSERT_EQ(axis::y, firstComponent(vec3f::neg_y));
        ASSERT_EQ(axis::z, firstComponent(vec3f::pos_z));
        ASSERT_EQ(axis::z, firstComponent(vec3f::neg_z));

        ASSERT_EQ(axis::x, firstComponent(vec3f(3.0f, -1.0f, 2.0f)));
    }

    TEST(VecTest, secondComponent) {
        ASSERT_EQ(axis::z, secondComponent(vec3f(3.0f, -1.0f, 2.0f)));
    }

    TEST(VecTest, thirdComponent) {
        ASSERT_EQ(axis::y, thirdComponent(vec3f(3.0f, -1.0f, 2.0f)));
    }

    TEST(VecTest, firstAxis) {
        ASSERT_EQ(vec3f::pos_x, firstAxis(vec3f::pos_x));
        ASSERT_EQ(vec3f::neg_x, firstAxis(vec3f::neg_x));
        ASSERT_EQ(vec3f::pos_y, firstAxis(vec3f::pos_y));
        ASSERT_EQ(vec3f::neg_y, firstAxis(vec3f::neg_y));
        ASSERT_EQ(vec3f::pos_z, firstAxis(vec3f::pos_z));
        ASSERT_EQ(vec3f::neg_z, firstAxis(vec3f::neg_z));

        ASSERT_EQ(vec3f::pos_x, firstAxis(vec3f(3.0f, -1.0f, 2.0f)));
    }

    TEST(VecTest, secondAxis) {
        ASSERT_EQ(vec3f::pos_z, secondAxis(vec3f(3.0f, -1.0f, 2.0f)));
    }

    TEST(VecTest, thirdAxis) {
        ASSERT_EQ(vec3f::neg_y, thirdAxis(vec3f(3.0f, -1.0f, 2.0f)));
    }

    /* ========== arithmetic operators ========== */

    TEST(VecTest, negate) {
        ASSERT_EQ( vec3f(-1.0f, +2.0f, -3.0f),
                  -vec3f(+1.0f, -2.0f, +3.0f));
    }

    TEST(VecTest, plus) {
        ASSERT_EQ(vec3f(4.0f, 4.0f, 4.0f),
                  vec3f(1.0f, 2.0f, 3.0f) +
                  vec3f(3.0f, 2.0f, 1.0f));
    }

    TEST(VecTest, subtract) {
        ASSERT_EQ(vec3f(1.0f, 1.0f, -1.0f),
                  vec3f(2.0f, 3.0f,  1.0f) -
                  vec3f(1.0f, 2.0f,  2.0f));
    }

    TEST(VecTest, multiply) {
        ASSERT_EQ(vec3f(2.0f, 6.0f, -2.0f),
                  vec3f(2.0f, 3.0f, -1.0f) *
                  vec3f(1.0f, 2.0f,  2.0f));
    }

    TEST(VecTest, scalarMultiply) {
        ASSERT_EQ(vec3f(6.0f, 9.0f, 3.0f),
                  vec3f(2.0f, 3.0f, 1.0f) * 3.0f);
        ASSERT_EQ(       vec3f(6.0f, 9.0f, 3.0f),
                  3.0f * vec3f(2.0f, 3.0f, 1.0f));
    }

    TEST(VecTest, division) {
        ASSERT_EQ(vec3f(2.0f,  6.0f, -2.0f),
                  vec3f(2.0f, 12.0f,  2.0f) /
                  vec3f(1.0f,  2.0f, -1.0f));
    }

    TEST(VecTest, scalarDivision) {
        ASSERT_EQ(vec3f(1.0f, 18.0f, 2.0f),
                  vec3f(2.0f, 36.0f, 4.0f) / 2.0f);
        ASSERT_EQ(       vec3f(4.0f, 1.0f, -2.0f),
                  8.0f / vec3f(2.0f, 8.0f, -4.0f));
    }

    /* ========== arithmetic functions ========== */

    TEST(VecTest, min) {
        ASSERT_EQ(vec3f(+2, +2, +2), min(vec3f(+2, +2, +2), vec3f(+3, +3, +3)));
        ASSERT_EQ(vec3f(-2, -2, -2), min(vec3f(-2, -2, -2), vec3f(-1, -1, -1)));
        ASSERT_EQ(vec3f(+1, +2, +1), min(vec3f(+2, +2, +2), vec3f(+1, +3, +1)));
        ASSERT_EQ(vec3f(-2, -3, -2), min(vec3f(-2, -2, -2), vec3f(-1, -3, -1)));
    }

    TEST(VecTest, max) {
        ASSERT_EQ(vec3f(+3, +3, +3), max(vec3f(+2, +2, +2), vec3f(+3, +3, +3)));
        ASSERT_EQ(vec3f(-1, -1, -1), max(vec3f(-2, -2, -2), vec3f(-1, -1, -1)));
        ASSERT_EQ(vec3f(+2, +3, +2), max(vec3f(+2, +2, +2), vec3f(+1, +3, +1)));
        ASSERT_EQ(vec3f(-1, -2, -1), max(vec3f(-2, -2, -2), vec3f(-1, -3, -1)));
    }

    TEST(VecTest, absMin) {
        ASSERT_EQ(vec3f(+2, +2, +2), absMin(vec3f(+2, +2, +2), vec3f(+3, +3, +3)));
        ASSERT_EQ(vec3f(-1, -1, -1), absMin(vec3f(-2, -2, -2), vec3f(-1, -1, -1)));
        ASSERT_EQ(vec3f(+1, +2, +1), absMin(vec3f(+2, +2, +2), vec3f(+1, +3, +1)));
        ASSERT_EQ(vec3f(-1, -2, -1), absMin(vec3f(-2, -2, -2), vec3f(-1, -3, -1)));
    }

    TEST(VecTest, absMax) {
        ASSERT_EQ(vec3f(+3, +3, +3), absMax(vec3f(+2, +2, +2), vec3f(+3, +3, +3)));
        ASSERT_EQ(vec3f(-2, -2, -2), absMax(vec3f(-2, -2, -2), vec3f(-1, -1, -1)));
        ASSERT_EQ(vec3f(+2, +3, +2), absMax(vec3f(+2, +2, +2), vec3f(+1, +3, +1)));
        ASSERT_EQ(vec3f(-2, -3, -2), absMax(vec3f(-2, -2, -2), vec3f(-1, -3, -1)));
    }

    TEST(VecTest, abs) {
        ASSERT_EQ(vec3f(1, 2, 3), abs(vec3f(1, -2, -3)));
        ASSERT_EQ(vec3f(0, 2, 3), abs(vec3f(0, -2, -3)));
    }

    TEST(VecTest, dot) {
        ASSERT_FLOAT_EQ(-748013.6097f, dot(vec3f(2.3f, 8.7878f, -2323.0f), vec3f(4.333f, -2.0f, 322.0f)));
        ASSERT_FLOAT_EQ(0.0f, dot(vec3f(2.3f, 8.7878f, -2323.0f), vec3f::zero));
    }

    TEST(VecTest, cross) {
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

    TEST(VecTest, squaredLength) {
        ASSERT_FLOAT_EQ(0.0f, squaredLength(vec3f::zero));
        ASSERT_FLOAT_EQ(1.0f, squaredLength(vec3f::pos_x));
        ASSERT_FLOAT_EQ(5396411.51542884f, squaredLength(vec3f(2.3f, 8.7878f, -2323.0f)));
    }

    TEST(VecTest, length) {
        ASSERT_FLOAT_EQ(0.0f, length(vec3f::zero));
        ASSERT_FLOAT_EQ(1.0f, length(vec3f::pos_x));
        ASSERT_FLOAT_EQ(std::sqrt(5396411.51542884f), length(vec3f(2.3f, 8.7878f, -2323.0f)));
    }

    TEST(VecTest, normalize) {
        ASSERT_EQ(vec3f::pos_x, normalize(vec3f::pos_x));
        ASSERT_EQ(vec3f::neg_x, normalize(vec3f::neg_x));

        const vec3f v1(2.3f, 8.7878f, -2323.0f);
        const vec3f v2(4.333f, -2.0f, 322.0f);
        ASSERT_VEC_EQ((v1 / length(v1)), normalize(v1));
        ASSERT_VEC_EQ((v2 / length(v2)), normalize(v2));
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

    TEST(VecTest, isUnit) {
        ASSERT_TRUE(isUnit(vec3f::pos_x));
        ASSERT_TRUE(isUnit(vec3f::pos_y));
        ASSERT_TRUE(isUnit(vec3f::pos_z));
        ASSERT_TRUE(isUnit(vec3f::neg_x));
        ASSERT_TRUE(isUnit(vec3f::neg_y));
        ASSERT_TRUE(isUnit(vec3f::neg_z));
        ASSERT_TRUE(isUnit(normalize(vec3f::one)));
        ASSERT_FALSE(isUnit(vec3f::one));
        ASSERT_FALSE(isUnit(vec3f::zero));
    }

    TEST(VecTest, isZero) {
        ASSERT_TRUE(isZero(vec3f::zero));
        ASSERT_FALSE(isZero(vec3f::pos_x));
    }

    TEST(VecTest, isNaN) {
        ASSERT_TRUE(isNaN(vec3f::NaN));
        ASSERT_FALSE(isNaN(vec3f::pos_x));
    }

    TEST(VecTest, isIntegral) {
        ASSERT_TRUE(isIntegral(vec3f::pos_x));
        ASSERT_TRUE(isIntegral(vec3f::pos_y));
        ASSERT_TRUE(isIntegral(vec3f::pos_z));
        ASSERT_TRUE(isIntegral(vec3f::neg_x));
        ASSERT_TRUE(isIntegral(vec3f::neg_y));
        ASSERT_TRUE(isIntegral(vec3f::neg_z));
        ASSERT_TRUE(isIntegral(vec3f::one));
        ASSERT_TRUE(isIntegral(vec3f::zero));
        ASSERT_FALSE(isIntegral(normalize(vec3f::one)));
    }

    TEST(VecTest, mix) {
        ASSERT_EQ(vec3d::zero, mix(vec3d::zero, vec3d::one, vec3d::zero));
        ASSERT_EQ(vec3d::one, mix(vec3d::zero, vec3d::one, vec3d::one));
        ASSERT_EQ(vec3d::one / 2.0, mix(vec3d::zero, vec3d::one, vec3d::one / 2.0));
    }

    TEST(VecTest, squaredDistance) {
        const vec3f v1(2.3f, 8.7878f, -2323.0f);
        const vec3f v2(4.333f, -2.0f, 322.0f);
        ASSERT_FLOAT_EQ(0.0f, squaredDistance(v1, v1));
        ASSERT_FLOAT_EQ(squaredLength(v1), squaredDistance(v1, vec3f::zero));
        ASSERT_FLOAT_EQ(squaredLength(v1 - v2), squaredDistance(v1, v2));
    }

    TEST(VecTest, distance) {
        const vec3f v1(2.3f, 8.7878f, -2323.0f);
        const vec3f v2(4.333f, -2.0f, 322.0f);
        ASSERT_FLOAT_EQ(0.0f, distance(v1, v1));
        ASSERT_FLOAT_EQ(length(v1), distance(v1, vec3f::zero));
        ASSERT_FLOAT_EQ(length(v1 - v2), distance(v1, v2));
    }

    TEST(VecTest, toHomogeneousCoordinates) {
        ASSERT_EQ(vec4f(1, 2, 3, 1), toHomogeneousCoords(vec3f(1, 2, 3)));
    }

    TEST(VecTest, toCartesianCoords) {
        vec4f v(2.0f, 4.0f, 8.0f, 2.0f);
        ASSERT_EQ(vec3f(1.0f, 2.0f, 4.0f), toCartesianCoords(v));
    }

    TEST(VecTest, colinear) {
        ASSERT_TRUE(colinear(vec3d::zero, vec3d::zero, vec3d::zero));
        ASSERT_TRUE(colinear(vec3d::one,  vec3d::one,  vec3d::one));
        ASSERT_TRUE(colinear(vec3d(0.0, 0.0, 0.0), vec3d(0.0, 0.0, 1.0), vec3d(0.0, 0.0, 2.0)));
        ASSERT_FALSE(colinear(vec3d(0.0, 0.0, 0.0), vec3d(1.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0)));
        ASSERT_FALSE(colinear(vec3d(0.0, 0.0, 0.0), vec3d(10.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0)));
    }

    TEST(VecTest, parallel) {
        ASSERT_FALSE(parallel(vec3f::zero, vec3f::zero));
        ASSERT_TRUE(parallel(vec3f::pos_x, vec3f::pos_x));
        ASSERT_TRUE(parallel(vec3f::pos_x, vec3f::neg_x));
        ASSERT_TRUE(parallel(vec3f::one, vec3f::one));
        ASSERT_TRUE(parallel(vec3f::one, normalize(vec3f::one)));
    }

    /* ========== rounding and error correction ========== */

    TEST(VecTest, round) {
        ASSERT_EQ(vec3f::pos_x, round(vec3f::pos_x));
        ASSERT_EQ(vec3f::one, round(vec3f::one));
        ASSERT_EQ(vec3f::zero, round(vec3f::zero));
        ASSERT_EQ(vec3f::one, round(normalize(vec3f::one)));
        ASSERT_EQ(vec3f::zero, round(vec3f(0.4, 0.4, 0.4)));
        ASSERT_EQ(vec3f(0, 1, 0), round(vec3f(0.4, 0.5, 0.4)));
        ASSERT_EQ(vec3f(0, -1, 0), round(vec3f(-0.4, -0.5, -0.4)));
    }

    TEST(VecTest, snapDown) {
        ASSERT_EQ( vec3f::zero, snapDown(vec3f::zero, vec3f::one));
        ASSERT_EQ( vec3f::zero, snapDown(vec3f(+0.4, +0.5, +0.6), vec3f::one));
        ASSERT_EQ( vec3f::zero, snapDown(vec3f(-0.4, -0.5, -0.6), vec3f::one));
        ASSERT_EQ(+vec3f::one,  snapDown(vec3f(+1.4, +1.5, +1.6), vec3f::one));
        ASSERT_EQ(-vec3f::one,  snapDown(vec3f(-1.4, -1.5, -1.6), vec3f::one));
        ASSERT_EQ( vec3f::zero, snapDown(vec3f(+1.4, +1.5, +1.6), vec3f(2, 2, 2)));
        ASSERT_EQ( vec3f::zero, snapDown(vec3f(-1.4, -1.5, -1.6), vec3f(2, 2, 2)));
        ASSERT_EQ( vec3f(0, +1, +1), snapDown(vec3f(+1.4, +1.5, +1.6), vec3f(2, 1, 1)));
        ASSERT_EQ( vec3f(0, -1, -1), snapDown(vec3f(-1.4, -1.5, -1.6), vec3f(2, 1, 1)));
    }

    TEST(VecTest, snapUp) {
        ASSERT_EQ( vec3f::zero, snapUp(vec3f::zero, vec3f::one));
        ASSERT_EQ(+vec3f::one,  snapUp(vec3f(+0.4, +0.5, +0.6), vec3f::one));
        ASSERT_EQ(-vec3f::one,  snapUp(vec3f(-0.4, -0.5, -0.6), vec3f::one));
        ASSERT_EQ(+vec3f(+2, +2, +2), snapUp(vec3f(+1.4, +1.5, +1.6), vec3f::one));
        ASSERT_EQ(-vec3f(+2, +2, +2), snapUp(vec3f(-1.4, -1.5, -1.6), vec3f::one));
        ASSERT_EQ( vec3f(+3, +3, +3), snapUp(vec3f(+1.4, +1.5, +1.6), vec3f(3, 3, 3)));
        ASSERT_EQ( vec3f(-3, -3, -3), snapUp(vec3f(-1.4, -1.5, -1.6), vec3f(3, 3, 3)));
        ASSERT_EQ( vec3f(+3, +2, +2), snapUp(vec3f(+1.4, +1.5, +1.6), vec3f(3, 1, 1)));
        ASSERT_EQ( vec3f(-3, -2, -2), snapUp(vec3f(-1.4, -1.5, -1.6), vec3f(3, 1, 1)));
    }

    TEST(VecTest, snap) {
        ASSERT_EQ(vec2f( 8.0f,  0.0f), snap(vec2f( 7.0f, -3.0f), vec2f( 4.0f, 12.0f)));
        ASSERT_EQ(vec2f( 8.0f, -6.0f), snap(vec2f( 7.0f, -5.0f), vec2f(-4.0f, -2.0f)));
        ASSERT_EQ(vec2f(-8.0f,  6.0f), snap(vec2f(-7.0f,  5.0f), vec2f(-4.0f, -2.0f)));
    }

    TEST(VecTest, correct) {
        ASSERT_EQ(vec3f(1.1, 2.2, 3.3), correct(vec3f(1.1, 2.2, 3.3)));
        ASSERT_EQ(vec3f(1, 2, 3), correct(vec3f(1.1, 2.2, 3.3), 0, 0.4f));
        ASSERT_EQ(vec3f(1.1, 2.2, 3.3), correct(vec3f(1.1, 2.2, 3.3), 1, 0.4f));
    }

    TEST(VecTest, between) {
        ASSERT_TRUE(between(vec3f(1, 0, 0), vec3f(0, 0, 0), vec3f(2, 0, 0)));
        ASSERT_TRUE(between(vec3f(1, 0, 0), vec3f(2, 0, 0), vec3f(0, 0, 0)));
        ASSERT_TRUE(between(vec3f(1, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
        ASSERT_TRUE(between(vec3f(0, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
        ASSERT_FALSE(between(vec3f(2, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
    }

    TEST(VecTest, average) {
        const auto vecs = std::vector<vec3f>{ vec3f(1, 1, 1), vec3f(1, 1, 1), vec3f(2, 2, 2) };
        ASSERT_EQ(vec3f(4.0 / 3.0, 4.0 / 3.0, 4.0 / 3.0), average(std::begin(vecs), std::end(vecs)));
    }

    TEST(VecTest, measureAngle) {
        ASSERT_FLOAT_EQ(measureAngle(vec3f::pos_x, vec3f::pos_x, vec3f::pos_z), 0.0f);
        ASSERT_FLOAT_EQ(measureAngle(vec3f::pos_y, vec3f::pos_x, vec3f::pos_z), Cf::piOverTwo());
        ASSERT_FLOAT_EQ(measureAngle(vec3f::neg_x, vec3f::pos_x, vec3f::pos_z), Cf::pi());
        ASSERT_FLOAT_EQ(measureAngle(vec3f::neg_y, vec3f::pos_x, vec3f::pos_z), 3.0f * Cf::piOverTwo());
    }

    /* ========== operations on vectors of vectors ========== */

    TEST(VecTest, addList) {
        const auto in  = std::vector<vec3f>{ vec3f(1, 2, 3), vec3f(2, 3, 4) };
        const auto exp = std::vector<vec3f>{ vec3f(0, 3, 1), vec3f(1, 4, 2) };
        ASSERT_EQ(exp, in + vec3f(-1, +1, -2));
        ASSERT_EQ(exp, vec3f(-1, +1, -2) + in);
    }

    TEST(VecTest, scalarMultiplyList) {
        const auto in  = std::vector<vec3f>{ vec3f(1, 2, 3), vec3f(2, 3, 4) };
        const auto exp = std::vector<vec3f>{ vec3f(3, 6, 9), vec3f(6, 9, 12) };
        ASSERT_EQ(exp, in * 3.0f);
        ASSERT_EQ(exp, 3.0f * in);
    }
}
