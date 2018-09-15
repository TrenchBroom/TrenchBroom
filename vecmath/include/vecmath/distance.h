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

#include "vec.h"
#include "line.h"
#include "ray.h"
#include "segment.h"
#include "polygon.h"

#include <cstddef>

namespace vm {
    /**
     * The distance of a point to an abstract line, which could be an infinite line, a ray, or a line segment.
     */
    template <typename T>
    struct PointDistance {
        /**
         * The distance from the origin of the line to the orthogonal projection of a point onto the line.
         */
        T position;

        /**
         * The distance between the orthogonal projection of a point to the point itself.
         */
        T distance;

        /**
         * Creates a new instance with the given values.
         *
         * @param i_position the position of the orthogonal projection of the point onto the line
         * @param i_distance the value of the distance
         */
        PointDistance(const T i_position, const T i_distance) :
        position(i_position),
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
        const auto position = max(dot(originToPoint, r.direction), T(0.0));
        if (position == T(0.0)) {
            return PointDistance<T>(position, squaredLength(originToPoint));
        } else {
            return PointDistance<T>(position, squaredLength(r.pointAtDistance(position) - p));
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
     * Computes the minimal squared distance between a given point and a segment. Two values are returned:
     *
     * - The squared distance between the closest point on given segment and the given point.
     * - The distance from the origin of the given segment the closest point on the given segment.
     *
     * Thereby, the closest point on the given segment is the orthogonal projection of the given point onto the given
     * segment.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param s the segment
     * @param p the point
     * @return the squared distance
     */
    template <typename T, size_t S>
    PointDistance<T> squaredDistance(const segment<T,S>& s, const vec<T,S>& p) {
        const auto vector = s.end() - s.start();
        const auto len = length(vector);
        const auto dir = vector / len;
        const T scale = dot(p - s.start(), dir);

        const T position = min(max(T(0.0), scale), len);
        const T distance = squaredLength(p - s.pointAtDistance(position));
        return PointDistance<T>(position, distance);
    }

    /**
     * Computes the minimal distance between a given point and a segment. Two values are returned:
     *
     * - The distance between the closest point on given segment and the given point.
     * - The distance from the origin of the given segment the closest point on the given segment.
     *
     * Thereby, the closest point on the given segment is the orthogonal projection of the given point onto the given
     * segment.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param s the segment
     * @param p the point
     * @return the distance
     */
    template <typename T, size_t S>
    PointDistance<T> distance(const segment<T,S>& s, const vec<T,S>& p) {
        auto distance2 = squaredDistance(s, p);
        distance2.distance = std::sqrt(distance2.distance);
        return distance2;
    }

    /**
     * The distance of two abstract lines. Thereby, the lines may be unbounded in each direction, that is, they may each
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
         * Indicates whether the lines are parallel.
         */
        bool parallel;

        /**
         * The distance between the closest point and the origin of the first line.
         */
        T position1;

        /**
         * The minimal distance between the segments.
         */
        T distance;

        /**
         * The distance between the closest point and the origin of the second line.
         */
        T position2;

        /**
         * Creates a new instance for the case when the segments are parallel.
         *
         * @param distance constant distance between the segments
         * @return the instance
         */
        static LineDistance Parallel(const T distance) {
            LineDistance result;
            result.parallel = true;
            result.position1 = nan<T>();
            result.distance = distance;
            result.position2 = nan<T>();
            return result;
        }

        /**
         * Creates a new instance for the case when the segments are not parallel.
         *
         * @param i_position1 the value for rayDistance
         * @param i_distance the value for distance
         * @param i_position2 the value for lineDistance
         * @return the instance
         */
        static LineDistance NonParallel(const T i_position1, const T i_distance, const T i_position2) {
            LineDistance result;
            result.parallel = false;
            result.position1 = i_position1;
            result.distance = i_distance;
            result.position2 = i_position2;
            return result;
        }

        /**
         * Indicates whether the segments are colinear, and whether their distance is at most the given value.
         *
         * @param maxDistance the maximal distance
         * @return true if the two segments are colinear and their distance is at most the given value
         */
        bool colinear(const T maxDistance = constants<T>::almostZero()) const {
            return parallel && distance <= maxDistance;
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

        if (isZero(D)) {
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

        const auto sc = isZero(sN) ? static_cast<T>(0.0) : sN / sD;
        const auto tc = std::max(isZero(tN) ? static_cast<T>(0.0) : tN / tD, static_cast<T>(0.0));

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

        const auto a = dot(u, u); // other.direction.dot(other.direction) (squared length)
        const auto b = dot(u, v); // other.direction.dot(this.direction)
        const auto c = dot(v, v); // this.direction.dot(this.direction) (squared length)
        const auto d = dot(u, w); // other.direction.dot(origin delta)
        const auto e = dot(v, w); // this.direction.dot(origin delta)
        const auto D = a * c - b * b;
        T sN, sD = D;
        T tN, tD = D;

        if (isZero(D)) {
            const auto f = dot(w, v);
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

        const auto sc = isZero(sN) ? static_cast<T>(0.0) : sN / sD;
        const auto tc = std::max(isZero(tN) ? static_cast<T>(0.0) : tN / tD, static_cast<T>(0.0));

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
        const auto w0 = r.origin - l.point;
        const auto a = dot(r.direction, r.direction);
        const auto b = dot(r.direction, l.direction);
        const auto c = dot(l.direction, l.direction);
        const auto d = dot(r.direction, w0);
        const auto e = dot(l.direction, w0);

        const auto D = a * c - b * b;
        if (isZero(D)) {
            const auto f = dot(w0, l.direction);
            const auto z = w0 - f * l.direction;
            return LineDistance<T>::Parallel(squaredLength(z));
        }

        const auto sc = std::max((b * e - c * d) / D, static_cast<T>(0.0));
        const auto tc = (a * e - b * d) / D;

        const auto rp = r.origin + sc * r.direction; // point on ray
        const auto lp = l.point + tc * l.direction; // point on line
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
}

#endif //TRENCHBROOM_DISTANCE_H
