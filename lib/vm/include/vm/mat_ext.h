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

#pragma once

#include "vm/bbox.h"
#include "vm/mat.h"
#include "vm/quat.h"
#include "vm/util.h"
#include "vm/vec.h"

#include <array>
#include <optional>
#include <tuple>
#include <vector>

namespace vm
{
/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
std::vector<vec<T, C>> operator*(
  const mat<T, R, C>& lhs, const std::vector<vec<T, C>>& rhs)
{
  std::vector<vec<T, C>> result;
  result.reserve(rhs.size());
  for (const auto& v : rhs)
  {
    result.push_back(lhs * v);
  }
  return result;
}

/**
 * Multiplies the given array of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam N the number of array elements
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the array of vectors
 * @param rhs the matrix
 * @return an array of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t N, std::size_t R, std::size_t C>
constexpr std::array<vec<T, C>, N> operator*(
  const mat<T, R, C>& lhs, const std::array<vec<T, C>, N>& rhs)
{
  std::array<vec<T, C>, N> result{};
  for (std::size_t i = 0u; i < N; ++i)
  {
    result[i] = lhs * rhs[i];
  }
  return result;
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
std::vector<vec<T, C - 1>> operator*(
  const mat<T, R, C>& lhs, const std::vector<vec<T, C - 1>>& rhs)
{
  std::vector<vec<T, C - 1>> result;
  result.reserve(rhs.size());
  for (const auto& v : rhs)
  {
    result.push_back(lhs * v);
  }
  return result;
}

/**
 * Multiplies the given array of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam N the number of array elements
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the array of vectors
 * @param rhs the matrix
 * @return an array of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t N, std::size_t R, std::size_t C>
constexpr std::array<vec<T, C - 1>, N> operator*(
  const mat<T, R, C>& lhs, const std::array<vec<T, C - 1>, N>& rhs)
{
  std::array<vec<T, C - 1>, N> result{};
  for (std::size_t i = 0u; i < N; ++i)
  {
    result[i] = lhs * rhs[i];
  }
  return result;
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
std::vector<vec<T, R>> operator*(
  const std::vector<vec<T, R>>& lhs, const mat<T, R, C>& rhs)
{
  std::vector<vec<T, R>> result;
  result.reserve(lhs.size());
  for (const auto& v : lhs)
  {
    result.push_back(v * rhs);
  }
  return result;
}

/**
 * Multiplies the given array of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam N the number of array elements
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the array of vectors
 * @param rhs the matrix
 * @return an array of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t N, std::size_t R, std::size_t C>
constexpr std::array<vec<T, R>, N> operator*(
  const std::array<vec<T, R>, N>& lhs, const mat<T, R, C>& rhs)
{
  std::array<vec<T, R>, N> result{};
  for (std::size_t i = 0u; i < N; ++i)
  {
    result[i] = lhs[i] * rhs;
  }
  return result;
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
std::vector<vec<T, R - 1>> operator*(
  const std::vector<vec<T, R - 1>>& lhs, const mat<T, R, C>& rhs)
{
  std::vector<vec<T, R - 1>> result;
  result.reserve(lhs.size());
  for (const auto& v : lhs)
  {
    result.push_back(v * rhs);
  }
  return result;
}

/**
 * Multiplies the given list of vectors with the given matrix.
 *
 * @tparam T the component type
 * @tparam N the number of array elements
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the list of vectors
 * @param rhs the matrix
 * @return a list of the the products of the given vectors and the given matrix
 */
template <typename T, std::size_t N, std::size_t R, std::size_t C>
constexpr std::array<vec<T, R - 1>, N> operator*(
  const std::array<vec<T, R - 1>, N>& lhs, const mat<T, R, C>& rhs)
{
  std::array<vec<T, R - 1>, N> result{};
  for (std::size_t i = 0u; i < N; ++i)
  {
    result[i] = lhs[i] * rhs;
  }
  return result;
}

/**
 * Returns a perspective camera transformation with the given parameters. The returned
 * matrix transforms from eye coordinates to clip coordinates.
 *
 * @tparam T the component type
 * @param fov the field of view, in degrees
 * @param nearPlane the distance to the near plane
 * @param farPlane the distance to the far plane
 * @param width the viewport width
 * @param height the viewport height
 * @return the perspective transformation matrix
 */
template <typename T>
constexpr mat<T, 4, 4> perspective_matrix(
  const T fov, const T nearPlane, const T farPlane, const int width, const int height)
{
  const auto vFrustum =
    std::tan(to_radians(fov) / static_cast<T>(2.0)) * static_cast<T>(0.75) * nearPlane;
  const auto hFrustum = vFrustum * static_cast<T>(width) / static_cast<T>(height);
  const auto depth = farPlane - nearPlane;

  constexpr auto zero = static_cast<T>(0.0);
  constexpr auto one = static_cast<T>(1.0);
  constexpr auto two = static_cast<T>(2.0);

  return mat<T, 4, 4>(
    nearPlane / hFrustum,
    zero,
    zero,
    zero,
    zero,
    nearPlane / vFrustum,
    zero,
    zero,
    zero,
    zero,
    -(farPlane + nearPlane) / depth,
    -two * farPlane * nearPlane / depth,
    zero,
    zero,
    -one,
    zero);
}

/**
 * Returns an orthographic camera transformation with the given parameters. The origin of
 * the given screen coordinates is at the center. The returned matrix transforms from eye
 * coordinates to clip coordinates.
 *
 * @tparam T the component type
 * @param nearPlane the distance to the near plane
 * @param farPlane the distance to the far plane
 * @param left the screen coordinate of the left border of the viewport
 * @param top the screen coordinate of the top border of the viewport
 * @param right the screen coordinate of the right border of the viewport
 * @param bottom the screen coordinate of the bottom border of the viewport
 * @return the orthographic transformation matrix
 */
template <typename T>
constexpr mat<T, 4, 4> ortho_matrix(
  const T nearPlane,
  const T farPlane,
  const T left,
  const T top,
  const T right,
  const T bottom)
{
  const auto width = right - left;
  const auto height = top - bottom;
  const auto depth = farPlane - nearPlane;

  constexpr auto zero = static_cast<T>(0.0);
  constexpr auto one = static_cast<T>(1.0);
  constexpr auto two = static_cast<T>(2.0);

  return mat<T, 4, 4>(
    two / width,
    zero,
    zero,
    -(left + right) / width,
    zero,
    two / height,
    zero,
    -(top + bottom) / height,
    zero,
    zero,
    -two / depth,
    -(farPlane + nearPlane) / depth,
    zero,
    zero,
    zero,
    one);
}

/**
 * Returns a view transformation matrix which transforms normalized device coordinates to
 * window coordinates.
 *
 * @tparam T the component type
 * @param direction the view direction
 * @param up the up vector
 * @return the view transformation matrix
 */
template <typename T>
constexpr mat<T, 4, 4> view_matrix(const vec<T, 3>& direction, const vec<T, 3>& up)
{
  const auto& f = direction;
  const auto s = cross(f, up);
  const auto u = cross(s, f);

  constexpr auto zero = static_cast<T>(0.0);
  constexpr auto one = static_cast<T>(1.0);

  return mat<T, 4, 4>(
    s[0],
    s[1],
    s[2],
    zero,
    u[0],
    u[1],
    u[2],
    zero,
    -f[0],
    -f[1],
    -f[2],
    zero,
    zero,
    zero,
    zero,
    one);
}

/**
 * Returns a matrix that will rotate a point counter clockwise by the given angles. The
 * rotation is applied in the same order the parameters are given: first roll, then pitch,
 * then yaw.
 *
 * @tparam T the component type
 * @param roll the roll angle (in radians)
 * @param pitch the pitch angle (in radians)
 * @param yaw the yaw angle (in radians)
 * @return the rotation matrix
 */
template <typename T>
mat<T, 4, 4> rotation_matrix(const T roll, const T pitch, const T yaw)
{
  constexpr auto I = static_cast<T>(1.0);
  constexpr auto O = static_cast<T>(0.0);

  const auto Cr = std::cos(roll);
  const auto Sr = std::sin(roll);
  const mat<T, 4, 4> R(
    +I, +O, +O, +O, +O, +Cr, -Sr, +O, +O, +Sr, +Cr, +O, +O, +O, +O, +I);

  const auto Cp = std::cos(pitch);
  const auto Sp = std::sin(pitch);
  const mat<T, 4, 4> P(
    +Cp, +O, +Sp, +O, +O, +I, +O, +O, -Sp, +O, +Cp, +O, +O, +O, +O, +I);

  const auto Cy = std::cos(yaw);
  const auto Sy = std::sin(yaw);
  const mat<T, 4, 4> Y(
    +Cy, -Sy, +O, +O, +Sy, +Cy, +O, +O, +O, +O, +I, +O, +O, +O, +O, +I);

  return Y * P * R;
}

/**
 * Converts the given rotation matrix to euler angles in radians.
 * The euler angles use the same convention as rotationMatrix(): first roll, then pitch,
 * then yaw.
 *
 * @tparam T the component type
 * @param rotMat the rotation matrix (must contain only rotation)
 * @return the roll, pitch, yaw in radians
 */
template <typename T>
vec<T, 3> rotation_matrix_to_euler_angles(const mat<T, 4, 4>& rotMat)
{
  // From: http://www.gregslabaugh.net/publications/euler.pdf
  // Their notation is row-major, and uses 1-based row/column indices, whereas TB is
  // column-major and 0-based. They use phi=yaw, theta=pitch, psi=roll

  T theta, psi, phi;

  if (abs(rotMat[0][2]) != T(1.0))
  {
    theta = -std::asin(rotMat[0][2]);
    const auto cosTheta = std::cos(theta);

    psi = std::atan2(rotMat[1][2] / cosTheta, rotMat[2][2] / cosTheta);
    phi = std::atan2(rotMat[0][1] / cosTheta, rotMat[0][0] / cosTheta);
  }
  else
  {
    phi = 0.0;
    if (rotMat[0][2] == T(-1.0))
    {
      theta = vm::constants<T>::half_pi();
      psi = std::atan2(rotMat[1][0], rotMat[2][0]);
    }
    else
    {
      theta = -vm::constants<T>::half_pi();
      psi = std::atan2(-rotMat[1][0], -rotMat[2][0]);
    }
  }

  return vec<T, 3>(psi, theta, phi);
}

/**
 * Returns a matrix that will rotate a point counter clockwise about the given axis by the
 * given angle.
 *
 * @tparam T the component type
 * @param axis the axis to rotate about
 * @param angle the rotation angle (in radians)
 * @return the rotation matrix
 */
template <typename T>
mat<T, 4, 4> rotation_matrix(const vec<T, 3>& axis, const T angle)
{
  const auto s = std::sin(-angle);
  const auto c = std::cos(-angle);
  const auto i = static_cast<T>(1.0) - c;

  const auto ix = i * axis[0];
  const auto ix2 = ix * axis[0];
  const auto ixy = ix * axis[1];
  const auto ixz = ix * axis[2];

  const auto iy = i * axis[1];
  const auto iy2 = iy * axis[1];
  const auto iyz = iy * axis[2];

  const auto iz2 = i * axis[2] * axis[2];

  const auto sx = s * axis[0];
  const auto sy = s * axis[1];
  const auto sz = s * axis[2];

  return mat<T, 4, 4>(
    ix2 + c,
    ixy + sz,
    ixz - sy,
    0,
    ixy - sz,
    iy2 + c,
    iyz + sx,
    0,
    ixz + sy,
    iyz - sx,
    iz2 + c,
    0,
    0,
    0,
    0,
    1);
}

/**
 * Returns a rotation matrix that performs the same rotation as the given quaternion.
 *
 * @see
 * http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
 *
 * @tparam T the component type
 * @param quat the quaternion
 * @return the rotation matrix
 */
template <typename T>
constexpr mat<T, 4, 4> rotation_matrix(const quat<T>& quat)
{
  constexpr auto one = static_cast<T>(1);
  constexpr auto two = static_cast<T>(2);

  const auto x = quat.v[0];
  const auto y = quat.v[1];
  const auto z = quat.v[2];
  const auto w = quat.r;

  const auto x2 = x * x;
  const auto y2 = y * y;
  const auto z2 = z * z;

  return mat<T, 4, 4>(
    one - two * (y2 + z2),
    two * (x * y - z * w),
    two * (x * z + y * w),
    0,
    two * (x * y + z * w),
    one - two * (x2 + z2),
    two * (y * z - x * w),
    0,
    two * (x * z - y * w),
    two * (y * z + x * w),
    one - two * (x2 + y2),
    0,
    0,
    0,
    0,
    1);
}

/**
 * Returns a matrix that will rotate the first given vector onto the second given vector
 * about their perpendicular axis. The vectors are expected to be normalized.
 *
 * @tparam T the component type
 * @param from the vector to rotate
 * @param to the vector to rotate onto
 * @return the rotation matrix
 */
template <typename T>
constexpr mat<T, 4, 4> rotation_matrix(const vec<T, 3>& from, const vec<T, 3>& to)
{
  return rotation_matrix(quat<T>(from, to));
}

/**
 * Returns a matrix that translates by the given delta.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param delta the deltas by which to translate
 * @return the translation matrix
 */
template <typename T, std::size_t S>
constexpr mat<T, S + 1, S + 1> translation_matrix(const vec<T, S>& delta)
{
  mat<T, S + 1, S + 1> translation;
  for (size_t i = 0; i < S; ++i)
  {
    translation[S][i] = delta[i];
  }
  return translation;
}

/**
 * Returns a matrix that contains only the translation part of the given transformation
 * matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the transformation matrix
 * @return the translation matrix
 */
template <typename T, std::size_t S>
constexpr mat<T, S, S> translation_matrix(const mat<T, S, S>& m)
{
  mat<T, S, S> result;
  for (size_t i = 0; i < S - 1; ++i)
  {
    result[S - 1][i] = m[S - 1][i];
  }
  return result;
}

/**
 * Strips the translation part from the given transformation matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the transformation matrix
 * @return the transformation matrix without its translation part
 */
template <typename T, std::size_t S>
constexpr mat<T, S, S> strip_translation(const mat<T, S, S>& m)
{
  mat<T, S, S> result(m);
  for (size_t i = 0; i < S - 1; ++i)
  {
    result[S - 1][i] = static_cast<T>(0.0);
  }
  return result;
}

/**
 * Returns a scaling matrix with the given scaling factors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param factors the scaling factors
 * @return the scaling matrix
 */
template <typename T, std::size_t S>
constexpr mat<T, S + 1, S + 1> scaling_matrix(const vec<T, S>& factors)
{
  mat<T, S + 1, S + 1> scaling;
  for (size_t i = 0; i < S; ++i)
  {
    scaling[i][i] = factors[i];
  }
  return scaling;
}

/**
 * Returns a matrix that mirrors along the given axis.
 *
 * @tparam T the component type
 * @param axis the axis along which to mirror
 * @return the mirroring axis
 */
template <typename T>
constexpr mat<T, 4, 4> mirror_matrix(const axis::type axis)
{
  switch (axis)
  {
  case axis::x:
    return mat<T, 4, 4>::mirror_x();
  case axis::y:
    return mat<T, 4, 4>::mirror_y();
  case axis::z:
    return mat<T, 4, 4>::mirror_z();
  default:
    return mat<T, 4, 4>::identity();
  }
}

/**
 * Returns a matrix that transforms to a coordinate system specified by the given axes and
 * offset.
 *
 * @tparam T the component type
 * @param x the X axis of the target coordinate system, expressed relative to the source
 * coordinate system
 * @param y the Y axis of the target coordinate system, expressed relative to the source
 * coordinate system
 * @param z the Z axis of the target coordinate system, expressed relative to the source
 * coordinate system
 * @param o the offset of the target coordinate system, expressed relative to the source
 * coordinate system
 * @return the transformation matrix
 */
template <typename T>
constexpr mat<T, 4, 4> coordinate_system_matrix(
  const vec<T, 3>& x, const vec<T, 3>& y, const vec<T, 3>& z, const vec<T, 3>& o)
{
  const auto result = invert(mat<T, 4, 4>(
    x[0], y[0], z[0], o[0], x[1], y[1], z[1], o[1], x[2], y[2], z[2], o[2], 0, 0, 0, 1));
  assert(std::get<0>(result));
  return std::get<1>(result);
}

/**
 * Returns a matrix that will transform a point to a coordinate system where the X and
 * Y axes are in the given plane and the Z axis is parallel to the given direction. This
 * is useful for projecting points onto a plane along a particular direction.
 *
 * @tparam T the component type
 * @param distance the distance of the plane
 * @param normal the normal of the plane
 * @param direction the projection direction
 * @return the transformation matrix
 */
template <typename T>
mat<T, 4, 4> plane_projection_matrix(
  const T distance, const vec<T, 3>& normal, const vec<T, 3>& direction)
{
  // create some coordinate system where the X and Y axes are contained within the plane
  // and the Z axis is the projection direction
  vec<T, 3> xAxis;

  switch (find_abs_max_component(normal))
  {
  case axis::x:
    xAxis = normalize(cross(normal, vec<T, 3>::pos_z()));
    break;
  default:
    xAxis = normalize(cross(normal, vec<T, 3>::pos_x()));
    break;
  }

  const auto yAxis = normalize(cross(normal, xAxis));
  const auto& zAxis = direction;

  assert(is_unit(xAxis, constants<T>::almost_zero()));
  assert(is_unit(yAxis, constants<T>::almost_zero()));
  assert(is_unit(zAxis, constants<T>::almost_zero()));

  return coordinate_system_matrix(xAxis, yAxis, zAxis, distance * normal);
}

/**
 * Returns a matrix that will transform a point to a coordinate system where the X and
 * Y axes are in the given plane and the Z axis is the plane normal. This is useful for
 * vertically projecting points onto a plane.
 *
 * @tparam T the component type
 * @param distance the distance of the plane
 * @param normal the normal of the plane
 * @return the transformation matrix
 */
template <typename T>
mat<T, 4, 4> plane_projection_matrix(const T distance, const vec<T, 3>& normal)
{
  return plane_projection_matrix(distance, normal, normal);
}

/**
 * Returns a matrix that performs a shearing transformation. In 3D, six shearing
 * directions are possible:
 *
 * - X in direction of Y
 * - X in direction of Z
 * - Y in direction of X
 * - Y in direction of Z
 * - Z in direction of X
 * - Z in direction of Y
 *
 * @tparam T the component type
 * @param Sxy amount by which to share the X axis in direction of the Y axis
 * @param Sxz amount by which to share the X axis in direction of the Z axis
 * @param Syx amount by which to share the Y axis in direction of the X axis
 * @param Syz amount by which to share the Y axis in direction of the Z axis
 * @param Szx amount by which to share the Z axis in direction of the X axis
 * @param Szy amount by which to share the Z axis in direction of the Y axis
 * @return the shearing matrix
 */
template <typename T>
constexpr mat<T, 4, 4> shear_matrix(
  const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T Szy)
{
  return mat<T, 4, 4>(1, Syx, Szx, 0, Sxy, 1, Szy, 0, Sxz, Syz, 1, 0, 0, 0, 0, 1);
}

// TODO: add documentation and tests
template <typename T>
constexpr mat<T, 4, 4> scale_bbox_matrix(
  const bbox<T, 3>& oldBBox, const bbox<T, 3>& newBBox)
{
  const auto scaleFactors = newBBox.size() / oldBBox.size();
  return translation_matrix(newBBox.min) * scaling_matrix(scaleFactors)
         * translation_matrix(-oldBBox.min);
}

// TODO: add documentation and tests
template <typename T>
constexpr mat<T, 4, 4> scale_bbox_matrix_with_anchor(
  const bbox<T, 3>& oldBBox, const vec<T, 3>& newSize, const vec<T, 3>& anchorPoint)
{
  const auto scaleFactors = newSize / oldBBox.size();
  return translation_matrix(anchorPoint) * scaling_matrix(scaleFactors)
         * translation_matrix(-anchorPoint);
}

// TODO: add documentation and tests
template <typename T>
constexpr mat<T, 4, 4> shear_bbox_matrix(
  const bbox<T, 3>& box, const vec<T, 3>& sideToShear, const vec<T, 3>& delta)
{
  const auto oldSize = box.size();

  // shearMatrix(const T Sxy, const T Sxz, const T Syx, const T Syz, const T Szx, const T
  // Szy) {
  mat<T, 4, 4> shearMat;
  if (sideToShear == vec<T, 3>::pos_x())
  {
    const auto relativeDelta = delta / oldSize.x();
    shearMat = shear_matrix(relativeDelta.y(), relativeDelta.z(), 0., 0., 0., 0.);
  }
  else if (sideToShear == vec<T, 3>::neg_x())
  {
    const auto relativeDelta = delta / oldSize.x();
    shearMat = shear_matrix(-relativeDelta.y(), -relativeDelta.z(), 0., 0., 0., 0.);
  }
  else if (sideToShear == vec<T, 3>::pos_y())
  {
    const auto relativeDelta = delta / oldSize.y();
    shearMat = shear_matrix(0., 0., relativeDelta.x(), relativeDelta.z(), 0., 0.);
  }
  else if (sideToShear == vec<T, 3>::neg_y())
  {
    const auto relativeDelta = delta / oldSize.y();
    shearMat = shear_matrix(0., 0., -relativeDelta.x(), -relativeDelta.z(), 0., 0.);
  }
  else if (sideToShear == vec<T, 3>::pos_z())
  {
    const auto relativeDelta = delta / oldSize.z();
    shearMat = shear_matrix(0., 0., 0., 0., relativeDelta.x(), relativeDelta.y());
  }
  else if (sideToShear == vec<T, 3>::neg_z())
  {
    const auto relativeDelta = delta / oldSize.z();
    shearMat = shear_matrix(0., 0., 0., 0., -relativeDelta.x(), -relativeDelta.y());
  }

  // grab any vertex on side that is opposite the one being sheared.
  const auto sideOppositeToShearSide = -sideToShear;
  vec<T, 3> vertOnOppositeSide;
  [[maybe_unused]] bool didGrab = false;
  auto visitor = [&](
                   const vec<T, 3>& p0,
                   const vec<T, 3>&,
                   const vec<T, 3>&,
                   const vec<T, 3>&,
                   const vec<T, 3>& n) {
    if (n == sideOppositeToShearSide)
    {
      vertOnOppositeSide = p0;
      didGrab = true;
    }
  };
  box.for_each_face(visitor);
  assert(didGrab);

  return translation_matrix(vertOnOppositeSide) * shearMat
         * translation_matrix(-vertOnOppositeSide);
}

/**
 * Finds a 4x4 affine transform that will transform the first 4 points into the following
 * 4 points.
 *
 * The offPlaneIn/offPlaneOut parameters should not be on the plane specified by the other
 * 3 input/output points.
 *
 * @tparam T vector element type
 * @param onPlane0In input point 0
 * @param onPlane1In input point 1
 * @param onPlane2In input point 2
 * @param offPlaneIn input point 3, should be off the plane specified by input points 0,
 * 1, 2
 * @param onPlane0Out what input point 0 should be mapped to
 * @param onPlane1Out what input point 1 should be mapped to
 * @param onPlane2Out what input point 2 should be mapped to
 * @param offPlaneOut what input point 3 should be mapped to, should be off the plane
 * specified by output points 0, 1, 2
 * @return a 4x4 matrix that performs the requested mapping of points or nullopt if the
 * mapping is impossible
 */
template <typename T>
constexpr std::optional<mat<T, 4, 4>> points_transformation_matrix(
  const vec<T, 3>& onPlane0In,
  const vec<T, 3>& onPlane1In,
  const vec<T, 3>& onPlane2In,
  const vec<T, 3>& offPlaneIn,
  const vec<T, 3>& onPlane0Out,
  const vec<T, 3>& onPlane1Out,
  const vec<T, 3>& onPlane2Out,
  const vec<T, 3>& offPlaneOut)
{

  // To simplify the matrix problem, translate the 4 input points so onPlane0In is at
  // origin and the 4 output points so onPlane0Out is at the origin. Then, compensate for
  // the translation at the end.
  const auto vec0In = onPlane1In - onPlane0In;
  const auto vec1In = onPlane2In - onPlane0In;
  const auto vec2In = offPlaneIn - onPlane0In;

  const auto vec0Out = onPlane1Out - onPlane0Out;
  const auto vec1Out = onPlane2Out - onPlane0Out;
  const auto vec2Out = offPlaneOut - onPlane0Out;

  // Set up a system of equations that will find the upper-left 3x3 part of a 4x4 affine
  // matrix with no translation.

  // A*X=B

  const vec<T, 9> B{
    vec0Out.x(),
    vec0Out.y(),
    vec0Out.z(),
    vec1Out.x(),
    vec1Out.y(),
    vec1Out.z(),
    vec2Out.x(),
    vec2Out.y(),
    vec2Out.z(),
  };

  const mat<T, 9, 9> A{
    vec0In.x(), vec0In.y(), vec0In.z(), 0.0,        0.0,        0.0,        0.0,
    0.0,        0.0,        0.0,        0.0,        0.0,        vec0In.x(), vec0In.y(),
    vec0In.z(), 0.0,        0.0,        0.0,        0.0,        0.0,        0.0,
    0.0,        0.0,        0.0,        vec0In.x(), vec0In.y(), vec0In.z(), vec1In.x(),
    vec1In.y(), vec1In.z(), 0.0,        0.0,        0.0,        0.0,        0.0,
    0.0,        0.0,        0.0,        0.0,        vec1In.x(), vec1In.y(), vec1In.z(),
    0.0,        0.0,        0.0,        0.0,        0.0,        0.0,        0.0,
    0.0,        0.0,        vec1In.x(), vec1In.y(), vec1In.z(), vec2In.x(), vec2In.y(),
    vec2In.z(), 0.0,        0.0,        0.0,        0.0,        0.0,        0.0,
    0.0,        0.0,        0.0,        vec2In.x(), vec2In.y(), vec2In.z(), 0.0,
    0.0,        0.0,        0.0,        0.0,        0.0,        0.0,        0.0,
    0.0,        vec2In.x(), vec2In.y(), vec2In.z()};

  const auto [success, X] = lup_solve(A, B);
  if (!success)
  {
    return std::nullopt;
  }

  const mat<T, 4, 4> xformWithoutTranslation(
    X[0],
    X[1],
    X[2],
    0.0,
    X[3],
    X[4],
    X[5],
    0.0,
    X[6],
    X[7],
    X[8],
    0.0,
    0.0,
    0.0,
    0.0,
    1.0);

  return translation_matrix(onPlane0Out) * xformWithoutTranslation
         * translation_matrix(-onPlane0In);
}

/**
 * Finds a 4x4 affine transform that will transform the first 3 points into the following
 * 3 points.
 *
 * Note, this leaves unspecified the scaling of the axis perpendicular the planes
 * specified by the 3 input and output points. What it does is uniform scaling along that
 * axis, so a point 1 unit off the input plane will be 1 unit off the output plane.
 *
 * @tparam T vector element type
 * @param onPlane0In input point 0
 * @param onPlane1In input point 1
 * @param onPlane2In input point 2
 * @param onPlane0Out what input point 0 should be mapped to
 * @param onPlane1Out what input point 1 should be mapped to
 * @param onPlane2Out what input point 2 should be mapped to
 * @return a 4x4 matrix that performs the requested mapping of points or nullopt if the
 * mapping is impossible
 */
template <typename T>
constexpr std::optional<mat<T, 4, 4>> points_transformation_matrix(
  const vec<T, 3>& onPlane0In,
  const vec<T, 3>& onPlane1In,
  const vec<T, 3>& onPlane2In,
  const vec<T, 3>& onPlane0Out,
  const vec<T, 3>& onPlane1Out,
  const vec<T, 3>& onPlane2Out)
{

  const auto offPlaneIn =
    onPlane0In + normalize(cross(onPlane1In - onPlane0In, onPlane2In - onPlane0In));
  const auto offPlaneOut =
    onPlane0Out + normalize(cross(onPlane1Out - onPlane0Out, onPlane2Out - onPlane0Out));

  return points_transformation_matrix(
    onPlane0In,
    onPlane1In,
    onPlane2In,
    offPlaneIn,
    onPlane0Out,
    onPlane1Out,
    onPlane2Out,
    offPlaneOut);
}
} // namespace vm
