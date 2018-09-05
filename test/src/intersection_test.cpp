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

#include "vec_decl.h"
#include "vec_impl.h"
#include "mat_decl.h"
#include "mat_impl.h"
#include "quat_decl.h"
#include "quat_impl.h"
#include "bbox_decl.h"
#include "bbox_impl.h"
#include "ray_decl.h"
#include "ray_impl.h"
#include "line_decl.h"
#include "line_impl.h"
#include "plane_decl.h"
#include "plane_impl.h"
#include "intersection.h"

bool lineOnPlane(const plane3f& plane, const line3f& line);

TEST(IntersectionTest, intersectRayAndPlane) {
    const ray3f ray(vec3f::zero, vec3f::pos_z);
    ASSERT_TRUE(Math::isnan(intersect(ray, plane3f(vec3f(0.0f, 0.0f, -1.0f), vec3f::pos_z))));
    ASSERT_FLOAT_EQ(0.0f, intersect(ray, plane3f(vec3f(0.0f, 0.0f,  0.0f), vec3f::pos_z)));
    ASSERT_FLOAT_EQ(1.0f, intersect(ray, plane3f(vec3f(0.0f, 0.0f,  1.0f), vec3f::pos_z)));
}

TEST(IntersectionTest, intersectRayAndTriangle) {
    const vec3d p0(2.0, 5.0, 2.0);
    const vec3d p1(4.0, 7.0, 2.0);
    const vec3d p2(3.0, 2.0, 2.0);

    ASSERT_TRUE(Math::isnan(intersect(ray3d(vec3d::zero, vec3d::pos_x), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersect(ray3d(vec3d::zero, vec3d::pos_y), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersect(ray3d(vec3d::zero, vec3d::pos_z), p0, p1, p2)));
    ASSERT_TRUE(Math::isnan(intersect(ray3d(vec3d(0.0, 0.0, 2.0), vec3d::pos_y), p0, p1, p2)));
    ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(3.0, 5.0, 0.0), vec3d::pos_z), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(2.0, 5.0, 0.0), vec3d::pos_z), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(4.0, 7.0, 0.0), vec3d::pos_z), p0, p1, p2));
    ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(3.0, 2.0, 0.0), vec3d::pos_z), p0, p1, p2));
}

TEST(IntersectionTest, intersectRayAndBBox) {
    const bbox3f bounds(vec3f(-12.0f, -3.0f,  4.0f), vec3f(  8.0f,  9.0f,  8.0f));

    float distance = intersect(ray3f(vec3f::zero, vec3f::neg_z), bounds);
    ASSERT_TRUE(Math::isnan(distance));

    distance = intersect(ray3f(vec3f::zero, vec3f::pos_z), bounds);
    ASSERT_FALSE(Math::isnan(distance));
    ASSERT_FLOAT_EQ(4.0f, distance);

    const vec3f origin = vec3f(-10.0f, -7.0f, 14.0f);
    const vec3f diff = vec3f(-2.0f, 3.0f, 8.0f) - origin;
    const vec3f dir = normalize(diff);
    distance = intersect(ray3f(origin, dir), bounds);
    ASSERT_FALSE(Math::isnan(distance));
    ASSERT_FLOAT_EQ(length(diff), distance);

}

TEST(IntersectionTest, intersectRayAndSphere) {
    const ray3f ray(vec3f::zero, vec3f::pos_z);

    // ray originates inside sphere and hits at north pole
    ASSERT_FLOAT_EQ(2.0f, intersect(ray, vec3f::zero, 2.0f));

    // ray originates outside sphere and hits at south pole
    ASSERT_FLOAT_EQ(3.0f, intersect(ray, vec3f(0.0f, 0.0f, 5.0f), 2.0f));

    // miss
    ASSERT_TRUE(Math::isnan(intersect(ray, vec3f(3.0f, 2.0f, 2.0f), 1.0f)));
}

TEST(IntersectionTest, intersectLineAndPlane) {
    const plane3f p(5.0f, vec3f::pos_z);
    const line3f l(vec3f(0, 0, 15), normalize(vec3f(1,0,-1)));

    const vec3f intersection = l.pointAtDistance(intersect(l, p));
    ASSERT_FLOAT_EQ(10, intersection.x());
    ASSERT_FLOAT_EQ(0, intersection.y());
    ASSERT_FLOAT_EQ(5, intersection.z());
}

TEST(IntersectionTest, intersectPlaneAndPlane_parallel) {
    const plane3f p1(10.0f, vec3f::pos_z);
    const plane3f p2(11.0f, vec3f::pos_z);
    const line3f line = intersect(p1, p2);;

    ASSERT_EQ(vec3f::zero, line.direction);
    ASSERT_EQ(vec3f::zero, line.point);
}

TEST(IntersectionTest, intersectPlaneAndPlane_too_similar) {
    const vec3f anchor(100,100,100);
    const plane3f p1(anchor, vec3f::pos_x);
    const plane3f p2(anchor, quatf(vec3f::neg_y, Math::radians(0.0001f)) * vec3f::pos_x); // p1 rotated by 0.0001 degrees
    const line3f line = intersect(p1, p2);;

    ASSERT_EQ(vec3f::zero, line.direction);
    ASSERT_EQ(vec3f::zero, line.point);
}

TEST(IntersectionTest, intersectPlaneAndPlane) {
    const plane3f p1(10.0f, vec3f::pos_z);
    const plane3f p2(20.0f, vec3f::pos_x);
    const line3f line = intersect(p1, p2);;

    ASSERT_TRUE(lineOnPlane(p1, line));
    ASSERT_TRUE(lineOnPlane(p2, line));
}

TEST(IntersectionTest, intersectPlaneAndPlane_similar) {
    const vec3f anchor(100,100,100);
    const plane3f p1(anchor, vec3f::pos_x);
    const plane3f p2(anchor, quatf(vec3f::neg_y, Math::radians(0.5f)) * vec3f::pos_x); // p1 rotated by 0.5 degrees
    const line3f line = intersect(p1, p2);;

    ASSERT_TRUE(lineOnPlane(p1, line));
    ASSERT_TRUE(lineOnPlane(p2, line));
}

bool lineOnPlane(const plane3f& plane, const line3f& line) {
    if (plane.pointStatus(line.point) != Math::PointStatus::PSInside){
        return false;
    } else if (plane.pointStatus(line.pointAtDistance(16.0f)) != Math::PointStatus::PSInside) {
        return false;
    } else {
        return true;
    }
}
