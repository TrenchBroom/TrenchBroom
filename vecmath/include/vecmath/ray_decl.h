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

#ifndef TRENCHBROOM_RAY_DECL_H
#define TRENCHBROOM_RAY_DECL_H

#include "abstract_line.h"
#include "vec_decl.h"
#include "mat_decl.h"

#include "vecmath/utils.h"

namespace vm {
    /**
     * A ray, represented by the origin and direction.
     *
     * @tparam T the component type
     * @tparam S the number of components
     */
    template <typename T, size_t S>
    class ray : public abstract_line<T,S> {
    public:
        vec<T,S> origin;
        vec<T,S> direction;

        /**
         * Creates a new ray with all components initialized to 0.
         */
        ray();

        // Copy and move constructors
        ray(const ray<T,S>& other) = default;
        ray(ray<T,S>&& other) noexcept = default;

        // Assignment operators
        ray<T,S>& operator=(const ray<T,S>& other) = default;
        ray<T,S>& operator=(ray<T,S>&& other) noexcept = default;

        /**
         * Creates a new ray by copying the values from the given ray. If the given ray has a different component
         * type, the values are converted using static_cast.
         *
         * @tparam U the component type of the given ray
         * @param other the ray to copy the values from
         */
        template <typename U>
        explicit ray(const ray<U,S>& other) :
        origin(vec<T,S>(other.origin)),
        direction(vec<T,S>(other.direction)) {}

        /**
         * Creates a new ray with the given origin and direction.
         *
         * @param i_origin the origin
         * @param i_direction the direction
         */
        ray(const vec<T,S>& i_origin, const vec<T,S>& i_direction);

        // implement abstract_line interface
        vec<T,S> getOrigin() const override;
        vec<T,S> getDirection() const override;

        /**
         * Transforms this line using the given transformation matrix. The translational part is not applied to the
         * direction.
         *
         * @param transform the transformation to apply
         * @return the transformed ray
         */
        ray<T,S> transform(const mat<T,S+1,S+1>& transform) const;

        /**
         * Determines the position of the given point in relation to the origin and direction of this ray. Suppose that the
         * ray determines a plane that splits the space into two half spaces. The plane position is determined byhte origin
         * and the plane normal is identical to the ray direction. Then the return value indicates one of three situations:
         *
         * - The given point is in the half space above the plane, or in direction of the ray.
         * - The given point is in the half space below the plane, or in the opposite direction of the ray.
         * - The given point is in neither half space, or it is exactly on the plane.
         *
         * @param point the point to check
         * @return a value indicating the relative position of the given point
         */
        point_status pointStatus(const vec<T,S>& point) const;
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
    bool isEqual(const ray<T,S>& lhs, const ray<T,S>& rhs, T epsilon);

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
    bool operator==(const ray<T,S>& lhs, const ray<T,S>& rhs);

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
    bool operator!=(const ray<T,S>& lhs, const ray<T,S>& rhs);

    /**
     * Prints a textual representation of the given ray on the given stream.
     *
     * @tparam T the component type
     * @tparam S the number of components
     * @param stream the stream to print to
     * @param ray the ray to print
     * @return the given stream
     */
    template <typename T, size_t S>
    std::ostream& operator<<(std::ostream& stream, const ray<T,S>& ray);
}

#endif
