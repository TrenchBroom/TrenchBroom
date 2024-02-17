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

#include "vecmath/abstract_line.h"
#include "vecmath/constants.h"
#include "vecmath/line.h"
#include "vecmath/polygon.h"
#include "vecmath/ray.h"
#include "vecmath/segment.h"
#include "vecmath/vec.h"

#include <cstddef>

namespace vm
{
/**
 * The distance of a point to an abstract line, which could be an infinite line, a ray, or
 * a line segment.
 */
template <typename T>
struct point_distance
{
  /**
   * The distance from the origin of the line to the orthogonal projection of a point onto
   * the line. For rays or line segments, this is clamped so it's between the ray/line
   * segment start, and end (for line segments). Never squared.
   */
  T position;

  /**
   * The distance between the clamped orthogonal projection of a point to the point
   * itself. Squared if squared_distance was used.
   */
  T distance;

  /**
   * Creates a new instance with the given values.
   *
   * @param i_position the position of the orthogonal projection of the point onto the
   * line
   * @param i_distance the value of the distance, may be squared
   */
  constexpr point_distance(const T i_position, const T i_distance)
    : position(i_position)
    , distance(i_distance)
  {
  }
};

/**
 * Computes the minimal squared distance between a given point and a ray.
 * After clamping the orthogonal projection of the point onto the ray to not
 * lie behind the ray origin, two values are returned:
 *
 * - The position of the clamped projected point relative to the given ray origin
 * (point_distance::position)
 * - The squared distance between the clamped projected point on the given ray and the
 * given point (point_distance::distance)
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param p the point
 * @return the squared distance
 */
template <typename T, size_t S>
constexpr point_distance<T> squared_distance(const ray<T, S>& r, const vec<T, S>& p)
{
  const auto origin_to_point = p - r.origin;
  const auto position = max(dot(origin_to_point, r.direction), T(0.0));
  if (position == T(0.0))
  {
    return point_distance<T>(position, squared_length(origin_to_point));
  }
  else
  {
    return point_distance<T>(
      position, squared_length(point_at_distance(r, position) - p));
  }
}

/**
 * Computes the minimal distance between a given point and a ray.
 * After clamping the orthogonal projection of the point onto the ray to not
 * lie behind the ray origin, two values are returned:
 *
 * - The position of the clamped projected point relative to the given ray origin
 * (point_distance::position)
 * - The squared distance between the clamped projected point on the given ray and the
 * given point (point_distance::distance)
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param p the point
 * @return the distance
 */
template <typename T, size_t S>
point_distance<T> distance(const ray<T, S>& r, const vec<T, S>& p)
{
  auto distance2 = squared_distance(r, p);
  distance2.distance = sqrt(distance2.distance);
  return distance2;
}

/**
 * Computes the minimal squared distance between a given point and a segment. Two values
 * are returned:
 *
 * - The squared distance between the closest point on given segment and the given point.
 * - The distance from the origin of the given segment the closest point on the given
 * segment.
 *
 * Thereby, the closest point on the given segment is the orthogonal projection of the
 * given point onto the given segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param s the segment
 * @param p the point
 * @return the squared distance
 */
template <typename T, size_t S>
point_distance<T> squared_distance(const segment<T, S>& s, const vec<T, S>& p)
{
  const auto vector = s.end() - s.start();
  const auto len = length(vector);
  const auto dir = vector / len;
  const T scale = dot(p - s.start(), dir);

  const T position = min(max(T(0.0), scale), len);
  const T distance = squared_length(p - point_at_distance(s, position));
  return point_distance<T>(position, distance);
}

/**
 * Computes the minimal distance between a given point and a segment. Two values are
 * returned:
 *
 * - The distance between the closest point on given segment and the given point.
 * - The distance from the origin of the given segment the closest point on the given
 * segment.
 *
 * Thereby, the closest point on the given segment is the orthogonal projection of the
 * given point onto the given segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param s the segment
 * @param p the point
 * @return the distance
 */
template <typename T, size_t S>
point_distance<T> distance(const segment<T, S>& s, const vec<T, S>& p)
{
  auto distance2 = squared_distance(s, p);
  distance2.distance = sqrt(distance2.distance);
  return distance2;
}

/**
 * The distance of two abstract lines. Thereby, the lines may be unbounded in each
 * direction, that is, they may each represent one of the following primitives:
 *
 * - a line (unbounded in both directions)
 * - a ray (bounded in one direction by the ray origin)
 * - a segment (bounded in both directions)
 *
 * Uses the notions of the "closest point" on each segment, which is the point at which
 * the distance to the other segment is minimal.
 */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif
template <typename T>
struct line_distance
{
  /**
   * Indicates whether the lines are parallel.
   */
  bool parallel;

  /**
   * The distance between the closest point and the origin of the first line.
   * Never squared.
   */
  T position1;

  /**
   * The minimal distance between the segments.
   * Squared if squared_distance was used.
   */
  T distance;

  /**
   * The distance between the closest point and the origin of the second line.
   * Never squared.
   */
  T position2;

  /**
   * Creates a new instance for the case when the segments are parallel.
   *
   * @param i_position1 the value for position1
   * @param i_distance the value for distance
   * @param i_position2 the value for position2
   * @return the instance
   */
  constexpr static line_distance Parallel(
    const T i_position1, const T i_distance, const T i_position2)
  {
    return {true, i_position1, i_distance, i_position2};
  }

  /**
   * Creates a new instance for the case when the segments are not parallel.
   *
   * @param i_position1 the value for position1
   * @param i_distance the value for distance
   * @param i_position2 the value for position2
   * @return the instance
   */
  constexpr static line_distance NonParallel(
    const T i_position1, const T i_distance, const T i_position2)
  {
    return {false, i_position1, i_distance, i_position2};
  }

  /**
   * Indicates whether the segments are colinear, and whether their distance is at most
   * the given value.
   *
   * @param maxDistance the maximal distance
   * @return true if the two segments are colinear and their distance is at most the given
   * value
   */
  constexpr bool is_colinear(const T maxDistance = constants<T>::almost_zero()) const
  {
    return parallel && distance <= maxDistance;
  }
};
#ifdef __clang__
#pragma clang diagnostic pop
#endif

/**
 * Computes the squared minimal distance of the given ray and the given line segment.
 * position1 and position2 are not squared.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param s the segment
 * @return the squared minimal distance (position1 and position2 are not squared)
 */
template <typename T, size_t S>
line_distance<T> squared_distance(const ray<T, S>& r, const segment<T, S>& s)
{
  const auto& p1 = s.start();
  const auto& p2 = s.end();

  auto u = p2 - p1;
  auto v = r.direction;
  auto w = p1 - r.origin;

  const auto a = dot(u, u); // squared length of u
  const auto b = dot(u, v);
  const auto c = dot(v, v);
  const auto d = dot(u, w);
  const auto e = dot(v, w);
  const auto D = a * c - b * b;

  if (is_zero(D, constants<T>::almost_zero()))
  {
    // parallel case
    const T p1_on_r = distance_to_projected_point(r, p1);
    const T p2_on_r = distance_to_projected_point(r, p2);

    if (p1_on_r < static_cast<T>(0) && p2_on_r < static_cast<T>(0))
    {
      // segment completely behind ray
      if (p1_on_r > p2_on_r)
      {
        // p1 closer to ray origin
        return line_distance<T>::Parallel(0, squared_distance(r.origin, p1), 0);
      }
      else
      {
        // p2 closer to ray origin
        return line_distance<T>::Parallel(
          0, squared_distance(r.origin, p2), p2_on_r - p1_on_r);
      }
    }
    else if (p1_on_r > static_cast<T>(0) && p2_on_r > static_cast<T>(0))
    {
      // segment completely in front of ray origin
      const T perpendicular_dist_squared =
        squared_distance(point_at_distance(r, p1_on_r), p1);
      if (p1_on_r > p2_on_r)
      {
        // p2 closer to ray origin
        return line_distance<T>::Parallel(
          p2_on_r, perpendicular_dist_squared, p1_on_r - p2_on_r);
      }
      else
      {
        // p1 closer to ray origin
        return line_distance<T>::Parallel(p1_on_r, perpendicular_dist_squared, 0);
      }
    }
    else
    {
      // segment straddles ray origin
      const T perpendicular_dist_squared =
        squared_distance(point_at_distance(r, p1_on_r), p1);
      const T r_origin_on_s = distance_to_projected_point(s, r.origin);
      return line_distance<T>::Parallel(0, perpendicular_dist_squared, r_origin_on_s);
    }
  }

  T sN, sD = D;
  T tN, tD = D;

  sN = (b * e - c * d);
  tN = (a * e - b * d);
  if (sN < static_cast<T>(0.0))
  {
    sN = static_cast<T>(0.0);
    tN = e;
    tD = c;
  }
  else if (sN > sD)
  {
    sN = sD;
    tN = e + b;
    tD = c;
  }

  const auto sc =
    is_zero(sN, constants<T>::almost_zero()) ? static_cast<T>(0.0) : sN / sD;
  const auto tc = max(
    is_zero(tN, constants<T>::almost_zero()) ? static_cast<T>(0.0) : tN / tD,
    static_cast<T>(0.0));

  u = u * sc; // vector from p1 to the closest point on the segment
  v = v * tc; // vector from ray origin to closest point on the ray
  w = w + u;
  const auto dP = w - v;

  return line_distance<T>::NonParallel(tc, squared_length(dP), sc * sqrt(a));
}

/**
 * Computes the minimal distance of the given ray and the given line segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param s the segment
 * @return the minimal distance
 */
template <typename T, size_t S>
line_distance<T> distance(const ray<T, S>& r, const segment<T, S>& s)
{
  auto distance2 = squared_distance(r, s);
  distance2.distance = sqrt(distance2.distance);
  return distance2;
}

/**
 * Computes the squared minimal distance of the given rays.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first ray
 * @param rhs the second ray
 * @return the squared minimal distance
 */
template <typename T, size_t S>
constexpr line_distance<T> squared_distance(const ray<T, S>& lhs, const ray<T, S>& rhs)
{
  auto u = rhs.direction;
  auto v = lhs.direction;
  auto w = rhs.origin - lhs.origin;

  const auto a = dot(u, u); // other.direction.dot(other.direction) (squared length)
  const auto b = dot(u, v); // other.direction.dot(this.direction)
  const auto c = dot(v, v); // this.direction.dot(this.direction) (squared length)
  const auto d = dot(u, w); // other.direction.dot(origin delta)
  const auto e = dot(v, w); // this.direction.dot(origin delta)
  const auto D = a * c - b * b;
  auto sD = D;
  auto tD = D;

  if (is_zero(D, constants<T>::almost_zero()))
  {
    // parallel case
    const T rhs_origin_on_lhs = distance_to_projected_point(lhs, rhs.origin);
    const T lhs_origin_on_rhs = distance_to_projected_point(rhs, lhs.origin);
    const T perpendicular_dist_squared =
      squared_distance(project_point(lhs, rhs.origin), rhs.origin);

    return line_distance<T>::Parallel(
      max(static_cast<T>(0), rhs_origin_on_lhs),
      perpendicular_dist_squared,
      max(static_cast<T>(0), lhs_origin_on_rhs));
  }

  auto sN = (b * e - c * d);
  auto tN = (a * e - b * d);
  if (sN < static_cast<T>(0.0))
  {
    sN = static_cast<T>(0.0);
    tN = e;
    tD = c;
  }

  const auto sc =
    is_zero(sN, constants<T>::almost_zero()) ? static_cast<T>(0.0) : sN / sD;
  const auto tc = max(
    is_zero(tN, constants<T>::almost_zero()) ? static_cast<T>(0.0) : tN / tD,
    static_cast<T>(0.0));

  u = u * sc; // vector from the second ray's origin to the closest point on first ray
  v = v * tc; // vector from the first ray's origin to closest point on the first ray
  w = w + u;
  const auto dP = w - v;

  return line_distance<T>::NonParallel(tc, squared_length(dP), sc);
}

/**
 * Computes the minimal distance of the given rays.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first ray
 * @param rhs the second ray
 * @return the minimal distance
 */
template <typename T, size_t S>
line_distance<T> distance(const ray<T, S>& lhs, const ray<T, S>& rhs)
{
  auto distance2 = squared_distance(lhs, rhs);
  distance2.distance = sqrt(distance2.distance);
  return distance2;
}

/**
 * Computes the squared minimal distance of the given ray and the given line.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param l the line
 * @return the squared minimal distance
 */
template <typename T, size_t S>
constexpr line_distance<T> squared_distance(const ray<T, S>& r, const line<T, S>& l)
{
  const auto w0 = r.origin - l.point;
  const auto a = dot(r.direction, r.direction);
  const auto b = dot(r.direction, l.direction);
  const auto c = dot(l.direction, l.direction);
  const auto d = dot(r.direction, w0);
  const auto e = dot(l.direction, w0);

  const auto D = a * c - b * b;
  if (is_zero(D, constants<T>::almost_zero()))
  {
    // parallel case
    const T perpendicular_dist_squared =
      squared_distance(project_point(r, l.point), l.point);
    const T r_origin_on_l = distance_to_projected_point(l, r.origin); // can be negative
    return line_distance<T>::Parallel(0, perpendicular_dist_squared, r_origin_on_l);
  }

  const auto sc = max((b * e - c * d) / D, static_cast<T>(0.0));
  const auto tc = (a * e - b * d) / D;

  const auto rp = r.origin + sc * r.direction; // point on ray
  const auto lp = l.point + tc * l.direction;  // point on line
  return line_distance<T>::NonParallel(sc, squared_length(rp - lp), tc);
}

/**
 * Computes the minimal distance of the given ray and the given line.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param l the line
 * @return the minimal distance
 */
template <typename T, size_t S>
line_distance<T> distance(const ray<T, S>& r, const line<T, S>& l)
{
  auto distance2 = squared_distance(r, l);
  distance2.distance = sqrt(distance2.distance);
  return distance2;
}
} // namespace vm
