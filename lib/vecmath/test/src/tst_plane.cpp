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

#include <vecmath/approx.h>
#include <vecmath/constexpr_util.h>
#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/plane.h>
#include <vecmath/plane_io.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "test_utils.h"

#include <array>
#include <sstream>

#include <catch2/catch_all.hpp>

namespace vm
{
TEST_CASE("plane.constructor_default")
{
  constexpr auto p = plane3f();
  CER_CHECK(p.distance == 0.0f);
  CER_CHECK(p.normal == vec3f::zero());
}

TEST_CASE("plane.constructor_convert")
{
  constexpr auto p = plane3d(1.0, vec3d::pos_z());
  constexpr auto q = plane3f(p);
  CER_CHECK(q.distance == approx(1.0f));
  CER_CHECK(q.normal == approx(vec3f::pos_z()));
}

TEST_CASE("plane.constructor_with_distance_and_normal")
{
  constexpr auto d = 123.0f;
  constexpr auto n = normalize_c(vec3f(1.0f, 2.0f, 3.0f));
  constexpr auto p = plane3f(d, n);
  CER_CHECK(p.distance == approx(d));
  CER_CHECK(p.normal == approx(n));
}

TEST_CASE("plane.constructor_with_anchor_and_normal")
{
  constexpr auto a = vec3f(-2038.034f, 0.0023f, 32.0f);
  constexpr auto n = normalize_c(vec3f(9.734f, -3.393f, 2.033f));
  constexpr auto p = plane3f(a, n);
  CER_CHECK(p.distance == approx(dot(a, n)));
  CER_CHECK(p.normal == approx(n));
}

TEST_CASE("plane.anchor")
{
  constexpr auto a = vec3f(-2038.034f, 0.0023f, 32.0f);
  constexpr auto n = normalize_c(vec3f(9.734f, -3.393f, 2.033f));
  constexpr auto p = plane3f(a, n);
  CER_CHECK(p.anchor() == approx(p.distance * n));
}

TEST_CASE("plane.at")
{
  constexpr auto a = vec3f(-2038.034f, 0.0023f, 32.0f);
  constexpr auto n = normalize_c(vec3f(9.734f, -3.393f, 2.033f));
  constexpr auto p = plane3f(a, n);
  constexpr auto point1 = vec2f(27.022f, -12.0123223f);

  CER_CHECK(
    p.at(point1, axis::x)
    == approx(
      (p.distance - point1.x() * p.normal.y() - point1.y() * p.normal.z())
      / p.normal[axis::x]));
  CER_CHECK(
    p.at(point1, axis::y)
    == approx(
      (p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.z())
      / p.normal[axis::y]));
  CER_CHECK(
    p.at(point1, axis::z)
    == approx(
      (p.distance - point1.x() * p.normal.x() - point1.y() * p.normal.y())
      / p.normal[axis::z]));
}

TEST_CASE("plane.at_parallel_planes")
{
  constexpr auto p1 = plane3f(10.0f, vec3f::pos_x());

  CER_CHECK(p1.at(vec2f(2.0f, 1.0f), axis::x) == approx(p1.distance));
  CER_CHECK(p1.at(vec2f(22.0f, -34322.0232f), axis::x) == approx(p1.distance));
  CER_CHECK(p1.at(vec2f(2.0f, 1.0f), axis::y) == approx(0.0f));
  CER_CHECK(p1.at(vec2f(22.0f, -34322.0232f), axis::y) == approx(0.0f));
  CER_CHECK(p1.at(vec2f(2.0f, 1.0f), axis::z) == approx(0.0f));
  CER_CHECK(p1.at(vec2f(22.0f, -34322.0232f), axis::z) == approx(0.0f));
}

TEST_CASE("plane.xyz_at")
{
  constexpr auto a = vec3f(-2038.034f, 0.0023f, 32.0f);
  constexpr auto n = normalize_c(vec3f(9.734f, -3.393f, 2.033f));
  constexpr auto p = plane3f(a, n);
  constexpr auto point1 = vec2f(27.022f, -12.0123223f);

  CER_CHECK(p.xAt(point1) == approx(p.at(point1, axis::x)));
  CER_CHECK(p.yAt(point1) == approx(p.at(point1, axis::y)));
  CER_CHECK(p.zAt(point1) == approx(p.at(point1, axis::z)));
}

TEST_CASE("plane.point_distance")
{
  constexpr auto a = vec3f(-2038.034f, 0.0023f, 32.0f);
  constexpr auto n = normalize_c(vec3f(9.734f, -3.393f, 2.033f));
  constexpr auto p = plane3f(a, n);
  constexpr auto point = vec3f(1.0f, -32.37873f, 32.0f);
  CER_CHECK(p.point_distance(point) == dot(point, p.normal) - p.distance);
}

TEST_CASE("plane.point_status")
{
  constexpr auto p = plane3f(10.0f, vec3f::pos_z());
  CER_CHECK(p.point_status(vec3f(0.0f, 0.0f, 11.0f)) == plane_status::above);
  CER_CHECK(p.point_status(vec3f(0.0f, 0.0f, 9.0f)) == plane_status::below);
  CER_CHECK(p.point_status(vec3f(0.0f, 0.0f, 10.0f)) == plane_status::inside);
}

TEST_CASE("plane.flip")
{
  constexpr auto p = plane3f(10.0f, vec3f::pos_z());
  CER_CHECK(p.flip() == plane3f(-10.0f, vec3f::neg_z()));
}

TEST_CASE("plane.transform")
{
  const auto p = plane3d(vec3d::one(), vec3d::pos_z());
  const auto rm = rotation_matrix(to_radians(15.0), to_radians(20.0), to_radians(-12.0));
  const auto tm = translation_matrix(vec3d::one());

  const auto pt = p.transform(rm * tm);
  CHECK(is_unit(p.normal, vm::Cd::almost_zero()));
  CHECK(pt.point_status(rm * tm * p.anchor()) == plane_status::inside);
  CHECK(pt.normal == approx(rm * p.normal));
}

TEST_CASE("plane.transform_c")
{
  constexpr auto p = plane3d(vec3d::one(), vec3d::pos_z());
  constexpr auto sm = scaling_matrix(vec3d(2.0, 0.5, 3.0));
  constexpr auto tm = translation_matrix(vec3d::one());

  constexpr auto pt = p.transform_c(sm * tm);
  CER_CHECK(is_unit_c(p.normal, vm::Cd::almost_zero()))
  CHECK(pt.point_status(sm * tm * p.anchor()) == plane_status::inside);
  CHECK(pt.normal == approx(normalize_c(sm * p.normal)));
}

TEST_CASE("plane.project_point")
{
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z()).project_point(vec3d(0, 0, 10))
    == approx(vec3d(0, 0, 0)));
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z()).project_point(vec3d(1, 2, 10))
    == approx(vec3d(1, 2, 0)));
  CER_CHECK(
    plane3d(0.0, normalize_c(vec3d(1, 1, 1))).project_point(vec3d(10, 10, 10))
    == approx(vec3d(0, 0, 0)));
}

TEST_CASE("plane.project_point_direction")
{
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z()).project_point(vec3d(0, 0, 10), vec3d::pos_z())
    == approx(vec3d(0, 0, 0)));
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z()).project_point(vec3d(1, 2, 10), vec3d::pos_z())
    == approx(vec3d(1, 2, 0)));
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z())
      .project_point(vec3d(10, 10, 10), normalize_c(vec3d(1, 1, 1)))
    == approx(vec3d(0, 0, 0)));
}

TEST_CASE("plane.project_vector")
{
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z()).project_vector(vec3d(1, 1, 1))
    == approx(vec3d(1, 1, 0)));
  CER_CHECK(
    plane3d(1.0, vec3d::pos_z()).project_vector(vec3d(1, 1, 1))
    == approx(vec3d(1, 1, 0)));
}

TEST_CASE("plane.project_vector_direction")
{
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z()).project_vector(vec3d(1, 1, 1), vec3d::pos_z())
    == approx(vec3d(1, 1, 0)));
  CER_CHECK(
    plane3d(1.0, vec3d::pos_z()).project_vector(vec3d(1, 1, 1), vec3d::pos_z())
    == approx(vec3d(1, 1, 0)));
  CER_CHECK(
    plane3d(0.0, vec3d::pos_z())
      .project_vector(vec3d(1, 1, 1), normalize_c(vec3d(1, 1, -1)))
    == approx(vec3d(2, 2, 0)));
}

TEST_CASE("plane.is_equal"){CER_CHECK(is_equal(
  plane3f(0.0f, vec3f::pos_x()),
  plane3f(0.0f, vec3f::pos_x()),
  constants<float>::almost_zero()))
                              CER_CHECK(is_equal(
                                plane3f(0.0f, vec3f::pos_y()),
                                plane3f(0.0f, vec3f::pos_y()),
                                constants<float>::almost_zero()))
                                CER_CHECK(is_equal(
                                  plane3f(0.0f, vec3f::pos_z()),
                                  plane3f(0.0f, vec3f::pos_z()),
                                  constants<float>::almost_zero()))
                                  CER_CHECK_FALSE(is_equal(
                                    plane3f(0.0f, vec3f::pos_x()),
                                    plane3f(0.0f, vec3f::neg_x()),
                                    constants<float>::almost_zero()))
                                    CER_CHECK_FALSE(is_equal(
                                      plane3f(0.0f, vec3f::pos_x()),
                                      plane3f(0.0f, vec3f::pos_y()),
                                      constants<float>::almost_zero()))}

TEST_CASE("plane.operator_equal"){
  CER_CHECK(plane3d() == plane3d())
    CER_CHECK(plane3d(10.0, vec3d::pos_z()) == plane3d(10.0, vec3d::pos_z()))
      CER_CHECK_FALSE(plane3d(20.0, vec3d::pos_z()) == plane3d(10.0, vec3d::pos_z()))
        CER_CHECK_FALSE(plane3d(10.0, vec3d::neg_z()) == plane3d(10.0, vec3d::pos_z()))
          CER_CHECK_FALSE(
            plane3d(10.0, normalize_c(vec3d::one())) == plane3d(10.0, vec3d::pos_z()))}

TEST_CASE("plane.operator_not_equal")
{
  CER_CHECK_FALSE(plane3d() != plane3d())
  CER_CHECK_FALSE(plane3d(10.0, vec3d::pos_z()) != plane3d(10.0, vec3d::pos_z()))
  CER_CHECK(plane3d(20.0, vec3d::pos_z()) != plane3d(10.0, vec3d::pos_z()))
  CER_CHECK(plane3d(10.0, vec3d::neg_z()) != plane3d(10.0, vec3d::pos_z()))
  CER_CHECK(plane3d(10.0, normalize_c(vec3d::one())) != plane3d(10.0, vec3d::pos_z()))
}

template <typename T>
void checkValidPlaneNormal(
  const vec<T, 3>& expected,
  const vec<T, 3>& p1,
  const vec<T, 3>& p2,
  const vec<T, 3>& p3)
{
  const auto result = plane_normal(p1, p2, p3);
  CHECK(std::get<0>(result));
  CHECK(std::get<1>(result) == approx(expected));
}

template <typename T>
void checkInvalidPlaneNormal(
  const vec<T, 3>& p1, const vec<T, 3>& p2, const vec<T, 3>& p3)
{
  const auto result = plane_normal(p1, p2, p3);
  CHECK_FALSE(std::get<0>(result));
}

TEST_CASE("plane.plane_normal")
{
  checkValidPlaneNormal(vec3d::pos_z(), vec3d::zero(), vec3d::pos_y(), vec3d::pos_x());
  checkValidPlaneNormal(
    vec3d::pos_z(), vec3d::zero(), normalize(vec3d(1, 1, 0)), vec3d::pos_x());
  checkInvalidPlaneNormal(vec3d::zero(), vec3d::zero(), vec3d::pos_x());
  checkInvalidPlaneNormal(vec3d::zero(), vec3d::pos_x(), vec3d::pos_x());
  checkInvalidPlaneNormal(vec3d::zero(), vec3d::neg_x(), vec3d::pos_x());
  checkInvalidPlaneNormal(vec3d::zero(), vec3d::zero(), vec3d::pos_x());
}

TEST_CASE("plane.from_points")
{
  bool valid;
  plane3f plane;
  std::array<vec3f, 3> points;
  const float epsilon = constants<float>::point_status_epsilon();

  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(0.0f, 1.0f, 0.0f);
  points[2] = vec3f(1.0f, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK(valid);
  CHECK(plane.normal == approx(vec3f::pos_z()));
  CHECK(plane.distance == approx(0.0f));

  // right angle, short vectors
  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(0.0f, epsilon, 0.0f);
  points[2] = vec3f(epsilon, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK(valid);
  CHECK(plane.normal == approx(vec3f::pos_z()));
  CHECK(plane.distance == approx(0.0f));

  // plane point vectors at a 45 degree angle, short vectors
  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(epsilon, epsilon, 0.0f);
  points[2] = vec3f(epsilon, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK(valid);
  CHECK(plane.normal == approx(vec3f::pos_z()));
  CHECK(plane.distance == approx(0.0f));

  // horizontal plane at z=length units above the origin
  points[0] = vec3f(0.0f, 0.0f, epsilon);
  points[1] = vec3f(0.0f, epsilon, epsilon);
  points[2] = vec3f(epsilon, 0.0f, epsilon);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK(valid);
  CHECK(plane.normal == approx(vec3f::pos_z()));
  CHECK(plane.distance == approx(epsilon));

  // small angle (triangle 1000 units wide, length units tall)
  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(1000.0f, epsilon, 0.0f);
  points[2] = vec3f(1000.0f, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK(valid);
  CHECK(plane.normal == approx(vec3f::pos_z()));
  CHECK(plane.distance == approx(0.0f));

  // small angle
  points[0] = vec3f(224.0f, -400.0f, 1648.0f);
  points[1] = vec3f(304.0f, -432.0f, 1248.0f + epsilon);
  points[2] = vec3f(304.0f, -432.0f, 1248.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK(valid);
  CHECK(length(plane.normal) == approx(1.0f));

  // too-small angle (triangle 1000 units wide, length/100 units tall)
  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(1000.0f, epsilon / 100.0f, 0.0f);
  points[2] = vec3f(1000.0f, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK_FALSE(valid);

  // all zero
  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(0.0f, 0.0f, 0.0f);
  points[2] = vec3f(0.0f, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK_FALSE(valid);

  // same direction, short vectors
  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(2 * epsilon, 0.0f, 0.0f);
  points[2] = vec3f(epsilon, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK_FALSE(valid);

  // opposite, short vectors
  points[0] = vec3f(0.0f, 0.0f, 0.0f);
  points[1] = vec3f(-epsilon, 0.0f, 0.0f);
  points[2] = vec3f(epsilon, 0.0f, 0.0f);

  std::tie(valid, plane) = from_points(std::begin(points), std::end(points));
  CHECK_FALSE(valid);
}

TEST_CASE("plane.horizontal_plane")
{
  constexpr auto position = vec3f(322.0f, -122.2392f, 34.0f);
  constexpr auto p = horizontal_plane(position);
  CER_CHECK(p.point_status(position) == plane_status::inside)
  CER_CHECK(p.normal == approx(vec3f::pos_z()));
}

TEST_CASE("plane.orthogonal_plane")
{
  const auto position = vec3f(322.0f, -122.2392f, 34.0f);
  const auto direction = normalize(vec3f(1.0f, 2.0f, -3.0f));
  const auto p = orthogonal_plane(position, direction);
  CHECK(p.point_status(position) == plane_status::inside);
  CHECK(p.normal == approx(direction));
}

TEST_CASE("plane.aligned_orthogonal_plane")
{
  constexpr auto position = vec3f(322.0f, -122.2392f, 34.0f);
  constexpr auto direction = normalize_c(vec3f(1.0f, 2.0f, -3.0f));
  constexpr auto p = aligned_orthogonal_plane(position, direction);
  CER_CHECK(p.point_status(position) == plane_status::inside)
  CER_CHECK(p.normal == approx(vec3f::neg_z()));
}

TEST_CASE("plane.stream_insertion")
{
  std::stringstream str;
  str << plane3d(10.0, vec3d::pos_z());
  CHECK(str.str() == "{ normal: (0 0 1), distance: 10 }");
}
} // namespace vm
