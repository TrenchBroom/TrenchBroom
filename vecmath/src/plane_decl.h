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

#ifndef TRENCHBROOM_PLANE_DECL_H
#define TRENCHBROOM_PLANE_DECL_H

#include "utils.h"
#include "forward.h"
#include "vec_decl.h"
#include "vec_impl.h"
#include "line_decl.h"
#include "ray_decl.h"

#include <set>
#include <tuple>
#include <type_traits>
#include <vector>

namespace vm {
    /**
     * A plane, represented as a normal vector and a distance of the plane from the origin.
     *
     * @tparam T the component type
     * @tparam S the number of components
     */
    template <typename T, size_t S>
    class plane {
    public:
        using List = std::vector<plane>;

        T distance;
        vec<T,S> normal;

        /**
         * Creates a new plane by setting all components to 0.
         */
        plane();

        // Copy and move constructors
        plane(const plane<T,S>& other) = default;
        plane(plane<T,S>&& other) noexcept = default;

        // Assignment operators
        plane<T,S>& operator=(const plane<T,S>& other) = default;
        plane<T,S>& operator=(plane<T,S>&& other) noexcept = default;

        /**
         * Converts the given plane by converting its components.
         *
         * @tparam U the component type of the given plane
         * @param other the plane to convert
         */
        template <typename U>
        explicit plane(const plane<U,S>& other) :
        distance(static_cast<T>(other.distance)),
        normal(other.normal) {}

        /**
         * Creates a new plane with the given distance and normal vector
         *
         * @param i_distance the distance
         * @param i_normal the normal vector
         */
        plane(T i_distance, const vec<T,S>& i_normal);

        /**
         * Creates a new plane with the given anchor point and normal.
         *
         * @param i_anchor the anchor point (any point on the plane)
         * @param i_normal the normal vector
         */
        plane(const vec<T,S>& i_anchor, const vec<T,S>& i_normal);

        /**
         * Returns a point on this plane with minimal distance to the coordinate system origin. Assumes that the normal has
         * unit length.
         *
         * @return the anchor point
         */
        vec<T,S> anchor() const;

        /**
         * Given a line, this function computes the intersection of the line and this plane. The line is restricted to be
         * parallel to a coordinate system axis (indicated by the axis parameter), and the point has a dimension of one less
         * than this plane (since the position of the point along the given axis does not matter).
         *
         * This function then computes the point of intersection, and returns the missing component of the given point, i.e.
         * the value of the component that, if inserted into the components of the point, would yield the intersection point.
         *
         * @param point the point
         * @param axis the axis
         * @return the missing component to transform the given point to the point of intersection
         */
        T at(const vec<T,S-1>& point, const Axis::Type axis) const;

        T xAt(const vec<T,S-1>& point) const;

        T yAt(const vec<T,S-1>& point) const;

        T zAt(const vec<T,S-1>& point) const;

        /**
         * Computes the distance of the given point to this plane. The sign of the distance indicates whether the point
         * is above or below this plane.
         *
         * @param point the point
         * @return the distance of the given point to this plane
         */
        T pointDistance(const vec<T,S>& point) const;

        /**
         * Determines the relative position of the given point to this plane. A plane can either be above (in direction of
         * the normal), below (opposite direction), or inside this plane.
         *
         * @param point the point to check
         * @param epsilon an epsilon value (the maximum absolute distance up to which a point will be considered to be inside)
         * @return a value indicating the point status
         */
        PointStatus::Type pointStatus(const vec<T,S>& point, T epsilon = Constants<T>::pointStatusEpsilon()) const;

        /**
         * Flips this plane by negating its normal.
         *
         * @return the flipped plane
         */
        plane<T,S> flip() const;

        /**
         * Transforms this plane using the given transformation matrix. The translational part is not applied to the normal.
         *
         * @param transform the transformation to apply
         * @return the transformed plane
         */
        plane<T,S> transform(const mat<T,S+1,S+1>& transform) const;

        /**
         * Projects the given point onto this plane along the plane normal.
         *
         * @param point the point to project
         * @return the projected point
         */
        vec<T,S> projectPoint(const vec<T,S>& point) const;

        /**
         * Projects the given point onto this plane along the given direction.
         *
         * @param point the point to project
         * @param direction the projection direction
         * @return the projected point
         */
        vec<T,S> projectPoint(const vec<T,S>& point, const vec<T,S>& direction) const;

        /**
         * Projects the given vector originating at the anchor point onto this plane along the plane normal.
         *
         * @param vector the vector to project
         * @return the projected vector
         */
        vec<T,S> projectVector(const vec<T,S>& vector) const;

        /**
         * Projects the given vector originating at the anchor point onto this plane along the given direction.
         *
         * @param vector the vector to project
         * @param direction the projection direction
         * @return the projected vector
         */
        vec<T,S> projectVector(const vec<T,S>& vector, const vec<T,S>& direction) const;
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
    template <typename T, size_t S>
    bool isEqual(const plane<T,S>& lhs, const plane<T,S>& rhs, T epsilon);

    /**
     * Checks if the given planes are identical.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lhs the first plane
     * @param rhs the second plane
     * @return true if the given planes are identical and false otherwise
     */
    template <typename T, size_t S>
    bool operator==(const plane<T,S>& lhs, const plane<T,S>& rhs);

    /**
     * Checks if the given planes are identical.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param lhs the first plane
     * @param rhs the second plane
     * @return false if the given planes are identical and true otherwise
     */
    template <typename T, size_t S>
    bool operator!=(const plane<T,S>& lhs, const plane<T,S>& rhs);

    /**
     * Prints a textual representation of the given plane to the given stream.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param stream the stream
     * @param plane the plane to print
     * @return the given stream
     */
    template <typename T, size_t S>
    std::ostream& operator<<(std::ostream& stream, const plane<T,S>& plane);

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
     * Note that the three points must not be colinear so as to be a valid three point representation of a plane. *
     *
     * @tparam T the component type
     * @param p1 the first plane point
     * @param p2 the second plane point
     * @param p3 the third plane point
     * @param epsilon an epsilon value used to determine whether the given three points do not define a plane
     * @return a pair of a boolean indicating whether the plane is valid, and the normal
     */
    template <typename T>
    std::tuple<bool, vec<T,3>> planeNormal(const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3, T epsilon = Constants<T>::angleEpsilon());

    /**
     * Creates a new plane from the given plane in three point form. Thereby, the orientation of the plane is derived
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
     * Note that the three points must not be colinear so as to be a valid three point representation of a plane.
     *
     * @param p1 the first point
     * @param p2 the second point
     * @param p3 the third point
     * @return a pair of a boolean indicating whether the plane is valid, and the plane itself
     */
    template <typename T>
    std::tuple<bool, plane<T,3>> fromPoints(const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3);

    /**
     * Creates a new plane from a plane in three point form using the first three points in the given range. Thereby, the
     * orientation of the plane is derived as follows. Given the following diagram of the given points:
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
     * Note that the three points must not be colinear so as to be a valid three point representation of a plane.
     * Also note that if the given range does not contain at least three points, this function returns a negative result.
     *
     * @tparam I the range iterator type
     * @tparam G a function that maps a range element to a point
     * @param cur the start of the range
     * @param end the end of the range
     * @param get the mapping function
     * @return a pair of a boolean indicating whether the plane is valid, and the plane itself
     */
    template <typename I, typename G = Identity>
    auto fromPoints(I cur, I end, const G& get = G()) -> std::tuple<bool, plane<typename std::remove_reference<decltype(get(*cur))>::type::type,3>>;

    /**
     * Creates a plane with the given point as its anchor and the positive Z axis as its normal.
     *
     * @tparam T the component type
     * @param position the position of the plane
     * @return the plane
     */
    template <typename T>
    plane<T,3> horizontalPlane(const vec<T, 3> &position);

    /**
     * Creates a plane at the given position and with its normal set to the normalized direction.
     *
     * @tparam T the component type
     * @param position the position of the plane
     * @param direction the direction to derive the normal from
     * @return the plane
     */
    template <typename T>
    plane<T,3> orthogonalPlane(const vec<T, 3> &position, const vec<T, 3> &direction);

    /**
     * Creates a plane at the given position and with its normal set to the major axis of the given direction.
     *
     * @tparam T the component type
     * @param position the position of the plane
     * @param direction the direction to derive the normal from
     * @return the plane
     */
    template <typename T>
    plane<T,3> alignedOrthogonalPlane(const vec<T, 3> &position, const vec<T, 3> &direction);
}

#endif
