/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Ray.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(RayTest, PointAtDistance) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosX);
    ASSERT_VEC_EQ(Vec3f(5.0f, 0.0f, 0.0f), ray.pointAtDistance(5.0f));
}
