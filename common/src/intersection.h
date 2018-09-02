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

#ifndef TRENCHBROOM_RAYTRACE_H
#define TRENCHBROOM_RAYTRACE_H

#include "MathUtils.h"
#include "vec_decl.h"
#include "vec_impl.h"
#include "Ray.h"
#include "bbox_decl.h"
#include "line_decl.h"
#include "Plane.h"

#include <array>

/**
 * Computes the point of intersection between the given ray and the given plane, and returns the distance
 * on the given ray from the ray's origin to that point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param p the plane
 * @return the distance to the intersection point, or NaN if the ray does not intersect the plane
 */
template <typename T, size_t S>
T intersect(const Ray<T,S>& r, const Plane<T,S>& p) {
    const auto d = dot(r.direction, p.normal);
    if (Math::zero(d)) {
        return Math::nan<T>();
    }

    const auto s = dot(p.anchor() - r.origin, p.normal) / d;
    if (Math::neg(s)) {
        return Math::nan<T>();
    }

    return s;
}

/**
 * Computes the point of intersection between the given ray and the given bounding box, and returns the distance
 * on the given ray from the ray's origin to that point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param r the ray
 * @param b the bounding box
 * @return the distance to the intersection point, or NaN if the ray does not intersect the bounding box
 */
template <typename T, size_t S>
T intersect(const Ray<T,S>& r, const bbox<T,S>& b) {
    // Compute candidate planes
    std::array<T, S> origins;
    std::array<bool, S> inside;
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
    std::array<T, S> distances;
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
        return std::numeric_limits<T>::quiet_NaN();
    }

    for (size_t i = 0; i < S; ++i) {
        if (bestPlane != i) {
            const T coord = r.origin[i] + distances[bestPlane] * r.direction[i];
            if (coord < b.min[i] || coord > b.max[i]) {
                return std::numeric_limits<T>::quiet_NaN();
            }
        }
    }

    return distances[bestPlane];
}


/**
 * Computes the point of intersection between the given ray and the given bounding box, and returns the distance
 * on the given line from the line's anchor to that point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param l the line
 * @param p the plane
 * @return the distance to the intersection point, or NaN if the line does not intersect the plane
 */
template <typename T, size_t S>
T intersect(const line<T,S>& l, const Plane<T,3>& p) {
    const auto f = dot(l.direction, p.normal);
    if (Math::zero(f)) {
        return Math::nan<T>();
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
 * @return the line of intersection, or an uninitialized plane (with normal 0) if the planes are parallel
 */
template <typename T, size_t S>
line<T,S> intersect(const Plane<T,S>& p1, const Plane<T,S>& p2) {
    const auto lineDirection = normalize(cross(p1.normal, p2.normal));

    if (isNaN(lineDirection)) {
        // the planes are parallel
        return line<T,S>();
    }

    // Now we need to find a point that is on both planes.

    // From: http://geomalgorithms.com/a05-_intersect-1.html
    // Project the other plane's normal onto this plane.
    // This will give us a line direction from this plane's anchor that
    // intersects the other plane.

    const auto lineToP2 = line<T,S>(p1.anchor(), normalize(p1.projectVector(p2.normal)));
    const auto dist = intersect(lineToP2, p2);
    const auto point = lineToP2.pointAtDistance(dist);

    if (isNaN(point)) {
        return line<T,S>();
    } else {
        return line<T,S>(point, lineDirection);
    }
}

#endif //TRENCHBROOM_RAYTRACE_H
