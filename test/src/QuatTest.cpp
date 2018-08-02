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

#include "MathUtils.h"
#include "Quat.h"
#include "Vec.h"
#include "TestUtils.h"

TEST(QuatTest, defaultConstructor) {
    const Quatf q;
    ASSERT_FLOAT_EQ(0.0f, q.r);
    ASSERT_TRUE(isNull(q.v));
}

TEST(QuatTest, rotationConstructor) {
    const float angle(Math::radians(15.0f));
    const Vec3f axis(normalize(Vec<float,3>(1.0f, 2.0f, 3.0f)));
    const Quatf q(axis, angle);
    
    ASSERT_FLOAT_EQ(std::cos(angle / 2.0f), q.r);
    ASSERT_VEC_EQ(axis * std::sin(angle / 2.0f), q.v);
}

TEST(QuatTest, rotateVecConstructor) {
    const Vec3d from(0.0, 1.0, 0.0);
    const Vec3d to(1.0, 0.0, 0.0);
    const Quatd q(from, to);
    ASSERT_VEC_EQ(to, q * from);
}

TEST(QuatTest, negation) {
    const Quatf q(Vec<float,3>::PosX, Math::radians(15.0f));
    const Quatf nq = -q;
    
    ASSERT_FLOAT_EQ(-(q.r), nq.r);
    ASSERT_VEC_EQ(q.v, nq.v);
}

TEST(QuatTest, scalarRightMultiplication) {
    const Quatf q(Vec<float,3>::PosX, Math::radians(15.0f));
    const Quatf p = q * 2.0f;
    ASSERT_FLOAT_EQ(q.r * 2.0f, p.r);
}

TEST(QuatTest, scalarLeftMultiplication) {
    const Quatf q(Vec<float,3>::PosX, Math::radians(15.0f));
    const Quatf p = 2.0f * q;
    ASSERT_FLOAT_EQ(q.r * 2.0f, p.r);
}

TEST(QuatTest, scalarRightMultiplicationAndAssign) {
    const Quatf q(Vec<float,3>::PosX, Math::radians(15.0f));
    Quatf p = q;
    p *= 2.0f;
    ASSERT_FLOAT_EQ(q.r * 2.0f, p.r);
}

TEST(QuatTest, multiplication) {
    const float angle1 = Math::radians(15.0f);
    const Quatf q1(Vec<float,3>::PosZ, angle1);
    const float angle2 = Math::radians(10.0f);
    const Quatf q2(Vec<float,3>::PosZ, angle2);
    const Quatf q = q1 * q2;
    const Vec3f v = Vec3f::PosX;
    const Vec3f w = q * v;

    ASSERT_VEC_EQ(Vec3f(std::cos(angle1 + angle2), std::sin(angle1 + angle2), 0.0f), w);
}

TEST(QuatTest, multiplicationAndAssign) {
    const float angle1 = Math::radians(15.0f);
    Quatf q1(Vec<float,3>::PosZ, angle1);
    const float angle2 = Math::radians(10.0f);
    const Quatf q2(Vec<float,3>::PosZ, angle2);
    q1 *= q2;
    const Vec3f v = Vec3f::PosX;
    const Vec3f w = q1 * v;
    
    ASSERT_VEC_EQ(Vec3f(std::cos(angle1 + angle2), std::sin(angle1 + angle2), 0.0f), w);
}

TEST(QuatTest, vectorMultiplication) {
    const float angle = Math::radians(15.0f);
    const Quatf q(Vec<float,3>::PosZ, angle);
    const Vec3f v = Vec3f::PosX;
    const Vec3f w = q * v;
    
    ASSERT_VEC_EQ(Vec3f(std::cos(angle), std::sin(angle), 0.0f), w);
}

TEST(QuatTest, angle) {
    const float angle = Math::radians(15.0f);
    const Quatf q(Vec<float,3>::PosZ, angle);
    
    ASSERT_NEAR(angle, q.angle(), 0.001f);
}

TEST(QuatTest, axis) {
    const Vec3f axis = Vec<float,3>::PosZ;
    const float angle = Math::radians(15.0f);
    const Quatf q(axis, angle);
    
    ASSERT_VEC_EQ(axis, q.axis());
}

TEST(QuatTest, conjugate) {
    const Vec3f axis = Vec<float,3>::PosZ;
    const float angle = Math::radians(15.0f);
    const Quatf q(axis, angle);
    Quatf p = q;
    p.conjugate();
    
    ASSERT_VEC_EQ(-q.v, p.v);
}

TEST(QuatTest, conjugated) {
    const Vec3f axis = Vec<float,3>::PosZ;
    const float angle = Math::radians(15.0f);
    const Quatf q(axis, angle);
    const Quatf p = q.conjugated();
    
    ASSERT_VEC_EQ(-q.v, p.v);
}
