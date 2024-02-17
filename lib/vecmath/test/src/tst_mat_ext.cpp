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
#include "vecmath/mat_ext.h"
#include "vecmath/mat_io.h"
#include "vecmath/vec.h"
#include "vecmath/vec_io.h"

#include <cstdlib>
#include <ctime>

#include <catch2/catch.hpp>

namespace vm
{
TEST_CASE("mat_ext.operator_multiply_vectors_right")
{
  const auto v =
    std::vector<vec4d>{vec4d(1, 2, 3, 1), vec4d(2, 3, 4, 1), vec4d(3, 2, 7, 23)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  const auto r = std::vector<vec4d>{
    vec4d(18, 46, 74, 102), vec4d(24, 64, 104, 144), vec4d(120, 260, 400, 540)};

  const auto o = m * v;
  for (size_t i = 0; i < 3; i++)
  {
    CHECK(o[i] == approx(r[i]));
  }
}

TEST_CASE("mat_ext.operator_multiply_array_right")
{
  constexpr auto v =
    std::array<vec4d, 3>{vec4d(1, 2, 3, 1), vec4d(2, 3, 4, 1), vec4d(3, 2, 7, 23)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  constexpr auto r = std::array<vec4d, 3>{
    vec4d(18, 46, 74, 102), vec4d(24, 64, 104, 144), vec4d(120, 260, 400, 540)};

  constexpr auto o = m * v;
  CER_CHECK(o[0] == approx(r[0]));
  CER_CHECK(o[1] == approx(r[1]));
  CER_CHECK(o[2] == approx(r[2]));
}

TEST_CASE("mat_ext.operator_multiply_vectors_right_lower_dimension")
{
  const auto v = std::vector<vec3d>{
    vec3d(1.0, 2.0, 3.0),
    vec3d(2.0, 3.0, 4.0),
    vec3d(3.0 / 23.0, 2.0 / 23.0, 7.0 / 23.0)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  const auto r = std::vector<vec3d>{
    to_cartesian_coords(vec4d(18, 46, 74, 102)),
    to_cartesian_coords(vec4d(24, 64, 104, 144)),
    to_cartesian_coords(vec4d(120, 260, 400, 540))};

  const auto o = m * v;
  for (size_t i = 0; i < 3; i++)
  {
    CHECK(o[i] == approx(r[i]));
  }
}

TEST_CASE("mat_ext.operator_multiply_array_right_lower_dimension")
{
  constexpr auto v = std::array<vec3d, 3>{
    vec3d(1.0, 2.0, 3.0),
    vec3d(2.0, 3.0, 4.0),
    vec3d(3.0 / 23.0, 2.0 / 23.0, 7.0 / 23.0)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  constexpr auto r = std::array<vec3d, 3>{
    to_cartesian_coords(vec4d(18, 46, 74, 102)),
    to_cartesian_coords(vec4d(24, 64, 104, 144)),
    to_cartesian_coords(vec4d(120, 260, 400, 540))};

  constexpr auto o = m * v;
  CER_CHECK(o[0] == approx(r[0]));
  CER_CHECK(o[1] == approx(r[1]));
  CER_CHECK(o[2] == approx(r[2]));
}

TEST_CASE("mat_ext.operator_multiply_vectors_left")
{
  const auto v =
    std::vector<vec4d>{vec4d(1, 2, 3, 1), vec4d(2, 3, 4, 1), vec4d(3, 2, 3, 23)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  const auto r = std::vector<vec4d>{
    vec4d(51, 58, 65, 72),
    vec4d(66, 76, 86, 96),
    vec4d(339, 370, 401, 432),
  };

  const auto o = v * m;
  for (size_t i = 0; i < 3; i++)
  {
    CHECK(o[i] == approx(r[i]));
  }
}

TEST_CASE("mat_ext.operator_multiply_array_left")
{
  constexpr auto v =
    std::array<vec4d, 3>{vec4d(1, 2, 3, 1), vec4d(2, 3, 4, 1), vec4d(3, 2, 3, 23)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  constexpr auto r = std::array<vec4d, 3>{
    vec4d(51, 58, 65, 72),
    vec4d(66, 76, 86, 96),
    vec4d(339, 370, 401, 432),
  };

  constexpr auto o = v * m;
  CER_CHECK(o[0] == approx(r[0]));
  CER_CHECK(o[1] == approx(r[1]));
  CER_CHECK(o[2] == approx(r[2]));
}

TEST_CASE("mat_ext.operator_multiply_vectors_left_lower_dimension")
{
  const auto v = std::vector<vec3d>{
    vec3d(1.0, 2.0, 3.0),
    vec3d(2.0, 3.0, 4.0),
    vec3d(3.0 / 23.0, 2.0 / 23.0, 3.0 / 23.0)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  const auto r = std::vector<vec3d>{
    to_cartesian_coords(vec4d(51.0, 58.0, 65.0, 72.0)),
    to_cartesian_coords(vec4d(66.0, 76.0, 86.0, 96.0)),
    to_cartesian_coords(vec4d(339.0, 370.0, 401.0, 432.0))};

  const auto o = v * m;
  for (size_t i = 0; i < 3; i++)
  {
    CHECK(o[i] == approx(r[i]));
  }
}

TEST_CASE("mat_ext.operator_multiply_array_left_lower_dimension")
{
  constexpr auto v = std::array<vec3d, 3>{
    vec3d(1.0, 2.0, 3.0),
    vec3d(2.0, 3.0, 4.0),
    vec3d(3.0 / 23.0, 2.0 / 23.0, 3.0 / 23.0)};

  constexpr auto m = mat4x4d(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);

  constexpr auto r = std::array<vec3d, 3>{
    to_cartesian_coords(vec4d(51.0, 58.0, 65.0, 72.0)),
    to_cartesian_coords(vec4d(66.0, 76.0, 86.0, 96.0)),
    to_cartesian_coords(vec4d(339.0, 370.0, 401.0, 432.0))};

  constexpr auto o = v * m;
  CER_CHECK(o[0] == approx(r[0]));
  CER_CHECK(o[1] == approx(r[1]));
  CER_CHECK(o[2] == approx(r[2]));
}

TEST_CASE("mat_ext.rotation_matrix_with_euler_angles")
{
  CHECK(rotation_matrix(to_radians(90.0), 0.0, 0.0) == approx(mat4x4d::rot_90_x_ccw()));
  CHECK(rotation_matrix(0.0, to_radians(90.0), 0.0) == approx(mat4x4d::rot_90_y_ccw()));
  CHECK(rotation_matrix(0.0, 0.0, to_radians(90.0)) == approx(mat4x4d::rot_90_z_ccw()));
}

TEST_CASE("mat_ext.rotationMatrixToEulerAngles_90DegreeRotations")
{
  CHECK(
    rotation_matrix_to_euler_angles(mat4x4d::rot_90_x_ccw())
    == approx(vec3d(to_radians(90.0), 0.0, 0.0)));
  CHECK(
    rotation_matrix_to_euler_angles(mat4x4d::rot_90_y_ccw())
    == approx(vec3d(0.0, to_radians(90.0), 0.0)));
  CHECK(
    rotation_matrix_to_euler_angles(mat4x4d::rot_90_z_ccw())
    == approx(vec3d(0.0, 0.0, to_radians(90.0))));
}

TEST_CASE("mat_ext.rotation_matrix_to_euler_angles")
{
  const auto roll = to_radians(12.0);
  const auto pitch = to_radians(13.0);
  const auto yaw = to_radians(14.0);

  const auto rotMat = rotation_matrix(roll, pitch, yaw);
  const auto rollPitchYaw = rotation_matrix_to_euler_angles(rotMat);

  CHECK(rollPitchYaw.x() == approx(roll));
  CHECK(rollPitchYaw.y() == approx(pitch));
  CHECK(rollPitchYaw.z() == approx(yaw));
}

TEST_CASE("mat_ext.rotation_matrix_with_axis_and_angle")
{
  CHECK(
    rotation_matrix(vec3d::pos_x(), to_radians(90.0)) == approx(mat4x4d::rot_90_x_ccw()));
  CHECK(
    rotation_matrix(vec3d::pos_y(), to_radians(90.0)) == approx(mat4x4d::rot_90_y_ccw()));
  CHECK(
    rotation_matrix(vec3d::pos_z(), to_radians(90.0)) == approx(mat4x4d::rot_90_z_ccw()));
  CHECK(
    rotation_matrix(vec3d::pos_z(), to_radians(90.0)) * vec3d::pos_x()
    == approx(vec3d::pos_y()));
}

TEST_CASE("mat_ext.rotation_matrix_with_quaternion")
{
  CHECK(
    rotation_matrix(quatd(vec3d::pos_x(), to_radians(90.0)))
    == approx(mat4x4d::rot_90_x_ccw()));
  CHECK(
    rotation_matrix(quatd(vec3d::pos_y(), to_radians(90.0)))
    == approx(mat4x4d::rot_90_y_ccw()));
  CHECK(
    rotation_matrix(quatd(vec3d::pos_z(), to_radians(90.0)))
    == approx(mat4x4d::rot_90_z_ccw()));

  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  for (size_t i = 0; i < 10; ++i)
  {
    vec3d axis;
    for (size_t j = 0; j < 3; ++j)
    {
      axis[j] = (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
    }
    axis = normalize(axis);
    const double angle =
      (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX)) * 2.0 * Cd::pi();
    CHECK(rotation_matrix(quatd(axis, angle)) == approx(rotation_matrix(axis, angle)));
  }
}

TEST_CASE("mat_ext.translation_matrix")
{
  constexpr auto v = vec3d(2, 3, 4);
  constexpr auto t = translation_matrix(v);

  CER_CHECK(vec4d::pos_x() == approx(t[0]));
  CER_CHECK(vec4d::pos_y() == approx(t[1]));
  CER_CHECK(vec4d::pos_z() == approx(t[2]));
  CER_CHECK(vec4d(v, 1) == approx(t[3]));
}

TEST_CASE("mat.strip_translation")
{
  constexpr auto v = vec3d(2, 3, 4);
  constexpr auto t = translation_matrix(v);
  constexpr auto s = scaling_matrix(vec3d(2, 3, 4));

  CER_CHECK(strip_translation(s * t) == approx(s));
  CER_CHECK(strip_translation(t * s) == approx(s));
}

TEST_CASE("mat_ext.scaling_matrix")
{
  CER_CHECK(
    scaling_matrix(vec3d(2, 3, 4))
    == mat4x4d(2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 4, 0, 0, 0, 0, 1));
}

TEST_CASE("mat_ext.mirror_matrix")
{
  constexpr auto mirX = mirror_matrix<double>(axis::x);
  constexpr auto mirY = mirror_matrix<double>(axis::y);
  constexpr auto mirZ = mirror_matrix<double>(axis::z);

  CER_CHECK(mirX * vec3d::pos_x() == vec3d::neg_x());
  CER_CHECK(mirX * vec3d::pos_y() == vec3d::pos_y());
  CER_CHECK(mirX * vec3d::pos_z() == vec3d::pos_z());

  CER_CHECK(mirY * vec3d::pos_x() == vec3d::pos_x());
  CER_CHECK(mirY * vec3d::pos_y() == vec3d::neg_y());
  CER_CHECK(mirY * vec3d::pos_z() == vec3d::pos_z());

  CER_CHECK(mirZ * vec3d::pos_x() == vec3d::pos_x());
  CER_CHECK(mirZ * vec3d::pos_y() == vec3d::pos_y());
  CER_CHECK(mirZ * vec3d::pos_z() == vec3d::neg_z());
}

TEST_CASE("mat_ext.coordinateSystemMatrix")
{
  constexpr auto m = coordinate_system_matrix(
    vec3d::neg_x(), vec3d::neg_y(), vec3d::neg_z(), vec3d::one());
  CER_CHECK(m * vec3d::pos_x() == vec3d::neg_x() + vec3d::one());
  CER_CHECK(m * vec3d::pos_y() == vec3d::neg_y() + vec3d::one());
  CER_CHECK(m * vec3d::pos_z() == vec3d::neg_z() + vec3d::one());
}

TEST_CASE("mat_ext.plane_projection_matrix")
{
  const auto m = vm::plane_projection_matrix(-160.0, vec3d(-1.0, -0.0, 0.0));

  // The plane is at x=160, so after transforming, this point should have a z component of
  // 0. The x and y components could be anything.
  CHECK(vec3d(m * vec3d(160.0, 1.0, 2.0)).z() == 0.0);
}

TEST_CASE("mat_ext.shear_matrix")
{
  CER_CHECK(
    shear_matrix(0.0, 0.0, 0.0, 0.0, 1.0, 1.0) * vec3d::pos_z() == vec3d(1, 1, 1));
  CER_CHECK(shear_matrix(0.0, 0.0, 0.0, 0.0, 1.0, 1.0) * vec3d::zero() == vec3d(0, 0, 0));
  CER_CHECK(
    shear_matrix(0.0, 0.0, 1.0, 1.0, 0.0, 0.0) * vec3d::pos_y() == vec3d(1, 1, 1));
  CER_CHECK(shear_matrix(0.0, 0.0, 1.0, 1.0, 0.0, 0.0) * vec3d::zero() == vec3d(0, 0, 0));
  CER_CHECK(
    shear_matrix(1.0, 1.0, 0.0, 0.0, 0.0, 0.0) * vec3d::pos_x() == vec3d(1, 1, 1));
  CER_CHECK(shear_matrix(1.0, 1.0, 0.0, 0.0, 0.0, 0.0) * vec3d::zero() == vec3d(0, 0, 0));
}

TEST_CASE("mat.points_transformation_matrix")
{
  const vec3d in[3] = {{2.0, 0.0, 0.0}, {4.0, 0.0, 0.0}, {2.0, 2.0, 0.0}};

  const auto M = translation_matrix(vec3d(100.0, 100.0, 100.0))
                 * scaling_matrix(vec3d(2.0, 2.0, 2.0))
                 * rotation_matrix(vec3d::pos_z(), to_radians(90.0));

  vec3d out[3];
  for (size_t i = 0; i < 3; ++i)
  {
    out[i] = M * in[i];
  }

  // in[0]: 0,2,0, then 0,4,0, then 100, 104, 100
  // in[1]: 0,4,0, then 0,8,0, then 100, 108, 100
  // in[2]: -2,2,0, then -4,4,0, then 96, 104, 100

  const auto M2 =
    points_transformation_matrix(in[0], in[1], in[2], out[0], out[1], out[2]);
  vec3d test[3];
  for (size_t i = 0; i < 3; ++i)
  {
    test[i] = M2 * in[i];

    CHECK(test[i] == approx(out[i]));
  }
}
} // namespace vm
