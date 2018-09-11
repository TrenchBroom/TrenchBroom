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
#include <vecmath/utils.h>
#include "TestUtils.h"

// TODO 2201: write more tests

namespace vm {
    TEST(LineTest, constructDefault) {
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
}
