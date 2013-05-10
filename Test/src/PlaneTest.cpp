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

#include "Vec.h"
#include "Plane.h"
#include "MathUtilities.h"
#include "TestUtilities.h"

TEST(PlaneTest, ConstructDefault) {
    const Plane3f p;
    ASSERT_EQ(0.0f, p.distance);
    ASSERT_EQ(Vec3f::Null, p.normal);
}

TEST(PlaneTest, ConstructWithDistanceAndNormal) {
    const float d = 123.0f;
    const Vec3f n = Vec3f(1.0f, 2.0f, 3.0f).normalized();
    const Plane3f p(d, n);
    ASSERT_FLOAT_EQ(d, p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, ConstructWithAnchorAndNormal) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    ASSERT_FLOAT_EQ(a.dot(n), p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, ConstructPlaneContainingVector) {
}

TEST(PlaneTest, Anchor) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    ASSERT_VEC_EQ(p.distance * n, p.anchor());
}

TEST(PlaneTest, IntersectWithRay) {
}

TEST(PlaneTest, IntersectWithLine) {
}

TEST(PlaneTest, PointStatus) {
    const Plane3f p(10.0f, Vec3f::PosZ);
    ASSERT_EQ(PointStatus::PSAbove, p.pointStatus(Vec3f(0.0f, 0.0f, 11.0f)));
    ASSERT_EQ(PointStatus::PSBelow, p.pointStatus(Vec3f(0.0f, 0.0f, 9.0f)));
    ASSERT_EQ(PointStatus::PSInside, p.pointStatus(Vec3f(0.0f, 0.0f, 10.0f)));
}

TEST(PlaneTest, PointDistance) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    const Vec3f point(1.0f, -32.37873f, 32.0f);
    ASSERT_EQ(point.dot(p.normal) - p.distance, p.pointDistance(point));
}

TEST(PlaneTest, ValueAtParallelPlanes) {
    const Plane3f p1(10.0f, Vec3f::PosX);
    
    ASSERT_FLOAT_EQ(p1.distance, p1.at(Vec2f(2.0f, 1.0f), Axis::AX));
    ASSERT_FLOAT_EQ(p1.distance, p1.at(Vec2f(22.0f, -34322.0232f), Axis::AX));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(2.0f, 1.0f), Axis::AY));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(22.0f, -34322.0232f), Axis::AY));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(2.0f, 1.0f), Axis::AZ));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(22.0f, -34322.0232f), Axis::AZ));
}

TEST(PlaneTest, ValueAt) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    const Vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.y() - point1.y() * p.normal.z()) / p.normal[Axis::AX],
                    p.at(point1, Axis::AX));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.z()) / p.normal[Axis::AY],
                    p.at(point1, Axis::AY));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.y()) / p.normal[Axis::AZ],
                    p.at(point1, Axis::AZ));
}

TEST(PlaneTest, XYZValueAt) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    const Vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ(p.at(point1, Axis::AX), p.xAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, Axis::AY), p.yAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, Axis::AZ), p.zAt(point1));
}

TEST(PlaneTest, Equals) {
    ASSERT_TRUE(Plane3f(0.0f, Vec3f::PosX).equals(Plane3f(0.0f, Vec3f::PosX)));
    ASSERT_TRUE(Plane3f(0.0f, Vec3f::PosY).equals(Plane3f(0.0f, Vec3f::PosY)));
    ASSERT_TRUE(Plane3f(0.0f, Vec3f::PosZ).equals(Plane3f(0.0f, Vec3f::PosZ)));
    ASSERT_FALSE(Plane3f(0.0f, Vec3f::PosX).equals(Plane3f(0.0f, Vec3f::NegX)));
    ASSERT_FALSE(Plane3f(0.0f, Vec3f::PosX).equals(Plane3f(0.0f, Vec3f::PosY)));
}

TEST(PlaneTest, Transform) {
}

TEST(PlaneTest, Transformed) {
}

TEST(PlaneTest, Rotate) {
}

TEST(PlaneTest, Rotated) {
}

TEST(PlaneTest, Project) {
    ASSERT_VEC_EQ(Vec3f(1.0f, 2.0f, 0.0f), Plane3f(0.0f, Vec3f::PosZ).project(Vec3f(1.0f, 2.0f, 3.0f)));
}

TEST(PlaneTest, HorizontalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Plane3f p = horizontalDragPlane(position);
    ASSERT_TRUE(p.pointStatus(position) == PointStatus::PSInside);
    ASSERT_VEC_EQ(Vec3f::PosZ, p.normal);
}

TEST(PlaneTest, VerticalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 3.0f, -2.0f).normalize();
    const Plane3f p = verticalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == PointStatus::PSInside);
    ASSERT_VEC_EQ(Vec3f::PosY, p.normal);
}

TEST(PlaneTest, VerticalDragPlaneWithZDirection) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 2.0f, -3.0f).normalize();
    const Plane3f p = verticalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == PointStatus::PSInside);
    ASSERT_VEC_EQ(Vec3f::PosY, p.normal);
}

TEST(PlaneTest, OrthogonalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 2.0f, -3.0f).normalize();
    const Plane3f p = orthogonalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == PointStatus::PSInside);
    ASSERT_VEC_EQ(direction, p.normal);
}

TEST(PlaneTest, AlignedOrthogonalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 2.0f, -3.0f).normalize();
    const Plane3f p = alignedOrthogonalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == PointStatus::PSInside);
    ASSERT_VEC_EQ(direction.firstAxis(), p.normal);
}
