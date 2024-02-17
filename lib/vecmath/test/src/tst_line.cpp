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

#include "vecmath/approx.h"
#include "vecmath/forward.h"
#include "vecmath/line.h"
#include "vecmath/line_io.h"
#include "vecmath/mat.h"
#include "vecmath/mat_ext.h"
#include "vecmath/scalar.h"

#include <sstream>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("line.constructor_default")
{
  constexpr auto p = line3f();
  CER_CHECK(p.point == vec3f::zero());
  CER_CHECK(p.direction == vec3f::zero());
}

TEST_CASE("line.constructor_convert")
{
  constexpr auto l = line3d(vec3d::one(), vec3d::pos_z());
  constexpr auto k = line3f(l);
  CER_CHECK(k.point == approx(vec3f::one()));
  CER_CHECK(k.direction == approx(vec3f::pos_z()));
}

TEST_CASE("line.constructor_with_point_and_direction")
{
  constexpr auto p = vec3f(10, 20, 30);
  constexpr auto n = normalize_c(vec3f(1.0f, 2.0f, 3.0f));
  constexpr auto l = line3f(p, n);
  CER_CHECK(l.point == approx(p));
  CER_CHECK(l.direction == approx(n));
}

TEST_CASE("line.get_origin")
{
  constexpr auto l = line3d(vec3d::one(), vec3d::pos_z());
  CER_CHECK(l.get_origin() == approx(l.point));
}

TEST_CASE("line.get_direction")
{
  constexpr auto l = line3d(vec3d::one(), vec3d::pos_z());
  CER_CHECK(l.get_direction() == approx(l.direction));
}

TEST_CASE("line.transform")
{
  const auto l = line3d(vec3d::one(), vec3d::pos_z());
  const auto rm = rotation_matrix(to_radians(15.0), to_radians(20.0), to_radians(-12.0));
  const auto tm = translation_matrix(vec3d::one());

  const auto lt = l.transform(rm * tm);
  CHECK(is_unit(l.direction, vm::Cd::almost_zero()));
  CHECK(lt.point == approx(rm * tm * l.point));
  CHECK(lt.direction == approx(rm * l.direction));
}

TEST_CASE("line.transform_c")
{
  constexpr auto l = line3d(vec3d::one(), vec3d::pos_z());
  constexpr auto sm = scaling_matrix(vec3d(2.0, 0.5, -2.0));
  constexpr auto tm = translation_matrix(vec3d::one());

  constexpr auto lt = l.transform_c(sm * tm);
  CHECK(is_unit(l.direction, vm::Cd::almost_zero()));
  CHECK(lt.point == approx(sm * tm * l.point));
  CHECK(lt.direction == approx(normalize_c(sm * l.direction)));
}

TEST_CASE("line.make_canonical")
{
  constexpr auto l1 = line3d(vec3d(-10, 0, 10), vec3d::pos_x());
  constexpr auto l2 = line3d(vec3d(+10, 0, 10), vec3d::pos_x());
  CER_CHECK(l2.make_canonical() == approx(l1.make_canonical()));
}

TEST_CASE("line.distance_to_projected_point")
{
  constexpr auto l = line3f(vec3f(10, 0, 0), vec3f::pos_z());
  CER_CHECK(distance_to_projected_point(l, vec3f(10, 0, 0)) == approx(0.0f));
  CER_CHECK(distance_to_projected_point(l, vec3f(10, 0, 10)) == approx(10.0f));
  CER_CHECK(distance_to_projected_point(l, vec3f(10, 10, 10)) == approx(10.0f));
}

TEST_CASE("line.project_point")
{
  constexpr auto l = line3f(vec3f(10, 0, 0), vec3f::pos_z());
  CER_CHECK(project_point(l, vec3f(100, 100, 5)) == approx(vec3f(10, 0, 5)));
}

TEST_CASE("line.is_equal"){
  CER_CHECK(is_equal(line3d(), line3d(), 0.0)) CER_CHECK(is_equal(
    line3d(vec3d::zero(), vec3d::pos_z()), line3d(vec3d::zero(), vec3d::pos_z()), 0.0))
    CER_CHECK_FALSE(is_equal(
      line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)),
      line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)),
      0.0))
      CER_CHECK(is_equal(
        line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)),
        line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)),
        2.0))}

TEST_CASE("line.operator_equal"){
  CER_CHECK(line3d() == line3d()) CER_CHECK(
    line3d(vec3d::zero(), vec3d::pos_z()) == line3d(vec3d::zero(), vec3d::pos_z()))
    CER_CHECK_FALSE(
      line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) == line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)))}

TEST_CASE("line.operator_not_equal"){
  CER_CHECK_FALSE(line3d() != line3d()) CER_CHECK_FALSE(
    line3d(vec3d::zero(), vec3d::pos_z()) != line3d(vec3d::zero(), vec3d::pos_z()))
    CER_CHECK(
      line3d(vec3d(0, 0, 0), vec3d(0, 0, 1)) != line3d(vec3d(1, 0, 0), vec3d(0, 0, 1)))}

TEST_CASE("line.stream_insertion")
{
  std::stringstream str;
  str << line3d(line3d(vec3d::zero(), vec3d::pos_z()));
  CHECK(str.str() == "{ point: (0 0 0), direction: (0 0 1) }");
}
} // namespace vm
