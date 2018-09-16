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

#include <vecmath/forward.h>
#include <vecmath/quat.h>
#include <vecmath/vec.h>
#include <vecmath/scalar.h>
#include "TestUtils.h"

namespace vm {
    TEST(QuatTest, defaultConstructor) {
        const quatf q;
        ASSERT_FLOAT_EQ(0.0f, q.r);
        ASSERT_TRUE(isZero(q.v, vm::Cf::almostZero()));
    }

    TEST(QuatTest, rotationConstructor) {
        const float angle(toRadians(15.0f));
        const vec3f axis(normalize(vec3f(1.0f, 2.0f, 3.0f)));
        const quatf q(axis, angle);
        
        ASSERT_FLOAT_EQ(std::cos(angle / 2.0f), q.r);
        ASSERT_VEC_EQ(axis * std::sin(angle / 2.0f), q.v);
    }
    
    TEST(QuatTest, rotateVecConstructor) {
        const vec3d from(0.0, 1.0, 0.0);
        const vec3d to(1.0, 0.0, 0.0);
        const quatd q(from, to);
        ASSERT_VEC_EQ(to, q * from);
    }
    
    TEST(QuatTest, rotateVecConstructor_oppositeVectors) {
        for (size_t i = 0; i < 3; ++i) {
            vec3d from(0.0, 0.0, 0.0);
            vec3d to(0.0, 0.0, 0.0);
    
            from[i] = 1.0;
            to[i] = -1.0;
    
            const quatd q(from, to);
            EXPECT_VEC_EQ(to, q * from);
    
            // The quaternion axis should be perpendicular to both from and to vectors
            EXPECT_DOUBLE_EQ(0.0, dot(q.axis(), from));
            EXPECT_DOUBLE_EQ(0.0, dot(q.axis(), to));
        }
    }
    
    TEST(QuatTest, rotateVecConstructor_equalVectors) {
        for (size_t i = 0; i < 3; ++i) {
            vec3d from(0.0, 0.0, 0.0);
            from[i] = 1.0;
    
            const vec3d to = from;
            const quatd q(from, to);
            EXPECT_VEC_EQ(to, q * from);
        }
    }

    TEST(QuatTest, angle) {
        const auto angle = toRadians(15.0f);
        const auto q = quatf(vec3f::pos_z, angle);

        ASSERT_NEAR(angle, q.angle(), 0.001f);
    }

    TEST(QuatTest, axis) {
        ASSERT_VEC_EQ(vec3d::zero, quatd().axis());
        ASSERT_VEC_EQ(vec3d::pos_z, quatd(vec3d::pos_z, toRadians(45.0)).axis());
        ASSERT_VEC_EQ(normalize(vec3d(1, 1, 0)), quatd(normalize(vec3d(1, 1, 0)), toRadians(25.0)).axis());
    }

    TEST(QuatTest, conjugate) {
        const vec3f axis = vec3f::pos_z;
        const float angle = toRadians(15.0f);
        const quatf q(axis, angle);
        quatf p = q;
        p = p.conjugate();

        ASSERT_VEC_EQ(-q.v, p.v);
    }

    TEST(QuatTest, negate) {
        const quatf q(vec3f::pos_x, toRadians(15.0f));
        const quatf nq = -q;
        
        ASSERT_FLOAT_EQ(-(q.r), nq.r);
        ASSERT_VEC_EQ(q.v, nq.v);
    }
    
    TEST(QuatTest, scalarMultiply_right) {
        const quatf q(vec3f::pos_x, toRadians(15.0f));
        const quatf p = q * 2.0f;
        ASSERT_FLOAT_EQ(q.r * 2.0f, p.r);
    }
    
    TEST(QuatTest, scalarMultiply_left) {
        const quatf q(vec3f::pos_x, toRadians(15.0f));
        const quatf p = 2.0f * q;
        ASSERT_FLOAT_EQ(q.r * 2.0f, p.r);
    }
    
    TEST(QuatTest, multiply) {
        const float angle1 = toRadians(15.0f);
        const quatf q1(vec3f::pos_z, angle1);
        const float angle2 = toRadians(10.0f);
        const quatf q2(vec3f::pos_z, angle2);
        const quatf q = q1 * q2;
        const vec3f v = vec3f::pos_x;
        const vec3f w = q * v;
    
        ASSERT_VEC_EQ(vec3f(std::cos(angle1 + angle2), std::sin(angle1 + angle2), 0.0f), w);
    }
    
    TEST(QuatTest, vectorMultiply) {
        const float angle = toRadians(15.0f);
        const quatf q(vec3f::pos_z, angle);
        const vec3f v = vec3f::pos_x;
        const vec3f w = q * v;
        
        ASSERT_VEC_EQ(vec3f(std::cos(angle), std::sin(angle), 0.0f), w);
    }

}
