/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRENCHBROOM_DISTANCE_H
#define TRENCHBROOM_DISTANCE_H

#include "vec_decl.h"
#include "vec_impl.h"
#include "line_decl.h"
#include "ray_decl.h"
#include "ray_impl.h"
#include "segment_decl.h"
#include "polygon_decl.h"

#include <cstddef>

/**
 * The distance of a point to a ray.
 */
template <typename T>
struct PointDistance {
    /**
     * The distance from the origin of the ray to the orthogonal projection of a point onto the ray.
     */
    T rayDistance;

    /**
     * The distance between the orthogonal projection of a point to the point itself.
     */
    T distance;

    /**
     * Creates a new instance with the given values.
     *
     * @param i_rayDistance the value of the ray distance
     * @param i_distance the value of the distance
     */
    PointDistance(const T i_rayDistance, const T i_distance) :
    rayDistance(i_rayDistance),
    distance(i_distance) {}
};

/**
 * Computes the minimal squared distance between a given point and a ray. Two values are returned:
 *
 * - The squared distance between the closest point on given ray and the given point.
 * - The distance from the origin of the given ray the closest point on the given ray.
 *
 * Thereby, the closest point on the given ray is the orthogonal projection of the given point onto the given
 * ray.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param p the point
 * @return the squared distance
 */
template <typename T, size_t S>
PointDistance<T> squaredDistance(const ray<T,S>& r, const vec<T,S>& p) {
    const auto originToPoint = p - r.origin;
    const auto rayDistance = Math::max(dot(originToPoint, r.direction), static_cast<T>(0.0));
    if (rayDistance == static_cast<T>(0.0)) {
        return PointDistance<T>(rayDistance, squaredLength(originToPoint));
    } else {
        return PointDistance<T>(rayDistance, squaredLength(r.pointAtDistance(rayDistance) - p));
    }
}

/**
 * Computes the minimal distance between a given point and a ray. Two values are returned:
 *
 * - The distance between the closest point on given ray and the given point.
 * - The distance from the origin of the given ray the closest point on the given ray.
 *
 * Thereby, the closest point on the given ray is the orthogonal projection of the given point onto the given
 * ray.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param p the point
 * @return the distance
 */
template <typename T, size_t S>
PointDistance<T> distance(const ray<T,S>& r, const vec<T,S>& p) {
    auto distance2 = squaredDistance(r, p);
    distance2.distance = std::sqrt(distance2.distance);
    return distance2;
}

/**
 * The distance of two line segments. Thereby, the segments may be unbounded in each direction, that is, they may each
 * represent one of the following primitives:
 *
 * - a line (unbounded in both directions)
 * - a ray (bounded in one direction by the ray origin)
 * - a segment (bounded in both directions)
 *
 * Uses the notions of the "closest point" on each segment, which is the point at which the distance to the other segment
 * is minimal.
 */
template <typename T>
struct LineDistance {
    /**
     * Indicates whether the segments are parallel.
     */
    bool parallel;

    /**
     * The distance between the closest point and the origin of the first segment.
     */
     // TODO 2201: Rename this to closestPoint1 or something
    T rayDistance;

    /**
     * The minimal distance between the segments.
     */
    T distance;

    /**
     * The distance between the closest point and the origin of the second segment.
     */
    // TODO 2201: Rename this to closestPoint2 or something
    T lineDistance;

    /**
     * Creates a new instance for the case when the segments are parallel.
     *
     * @param distance constant distance between the segments
     * @return the instance
     */
    static LineDistance Parallel(const T distance) {
        LineDistance result;
        result.parallel = true;
        result.rayDistance = Math::nan<T>();
        result.distance = distance;
        result.lineDistance = Math::nan<T>();
        return result;
    }

    /**
     * Creates a new instance for the case when the segments are not parallel.
     *
     * @param rayDistance the value for rayDistance
     * @param distance the value for distance
     * @param lineDistance the value for lineDistance
     * @return the instance
     */
    static LineDistance NonParallel(const T rayDistance, const T distance, const T lineDistance) {
        LineDistance result;
        result.parallel = false;
        result.rayDistance = rayDistance;
        result.distance = distance;
        result.lineDistance = lineDistance;
        return result;
    }

    /**
     * Indicates whether the segments are colinear, and whether their distance is at most the given value.
     *
     * @param maxDistance the maximal distance
     * @return true if the two segments are colinear and their distance is at most the given value
     */
    bool colinear(const T maxDistance = Math::Constants<T>::almostZero()) const {
        return parallel && Math::lte(distance, maxDistance);
    }
};

/**
 * Computes the squared minimal distance of the given ray and the given line segment.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param s the segment
 * @return the squared minimal distance
 */
template <typename T, size_t S>
LineDistance<T> squaredDistance(const ray<T,S>& r, const segment<T,S>& s) {
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

    if (Math::zero(D)) {
        const auto f = dot(w, v);
        const auto z = w - f * v;
        return LineDistance<T>::Parallel(squaredLength(z));
    }

    T sN, sD = D;
    T tN, tD = D;

    sN = (b * e - c * d);
    tN = (a * e - b * d);
    if (sN < static_cast<T>(0.0)) {
        sN = static_cast<T>(0.0);
        tN = e;
        tD = c;
    } else if (sN > sD) {
        sN = sD;
        tN = e + b;
        tD = c;
    }

    const auto sc = Math::zero(sN) ? static_cast<T>(0.0) : sN / sD;
    const auto tc = std::max(Math::zero(tN) ? static_cast<T>(0.0) : tN / tD, static_cast<T>(0.0));

    u = u * sc; // vector from p1 to the closest point on the segment
    v = v * tc; // vector from ray origin to closest point on the ray
    w = w + u;
    const auto dP = w - v;

    return LineDistance<T>::NonParallel(tc, squaredLength(dP), sc * std::sqrt(a));
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
LineDistance<T> distance(const ray<T,S>& r, const segment<T,S>& s) {
    auto distance2 = squaredDistance(r, s);
    distance2.distance = std::sqrt(distance2.distance);
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
LineDistance<T> squaredDistance(const ray<T,S>& lhs, const ray<T,S>& rhs) {
    auto u = rhs.direction;
    auto v = lhs.direction;
    auto w = rhs.origin - lhs.origin;

    const auto a = u.dot(u); // other.direction.dot(other.direction) (squared length)
    const auto b = u.dot(v); // other.direction.dot(this.direction)
    const auto c = v.dot(v); // this.direction.dot(this.direction) (squared length)
    const auto d = u.dot(w); // other.direction.dot(origin delta)
    const auto e = v.dot(w); // this.direction.dot(origin delta)
    const auto D = a * c - b * b;
    T sN, sD = D;
    T tN, tD = D;

    if (Math::zero(D)) {
        const auto f = w.dot(v);
        const auto z = w - f * v;
        return LineDistance<T>::Parallel(squaredLength(z));
    }

    sN = (b * e - c * d);
    tN = (a * e - b * d);
    if (sN < static_cast<T>(0.0)) {
        sN = static_cast<T>(0.0);
        tN = e;
        tD = c;
    }

    const auto sc = Math::zero(sN) ? static_cast<T>(0.0) : sN / sD;
    const auto tc = std::max(Math::zero(tN) ? static_cast<T>(0.0) : tN / tD, static_cast<T>(0.0));

    u = u * sc; // vector from the second ray's origin to the closest point on first ray
    v = v * tc; // vector from the first ray's origin to closest point on the first ray
    w = w + u;
    const auto dP = w - v;

    return LineDistance<T>::NonParallel(tc, squaredLength(dP), sc);
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
LineDistance<T> distance(const ray<T,S>& lhs, const ray<T,S>& rhs) {
    auto distance2 = squaredDistance(lhs, rhs);
    distance2.distance = std::sqrt(distance2.distance);
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
LineDistance<T> squaredDistance(const ray<T,S>& r, const line<T,S>& l) {
    const vec<T,S> w0 = r.origin - l.point;
    const T a = dot(r.direction, r.direction);
    const T b = dot(r.direction, l.direction);
    const T c = dot(l.direction, l.direction);
    const T d = dot(r.direction, w0);
    const T e = dot(l.direction, w0);

    const T D = a * c - b * b;
    if (Math::zero(D)) {
        const T f = dot(w0, l.direction);
        const vec<T,S> z = w0 - f * l.direction;
        return LineDistance<T>::Parallel(squaredLength(z));
    }

    const T sc = std::max((b * e - c * d) / D, static_cast<T>(0.0));
    const T tc = (a * e - b * d) / D;

    const vec<T,S> rp = r.origin + sc * r.direction; // point on ray
    const vec<T,S> lp = l.point + tc * l.direction; // point on line
    return LineDistance<T>::NonParallel(sc, squaredLength(rp - lp), tc);
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
LineDistance<T> distance(const ray<T,S>& r, const line<T,S>& l) {
    auto distance2 = squaredDistance(r, l);
    distance2.distance = std::sqrt(distance2.distance);
    return distance2;
}

#endif //TRENCHBROOM_DISTANCE_H
