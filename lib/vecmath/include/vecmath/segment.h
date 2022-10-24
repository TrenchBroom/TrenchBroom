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
#include "vec.h"

namespace vm
{
/**
 * A line segment, represented by its two end points.
 *
 * This class enforces the following invariant: the start point of the segment is always
 * less than or equal to the end point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, size_t S>
class segment
{
public:
  using component_type = T;
  using float_type = segment<float, S>;
  static constexpr std::size_t size = S;

private:
  vec<T, S> m_start;
  vec<T, S> m_end;

public:
  /**
   * Creates a new empty segment of length 0 with both the start and the end set to 0.
   */
  constexpr segment() = default;

  // Copy and move constructors
  segment(const segment<T, S>& other) = default;
  segment(segment<T, S>&& other) noexcept = default;

  // Assignment operators
  segment<T, S>& operator=(const segment<T, S>& other) = default;
  segment<T, S>& operator=(segment<T, S>&& other) noexcept = default;

  /**
   * Creates a new segment with the given points.
   *
   * @param p1 one end point
   * @param p2 the opposite end point
   */
  constexpr segment(const vec<T, S>& p1, const vec<T, S>& p2)
    : m_start(p1 < p2 ? p1 : p2)
    , m_end(p1 < p2 ? p2 : p1)
  {
  }

  /**
   * Creates a new segment by copying the values from the given segment. If the given
   * segment has a different component type, the values are converted using static_cast.
   *
   * @tparam U the component type of the given segment
   * @param other the segment to copy the values from
   */
  template <typename U>
  explicit constexpr segment(const segment<U, S>& other)
    : m_start(other.start())
    , m_end(other.end())
  {
  }

  /**
   * Returns the origin of this segment.
   */
  constexpr vec<T, S> get_origin() const { return start(); }

  /**
   * Returns the direction of this segment.
   */
  constexpr vec<T, S> get_direction() const { return direction(); }

  /**
   * Returns the length of this segment.
   *
   * @return the length of this segment
   */
  T length() const { return vm::length(m_end - m_start); }

  /**
   * Returns the length of this segment at compile time.
   *
   * @return the length of this segment
   */
  constexpr T length_c() const { return vm::length_c(m_end - m_start); }

  /**
   * Returns the squared length of this segment.
   *
   * @return the squared length of this segment
   */
  constexpr T squared_length() const { return vm::squared_length(m_end - m_start); }

  /**
   * Checks whether the given point is contained in this segment.
   *
   * @param point the point to check
   * @param maxDistance the maximum distance from the point and this segment
   * @return true if the given point is contained in this segment and false otherwise
   */
  bool contains(const vec<T, S>& point, const T maxDistance) const
  {
    const auto f = distance_to_projected_point(*this, point);
    if (f < -maxDistance || f * f > squared_length() + maxDistance * maxDistance)
    {
      return false;
    }
    else
    {
      const auto proj = point_at_distance(*this, f);
      return squared_distance(proj, point) <= (maxDistance * maxDistance);
    }
  }

  /**
   * Transforms this segment using the given transformation matrix.
   *
   * @param transform the transformation to apply
   * @return the transformed segment
   */
  constexpr segment<T, S> transform(const mat<T, S + 1, S + 1>& transform) const
  {
    return segment<T, S>(transform * m_start, transform * m_end);
  }

  /**
   * Translates this segment by the given offset.
   *
   * @param delta the offset by which to translate the segment
   * @return the translated segment
   */
  constexpr segment<T, S> translate(const vec<T, S>& delta) const
  {
    return segment<T, S>(m_start + delta, m_end + delta);
  }

  /**
   * Returns the start point of this segment.
   *
   * @return the start point
   */
  constexpr const vec<T, S>& start() const { return m_start; }

  /**
   * Returns the end point of this segment.
   *
   * @return the end point
   */
  constexpr const vec<T, S>& end() const { return m_end; }

  /**
   * Returns the center point of this segment.
   *
   * @return the center point
   */
  constexpr vec<T, S> center() const { return (m_start + m_end) / static_cast<T>(2.0); }

  /**
   * Returns the normalized direction vector of this segment, i.e., a unit vector which
   * points at the end point, assuming the start point is the origin of the vector.
   *
   * @return the direction vector
   */
  vec<T, S> direction() const { return normalize(m_end - m_start); }

  // FIXME: this is only here because TB's VertexToolBase needs it, it should be moved
  // elsewhere
  /**
   * Adds the start and end points of the given range of segments to the given output
   * iterator.
   *
   * @tparam I the range iterator type
   * @tparam O the output iterator type
   * @param cur the range start
   * @param end the range end
   * @param out the output iterator
   */
  template <typename I, typename O>
  static void get_vertices(I cur, I end, O out)
  {
    while (cur != end)
    {
      const auto& segment = *cur;
      out = segment.start();
      ++out;
      out = segment.end();
      ++out;
      ++cur;
    }
  }
};

/**
 * Compares the given segments using the given epsilon value. Thereby, the start points of
 * the segments are compared first, and if the comparison yields a value other than 0,
 * that value is returned. Otherwise, the result of comparing the end points is returned.
 *
 * Note that by the invariant of the segment class, the start point is always less than or
 * equal to the end point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @param epsilon an epsilon value
 * @return -1 if the first segment is less than the second segment, +1 in the opposite
 * case, and 0 if the segments are equal
 */
template <typename T, size_t S>
constexpr int compare(
  const segment<T, S>& lhs,
  const segment<T, S>& rhs,
  const T epsilon = static_cast<T>(0.0))
{
  const int startCmp = compare(lhs.start(), rhs.start(), epsilon);
  if (startCmp < 0)
  {
    return -1;
  }
  else if (startCmp > 0)
  {
    return 1;
  }
  else
  {
    return compare(lhs.end(), rhs.end(), epsilon);
  }
}

/**
 * Checks whether the given segments have equal components.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @param epsilon the epsilon value
 * @return true if all components of the given segments are equal, and false otherwise
 */
template <typename T, size_t S>
constexpr bool is_equal(
  const segment<T, S>& lhs, const segment<T, S>& rhs, const T epsilon)
{
  return compare(lhs, rhs, epsilon) == 0;
}

/**
 * Checks if the first given segment identical to the second segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @return true if the segments are identical and false otherwise
 */
template <typename T, size_t S>
constexpr bool operator==(const segment<T, S>& lhs, const segment<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) == 0;
}

/**
 * Checks if the first given segment identical to the second segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @return false if the segments are identical and true otherwise
 */
template <typename T, size_t S>
constexpr bool operator!=(const segment<T, S>& lhs, const segment<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) != 0;
}

/**
 * Checks if the first given segment less than the second segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @return true if the first segment is less than the second and false otherwise
 */
template <typename T, size_t S>
constexpr bool operator<(const segment<T, S>& lhs, const segment<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) < 0;
}

/**
 * Checks if the first given segment less than or equal to the second segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @return true if the first segment is less than or equal to the second and false
 * otherwise
 */
template <typename T, size_t S>
constexpr bool operator<=(const segment<T, S>& lhs, const segment<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) <= 0;
}

/**
 * Checks if the first given segment greater than the second segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @return true if the first segment is greater than the second and false otherwise
 */
template <typename T, size_t S>
constexpr bool operator>(const segment<T, S>& lhs, const segment<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) > 0;
}

/**
 * Checks if the first given segment greater than or equal to the second segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first segment
 * @param rhs the second segment
 * @return true if the first segment is greater than or equal to the second and false
 * otherwise
 */
template <typename T, size_t S>
constexpr bool operator>=(const segment<T, S>& lhs, const segment<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) >= 0;
}

/**
 * Translates the given segment by the given offset.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param s the segment to translate
 * @param offset the offset
 * @return the translated segment
 */
template <typename T, size_t S>
constexpr segment<T, S> translate(const segment<T, S>& s, const vec<T, S>& offset)
{
  return s.translate(offset);
}
} // namespace vm
