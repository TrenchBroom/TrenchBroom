/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Vec.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(VecTest, parseVec3fWithValidString) {
    ASSERT_EQ(Vec3f(1.0f, 3.0f, 3.5f), Vec3f::parse("1.0 3 3.5"));
}

TEST(VecTest, parseVec3fWithShortString) {
    ASSERT_EQ(Vec3f(1.0f, 3.0f, 0.0f), Vec3f::parse("1.0 3"));
}

TEST(VecTest, constructVec3fWithInvalidString) {
    ASSERT_EQ(Vec3f::Null, Vec3f::parse("asdf"));
}

TEST(VecTest, constructVec3fFrom1Float) {
    ASSERT_EQ(Vec3f(1.0f, 0.0f, 0.0f), Vec3f::create(1.0f));
}

TEST(VecTest, constructVec3fFrom2Floats) {
    ASSERT_EQ(Vec3f(1.0f, 2.0f, 0.0f), Vec3f(1.0f, 2.0f));
}

TEST(VecTest, constructVec3fFrom4Floats) {
    ASSERT_EQ(Vec3f(1.0f, 2.0f, 3.0f), Vec3f(1.0f, 2.0f, 3.0f, 4.0f));
}

TEST(VecTest, constructVec4fFrom3Floats) {
    ASSERT_EQ(Vec4f(1.0f, 2.0f, 3.0f, 0.0f), Vec4f(1.0f, 2.0f, 3.0f));
}

TEST(VecTest, constructVec2fFromVec2f) {
    const Vec2f v(2.0f, 3.0f);
    ASSERT_EQ(v, Vec2f(v));
}

TEST(VecTest, constructVec2fFromVec3f) {
    const Vec3f v(3.0f, 5.0f, 78.0f);
    ASSERT_EQ(Vec2f(v[0], v[1]), Vec2f(v));
}

TEST(VecTest, constructVec2fFromVec4f) {
    const Vec4f v(3.0f, 5.0f, 2.0f, 7.0f);
    ASSERT_EQ(Vec2f(v[0], v[1]), Vec2f(v));
}

TEST(VecTest, constructVec3fFromVec2f) {
    const Vec2f v(2.0f, 3.0f);
    ASSERT_EQ(Vec3f(v[0], v[1], 0.0f), Vec3f(v));
}

TEST(VecTest, constructVec4fFromVec2f) {
    const Vec2f v(2.0f, 3.0f);
    ASSERT_EQ(Vec4f(v[0], v[1], 0.0f, 0.0f), Vec4f(v));
}

TEST(VecTest, constructVec4fFromVec2fWithLast1) {
    const Vec2f v(3.0f, 5.0f);
    ASSERT_EQ(Vec4f(v[0], v[1], 0.0f, 2.0f), Vec4f(v, 2.0f));
}

TEST(VecTest, constructVec4fFromVec2fWithLast2) {
    const Vec2f v(3.0f, 5.0f);
    ASSERT_EQ(Vec4f(v[0], v[1], 3.0f, 2.0f), Vec4f(v, 3.0f, 2.0f));
}

TEST(VecTest, constructVec3fFromVec3fWithLast1) {
    const Vec3f v(3.0f, 5.0f, 8.0f);
    ASSERT_EQ(Vec3f(v[0], v[1], 2.0f), Vec3f(v, 2.0f));
}

TEST(VecTest, constructVec3fFromVec3fWithLast2) {
    const Vec3f v(3.0f, 5.0f, 8.0f);
    ASSERT_EQ(Vec3f(v[0], 2.0f, 4.0f), Vec3f(v, 2.0f, 4.0f));
}

TEST(VecTest, assignVec2fToVec3f) {
    const Vec2f t(2.0f, 3.0f);
    Vec3f v;
    ASSERT_EQ(Vec3f(t), (v = t));
}

TEST(VecTest, assignVec3fToVec3f) {
    const Vec3f t(2.0f, 3.0f, 5.0f);
    Vec3f v;
    ASSERT_EQ(Vec3f(t), (v = t));
}

TEST(VecTest, assignVec4fToVec3f) {
    const Vec4f t(2.0f, 3.0f, 5.0f, 6.0f);
    Vec3f v;
    ASSERT_EQ(Vec3f(t), (v = t));
}

TEST(VecTest, invertVec3f) {
    ASSERT_EQ( Vec3f(-1.0f, -2.0f, -3.0f),
              -Vec3f( 1.0f,  2.0f,  3.0f));
}

TEST(VecTest, addVec3f) {
    ASSERT_EQ(Vec3f(4.0f, 4.0f, 4.0f),
              Vec3f(1.0f, 2.0f, 3.0f) +
              Vec3f(3.0f, 2.0f, 1.0f));
}

TEST(VecTest, subtractVec3f) {
    ASSERT_EQ(Vec3f(1.0f, 1.0f, -1.0f),
              Vec3f(2.0f, 3.0f, 1.0f) -
              Vec3f(1.0f, 2.0f, 2.0f));
}

TEST(VecTest, multiplyVec3fWithScalar) {
    ASSERT_EQ(Vec3f(6.0f, 9.0f, 3.0f),
              Vec3f(2.0f, 3.0f, 1.0f) * 3.0f);
}

TEST(VecTest, divideVec3fByScalar) {
    ASSERT_EQ(Vec3f(1.0f, 18.0f, 2.0f),
              Vec3f(2.0f, 36.0f, 4.0f) / 2.0f);
}

TEST(VecTest, addVec3fAndAssign) {
    Vec3f v(1.0f, 2.0f, 3.0f);
    ASSERT_EQ(Vec3f(4.0f, 4.0f, 4.0f), (v += Vec3f(3.0f, 2.0f, 1.0f)));
}

TEST(VecTest, substractVec3fAndAssign) {
    Vec3f v(2.0f, 3.0f, 1.0f);
    ASSERT_EQ(Vec3f(1.0f, 1.0f, -1.0f), (v -= Vec3f(1.0f, 2.0f, 2.0f)));
}

TEST(VecTest, multiplyVec3fWithScalarAndAssign) {
    Vec3f v(2.0f, 3.0f, 1.0f);
    ASSERT_EQ(Vec3f(6.0f, 9.0f, 3.0f), (v *= 3.0f));
}

TEST(VecTest, divideVec3fByScalarAndAssign) {
    Vec3f v(2.0f, 36.0f, 4.0f);
    ASSERT_EQ(Vec3f(1.0f, 18.0f, 2.0f), (v /= 2.0f));
}

TEST(VecTest, subscriptAccess) {
    Vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
    ASSERT_EQ(1.0f, v[0]);
    ASSERT_EQ(2.0f, v[1]);
    ASSERT_EQ(3.0f, v[2]);
    ASSERT_EQ(4.0f, v[3]);
}

TEST(VecTest, accessors) {
    Vec4f v(1.0f, 2.0f, 3.0f, 4.0f);
    ASSERT_EQ(v[0], v.x());
    ASSERT_EQ(v[1], v.y());
    ASSERT_EQ(v[2], v.z());
    ASSERT_EQ(v[3], v.w());
    ASSERT_EQ(Vec2f(1.0f, 2.0f), v.xy());
    ASSERT_EQ(Vec3f(1.0f, 2.0f, 3.0f), v.xyz());
    ASSERT_EQ(v, v.xyzw());
}

TEST(VecTest, vec4fOverLast) {
    Vec4f v(2.0f, 4.0f, 8.0f, 2.0f);
    ASSERT_EQ(Vec3f(1.0f, 2.0f, 4.0f), v.overLast());
}

TEST(VecTest, vec2RoundToMultiple) {
    ASSERT_EQ(Vec2f(8.0f,  0.0f), Vec2f( 7.0f, -3.0f).roundToMultiple(Vec2f( 4.0f, 12.0f)));
    ASSERT_EQ(Vec2f( 8.0f, -6.0f), Vec2f( 7.0f, -5.0f).roundToMultiple(Vec2f(-4.0f, -2.0f)));
    ASSERT_EQ(Vec2f(-8.0f,  6.0f), Vec2f(-7.0f,  5.0f).roundToMultiple(Vec2f(-4.0f, -2.0f)));
}

TEST(VecTest, vec3fDot) {
    ASSERT_FLOAT_EQ(-748013.6097f, Vec3f(2.3f, 8.7878f, -2323.0f).dot(Vec3f(4.333f, -2.0f, 322.0f)));
}

TEST(VecTest, vec3fDotNull) {
    ASSERT_FLOAT_EQ(0.0f, Vec3f(2.3f, 8.7878f, -2323.0f).dot(Vec3f::Null));
}

TEST(VecTest, vec3fLength) {
    ASSERT_FLOAT_EQ(0.0f, Vec3f::Null.length());
    ASSERT_FLOAT_EQ(1.0f, Vec3f::PosX.length());
    ASSERT_FLOAT_EQ(std::sqrt(5396411.51542884f), Vec3f(2.3f, 8.7878f, -2323.0f).length());
}

TEST(VecTest, vec3fLengthSquared) {
    ASSERT_FLOAT_EQ(0.0f, Vec3f::Null.squaredLength());
    ASSERT_FLOAT_EQ(1.0f, Vec3f::PosX.squaredLength());
    ASSERT_FLOAT_EQ(5396411.51542884f, Vec3f(2.3f, 8.7878f, -2323.0f).squaredLength());
}

TEST(VecTest, vec3fDistanceTo) {
    const Vec3f v1(2.3f, 8.7878f, -2323.0f);
    const Vec3f v2(4.333f, -2.0f, 322.0f);
    ASSERT_FLOAT_EQ(0.0f, v1.distanceTo(v1));
    ASSERT_FLOAT_EQ(v1.length(), v1.distanceTo(Vec3f::Null));
    ASSERT_FLOAT_EQ((v1 - v2).length(), v1.distanceTo(v2));
}

TEST(VecTest, vec3fSquaredDistanceTo) {
    const Vec3f v1(2.3f, 8.7878f, -2323.0f);
    const Vec3f v2(4.333f, -2.0f, 322.0f);
    ASSERT_FLOAT_EQ(0.0f, v1.squaredDistanceTo(v1));
    ASSERT_FLOAT_EQ(v1.squaredLength(), v1.squaredDistanceTo(Vec3f::Null));
    ASSERT_FLOAT_EQ((v1 - v2).squaredLength(), v1.squaredDistanceTo(v2));
}

TEST(VecTest, vec3fNormalize) {
    ASSERT_EQ(Vec3f::PosX, Vec3f::PosX.normalized());
    ASSERT_EQ(Vec3f::NegX, Vec3f::NegX.normalized());
    
    const Vec3f v1(2.3f, 8.7878f, -2323.0f);
    const Vec3f v2(4.333f, -2.0f, 322.0f);
    ASSERT_VEC_EQ((v1 / v1.length()), v1.normalized());
    ASSERT_VEC_EQ((v2 / v2.length()), v2.normalized());
}

TEST(VecTest, vec3fNull) {
    ASSERT_TRUE(Vec3f::Null.null());
    ASSERT_FALSE(Vec3f::PosX.null());
}

TEST(VecTest, vec3fSetNull) {
    Vec3f v(1.0f, 2.0f, 3.0f);
    v.setNull();
    ASSERT_VEC_EQ(Vec3f::Null, v);
}

TEST(VecTest, vec3fSetSingleValue) {
    Vec3f v(1.0f, 2.0f, 3.0f);
    v.set(7.0f);
    ASSERT_VEC_EQ(Vec3f(7.0f, 7.0f, 7.0f), v);
}

TEST(VecTest, vec3fParallelTo) {
    ASSERT_TRUE(Vec3f::PosX.parallelTo(Vec3f::PosX));
    ASSERT_TRUE(Vec3f::PosX.parallelTo(Vec3f::NegX));
    ASSERT_FALSE(Vec3f::PosX.parallelTo(Vec3f::PosY));
    ASSERT_TRUE(Vec3f::PosZ.parallelTo(Vec3f(0.1f, -0.02f, 100.0f)));
    ASSERT_TRUE(Vec3d(-1.0, 0.0, 0.0).parallelTo(Vec3d(3.0, 0.0, 0.0)));
    ASSERT_TRUE(Vec3d(-1.0, 2.0, 42.0).parallelTo(-3.2253 * Vec3d(-1.0, 2.0, 42.0)));
    ASSERT_FALSE(Vec3d(-1.0, 2.0, 21.0).parallelTo(Vec3d(-1.0, 5.0, 21.0)));
    ASSERT_FALSE(Vec3d(0.0, 256.0, 0.0).parallelTo(Vec3d(0.0, 256.0, 32.0)));
    
    // precision problems
    // ASSERT_FALSE(Vec3d(0.0, -288.0, 32.0).parallelTo(Vec3d(0.0, 448.0, -32.0)));
}

TEST(VecTest, vec3fMajorComponent) {
    ASSERT_EQ(Math::Axis::AX, Vec3f::PosX.majorComponent(0));
    ASSERT_EQ(Math::Axis::AX, Vec3f::NegX.majorComponent(0));
    ASSERT_EQ(Math::Axis::AY, Vec3f::PosY.majorComponent(0));
    ASSERT_EQ(Math::Axis::AY, Vec3f::NegY.majorComponent(0));
    ASSERT_EQ(Math::Axis::AZ, Vec3f::PosZ.majorComponent(0));
    ASSERT_EQ(Math::Axis::AZ, Vec3f::NegZ.majorComponent(0));
    
    ASSERT_EQ(Math::Axis::AX, Vec3f(3.0f, -1.0f, 2.0f).majorComponent(0));
    ASSERT_EQ(Math::Axis::AZ, Vec3f(3.0f, -1.0f, 2.0f).majorComponent(1));
    ASSERT_EQ(Math::Axis::AY, Vec3f(3.0f, -1.0f, 2.0f).majorComponent(2));
}

TEST(VecTest, vec3fMajorAxis) {
    ASSERT_EQ(Vec3f::PosX, Vec3f::PosX.majorAxis(0));
    ASSERT_EQ(Vec3f::NegX, Vec3f::NegX.majorAxis(0));
    ASSERT_EQ(Vec3f::PosY, Vec3f::PosY.majorAxis(0));
    ASSERT_EQ(Vec3f::NegY, Vec3f::NegY.majorAxis(0));
    ASSERT_EQ(Vec3f::PosZ, Vec3f::PosZ.majorAxis(0));
    ASSERT_EQ(Vec3f::NegZ, Vec3f::NegZ.majorAxis(0));

    ASSERT_EQ(Vec3f::PosX, Vec3f(3.0f, -1.0f, 2.0f).majorAxis(0));
    ASSERT_EQ(Vec3f::PosZ, Vec3f(3.0f, -1.0f, 2.0f).majorAxis(1));
    ASSERT_EQ(Vec3f::NegY, Vec3f(3.0f, -1.0f, 2.0f).majorAxis(2));
}

TEST(VecTest, vec3fAbsMajorAxis) {
    ASSERT_EQ(Vec3f::PosX, Vec3f::PosX.absMajorAxis(0));
    ASSERT_EQ(Vec3f::PosX, Vec3f::NegX.absMajorAxis(0));
    ASSERT_EQ(Vec3f::PosY, Vec3f::PosY.absMajorAxis(0));
    ASSERT_EQ(Vec3f::PosY, Vec3f::NegY.absMajorAxis(0));
    ASSERT_EQ(Vec3f::PosZ, Vec3f::PosZ.absMajorAxis(0));
    ASSERT_EQ(Vec3f::PosZ, Vec3f::NegZ.absMajorAxis(0));
    
    ASSERT_EQ(Vec3f::PosX, Vec3f(3.0f, -1.0f, 2.0f).absMajorAxis(0));
    ASSERT_EQ(Vec3f::PosZ, Vec3f(3.0f, -1.0f, 2.0f).absMajorAxis(1));
    ASSERT_EQ(Vec3f::PosY, Vec3f(3.0f, -1.0f, 2.0f).absMajorAxis(2));
}

TEST(VecTest, multiplyScalarWithVec3f) {
    ASSERT_EQ(       Vec3f(6.0f, 9.0f, 3.0f),
              3.0f * Vec3f(2.0f, 3.0f, 1.0f));
}

TEST(VecTest, vec3fCrossProduct) {
    ASSERT_EQ(Vec3f::Null, crossed(Vec3f::Null, Vec3f::Null));
    ASSERT_EQ(Vec3f::Null, crossed(Vec3f::Null, Vec3f(2.0f, 34.233f, -10003.0002f)));
    ASSERT_EQ(Vec3f::PosZ, crossed(Vec3f::PosX, Vec3f::PosY));
    ASSERT_VEC_EQ(Vec3f(-2735141.499f, 282853.508f, 421.138f), crossed(Vec3f(12.302f, -0.0017f, 79898.3f),
                                                                       Vec3f(2.0f, 34.233f, -10003.0002f)));

    const Vec3f t1(7.0f, 4.0f, 0.0f);
    const Vec3f t2(-2.0f, 22.0f, 0.0f);

    const Vec3f c1 = crossed(t1, t2).normalized();
    const Vec3f c2 = crossed(t1.normalized(), t2.normalized()).normalized();
    ASSERT_VEC_EQ(c1, c2);
}

TEST(VecTest, angleBetween) {
    ASSERT_FLOAT_EQ(angleBetween(Vec3f::PosX, Vec3f::PosX, Vec3f::PosZ), 0.0f);
    ASSERT_FLOAT_EQ(angleBetween(Vec3f::PosY, Vec3f::PosX, Vec3f::PosZ), Math::Cf::piOverTwo());
    ASSERT_FLOAT_EQ(angleBetween(Vec3f::NegX, Vec3f::PosX, Vec3f::PosZ), Math::Cf::pi());
    ASSERT_FLOAT_EQ(angleBetween(Vec3f::NegY, Vec3f::PosX, Vec3f::PosZ), 3.0f * Math::Cf::piOverTwo());
}

TEST(VecTest, convexHull2dSimple) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(8.0, 8.0, 0.0);
    const Vec3d p3(8.0, 0.0, 0.0);
    const Vec3d p4(0.0, 8.0, 0.0);
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    
    const Vec3d::List hull = convexHull2D<double>(points);
    ASSERT_EQ(4u, hull.size());
    ASSERT_VEC_EQ(p3, hull[0]);
    ASSERT_VEC_EQ(p2, hull[1]);
    ASSERT_VEC_EQ(p4, hull[2]);
    ASSERT_VEC_EQ(p1, hull[3]);
}

TEST(VecTest, convexHull2dSimpleWithInternalPoint) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(8.0, 8.0, 0.0);
    const Vec3d p3(8.0, 0.0, 0.0);
    const Vec3d p4(0.0, 8.0, 0.0);
    const Vec3d p5(4.0, 4.0, 0.0);
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    
    const Vec3d::List hull = convexHull2D<double>(points);
    ASSERT_EQ(4u, hull.size());
    ASSERT_VEC_EQ(p3, hull[0]);
    ASSERT_VEC_EQ(p2, hull[1]);
    ASSERT_VEC_EQ(p4, hull[2]);
    ASSERT_VEC_EQ(p1, hull[3]);
}

TEST(VecTest, convexHull2dSimpleWithPointOnLine) {
    const Vec3d p1(0.0, 0.0, 0.0);
    const Vec3d p2(8.0, 8.0, 0.0);
    const Vec3d p3(8.0, 0.0, 0.0);
    const Vec3d p4(0.0, 8.0, 0.0);
    const Vec3d p5(4.0, 0.0, 0.0);
    
    Vec3d::List points;
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);
    
    const Vec3d::List hull = convexHull2D<double>(points);
    ASSERT_EQ(4u, hull.size());
    ASSERT_VEC_EQ(p3, hull[0]);
    ASSERT_VEC_EQ(p2, hull[1]);
    ASSERT_VEC_EQ(p4, hull[2]);
    ASSERT_VEC_EQ(p1, hull[3]);
}
