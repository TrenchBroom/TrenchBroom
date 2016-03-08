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

#include "Ray.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(RayTest, pointAtDistance) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosX);
    ASSERT_VEC_EQ(Vec3f(5.0f, 0.0f, 0.0f), ray.pointAtDistance(5.0f));
}

TEST(RayTest, pointStatus) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    ASSERT_EQ(Math::PointStatus::PSAbove, ray.pointStatus(Vec3f(0.0f, 0.0f, 1.0f)));
    ASSERT_EQ(Math::PointStatus::PSInside, ray.pointStatus(Vec3f(0.0f, 0.0f, 0.0f)));
    ASSERT_EQ(Math::PointStatus::PSBelow, ray.pointStatus(Vec3f(0.0f, 0.0f, -1.0f)));
}

TEST(RayTest, intersectWithPlane) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    ASSERT_TRUE(Math::isnan(ray.intersectWithPlane(Vec3f::PosZ, Vec3f(0.0f, 0.0f, -1.0f))));
    ASSERT_FLOAT_EQ(0.0f, ray.intersectWithPlane(Vec3f::PosZ, Vec3f(0.0f, 0.0f,  0.0f)));
    ASSERT_FLOAT_EQ(1.0f, ray.intersectWithPlane(Vec3f::PosZ, Vec3f(0.0f, 0.0f,  1.0f)));
}

TEST(RayTest, intersectWithSphere) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    
    // ray originates inside sphere and hits at north pole
    ASSERT_FLOAT_EQ(2.0f, ray.intersectWithSphere(Vec3f::Null, 2.0f));

    // ray originates outside sphere and hits at south pole
    ASSERT_FLOAT_EQ(3.0f, ray.intersectWithSphere(Vec3f(0.0f, 0.0f, 5.0f), 2.0f));
    
    // miss
    ASSERT_TRUE(Math::isnan(ray.intersectWithSphere(Vec3f(3.0f, 2.0f, 2.0f), 1.0f)));
}

TEST(RayTest, distanceToPoint) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    
    // point is behind ray
    ASSERT_FLOAT_EQ(0.0f, ray.squaredDistanceToPoint(Vec3f(-1.0f, -1.0f, -1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(3.0f, ray.squaredDistanceToPoint(Vec3f(-1.0f, -1.0f, -1.0f)).distance);
    
    // point is in front of ray
    ASSERT_FLOAT_EQ(1.0f, ray.squaredDistanceToPoint(Vec3f(1.0f, 1.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(2.0f, ray.squaredDistanceToPoint(Vec3f(1.0f, 1.0f, 1.0f)).distance);
    
    // point is on ray
    ASSERT_FLOAT_EQ(1.0f, ray.squaredDistanceToPoint(Vec3f(0.0f, 0.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(0.0f, ray.squaredDistanceToPoint(Vec3f(0.0f, 0.0f, 1.0f)).distance);
}

TEST(RayTest, distanceToSegment) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    Ray3f::LineDistance segDist;
    
    segDist = ray.squaredDistanceToSegment(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);

    segDist = ray.squaredDistanceToSegment(Vec3f(1.0f, 1.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);
    
    segDist = ray.squaredDistanceToSegment(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(0.5f, segDist.lineDistance);
    
    segDist = ray.squaredDistanceToSegment(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(2.0f, -1.0f, 0.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(1.0f, segDist.distance);
    ASSERT_FLOAT_EQ(0.0f, segDist.lineDistance);
}

TEST(RayTest, distanceToLine) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    Ray3f::LineDistance segDist;
    
    segDist = ray.squaredDistanceToLine(Vec3f(0.0f, 0.0f, 0.0f), Vec3f::PosZ);
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);
    
    segDist = ray.squaredDistanceToLine(Vec3f(1.0f, 1.0f, 0.0f), Vec3f::PosZ);
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);
    
    segDist = ray.squaredDistanceToLine(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(-1.0f, 1.0f, 0.0f).normalized());
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(std::sqrt(2.0f) / 2.0f, segDist.lineDistance);
    
    segDist = ray.squaredDistanceToLine(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(1.0f, -1.0f, 0.0f).normalized());
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(-std::sqrt(2.0f) / 2.0f, segDist.lineDistance);
}

TEST(RayTest, intersectRayWithTriangle) {
    const Vec3d p0(2.0, 5.0, 2.0);
    const Vec3d p1(4.0, 7.0, 2.0);
    const Vec3d p2(3.0, 2.0, 2.0);
    
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(Vec3d::Null, Vec3d::PosX), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(Vec3d::Null, Vec3d::PosY), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(Vec3d::Null, Vec3d::PosZ), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(Vec3d(0.0, 0.0, 2.0), Vec3d::PosY), p0, p1, p2)));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(Vec3d(3.0, 5.0, 0.0), Vec3d::PosZ), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(Vec3d(2.0, 5.0, 0.0), Vec3d::PosZ), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(Vec3d(4.0, 7.0, 0.0), Vec3d::PosZ), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(Vec3d(3.0, 2.0, 0.0), Vec3d::PosZ), p0, p1, p2));
}
