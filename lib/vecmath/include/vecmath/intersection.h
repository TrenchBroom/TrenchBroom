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

#include "bbox.h"
#include "line.h"
#include "plane.h"
#include "ray.h"
#include "scalar.h"
#include "util.h"
#include "vec.h"

namespace vm {

namespace detail {
/**
 * Checks whether the given segment line intersects the positive X axis.
 *
 * @tparam T the component type
 * @param v0 the first segment vertex
 * @param v1 the second segment vertex
 * @return -1 if either segment vertex is identical to the origin, +1 if the segment intersects the
 * positve X axis, and 0 if it does not
 */
template <typename T>
constexpr int handle_polygon_edge_intersection(const vec<T, 3>& v0, const vec<T, 3>& v1) {
  if (is_zero(v0, constants<T>::almost_zero())) {
    // the point is identical to a polygon vertex, cancel search
    return -1;
  }

  /*
   * A polygon edge intersects with the positive X axis if the
   * following conditions are met: The Y coordinates of its
   * vertices must have different signs (we assign a negative sign
   * to 0 here in order to count it as a negative number) and one
   * of the following two conditions must be met: Either the X
   * coordinates of the vertices are both positive or the X
   * coordinates of the edge have different signs (again, we
   * assign a negative sign to 0 here). In the latter case, we
   * must calculate the point of intersection between the edge and
   * the X axis and determine whether its X coordinate is positive
   * or zero.
   */

  // Does Y segment covered by the given edge touch the X axis at all?
  if (
    (is_zero(v0.y(), constants<T>::almost_zero()) &&
     is_zero(v1.y(), constants<T>::almost_zero())) ||
    (v0.y() > T(0.0) && v1.y() > T(0.0)) || (v0.y() < T(0.0) && v1.y() < T(0.0))) {
    return 0;
  }

  // Is segment entirely on the positive side of the X axis?
  if (v0.x() > T(0.0) && v1.x() > T(0.0)) {
    return 1;
  }

  // Is segment entirely on the negative side of the X axis?
  if (v0.x() < T(0.0) && v1.x() < T(0.0)) {
    return 0;
  }

  // Calculate the point of intersection between the edge and the X axis.
  const T x = -v0.y() * (v1.x() - v0.x()) / (v1.y() - v0.y()) + v0.x();

  // Is the point of intersection on the given edge?
  if (is_zero(x, constants<T>::almost_zero())) {
    return -1;
  }

  // Is the point of intersection on the positive X axis?
  if (x > T(0.0)) {
    return 1;
  }

  // The point of intersection is on the negative X axis.
  return 0;
}
} // namespace detail

/**
 * Checks whether the given point is contained in the polygon formed by the given range of vertices.
 *
 * This function assumes that the point is in the same plane as the polygon, but this is not checked
 * or asserted.
 *
 * @tparam T the component type
 * @tparam I the vertex range iterator
 * @tparam G a transformation from the range elements to points
 * @param p the point to check
 * @param axis the major axis of the polygon's normal
 * @param cur the range start iterator
 * @param end the range end iterator
 * @param get the transformation function to apply to the range elements to obtain a point of type
 * vec<T,S>
 * @return true if the given point is contained in the given polygon, and false otherwise
 */
template <typename T, typename I, typename G = identity>
constexpr bool polygon_contains_point(
  const vec<T, 3>& p, const axis::type axis, I cur, I end, const G& get = G()) {
  const auto o = swizzle(p, axis);

  const auto fv = swizzle(get(*cur++), axis) - o; // The first vertex.
  vec<T, 3> pv = fv;                              // The previous vertex.

  int d = 0;
  while (cur != end) {
    const vec<T, 3> cv = swizzle(get(*cur++), axis) - o; // The current vertex.
    const int s = detail::handle_polygon_edge_intersection(pv, cv);
    if (s == -1) {
      return true;
    }
    d += s;
    pv = cv;
  }

  // Handle the edge from the last to the first vertex.
  const int s = detail::handle_polygon_edge_intersection(pv, fv);
  if (s == -1) {
    return true;
  }

  d += s;
  return d % 2 != 0;
}

/**
 * Checks whether the given point is contained in the polygon formed by the given range of vertices.
 *
 * This function assumes that the point is in the same plane as the polygon, but this is not checked
 * or asserted.
 *
 * @tparam T the component type
 * @tparam I the vertex range iterator
 * @tparam G a transformation from the range elements to points
 * @param p the point to check
 * @param n the polygon's normal
 * @param cur the range start iterator
 * @param end the range end iterator
 * @param get the transformation function to apply to the range elements to obtain a point of type
 * vec<T,S>
 * @return true if the given point is contained in the given polygon, and false otherwise
 */
template <typename T, typename I, typename G = identity>
constexpr bool polygon_contains_point(
  const vec<T, 3>& p, const vec<T, 3>& n, I cur, I end, const G& get = G()) {
  return polygon_contains_point(p, find_abs_max_component(n), cur, end, get);
}

/**
 * Checks whether the given point is contained in the polygon formed by the given range of vertices.
 *
 * This function assumes that the point is in the same plane as the polygon, but this is not checked
 * or asserted.
 *
 * @tparam T the component type
 * @tparam I the vertex range iterator
 * @tparam G a transformation from the range elements to points
 * @param p the point to check
 * @param cur the range start iterator
 * @param end the range end iterator
 * @param get the transformation function to apply to the range elements to obtain a point of type
 * vec<T,S>
 * @return true if the given point is contained in the given polygon, and false otherwise
 */
template <typename T, typename I, typename G = identity>
bool polygon_contains_point(const vec<T, 3>& p, I cur, I end, const G& get = G()) {
  I temp = cur;

  assert(temp != end);
  const vec<T, 3> p1 = get(*temp++);
  assert(temp != end);
  const vec<T, 3> p2 = get(*temp++);
  assert(temp != end);
  const vec<T, 3> p3 = get(*temp);

  const auto normal_result = plane_normal(p1, p2, p3);
  [[maybe_unused]] const auto normal_valid = std::get<0>(normal_result);
  const auto normal_vector = std::get<1>(normal_result);
  assert(normal_valid);

  return polygon_contains_point(p, find_abs_max_component(normal_vector), cur, end, get);
}

/**
 * Computes the point of intersection between the given ray and the given plane, and returns the
 * distance on the given ray from the ray's origin to that point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param p the plane
 * @return the distance to the intersection point, or NaN if the ray does not intersect the plane
 */
template <typename T, size_t S>
constexpr T intersect_ray_plane(const ray<T, S>& r, const plane<T, S>& p) {
  const auto d = dot(r.direction, p.normal);
  if (is_zero(d, constants<T>::almost_zero())) {
    return nan<T>();
  }

  const auto s = dot(p.anchor() - r.origin, p.normal) / d;
  if (s < -constants<T>::almost_zero()) {
    return nan<T>();
  }

  return s;
}

/**
 * Compute the point of intersection of the given ray and a triangle with the given points as
 * vertices.
 *
 * @tparam T the component type
 * @param r the ray
 * @param p1 the first point
 * @param p2 the second point
 * @param p3 the third point
 * @return the distance to the point of intersection or NaN if the given ray does not intersect the
 * given triangle
 */
template <typename T>
constexpr T intersect_ray_triangle(
  const ray<T, 3>& r, const vec<T, 3>& p1, const vec<T, 3>& p2, const vec<T, 3>& p3) {
  // see
  // http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf

  const auto& o = r.origin;
  const auto& d = r.direction;
  const auto e1 = p2 - p1;
  const auto e2 = p3 - p1;
  const auto p = cross(d, e2);
  const auto a = dot(p, e1);
  if (is_zero(a, constants<T>::almost_zero())) {
    return nan<T>();
  }

  const auto t = o - p1;
  const auto q = cross(t, e1);

  const auto u = dot(q, e2) / a;
  if (u < T(0.0)) {
    return nan<T>();
  }

  const auto v = dot(p, t) / a;
  if (v < T(0.0)) {
    return nan<T>();
  }

  const auto w = dot(q, d) / a;
  if (w < T(0.0)) {
    return nan<T>();
  }

  if (v + w > T(1.0)) {
    return nan<T>();
  }

  return u;
}

/**
 * Computes the point of intersection of the given ray and the polygon with the given vertices.
 *
 * @tparam T the component type
 * @tparam I the vertex range iterator
 * @tparam G a transformation function that transforms a range element to a vec<T,3>
 * @param r the ray
 * @param p the plane on which all vertices lie
 * @param cur the vertex range start iterator
 * @param end the vertex range end iterator
 * @param get the transformation function
 * @return the distance from the origin of the ray to the point of intersection or NaN if the ray
 * does not intersect the polygon
 */
template <typename T, typename I, typename G = identity>
constexpr T intersect_ray_polygon(
  const ray<T, 3>& r, const plane<T, 3>& p, I cur, I end, const G& get = G()) {
  const auto distance = intersect_ray_plane(r, p);
  if (is_nan(distance)) {
    return distance;
  }

  const auto point = point_at_distance(r, distance);
  if (polygon_contains_point(point, p.normal, cur, end, get)) {
    return distance;
  }
  return nan<T>();
}

/**
 * Computes the point of intersection of the given ray and the polygon with the given vertices.
 *
 * @tparam T the component type
 * @tparam I the vertex range iterator
 * @tparam G a transformation function that transforms a range element to a vec<T,3>
 * @param r the ray
 * @param cur the vertex range start iterator
 * @param end the vertex range end iterator
 * @param get the transformation function
 * @return the distance from the origin of the ray to the point of intersection or NaN if the ray
 * does not intersect the polygon
 */
template <typename T, typename I, typename G = identity>
T intersect_ray_polygon(const ray<T, 3>& r, I cur, I end, const G& get = G()) {
  const auto [valid, plane] = from_points(cur, end, get);
  if (!valid) {
    return nan<T>();
  } else {
    return intersect_ray_polygon(r, plane, cur, end, get);
  }
}

/**
 * Computes the point of intersection between the given ray and the given bounding box, and returns
 * the distance on the given ray from the ray's origin to that point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param b the bounding box
 * @return the distance to the closest intersection point, or NaN if the ray does not intersect the
 * bounding box
 */
template <typename T, size_t S>
constexpr T intersect_ray_bbox(const ray<T, S>& r, const bbox<T, S>& b) {
  // Compute candidate planes
  T origins[S]{};
  bool inside[S]{};
  bool allInside = true;
  for (size_t i = 0; i < S; ++i) {
    if (r.origin[i] < b.min[i]) {
      origins[i] = b.min[i];
      allInside = inside[i] = false;
    } else if (r.origin[i] > b.max[i]) {
      origins[i] = b.max[i];
      allInside = inside[i] = false;
    } else {
      if (r.direction[i] < static_cast<T>(0.0)) {
        origins[i] = b.min[i];
      } else {
        origins[i] = b.max[i];
      }
      inside[i] = true;
    }
  }

  // Intersect candidate planes with ray
  T distances[S]{};
  for (size_t i = 0; i < S; ++i) {
    if (r.direction[i] != static_cast<T>(0.0)) {
      distances[i] = (origins[i] - r.origin[i]) / r.direction[i];
    } else {
      distances[i] = static_cast<T>(-1.0);
    }
  }

  size_t bestPlane = 0;
  if (allInside) {
    // find the closest plane that was hit
    for (size_t i = 1; i < S; ++i) {
      if (distances[i] < distances[bestPlane]) {
        bestPlane = i;
      }
    }
  } else {
    // find the farthest plane that was hit
    for (size_t i = 0; i < S; ++i) {
      if (!inside[i]) {
        bestPlane = i;
        break;
      }
    }
    for (size_t i = bestPlane + 1; i < S; ++i) {
      if (!inside[i] && distances[i] > distances[bestPlane]) {
        bestPlane = i;
      }
    }
  }

  // Check if the final candidate actually hits the box
  if (distances[bestPlane] < static_cast<T>(0.0)) {
    return nan<T>();
  }

  for (size_t i = 0; i < S; ++i) {
    if (bestPlane != i) {
      const T coord = r.origin[i] + distances[bestPlane] * r.direction[i];
      if (coord < b.min[i] || coord > b.max[i]) {
        return nan<T>();
      }
    }
  }

  return distances[bestPlane];
}

/**
 * Computes the point of intersection between the given ray and a sphere centered at the given
 * position and with the given radius.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param position the position of the sphere (its center)
 * @param radius the radius of the sphere
 * @return the distance to the closest intersection point, or NaN if the given ray does not
 * intersect the given sphere
 */
template <typename T, size_t S>
T intersect_ray_sphere(const ray<T, S>& r, const vec<T, S>& position, const T radius) {
  const auto diff = r.origin - position;

  const auto p = static_cast<T>(2.0) * dot(diff, r.direction);
  const auto q = squared_length(diff) - radius * radius;

  const auto d = p * p - static_cast<T>(4.0) * q;
  if (d < static_cast<T>(0.0)) {
    return nan<T>();
  }

  const auto s = sqrt(d);
  const auto t0 = (-p + s) / static_cast<T>(2.0);
  const auto t1 = (-p - s) / static_cast<T>(2.0);

  if (t0 < static_cast<T>(0.0) && t1 < static_cast<T>(0.0)) {
    return nan<T>();
  } else if (t0 > static_cast<T>(0.0) && t1 > static_cast<T>(0.0)) {
    return min(t0, t1);
  } else {
    return max(t0, t1);
  }
}

/**
 * Computes the point of intersection between the given ray and a torus centered at the given
 * position and with the given tube distance and radii. Note that the torus is assumed to be in the
 * XY plane.
 *
 * The given major radius is expected to be larger than the given minor radius.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param position the position of the torus (its center)
 * @param majorRadius the major radius (the distance between the tube's center and the center of the
 * torus)
 * @param minorRadius the minor radius (the radius of the tube)
 * @return the distance to the closest intersection point, or NaN if the given ray does not
 * intersect the given torus
 */
template <typename T, size_t S>
T intersect_ray_torus(
  const ray<T, S>& r, const vec<T, S>& position, const T majorRadius, const T minorRadius) {
  // see https://marcin-chwedczuk.github.io/ray-tracing-torus

  // translate the ray origin and act as if the torus is centered around the origin
  // since the distance from the ray origin to the point of intersection is invariant under
  // translation, we will not have to transform the result in any way
  const auto origin = r.origin - position;
  const auto dd = vm::dot(r.direction, r.direction);
  const auto od = vm::dot(origin, r.direction);
  const auto oo = vm::dot(origin, origin);
  const auto MM = majorRadius * majorRadius;
  const auto mm = minorRadius * minorRadius;
  const auto dz = r.direction.z();
  const auto oz = origin.z();
  const auto omM = oo - mm - MM;

  const auto a = dd * dd;
  const auto b = T(4.0) * dd * od;
  const auto c = T(2.0) * dd * omM + T(4.0) * (od * od + MM * dz * dz);
  const auto d = T(4.0) * od * omM + T(8.0) * MM * oz * dz;
  const auto e = omM * omM - T(4.0) * MM * (mm - oz * oz);

  auto [num, s1, s2, s3, s4] = vm::solve_quartic(a, b, c, d, e, vm::constants<T>::almost_zero());
  if (num == 0) {
    return vm::nan<T>();
  } else {
    // only consider positive solutions
    if (num > 0) {
      s1 = s1 > T(0.0) ? s1 : vm::nan<T>();
    }
    if (num > 1) {
      s2 = s2 > T(0.0) ? s2 : vm::nan<T>();
    }
    if (num > 2) {
      s3 = s3 > T(0.0) ? s3 : vm::nan<T>();
    }
    if (num > 3) {
      s4 = s4 > T(0.0) ? s4 : vm::nan<T>();
    }
    return safe_min(s1, s2, s3, s4);
  }
}

/**
 * Computes the point of intersection between the given ray and the given bounding box, and returns
 * the distance on the given line from the line's anchor to that point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param l the line
 * @param p the plane
 * @return the distance to the intersection point, or NaN if the line does not intersect the plane
 */
template <typename T, size_t S>
constexpr T intersect_line_plane(const line<T, S>& l, const plane<T, 3>& p) {
  const auto f = dot(l.direction, p.normal);
  if (is_zero(f, constants<T>::almost_zero())) {
    return nan<T>();
  } else {
    return dot(p.distance * p.normal - l.point, p.normal) / f;
  }
}

/**
 * Computes the line of intersection between the given planes.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param p1 the first plane
 * @param p2 the second plane
 * @return the line of intersection, or an uninitialized plane (with normal 0) if the planes are
 * parallel
 */
template <typename T, size_t S>
line<T, S> intersect_plane_plane(const plane<T, S>& p1, const plane<T, S>& p2) {
  const auto lineDirection = normalize(cross(p1.normal, p2.normal));

  if (is_nan(lineDirection)) {
    // the planes are parallel
    return line<T, S>();
  }

  // Now we need to find a point that is on both planes.

  // From: http://geomalgorithms.com/a05-_intersect-1.html
  // Project the other plane's normal onto this plane.
  // This will give us a line direction from this plane's anchor that
  // intersects the other plane.

  const auto lineToP2 = line<T, S>(p1.anchor(), normalize(p1.project_vector(p2.normal)));
  const auto dist = intersect_line_plane(lineToP2, p2);
  const auto point = point_at_distance(lineToP2, dist);

  if (is_nan(point)) {
    return line<T, S>();
  } else {
    return line<T, S>(point, lineDirection);
  }
}
} // namespace vm
