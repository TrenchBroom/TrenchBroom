/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#include "test_utils.h"

#include <vecmath/approx.h>
#include <vecmath/constexpr_util.h>
#include <vecmath/forward.h>
#include <vecmath/intersection.h>
#include <vecmath/quat.h>
#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>
#include <vecmath/vec_io.h>

#include <array>

#define CATCH_CONFIG_ENABLE_ALL_STRINGMAKERS 1
#include <catch2/catch.hpp>

namespace vm
{
bool lineOnPlane(const plane3f& plane, const line3f& line);

template <typename C>
constexpr bool containsPoint(const C& vertices, const vec3d& point)
{
  return polygon_contains_point(
    point, vec3d::pos_z(), std::begin(vertices), std::end(vertices));
}

constexpr std::array<vec3d, 4> square()
{
  return {
    vec3d(-1.0, -1.0, 0.0),
    vec3d(-1.0, +1.0, 0.0),
    vec3d(+1.0, +1.0, 0.0),
    vec3d(+1.0, -1.0, 0.0)};
}

constexpr std::array<vec3d, 3> triangle()
{
  return {
    vec3d(-1.0, +1.0, 0.0), // top
    vec3d(-1.0, -1.0, 0.0), // left bottom
    vec3d(+1.0, -1.0, 0.0), // right bottom
  };
}

TEST_CASE("intersection.square_contains_center")
{
  CER_CHECK(containsPoint(square(), vec3d(0.0, 0.0, 0.0)));
}

TEST_CASE("intersection.square_contains_corner_top_left")
{
  CER_CHECK(containsPoint(square(), vec3d(-1.0, +1.0, 0.0)));
}

TEST_CASE("intersection.square_contains_corner_top_right")
{
  CER_CHECK(containsPoint(square(), vec3d(+1.0, +1.0, 0.0)));
}

TEST_CASE("intersection.square_contains_corner_bottom_right")
{
  CER_CHECK(containsPoint(square(), vec3d(+1.0, -1.0, 0.0)));
}

TEST_CASE("intersection.square_contains_corner_bottom_left")
{
  CER_CHECK(containsPoint(square(), vec3d(-1.0, -1.0, 0.0)));
}

TEST_CASE("intersection.square_contains_edge_center_left")
{
  CER_CHECK(containsPoint(square(), vec3d(-1.0, 0.0, 0.0)));
}

TEST_CASE("intersection.square_contains_edge_center_top")
{
  CER_CHECK(containsPoint(square(), vec3d(0.0, +1.0, 0.0)));
}

TEST_CASE("intersection.square_contains_edge_center_right")
{
  CER_CHECK(containsPoint(square(), vec3d(+1.0, 0.0, 0.0)));
}

TEST_CASE("intersection.square_contains_edge_center_bottom")
{
  CER_CHECK(containsPoint(square(), vec3d(0.0, -1.0, 0.0)));
}

TEST_CASE("intersection.triangle_contains_origin")
{
  CER_CHECK(containsPoint(triangle(), vec3d(0.0, 0.0, 0.0)));
}

TEST_CASE("intersection.triangle_contains_corner_top")
{
  CER_CHECK(containsPoint(triangle(), vec3d(-1.0, +1.0, 0.0)));
}

TEST_CASE("intersection.triangle_contains_corner_left")
{
  CER_CHECK(containsPoint(triangle(), vec3d(-1.0, -1.0, 0.0)));
}

TEST_CASE("intersection.triangle_contains_corner_right")
{
  CER_CHECK(containsPoint(triangle(), vec3d(+1.0, -1.0, 0.0)));
}

TEST_CASE("intersection.triangle_contains_edge_center_top_left")
{
  CER_CHECK(containsPoint(triangle(), (triangle()[0] + triangle()[1]) / 2.0));
}

TEST_CASE("intersection.triangle_contains_edge_center_top_right")
{
  CER_CHECK(containsPoint(triangle(), (triangle()[1] + triangle()[2]) / 2.0));
}

TEST_CASE("intersection.triangle_contains_edge_center_bottom")
{
  CER_CHECK(containsPoint(triangle(), (triangle()[2] + triangle()[0]) / 2.0));
}

TEST_CASE("intersection.triangle_contains_outer_point"){
  CER_CHECK_FALSE(containsPoint(triangle(), vec3d(+1.0, +1.0, 0.0)))}

TEST_CASE("intersection.intersect_ray_plane")
{
  constexpr auto ray = ray3f(vec3f::zero(), vec3f::pos_z());
  CER_CHECK(
    is_nan(intersect_ray_plane(ray, plane3f(vec3f(0.0f, 0.0f, -1.0f), vec3f::pos_z()))));
  CER_CHECK(
    intersect_ray_plane(ray, plane3f(vec3f(0.0f, 0.0f, 0.0f), vec3f::pos_z()))
    == approx(0.0f));
  CER_CHECK(
    intersect_ray_plane(ray, plane3f(vec3f(0.0f, 0.0f, 1.0f), vec3f::pos_z()))
    == approx(1.0f));
}

TEST_CASE("intersection.intersect_ray_triangle")
{
  constexpr auto p0 = vec3d(2.0, 5.0, 2.0);
  constexpr auto p1 = vec3d(4.0, 7.0, 2.0);
  constexpr auto p2 = vec3d(3.0, 2.0, 2.0);

  CER_CHECK(
    is_nan(intersect_ray_triangle(ray3d(vec3d::zero(), vec3d::pos_x()), p0, p1, p2)));
  CER_CHECK(
    is_nan(intersect_ray_triangle(ray3d(vec3d::zero(), vec3d::pos_y()), p0, p1, p2)));
  CER_CHECK(
    is_nan(intersect_ray_triangle(ray3d(vec3d::zero(), vec3d::pos_z()), p0, p1, p2)));
  CER_CHECK(is_nan(
    intersect_ray_triangle(ray3d(vec3d(0.0, 0.0, 2.0), vec3d::pos_y()), p0, p1, p2)));
  CER_CHECK(
    intersect_ray_triangle(ray3d(vec3d(3.0, 5.0, 0.0), vec3d::pos_z()), p0, p1, p2)
    == approx(2.0));
  CER_CHECK(
    intersect_ray_triangle(ray3d(vec3d(2.0, 5.0, 0.0), vec3d::pos_z()), p0, p1, p2)
    == approx(2.0));
  CER_CHECK(
    intersect_ray_triangle(ray3d(vec3d(4.0, 7.0, 0.0), vec3d::pos_z()), p0, p1, p2)
    == approx(2.0));
  CER_CHECK(
    intersect_ray_triangle(ray3d(vec3d(3.0, 2.0, 0.0), vec3d::pos_z()), p0, p1, p2)
    == approx(2.0));
}

TEST_CASE("intersection.intersect_ray_square")
{
  constexpr auto poly = square() + vec3d(0, 0, 1);

  CER_CHECK(is_nan(intersect_ray_polygon(
    ray3d(vec3d::zero(), vec3d::neg_z()),
    plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
    std::begin(poly),
    std::end(poly))));
  CER_CHECK(is_nan(intersect_ray_polygon(
    ray3d(vec3d(2, 2, 0), vec3d::pos_z()),
    plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
    std::begin(poly),
    std::end(poly))));
  CER_CHECK(is_nan(intersect_ray_polygon(
    ray3d(vec3d(-2, 0, 1), vec3d::pos_x()),
    plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
    std::begin(poly),
    std::end(poly))));
  CER_CHECK(is_nan(intersect_ray_polygon(
    ray3d(vec3d(-2, 0, 0), vec3d::pos_x()),
    plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
    std::begin(poly),
    std::end(poly))));

  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(0, 0, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(0, 0, 2), vec3d::neg_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(+1, +1, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(+1, -1, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(-1, +1, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(-1, -1, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(0, +1, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(0, -1, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(+1, 0, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
  CER_CHECK(
    intersect_ray_polygon(
      ray3d(vec3d(-1, 0, 0), vec3d::pos_z()),
      plane3d(vec3d(0, 0, 1), vec3d::pos_z()),
      std::begin(poly),
      std::end(poly))
    == approx(+1.0));
}

TEST_CASE("intersection.intersect_ray_bbox")
{
  constexpr auto bounds = bbox3f(vec3f(-12.0f, -3.0f, 4.0f), vec3f(8.0f, 9.0f, 8.0f));

  CER_CHECK(is_nan(intersect_ray_bbox(ray3f(vec3f::zero(), vec3f::neg_z()), bounds)));
  CER_CHECK(
    intersect_ray_bbox(ray3f(vec3f::zero(), vec3f::pos_z()), bounds) == approx(4.0f));

  constexpr auto origin = vec3f(-10.0f, -7.0f, 14.0f);
  constexpr auto diff = vec3f(-2.0f, 3.0f, 8.0f) - origin;
  constexpr auto dir = normalize_c(diff);
  CHECK(intersect_ray_bbox(ray3f(origin, dir), bounds) == approx(length(diff)));
}

TEST_CASE("intersection.intersect_ray_sphere")
{
  const ray3f ray(vec3f::zero(), vec3f::pos_z());

  // ray originates inside sphere and hits at north pole
  CHECK(intersect_ray_sphere(ray, vec3f::zero(), 2.0f) == approx(2.0f));

  // ray originates outside sphere and hits at south pole
  CHECK(intersect_ray_sphere(ray, vec3f(0.0f, 0.0f, 5.0f), 2.0f) == approx(3.0f));

  // miss
  CHECK(is_nan(intersect_ray_sphere(ray, vec3f(3.0f, 2.0f, 2.0f), 1.0f)));
}

TEST_CASE("intersection.intersect_ray_torus")
{
  CHECK(
    intersect_ray_torus(ray3f(vec3f::zero(), vec3f::pos_y()), vec3f::zero(), 5.0f, 1.0f)
    == approx(4.0f));
  CHECK(
    intersect_ray_torus(ray3f(vec3f::zero(), vec3f::pos_x()), vec3f::zero(), 5.0f, 1.0f)
    == approx(4.0f));

  CHECK(
    intersect_ray_torus(
      ray3f(vec3f(0.0f, -10.0f, 0.0f), vec3f::pos_y()), vec3f::zero(), 5.0f, 1.0f)
    == approx(4.0f));
  CHECK(
    intersect_ray_torus(
      ray3f(vec3f(-10.0f, 0.0f, 0.0f), vec3f::pos_x()), vec3f::zero(), 5.0f, 1.0f)
    == approx(4.0f));

  CHECK(
    intersect_ray_torus(
      ray3f(vec3f(0.0f, -5.0f, 5.0f), vec3f::neg_z()), vec3f::zero(), 5.0f, 1.0f)
    == approx(4.0f));

  CHECK(
    intersect_ray_torus(
      ray3f(vec3f(5.0f, -5.0f, 5.0f), vec3f::neg_z()),
      vec3f(5.0f, 0.0f, 0.0f),
      5.0f,
      1.0f)
    == approx(4.0f));

  CHECK(is_nan(intersect_ray_torus(
    ray3f(vec3f::zero(), vec3f::pos_z()), vec3f::zero(), 5.0f, 1.0f)));
}

TEST_CASE("intersection.intersect_line_plane")
{
  constexpr auto p = plane3f(5.0f, vec3f::pos_z());
  constexpr auto l = line3f(vec3f(0, 0, 15), normalize_c(vec3f(1, 0, -1)));
  CER_CHECK(point_at_distance(l, intersect_line_plane(l, p)) == approx(vec3f(10, 0, 5)));
}

TEST_CASE("intersection.intersect_plane_plane")
{
  constexpr auto p1 = plane3f(10.0f, vec3f::pos_z());
  constexpr auto p2 = plane3f(20.0f, vec3f::pos_x());
  const auto line = intersect_plane_plane(p1, p2);

  CHECK(lineOnPlane(p1, line));
  CHECK(lineOnPlane(p2, line));
}

TEST_CASE("intersection.intersect_plane_plane_parallel")
{
  constexpr auto p1 = plane3f(10.0f, vec3f::pos_z());
  constexpr auto p2 = plane3f(11.0f, vec3f::pos_z());
  const line3f line = intersect_plane_plane(p1, p2);

  CHECK(line.direction == vec3f::zero());
  CHECK(line.point == vec3f::zero());
}

TEST_CASE("intersection.intersect_plane_plane_similar")
{
  constexpr auto anchor = vec3f(100, 100, 100);
  constexpr auto p1 = plane3f(anchor, vec3f::pos_x());
  const auto p2 = plane3f(
    anchor,
    quatf(vec3f::neg_y(), to_radians(0.5f))
      * vec3f::pos_x()); // p1 rotated by 0.5 degrees
  const auto line = intersect_plane_plane(p1, p2);

  CHECK(lineOnPlane(p1, line));
  CHECK(lineOnPlane(p2, line));
}

TEST_CASE("intersection.intersect_plane_plane_too_similar")
{
  constexpr auto anchor = vec3f(100, 100, 100);
  constexpr auto p1 = plane3f(anchor, vec3f::pos_x());
  const auto p2 = plane3f(
    anchor,
    quatf(vec3f::neg_y(), to_radians(0.0001f))
      * vec3f::pos_x()); // p1 rotated by 0.0001 degrees
  const auto line = intersect_plane_plane(p1, p2);

  CHECK(line.direction == vec3f::zero());
  CHECK(line.point == vec3f::zero());
}

bool lineOnPlane(const plane3f& plane, const line3f& line)
{
  if (plane.point_status(line.point) != plane_status::inside)
  {
    return false;
  }
  else if (plane.point_status(point_at_distance(line, 16.0f)) != plane_status::inside)
  {
    return false;
  }
  else
  {
    return true;
  }
}

TEST_CASE("intersection.polygon_clip_by_plane")
{
  constexpr auto poly = square();

  constexpr auto plane1 = plane3d{{0, 0, 0}, vec3d::pos_z()};
  constexpr auto plane2 = plane3d{{0, 1, 0}, vec3d::pos_z()};
  constexpr auto plane5 = plane3d{{0, -1, 0}, -vec3d::pos_z()};

  constexpr auto plane3 = plane3d{{0, 0, 0}, vec3d::pos_x()};
  const auto [_, plane4] =
    vm::from_points(vec3d{-1, -1, 0}, vec3d{1, 1, 0}, vec3d{0, 0, 1});

  SECTION("no clipping")
  {
    CHECK(polygon_clip_by_plane(plane1, std::begin(poly), std::end(poly)).empty());
    CHECK(polygon_clip_by_plane(plane2, std::begin(poly), std::end(poly)).empty());
    CHECK(polygon_clip_by_plane(plane5, std::begin(poly), std::end(poly)).empty());
  }

  SECTION("clipping")
  {
    // split into two rectangles
    CHECK(
      polygon_clip_by_plane(plane3, std::begin(poly), std::end(poly))
      == std::vector<vec3d>{
        {-1, -1, 0},
        {-1, 1, 0},
        {0, 1, 0},
        {0, -1, 0},
      });

    // split into two triangles
    CHECK(
      polygon_clip_by_plane(plane4, std::begin(poly), std::end(poly))
      == std::vector<vec3d>{
        {-1, -1, 0},
        {1, 1, 0},
        {1, -1, 0},
      });
  }
}
} // namespace vm
