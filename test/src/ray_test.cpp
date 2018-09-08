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

#include "ray_decl.h"
#include "ray_impl.h"
#include "utils.h"
#include "TestUtils.h"

TEST(RayTest, pointAtDistance) {
    const vm::ray3f ray(vm::vec3f::zero, vm::vec3f::pos_x);
    ASSERT_VEC_EQ(vm::vec3f(5.0f, 0.0f, 0.0f), ray.pointAtDistance(5.0f));
}

TEST(RayTest, pointStatus) {
    const vm::ray3f ray(vm::vec3f::zero, vm::vec3f::pos_z);
    ASSERT_EQ(vm::point_status::above, ray.pointStatus(vm::vec3f(0.0f, 0.0f, 1.0f)));
    ASSERT_EQ(vm::point_status::inside, ray.pointStatus(vm::vec3f(0.0f, 0.0f, 0.0f)));
    ASSERT_EQ(vm::point_status::below, ray.pointStatus(vm::vec3f(0.0f, 0.0f, -1.0f)));
}
