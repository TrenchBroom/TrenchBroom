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
#include "Plane.h"
#include "MathUtils.h"
#include "TestUtils.h"
#include "Model/PlanePointFinder.h"

TEST(PlaneTest, constructDefault) {
    const Plane3f p;
    ASSERT_EQ(0.0f, p.distance);
    ASSERT_EQ(Vec3f::Null, p.normal);
}

TEST(PlaneTest, constructWithDistanceAndNormal) {
    const float d = 123.0f;
    const Vec3f n = Vec3f(1.0f, 2.0f, 3.0f).normalized();
    const Plane3f p(d, n);
    ASSERT_FLOAT_EQ(d, p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, constructWithAnchorAndNormal) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    ASSERT_FLOAT_EQ(a.dot(n), p.distance);
    ASSERT_VEC_EQ(n, p.normal);
}

TEST(PlaneTest, constructPlaneContainingVector) {
}

TEST(PlaneTest, anchor) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    ASSERT_VEC_EQ(p.distance * n, p.anchor());
}

TEST(PlaneTest, intersectWithRay) {
}

TEST(PlaneTest, intersectWithLine) {
}

TEST(PlaneTest, pointStatus) {
    const Plane3f p(10.0f, Vec3f::PosZ);
    ASSERT_EQ(Math::PointStatus::PSAbove, p.pointStatus(Vec3f(0.0f, 0.0f, 11.0f)));
    ASSERT_EQ(Math::PointStatus::PSBelow, p.pointStatus(Vec3f(0.0f, 0.0f, 9.0f)));
    ASSERT_EQ(Math::PointStatus::PSInside, p.pointStatus(Vec3f(0.0f, 0.0f, 10.0f)));
}

TEST(PlaneTest, pointDistance) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    const Vec3f point(1.0f, -32.37873f, 32.0f);
    ASSERT_EQ(point.dot(p.normal) - p.distance, p.pointDistance(point));
}

TEST(PlaneTest, valueAtParallelPlanes) {
    const Plane3f p1(10.0f, Vec3f::PosX);
    
    ASSERT_FLOAT_EQ(p1.distance, p1.at(Vec2f(2.0f, 1.0f), Math::Axis::AX));
    ASSERT_FLOAT_EQ(p1.distance, p1.at(Vec2f(22.0f, -34322.0232f), Math::Axis::AX));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(2.0f, 1.0f), Math::Axis::AY));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(22.0f, -34322.0232f), Math::Axis::AY));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(2.0f, 1.0f), Math::Axis::AZ));
    ASSERT_FLOAT_EQ(0.0f, p1.at(Vec2f(22.0f, -34322.0232f), Math::Axis::AZ));
}

TEST(PlaneTest, valueAt) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    const Vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.y() - point1.y() * p.normal.z()) / p.normal[Math::Axis::AX],
                    p.at(point1, Math::Axis::AX));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.z()) / p.normal[Math::Axis::AY],
                    p.at(point1, Math::Axis::AY));
    ASSERT_FLOAT_EQ((p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.y()) / p.normal[Math::Axis::AZ],
                    p.at(point1, Math::Axis::AZ));
}

TEST(PlaneTest, xYZValueAt) {
    const Vec3f a = Vec3f(-2038.034f, 0.0023f, 32.0f);
    const Vec3f n = Vec3f(9.734f, -3.393f, 2.033f).normalized();
    const Plane3f p(a, n);
    const Vec2f point1(27.022f, -12.0123223f);
    
    ASSERT_FLOAT_EQ(p.at(point1, Math::Axis::AX), p.xAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, Math::Axis::AY), p.yAt(point1));
    ASSERT_FLOAT_EQ(p.at(point1, Math::Axis::AZ), p.zAt(point1));
}

TEST(PlaneTest, equals) {
    ASSERT_TRUE(Plane3f(0.0f, Vec3f::PosX).equals(Plane3f(0.0f, Vec3f::PosX)));
    ASSERT_TRUE(Plane3f(0.0f, Vec3f::PosY).equals(Plane3f(0.0f, Vec3f::PosY)));
    ASSERT_TRUE(Plane3f(0.0f, Vec3f::PosZ).equals(Plane3f(0.0f, Vec3f::PosZ)));
    ASSERT_FALSE(Plane3f(0.0f, Vec3f::PosX).equals(Plane3f(0.0f, Vec3f::NegX)));
    ASSERT_FALSE(Plane3f(0.0f, Vec3f::PosX).equals(Plane3f(0.0f, Vec3f::PosY)));
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
    ASSERT_VEC_EQ(Vec3f(1.0f, 2.0f, 0.0f), Plane3f(0.0f, Vec3f::PosZ).project(Vec3f(1.0f, 2.0f, 3.0f)));
    ASSERT_VEC_EQ(Vec3f(1.0f, 2.0f, 2.0f), Plane3f(2.0f, Vec3f::PosZ).project(Vec3f(1.0f, 2.0f, 3.0f)));
}

TEST(PlaneTest, setPlanePoints) {
    Plane3f plane;
    Vec3f points[3];

    points[0] = Vec3f(0.0f, 0.0f, 0.0f);
    points[1] = Vec3f(0.0f, 0.0f, 0.0f);
    points[2] = Vec3f(0.0f, 0.0f, 0.0f);
    ASSERT_FALSE(setPlanePoints(plane, points));
    
    points[0] = Vec3f(0.0f, 0.0f, 0.0f);
    points[1] = Vec3f(0.0f, 1.0f, 0.0f);
    points[2] = Vec3f(1.0f, 0.0f, 0.0f);
    ASSERT_TRUE(setPlanePoints(plane, points));
    ASSERT_VEC_EQ(Vec3f::PosZ, plane.normal);
    ASSERT_FLOAT_EQ(0.0f, plane.distance);
}

TEST(PlaneTest, horizontalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Plane3f p = horizontalDragPlane(position);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(Vec3f::PosZ, p.normal);
}

TEST(PlaneTest, verticalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 3.0f, -2.0f).normalize();
    const Plane3f p = verticalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(Vec3f::PosY, p.normal);
}

TEST(PlaneTest, verticalDragPlaneWithZDirection) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 2.0f, -3.0f).normalize();
    const Plane3f p = verticalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(Vec3f::PosY, p.normal);
}

TEST(PlaneTest, orthogonalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 2.0f, -3.0f).normalize();
    const Plane3f p = orthogonalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(direction, p.normal);
}

TEST(PlaneTest, alignedOrthogonalDragPlane) {
    const Vec3f position(322.0f, -122.2392f, 34.0f);
    const Vec3f direction = Vec3f(1.0f, 2.0f, -3.0f).normalize();
    const Plane3f p = alignedOrthogonalDragPlane(position, direction);
    ASSERT_TRUE(p.pointStatus(position) == Math::PointStatus::PSInside);
    ASSERT_VEC_EQ(direction.firstAxis(), p.normal);
}

TEST(PlaneTest, planePointFinder) {
	Plane3 plane;
	const Vec3 points[3] = {Vec3(48, 16, 28), Vec3(16.0, 16.0, 27.9980487823486328125), Vec3(48, 18, 22)};
	ASSERT_FALSE(points[1].isInteger());
	ASSERT_TRUE(setPlanePoints(plane, points[0], points[1], points[2]));
	
	// Some verts that should lie (very close to) on the plane
	std::vector<Vec3> verts;
	verts.push_back(Vec3(48, 18, 22));
	verts.push_back(Vec3(48, 16, 28));
	verts.push_back(Vec3(16, 16, 28));
	verts.push_back(Vec3(16, 18, 22));
	
	for (size_t i=0; i<verts.size(); i++) {
		FloatType dist = Math::abs(plane.pointDistance(verts[i]));
		ASSERT_LT(dist, 0.01);
	}
	
	// Now find a similar plane with integer points
	
	Vec3 intpoints[3];
	for (size_t i=0; i<3; i++)
		intpoints[i] = points[i];
	
	TrenchBroom::Model::PlanePointFinder::findPoints(plane, intpoints, 3);

	ASSERT_TRUE(intpoints[0].isInteger());
	ASSERT_TRUE(intpoints[1].isInteger());
	ASSERT_TRUE(intpoints[2].isInteger());
	
	Plane3 intplane;
	ASSERT_TRUE(setPlanePoints(intplane, intpoints[0], intpoints[1], intpoints[2]));
	ASSERT_FALSE(intplane.equals(plane));
	
	// Check that the verts are still close to the new integer plane
	
	for (size_t i=0; i<verts.size(); i++) {
		FloatType dist = Math::abs(intplane.pointDistance(verts[i]));
		ASSERT_LT(dist, 0.01);
	}
}
