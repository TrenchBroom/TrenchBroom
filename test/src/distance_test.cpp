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

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/quat.h>
#include <vecmath/bbox.h>
#include <vecmath/line.h>
#include <vecmath/ray.h>
#include <vecmath/segment.h>
#include <vecmath/polygon.h>
#include <vecmath/plane.h>
#include <vecmath/distance.h>

// TODO 2201: write more tests

namespace vm {
    TEST(DistanceTest, distanceRayAndPoint) {
        const ray3f ray(vec3f::zero, vec3f::pos_z);

        // point is behind ray
        ASSERT_FLOAT_EQ(0.0f, squaredDistance(ray, vec3f(-1.0f, -1.0f, -1.0f)).rayDistance);
        ASSERT_FLOAT_EQ(3.0f, squaredDistance(ray, vec3f(-1.0f, -1.0f, -1.0f)).distance);

        // point is in front of ray
        ASSERT_FLOAT_EQ(1.0f, squaredDistance(ray, vec3f(1.0f, 1.0f, 1.0f)).rayDistance);
        ASSERT_FLOAT_EQ(2.0f, squaredDistance(ray, vec3f(1.0f, 1.0f, 1.0f)).distance);

        // point is on ray
        ASSERT_FLOAT_EQ(1.0f, squaredDistance(ray, vec3f(0.0f, 0.0f, 1.0f)).rayDistance);
        ASSERT_FLOAT_EQ(0.0f, squaredDistance(ray, vec3f(0.0f, 0.0f, 1.0f)).distance);
    }

    TEST(DistanceTest, distanceRayAndSegment) {
        const ray3f ray(vec3f::zero, vec3f::pos_z);
        LineDistance<float> segDist;

        segDist = squaredDistance(ray, segment3f(vec3f(0.0f, 0.0f, 0.0f), vec3f(0.0f, 0.0f, 1.0f)));
        ASSERT_TRUE(segDist.parallel);
        ASSERT_FLOAT_EQ(0.0f, segDist.distance);

        segDist = squaredDistance(ray, segment3f(vec3f(1.0f, 1.0f, 0.0f), vec3f(1.0f, 1.0f, 1.0f)));
        ASSERT_TRUE(segDist.parallel);
        ASSERT_FLOAT_EQ(2.0f, segDist.distance);

        segDist = squaredDistance(ray, segment3f(vec3f(1.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f)));
        ASSERT_FALSE(segDist.parallel);
        ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
        ASSERT_FLOAT_EQ(0.5f, segDist.distance);
        ASSERT_FLOAT_EQ(0.70710677f, segDist.lineDistance);

        segDist = squaredDistance(ray, segment3f(vec3f(1.0f, 0.0f, 0.0f), vec3f(2.0f, -1.0f, 0.0f)));
        ASSERT_FALSE(segDist.parallel);
        ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
        ASSERT_FLOAT_EQ(1.0f, segDist.distance);
        ASSERT_FLOAT_EQ(0.0f, segDist.lineDistance);

        segDist = distance(ray, segment3f(vec3f(-1.0f, 1.5f, 2.0f), vec3f(+1.0f, 1.5f, 2.0f)));
        ASSERT_FALSE(segDist.parallel);
        ASSERT_FLOAT_EQ(2.0f, segDist.rayDistance);
        ASSERT_FLOAT_EQ(1.5f, segDist.distance);
        ASSERT_FLOAT_EQ(1.0f, segDist.lineDistance);
    }

    TEST(DistanceTest, distanceRayAndLine) {
        const ray3f ray(vec3f::zero, vec3f::pos_z);
        LineDistance<float> segDist;

        segDist = squaredDistance(ray, line3f(vec3f(0.0f, 0.0f, 0.0f), vec3f::pos_z));
        ASSERT_TRUE(segDist.parallel);
        ASSERT_FLOAT_EQ(0.0f, segDist.distance);

        segDist = squaredDistance(ray, line3f(vec3f(1.0f, 1.0f, 0.0f), vec3f::pos_z));
        ASSERT_TRUE(segDist.parallel);
        ASSERT_FLOAT_EQ(2.0f, segDist.distance);

        segDist = squaredDistance(ray, line3f(vec3f(1.0f, 0.0f, 0.0f), normalize(vec3f(-1.0f, 1.0f, 0.0f))));
        ASSERT_FALSE(segDist.parallel);
        ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
        ASSERT_FLOAT_EQ(0.5f, segDist.distance);
        ASSERT_FLOAT_EQ(std::sqrt(2.0f) / 2.0f, segDist.lineDistance);

        segDist = squaredDistance(ray, line3f(vec3f(1.0f, 0.0f, 0.0f), normalize(vec3f(1.0f, -1.0f, 0.0f))));
        ASSERT_FALSE(segDist.parallel);
        ASSERT_FLOAT_EQ(0.0f, segDist.rayDistance);
        ASSERT_FLOAT_EQ(0.5f, segDist.distance);
        ASSERT_FLOAT_EQ(-std::sqrt(2.0f) / 2.0f, segDist.lineDistance);
    }
}
