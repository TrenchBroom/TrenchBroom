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

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/quat.h>
#include <vecmath/bbox.h>
#include <vecmath/ray.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>
#include <vecmath/util.h>
#include <vecmath/intersection.h>

// TODO 2201: write more tests

namespace vm {
    bool lineOnPlane(const plane3f& plane, const line3f& line);

    bool containsPoint(const std::vector<vec3d>& vertices, const vec3d& point);

    std::vector<vec3d> square();
    std::vector<vec3d> triangle();

    TEST(IntersectionTest, intersectRayAndPlane) {
        const ray3f ray(vec3f::zero, vec3f::pos_z);
        ASSERT_TRUE(isNan(intersect(ray, plane3f(vec3f(0.0f, 0.0f, -1.0f), vec3f::pos_z))));
        ASSERT_FLOAT_EQ(0.0f, intersect(ray, plane3f(vec3f(0.0f, 0.0f,  0.0f), vec3f::pos_z)));
        ASSERT_FLOAT_EQ(1.0f, intersect(ray, plane3f(vec3f(0.0f, 0.0f,  1.0f), vec3f::pos_z)));
    }

    TEST(IntersectionTest, intersectRayAndTriangle) {
        const vec3d p0(2.0, 5.0, 2.0);
        const vec3d p1(4.0, 7.0, 2.0);
        const vec3d p2(3.0, 2.0, 2.0);

        ASSERT_TRUE(isNan(intersect(ray3d(vec3d::zero, vec3d::pos_x), p0, p1, p2)));
        ASSERT_TRUE(isNan(intersect(ray3d(vec3d::zero, vec3d::pos_y), p0, p1, p2)));
        ASSERT_TRUE(isNan(intersect(ray3d(vec3d::zero, vec3d::pos_z), p0, p1, p2)));
        ASSERT_TRUE(isNan(intersect(ray3d(vec3d(0.0, 0.0, 2.0), vec3d::pos_y), p0, p1, p2)));
        ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(3.0, 5.0, 0.0), vec3d::pos_z), p0, p1, p2));
        ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(2.0, 5.0, 0.0), vec3d::pos_z), p0, p1, p2));
        ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(4.0, 7.0, 0.0), vec3d::pos_z), p0, p1, p2));
        ASSERT_DOUBLE_EQ(2.0, intersect(ray3d(vec3d(3.0, 2.0, 0.0), vec3d::pos_z), p0, p1, p2));
    }

    TEST(IntersectionTest, intersectRayAndBBox) {
        const bbox3f bounds(vec3f(-12.0f, -3.0f,  4.0f), vec3f(  8.0f,  9.0f,  8.0f));

        float distance = intersect(ray3f(vec3f::zero, vec3f::neg_z), bounds);
        ASSERT_TRUE(isNan(distance));

        distance = intersect(ray3f(vec3f::zero, vec3f::pos_z), bounds);
        ASSERT_FALSE(isNan(distance));
        ASSERT_FLOAT_EQ(4.0f, distance);

        const vec3f origin = vec3f(-10.0f, -7.0f, 14.0f);
        const vec3f diff = vec3f(-2.0f, 3.0f, 8.0f) - origin;
        const vec3f dir = normalize(diff);
        distance = intersect(ray3f(origin, dir), bounds);
        ASSERT_FALSE(isNan(distance));
        ASSERT_FLOAT_EQ(length(diff), distance);

    }

    TEST(IntersectionTest, intersectRayAndSphere) {
        const ray3f ray(vec3f::zero, vec3f::pos_z);

        // ray originates inside sphere and hits at north pole
        ASSERT_FLOAT_EQ(2.0f, intersect(ray, vec3f::zero, 2.0f));

        // ray originates outside sphere and hits at south pole
        ASSERT_FLOAT_EQ(3.0f, intersect(ray, vec3f(0.0f, 0.0f, 5.0f), 2.0f));

        // miss
        ASSERT_TRUE(isNan(intersect(ray, vec3f(3.0f, 2.0f, 2.0f), 1.0f)));
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
        const plane3f p2(anchor, quatf(vec3f::neg_y, radians(0.0001f)) * vec3f::pos_x); // p1 rotated by 0.0001 degrees
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
        const plane3f p2(anchor, quatf(vec3f::neg_y, radians(0.5f)) * vec3f::pos_x); // p1 rotated by 0.5 degrees
        const line3f line = intersect(p1, p2);;

        ASSERT_TRUE(lineOnPlane(p1, line));
        ASSERT_TRUE(lineOnPlane(p2, line));
    }

    TEST(IntersectionTest, squareContainsPointInCenter) {
        ASSERT_TRUE(containsPoint(square(), vec3d(0.0, 0.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsLeftTopVertex) {
        ASSERT_TRUE(containsPoint(square(), vec3d(-1.0, +1.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsRightTopVertex) {
        ASSERT_TRUE(containsPoint(square(), vec3d(+1.0, +1.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsRightBottomVertex) {
        ASSERT_TRUE(containsPoint(square(), vec3d(+1.0, -1.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsLeftBottomVertex) {
        ASSERT_TRUE(containsPoint(square(), vec3d(-1.0, -1.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsCenterOfLeftEdge) {
        ASSERT_TRUE(containsPoint(square(), vec3d(-1.0, 0.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsCenterOfTopEdge) {
        ASSERT_TRUE(containsPoint(square(), vec3d(0.0, +1.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsCenterOfRightEdge) {
        ASSERT_TRUE(containsPoint(square(), vec3d(+1.0, 0.0, 0.0)));
    }

    TEST(IntersectionTest, squareContainsCenterOfBottomEdge) {
        ASSERT_TRUE(containsPoint(square(), vec3d(0.0, -1.0, 0.0)));
    }

    TEST(IntersectionTest, triangleContainsOrigin) {
        ASSERT_TRUE(containsPoint(triangle(), vec3d(0.0, 0.0, 0.0)));
    }

    TEST(IntersectionTest, triangleContainsTopPoint) {
        ASSERT_TRUE(containsPoint(triangle(), vec3d(-1.0, +1.0, 0.0)));
    }

    TEST(IntersectionTest, triangleContainsLeftBottomPoint) {
        ASSERT_TRUE(containsPoint(triangle(), vec3d(-1.0, -1.0, 0.0)));
    }

    TEST(IntersectionTest, triangleContainsRightBottomPoint) {
        ASSERT_TRUE(containsPoint(triangle(), vec3d(+1.0, -1.0, 0.0)));
    }

    TEST(IntersectionTest, triangleContainsCenterOfTopToLeftBottomEdge) {
        ASSERT_TRUE(containsPoint(triangle(), (triangle()[0] + triangle()[1]) / 2.0));
    }

    TEST(IntersectionTest, triangleContainsCenterOfLeftBottomToRightBottomEdge) {
        ASSERT_TRUE(containsPoint(triangle(), (triangle()[1] + triangle()[2]) / 2.0));
    }

    TEST(IntersectionTest, triangleContainsCenterOfRightBottomToTopEdge) {
        ASSERT_TRUE(containsPoint(triangle(), (triangle()[2] + triangle()[0]) / 2.0));
    }

    TEST(IntersectionTest, triangleContainsOuterPoint) {
        ASSERT_FALSE(containsPoint(triangle(), vec3d(+1.0, +1.0, 0.0)));
    }

    bool containsPoint(const std::vector<vec3d>& vertices, const vec3d& point) {
        return contains(point, std::begin(vertices), std::end(vertices));
    }

    std::vector<vec3d> square() {
        return std::vector<vec3d> {
                vec3d(-1.0, -1.0, 0.0),
                vec3d(-1.0, +1.0, 0.0),
                vec3d(+1.0, +1.0, 0.0),
                vec3d(+1.0, -1.0, 0.0)
        };
    }

    std::vector<vec3d> triangle() {
        return std::vector<vec3d> {
                vec3d(-1.0, +1.0, 0.0), // top
                vec3d(-1.0, -1.0, 0.0), // left bottom
                vec3d(+1.0, -1.0, 0.0), // right bottom
        };
    }

    bool lineOnPlane(const plane3f& plane, const line3f& line) {
        if (plane.pointStatus(line.point) != point_status::inside){
            return false;
        } else if (plane.pointStatus(line.pointAtDistance(16.0f)) != point_status::inside) {
            return false;
        } else {
            return true;
        }
    }
}
