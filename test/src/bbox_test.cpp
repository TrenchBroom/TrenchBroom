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

#include <vecmath/bbox.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include "TestUtils.h"

#include <vector>

// TODO 2201: write more tests

namespace vm {
    TEST(BBoxTest, defaultConstructor) {
        const bbox3f bounds;
        ASSERT_EQ(vec3f::zero, bounds.min);
        ASSERT_EQ(vec3f::zero, bounds.max);
    }

    TEST(BBoxTest, constructBBox3fWithMinAndMaxPoints) {
        const vec3f min(-1.0f, -2.0f, -3.0f);
        const vec3f max( 1.0f,  2.0f,  3.0f);

        const bbox3f bounds(min, max);
        ASSERT_EQ(min, bounds.min);
        ASSERT_EQ(max, bounds.max);
    }

    TEST(BBoxTest, constructBBox3fWithMinAndMaxValues) {
        const auto min = -16.f;
        const auto max = +32.0f;

        const bbox3f bounds(min, max);
        ASSERT_EQ(vec3f::fill(min), bounds.min);
        ASSERT_EQ(vec3f::fill(max), bounds.max);
    }

    TEST(BBoxTest, constructBBox3fWithMinMaxValue) {
        const auto minMax = 16.f;

        const bbox3f bounds(minMax);
        ASSERT_EQ(-vec3f::fill(minMax), bounds.min);
        ASSERT_EQ(+vec3f::fill(minMax), bounds.max);
    }

    TEST(BBoxTest, mergeAll) {
        const auto points = std::vector<vec3d>{
            vec3d(-32, -16, - 8), vec3d(  0, - 4, -4),
            vec3d(+ 4, + 8, -16), vec3d(+32, +16, -4),
            vec3d(+16, + 4, - 8), vec3d(+24, +32, +4)
        };

        auto min = points[0];
        auto max = points[0];
        for (size_t i = 1; i < points.size(); ++i) {
            min = vm::min(min, points[i]);
            max = vm::max(max, points[i]);
        }

        const auto merged = bbox3d::mergeAll(std::begin(points), std::end(points));
        ASSERT_EQ(min, merged.min);
        ASSERT_EQ(max, merged.max);
    }

    TEST(BBoxTest, valid) {
        ASSERT_TRUE(bbox3d::valid(vec3d::zero, vec3d::zero));
        ASSERT_TRUE(bbox3d::valid(vec3d(-1, -1, -1), vec3d(+1, +1, +1)));
        ASSERT_FALSE(bbox3d::valid(vec3d(+1, -1, -1), vec3d(-1, +1, +1)));
        ASSERT_FALSE(bbox3d::valid(vec3d(-1, +1, -1), vec3d(+1, -1, +1)));
        ASSERT_FALSE(bbox3d::valid(vec3d(-1, -1, +1), vec3d(+1, +1, -1)));
    }

    TEST(BBoxTest, empty) {
        ASSERT_TRUE(bbox3d().empty());
        ASSERT_FALSE(bbox3d(1.0).empty());
        ASSERT_TRUE(bbox3d(vec3d(-1, 0, -1), vec3d(+1, 0, +1)).empty());
    }

    TEST(BBoxTest, center) {
        const vec3f min(-1, -2, -3);
        const vec3f max( 1,  4,  5);
        const bbox3f bounds(min, max);

        ASSERT_EQ(vec3f(0, 1, 1), bounds.center());
    }

    TEST(BBoxTest, size) {
        const vec3f min(-1, -2, -3);
        const vec3f max( 1,  3,  5);
        const bbox3f bounds(min, max);

        ASSERT_EQ(vec3f(2, 5, 8), bounds.size());
    }

    TEST(BBoxTest, volume) {
        ASSERT_DOUBLE_EQ(0.0, bbox3d().volume());
        ASSERT_DOUBLE_EQ(4.0 * 4.0 * 4.0, bbox3d(2.0).volume());
    }

    TEST(BBoxTest, containsPoint) {
        const bbox3f bounds(vec3f(-12, -3,  4), vec3f( 8, 9, 8));
        ASSERT_TRUE(bounds.contains(vec3f(2, 1, 7)));
        ASSERT_TRUE(bounds.contains(vec3f(-12, -3, 7)));
        ASSERT_FALSE(bounds.contains(vec3f(-13, -3, 7)));
    }

    TEST(BBoxTest, containsBBox) {
        const bbox3f bounds1(vec3f(-12, -3,  4), vec3f( 8, 9, 8));
        const bbox3f bounds2(vec3f(-10, -2,  5), vec3f( 7, 8, 7));
        const bbox3f bounds3(vec3f(-13, -2,  5), vec3f( 7, 8, 7));
        ASSERT_TRUE(bounds1.contains(bounds1));
        ASSERT_TRUE(bounds1.contains(bounds2));
        ASSERT_FALSE(bounds1.contains(bounds3));
    }

    TEST(BBoxTest, enclosesBBox) {
        const bbox3f bounds1(vec3f(-12, -3,  4), vec3f( 8, 9, 8));
        const bbox3f bounds2(vec3f(-10, -2,  5), vec3f( 7, 8, 7));
        const bbox3f bounds3(vec3f(-10, -3,  5), vec3f( 7, 8, 7));
        ASSERT_FALSE(bounds1.encloses(bounds1));
        ASSERT_TRUE(bounds1.encloses(bounds2));
        ASSERT_FALSE(bounds1.encloses(bounds3));
    }

    TEST(BBoxTest, constrain) {
        const bbox3d bounds (1024.0);
        ASSERT_VEC_EQ(vec3d::zero, bounds.constrain(vec3d::zero));
        ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min));
        ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min + vec3d::neg_x));
        ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min + vec3d::neg_y));
        ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min + vec3d::neg_z));
        ASSERT_VEC_EQ(bounds.max, bounds.constrain(bounds.max + vec3d::pos_x));
        ASSERT_VEC_EQ(bounds.max, bounds.constrain(bounds.max + vec3d::pos_y));
        ASSERT_VEC_EQ(bounds.max, bounds.constrain(bounds.max + vec3d::pos_z));
    }

    TEST(BBoxTest, corner) {
        const vec3f min(-1.0f, -2.0f, -3.0f);
        const vec3f max( 1.0f,  3.0f,  5.0f);
        const bbox3f bounds(min, max);

        ASSERT_VEC_EQ(vec3f(-1.0f, -2.0f, -3.0f), bounds.corner(bbox3f::Corner::min, bbox3f::Corner::min, bbox3f::Corner::min));
        ASSERT_VEC_EQ(vec3f(-1.0f, -2.0f,  5.0f), bounds.corner(bbox3f::Corner::min, bbox3f::Corner::min, bbox3f::Corner::max));
        ASSERT_VEC_EQ(vec3f(-1.0f,  3.0f, -3.0f), bounds.corner(bbox3f::Corner::min, bbox3f::Corner::max, bbox3f::Corner::min));
        ASSERT_VEC_EQ(vec3f(-1.0f,  3.0f,  5.0f), bounds.corner(bbox3f::Corner::min, bbox3f::Corner::max, bbox3f::Corner::max));
        ASSERT_VEC_EQ(vec3f( 1.0f, -2.0f, -3.0f), bounds.corner(bbox3f::Corner::max, bbox3f::Corner::min, bbox3f::Corner::min));
        ASSERT_VEC_EQ(vec3f( 1.0f, -2.0f,  5.0f), bounds.corner(bbox3f::Corner::max, bbox3f::Corner::min, bbox3f::Corner::max));
        ASSERT_VEC_EQ(vec3f( 1.0f,  3.0f, -3.0f), bounds.corner(bbox3f::Corner::max, bbox3f::Corner::max, bbox3f::Corner::min));
        ASSERT_VEC_EQ(vec3f( 1.0f,  3.0f,  5.0f), bounds.corner(bbox3f::Corner::max, bbox3f::Corner::max, bbox3f::Corner::max));
    }

    TEST(BBoxTest, relativePosition) {
        const bbox3f bounds(vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f, 9.0f, 8.0f));
        const vec3f point1(-1.0f, 0.0f, 0.0f);
        const auto pos1 = bounds.relativePosition(point1);
        ASSERT_EQ(bbox3f::Range::within, pos1[0]);
        ASSERT_EQ(bbox3f::Range::within, pos1[1]);
        ASSERT_EQ(bbox3f::Range::less,   pos1[2]);
    }

    TEST(BBoxTest, expand) {
        const bbox3f bounds  (vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f,  9.0f,  8.0f));
        const bbox3f expanded(vec3f(-14.0f, -5.0f,  2.0f), vec3f(10.0f, 11.0f, 10.0f));
        ASSERT_EQ(expanded, bounds.expand(2.0f));
    }

    TEST(BBoxTest, translate) {
        const bbox3f bounds    (vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f, 9.0f, 8.0f));
        const bbox3f translated(vec3f(-10.0f, -4.0f,  1.0f), vec3f(10.0f, 8.0f, 5.0f));
        ASSERT_EQ(translated, bounds.translate(vec3f(2.0f, -1.0f, -3.0f)));
    }

    TEST(BBoxTest, transform) {
        const auto bounds = bbox3d(-2.0, +10.0);
        const auto transform = rotationMatrix(radians(10.0), radians(77.0), radians(227.0));
        const auto points = bounds.vertices();
        const auto transformedPoints = transform * std::vector<vec3d>(std::begin(points), std::end(points));
        const auto transformed = bbox3d::mergeAll(std::begin(transformedPoints), std::end(transformedPoints));
        ASSERT_VEC_EQ(transformed.min, bounds.transform(transform).min);
        ASSERT_VEC_EQ(transformed.max, bounds.transform(transform).max);
    }

    TEST(BBoxTest, equal) {
        const vec3f min(-1, -2, -3);
        const vec3f max( 1,  2,  3);

        const bbox3f bounds1(min, max);
        const bbox3f bounds2(min, max);
        const bbox3f bounds3(22.0f);

        ASSERT_TRUE(bounds1 == bounds2);
        ASSERT_FALSE(bounds1 == bounds3);
    }

    TEST(BBoxTest, notEqual) {
        const vec3f min(-1, -2, -3);
        const vec3f max( 1,  2,  3);

        const bbox3f bounds1(min, max);
        const bbox3f bounds2(min, max);
        const bbox3f bounds3(22.0f);

        ASSERT_FALSE(bounds1 != bounds2);
        ASSERT_TRUE(bounds1 != bounds3);
    }

    TEST(BBoxTest, mergeWithBBox) {
        const bbox3f bounds1(vec3f(-12.0f, -3.0f, 4.0f), vec3f(7.0f, 8.0f, 9.0f));
        const bbox3f bounds2(vec3f(-10.0f, -5.0f, 3.0f), vec3f(9.0f, 9.0f, 5.0f));
        const bbox3f merged( vec3f(-12.0f, -5.0f, 3.0f), vec3f(9.0f, 9.0f, 9.0f));

        ASSERT_EQ(merged, merge(bounds1, bounds2));
    }

    TEST(BBoxTest, mergeWithVec) {
        bbox3f bounds(vec3f(-12.0f, -3.0f, 4.0f), vec3f(7.0f, 8.0f,  9.0f));
        const vec3f  vec(-10.0f, -6.0f, 10.0f);
        const bbox3f merged(vec3f(-12.0f, -6.0f, 4.0f), vec3f(7.0f, 8.0f, 10.0f));

        ASSERT_EQ(merged, merge(bounds, vec));
    }

    TEST(BBoxTest, intersectsBBox) {
        const bbox3f bounds1(vec3f(-12.0f, -3.0f,  4.0f), vec3f(  8.0f,  9.0f,  8.0f));
        const bbox3f bounds2(vec3f(-10.0f, -2.0f,  5.0f), vec3f(  7.0f,  8.0f,  7.0f));
        const bbox3f bounds3(vec3f(-13.0f, -2.0f,  5.0f), vec3f(  7.0f,  8.0f,  7.0f));
        const bbox3f bounds4(vec3f(-15.0f, 10.0f,  9.0f), vec3f(-13.0f, 12.0f, 10.0f));
        const bbox3f bounds5(vec3f(-15.0f, 10.0f,  9.0f), vec3f(-12.0f, 12.0f, 10.0f));
        ASSERT_TRUE(bounds1.intersects(bounds1));
        ASSERT_TRUE(bounds1.intersects(bounds2));
        ASSERT_TRUE(bounds1.intersects(bounds3));
        ASSERT_FALSE(bounds1.intersects(bounds4));
        ASSERT_FALSE(bounds1.intersects(bounds5));
    }
}
