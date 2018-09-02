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

#include "bbox_decl.h"
#include "bbox_impl.h"
#include "vec_decl.h"

#include "TestUtils.h"

TEST(BBoxTest, constructBBox3fWithDefaults) {
    const bbox3f bounds;
    ASSERT_EQ(vec3f::zero, bounds.min);
    ASSERT_EQ(vec3f::zero, bounds.max);
}

TEST(BBoxTest, constructBBox3fWithMinAndMax) {
    const vec3f min(-1.0f, -2.0f, -3.0f);
    const vec3f max( 1.0f,  2.0f,  3.0f);
    
    const bbox3f bounds(min, max);
    ASSERT_EQ(min, bounds.min);
    ASSERT_EQ(max, bounds.max);
}

TEST(BBoxTest, operatorEquals) {
    const vec3f min(-1.0f, -2.0f, -3.0f);
    const vec3f max( 1.0f,  2.0f,  3.0f);
    
    const bbox3f bounds1(min, max);
    const bbox3f bounds2(min, max);

    ASSERT_TRUE(bounds1 == bounds2);
}

TEST(BBoxTest, getCenter) {
    const vec3f min(-1.0f, -2.0f, -3.0f);
    const vec3f max( 1.0f,  3.0f,  5.0f);
    const bbox3f bounds(min, max);
    
    ASSERT_EQ(vec3f(0.0f, 0.5f, 1.0f), bounds.center());
}

TEST(BBoxTest, getSize) {
    const vec3f min(-1.0f, -2.0f, -3.0f);
    const vec3f max( 1.0f,  3.0f,  5.0f);
    const bbox3f bounds(min, max);
    
    ASSERT_EQ(vec3f(2.0f, 5.0f, 8.0f), bounds.size());
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

TEST(BBoxTest, containsPoint) {
    const bbox3f bounds(vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f, 9.0f, 8.0f));
    ASSERT_TRUE(bounds.contains(vec3f(2.0f, 1.0f, 7.0f)));
    ASSERT_TRUE(bounds.contains(vec3f(-12.0f, -3.0f, 7.0f)));
    ASSERT_FALSE(bounds.contains(vec3f(-13.0f, -3.0f, 7.0f)));
}

TEST(BBoxTest, relativePosition) {
    const bbox3f bounds(vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f, 9.0f, 8.0f));
    const vec3f point1(-1.0f, 0.0f, 0.0f);
    const auto pos1 = bounds.relativePosition(point1);
    ASSERT_EQ(bbox3f::Range::within, pos1[0]);
    ASSERT_EQ(bbox3f::Range::within, pos1[1]);
    ASSERT_EQ(bbox3f::Range::less,   pos1[2]);
}

TEST(BBoxTest, containsBBox) {
    const bbox3f bounds1(vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f, 9.0f, 8.0f));
    const bbox3f bounds2(vec3f(-10.0f, -2.0f,  5.0f), vec3f( 7.0f, 8.0f, 7.0f));
    const bbox3f bounds3(vec3f(-13.0f, -2.0f,  5.0f), vec3f( 7.0f, 8.0f, 7.0f));
    ASSERT_TRUE(bounds1.contains(bounds1));
    ASSERT_TRUE(bounds1.contains(bounds2));
    ASSERT_FALSE(bounds1.contains(bounds3));
}

TEST(BBoxTest, enclosesBBox) {
    const bbox3f bounds1(vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f, 9.0f, 8.0f));
    const bbox3f bounds2(vec3f(-10.0f, -2.0f,  5.0f), vec3f( 7.0f, 8.0f, 7.0f));
    const bbox3f bounds3(vec3f(-10.0f, -3.0f,  5.0f), vec3f( 7.0f, 8.0f, 7.0f));
    ASSERT_FALSE(bounds1.encloses(bounds1));
    ASSERT_TRUE(bounds1.encloses(bounds2));
    ASSERT_FALSE(bounds1.encloses(bounds3));
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

TEST(BBoxTest, intersectWithRay) {
    const bbox3f bounds(vec3f(-12.0f, -3.0f,  4.0f), vec3f(  8.0f,  9.0f,  8.0f));

    float distance = intersect(Ray3f(vec3f::zero, vec3f::neg_z), bounds);
    ASSERT_TRUE(Math::isnan(distance));
    
    distance = intersect(Ray3f(vec3f::zero, vec3f::pos_z), bounds);
    ASSERT_FALSE(Math::isnan(distance));
    ASSERT_FLOAT_EQ(4.0f, distance);

    const vec3f origin = vec3f(-10.0f, -7.0f, 14.0f);
    const vec3f diff = vec3f(-2.0f, 3.0f, 8.0f) - origin;
    const vec3f dir = normalize(diff);
    distance = intersect(Ray3f(origin, dir), bounds);
    ASSERT_FALSE(Math::isnan(distance));
    ASSERT_FLOAT_EQ(length(diff), distance);

}

TEST(BBoxTest, expand) {
          bbox3f bounds  (vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f,  9.0f,  8.0f));
    const bbox3f expanded(vec3f(-14.0f, -5.0f,  2.0f), vec3f(10.0f, 11.0f, 10.0f));
    ASSERT_EQ(expanded, bounds.expand(2.0f));
}

TEST(BBoxTest, translate) {
          bbox3f bounds    (vec3f(-12.0f, -3.0f,  4.0f), vec3f( 8.0f, 9.0f, 8.0f));
    const bbox3f translated(vec3f(-10.0f, -4.0f,  1.0f), vec3f(10.0f, 8.0f, 5.0f));
    ASSERT_EQ(translated, bounds.translate(vec3f(2.0f, -1.0f, -3.0f)));
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
