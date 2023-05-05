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
#include <vecmath/forward.h>
#include <vecmath/mat_ext.h> // used by rotate_pos_x_by_degrees
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <array>
#include <limits>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("vec.default_constructor")
{
  constexpr auto v = vec3f();
  CER_CHECK(v[0] == 0.0f);
  CER_CHECK(v[1] == 0.0f);
  CER_CHECK(v[2] == 0.0f);
}

TEST_CASE("vec.initializer_list_constructor")
{
  constexpr auto v = vec3f({1.0f, 2.0f, 3.0f});
  CER_CHECK(v == vec3f(1.0f, 2.0f, 3.0f));
}

TEST_CASE("vec.componentwise_constructor_with_matching_type")
{
  constexpr auto v = vec3f(1.0f, 2.0f, 3.0f);
  CER_CHECK(v[0] == 1.0f);
  CER_CHECK(v[1] == 2.0f);
  CER_CHECK(v[2] == 3.0f);
}

TEST_CASE("vec.componentwise_constructor_with_mixed_types")
{
  constexpr auto v = vec3f(1.0, 2.0f, 3u);
  CER_CHECK(v[0] == 1.0f);
  CER_CHECK(v[1] == 2.0f);
  CER_CHECK(v[2] == 3.0f);
}

TEST_CASE("vec.converting_constructor")
{
  constexpr auto vf = vec3f(1.0f, 2.0f, 3.0f);
  constexpr auto vd = vec3d(vf);
  CER_CHECK(vd[0] == static_cast<double>(vf[0]));
  CER_CHECK(vd[1] == static_cast<double>(vf[1]));
  CER_CHECK(vd[2] == static_cast<double>(vf[2]));
}

TEST_CASE("vec.converting_constructor_embed")
{
  constexpr auto vf = vec3f(1.0f, 2.0f, 3.0f);
  constexpr auto vd = vec4d(vf);
  CER_CHECK(vd[0] == static_cast<double>(vf[0]));
  CER_CHECK(vd[1] == static_cast<double>(vf[1]));
  CER_CHECK(vd[2] == static_cast<double>(vf[2]));
  CER_CHECK(vd[3] == 0.0);
}

TEST_CASE("vec.converting_constructor_trunc")
{
  constexpr auto vf = vec3f(1.0f, 2.0f, 3.0f);
  constexpr auto vd = vec2d(vf);
  CER_CHECK(vd[0] == static_cast<double>(vf[0]));
  CER_CHECK(vd[1] == static_cast<double>(vf[1]));
}

TEST_CASE("vec.embedding_constructor")
{
  constexpr auto vf = vec2f(1.0f, 2.0f);
  constexpr auto vd = vec3d(vf, 3.0f);
  CER_CHECK(vd[0] == static_cast<double>(vf[0]));
  CER_CHECK(vd[1] == static_cast<double>(vf[1]));
  CER_CHECK(vd[2] == static_cast<double>(3.0f));
}

TEST_CASE("vec.assignment")
{
  constexpr auto t = vec3f(2.0f, 3.0f, 5.0f);
  constexpr auto v = t;
  CER_CHECK(v == t);
}

TEST_CASE("vec.fill")
{
  constexpr auto v1 = vec3f::fill(2.0f);
  constexpr auto v2 = vec3f::fill(0.0f);
  constexpr auto v3 = vec3f::fill(-2.0f);
  CER_CHECK(v1 == vec3f(2.0f, 2.0f, 2.0f));
  CER_CHECK(v2 == vec3f(0.0f, 0.0f, 0.0f));
  CER_CHECK(v3 == vec3f(-2.0f, -2.0f, -2.0f));
}

TEST_CASE("vec.axis")
{
  constexpr auto vx = vec3f::axis(0);
  constexpr auto vy = vec3f::axis(1);
  constexpr auto vz = vec3f::axis(2);
  CER_CHECK(vx == vec3f(1, 0, 0));
  CER_CHECK(vy == vec3f(0, 1, 0));
  CER_CHECK(vz == vec3f(0, 0, 1));
}

TEST_CASE("vec.operator_subscript")
{
  // aggregate initialization circumvents constructors and constructs the value array in
  // place
  constexpr auto v = vec4f{{1.0f, 2.0f, 3.0f, 4.0f}};
  CER_CHECK(v[0] == 1.0f);
  CER_CHECK(v[1] == 2.0f);
  CER_CHECK(v[2] == 3.0f);
  CER_CHECK(v[3] == 4.0f);
}

TEST_CASE("vec.accessors")
{
  // aggregate initialization circumvents constructors and constructs the value array in
  // place
  constexpr auto v = vec4f{{1.0f, 2.0f, 3.0f, 4.0f}};
  constexpr auto vx = v.x();
  constexpr auto vy = v.y();
  constexpr auto vz = v.z();
  constexpr auto vw = v.w();
  constexpr auto vxy = v.xy();
  constexpr auto vxyz = v.xyz();
  constexpr auto vxyzw = v.xyzw();

  CER_CHECK(vx == v[0]);
  CER_CHECK(vy == v[1]);
  CER_CHECK(vz == v[2]);
  CER_CHECK(vw == v[3]);
  CER_CHECK(vxy == vec2f(1.0f, 2.0f));
  CER_CHECK(vxyz == vec3f(1.0f, 2.0f, 3.0f));
  CER_CHECK(vxyzw == v);
}

TEST_CASE("vec.static_members")
{
  constexpr auto pos_x = vec3f::pos_x();
  constexpr auto pos_y = vec3f::pos_y();
  constexpr auto pos_z = vec3f::pos_z();
  constexpr auto neg_x = vec3f::neg_x();
  constexpr auto neg_y = vec3f::neg_y();
  constexpr auto neg_z = vec3f::neg_z();
  constexpr auto zero = vec3f::zero();
  constexpr auto one = vec3f::one();
  constexpr auto nan = vec3f::nan();
  constexpr auto min = vec3f::min();
  constexpr auto max = vec3f::max();

  CER_CHECK(pos_x == vec3f(+1, 0, 0));
  CER_CHECK(pos_y == vec3f(0, +1, 0));
  CER_CHECK(pos_z == vec3f(0, 0, +1));
  CER_CHECK(neg_x == vec3f(-1, 0, 0));
  CER_CHECK(neg_y == vec3f(0, -1, 0));
  CER_CHECK(neg_z == vec3f(0, 0, -1));
  CER_CHECK(zero == vec3f(0, 0, 0));
  CER_CHECK(one == vec3f(1, 1, 1));

  for (size_t i = 0; i < 3; ++i)
  {
    CHECK(min[i] == approx(std::numeric_limits<float>::min()));
    CHECK(max[i] == approx(std::numeric_limits<float>::max()));
    CHECK(is_nan(nan[i]));
  }
}

// ========== comparison operators ==========

TEST_CASE("vec.compare")
{
  CER_CHECK(compare(vec3f::zero(), vec3f::zero()) == 0);
  CER_CHECK(compare(vec3f::zero(), vec3f::one()) == -1);
  CER_CHECK(compare(vec3f::one(), vec3f(2, 1, 1)) == -1);
  CER_CHECK(compare(vec3f::one(), vec3f(1, 2, 1)) == -1);
  CER_CHECK(compare(vec3f::one(), vec3f(1, 1, 2)) == -1);
  CER_CHECK(compare(vec3f::one(), vec3f(2, 0, 0)) == -1);
  CER_CHECK(compare(vec3f::one(), vec3f(1, 2, 0)) == -1);

  CER_CHECK(compare(vec3f::one(), vec3f::zero()) == +1);
  CER_CHECK(compare(vec3f(2, 1, 1), vec3f::one()) == +1);
  CER_CHECK(compare(vec3f(1, 2, 1), vec3f::one()) == +1);
  CER_CHECK(compare(vec3f(1, 1, 2), vec3f::one()) == +1);
  CER_CHECK(compare(vec3f(2, 0, 0), vec3f::one()) == +1);
  CER_CHECK(compare(vec3f(1, 2, 0), vec3f::one()) == +1);

  CER_CHECK(compare(vec3f(1, 2, 0), vec3f::nan()) != 0)
  CER_CHECK(compare(vec3f::nan(), vec3f(1, 2, 0)) != 0)
  // This is inconsistent with how operator== on two float values that are nan returns
  // false, but it is consistent with the totalOrder() function from IEEE 754-2008 It's
  // unclear what we should do here and this may need revisiting.
  CER_CHECK(compare(vec3f::nan(), vec3f::nan()) == 0);
}

TEST_CASE("vec.compare_ranges")
{
  constexpr auto r1 = std::array{vec3f(1, 2, 3), vec3f(1, 2, 3)};
  constexpr auto r2 = std::array{vec3f(1, 2, 3), vec3f(2, 2, 3)};
  constexpr auto r3 = std::array{vec3f(2, 2, 3)};

  // same length
  CER_CHECK(
    compare<float>(std::begin(r1), std::end(r1), std::begin(r1), std::end(r1)) == 0);
  CER_CHECK(
    compare<float>(std::begin(r1), std::end(r1), std::begin(r2), std::end(r2)) == -1);
  CER_CHECK(
    compare<float>(std::begin(r2), std::end(r2), std::begin(r1), std::end(r1)) == +1);

  // prefi
  CER_CHECK(
    compare<float>(
      std::begin(r1), std::next(std::begin(r1)), std::begin(r1), std::end(r1))
    == -1);
  CER_CHECK(
    compare<float>(
      std::begin(r1), std::end(r1), std::begin(r1), std::next(std::begin(r1)))
    == +1);

  // different length and not prefi
  CER_CHECK(
    compare<float>(std::begin(r1), std::end(r1), std::begin(r3), std::end(r3)) == -1);
  CER_CHECK(
    compare<float>(std::begin(r3), std::end(r3), std::begin(r1), std::end(r1)) == +1);
}

TEST_CASE("vec.is_equal")
{
  CER_CHECK(is_equal(vec2f::zero(), vec2f::zero(), 0.0f));
  CER_CHECK_FALSE(is_equal(vec2f::zero(), vec2f::one(), 0.0f));
  CER_CHECK(is_equal(vec2f::zero(), vec2f::one(), 2.0f));

  // nan
  CER_CHECK_FALSE(is_equal(vec2f::zero(), vec2f::nan(), 0.0f));
  CER_CHECK_FALSE(is_equal(vec2f::nan(), vec2f::zero(), 0.0f));
  CER_CHECK_FALSE(is_equal(vec2f::zero(), vec2f::nan(), 2.0f));
  CER_CHECK_FALSE(is_equal(vec2f::nan(), vec2f::zero(), 2.0f));

  // See comment in vec_test::compare.
  CER_CHECK(is_equal(vec2f::nan(), vec2f::nan(), 0.0f));
  CER_CHECK(is_equal(vec2f::nan(), vec2f::nan(), 2.0f));
}

TEST_CASE("vec.operator_equal")
{
  CER_CHECK_FALSE(vec3f(1, 2, 3) == vec3f(2, 2, 2));
  CER_CHECK(vec3f(1, 2, 3) == vec3f(1, 2, 3));
  CER_CHECK_FALSE(vec3f(1, 2, 4) == vec3f(1, 2, 2));

  // NaN
  CER_CHECK_FALSE(vec2f::zero() == vec2f::nan());
  CER_CHECK_FALSE(vec2f::nan() == vec2f::zero());

  // See comment in vec_test::compare.
  CER_CHECK(vec2f::nan() == vec2f::nan());
}

TEST_CASE("vec.operator_not_equal")
{
  CER_CHECK(vec3f(1, 2, 3) != vec3f(2, 2, 2));
  CER_CHECK_FALSE(vec3f(1, 2, 3) != vec3f(1, 2, 3));
  CER_CHECK(vec3f(1, 2, 4) != vec3f(1, 2, 2));

  // NaN
  CER_CHECK(vec2f::zero() != vec2f::nan());
  CER_CHECK(vec2f::nan() != vec2f::zero());

  // See comment in vec_test::compare.
  CER_CHECK_FALSE(vec2f::nan() != vec2f::nan());
}

TEST_CASE("vec.operator_less_than")
{
  CER_CHECK(vec3f(1, 2, 3) < vec3f(2, 2, 2));
  CER_CHECK_FALSE(vec3f(1, 2, 3) < vec3f(1, 2, 3));
  CER_CHECK_FALSE(vec3f(1, 2, 4) < vec3f(1, 2, 2));
}

TEST_CASE("vec.operator_less_than_or_equal")
{
  CER_CHECK(vec3f(1, 2, 3) <= vec3f(2, 2, 2));
  CER_CHECK(vec3f(1, 2, 3) <= vec3f(1, 2, 3));
  CER_CHECK_FALSE(vec3f(1, 2, 4) <= vec3f(1, 2, 2));
}

TEST_CASE("vec.operator_greater_than")
{
  CER_CHECK_FALSE(vec3f(1, 2, 3) > vec3f(2, 2, 2));
  CER_CHECK_FALSE(vec3f(1, 2, 3) > vec3f(1, 2, 3));
  CER_CHECK(vec3f(1, 2, 4) > vec3f(1, 2, 2));
}

TEST_CASE("vec.operator_greater_than_or_equal")
{
  CER_CHECK_FALSE(vec3f(1, 2, 3) >= vec3f(2, 2, 2));
  CER_CHECK(vec3f(1, 2, 3) >= vec3f(1, 2, 3));
  CER_CHECK(vec3f(1, 2, 4) >= vec3f(1, 2, 2));
}

/* ========== slicing ========== */

constexpr vec2d slice(const vec4d& vector, const std::size_t offset)
{
  return slice<2>(vector, offset);
}

TEST_CASE("vec.slice")
{
  CER_CHECK(slice(vec4d(1, 2, 3, 4), 0) == vec2d(1, 2));
  CER_CHECK(slice(vec4d(1, 2, 3, 4), 1) == vec2d(2, 3));
  CER_CHECK(slice(vec4d(1, 2, 3, 4), 2) == vec2d(3, 4));
}

/* ========== finding components ========== */

TEST_CASE("vec.find_max_component")
{
  CER_CHECK(find_max_component(vec3f::pos_x(), 0) == 0u);
  CER_CHECK(find_max_component(vec3f::neg_x(), 0) != 0u);
  CER_CHECK(find_max_component(vec3f::pos_y(), 0) == 1u);
  CER_CHECK(find_max_component(vec3f::neg_y(), 0) != 1u);
  CER_CHECK(find_max_component(vec3f::pos_z(), 0) == 2u);
  CER_CHECK(find_max_component(vec3f::neg_z(), 0) != 2u);

  CER_CHECK(find_max_component(vec3f(3.0f, 1.0f, -2.0f), 0) == 0u);
  CER_CHECK(find_max_component(vec3f(3.0f, 1.0f, -2.0f), 1) == 1u);
  CER_CHECK(find_max_component(vec3f(3.0f, 1.0f, -2.0f), 2) == 2u);
  CER_CHECK(find_max_component(normalize_c(vec3f(1.0f, 2.0f, -3.0f)), 0) == 1u);
}

TEST_CASE("vec.find_abs_max_component")
{
  CER_CHECK(find_abs_max_component(vec3f::pos_x(), 0) == 0u);
  CER_CHECK(find_abs_max_component(vec3f::neg_x(), 0) == 0u);
  CER_CHECK(find_abs_max_component(vec3f::pos_y(), 0) == 1u);
  CER_CHECK(find_abs_max_component(vec3f::neg_y(), 0) == 1u);
  CER_CHECK(find_abs_max_component(vec3f::pos_z(), 0) == 2u);
  CER_CHECK(find_abs_max_component(vec3f::neg_z(), 0) == 2u);

  CER_CHECK(find_abs_max_component(vec3f(3.0f, 1.0f, -2.0f), 0) == 0u);
  CER_CHECK(find_abs_max_component(vec3f(3.0f, 1.0f, -2.0f), 1) == 2u);
  CER_CHECK(find_abs_max_component(vec3f(3.0f, 1.0f, -2.0f), 2) == 1u);
  CER_CHECK(find_abs_max_component(normalize_c(vec3f(1.0f, 2.0f, -3.0f)), 0) == 2u);
}

TEST_CASE("vec.get_abs_max_component_axis")
{
  CER_CHECK(get_abs_max_component_axis(vec3f::pos_x()) == vec3f::pos_x());
  CER_CHECK(get_abs_max_component_axis(vec3f::neg_x()) == vec3f::neg_x());
  CER_CHECK(get_abs_max_component_axis(vec3f::pos_y()) == vec3f::pos_y());
  CER_CHECK(get_abs_max_component_axis(vec3f::neg_y()) == vec3f::neg_y());
  CER_CHECK(get_abs_max_component_axis(vec3f::pos_z()) == vec3f::pos_z());
  CER_CHECK(get_abs_max_component_axis(vec3f::neg_z()) == vec3f::neg_z());

  CER_CHECK(get_abs_max_component_axis(vec3f(3.0f, -1.0f, 2.0f), 0u) == vec3f::pos_x());
  CER_CHECK(get_abs_max_component_axis(vec3f(3.0f, -1.0f, 2.0f), 1u) == vec3f::pos_z());
  CER_CHECK(get_abs_max_component_axis(vec3f(3.0f, -1.0f, 2.0f), 2u) == vec3f::neg_y());
}

TEST_CASE("vec.get_max_component")
{
  CER_CHECK(get_max_component(vec3f::pos_x(), 0) == 1.0f);
  CER_CHECK(get_max_component(vec3f::neg_x(), 0) == 0.0f);
  CER_CHECK(get_max_component(vec3f::pos_y(), 0) == 1.0f);
  CER_CHECK(get_max_component(vec3f::neg_y(), 0) == 0.0f);
  CER_CHECK(get_max_component(vec3f::pos_z(), 0) == 1.0f);
  CER_CHECK(get_max_component(vec3f::neg_z(), 0) == 0.0f);

  CER_CHECK(get_max_component(vec3f(3.0f, 1.0f, -2.0f), 0) == 3.0f);
  CER_CHECK(get_max_component(vec3f(3.0f, 1.0f, -2.0f), 1) == 1.0f);
  CER_CHECK(get_max_component(vec3f(3.0f, 1.0f, -2.0f), 2) == -2.0f)
}

TEST_CASE("vec.get_abs_max_component")
{
  CER_CHECK(get_abs_max_component(vec3f::pos_x(), 0) == 1.0f);
  CER_CHECK(get_abs_max_component(vec3f::neg_x(), 0) == -1.0f);
  CER_CHECK(get_abs_max_component(vec3f::pos_y(), 0) == 1.0f);
  CER_CHECK(get_abs_max_component(vec3f::neg_y(), 0) == -1.0f);
  CER_CHECK(get_abs_max_component(vec3f::pos_z(), 0) == 1.0f);
  CER_CHECK(get_abs_max_component(vec3f::neg_z(), 0) == -1.0f);

  CER_CHECK(get_abs_max_component(vec3f(3.0f, 1.0f, -2.0f), 0) == 3.0f);
  CER_CHECK(get_abs_max_component(vec3f(3.0f, 1.0f, -2.0f), 1) == -2.0f);
  CER_CHECK(get_abs_max_component(vec3f(3.0f, 1.0f, -2.0f), 2) == 1.0f);
}

// ========== arithmetic operators ==========

TEST_CASE("vec.operator_unary_plus")
{
  CER_CHECK(+vec3f(+1.0f, -2.0f, +3.0f) == vec3f(+1.0f, -2.0f, +3.0f));
}

TEST_CASE("vec.operator_unary_minus")
{
  CER_CHECK(-vec3f(+1.0f, -2.0f, +3.0f) == vec3f(-1.0f, +2.0f, -3.0f));
}

TEST_CASE("vec.operator_binary_plus")
{
  CER_CHECK(vec3f(1.0f, 2.0f, 3.0f) + vec3f(3.0f, 2.0f, 1.0f) == vec3f(4.0f, 4.0f, 4.0f));
}

TEST_CASE("vec.operator_binary_minus")
{
  CER_CHECK(
    vec3f(2.0f, 3.0f, 1.0f) - vec3f(1.0f, 2.0f, 2.0f) == vec3f(1.0f, 1.0f, -1.0f));
}

TEST_CASE("vec.operator_multiply_vectors")
{
  CER_CHECK(
    vec3f(2.0f, 3.0f, -1.0f) * vec3f(1.0f, 2.0f, 2.0f) == vec3f(2.0f, 6.0f, -2.0f));
}

TEST_CASE("vec.operator_multiply_scalar")
{
  CER_CHECK(vec3f(2.0f, 3.0f, 1.0f) * 3.0f == vec3f(6.0f, 9.0f, 3.0f));
  CER_CHECK(3.0f * vec3f(2.0f, 3.0f, 1.0f) == vec3f(6.0f, 9.0f, 3.0f));
}

TEST_CASE("vec.operator_divide_vectors")
{
  CER_CHECK(
    vec3f(2.0f, 12.0f, 2.0f) / vec3f(1.0f, 2.0f, -1.0f) == vec3f(2.0f, 6.0f, -2.0f));
}

TEST_CASE("vec.operator_divide_scalar")
{
  CER_CHECK(vec3f(2.0f, 36.0f, 4.0f) / 2.0f == vec3f(1.0f, 18.0f, 2.0f));
  CER_CHECK(8.0f / vec3f(2.0f, 8.0f, -4.0f) == vec3f(4.0f, 1.0f, -2.0f));
}

// ========== arithmetic functions ==========

TEST_CASE("vec.min")
{
  CER_CHECK(min(vec3f(+2, +2, +2), vec3f(+3, +3, +3)) == vec3f(+2, +2, +2));
  CER_CHECK(min(vec3f(-2, -2, -2), vec3f(-1, -1, -1)) == vec3f(-2, -2, -2));
  CER_CHECK(min(vec3f(+2, +2, +2), vec3f(+1, +3, +1)) == vec3f(+1, +2, +1));
  CER_CHECK(min(vec3f(-2, -2, -2), vec3f(-1, -3, -1)) == vec3f(-2, -3, -2));
  CER_CHECK(
    min(vec3f(-2, -2, -2), vec3f(-1, -3, -1), vec3f(-1, -3, -4)) == vec3f(-2, -3, -4));
}

TEST_CASE("vec.max")
{
  CER_CHECK(max(vec3f(+2, +2, +2), vec3f(+3, +3, +3)) == vec3f(+3, +3, +3));
  CER_CHECK(max(vec3f(-2, -2, -2), vec3f(-1, -1, -1)) == vec3f(-1, -1, -1));
  CER_CHECK(max(vec3f(+2, +2, +2), vec3f(+1, +3, +1)) == vec3f(+2, +3, +2));
  CER_CHECK(max(vec3f(-2, -2, -2), vec3f(-1, -3, -1)) == vec3f(-1, -2, -1));
  CER_CHECK(
    max(vec3f(-2, -2, -2), vec3f(-1, -3, -1), vec3f(4, -4, 1)) == vec3f(+4, -2, +1));
}

TEST_CASE("vec.abs_min")
{
  CER_CHECK(abs_min(vec3f(+2, +2, +2), vec3f(+3, +3, +3)) == vec3f(+2, +2, +2));
  CER_CHECK(abs_min(vec3f(-2, -2, -2), vec3f(-1, -1, -1)) == vec3f(-1, -1, -1));
  CER_CHECK(abs_min(vec3f(+2, +2, +2), vec3f(+1, +3, +1)) == vec3f(+1, +2, +1));
  CER_CHECK(abs_min(vec3f(-2, -2, -2), vec3f(-1, -3, -1)) == vec3f(-1, -2, -1));
  CER_CHECK(
    abs_min(vec3f(-2, -2, -2), vec3f(-1, -3, -1), vec3f(0, 1, -4)) == vec3f(0, 1, -1));
}

TEST_CASE("vec.abs_max")
{
  CER_CHECK(abs_max(vec3f(+2, +2, +2), vec3f(+3, +3, +3)) == vec3f(+3, +3, +3));
  CER_CHECK(abs_max(vec3f(-2, -2, -2), vec3f(-1, -1, -1)) == vec3f(-2, -2, -2));
  CER_CHECK(abs_max(vec3f(+2, +2, +2), vec3f(+1, +3, +1)) == vec3f(+2, +3, +2));
  CER_CHECK(abs_max(vec3f(-2, -2, -2), vec3f(-1, -3, -1)) == vec3f(-2, -3, -2));
  CER_CHECK(
    abs_max(vec3f(-2, -2, -2), vec3f(-1, -3, -1), vec3f(+4, -1, 0)) == vec3f(+4, -3, -2));
}

TEST_CASE("vec.abs")
{
  CER_CHECK(abs(vec3f(1, -2, -3)) == vec3f(1, 2, 3));
  CER_CHECK(abs(vec3f(0, -2, -3)) == vec3f(0, 2, 3));
}

TEST_CASE("vec.sign")
{
  CER_CHECK(sign(vec3d::one()) == vec3d(+1, +1, +1));
  CER_CHECK(sign(vec3d::zero()) == vec3d(0, 0, 0));
  CER_CHECK(sign(-vec3d::one()) == vec3d(-1, -1, -1));
}

TEST_CASE("vec.step")
{
  CER_CHECK(step(+vec3d::one(), vec3d::zero()) == vec3d(0, 0, 0));
  CER_CHECK(step(+vec3d::one(), vec3d::one()) == vec3d(1, 1, 1));
  CER_CHECK(step(+vec3d::one(), vec3d(-1, 0, 1)) == vec3d(0, 0, 1));
  CER_CHECK(step(-vec3d::one(), vec3d(-1, 0, 1)) == vec3d(1, 1, 1));
  CER_CHECK(step(-vec3d::one(), vec3d(-2, 0, 1)) == vec3d(0, 1, 1));
}

TEST_CASE("vec.smoothstep")
{
  CER_CHECK(
    smoothstep(vec3d::zero(), vec3d::one(), vec3d(-1.0, -1.0, -1.0))
    == vec3d(0.0, 0.0, 0.0));
  CER_CHECK(
    smoothstep(vec3d::zero(), vec3d::one(), vec3d(0.0, 0.0, 0.0))
    == vec3d(0.0, 0.0, 0.0));
  CER_CHECK(
    smoothstep(vec3d::zero(), vec3d::one(), vec3d(+1.0, +1.0, +1.0))
    == vec3d(1.0, 1.0, 1.0));
  CER_CHECK(
    smoothstep(vec3d::zero(), vec3d::one(), vec3d(+2.0, +2.0, +2.0))
    == vec3d(1.0, 1.0, 1.0));
  CER_CHECK(
    smoothstep(vec3d::zero(), vec3d::one(), vec3d(-1.0, 0.0, +2.0))
    == vec3d(0.0, 0.0, 1.0));
  CER_CHECK(
    smoothstep(vec3d::zero(), vec3d::one(), vec3d(0.0, +0.5, +1.0))
    == vec3d(0.0, 0.5, 1.0));
  CER_CHECK(
    smoothstep(vec3d::zero(), vec3d::one(), vec3d(+0.25, +0.5, +0.75))
    == vec3d(0.15625, 0.5, 0.84375));
}

TEST_CASE("vec.dot")
{
  CER_CHECK(
    dot(vec3f(2.3f, 8.7878f, -2323.0f), vec3f(4.333f, -2.0f, 322.0f))
    == approx(-748013.6097f));
  CER_CHECK(dot(vec3f(2.3f, 8.7878f, -2323.0f), vec3f::zero()) == approx(0.0f));
}

TEST_CASE("vec.cross")
{
  CER_CHECK(cross(vec3f::zero(), vec3f::zero()) == vec3f::zero());
  CER_CHECK(cross(vec3f::zero(), vec3f(2.0f, 34.233f, -10003.0002f)) == vec3f::zero());
  CER_CHECK(cross(vec3f::pos_x(), vec3f::pos_y()) == vec3f::pos_z());
  CER_CHECK(
    cross(vec3f(12.302f, -0.0017f, 79898.3f), vec3f(2.0f, 34.233f, -10003.0002f))
    == approx(vec3f(-2735141.499f, 282853.508f, 421.138f)));

  constexpr auto t1 = vec3f(7.0f, 4.0f, 0.0f);
  constexpr auto t2 = vec3f(-2.0f, 22.0f, 0.0f);
  CER_CHECK(
    normalize_c(cross(normalize_c(t1), normalize_c(t2)))
    == approx(normalize_c(cross(t1, t2))));
}

TEST_CASE("vec.squared_length")
{
  CER_CHECK(squared_length(vec3f::zero()) == approx(0.0f));
  CER_CHECK(squared_length(vec3f::pos_x()) == approx(1.0f));
  CER_CHECK(squared_length(vec3f(2.3f, 8.7878f, -2323.0f)) == approx(5396411.51542884f));
}

TEST_CASE("vec.length")
{
  CHECK(length(vec3f::zero()) == approx(0.0f));
  CHECK(length(vec3f::pos_x()) == approx(1.0f));
  CHECK(length(vec3f(2.3f, 8.7878f, -2323.0f)) == approx(std::sqrt(5396411.51542884f)));
}

TEST_CASE("vec.length_c")
{
  CE_CHECK(length_c(vec3f::zero()) == approx(0.0f));
  CE_CHECK(length_c(vec3f::pos_x()) == approx(1.0f));
  CE_CHECK(length_c(vec3f(2.3f, 8.7878f, -2323.0f)) == approx(sqrt_c(5396411.51542884f)));
}

TEST_CASE("vec.normalize")
{
  CHECK(normalize(vec3f::pos_x()) == vec3f::pos_x());
  CHECK(normalize(vec3f::neg_x()) == vec3f::neg_x());

  const vec3f v1(2.3f, 8.7878f, -2323.0f);
  const vec3f v2(4.333f, -2.0f, 322.0f);
  CHECK(normalize(v1) == approx(v1 / length(v1)));
  CHECK(normalize(v2) == approx(v2 / length(v2)));
}

TEST_CASE("vec.normalize_c")
{
  CE_CHECK(normalize_c(vec3f::pos_x()) == vec3f::pos_x());
  CE_CHECK(normalize_c(vec3f::neg_x()) == vec3f::neg_x());

  constexpr vec3f v1(2.3f, 8.7878f, -2323.0f);
  constexpr vec3f v2(4.333f, -2.0f, 322.0f);
  CE_CHECK(normalize_c(v1) == approx(v1 / length_c(v1)));
  CE_CHECK(normalize_c(v2) == approx(v2 / length_c(v2)));
}

TEST_CASE("vec.swizzle")
{
  CER_CHECK(swizzle(vec3d(1, 2, 3), 0) == vec3d(2, 3, 1));
  CER_CHECK(swizzle(vec3d(1, 2, 3), 1) == vec3d(3, 1, 2));
  CER_CHECK(swizzle(vec3d(1, 2, 3), 2) == vec3d(1, 2, 3));
}

TEST_CASE("vec.unswizzle")
{
  CER_CHECK(unswizzle(swizzle(vec3d(1, 2, 3), 0), 0) == vec3d(1, 2, 3));
  CER_CHECK(unswizzle(swizzle(vec3d(1, 2, 3), 1), 1) == vec3d(1, 2, 3));
  CER_CHECK(unswizzle(swizzle(vec3d(1, 2, 3), 2), 2) == vec3d(1, 2, 3));
}

TEST_CASE("vec.is_unit")
{
  CHECK(is_unit(vec3f::pos_x(), vm::Cf::almost_zero()));
  CHECK(is_unit(vec3f::pos_y(), vm::Cf::almost_zero()));
  CHECK(is_unit(vec3f::pos_z(), vm::Cf::almost_zero()));
  CHECK(is_unit(vec3f::neg_x(), vm::Cf::almost_zero()));
  CHECK(is_unit(vec3f::neg_y(), vm::Cf::almost_zero()));
  CHECK(is_unit(vec3f::neg_z(), vm::Cf::almost_zero()));
  CHECK(is_unit(normalize(vec3f::one()), vm::Cf::almost_zero()));
  CHECK_FALSE(is_unit(vec3f::one(), vm::Cf::almost_zero()));
  CHECK_FALSE(is_unit(vec3f::zero(), vm::Cf::almost_zero()));
}

TEST_CASE("vec.is_unit_c"){
  CE_CHECK(is_unit_c(vec3f::pos_x(), vm::Cf::almost_zero()))
    CE_CHECK(is_unit_c(vec3f::pos_y(), vm::Cf::almost_zero()))
      CE_CHECK(is_unit_c(vec3f::pos_z(), vm::Cf::almost_zero()))
        CE_CHECK(is_unit_c(vec3f::neg_x(), vm::Cf::almost_zero()))
          CE_CHECK(is_unit_c(vec3f::neg_y(), vm::Cf::almost_zero()))
            CE_CHECK(is_unit_c(vec3f::neg_z(), vm::Cf::almost_zero()))
              CE_CHECK(is_unit_c(normalize_c(vec3f::one()), vm::Cf::almost_zero()))
                CE_CHECK_FALSE(is_unit_c(vec3f::one(), vm::Cf::almost_zero()))
                  CE_CHECK_FALSE(is_unit_c(vec3f::zero(), vm::Cf::almost_zero()))}

TEST_CASE("vec.is_zero")
{
  CER_CHECK(is_zero(vec3f::zero(), vm::Cf::almost_zero()));
  CER_CHECK_FALSE(is_zero(vec3f::pos_x(), vm::Cf::almost_zero()));
}

TEST_CASE("vec.is_nan")
{
  CER_CHECK(is_nan(vec3f::nan()));
  CER_CHECK_FALSE(is_nan(vec3f::pos_x()));
}

TEST_CASE("vec.is_integral")
{
  CER_CHECK(is_integral(vec3f::pos_x()));
  CER_CHECK(is_integral(vec3f::pos_y()));
  CER_CHECK(is_integral(vec3f::pos_z()));
  CER_CHECK(is_integral(vec3f::neg_x()));
  CER_CHECK(is_integral(vec3f::neg_y()));
  CER_CHECK(is_integral(vec3f::neg_z()));
  CER_CHECK(is_integral(vec3f::one()));
  CER_CHECK(is_integral(vec3f::zero()));
  CER_CHECK_FALSE(is_integral(normalize_c(vec3f::one())));
}

TEST_CASE("vec.mix")
{
  CER_CHECK(mix(vec3d::zero(), vec3d::one(), vec3d::zero()) == vec3d::zero());
  CER_CHECK(mix(vec3d::zero(), vec3d::one(), vec3d::one()) == vec3d::one());
  CER_CHECK(mix(vec3d::zero(), vec3d::one(), vec3d::one() / 2.0) == vec3d::one() / 2.0);
}

TEST_CASE("vec.clamp")
{
  CER_CHECK(clamp(vec3d::one(), vec3d::zero(), vec3d(2, 2, 2)) == vec3d::one());
  CER_CHECK(clamp(vec3d::one(), vec3d::zero(), vec3d::one()) == vec3d::one());
  CER_CHECK(clamp(vec3d::zero(), vec3d::zero(), vec3d::one()) == vec3d::zero());
  CER_CHECK(clamp(vec3d(2, 0, -1), vec3d::zero(), vec3d::one()) == vec3d(1, 0, 0));
  CER_CHECK(clamp(vec3d(2, 0, -1), vec3d(1, 0, -2), vec3d(3, 1, 1)) == vec3d(2, 0, -1));
}

TEST_CASE("vec.fract")
{
  CER_CHECK(fract(vec3d::zero()) == approx(vec3d::zero()));
  CER_CHECK(fract(vec3d(0.1, 0.7, 0.99999)) == approx(vec3d(0.1, 0.7, 0.99999)));
  CER_CHECK(fract(vec3d(-0.1, 0.7, -0.99999)) == approx(vec3d(-0.1, 0.7, -0.99999)));
  CER_CHECK(fract(vec3d(-1.3, 0.7, 1.99999)) == approx(vec3d(-0.3, 0.7, 0.99999)));
}

TEST_CASE("vec.mod")
{
  CER_CHECK(mod(vec3d::one(), vec3d::one()) == approx(vec3d::zero()));
  CER_CHECK(mod(vec3d(2, -1, 0), vec3d::one()) == approx(vec3d::zero()));
  CER_CHECK(mod(vec3d(6.5, -6.5, 6.5), vec3d(2, 2, -2)) == approx(vec3d(0.5, -0.5, 0.5)));
}

TEST_CASE("vec.squared_distance")
{
  constexpr auto v1 = vec3f(2.3f, 8.7878f, -2323.0f);
  constexpr auto v2 = vec3f(4.333f, -2.0f, 322.0f);

  CER_CHECK(squared_distance(v1, v1) == approx(0.0f));
  CER_CHECK(squared_distance(v1, vec3f::zero()) == approx(squared_length(v1)));
  CER_CHECK(squared_distance(v1, v2) == approx(squared_length(v1 - v2)));
}

TEST_CASE("vec.distance")
{
  constexpr auto v1 = vec3f(2.3f, 8.7878f, -2323.0f);
  constexpr auto v2 = vec3f(4.333f, -2.0f, 322.0f);

  CHECK(distance(v1, v1) == approx(0.0f));
  CHECK(distance(v1, vec3f::zero()) == approx(length(v1)));
  CHECK(distance(v1, v2) == approx(length(v1 - v2)));
}

TEST_CASE("vec.distance_c")
{
  constexpr auto v1 = vec3f(2.3f, 8.7878f, -2323.0f);
  constexpr auto v2 = vec3f(4.333f, -2.0f, 322.0f);

  CE_CHECK(distance_c(v1, v1) == approx(0.0f));
  CE_CHECK(distance_c(v1, vec3f::zero()) == approx(length_c(v1)));
  CE_CHECK(distance_c(v1, v2) == approx(length_c(v1 - v2)));
}

TEST_CASE("vec.to_homogeneous_coords")
{
  CER_CHECK(to_homogeneous_coords(vec3f(1, 2, 3)) == vec4f(1, 2, 3, 1));
}

TEST_CASE("vec.to_cartesian_coords")
{
  constexpr auto v = vec4f(2.0f, 4.0f, 8.0f, 2.0f);
  CER_CHECK(to_cartesian_coords(v) == vec3f(1.0f, 2.0f, 4.0f));
}

TEST_CASE("vec.is_colinear")
{
  CER_CHECK(is_colinear(vec3d::zero(), vec3d::zero(), vec3d::zero()));
  CER_CHECK(is_colinear(vec3d::one(), vec3d::one(), vec3d::one()));
  CER_CHECK(
    is_colinear(vec3d(0.0, 0.0, 0.0), vec3d(0.0, 0.0, 1.0), vec3d(0.0, 0.0, 2.0)));
  CER_CHECK_FALSE(
    is_colinear(vec3d(0.0, 0.0, 0.0), vec3d(1.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0)));
  CER_CHECK_FALSE(
    is_colinear(vec3d(0.0, 0.0, 0.0), vec3d(10.0, 0.0, 0.0), vec3d(0.0, 1.0, 0.0)));
}

TEST_CASE("vec.is_parallel")
{
  CHECK(is_parallel(vec3f::pos_x(), vec3f::pos_x()));
  CHECK(is_parallel(vec3f::pos_x(), vec3f::neg_x()));
  CHECK(is_parallel(vec3f::one(), vec3f::one()));
  CHECK(is_parallel(vec3f::one(), normalize(vec3f::one())));
}

TEST_CASE("vec.is_parallel_c"){
  CE_CHECK(is_parallel_c(vec3f::pos_x(), vec3f::pos_x()))
    CE_CHECK(is_parallel_c(vec3f::pos_x(), vec3f::neg_x()))
      CE_CHECK(is_parallel_c(vec3f::one(), vec3f::one()))
        CE_CHECK(is_parallel_c(vec3f::one(), normalize_c(vec3f::one())))}

// ========== rounding and error correction ==========

TEST_CASE("vec.floor")
{
  CER_CHECK(floor(vec3f::pos_x()) == vec3f::pos_x());
  CER_CHECK(floor(vec3f::one()) == vec3f::one());
  CER_CHECK(floor(vec3f::zero()) == vec3f::zero());
  CER_CHECK(floor(normalize_c(vec3f::one())) == vec3f::zero());
  CER_CHECK(floor(vec3f(0.4, 0.4, 0.4)) == vec3f::zero());
  CER_CHECK(floor(vec3f(0.4, 0.5, 0.4)) == vec3f(0, 0, 0));
  CER_CHECK(floor(vec3f(-0.4, -0.5, -0.4)) == vec3f(-1, -1, -1));
}

TEST_CASE("vec.ceil")
{
  CER_CHECK(ceil(vec3f::pos_x()) == vec3f::pos_x());
  CER_CHECK(ceil(vec3f::one()) == vec3f::one());
  CER_CHECK(ceil(vec3f::zero()) == vec3f::zero());
  CER_CHECK(ceil(normalize_c(vec3f::one())) == vec3f::one());
  CER_CHECK(ceil(vec3f(0.4, 0.4, 0.4)) == vec3f::one());
  CER_CHECK(ceil(vec3f(0.4, 0.5, 0.4)) == vec3f::one());
  CER_CHECK(ceil(vec3f(-0.4, -0.5, -0.4)) == vec3f::zero());
  CER_CHECK(ceil(vec3f(-1.4, -1.5, -1.4)) == vec3f(-1, -1, -1));
}

TEST_CASE("vec.trunc")
{
  CER_CHECK(trunc(vec3f::pos_x()) == vec3f::pos_x());
  CER_CHECK(trunc(vec3f::one()) == vec3f::one());
  CER_CHECK(trunc(vec3f::zero()) == vec3f::zero());
  CER_CHECK(trunc(normalize_c(vec3f::one())) == vec3f::zero());
  CER_CHECK(trunc(normalize_c(-vec3f::one())) == vec3f::zero());
  CER_CHECK(trunc(vec3f(0.4, 0.4, 0.4)) == vec3f::zero());
  CER_CHECK(trunc(vec3f(0.4, 0.5, 0.4)) == vec3f::zero());
  CER_CHECK(trunc(vec3f(-0.4, -0.5, -0.4)) == vec3f::zero());
  CER_CHECK(trunc(vec3f(-1.4, -1.5, -1.4)) == vec3f(-1, -1, -1));
}

TEST_CASE("vec.round")
{
  CER_CHECK(round(vec3f::pos_x()) == vec3f::pos_x());
  CER_CHECK(round(vec3f::one()) == vec3f::one());
  CER_CHECK(round(vec3f::zero()) == vec3f::zero());
  CER_CHECK(round(normalize_c(vec3f::one())) == vec3f::one());
  CER_CHECK(round(vec3f(0.4, 0.4, 0.4)) == vec3f::zero());
  CER_CHECK(round(vec3f(0.4, 0.5, 0.4)) == vec3f(0, 1, 0));
  CER_CHECK(round(vec3f(-0.4, -0.5, -0.4)) == vec3f(0, -1, 0));
}

TEST_CASE("vec.snapDown")
{
  CER_CHECK(snapDown(vec3f::zero(), vec3f::one()) == vec3f::zero());
  CER_CHECK(snapDown(vec3f(+0.4, +0.5, +0.6), vec3f::one()) == vec3f::zero());
  CER_CHECK(snapDown(vec3f(-0.4, -0.5, -0.6), vec3f::one()) == vec3f::zero());
  CER_CHECK(snapDown(vec3f(+1.4, +1.5, +1.6), vec3f::one()) == +vec3f::one());
  CER_CHECK(snapDown(vec3f(-1.4, -1.5, -1.6), vec3f::one()) == -vec3f::one());
  CER_CHECK(snapDown(vec3f(+1.4, +1.5, +1.6), vec3f(2, 2, 2)) == vec3f::zero());
  CER_CHECK(snapDown(vec3f(-1.4, -1.5, -1.6), vec3f(2, 2, 2)) == vec3f::zero());
  CER_CHECK(snapDown(vec3f(+1.4, +1.5, +1.6), vec3f(2, 1, 1)) == vec3f(0, +1, +1));
  CER_CHECK(snapDown(vec3f(-1.4, -1.5, -1.6), vec3f(2, 1, 1)) == vec3f(0, -1, -1));
}

TEST_CASE("vec.snapUp")
{
  CER_CHECK(snapUp(vec3f::zero(), vec3f::one()) == vec3f::zero());
  CER_CHECK(snapUp(vec3f(+0.4, +0.5, +0.6), vec3f::one()) == +vec3f::one());
  CER_CHECK(snapUp(vec3f(-0.4, -0.5, -0.6), vec3f::one()) == -vec3f::one());
  CER_CHECK(snapUp(vec3f(+1.4, +1.5, +1.6), vec3f::one()) == +vec3f(+2, +2, +2));
  CER_CHECK(snapUp(vec3f(-1.4, -1.5, -1.6), vec3f::one()) == -vec3f(+2, +2, +2));
  CER_CHECK(snapUp(vec3f(+1.4, +1.5, +1.6), vec3f(3, 3, 3)) == vec3f(+3, +3, +3));
  CER_CHECK(snapUp(vec3f(-1.4, -1.5, -1.6), vec3f(3, 3, 3)) == vec3f(-3, -3, -3));
  CER_CHECK(snapUp(vec3f(+1.4, +1.5, +1.6), vec3f(3, 1, 1)) == vec3f(+3, +2, +2));
  CER_CHECK(snapUp(vec3f(-1.4, -1.5, -1.6), vec3f(3, 1, 1)) == vec3f(-3, -2, -2));
}

TEST_CASE("vec.snap")
{
  CER_CHECK(snap(vec2f(7.0f, -3.0f), vec2f(4.0f, 12.0f)) == vec2f(8.0f, 0.0f));
  CER_CHECK(snap(vec2f(7.0f, -5.0f), vec2f(-4.0f, -2.0f)) == vec2f(8.0f, -6.0f));
  CER_CHECK(snap(vec2f(-7.0f, 5.0f), vec2f(-4.0f, -2.0f)) == vec2f(-8.0f, 6.0f));
}

TEST_CASE("vec.correct")
{
  CER_CHECK(correct(vec3f(1.1, 2.2, 3.3)) == vec3f(1.1, 2.2, 3.3));
  CER_CHECK(correct(vec3f(1.1, 2.2, 3.3), 0, 0.4f) == vec3f(1, 2, 3));
  CER_CHECK(correct(vec3f(1.1, 2.2, 3.3), 1, 0.4f) == vec3f(1.1, 2.2, 3.3));
}

TEST_CASE("vec.is_between")
{
  CHECK(is_between(vec3f(1, 0, 0), vec3f(0, 0, 0), vec3f(2, 0, 0)));
  CHECK(is_between(vec3f(1, 0, 0), vec3f(2, 0, 0), vec3f(0, 0, 0)));
  CHECK(is_between(vec3f(1, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
  CHECK(is_between(vec3f(0, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
  CHECK_FALSE(is_between(vec3f(2, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
}

TEST_CASE("vec.is_between_c")
{
  CE_CHECK(is_between_c(vec3f(1, 0, 0), vec3f(0, 0, 0), vec3f(2, 0, 0)));
  CE_CHECK(is_between_c(vec3f(1, 0, 0), vec3f(2, 0, 0), vec3f(0, 0, 0)));
  CE_CHECK(is_between_c(vec3f(1, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
  CE_CHECK(is_between_c(vec3f(0, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
  CE_CHECK_FALSE(is_between_c(vec3f(2, 0, 0), vec3f(1, 0, 0), vec3f(0, 0, 0)));
}

TEST_CASE("vec.average")
{
  constexpr auto vecs = std::array{vec3f(1, 1, 1), vec3f(1, 1, 1), vec3f(2, 2, 2)};
  CER_CHECK(
    average(std::begin(vecs), std::end(vecs)) == vec3f(4.0 / 3.0, 4.0 / 3.0, 4.0 / 3.0));
}

/**
 * rotates vec3f::pos_x() by the given number of degrees CCW wrt the positive Z axis
 */
static vec3f rotate_pos_x_by_degrees(const float degrees)
{
  const auto M = rotation_matrix(vec3f::pos_z(), to_radians(degrees));
  const auto rotatedVec = vec3f(M * vec3f::pos_x());
  return rotatedVec;
}

TEST_CASE("vec.measure_angle")
{
  CHECK(measure_angle(vec3f::pos_x(), vec3f::pos_x(), vec3f::pos_z()) == approx(0.0f));
  CHECK(
    measure_angle(vec3f::pos_y(), vec3f::pos_x(), vec3f::pos_z())
    == approx(Cf::half_pi()));
  CHECK(
    measure_angle(vec3f::neg_x(), vec3f::pos_x(), vec3f::pos_z()) == approx(Cf::pi()));
  CHECK(
    measure_angle(vec3f::neg_y(), vec3f::pos_x(), vec3f::pos_z())
    == approx(3.0f * Cf::half_pi()));
  CHECK(
    to_degrees(
      measure_angle(rotate_pos_x_by_degrees(0.13f), vec3f::pos_x(), vec3f::pos_z()))
    == approx(0.13f, 0.001f));
  CHECK(
    to_degrees(
      measure_angle(rotate_pos_x_by_degrees(15.13f), vec3f::pos_x(), vec3f::pos_z()))
    == approx(15.13f, 0.001f));
  CHECK(
    to_degrees(
      measure_angle(rotate_pos_x_by_degrees(359.95f), vec3f::pos_x(), vec3f::pos_z()))
    == approx(359.95f, 0.002f));
}
} // namespace vm
