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
#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/ray.h>
#include <vecmath/ray_io.h>
#include <vecmath/scalar.h>
#include <vecmath/util.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "test_utils.h"

#include <sstream>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("ray.constructor_default")
{
  constexpr auto r = ray3d();
  CER_CHECK(r.origin == approx(vec3d::zero()));
  CER_CHECK(r.direction == approx(vec3d::zero()));
}

TEST_CASE("ray.constructor_convert")
{
  constexpr auto r = ray3d(vec3d::one(), vec3d::pos_z());
  constexpr auto s = ray3f(r);
  CER_CHECK(s.origin == approx(vec3f::one()));
  CER_CHECK(s.direction == approx(vec3f::pos_z()));
}

TEST_CASE("ray.constructor_with_origin_and_direction")
{
  constexpr auto r = ray3d(vec3d::one(), vec3d::pos_z());
  CER_CHECK(r.origin == approx(vec3d::one()));
  CER_CHECK(r.direction == approx(vec3d::pos_z()));
}

TEST_CASE("ray.get_origin")
{
  const auto r = ray3d(vec3d::one(), vec3d::pos_z());
  CHECK(r.get_origin() == approx(r.origin));
}

TEST_CASE("ray.get_direction")
{
  const auto r = ray3d(vec3d::one(), vec3d::pos_z());
  CHECK(r.get_direction() == approx(r.direction));
}

TEST_CASE("ray.transform")
{
  const auto r = ray3d(vec3d::one(), vec3d::pos_z());
  const auto rm = rotation_matrix(to_radians(15.0), to_radians(20.0), to_radians(-12.0));
  const auto tm = translation_matrix(vec3d::one());

  const auto rt = r.transform(rm * tm);
  CHECK(is_unit(r.direction, vm::Cd::almost_zero()));
  CHECK(rt.origin == approx(rm * tm * r.origin));
  CHECK(rt.direction == approx(rm * r.direction));
}

TEST_CASE("ray.transform_c")
{
  constexpr auto r = ray3d(vec3d::one(), vec3d::pos_z());
  constexpr auto sm = scaling_matrix(vec3d(2.0, 0.5, -2.0));
  constexpr auto tm = translation_matrix(vec3d::one());

  constexpr auto rt = r.transform_c(sm * tm);
  CER_CHECK(is_unit_c(r.direction, vm::Cd::almost_zero()));
  CER_CHECK(rt.origin == approx(sm * tm * r.origin));
  CER_CHECK(rt.direction == approx(normalize_c(sm * r.direction)));
}

TEST_CASE("ray.point_status")
{
  constexpr auto ray = ray3f(vec3f::zero(), vec3f::pos_z());
  CER_CHECK(ray.point_status(vec3f(0.0f, 0.0f, 1.0f)) == plane_status::above);
  CER_CHECK(ray.point_status(vec3f(0.0f, 0.0f, 0.0f)) == plane_status::inside);
  CER_CHECK(ray.point_status(vec3f(0.0f, 0.0f, -1.0f)) == plane_status::below);
}

TEST_CASE("ray.point_at_distance")
{
  constexpr auto ray = ray3f(vec3f::zero(), vec3f::pos_x());
  CER_CHECK(point_at_distance(ray, 5.0f) == approx(vec3f(5.0f, 0.0f, 0.0f)));
}

TEST_CASE("ray.is_equal")
{
  CER_CHECK(is_equal(ray3d(), ray3d(), 0.0));
  CER_CHECK(is_equal(
    ray3d(vec3d::zero(), vec3d::pos_z()), ray3d(vec3d::zero(), vec3d::pos_z()), 0.0));
  CER_CHECK_FALSE(is_equal(
    ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)), ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)), 0.0));
  CER_CHECK(is_equal(
    ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)), ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)), 2.0));
}

TEST_CASE("ray.operator_equal")
{
  CER_CHECK(ray3d() == ray3d());
  CER_CHECK(ray3d(vec3d::zero(), vec3d::pos_z()) == ray3d(vec3d::zero(), vec3d::pos_z()));
  CER_CHECK_FALSE(
    ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) == ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)));
}

TEST_CASE("ray.operator_not_equal")
{
  CER_CHECK_FALSE(ray3d() != ray3d());
  CER_CHECK_FALSE(
    ray3d(vec3d::zero(), vec3d::pos_z()) != ray3d(vec3d::zero(), vec3d::pos_z()));
  CER_CHECK(
    ray3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) != ray3d(vec3d(1, 0, 0), vec3d(0, 0, 1)));
}

TEST_CASE("ray.stream_insertion")
{
  std::stringstream str;
  str << ray3d(vec3d::zero(), vec3d::pos_z());
  CHECK(str.str() == "{ origin: (0 0 0), direction: (0 0 1) }");
}
} // namespace vm
