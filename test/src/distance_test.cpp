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

#include <vecmath/vec_decl.h>
#include <vecmath/vec_impl.h>
#include <vecmath/mat_decl.h>
#include <vecmath/mat_impl.h>
#include <vecmath/quat_decl.h>
#include <vecmath/quat_impl.h>
#include <vecmath/bbox_decl.h>
#include <vecmath/bbox_impl.h>
#include <vecmath/line_decl.h>
#include <vecmath/line_impl.h>
#include <vecmath/ray_decl.h>
#include <vecmath/ray_impl.h>
#include <vecmath/segment_decl.h>
#include <vecmath/segment_impl.h>
#include <vecmath/polygon_decl.h>
#include <vecmath/polygon_impl.h>
#include <vecmath/plane_decl.h>
#include <vecmath/plane_impl.h>
#include <vecmath/distance.h>

TEST(DistanceTest, distanceRayAndPoint) {
    const vm::ray3f ray(vm::vec3f::zero, vm::vec3f::pos_z);

    // point is behind ray
    ASSERT_FLOAT_EQ(0.0f, squaredDistance(ray, vm::vec3f(-1.0f, -1.0f, -1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(3.0f, squaredDistance(ray, vm::vec3f(-1.0f, -1.0f, -1.0f)).distance);

    // point is in front of ray
    ASSERT_FLOAT_EQ(1.0f, squaredDistance(ray, vm::vec3f(1.0f, 1.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(2.0f, squaredDistance(ray, vm::vec3f(1.0f, 1.0f, 1.0f)).distance);

    // point is on ray
    ASSERT_FLOAT_EQ(1.0f, squaredDistance(ray, vm::vec3f(0.0f, 0.0f, 1.0f)).rayDistance);
    ASSERT_FLOAT_EQ(0.0f, squaredDistance(ray, vm::vec3f(0.0f, 0.0f, 1.0f)).distance);
}

TEST(DistanceTest, distanceRayAndSegment) {
    const vm::ray3f ray(vm::vec3f::zero, vm::vec3f::pos_z);
    vm::LineDistance<float> segDist;

    segDist = squaredDistance(ray, vm::segment3f(vm::vec3f(0.0f, 0.0f, 0.0f), vm::vec3f(0.0f, 0.0f, 1.0f)));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);

    segDist = squaredDistance(ray, vm::segment3f(vm::vec3f(1.0f, 1.0f, 0.0f), vm::vec3f(1.0f, 1.0f, 1.0f)));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);

    segDist = squaredDistance(ray, vm::segment3f(vm::vec3f(1.0f, 0.0f, 0.0f), vm::vec3f(0.0f, 1.0f, 0.0f)));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(0.70710677f, segDist.lineDistance);

    segDist = squaredDistance(ray, vm::segment3f(vm::vec3f(1.0f, 0.0f, 0.0f), vm::vec3f(2.0f, -1.0f, 0.0f)));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(1.0f, segDist.distance);
    ASSERT_FLOAT_EQ(0.0f, segDist.lineDistance);

    segDist = distance(ray, vm::segment3f(vm::vec3f(-1.0f, 1.5f, 2.0f), vm::vec3f(+1.0f, 1.5f, 2.0f)));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(1.5f, segDist.distance);
    ASSERT_FLOAT_EQ(1.0f, segDist.lineDistance);
}

TEST(DistanceTest, distanceRayAndLine) {
    const vm::ray3f ray(vm::vec3f::zero, vm::vec3f::pos_z);
    vm::LineDistance<float> segDist;

    segDist = squaredDistance(ray, vm::line3f(vm::vec3f(0.0f, 0.0f, 0.0f), vm::vec3f::pos_z));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.distance);

    segDist = squaredDistance(ray, vm::line3f(vm::vec3f(1.0f, 1.0f, 0.0f), vm::vec3f::pos_z));
    ASSERT_TRUE(segDist.parallel);
    ASSERT_FLOAT_EQ(2.0f, segDist.distance);

    segDist = squaredDistance(ray, vm::line3f(vm::vec3f(1.0f, 0.0f, 0.0f), normalize(vm::vec3f(-1.0f, 1.0f, 0.0f))));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(std::sqrt(2.0f) / 2.0f, segDist.lineDistance);

    segDist = squaredDistance(ray, vm::line3f(vm::vec3f(1.0f, 0.0f, 0.0f), normalize(vm::vec3f(1.0f, -1.0f, 0.0f))));
    ASSERT_FALSE(segDist.parallel);
    ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
    ASSERT_FLOAT_EQ(0.5f, segDist.distance);
    ASSERT_FLOAT_EQ(-std::sqrt(2.0f) / 2.0f, segDist.lineDistance);
}
