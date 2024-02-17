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
#include "vecmath/mat.h"
#include "vecmath/mat_io.h"
#include "vecmath/vec.h"
#include "vecmath/vec_io.h"

#include <sstream>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("mat.constructor_default")
{
  CER_CHECK(mat4x4d() == mat4x4d::identity());
}

TEST_CASE("mat.constructor_initializer_list")
{
  constexpr auto m1 = mat3x3d({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0});
  CER_CHECK(m1[0][0] == approx(1.0));
  CER_CHECK(m1[1][0] == approx(2.0));
  CER_CHECK(m1[2][0] == approx(3.0));
  CER_CHECK(m1[0][1] == approx(4.0));
  CER_CHECK(m1[1][1] == approx(5.0));
  CER_CHECK(m1[2][1] == approx(6.0));
  CER_CHECK(m1[0][2] == approx(7.0));
  CER_CHECK(m1[1][2] == approx(8.0));
  CER_CHECK(m1[2][2] == approx(9.0));

  constexpr auto m2 =
    mat<double, 4, 3>({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0});
  CER_CHECK(m2[0][0] == approx(1.0));
  CER_CHECK(m2[1][0] == approx(2.0));
  CER_CHECK(m2[2][0] == approx(3.0));
  CER_CHECK(m2[0][1] == approx(4.0));
  CER_CHECK(m2[1][1] == approx(5.0));
  CER_CHECK(m2[2][1] == approx(6.0));
  CER_CHECK(m2[0][2] == approx(7.0));
  CER_CHECK(m2[1][2] == approx(8.0));
  CER_CHECK(m2[2][2] == approx(9.0));
  CER_CHECK(m2[0][3] == approx(10.0));
  CER_CHECK(m2[1][3] == approx(11.0));
  CER_CHECK(m2[2][3] == approx(12.0));
}

TEST_CASE("mat.componentwise_constructor_with_matching_type")
{
  constexpr auto m1 = mat3x3d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  CER_CHECK(m1[0][0] == approx(1.0));
  CER_CHECK(m1[1][0] == approx(2.0));
  CER_CHECK(m1[2][0] == approx(3.0));
  CER_CHECK(m1[0][1] == approx(4.0));
  CER_CHECK(m1[1][1] == approx(5.0));
  CER_CHECK(m1[2][1] == approx(6.0));
  CER_CHECK(m1[0][2] == approx(7.0));
  CER_CHECK(m1[1][2] == approx(8.0));
  CER_CHECK(m1[2][2] == approx(9.0));

  constexpr auto m2 =
    mat<double, 4, 3>(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0);
  CER_CHECK(m2[0][0] == approx(1.0));
  CER_CHECK(m2[1][0] == approx(2.0));
  CER_CHECK(m2[2][0] == approx(3.0));
  CER_CHECK(m2[0][1] == approx(4.0));
  CER_CHECK(m2[1][1] == approx(5.0));
  CER_CHECK(m2[2][1] == approx(6.0));
  CER_CHECK(m2[0][2] == approx(7.0));
  CER_CHECK(m2[1][2] == approx(8.0));
  CER_CHECK(m2[2][2] == approx(9.0));
  CER_CHECK(m2[0][3] == approx(10.0));
  CER_CHECK(m2[1][3] == approx(11.0));
  CER_CHECK(m2[2][3] == approx(12.0));
}

TEST_CASE("mat.componentwise_constructor_with_mixed_types")
{
  constexpr auto m1 = mat3x3d(1.0f, 2.0, 3, 4.0f, 5.0, 6, 7.0f, 8.0, 9);
  CER_CHECK(m1[0][0] == approx(1.0));
  CER_CHECK(m1[1][0] == approx(2.0));
  CER_CHECK(m1[2][0] == approx(3.0));
  CER_CHECK(m1[0][1] == approx(4.0));
  CER_CHECK(m1[1][1] == approx(5.0));
  CER_CHECK(m1[2][1] == approx(6.0));
  CER_CHECK(m1[0][2] == approx(7.0));
  CER_CHECK(m1[1][2] == approx(8.0));
  CER_CHECK(m1[2][2] == approx(9.0));

  constexpr auto m2 =
    mat<double, 4, 3>(1.0f, 2.0, 3, 4.0f, 5.0, 6, 7.0f, 8.0, 9, 10.0f, 11.0, 12);
  CER_CHECK(m2[0][0] == approx(1.0));
  CER_CHECK(m2[1][0] == approx(2.0));
  CER_CHECK(m2[2][0] == approx(3.0));
  CER_CHECK(m2[0][1] == approx(4.0));
  CER_CHECK(m2[1][1] == approx(5.0));
  CER_CHECK(m2[2][1] == approx(6.0));
  CER_CHECK(m2[0][2] == approx(7.0));
  CER_CHECK(m2[1][2] == approx(8.0));
  CER_CHECK(m2[2][2] == approx(9.0));
  CER_CHECK(m2[0][3] == approx(10.0));
  CER_CHECK(m2[1][3] == approx(11.0));
  CER_CHECK(m2[2][3] == approx(12.0));
}

TEST_CASE("mat.converting_constructor")
{
  constexpr auto from = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  constexpr auto to = mat4x4f(1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0);
  CER_CHECK(mat4x4f(from) == to);
}

TEST_CASE("mat.fill")
{
  CER_CHECK(
    mat4x4f::fill(1.0f) == mat4x4f(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
  CER_CHECK(
    mat4x4f::fill(-2.0f)
    == mat4x4f(-2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2));
}

TEST_CASE("mat.identity")
{
  CER_CHECK(
    mat4x4f::identity() == mat4x4f(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
}

TEST_CASE("mat.zero")
{
  CER_CHECK(mat4x4f::zero() == mat4x4f(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

TEST_CASE("mat.rotate_x_90_cw")
{
  constexpr auto m = mat4x4d::rot_90_x_cw();
  constexpr auto v = vec4d::pos_y();
  CER_CHECK(m * v == approx(vec4d::neg_z()));
}

TEST_CASE("mat.rotate_y_90_cw")
{
  constexpr auto m = mat4x4d::rot_90_y_cw();
  constexpr auto v = vec4d::pos_x();
  CER_CHECK(m * v == approx(vec4d::pos_z()));
}

TEST_CASE("mat.rotate_z_90_cw")
{
  constexpr auto m = mat4x4d::rot_90_z_cw();
  constexpr auto v = vec4d::pos_y();
  CER_CHECK(m * v == approx(vec4d::pos_x()));
}

TEST_CASE("mat.rotate_x_90_ccw")
{
  constexpr auto m = mat4x4d::rot_90_x_ccw();
  constexpr auto v = vec4d::pos_y();
  CER_CHECK(m * v == approx(vec4d::pos_z()));
}

TEST_CASE("mat.rotate_y_90_ccw")
{
  constexpr auto m = mat4x4d::rot_90_y_ccw();
  constexpr auto v = vec4d::pos_x();
  CER_CHECK(m * v == approx(vec4d::neg_z()));
}

TEST_CASE("mat.rotate_z_90_ccw")
{
  constexpr auto m = mat4x4d::rot_90_z_ccw();
  constexpr auto v = vec4d::pos_x();
  CER_CHECK(m * v == approx(vec4d::pos_y()));
}

TEST_CASE("mat.rotate_x_180")
{
  constexpr auto m = mat4x4d::rot_180_x();
  constexpr auto v = vec4d::pos_y();
  CER_CHECK(m * v == approx(vec4d::neg_y()));
}

TEST_CASE("mat.rotate_y_180")
{
  constexpr auto m = mat4x4d::rot_180_y();
  constexpr auto v = vec4d::pos_x();
  CER_CHECK(m * v == approx(vec4d::neg_x()));
}

TEST_CASE("mat.rotate_z_180")
{
  constexpr auto m = mat4x4d::rot_180_z();
  constexpr auto v = vec4d::pos_y();
  CER_CHECK(m * v == approx(vec4d::neg_y()));
}

TEST_CASE("mat.mirror_x")
{
  constexpr auto m = mat4x4d::mirror_x();
  constexpr auto v = vec4d(1.0, 1.0, 1.0, 0.0);
  CER_CHECK(m * v == approx(vec4d(-1.0, 1.0, 1.0, 0.0)));
}

TEST_CASE("mat.mirror_y")
{
  constexpr auto m = mat4x4d::mirror_y();
  constexpr auto v = vec4d(1.0, 1.0, 1.0, 0.0);
  CER_CHECK(m * v == approx(vec4d(1.0, -1.0, 1.0, 0.0)));
}

TEST_CASE("mat.mirror_z")
{
  constexpr auto m = mat4x4d::mirror_z();
  constexpr auto v = vec4d(1.0, 1.0, 1.0, 0.0);
  CER_CHECK(m * v == approx(vec4d(1.0, 1.0, -1.0, 0.0)));
}

TEST_CASE("mat.zero_out_x")
{
  constexpr auto m = mat4x4d::zero_out<0>();
  constexpr vec4d v(1.0, 1.0, 1.0, 1.0);
  CER_CHECK(m * v == approx(vec4d(0.0, 1.0, 1.0, 1.0)));
}

TEST_CASE("mat.zero_out_y")
{
  constexpr auto m = mat4x4d::zero_out<1>();
  constexpr vec4d v(1.0, 1.0, 1.0, 1.0);
  CER_CHECK(m * v == approx(vec4d(1.0, 0.0, 1.0, 1.0)));
}

TEST_CASE("mat.zero_out_z")
{
  constexpr auto m = mat4x4d::zero_out<2>();
  constexpr vec4d v(1.0, 1.0, 1.0, 1.0);
  CER_CHECK(m * v == approx(vec4d(1.0, 1.0, 0.0, 1.0)));
}

TEST_CASE("mat.operator_subscript")
{
  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0);

  CER_CHECK(m[0][0] == approx(m.v[0][0]));
  CER_CHECK(m[0][1] == approx(m.v[0][1]));
  CER_CHECK(m[0][2] == approx(m.v[0][2]));
  CER_CHECK(m[0][3] == approx(m.v[0][3]));

  CER_CHECK(m[1][0] == approx(m.v[1][0]));
  CER_CHECK(m[1][1] == approx(m.v[1][1]));
  CER_CHECK(m[1][2] == approx(m.v[1][2]));
  CER_CHECK(m[1][3] == approx(m.v[1][3]));

  CER_CHECK(m[2][0] == approx(m.v[2][0]));
  CER_CHECK(m[2][1] == approx(m.v[2][1]));
  CER_CHECK(m[2][2] == approx(m.v[2][2]));
  CER_CHECK(m[2][3] == approx(m.v[2][3]));

  CER_CHECK(m[3][0] == approx(m.v[3][0]));
  CER_CHECK(m[3][1] == approx(m.v[3][1]));
  CER_CHECK(m[3][2] == approx(m.v[3][2]));
  CER_CHECK(m[3][3] == approx(m.v[3][3]));
}

TEST_CASE("mat.compare")
{
  CER_CHECK(
    compare(
      mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4),
      mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4))
    == 0);

  CER_CHECK(
    compare(
      mat4x4d(1, 2, 3, 1, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4),
      mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4))
    < 0)

  CER_CHECK(
    compare(
      mat4x4d(1, 2, 3, 5, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4),
      mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4))
    > 0);
}

TEST_CASE("mat.is_equal")
{
  CER_CHECK(is_equal(
    mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4),
    mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4),
    0.0));
  CER_CHECK(is_equal(
    mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4),
    mat4x4d(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4),
    0.1));
  CER_CHECK(is_equal(
    mat4x4d(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4),
    mat4x4d(
      1.1, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0),
    0.11));
  CER_CHECK(is_equal(
    mat4x4d(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4),
    mat4x4d(
      1.1, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0),
    0.1));
  CER_CHECK_FALSE(is_equal(
    mat4x4d(1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4),
    mat4x4d(
      1.11, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0),
    0.1));
}

TEST_CASE("mat.is_zero")
{
  CHECK(is_zero(mat4x4d::zero(), vm::Cd::almost_zero()));
  CHECK_FALSE(is_zero(mat4x4d::identity(), vm::Cd::almost_zero()));
}

TEST_CASE("mat.operator_equal")
{
  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto n = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto o = mat4x4d(2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  CER_CHECK(m == n);
  CER_CHECK_FALSE(m == o);
}

TEST_CASE("mat.operator_not_equal")
{
  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto n = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto o = mat4x4d(2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  CER_CHECK_FALSE(m != n);
  CER_CHECK(m != o);
}

TEST_CASE("mat.operator_unary_plus")
{
  CER_CHECK(
    +mat4x4d(+1, +2, +3, +4, +1, +2, +3, +4, +1, +2, +3, +4, +1, +2, +3, +4)
    == mat4x4d(+1, +2, +3, +4, +1, +2, +3, +4, +1, +2, +3, +4, +1, +2, +3, +4));
}

TEST_CASE("mat.operator_unary_minus")
{
  CER_CHECK(
    -mat4x4d(+1, +2, +3, +4, +1, +2, +3, +4, +1, +2, +3, +4, +1, +2, +3, +4)
    == mat4x4d(-1, -2, -3, -4, -1, -2, -3, -4, -1, -2, -3, -4, -1, -2, -3, -4));
}

TEST_CASE("mat.operator_binary_plus")
{
  CER_CHECK(
    mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
      + mat4x4d(2, 2, 3, 4, 5, 8, 7, 8, 9, 11, 11, 12, 13, 14, 15, 16)
    == mat4x4d(3, 4, 6, 8, 10, 14, 14, 16, 18, 21, 22, 24, 26, 28, 30, 32));
}

TEST_CASE("mat.operator_binary_minus")
{
  CER_CHECK(
    mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
      - mat4x4d(2, 2, 3, 4, 5, 8, 7, 8, 9, 11, 11, 12, 13, 14, 15, 16)
    == mat4x4d(-1, 0, 0, 0, 0, -2, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0));
}

TEST_CASE("mat.operator_multiply_matrix")
{
  CER_CHECK(
    mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
      * mat4x4d(2, 2, 3, 4, 5, 8, 7, 8, 9, 11, 11, 12, 13, 14, 15, 16)
    == mat4x4d(
      91, 107, 110, 120, 207, 247, 254, 280, 323, 387, 398, 440, 439, 527, 542, 600));

  constexpr auto exp = mat<double, 4, 2>(39, 51, 103, 135, 167, 219, 231, 303);
  constexpr auto lhs = mat<double, 4, 3>(1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 14, 15);
  constexpr auto rhs = mat<double, 3, 2>(2, 2, 5, 8, 9, 11);

  CER_CHECK(lhs * rhs == exp)
}

TEST_CASE("mat.operator_multiply_scalar_right")
{
  CER_CHECK(
    mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) * 3.0
    == mat4x4d(3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48));
}

TEST_CASE("mat.operator_multiply_scalar_left")
{
  CER_CHECK(
    3.0 * mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
    == mat4x4d(3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48));
}

TEST_CASE("mat.operator_divide")
{
  CER_CHECK(
    mat4x4d(3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48) / 3.0
    == mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
}

TEST_CASE("mat.operator_multiply_vector_right")
{
  constexpr auto v = vec4d(1, 2, 3, 1);
  CER_CHECK(mat4x4d::identity() * v == approx(v));

  constexpr auto exp = vec4d(18, 46, 74, 102);
  CER_CHECK(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) * v == exp);

  CER_CHECK(
    mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) * v.xyz()
    == to_cartesian_coords(exp));
}

TEST_CASE("mat.operator_multiply_vector_left")
{
  constexpr auto v = vec4d(1, 2, 3, 1);
  CER_CHECK(v * mat4x4d::identity() == approx(v));

  constexpr auto exp = vec4d(51, 58, 65, 72);
  CER_CHECK(v * mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16) == exp);

  CER_CHECK(
    v.xyz() * mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16)
    == to_cartesian_coords(exp));
}

TEST_CASE("mat.set")
{
  CER_CHECK(
    set(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), 0, 0, 0.0)
    == mat4x4d(0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
  CER_CHECK(
    set(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), 0, 2, 0.0)
    == mat4x4d(1, 2, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16));
  CER_CHECK(
    set(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), 3, 2, 0.0)
    == mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 16));
}

TEST_CASE("mat.transpose")
{
  CER_CHECK(
    transpose(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16))
    == mat4x4d(1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 4, 8, 12, 16));
}

constexpr mat2x2d slice(const mat4x4d& m, std::size_t r, std::size_t c)
{
  return slice<2, 2>(m, r, c);
}

TEST_CASE("mat.slice")
{
  CER_CHECK(
    slice(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), 1, 1)
    == mat2x2d(6, 7, 10, 11));
  CER_CHECK(
    slice(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), 2, 1)
    == mat2x2d(10, 11, 14, 15));
  CER_CHECK(
    slice(mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16), 2, 2)
    == mat2x2d(11, 12, 15, 16));
}

TEST_CASE("mat.extractMinor")
{
  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto m00 = mat3x3d(6, 7, 8, 10, 11, 12, 14, 15, 16);
  constexpr auto m33 = mat3x3d(1, 2, 3, 5, 6, 7, 9, 10, 11);
  constexpr auto m12 = mat3x3d(1, 2, 4, 9, 10, 12, 13, 14, 16);
  constexpr auto m21 = mat3x3d(1, 3, 4, 5, 7, 8, 13, 15, 16);
  CER_CHECK(extract_minor(m, 0, 0) == approx(m00));
  CER_CHECK(extract_minor(m, 3, 3) == approx(m33));
  CER_CHECK(extract_minor(m, 1, 2) == approx(m12));
  CER_CHECK(extract_minor(m, 2, 1) == approx(m21));
}

TEST_CASE("mat.compute_determinant")
{
  constexpr auto m1 = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto m2 = mat4x4d(65, 12, -3, -5, -5, 1, 0, 0, 19, 10, 11, 8, 0, 1, -8, 3);
  constexpr auto m3 = mat4x4d(3, 2, -1, 4, 2, 1, 5, 7, 0, 5, 2, -6, -1, 2, 1, 0);
  CER_CHECK(compute_determinant(mat4x4d::zero()) == approx(0.0));
  CER_CHECK(compute_determinant(mat4x4d::identity()) == approx(1.0));
  CER_CHECK(compute_determinant(m1) == approx(0.0));
  CER_CHECK(compute_determinant(m2) == approx(15661.0));
  CER_CHECK(compute_determinant(m3) == approx(-418.0));
}

TEST_CASE("mat.compute_adjugate")
{
  constexpr auto m1 = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto m2 = mat4x4d(65, 12, -3, -5, -5, 1, 0, 0, 19, 10, 11, 8, 0, 1, -8, 3);
  constexpr auto m3 = mat4x4d(3, 2, -1, 4, 2, 1, 5, 7, 0, 5, 2, -6, -1, 2, 1, 0);
  constexpr auto r1 = mat4x4d(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  constexpr auto r2 = mat4x4d(
    97,
    -1685,
    49,
    31,
    485,
    7236,
    245,
    155,
    -167,
    -651,
    400,
    -1345,
    -607,
    -4148,
    985,
    1582);
  constexpr auto r3 =
    mat4x4d(-47, -28, -64, 221, -56, 20, -14, -128, 65, -68, -36, 59, -25, -6, 46, -87);

  CER_CHECK(compute_adjugate(mat4x4d::identity()) == approx(mat4x4d::identity()));
  CER_CHECK(compute_adjugate(mat4x4d::zero()) == approx(mat4x4d::zero()));
  CER_CHECK(compute_adjugate(m1) == approx(r1));
  CER_CHECK(compute_adjugate(m2) == approx(r2));
  CER_CHECK(compute_adjugate(m3) == approx(r3));
}

#define CER_CHECK_INVERTIBLE(exp, mat)                                                   \
  {                                                                                      \
    constexpr auto _i_r = invert((mat));                                                 \
    CER_CHECK(std::get<0>(_i_r)) CER_CHECK(std::get<1>(_i_r) == approx(exp));            \
  }
#define CER_CHECK_NOT_INVERTIBLE(mat)                                                    \
  {                                                                                      \
    constexpr auto _i_r = invert((mat));                                                 \
    CER_CHECK_FALSE(std::get<0>(_i_r))                                                   \
  }

TEST_CASE("mat.invert")
{
  constexpr auto m1 = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
  constexpr auto m2 = mat4x4d(65, 12, -3, -5, -5, 1, 0, 0, 19, 10, 11, 8, 0, 1, -8, 3);
  constexpr auto m3 = mat4x4d(0, -1, 0, 0, 0, 0, 1, 128, -1, 0, 0, 0, 0, 0, 0, 1);
  constexpr auto m4 = mat4x4d(0, 0, -1, 0, -1, 0, 0, 0, 0, 1, 0, -128, 0, 0, 0, 1);
  constexpr auto r2 = mat4x4d(
    0.0061937296468936,
    -0.10759210778367,
    0.0031287912649256,
    0.0019794393716876,
    0.030968648234468,
    0.46203946108167,
    0.015643956324628,
    0.0098971968584382,
    -0.01066343145393,
    -0.04156822680544,
    0.025541153183066,
    -0.08588212757806,
    -0.038758699955303,
    -0.2648617585084,
    0.062895089713301,
    0.10101526083903);

  CER_CHECK_INVERTIBLE(mat4x4d::identity(), mat4x4d::identity())
  CER_CHECK_INVERTIBLE(r2, m2)
  CER_CHECK_INVERTIBLE(m4, m3)
  CER_CHECK_NOT_INVERTIBLE(mat4x4d::zero())
  CER_CHECK_NOT_INVERTIBLE(m1)
}

TEST_CASE("mat.lup_solve")
{
  constexpr auto A = mat4x4d(
    0.93629336358419923,
    -0.27509584731824366,
    0.21835066314633442,
    87.954817941228995,
    0.28962947762551555,
    0.95642508584923236,
    -0.03695701352462509,
    120.90975499501228,
    -0.19866933079506122,
    -0.09784339500725571,
    0.97517032720181584,
    87.434439141401043,
    0,
    0,
    0,
    1);
  constexpr auto x = vec4d(20, -60, 32, 1);
  constexpr auto b = A * x;

  // solve for x
  constexpr auto result = lup_solve(A, b);
  constexpr auto success = std::get<0>(result);
  constexpr auto x2 = std::get<1>(result);

  CER_CHECK(success)
  CER_CHECK(x2 == approx(x));
  CER_CHECK(A * x2 == approx(b));
}
} // namespace vm
