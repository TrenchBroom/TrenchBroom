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

#ifndef TRENCHBROOM_RAY_IMPL_H
#define TRENCHBROOM_RAY_IMPL_H

#include "ray_decl.h"
#include "vec_decl.h"
#include "vec_impl.h"
#include "mat_decl.h"
#include "mat_impl.h"

#include "vecmath/utils.h"

#include <cstddef>

namespace vm {
    template <typename T, size_t S>
    ray<T,S>::ray() :
    origin(vec<T,S>::zero),
    direction(vec<T,S>::zero) {}

    template <typename T, size_t S>
    ray<T,S>::ray(const vec<T,S>& i_origin, const vec<T,S>& i_direction) :
    origin(i_origin),
    direction(i_direction) {}

    template <typename T, size_t S>
    vec<T,S> ray<T,S>::getOrigin() const {
        return origin;
    }

    template <typename T, size_t S>
    vec<T,S> ray<T,S>::getDirection() const {
        return direction;
    }

    template <typename T, size_t S>
    ray<T,S> ray<T,S>::transform(const mat<T,S+1,S+1>& transform) const {
        const auto newOrigin = origin * transform;
        const auto newDirection = direction * stripTranslation(transform);
        return ray<T,S>(newOrigin, newDirection);
    }

    template <typename T, size_t S>
    point_status ray<T,S>::pointStatus(const vec<T,S>& point) const {
        const auto scale = dot(direction, point - origin);
        if (scale >  constants<T>::pointStatusEpsilon()) {
            return point_status::above;
        } else if (scale < -constants<T>::pointStatusEpsilon()) {
            return point_status::below;
        } else {
            return point_status::inside;
        }
    }

    template <typename T, size_t S>
    bool isEqual(const ray<T,S>& lhs, const ray<T,S>& rhs, const T epsilon) {
        return isEqual(lhs.origin, rhs.origin, epsilon) && isEqual(lhs.direction, rhs.direction, epsilon);
    }

    template <typename T, size_t S>
    bool operator==(const ray<T,S>& lhs, const ray<T,S>& rhs) {
        return lhs.origin == rhs.origin && lhs.direction == rhs.direction;
    }

    template <typename T, size_t S>
    bool operator!=(const ray<T,S>& lhs, const ray<T,S>& rhs) {
        return lhs.origin != rhs.origin || lhs.direction != rhs.direction;
    }

    template <typename T, size_t S>
    std::ostream& operator<<(std::ostream& stream, const ray<T,S>& ray) {
        stream << "{ origin: (" << ray.origin << "), direction: (" << ray.direction << ") }";
        return stream;
    }
}

#endif //TRENCHBROOM_RAY_IMPL_H
