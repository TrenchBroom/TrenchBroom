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

#include "constants.h"
#include "mat.h"
#include "scalar.h"
#include "util.h"
#include "vec.h"

#include <tuple>
#include <type_traits>

namespace vm {
/**
 * A plane, represented as a normal vector and a distance of the plane from the origin.
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, std::size_t S> class plane {
public:
  T distance;
  vec<T, S> normal;

  /**
   * Creates a new plane by setting all components to 0.
   */
  constexpr plane()
    : distance{}
    , normal{} {}

  // Copy and move constructors
  plane(const plane<T, S>& other) = default;
  plane(plane<T, S>&& other) noexcept = default;

  // Assignment operators
  plane<T, S>& operator=(const plane<T, S>& other) = default;
  plane<T, S>& operator=(plane<T, S>&& other) noexcept = default;

  /**
   * Converts the given plane by converting its components.
   *
   * @tparam U the component type of the given plane
   * @param other the plane to convert
   */
  template <typename U>
  constexpr explicit plane(const plane<U, S>& other)
    : distance(static_cast<T>(other.distance))
    , normal(other.normal) {}

  /**
   * Creates a new plane with the given distance and normal vector
   *
   * @param i_distance the distance
   * @param i_normal the normal vector
   */
  constexpr plane(const T i_distance, const vec<T, S>& i_normal)
    : distance(i_distance)
    , normal(i_normal) {}

  /**
   * Creates a new plane with the given anchor point and normal.
   *
   * @param i_anchor the anchor point (any point on the plane)
   * @param i_normal the normal vector
   */
  constexpr plane(const vec<T, S>& i_anchor, const vec<T, S>& i_normal)
    : distance(dot(i_anchor, i_normal))
    , normal(i_normal) {}

  /**
   * Returns a point on this plane with minimal distance to the coordinate system origin. Assumes
   * that the normal has unit length.
   *
   * @return the anchor point
   */
  constexpr vec<T, S> anchor() const { return normal * distance; }

  /**
   * Given a line, this function computes the intersection of the line and this plane. The line is
   * restricted to be parallel to a coordinate system axis (indicated by the axis parameter), and
   * the point has a dimension of one less than this plane (since the position of the point along
   * the given axis does not matter).
   *
   * This function then computes the point of intersection, and returns the missing component of the
   * given point, i.e. the value of the component that, if inserted into the components of the
   * point, would yield the intersection point.
   *
   * @param point the point
   * @param axis the axis
   * @return the missing component to transform the given point to the point of intersection
   */
  constexpr T at(const vec<T, S - 1>& point, const axis::type axis) const {
    if (normal[axis] == static_cast<T>(0.0)) {
      return static_cast<T>(0.0);
    }

    auto t = static_cast<T>(0.0);
    std::size_t index = 0u;
    for (std::size_t i = 0u; i < S; i++) {
      if (i != axis) {
        t += normal[i] * point[index++];
      }
    }
    return (distance - t) / normal[axis];
  }

  constexpr T xAt(const vec<T, S - 1>& point) const { return at(point, axis::x); }

  constexpr T yAt(const vec<T, S - 1>& point) const { return at(point, axis::y); }

  constexpr T zAt(const vec<T, S - 1>& point) const { return at(point, axis::z); }

  /**
   * Computes the distance of the given point to this plane. The sign of the distance indicates
   * whether the point is above or below this plane.
   *
   * @param point the point
   * @return the distance of the given point to this plane
   */
  constexpr T point_distance(const vec<T, S>& point) const { return dot(point, normal) - distance; }

  /**
   * Determines the relative position of the given point to this plane. A plane can either be above
   * (in direction of the normal), below (opposite direction), or inside this plane.
   *
   * @param point the point to check
   * @param epsilon an epsilon value (the maximum absolute distance up to which a point will be
   * considered to be inside)
   * @return a value indicating the point status
   */
  constexpr plane_status point_status(
    const vec<T, S>& point, const T epsilon = constants<T>::point_status_epsilon()) const {
    const auto dist = point_distance(point);
    if (dist > epsilon) {
      return plane_status::above;
    } else if (dist < -epsilon) {
      return plane_status::below;
    } else {
      return plane_status::inside;
    }
  }

  /**
   * Flips this plane by negating its normal.
   *
   * @return the flipped plane
   */
  constexpr plane<T, S> flip() const {
    // Distance must also be flipped to compensate for the changed sign of the normal. The location
    // of the plane does not change!
    return plane<T, S>(-distance, -normal);
  }

  /**
   * Transforms this plane using the given transformation matrix. The translational part is not
   * applied to the normal.
   *
   * @param transform the transformation to apply
   * @return the transformed plane
   */
  plane<T, S> transform(const mat<T, S + 1, S + 1>& transform) const {
    return plane<T, S>(transform * anchor(), normalize(strip_translation(transform) * normal));
  }

  /**
   * Transforms this plane using the given transformation matrix at compile time. The translational
   * part is not applied to the normal.
   *
   * @param transform the transformation to apply
   * @return the transformed plane
   */
  constexpr plane<T, S> transform_c(const mat<T, S + 1, S + 1>& transform) const {
    return plane<T, S>(transform * anchor(), normalize_c(strip_translation(transform) * normal));
  }

  /**
   * Projects the given point onto this plane along the plane normal.
   *
   * @param point the point to project
   * @return the projected point
   */
  constexpr vec<T, S> project_point(const vec<T, S>& point) const {
    return point - dot(point, normal) * normal + distance * normal;
  }

  /**
   * Projects the given point onto this plane along the given direction.
   *
   * @param point the point to project
   * @param direction the projection direction
   * @return the projected point
   */
  constexpr vec<T, S> project_point(const vec<T, S>& point, const vec<T, S>& direction) const {
    const auto cos = dot(direction, normal);
    if (is_zero(cos, constants<T>::almost_zero())) {
      return vec<T, S>::nan();
    }
    const auto d = dot(distance * normal - point, normal) / cos;
    return point + direction * d;
  }

  /**
   * Projects the given vector originating at the anchor point onto this plane along the plane
   * normal.
   *
   * @param vector the vector to project
   * @return the projected vector
   */
  constexpr vec<T, S> project_vector(const vec<T, S>& vector) const {
    return project_point(anchor() + vector) - anchor();
  }

  /**
   * Projects the given vector originating at the anchor point onto this plane along the given
   * direction.
   *
   * @param vector the vector to project
   * @param direction the projection direction
   * @return the projected vector
   */
  constexpr vec<T, S> project_vector(const vec<T, S>& vector, const vec<T, S>& direction) const {
    return project_point(anchor() + vector, direction) - anchor();
  }
};

/**
 * Checks whether the given planes are equal using the given epsilon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first plane
 * @param rhs the second plane
 * @param epsilon an epsilon value
 * @return bool if the two planes are considered equal and false otherwise
 */
template <typename T, std::size_t S>
constexpr bool is_equal(const plane<T, S>& lhs, const plane<T, S>& rhs, const T epsilon) {
  return is_equal(lhs.distance, rhs.distance, epsilon) && is_equal(lhs.normal, rhs.normal, epsilon);
}

/**
 * Checks if the given planes are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first plane
 * @param rhs the second plane
 * @return true if the given planes are identical and false otherwise
 */
template <typename T, std::size_t S>
constexpr bool operator==(const plane<T, S>& lhs, const plane<T, S>& rhs) {
  return lhs.distance == rhs.distance && lhs.normal == rhs.normal;
}

/**
 * Checks if the given planes are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first plane
 * @param rhs the second plane
 * @return false if the given planes are identical and true otherwise
 */
template <typename T, std::size_t S>
constexpr bool operator!=(const plane<T, S>& lhs, const plane<T, S>& rhs) {
  return lhs.distance != rhs.distance || lhs.normal != rhs.normal;
}

/**
 * Computes the normal of a plane in three point form.
 *
 * Thereby, the orientation of the plane is derived
 * as follows. Given the following diagram of the given points:
 *
 * p2
 * |
 * |
 * |
 * |
 * p1---------p3
 *
 * The normal of the plane will be the normalized cross product of vectors (p3-p1) and (p2-p1).
 *
 * Note that the three points must not be colinear so as to be a valid three point representation of
 * a plane. *
 *
 * @tparam T the component type
 * @param p1 the first plane point
 * @param p2 the second plane point
 * @param p3 the third plane point
 * @param epsilon an epsilon value used to determine whether the given three points do not define a
 * plane
 * @return a pair of a boolean indicating whether the plane is valid, and the normal
 */
template <typename T>
std::tuple<bool, vec<T, 3>> plane_normal(
  const vec<T, 3>& p1, const vec<T, 3>& p2, const vec<T, 3>& p3,
  const T epsilon = constants<T>::angle_epsilon()) {
  const auto v1 = p3 - p1;
  const auto v2 = p2 - p1;
  const auto normal = cross(v1, v2);

  // Fail if v1 and v2 are parallel, opposite, or either is zero-length.
  // Rearranging "A cross B = ||A|| * ||B|| * sin(theta) * n" (n is a unit vector perpendicular to A
  // and B) gives sin_theta below.
  const auto sin_theta = abs(length(normal) / (length(v1) * length(v2)));
  if (is_nan(sin_theta) || is_inf(sin_theta) || sin_theta < epsilon) {
    return std::make_tuple(false, vec<T, 3>::zero());
  } else {
    return std::make_tuple(true, normalize(normal));
  }
}

/**
 * Creates a new plane from the given plane in three point form. Thereby, the orientation of the
 * plane is derived as follows. Given the following diagram of the given points:
 *
 * p2
 * |
 * |
 * |
 * |
 * p1---------p3
 *
 * The normal of the plane will be the normalized cross product of vectors (p3-p1) and (p2-p1).
 *
 * Note that the three points must not be colinear so as to be a valid three point representation of
 * a plane.
 *
 * @param p1 the first point
 * @param p2 the second point
 * @param p3 the third point
 * @return a pair of a boolean indicating whether the plane is valid, and the plane itself
 */
template <typename T>
std::tuple<bool, plane<T, 3>> from_points(
  const vec<T, 3>& p1, const vec<T, 3>& p2, const vec<T, 3>& p3) {
  const auto [valid, normal] = plane_normal(p1, p2, p3);
  if (!valid) {
    return std::make_tuple(false, plane<T, 3>());
  } else {
    return std::make_tuple(true, plane<T, 3>(p1, normal));
  }
}

/**
 * Creates a new plane from a plane in three point form using the first three points in the given
 * range. Thereby, the orientation of the plane is derived as follows. Given the following diagram
 * of the given points:
 *
 * p2
 * |
 * |
 * |
 * |
 * p1---------p3
 *
 * The normal of the plane will be the normalized cross product of vectors (p3-p1) and (p2-p1).
 *
 * Note that the three points must not be colinear so as to be a valid three point representation of
 * a plane. Also note that if the given range does not contain at least three points, this function
 * returns a negative result.
 *
 * @tparam I the range iterator type
 * @tparam G a function that maps a range element to a point
 * @param cur the start of the range
 * @param end the end of the range
 * @param get the mapping function
 * @return a pair of a boolean indicating whether the plane is valid, and the plane itself
 */
template <typename I, typename G = identity>
auto from_points(I cur, I end, const G& get = G())
  -> std::tuple<bool, plane<typename std::remove_reference<decltype(get(*cur))>::type::type, 3>> {
  using T = typename std::remove_reference<decltype(get(*cur))>::type::type;

  if (cur == end) {
    return std::make_tuple(false, plane<T, 3>());
  }
  const auto p1 = *cur;
  ++cur;
  if (cur == end) {
    return std::make_tuple(false, plane<T, 3>());
  }
  const auto p2 = *cur;
  ++cur;
  if (cur == end) {
    return std::make_tuple(false, plane<T, 3>());
  }
  const auto p3 = *cur;

  return from_points(p1, p2, p3);
}

/**
 * Creates a plane with the given point as its anchor and the positive Z axis as its normal.
 *
 * @tparam T the component type
 * @param position the position of the plane
 * @return the plane
 */
template <typename T> constexpr plane<T, 3> horizontal_plane(const vec<T, 3>& position) {
  return plane<T, 3>(position, vec<T, 3>::pos_z());
}

/**
 * Creates a plane at the given position and with its normal set to the normalized direction.
 *
 * @tparam T the component type
 * @param position the position of the plane
 * @param direction the direction to derive the normal from
 * @return the plane
 */
template <typename T>
plane<T, 3> orthogonal_plane(const vec<T, 3>& position, const vec<T, 3>& direction) {
  return plane<T, 3>(position, normalize(direction));
}

/**
 * Creates a plane at the given position and with its normal set to the major axis of the given
 * direction.
 *
 * @tparam T the component type
 * @param position the position of the plane
 * @param direction the direction to derive the normal from
 * @return the plane
 */
template <typename T>
constexpr plane<T, 3> aligned_orthogonal_plane(
  const vec<T, 3>& position, const vec<T, 3>& direction) {
  return plane<T, 3>(position, get_abs_max_component_axis(direction));
}
} // namespace vm
