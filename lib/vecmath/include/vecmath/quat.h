/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "scalar.h"
#include "vec.h"

#include <cassert>

namespace vm {
template <typename T> class quat {
public:
  /**
   * The real component.
   */
  T r;

  /**
   * The imaginary component.
   */
  vec<T, 3> v;

  /**
   * Creates a new quaternion initialized to 0.
   */
  constexpr quat()
    : r(T(0))
    , v(vec<T, 3>::zero()) {}

  // Copy and move constructors
  quat(const quat<T>& other) = default;
  quat(quat<T>&& other) noexcept = default;

  // Assignment operators
  quat<T>& operator=(const quat<T>& other) = default;
  quat<T>& operator=(quat<T>&& other) noexcept = default;

  /**
   * Converts the given quaternion by converting its members to this quaternion's component type.
   *
   * @tparam U the component type of the quaternion to convert
   * @param other the quaternion to convert
   */
  template <typename U>
  explicit constexpr quat(const quat<U>& other)
    : r(T(other.r))
    , v(other.v) {}

  /**
   * Creates a new quaternion with the given real and imaginary components.
   *
   * @param i_r the real component
   * @param i_v the imaginary components
   */
  constexpr quat(const T i_r, const vec<T, 3>& i_v)
    : r(i_r)
    , v(i_v) {}

  /**
   * Creates a new quaternion that represent a clounter-clockwise rotation by the given angle (in
   * radians) about the given axis.
   *
   * @param axis the rotation axis
   * @param angle the rotation angle (in radians)
   */
  quat(const vec<T, 3>& axis, const T angle) { set_rotation(axis, angle); }

  /**
   * Creates a new quaternion that rotates the 1st given vector onto the 2nd given vector. Both
   * vectors are expected to be normalized.
   *
   * @param from the vector to rotate
   * @param to the vector to rotate onto
   */
  quat(const vec<T, 3>& from, const vec<T, 3>& to) {
    assert(is_unit(from, constants<T>::almost_zero()));
    assert(is_unit(to, constants<T>::almost_zero()));

    const auto cos = dot(from, to);
    if (is_equal(+cos, T(1.0), constants<T>::almost_zero())) {
      // `from` and `to` are equal.
      set_rotation(vec<T, 3>::pos_z(), T(0.0));
    } else if (is_equal(-cos, T(1.0), constants<T>::almost_zero())) {
      // `from` and `to` are opposite.
      // We need to find a rotation axis that is perpendicular to `from`.
      auto axis = cross(from, vec<T, 3>::pos_z());
      if (is_zero(squared_length(axis), constants<T>::almost_zero())) {
        axis = cross(from, vec<T, 3>::pos_x());
      }
      set_rotation(normalize(axis), to_radians(T(180)));
    } else {
      const auto axis = normalize(cross(from, to));
      const auto angle = std::acos(cos);
      set_rotation(axis, angle);
    }
  }

private:
  void set_rotation(const vec<T, 3>& axis, const T angle) {
    assert(is_unit(axis, constants<T>::almost_zero()));
    r = std::cos(angle / T(2.0));
    v = axis * std::sin(angle / T(2.0));
  }

public:
  /**
   * Returns the angle by which this quaternion would rotate a vector.
   *
   * @return the rotation angle in radians
   */
  T angle() const { return T(2.0) * std::acos(r); }

  /**
   * Returns the rotation axis of this quaternion.
   *
   * @return the rotation axis
   */
  vec<T, 3> axis() const {
    if (is_zero(v, constants<T>::almost_zero())) {
      return v;
    } else {
      return v / std::sin(std::acos(r));
    }
  }

  /**
   * Conjugates this quaternion by negating its imaginary components.
   *
   * @return the conjugated quaternion
   */
  constexpr quat<T> conjugate() const { return quat<T>(r, -v); }
};

/**
 * Checks whether the given quaternions are equal up to the given epsilon. Two quaternions p, q are
 * considered to be equal when they represent the same rotation, that is, if
 *
 * - p is component wise equal to q or
 * - p is component wise equal to -q
 *
 * up to the given epsilon value.
 *
 * @tparam T the component type
 * @param lhs the first quaternion
 * @param rhs the second quaternion
 * @param epsilon the epsilon value
 * @return true if the given quaternions represent the same rotation and false otherwise
 */
template <typename T>
constexpr bool is_equal(const quat<T>& lhs, const quat<T>& rhs, const T epsilon) {
  return (is_equal(lhs.r, rhs.r, epsilon) || is_equal(lhs.r, -rhs.r, epsilon)) &&
         is_equal(lhs.v, rhs.v, epsilon);
}

/**
 * Checks whether the given quaternions are identical. Two quaternions p, q are considered to be
 * identical if they represent the same rotation, this is, if
 *
 * - p is component wise identical to q or
 * - p is component wise identical to -q.
 *
 * This check is equivalent to is_equal(lhs, rhs, 0).
 *
 * @tparam T the component type
 * @param lhs the first quaternion
 * @param rhs the second quaternion
 * @return true if the given quaternions represent the same rotation and false otherwise
 */
template <typename T> constexpr bool operator==(const quat<T>& lhs, const quat<T>& rhs) {
  return (lhs.r == rhs.r || lhs.r == -rhs.r) && lhs.v == rhs.v;
}

/**
 * Checks whether the given quaternions are identical.
 *
 * This check is equivalent to !is_equal(lhs, rhs, 0).
 *
 * @tparam T the component type
 * @param lhs the first quaternion
 * @param rhs the second quaternion
 * @return false if the given quaternions represent the same rotation and true otherwise
 */
template <typename T> constexpr bool operator!=(const quat<T>& lhs, const quat<T>& rhs) {
  return (lhs.r != rhs.r && lhs.r != -rhs.r) || lhs.v != rhs.v;
}

/**
 * Returns the given quaternion.
 *
 * @tparam T the component type
 * @param q the quaternion to return
 * @return the given quaternion
 */
template <typename T> constexpr quat<T> operator+(const quat<T>& q) {
  return q;
}

/**
 * Negates the given quaternion by negating its real component.
 *
 * @tparam T the component type
 * @param q the quaternion to negate
 * @return the negated quaternion
 */
template <typename T> constexpr quat<T> operator-(const quat<T>& q) {
  return quat<T>(-q.r, q.v);
}

/**
 * Multiplies the given quaternion with the given scalar value.
 *
 * @tparam T the component type
 * @param lhs the quaternion
 * @param rhs the scalar
 * @return the multiplied quaternion
 */
template <typename T> constexpr quat<T> operator*(const quat<T> lhs, const T rhs) {
  return quat<T>(lhs.r * rhs, lhs.v);
}

/**
 * Multiplies the given quaternion with the given scalar value.
 *
 * @tparam T the component type
 * @param lhs the scalar
 * @param rhs the quaternion
 * @return the multiplied quaternion
 */
template <typename T> constexpr quat<T> operator*(const T lhs, const quat<T>& rhs) {
  return quat<T>(lhs * rhs.r, rhs.v);
}

/**
 * Multiplies the given quaternions.
 *
 * @tparam T the component type
 * @param lhs the first quaternion
 * @param rhs the second quaternion
 * @return the product of the given quaternions.
 */
template <typename T> constexpr quat<T> operator*(const quat<T>& lhs, const quat<T>& rhs) {
  const auto nr = lhs.r * rhs.r - dot(lhs.v, rhs.v);
  const auto nx =
    lhs.r * rhs.v.x() + lhs.v.x() * rhs.r + lhs.v.y() * rhs.v.z() - lhs.v.z() * rhs.v.y();
  const auto ny =
    lhs.r * rhs.v.y() + lhs.v.y() * rhs.r + lhs.v.z() * rhs.v.x() - lhs.v.x() * rhs.v.z();
  const auto nz =
    lhs.r * rhs.v.z() + lhs.v.z() * rhs.r + lhs.v.x() * rhs.v.y() - lhs.v.y() * rhs.v.x();

  return quat<T>(nr, vec<T, 3>(nx, ny, nz));
}

/**
 * Applies the given quaternion to the given vector, in effect rotating it.
 *
 * @tparam T the component type
 * @param lhs the quaternion
 * @param rhs the vector
 * @return the rotated vector
 */
template <typename T> constexpr vec<T, 3> operator*(const quat<T>& lhs, const vec<T, 3>& rhs) {
  return (lhs * quat<T>(T(0.0), rhs) * lhs.conjugate()).v;
}
} // namespace vm
