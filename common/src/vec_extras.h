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

#ifndef TrenchBroom_vec_extras_h
#define TrenchBroom_vec_extras_h

#include "Vec.h"

/* ========== rounding and error correction ========== */

/**
 * Returns a vector where each component is the rounded value of the corresponding component of the given
 * vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> round(const Vec<T,S>& vec) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::round(vec[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector down to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round down
 * @param m the multiples to round down to
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> roundDownToMultiple(const Vec<T,S>& vec, const Vec<T,S>& m) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundDownToMultiple(vec[i], m[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector up to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round down
 * @param m the multiples to round up to
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> roundUpToMultiple(const Vec<T,S>& vec, const Vec<T,S>& m) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundUpToMultiple(vec[i], m[i]);
    }
    return result;
}

/**
 * Rounds the components of the given vector to multiples of the components of the given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to round down
 * @param m the multiples to round to
 * @return the rounded vector
 */
template <typename T, size_t S>
Vec<T,S> roundToMultiple(const Vec<T,S>& vec, const Vec<T,S>& m) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::roundToMultiple(vec[i], m[i]);
    }
    return result;
}

/**
 * Corrects the given vector's components to the given number of decimal places.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to correct
 * @param decimals the number of decimal places to keep
 * @param epsilon the epsilon value
 * @return the corrected vector
 */
template <typename T, size_t S>
Vec<T,S> correct(const Vec<T,S>& vec, const size_t decimals = 0, const T epsilon = Math::Constants<T>::correctEpsilon()) {
    Vec<T,S> result;
    for (size_t i = 0; i < S; ++i) {
        result[i] = Math::correct(vec[i], decimals, epsilon);
    }
    return result;
}

/**
 * Given three colinear points, this function checks whether the first point is contained in a segment formed by the
 * other two points.
 *
 * The result is undefined for the case of non-colinear points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param point the point to check
 * @param start the segment start
 * @param end the segment end
 * @return true if the given point is contained within the segment
 */
template <typename T, size_t S>
bool between(const Vec<T,S>& point, const Vec<T,S>& start, const Vec<T,S>& end) {
    assert(colinear(point, start, end));
    const Vec<T,S> toStart = start - point;
    const Vec<T,S> toEnd   =   end - point;

    const T d = dot(toEnd, normalize(toStart));
    return !Math::pos(d);
}

/**
 * Computes the average of the given range of elements, using the given function to transform an element into a vector.
 *
 * @tparam I the type of the range iterators
 * @tparam G the type of the transformation function from a range element to a vector type
 * @param cur the start of the range
 * @param end the end of the range
 * @param get the transformation function, defaults to identity
 * @return the average of the vectors obtained from the given range of elements
 */
template <typename I, typename G>
auto average(I cur, I end, const G& get = Math::Identity()) -> typename std::remove_reference<decltype(get(*cur))>::type {
    assert(cur != end);

    auto result = get(*cur++);
    auto count = 1.0;
    while (cur != end) {
        result += get(*cur++);
        count += 1.0;
    }
    return result / count;
}

/**
 * Computes the CCW angle between axis and vector in relation to the given up vector. All vectors are expected to be
 * normalized. The CCW angle is the angle by which the given axis must be rotated in CCW direction about the given up
 * vector so that it becomes identical to the given vector.
 *
 * @tparam T the coordinate type
 * @param vec the vector
 * @param axis the axis
 * @param up the up vector
 * @return the CCW angle
 */
template <typename T>
T angleBetween(const Vec<T,3>& vec, const Vec<T,3>& axis, const Vec<T,3>& up) {
    const auto cos = dot(vec, axis);
    if (Math::one(+cos)) {
        return static_cast<T>(0.0);
    } else if (Math::one(-cos)) {
        return Math::Constants<T>::pi();
    } else {
        const auto perp = cross(axis, vec);
        if (!Math::neg(dot(perp, up))) {
            return std::acos(cos);
        } else {
            return Math::Constants<T>::twoPi() - std::acos(cos);
        }
    }
}

/**
 * Return type for the distanceToSegment function. Contains the point on a segment which is closest to some given
 * point, and the distance between that segment point and the given point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, size_t S>
struct EdgeDistance {
    /**
     * The closest point on a given segment to a given point.
     */

    const Vec<T,S> point;
    /**
     * The distance between the closest segment point and a given point.
     */
    const T distance;

    /**
     * Constructs a new instance with the given info.
     *
     * @param i_point the closest point on the segment
     * @param i_distance the distance between the closest point and the given point
     */
    EdgeDistance(const Vec<T,S>& i_point, const T i_distance) :
            point(i_point),
            distance(i_distance) {}
};

/**
 * Given a point X and a segment represented by two points A and B, this function computes the closest point P on the
 * segment AB and the given point X, as well as the distance between X and P.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param point the point
 * @param start the start point of the segment
 * @param end the end point of the segment
 * @return a struct containing the closest point on the segment and the distance between that point and the given point
 */
template <typename T, size_t S>
EdgeDistance<T,S> distanceOfPointAndSegment(const Vec<T,S>& point, const Vec<T,S>& start, const Vec<T,S>& end) {
    const Vec<T,S> edgeVec = end - start;
    const Vec<T,S> edgeDir = normalize(edgeVec);
    const T scale = dot(point - start, edgeDir);

    // determine the closest point on the edge
    Vec<T,S> closestPoint;
    if (scale < 0.0) {
        closestPoint = start;
    } else if ((scale * scale) > squaredLength(edgeVec)) {
        closestPoint = end;
    } else {
        closestPoint = start + edgeDir * scale;
    }

    const T distance = length(point - closestPoint);
    return EdgeDistance<T,S>(closestPoint, distance);
}

#endif
