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

#ifndef TRENCHBROOM_ABSTRACT_LINE_H
#define TRENCHBROOM_ABSTRACT_LINE_H

#include <vecmath/vec.h>

#include <cstddef>

namespace vm {
    /**
     * An abstract line is a generalization of lines in space. Thereby, a line can be bounded or unbounded in either
     * direction. The following concepts arise:
     *
     * - If the line is unbounded in both directions, it is just that, a line.
     * - If the line is bounded in one direction, it is a ray.
     * - If the line is bounded in both directions, it is a segment.
     *
     * @tparam T the component type
     * @tparam S the number of components
     */
    template <typename T, size_t S>
    class abstract_line {
    protected:
        abstract_line() {}

        abstract_line(const abstract_line<T,S>& other) = default;
        abstract_line(abstract_line<T,S>&& other) noexcept = default;

        abstract_line<T,S>& operator=(const abstract_line<T,S>& other) = default;
        abstract_line<T,S>& operator=(abstract_line<T,S>&& other) noexcept = default;
    public:
        virtual ~abstract_line() {}

        /**
         * Returns the origin of this line.
         *
         * @return the origin
         */
        virtual vec<T,S> getOrigin() const = 0;

        /**
         * Returns the direction of this line.
         *
         * @return the direction
         */
        virtual vec<T,S> getDirection() const = 0;

        /**
         * Computes the distance from the origin to the orthogonal projection of the given point onto the direction of this
         * line.
         *
         * @param point the point to project
         * @return the distance from the origin to the orthogonal projection of the given point
         */
        T distanceToProjectedPoint(const vec<T,S>& point) const {
            return dot(point - getOrigin(), getDirection());
        }

        /**
         * Computes the point on this line at the given distance from the ray origin.
         *
         * @param distance the distance of the point
         * @return the point
         */
        vec<T,S> pointAtDistance(const T distance) const {
            return getOrigin() + getDirection() * distance;
        }

        /**
         * Orthogonally projects the given point onto this line.
         *
         * @param i_point the point to project
         * @return the projected point
         */
        vec<T,S> projectPoint(const vec<T,S>& i_point) const {
            return pointAtDistance(distanceToProjectedPoint(i_point));
        }
    };
}

#endif //TRENCHBROOM_ABSTRACT_LINE_H
