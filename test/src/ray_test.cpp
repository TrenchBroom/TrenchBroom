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

#include <vecmath/ray.h>
#include <vecmath/scalar.h>
#include <vecmath/util.h>

#include "TestUtils.h"

// TODO 2201: write more tests

namespace vm {
    TEST(RayTest, defaultConstructor) {
        const auto r = ray3d();
        ASSERT_VEC_EQ(vec3d::zero, r.origin);
        ASSERT_VEC_EQ(vec3d::zero, r.direction);
    }

    TEST(RayTest, constructWithOriginAndDirection) {
        const auto r = ray3d(vec3d::one, vec3d::pos_z);
        ASSERT_VEC_EQ(vec3d::one, r.origin);
        ASSERT_VEC_EQ(vec3d::pos_z, r.direction);
    }

    TEST(RayTest, getOrigin) {
        const auto r = ray3d(vec3d::one, vec3d::pos_z);
        ASSERT_VEC_EQ(r.origin, r.getOrigin());
    }

    TEST(RayTest, getDirection) {
        const auto r = ray3d(vec3d::one, vec3d::pos_z);
        ASSERT_VEC_EQ(r.direction, r.getDirection());
    }

    TEST(RayTest, transform) {
        const auto r = ray3d(vec3d::one, vec3d::pos_z);
        const auto rm = rotationMatrix(radians(15.0), radians(20.0), radians(-12.0));
        const auto tm = translationMatrix(vec3d::one);

        const auto rt = r.transform(rm * tm);
        ASSERT_TRUE(isUnit(r.direction));
        ASSERT_VEC_EQ(r.origin * rm * tm, rt.origin);
        ASSERT_VEC_EQ(r.direction * rm, rt.direction);
    }

    TEST(RayTest, pointStatus) {
        const ray3f ray(vec3f::zero, vec3f::pos_z);
        ASSERT_EQ(point_status::above, ray.pointStatus(vec3f(0.0f, 0.0f, 1.0f)));
        ASSERT_EQ(point_status::inside, ray.pointStatus(vec3f(0.0f, 0.0f, 0.0f)));
        ASSERT_EQ(point_status::below, ray.pointStatus(vec3f(0.0f, 0.0f, -1.0f)));
    }

    TEST(RayTest, isEqual) {
        ASSERT_TRUE(isEqual(ray3d(), ray3d(), 0.0));
        ASSERT_TRUE(isEqual(ray3d(vec3d::zero, vec3d::pos_z), ray3d(vec3d::zero, vec3d::pos_z), 0.0));
        ASSERT_FALSE(isEqual(ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)), ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)), 0.0));
        ASSERT_TRUE(isEqual(ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)), ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)), 2.0));
    }

    TEST(RayTest, equal) {
        ASSERT_TRUE(ray3d() == ray3d());
        ASSERT_TRUE(ray3d(vec3d::zero, vec3d::pos_z) == ray3d(vec3d::zero, vec3d::pos_z));
        ASSERT_FALSE(ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) == ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)));
    }

    TEST(RayTest, notEqual) {
        ASSERT_FALSE(ray3d() != ray3d());
        ASSERT_FALSE(ray3d(vec3d::zero, vec3d::pos_z) != ray3d(vec3d::zero, vec3d::pos_z));
        ASSERT_TRUE(ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) != ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)));
    }

    TEST(RayTest, pointAtDistance) {
        const ray3f ray(vec3f::zero, vec3f::pos_x);
        ASSERT_VEC_EQ(vec3f(5.0f, 0.0f, 0.0f), ray.pointAtDistance(5.0f));
    }
}
