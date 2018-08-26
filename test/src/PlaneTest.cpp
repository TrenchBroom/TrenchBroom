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

#include "vec_type.h"
#include "Plane.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(PlaneTest, constructDefault) {
    const Plane3f p;
    ASSERT_EQ(0.0f, p.distance);
    ASSERT_EQ(vec3f::zero, p.normal);
}

TEST(PlaneTest, constructWithDistanceAndNormal) {
    const float d = 123.0f;
    const vec3f n = normalize(vec3f(1.0f, 2.0f, 3.0f));
    const Plane3f p(d, n);
    ASSERT_FLOAT_EQ(d, p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, constructWithAnchorAndNormal) {
    const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
    const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
    const Plane3f p(a, n);
    ASSERT_FLOAT_EQ(dot(a, n), p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, constructPlaneContainingVector) {
}

TEST(PlaneTest, anchor) {
    const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
    const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
    const Plane3f p(a, n);
    ASSERT_VEC_EQ(p.distance * n, p.anchor());
}

TEST(PlaneTest, intersectWithRay) {
}

TEST(PlaneTest, intersectWithLine) {
    const Plane3f p(5.0f, vec3f::pos_z);
    const Line3f l(vec3f(0, 0, 15), normalize(vec3f(1,0,-1)));
    
    const vec3f intersection = l.pointAtDistance(p.intersectWithLine(l));
    ASSERT_FLOAT_EQ(10, intersection.x());
    ASSERT_FLOAT_EQ(0, intersection.y());
    ASSERT_FLOAT_EQ(5, intersection.z());
}

TEST(PlaneTest, intersectWithPlane_parallel) {
    const Plane3f p1(10.0f, vec3f::pos_z);
    const Plane3f p2(11.0f, vec3f::pos_z);
    const Line3f line = p1.intersectWithPlane(p2);
    
    ASSERT_EQ(vec3f::zero, line.direction);
    ASSERT_EQ(vec3f::zero, line.point);
}

TEST(PlaneTest, intersectWithPlane_too_similar) {
    const vec3f anchor(100,100,100);
    const Plane3f p1(anchor, vec3f::pos_x);
    const Plane3f p2(anchor, Quatf(vec3f::neg_y, Math::radians(0.0001f)) * vec3f::pos_x); // p1 rotated by 0.0001 degrees
    const Line3f line = p1.intersectWithPlane(p2);
    
    ASSERT_EQ(vec3f::zero, line.direction);
    ASSERT_EQ(vec3f::zero, line.point);
}

static bool lineOnPlane(const Plane3f& plane, const Line3f& line) {
    if (plane.pointStatus(line.point) != Math::PointStatus::PSInside)
        return false;
    if (plane.pointStatus(line.pointAtDistance(16.0f)) != Math::PointStatus::PSInside)
        return false;
    return true;
}

TEST(PlaneTest, intersectWithPlane) {
    const Plane3f p1(10.0f, vec3f::pos_z);
    const Plane3f p2(20.0f, vec3f::pos_x);
    const Line3f line = p1.intersectWithPlane(p2);
    
    ASSERT_TRUE(lineOnPlane(p1, line));
    ASSERT_TRUE(lineOnPlane(p2, line));
}

TEST(PlaneTest, intersectWithPlane_similar) {
    const vec3f anchor(100,100,100);
    const Plane3f p1(anchor, vec3f::pos_x);
    const Plane3f p2(anchor, Quatf(vec3f::neg_y, Math::radians(0.5f)) * vec3f::pos_x); // p1 rotated by 0.5 degrees
    const Line3f line = p1.intersectWithPlane(p2);

    ASSERT_TRUE(lineOnPlane(p1, line));
    ASSERT_TRUE(lineOnPlane(p2, line));
}

TEST(PlaneTest, pointStatus) {
    const Plane3f p(10.0f, vec3f::pos_z);
    ASSERT_EQ(Math::PointStatus::PSAbove, p.pointStatus(vec3f(0.0f, 0.0f, 11.0f)));
    ASSERT_EQ(Math::PointStatus::PSBelow, p.pointStatus(vec3f(0.0f, 0.0f, 9.0f)));
    ASSERT_EQ(Math::PointStatus::PSInside, p.pointStatus(vec3f(0.0f, 0.0f, 10.0f)));
}

TEST(PlaneTest, pointDistance) {
    const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
    const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
    const Plane3f p(a, n);
    const vec3f point(1.0f, -32.37873f, 32.0f);
    ASSERT_EQ(dot(point, p.normal) - p.distance, p.pointDistance(point));
}

TEST(PlaneTest, valueAtParallelPlanes) {
    const Plane3f p1(10.0f, vec3f::pos_x);
    
    ASSERT_FLOAT_EQ(p1.distance, p1.at(vec2f(2.0f, 1.0f), Math::Axis::AX));
    ASSERT_FLOAT_EQ(p1.distance, p1.at(vec2f(22.0f, -34322.0232f), Math::Axis::AX));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(2.0f, 1.0f), Math::Axis::AY));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(22.0f, -34322.0232f), Math::Axis::AY));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(2.0f, 1.0f), Math::Axis::AZ));
    ASSERT_FLOAT_EQ(0.0f, p1.at(vec2f(22.0f, -34322.0232f), Math::Axis::AZ));
}

TEST(PlaneTest, valueAt) {
    const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
    const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
    const Plane3f p(a, n);
    const vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.y() - point1.y() * p.normal.z()) / p.normal[Math::Axis::AX],
                    p.at(point1, Math::Axis::AX));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.z()) / p.normal[Math::Axis::AY],
                    p.at(point1, Math::Axis::AY));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.y()) / p.normal[Math::Axis::AZ],
                    p.at(point1, Math::Axis::AZ));
}

TEST(PlaneTest, xYZValueAt) {
    const vec3f a = vec3f(-2038.034f, 0.0023f, 32.0f);
    const vec3f n = normalize(vec3f(9.734f, -3.393f, 2.033f));
    const Plane3f p(a, n);
    const vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ(p.at(point1, Math::Axis::AX), p.xAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, Math::Axis::AY), p.yAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, Math::Axis::AZ), p.zAt(point1));
}

TEST(PlaneTest, equals) {
    ASSERT_TRUE(Plane3f(0.0f, vec3f::pos_x).equals(Plane3f(0.0f, vec3f::pos_x)));
    ASSERT_TRUE(Plane3f(0.0f, vec3f::pos_y).equals(Plane3f(0.0f, vec3f::pos_y)));
    ASSERT_TRUE(Plane3f(0.0f, vec3f::pos_z).equals(Plane3f(0.0f, vec3f::pos_z)));
    ASSERT_FALSE(Plane3f(0.0f, vec3f::pos_x).equals(Plane3f(0.0f, vec3f::neg_x)));
    ASSERT_FALSE(Plane3f(0.0f, vec3f::pos_x).equals(Plane3f(0.0f, vec3f::pos_y)));
}

TEST(PlaneTest, transform) {
}

TEST(PlaneTest, transformed) {
}

TEST(PlaneTest, rotate) {
}

TEST(PlaneTest, rotated) {
}

TEST(PlaneTest, project) {
    ASSERT_VEC_EQ(vec3f(1.0f, 2.0f, 0.0f), Plane3f(0.0f, vec3f::pos_z).projectPoint(vec3f(1.0f, 2.0f, 3.0f)));
    ASSERT_VEC_EQ(vec3f(1.0f, 2.0f, 2.0f), Plane3f(2.0f, vec3f::pos_z).projectPoint(vec3f(1.0f, 2.0f, 3.0f)));
}

TEST(PlaneTest, setPlanePoints) {
    Plane3f plane;
    vec3f points[3];
    const float epsilon = Math::Constants<float>::pointStatusEpsilon();
    
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(0.0f, 1.0f, 0.0f);
    points[2] = vec3f(1.0f, 0.0f, 0.0f);
    ASSERT_TRUE(setPlanePoints(plane, points));
    ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);

    // right angle, short vectors
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(0.0f, epsilon, 0.0f);
    points[2] = vec3f(epsilon, 0.0f, 0.0f);
    ASSERT_TRUE(setPlanePoints(plane, points));
    ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);

    // plane point vectors at a 45 degree angle, short vectors
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(epsilon, epsilon, 0.0f);
    points[2] = vec3f(epsilon, 0.0f, 0.0f);
    ASSERT_TRUE(setPlanePoints(plane, points));
    ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);
    
    // horizontal plane at z=length units above the origin
    points[0] = vec3f(0.0f, 0.0f, epsilon);
    points[1] = vec3f(0.0f, epsilon, epsilon);
    points[2] = vec3f(epsilon, 0.0f, epsilon);
    ASSERT_TRUE(setPlanePoints(plane, points));
    ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(epsilon, plane.distance);
    
    // small angle (triangle 1000 units wide, length units tall)
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(1000.0f, epsilon, 0.0f);
    points[2] = vec3f(1000.0f, 0.0f, 0.0f);
    ASSERT_TRUE(setPlanePoints(plane, points));
    ASSERT_VEC_EQ(vec3f::pos_z, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);
    
    // small angle
    points[0] = vec3f(224.0f, -400.0f, 1648.0f);
    points[1] = vec3f(304.0f, -432.0f, 1248.0f + epsilon);
    points[2] = vec3f(304.0f, -432.0f, 1248.0f);
    ASSERT_TRUE(setPlanePoints(plane, points));
    ASSERT_FLOAT_EQ(1.0f, length(plane.normal));
    
    // too-small angle (triangle 1000 units wide, length/100 units tall)
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(1000.0f, epsilon/100.0f, 0.0f);
    points[2] = vec3f(1000.0f, 0.0f, 0.0f);
    ASSERT_FALSE(setPlanePoints(plane, points));
    
    // all zero
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(0.0f, 0.0f, 0.0f);
    points[2] = vec3f(0.0f, 0.0f, 0.0f);
    ASSERT_FALSE(setPlanePoints(plane, points));
    
    // same direction, short vectors
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(2*epsilon, 0.0f, 0.0f);
    points[2] = vec3f(epsilon, 0.0f, 0.0f);
    ASSERT_FALSE(setPlanePoints(plane, points));
    
    // opposite, short vectors
    points[0] = vec3f(0.0f, 0.0f, 0.0f);
    points[1] = vec3f(-epsilon, 0.0f, 0.0f);
    points[2] = vec3f(epsilon, 0.0f, 0.0f);
    ASSERT_FALSE(setPlanePoints(plane, points));
}

TEST(PlaneTest, horizontalDragPlane) {
    const vec3f position(322.0f, -122.2392f, 34.0f);
    const Plane3f p = horizontalDragPlane(position);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(vec3f::pos_z, p.normal);
}

TEST(PlaneTest, verticalDragPlane) {
    const vec3f position(322.0f, -122.2392f, 34.0f);
    const vec3f direction = normalize(vec3f(1.0f, 3.0f, -2.0f));
    const Plane3f p = verticalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(vec3f::pos_y, p.normal);
}

TEST(PlaneTest, verticalDragPlaneWithZDirection) {
    const vec3f position(322.0f, -122.2392f, 34.0f);
    const vec3f direction = normalize(vec3f(1.0f, 2.0f, -3.0f));
    const Plane3f p = verticalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(vec3f::pos_y, p.normal);
}

TEST(PlaneTest, orthogonalDragPlane) {
    const vec3f position(322.0f, -122.2392f, 34.0f);
    const vec3f direction = normalize(vec3f(1.0f, 2.0f, -3.0f));
    const Plane3f p = orthogonalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(direction, p.normal);
}

TEST(PlaneTest, alignedOrthogonalDragPlane) {
    const vec3f position(322.0f, -122.2392f, 34.0f);
    const vec3f direction = normalize(vec3f(1.0f, 2.0f, -3.0f));
    const Plane3f p = alignedOrthogonalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(firstAxis(direction), p.normal);
}
