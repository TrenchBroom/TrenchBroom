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

#include "vecmath/utils.h"
#include "vecmath/quat_decl.h"
#include "vecmath/quat_impl.h"
#include "vecmath/vec_decl.h"
#include "TestUtils.h"

TEST(QuatTest, defaultConstructor) {
    const vm::quatf q;
    ASSERT_FLOAT_EQ(0.0f, q.r);
    ASSERT_TRUE(isZero(q.v));
}

TEST(QuatTest, rotationConstructor) {
    const float angle(vm::radians(15.0f));
    const vm::vec3f axis(normalize(vm::vec3f(1.0f, 2.0f, 3.0f)));
    const vm::quatf q(axis, angle);
    
    ASSERT_FLOAT_EQ(std::cos(angle / 2.0f), q.r);
    ASSERT_VEC_EQ(axis * std::sin(angle / 2.0f), q.v);
}

TEST(QuatTest, rotateVecConstructor) {
    const vm::vec3d from(0.0, 1.0, 0.0);
    const vm::vec3d to(1.0, 0.0, 0.0);
    const vm::quatd q(from, to);
    ASSERT_VEC_EQ(to, q * from);
}

TEST(QuatTest, rotateVecConstructor_oppositeVectors) {
    for (size_t i = 0; i < 3; ++i) {
        vm::vec3d from(0.0, 0.0, 0.0);
        vm::vec3d to(0.0, 0.0, 0.0);

        from[i] = 1.0;
        to[i] = -1.0;

        const vm::quatd q(from, to);
        EXPECT_VEC_EQ(to, q * from);

        // The quaternion axis should be perpendicular to both from and to vectors
        EXPECT_DOUBLE_EQ(0.0, dot(q.axis(), from));
        EXPECT_DOUBLE_EQ(0.0, dot(q.axis(), to));
    }
}

TEST(QuatTest, rotateVecConstructor_equalVectors) {
    for (size_t i = 0; i < 3; ++i) {
        vm::vec3d from(0.0, 0.0, 0.0);
        from[i] = 1.0;

        const vm::vec3d to = from;
        const vm::quatd q(from, to);
        EXPECT_VEC_EQ(to, q * from);
    }
}

TEST(QuatTest, negation) {
    const vm::quatf q(vm::vec3f::pos_x, vm::radians(15.0f));
    const vm::quatf nq = -q;
    
    ASSERT_FLOAT_EQ(-(q.r), nq.r);
    ASSERT_VEC_EQ(q.v, nq.v);
}

TEST(QuatTest, scalarRightMultiplication) {
    const vm::quatf q(vm::vec3f::pos_x, vm::radians(15.0f));
    const vm::quatf p = q * 2.0f;
    ASSERT_FLOAT_EQ(q.r * 2.0f, p.r);
}

TEST(QuatTest, scalarLeftMultiplication) {
    const vm::quatf q(vm::vec3f::pos_x, vm::radians(15.0f));
    const vm::quatf p = 2.0f * q;
    ASSERT_FLOAT_EQ(q.r * 2.0f, p.r);
}

TEST(QuatTest, multiplication) {
    const float angle1 = vm::radians(15.0f);
    const vm::quatf q1(vm::vec3f::pos_z, angle1);
    const float angle2 = vm::radians(10.0f);
    const vm::quatf q2(vm::vec3f::pos_z, angle2);
    const vm::quatf q = q1 * q2;
    const vm::vec3f v = vm::vec3f::pos_x;
    const vm::vec3f w = q * v;

    ASSERT_VEC_EQ(vm::vec3f(std::cos(angle1 + angle2), std::sin(angle1 + angle2), 0.0f), w);
}

TEST(QuatTest, vectorMultiplication) {
    const float angle = vm::radians(15.0f);
    const vm::quatf q(vm::vec3f::pos_z, angle);
    const vm::vec3f v = vm::vec3f::pos_x;
    const vm::vec3f w = q * v;
    
    ASSERT_VEC_EQ(vm::vec3f(std::cos(angle), std::sin(angle), 0.0f), w);
}

TEST(QuatTest, angle) {
    const float angle = vm::radians(15.0f);
    const vm::quatf q(vm::vec3f::pos_z, angle);
    
    ASSERT_NEAR(angle, q.angle(), 0.001f);
}

TEST(QuatTest, axis) {
    const vm::vec3f axis = vm::vec3f::pos_z;
    const float angle = vm::radians(15.0f);
    const vm::quatf q(axis, angle);
    
    ASSERT_VEC_EQ(axis, q.axis());
}

TEST(QuatTest, conjugate) {
    const vm::vec3f axis = vm::vec3f::pos_z;
    const float angle = vm::radians(15.0f);
    const vm::quatf q(axis, angle);
    vm::quatf p = q;
    p = p.conjugate();
    
    ASSERT_VEC_EQ(-q.v, p.v);
}

