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

#include "Ray.h"
#include "MathUtils.h"
#include "TestUtils.h"

TEST(RayTest, pointAtDistance) {
    const Ray3f ray(vec3f::zero, vec3f::pos_x);
    ASSERT_VEC_EQ(vec3f(5.0f, 0.0f, 0.0f), ray.pointAtDistance(5.0f));
}

TEST(RayTest, pointStatus) {
    const Ray3f ray(vec3f::zero, vec3f::pos_z);
    ASSERT_EQ(Math::PointStatus::PSAbove, ray.pointStatus(vec3f(0.0f, 0.0f, 1.0f)));
    ASSERT_EQ(Math::PointStatus::PSInside, ray.pointStatus(vec3f(0.0f, 0.0f, 0.0f)));
    ASSERT_EQ(Math::PointStatus::PSBelow, ray.pointStatus(vec3f(0.0f, 0.0f, -1.0f)));
}

TEST(RayTest, intersectWithPlane) {
    const Ray3f ray(vec3f::zero, vec3f::pos_z);
    ASSERT_TRUE(Math::isnan(ray.intersectWithPlane(vec3f::pos_z, vec3f(0.0f, 0.0f, -1.0f))));
    ASSERT_FLOAT_EQ(0.0f, ray.intersectWithPlane(vec3f::pos_z, vec3f(0.0f, 0.0f,  0.0f)));
    ASSERT_FLOAT_EQ(1.0f, ray.intersectWithPlane(vec3f::pos_z, vec3f(0.0f, 0.0f,  1.0f)));
}

TEST(RayTest, intersectWithSphere) {
    const Ray3f ray(vec3f::zero, vec3f::pos_z);
    
    // ray originates inside sphere and hits at north pole
    ASSERT_FLOAT_EQ(2.0f, ray.intersectWithSphere(vec3f::zero, 2.0f));

    // ray originates outside sphere and hits at south pole
    ASSERT_FLOAT_EQ(3.0f, ray.intersectWithSphere(vec3f(0.0f, 0.0f, 5.0f), 2.0f));
    
    // miss
    ASSERT_TRUE(Math::isnan(ray.intersectWithSphere(vec3f(3.0f, 2.0f, 2.0f), 1.0f)));
}

TEST(RayTest, distanceToPoint) {
    const Ray3f ray(vec3f::zero, vec3f::pos_z);
    
    // point is behind ray
    ASSERT_FLOAT_EQ(0.0f, ray.squaredDistanceToPoint(vec3f(-1.0f, -1.0f, -1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(3.0f, ray.squaredDistanceToPoint(vec3f(-1.0f, -1.0f, -1.0f)).distance);
    
    // point is in front of ray
    ASSERT_FLOAT_EQ(1.0f, ray.squaredDistanceToPoint(vec3f(1.0f, 1.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(2.0f, ray.squaredDistanceToPoint(vec3f(1.0f, 1.0f, 1.0f)).distance);
    
    // point is on ray
    ASSERT_FLOAT_EQ(1.0f, ray.squaredDistanceToPoint(vec3f(0.0f, 0.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(0.0f, ray.squaredDistanceToPoint(vec3f(0.0f, 0.0f, 1.0f)).distance);
}

TEST(RayTest, distanceToSegment) {
    const Ray3f ray(vec3f::zero, vec3f::pos_z);
    Ray3f::LineDistance segDist;
    
    segDist = ray.squaredDistanceToSegment(vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 0.0f, 1.0f));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);

    segDist = ray.squaredDistanceToSegment(vec3f(1.0f, 1.0f, 0.0f), vec3f(1.0f, 1.0f, 1.0f));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);
    
    segDist = ray.squaredDistanceToSegment(vec3f(1.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(0.70710677f, segDist.lineDistance);
    
    segDist = ray.squaredDistanceToSegment(vec3f(1.0f, 0.0f, 0.0f), vec3f(2.0f, -1.0f, 0.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(1.0f, segDist.distance);
    ASSERT_FLOAT_EQ(0.0f, segDist.lineDistance);
    
    segDist = ray.distanceToSegment(vec3f(-1.0f, 1.5f, 2.0f), vec3f(+1.0f, 1.5f, 2.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(1.5f, segDist.distance);
    ASSERT_FLOAT_EQ(1.0f, segDist.lineDistance);
}

TEST(RayTest, distanceToLine) {
    const Ray3f ray(vec3f::zero, vec3f::pos_z);
    Ray3f::LineDistance segDist;
    
    segDist = ray.squaredDistanceToLine(vec3f(0.0f, 0.0f, 0.0f), vec3f::pos_z);
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);
    
    segDist = ray.squaredDistanceToLine(vec3f(1.0f, 1.0f, 0.0f), vec3f::pos_z);
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);
    
    segDist = ray.squaredDistanceToLine(vec3f(1.0f, 0.0f, 0.0f), normalize(vec3f(-1.0f, 1.0f, 0.0f)));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(std::sqrt(2.0f) / 2.0f, segDist.lineDistance);
    
    segDist = ray.squaredDistanceToLine(vec3f(1.0f, 0.0f, 0.0f), normalize(vec3f(1.0f, -1.0f, 0.0f)));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(-std::sqrt(2.0f) / 2.0f, segDist.lineDistance);
}

TEST(RayTest, intersectRayWithTriangle) {
    const vec3d p0(2.0, 5.0, 2.0);
    const vec3d p1(4.0, 7.0, 2.0);
    const vec3d p2(3.0, 2.0, 2.0);
    
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(vec3d::zero, vec3d::pos_x), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(vec3d::zero, vec3d::pos_y), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(vec3d::zero, vec3d::pos_z), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersectRayWithTriangle(Ray3d(vec3d(0.0, 0.0, 2.0), vec3d::pos_y), p0, p1, p2)));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(vec3d(3.0, 5.0, 0.0), vec3d::pos_z), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(vec3d(2.0, 5.0, 0.0), vec3d::pos_z), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(vec3d(4.0, 7.0, 0.0), vec3d::pos_z), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersectRayWithTriangle(Ray3d(vec3d(3.0, 2.0, 0.0), vec3d::pos_z), p0, p1, p2));
}
