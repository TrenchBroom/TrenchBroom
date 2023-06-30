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

#include "abstract_line.h"
#include "mat.h"
#include "scalar.h"
#include "util.h"
#include "vec.h"

namespace vm
{
/**
 * A ray, represented by the origin and direction.
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, size_t S>
class ray
{
public:
  using component_type = T;
  static constexpr std::size_t size = S;

public:
  vec<T, S> origin;
  vec<T, S> direction;

  /**
   * Creates a new ray with all components initialized to 0.
   */
  constexpr ray()
    : origin(vec<T, S>::zero())
    , direction(vec<T, S>::zero())
  {
  }

  // Copy and move constructors
  ray(const ray<T, S>& other) = default;
  ray(ray<T, S>&& other) noexcept = default;

  // Assignment operators
  ray<T, S>& operator=(const ray<T, S>& other) = default;
  ray<T, S>& operator=(ray<T, S>&& other) noexcept = default;

  /**
   * Creates a new ray by copying the values from the given ray. If the given ray has a
   * different component type, the values are converted using static_cast.
   *
   * @tparam U the component type of the given ray
   * @param other the ray to copy the values from
   */
  template <typename U>
  explicit constexpr ray(const ray<U, S>& other)
    : origin(vec<T, S>(other.origin))
    , direction(vec<T, S>(other.direction))
  {
  }

  /**
   * Creates a new ray with the given origin and direction.
   *
   * @param i_origin the origin
   * @param i_direction the direction
   */
  constexpr ray(const vec<T, S>& i_origin, const vec<T, S>& i_direction)
    : origin(i_origin)
    , direction(i_direction)
  {
  }

  /**
   * Returns the origin of this ray.
   */
  constexpr vec<T, S> get_origin() const { return origin; }

  /**
   * Returns the direction of this ray.
   */
  constexpr vec<T, S> get_direction() const { return direction; }

  /**
   * Transforms this line using the given transformation matrix. The translational part is
   * not applied to the direction, and the direction is normalized after the
   * transformation has been applied.
   *
   * @param transform the transformation to apply
   * @return the transformed ray
   */
  ray<T, S> transform(const mat<T, S + 1, S + 1>& transform) const
  {
    const auto newOrigin = transform * origin;
    const auto newDirection = normalize(strip_translation(transform) * direction);
    return ray<T, S>(newOrigin, newDirection);
  }

  /**
   * Transforms this line using the given transformation matrix at compile time. The
   * translational part is not applied to the direction, and the direction is normalized
   * after the transformation has been applied.
   *
   * @param transform the transformation to apply
   * @return the transformed ray
   */
  constexpr ray<T, S> transform_c(const mat<T, S + 1, S + 1>& transform) const
  {
    const auto newOrigin = transform * origin;
    const auto newDirection = normalize_c(strip_translation(transform) * direction);
    return ray<T, S>(newOrigin, newDirection);
  }

  /**
   * Determines the position of the given point in relation to the origin and direction of
   * this ray. Suppose that the ray determines a plane that splits the space into two half
   * spaces. The plane position is determined byhte origin and the plane normal is
   * identical to the ray direction. Then the return value indicates one of three
   * situations:
   *
   * - The given point is in the half space above the plane, or in direction of the ray.
   * - The given point is in the half space below the plane, or in the opposite direction
   * of the ray.
   * - The given point is in neither half space, or it is exactly on the plane.
   *
   * @param point the point to check
   * @return a value indicating the relative position of the given point
   */
  constexpr plane_status point_status(const vec<T, S>& point) const
  {
    const auto scale = dot(direction, point - origin);
    if (scale > constants<T>::point_status_epsilon())
    {
      return plane_status::above;
    }
    else if (scale < -constants<T>::point_status_epsilon())
    {
      return plane_status::below;
    }
    else
    {
      return plane_status::inside;
    }
  }
};

/**
 * Checks whether the given rays have equal components.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first ray
 * @param rhs the second ray
 * @param epsilon the epsilon value
 * @return true if all components of the given rays are equal, and false otherwise
 */
template <typename T, size_t S>
constexpr bool is_equal(const ray<T, S>& lhs, const ray<T, S>& rhs, const T epsilon)
{
  return is_equal(lhs.origin, rhs.origin, epsilon)
         && is_equal(lhs.direction, rhs.direction, epsilon);
}

/**
 * Checks whether the given rays are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first ray
 * @param rhs the second ray
 * @return true if the given rays are identical and false otherwise
 */
template <typename T, size_t S>
constexpr bool operator==(const ray<T, S>& lhs, const ray<T, S>& rhs)
{
  return lhs.origin == rhs.origin && lhs.direction == rhs.direction;
}

/**
 * Checks whether the given rays are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first ray
 * @param rhs the second ray
 * @return false if the given rays are identical and true otherwise
 */
template <typename T, size_t S>
constexpr bool operator!=(const ray<T, S>& lhs, const ray<T, S>& rhs)
{
  return lhs.origin != rhs.origin || lhs.direction != rhs.direction;
}
} // namespace vm
