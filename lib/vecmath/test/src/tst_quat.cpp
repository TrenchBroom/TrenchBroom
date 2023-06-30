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
#include <vecmath/quat.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("quat.constructor_default")
{
  constexpr auto q = quatf();
  CER_CHECK(q.r == 0.0f);
  CER_CHECK(is_zero(q.v, vm::Cf::almost_zero()));
}

TEST_CASE("quat.construtor_with_rotation")
{
  const auto angle = to_radians(15.0f);
  const auto axis = normalize(vec3f(1, 2, 3));
  const auto q = quatf(axis, angle);

  CHECK(q.r == approx(std::cos(angle / 2.0f)));
  CHECK(q.v == approx(axis * std::sin(angle / 2.0f)));
}

TEST_CASE("quat.constructor_with_vector_rotation")
{
  const auto from = vec3f(0, 1, 0);
  const auto to = vec3f(1, 0, 0);
  const auto q = quatf(from, to);
  CHECK(q * from == approx(to));
}

TEST_CASE("quat.constructor_with_opposite_vector_rotation")
{
  for (std::size_t i = 0; i < 3; ++i)
  {
    auto from = vec3d(0, 0, 0);
    auto to = vec3d(0, 0, 0);

    from[i] = 1.0;
    to[i] = -1.0;

    const auto q = quatd(from, to);
    CHECK(q * from == approx(to));
    // The quaternion axis should be perpendicular to both from and to vectors
    CHECK(dot(q.axis(), from) == approx(0.0));
    CHECK(dot(q.axis(), to) == approx(0.0));
  }
}

TEST_CASE("quat.constructor_with_equal_vector_rotation")
{
  for (std::size_t i = 0; i < 3; ++i)
  {
    auto from = vec3d(0, 0, 0);
    from[i] = 1.0;

    const auto to = from;
    const auto q = quatd(from, to);
    CHECK(q * from == approx(to));
  }
}

TEST_CASE("quat.angle")
{
  const auto angle = to_radians(15.0f);
  const auto q = quatf(vec3f::pos_z(), angle);

  CHECK(q.angle() == approx(angle, 0.001f));
}

TEST_CASE("quat.axis")
{
  CHECK(quatd().axis() == approx(vec3d::zero()));
  CHECK(quatd(vec3d::pos_z(), to_radians(45.0)).axis() == approx(vec3d::pos_z()));
  CHECK(
    quatd(normalize(vec3d(1, 1, 0)), to_radians(25.0)).axis()
    == approx(normalize(vec3d(1, 1, 0))));
}

TEST_CASE("quat.conjugate")
{
  // create quaternion with axis pos_z and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0, 0, 0.1305262f));
  constexpr auto p = q.conjugate();

  CER_CHECK(p.v == approx(-q.v));
}

TEST_CASE("quat.is_equal")
{
  CER_CHECK(is_equal(quatd(), quatd(), 0.0))

  // create quaternion with axis pos_z and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0, 0, 0.1305262f));
  CER_CHECK(is_equal(q, q, 0.0f));
  CER_CHECK(is_equal(q, -q, 0.0f));
}

TEST_CASE("quat.operator_equal")
{
  CER_CHECK(quatd() == quatd())

  // create quaternion with axis pos_z and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0, 0, 0.1305262f));
  constexpr auto p = quatf(0.991444885f, vec3f(0.1305262f, 0, 0));

  CER_CHECK(q == q);
  CER_CHECK(q == -q);
  CER_CHECK(p == p);
  CER_CHECK(p == -p);
  CER_CHECK_FALSE(q == p);
}

TEST_CASE("quat.operator_not_equal")
{
  CER_CHECK_FALSE(quatd() != quatd());

  // create quaternion with axis pos_z and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0, 0, 0.1305262f));
  constexpr auto p = quatf(0.991444885f, vec3f(0.1305262f, 0, 0));

  CER_CHECK_FALSE(q != q);
  CER_CHECK_FALSE(q != -q);
  CER_CHECK_FALSE(p != p);
  CER_CHECK_FALSE(p != -p);
  CER_CHECK(q != p);
}

TEST_CASE("quat.operator_unary_plus")
{
  CER_CHECK(+quatf() == quatf());
}

TEST_CASE("quat.operator_unary_minus")
{
  // create quaternion with axis pos_x and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0.1305262f, 0, 0));
  constexpr auto nq = -q;

  CER_CHECK(nq.r == approx(-(q.r)));
  CER_CHECK(nq.v == approx(q.v));
}

TEST_CASE("quat.operator_multiply_scalar_right")
{
  // create quaternion with axis pos_x and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0.1305262f, 0, 0));
  constexpr auto p = q * 2.0f;
  CER_CHECK(p.r == approx(q.r * 2.0f));
}

TEST_CASE("quat.operator_multiply_scalar_left")
{
  // create quaternion with axis pos_x and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0.1305262f, 0, 0));
  constexpr auto p = 2.0f * q;
  CER_CHECK(p.r == approx(q.r * 2.0f));
}

TEST_CASE("quat.operator_multiply_quaternions")
{
  // constexpr auto angle1 = to_radians(15.0f);
  // create quaternion with axis pos_z and angle 15.0f to_degrees
  constexpr auto q1 = quatf(0.991444885f, vec3f(0, 0, 0.1305262f));

  // const float angle2 = to_radians(10.0f);
  // create quaternion with axis pos_z and angle 15.0f to_degrees
  constexpr auto q2 = quatf(0.99619472f, vec3f(0, 0, 0.0871557369f));
  constexpr quatf q = q1 * q2;

  constexpr auto v = vec3f::pos_x();
  constexpr auto w = q * v;

  constexpr auto cos_a1_a2 = 0.906307756f; // std::cos(angle1 + angle2)
  constexpr auto sin_a1_a2 = 0.42261827f;  // std::sin(angle1 + angle2)

  CER_CHECK(w == approx(vec3f(cos_a1_a2, sin_a1_a2, 0.0f)));
}

TEST_CASE("quat.operator_multiply_vector")
{
  // constexpr auto angle = to_radians(15.0f);
  // create quaternion with axis pos_z and angle 15.0f to_degrees
  constexpr auto q = quatf(0.991444885f, vec3f(0, 0, 0.1305262f));
  constexpr auto x = vec3f::pos_x();

  constexpr auto cos_a = 0.965925812f; // std::cos(angle);
  constexpr auto sin_a = 0.258819044f; // std::sin(angle);

  CER_CHECK(q * x == approx(vec3f(cos_a, sin_a, 0)));
}
} // namespace vm
