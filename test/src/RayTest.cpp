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
    ASSERT_FLOAT_EQ(0.0f, ray.distanceToPointSquared(Vec3f(-1.0f, -1.0f, -1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(3.0f, ray.distanceToPointSquared(Vec3f(-1.0f, -1.0f, -1.0f)).distance);
    
    // point is in front of ray
    ASSERT_FLOAT_EQ(1.0f, ray.distanceToPointSquared(Vec3f(1.0f, 1.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(2.0f, ray.distanceToPointSquared(Vec3f(1.0f, 1.0f, 1.0f)).distance);
    
    // point is on ray
    ASSERT_FLOAT_EQ(1.0f, ray.distanceToPointSquared(Vec3f(0.0f, 0.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(0.0f, ray.distanceToPointSquared(Vec3f(0.0f, 0.0f, 1.0f)).distance);
}

TEST(RayTest, distanceToSegment) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    Ray3f::LineDistance segDist;
    
    segDist = ray.distanceToSegmentSquared(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);

    segDist = ray.distanceToSegmentSquared(Vec3f(1.0f, 1.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);
    
    segDist = ray.distanceToSegmentSquared(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_VEC_EQ(Vec3f(0.5f, 0.5f, 0.0f), segDist.point);
    
    segDist = ray.distanceToSegmentSquared(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(2.0f, -1.0f, 0.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(1.0f, segDist.distance);
    ASSERT_VEC_EQ(Vec3f(1.0f, 0.0f, 0.0f), segDist.point);
}

TEST(RayTest, distanceToLine) {
    const Ray3f ray(Vec3f::Null, Vec3f::PosZ);
    Ray3f::LineDistance segDist;
    
    segDist = ray.distanceToLineSquared(Vec3f(0.0f, 0.0f, 0.0f), Vec3f::PosZ);
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);
    
    segDist = ray.distanceToLineSquared(Vec3f(1.0f, 1.0f, 0.0f), Vec3f::PosZ);
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);
    
    segDist = ray.distanceToLineSquared(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(-1.0f, 1.0f, 0.0f).normalized());
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_VEC_EQ(Vec3f(0.5f, 0.5f, 0.0f), segDist.point);
    
    segDist = ray.distanceToLineSquared(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(1.0f, -1.0f, 0.0f));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_VEC_EQ(Vec3f(0.5f, 0.5f, 0.0f), segDist.point);
}
