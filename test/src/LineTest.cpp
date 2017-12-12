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

#include "Line.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(LineTest, constructDefault) {
    const Line3f p;
    ASSERT_EQ(Vec3f::Null, p.point);
    ASSERT_EQ(Vec3f::Null, p.direction);
}

TEST(LineTest, constructWithPointAndDirection) {
    const Vec3f p(10,20,30);
    const Vec3f n = Vec3f(1.0f, 2.0f, 3.0f).normalized();
    const Line3f l(p, n);
    ASSERT_VEC_EQ(p, l.point);
    ASSERT_VEC_EQ(n, l.direction);
}

TEST(LineTest, pointOnLineClosestToPoint) {
    const Line3f l(Vec3f(10,0,0), Vec3::PosZ);
    ASSERT_VEC_EQ(Vec3f(10,0,5), l.pointOnLineClosestToPoint(Vec3f(100,100,5)));
}
