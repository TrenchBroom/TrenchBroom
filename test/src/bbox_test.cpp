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
    const vm::bbox3f bounds;
    ASSERT_EQ(vm::vec3f::zero, bounds.min);
    ASSERT_EQ(vm::vec3f::zero, bounds.max);
}

TEST(BBoxTest, constructBBox3fWithMinAndMax) {
    const vm::vec3f min(-1.0f, -2.0f, -3.0f);
    const vm::vec3f max( 1.0f,  2.0f,  3.0f);
    
    const vm::bbox3f bounds(min, max);
    ASSERT_EQ(min, bounds.min);
    ASSERT_EQ(max, bounds.max);
}

TEST(BBoxTest, operatorEquals) {
    const vm::vec3f min(-1.0f, -2.0f, -3.0f);
    const vm::vec3f max( 1.0f,  2.0f,  3.0f);
    
    const vm::bbox3f bounds1(min, max);
    const vm::bbox3f bounds2(min, max);

    ASSERT_TRUE(bounds1 == bounds2);
}

TEST(BBoxTest, getCenter) {
    const vm::vec3f min(-1.0f, -2.0f, -3.0f);
    const vm::vec3f max( 1.0f,  3.0f,  5.0f);
    const vm::bbox3f bounds(min, max);
    
    ASSERT_EQ(vm::vec3f(0.0f, 0.5f, 1.0f), bounds.center());
}

TEST(BBoxTest, getSize) {
    const vm::vec3f min(-1.0f, -2.0f, -3.0f);
    const vm::vec3f max( 1.0f,  3.0f,  5.0f);
    const vm::bbox3f bounds(min, max);
    
    ASSERT_EQ(vm::vec3f(2.0f, 5.0f, 8.0f), bounds.size());
}

TEST(BBoxTest, corner) {
    const vm::vec3f min(-1.0f, -2.0f, -3.0f);
    const vm::vec3f max( 1.0f,  3.0f,  5.0f);
    const vm::bbox3f bounds(min, max);

    ASSERT_VEC_EQ(vm::vec3f(-1.0f, -2.0f, -3.0f), bounds.corner(vm::bbox3f::Corner::min, vm::bbox3f::Corner::min, vm::bbox3f::Corner::min));
    ASSERT_VEC_EQ(vm::vec3f(-1.0f, -2.0f,  5.0f), bounds.corner(vm::bbox3f::Corner::min, vm::bbox3f::Corner::min, vm::bbox3f::Corner::max));
    ASSERT_VEC_EQ(vm::vec3f(-1.0f,  3.0f, -3.0f), bounds.corner(vm::bbox3f::Corner::min, vm::bbox3f::Corner::max, vm::bbox3f::Corner::min));
    ASSERT_VEC_EQ(vm::vec3f(-1.0f,  3.0f,  5.0f), bounds.corner(vm::bbox3f::Corner::min, vm::bbox3f::Corner::max, vm::bbox3f::Corner::max));
    ASSERT_VEC_EQ(vm::vec3f( 1.0f, -2.0f, -3.0f), bounds.corner(vm::bbox3f::Corner::max, vm::bbox3f::Corner::min, vm::bbox3f::Corner::min));
    ASSERT_VEC_EQ(vm::vec3f( 1.0f, -2.0f,  5.0f), bounds.corner(vm::bbox3f::Corner::max, vm::bbox3f::Corner::min, vm::bbox3f::Corner::max));
    ASSERT_VEC_EQ(vm::vec3f( 1.0f,  3.0f, -3.0f), bounds.corner(vm::bbox3f::Corner::max, vm::bbox3f::Corner::max, vm::bbox3f::Corner::min));
    ASSERT_VEC_EQ(vm::vec3f( 1.0f,  3.0f,  5.0f), bounds.corner(vm::bbox3f::Corner::max, vm::bbox3f::Corner::max, vm::bbox3f::Corner::max));
}

TEST(BBoxTest, mergeWithBBox) {
    const vm::bbox3f bounds1(vm::vec3f(-12.0f, -3.0f, 4.0f), vm::vec3f(7.0f, 8.0f, 9.0f));
    const vm::bbox3f bounds2(vm::vec3f(-10.0f, -5.0f, 3.0f), vm::vec3f(9.0f, 9.0f, 5.0f));
    const vm::bbox3f merged( vm::vec3f(-12.0f, -5.0f, 3.0f), vm::vec3f(9.0f, 9.0f, 9.0f));
    
    ASSERT_EQ(merged, merge(bounds1, bounds2));
}

TEST(BBoxTest, mergeWithVec) {
    vm::bbox3f bounds(vm::vec3f(-12.0f, -3.0f, 4.0f), vm::vec3f(7.0f, 8.0f,  9.0f));
    const vm::vec3f  vec(-10.0f, -6.0f, 10.0f);
    const vm::bbox3f merged(vm::vec3f(-12.0f, -6.0f, 4.0f), vm::vec3f(7.0f, 8.0f, 10.0f));

    ASSERT_EQ(merged, merge(bounds, vec));
}

TEST(BBoxTest, containsPoint) {
    const vm::bbox3f bounds(vm::vec3f(-12.0f, -3.0f,  4.0f), vm::vec3f( 8.0f, 9.0f, 8.0f));
    ASSERT_TRUE(bounds.contains(vm::vec3f(2.0f, 1.0f, 7.0f)));
    ASSERT_TRUE(bounds.contains(vm::vec3f(-12.0f, -3.0f, 7.0f)));
    ASSERT_FALSE(bounds.contains(vm::vec3f(-13.0f, -3.0f, 7.0f)));
}

TEST(BBoxTest, relativePosition) {
    const vm::bbox3f bounds(vm::vec3f(-12.0f, -3.0f,  4.0f), vm::vec3f( 8.0f, 9.0f, 8.0f));
    const vm::vec3f point1(-1.0f, 0.0f, 0.0f);
    const auto pos1 = bounds.relativePosition(point1);
    ASSERT_EQ(vm::bbox3f::Range::within, pos1[0]);
    ASSERT_EQ(vm::bbox3f::Range::within, pos1[1]);
    ASSERT_EQ(vm::bbox3f::Range::less,   pos1[2]);
}

TEST(BBoxTest, containsBBox) {
    const vm::bbox3f bounds1(vm::vec3f(-12.0f, -3.0f,  4.0f), vm::vec3f( 8.0f, 9.0f, 8.0f));
    const vm::bbox3f bounds2(vm::vec3f(-10.0f, -2.0f,  5.0f), vm::vec3f( 7.0f, 8.0f, 7.0f));
    const vm::bbox3f bounds3(vm::vec3f(-13.0f, -2.0f,  5.0f), vm::vec3f( 7.0f, 8.0f, 7.0f));
    ASSERT_TRUE(bounds1.contains(bounds1));
    ASSERT_TRUE(bounds1.contains(bounds2));
    ASSERT_FALSE(bounds1.contains(bounds3));
}

TEST(BBoxTest, enclosesBBox) {
    const vm::bbox3f bounds1(vm::vec3f(-12.0f, -3.0f,  4.0f), vm::vec3f( 8.0f, 9.0f, 8.0f));
    const vm::bbox3f bounds2(vm::vec3f(-10.0f, -2.0f,  5.0f), vm::vec3f( 7.0f, 8.0f, 7.0f));
    const vm::bbox3f bounds3(vm::vec3f(-10.0f, -3.0f,  5.0f), vm::vec3f( 7.0f, 8.0f, 7.0f));
    ASSERT_FALSE(bounds1.encloses(bounds1));
    ASSERT_TRUE(bounds1.encloses(bounds2));
    ASSERT_FALSE(bounds1.encloses(bounds3));
}

TEST(BBoxTest, intersectsBBox) {
    const vm::bbox3f bounds1(vm::vec3f(-12.0f, -3.0f,  4.0f), vm::vec3f(  8.0f,  9.0f,  8.0f));
    const vm::bbox3f bounds2(vm::vec3f(-10.0f, -2.0f,  5.0f), vm::vec3f(  7.0f,  8.0f,  7.0f));
    const vm::bbox3f bounds3(vm::vec3f(-13.0f, -2.0f,  5.0f), vm::vec3f(  7.0f,  8.0f,  7.0f));
    const vm::bbox3f bounds4(vm::vec3f(-15.0f, 10.0f,  9.0f), vm::vec3f(-13.0f, 12.0f, 10.0f));
    const vm::bbox3f bounds5(vm::vec3f(-15.0f, 10.0f,  9.0f), vm::vec3f(-12.0f, 12.0f, 10.0f));
    ASSERT_TRUE(bounds1.intersects(bounds1));
    ASSERT_TRUE(bounds1.intersects(bounds2));
    ASSERT_TRUE(bounds1.intersects(bounds3));
    ASSERT_FALSE(bounds1.intersects(bounds4));
    ASSERT_FALSE(bounds1.intersects(bounds5));
}

TEST(BBoxTest, expand) {
          vm::bbox3f bounds  (vm::vec3f(-12.0f, -3.0f,  4.0f), vm::vec3f( 8.0f,  9.0f,  8.0f));
    const vm::bbox3f expanded(vm::vec3f(-14.0f, -5.0f,  2.0f), vm::vec3f(10.0f, 11.0f, 10.0f));
    ASSERT_EQ(expanded, bounds.expand(2.0f));
}

TEST(BBoxTest, translate) {
          vm::bbox3f bounds    (vm::vec3f(-12.0f, -3.0f,  4.0f), vm::vec3f( 8.0f, 9.0f, 8.0f));
    const vm::bbox3f translated(vm::vec3f(-10.0f, -4.0f,  1.0f), vm::vec3f(10.0f, 8.0f, 5.0f));
    ASSERT_EQ(translated, bounds.translate(vm::vec3f(2.0f, -1.0f, -3.0f)));
}

TEST(BBoxTest, constrain) {
    const vm::bbox3d bounds (1024.0);
    ASSERT_VEC_EQ(vm::vec3d::zero, bounds.constrain(vm::vec3d::zero));
    ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min));
    ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min + vm::vec3d::neg_x));
    ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min + vm::vec3d::neg_y));
    ASSERT_VEC_EQ(bounds.min, bounds.constrain(bounds.min + vm::vec3d::neg_z));
    ASSERT_VEC_EQ(bounds.max, bounds.constrain(bounds.max + vm::vec3d::pos_x));
    ASSERT_VEC_EQ(bounds.max, bounds.constrain(bounds.max + vm::vec3d::pos_y));
    ASSERT_VEC_EQ(bounds.max, bounds.constrain(bounds.max + vm::vec3d::pos_z));
}
