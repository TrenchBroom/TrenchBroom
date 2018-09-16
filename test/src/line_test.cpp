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

#include <vecmath/line.h>
#include <vecmath/scalar.h>
#include "TestUtils.h"

namespace vm {
    TEST(LineTest, defaultConstructor) {
        const line3f p;
        ASSERT_EQ(vec3f::zero, p.point);
        ASSERT_EQ(vec3f::zero, p.direction);
    }
    
    TEST(LineTest, constructWithPointAndDirection) {
        const vec3f p(10,20,30);
        const vec3f n = normalize(vec3f(1.0f, 2.0f, 3.0f));
        const line3f l(p, n);
        ASSERT_VEC_EQ(p, l.point);
        ASSERT_VEC_EQ(n, l.direction);
    }

    TEST(LineTest, getOrigin) {
        const auto l = line3d(vec3d::one, vec3d::pos_z);
        ASSERT_VEC_EQ(l.point, l.getOrigin());
    }

    TEST(LineTest, getDirection) {
        const auto l = line3d(vec3d::one, vec3d::pos_z);
        ASSERT_VEC_EQ(l.direction, l.getDirection());
    }

    TEST(LineTest, transform) {
        const auto l = line3d(vec3d::one, vec3d::pos_z);
        const auto rm = rotationMatrix(toRadians(15.0), toRadians(20.0), toRadians(-12.0));
        const auto tm = translationMatrix(vec3d::one);

        const auto lt = l.transform(rm * tm);
        ASSERT_TRUE(isUnit(l.direction, vm::Cd::almostZero()));
        ASSERT_VEC_EQ(rm * tm * l.point, lt.point);
        ASSERT_VEC_EQ(rm * l.direction, lt.direction);
    }

    TEST(LineTest, makeCanonical) {
        const auto l1 = line3d(vec3d(-10, 0, 10), vec3d::pos_x);
        const auto l2 = line3d(vec3d(+10, 0, 10), vec3d::pos_x);
        ASSERT_EQ(l1.makeCanonical(), l2.makeCanonical());
    }

    TEST(LineTest, distanceToProjectedPoint) {
        const line3f l(vec3f(10,0,0), vec3f::pos_z);
        ASSERT_FLOAT_EQ(0.0f, l.distanceToProjectedPoint(vec3f(10,0,0)));
        ASSERT_FLOAT_EQ(10.0f, l.distanceToProjectedPoint(vec3f(10,0,10)));
        ASSERT_FLOAT_EQ(10.0f, l.distanceToProjectedPoint(vec3f(10,10,10)));
    }
    
    TEST(LineTest, projectPoint) {
        const line3f l(vec3f(10,0,0), vec3f::pos_z);
        ASSERT_VEC_EQ(vec3f(10,0,5), l.projectPoint(vec3f(100,100,5)));
    }

    TEST(LineTest, isEqual) {
        ASSERT_TRUE(isEqual(line3d(), line3d(), 0.0));
        ASSERT_TRUE(isEqual(line3d(vec3d::zero, vec3d::pos_z), line3d(vec3d::zero, vec3d::pos_z), 0.0));
        ASSERT_FALSE(isEqual(line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)), line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)), 0.0));
        ASSERT_TRUE(isEqual(line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)), line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)), 2.0));
    }

    TEST(LineTest, equal) {
        ASSERT_TRUE(line3d() == line3d());
        ASSERT_TRUE(line3d(vec3d::zero, vec3d::pos_z) == line3d(vec3d::zero, vec3d::pos_z));
        ASSERT_FALSE(line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) == line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)));
    }

    TEST(LineTest, notEqual) {
        ASSERT_FALSE(line3d() != line3d());
        ASSERT_FALSE(line3d(vec3d::zero, vec3d::pos_z) != line3d(vec3d::zero, vec3d::pos_z));
        ASSERT_TRUE(line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) != line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)));
    }
}
