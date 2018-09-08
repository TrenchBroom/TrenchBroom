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

#ifndef TRENCHBROOM_PLANE_IMPL_H
#define TRENCHBROOM_PLANE_IMPL_H

#include "plane_decl.h"
#include "vec_decl.h"
#include "vec_impl.h"

namespace vm {
    template <typename T, size_t S>
    plane<T,S>::plane() :
    distance(static_cast<T>(0.0)),
    normal(vec<T,S>::zero) {}

    template <typename T, size_t S>
    plane<T,S>::plane(const T i_distance, const vec<T,S>& i_normal) :
    distance(i_distance),
    normal(i_normal) {}

    template <typename T, size_t S>
    plane<T,S>::plane(const vec<T,S>& i_anchor, const vec<T,S>& i_normal) :
    distance(dot(i_anchor, i_normal)),
    normal(i_normal) {}

    template <typename T, size_t S>
    vec<T,S> plane<T,S>::anchor() const {
        return normal * distance;
    }

    template <typename T, size_t S>
    T plane<T,S>::at(const vec<T,S-1>& point, const Math::Axis::Type axis) const {
        if (Math::zero(normal[axis])) {
            return static_cast<T>(0.0);
        }

        auto t = static_cast<T>(0.0);
        size_t index = 0;
        for (size_t i = 0; i < S; i++) {
            if (i != axis) {
                t += normal[i] * point[index++];
            }
        }
        return (distance - t) / normal[axis];
    }

    template <typename T, size_t S>
    T plane<T,S>::xAt(const vec<T,S-1>& point) const {
        return at(point, Math::Axis::AX);
    }

    template <typename T, size_t S>
    T plane<T,S>::yAt(const vec<T,S-1>& point) const {
        return at(point, Math::Axis::AY);
    }

    template <typename T, size_t S>
    T plane<T,S>::zAt(const vec<T,S-1>& point) const {
        return at(point, Math::Axis::AZ);
    }

    template <typename T, size_t S>
    T plane<T,S>::pointDistance(const vec<T,S>& point) const {
        return dot(point, normal) - distance;
    }

    template <typename T, size_t S>
    Math::PointStatus::Type plane<T,S>::pointStatus(const vec<T,S>& point, const T epsilon) const {
        const auto dist = pointDistance(point);
        if (dist >  epsilon) {
            return Math::PointStatus::PSAbove;
        } else if (dist < -epsilon) {
            return Math::PointStatus::PSBelow;
        } else {
            return Math::PointStatus::PSInside;
        }
    }

    template <typename T, size_t S>
    plane<T,S> plane<T,S>::flip() const {
        // Distance must also be flipped to compensate for the changed sign of the normal. The location of the plane does not change!
        return plane<T,S>(-distance, -normal);
    }

    template <typename T, size_t S>
    plane<T,S> plane<T,S>::transform(const mat<T,S+1,S+1>& transform) const {
        const auto newNormal   = normalize(stripTranslation(transform) * normal);
        const auto newDistance = dot(transform * anchor(), newNormal);
        return plane<T,S>(newDistance, newNormal);
    }

    template <typename T, size_t S>
    vec<T,S> plane<T,S>::projectPoint(const vec<T,S>& point) const {
        return point - dot(point, normal) * normal + distance * normal;
    }

    template <typename T, size_t S>
    vec<T,S> plane<T,S>::projectPoint(const vec<T,S>& point, const vec<T,S>& direction) const {
        const auto cos = dot(direction, normal);
        if (Math::zero(cos)) {
            return vec<T,S>::NaN;
        }
        const auto d = dot(distance * normal - point, normal) / cos;
        return point + direction * d;
    }

    template <typename T, size_t S>
    vec<T,S> plane<T,S>::projectVector(const vec<T,S>& vector) const {
        return projectPoint(anchor() + vector) - anchor();
    }

    template <typename T, size_t S>
    vec<T,S> plane<T,S>::projectVector(const vec<T,S>& vector, const vec<T,S>& direction) const {
        return projectPoint(anchor() + vector, direction) - anchor();
    }

    template <typename T, size_t S>
    bool operator==(const plane<T,S>& lhs, const plane<T,S>& rhs) {
        return lhs.distance == rhs.distance && lhs.normal == rhs.normal;
    }

    template <typename T, size_t S>
    bool operator!=(const plane<T,S>& lhs, const plane<T,S>& rhs) {
        return lhs.distance != rhs.distance || lhs.normal != rhs.normal;
    }

    template <typename T, size_t S>
    std::ostream& operator<<(std::ostream& stream, const plane<T,S>& plane) {
        stream << "{ normal: (" << plane.normal << ") distance: " << plane.distance << " }";
        return stream;
    }

    template <typename T, size_t S>
    bool equal(const plane<T,S>& lhs, const plane<T,S>& rhs, const T epsilon) {
        return Math::eq(lhs.distance, rhs.distance, epsilon) && equal(lhs.normal, rhs.normal, epsilon);
    }

    template <typename T>
    std::tuple<bool, vec<T,3>> planeNormal(const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3, const T epsilon) {
        const auto v1 = p3 - p1;
        const auto v2 = p2 - p1;
        const auto normal = cross(v1, v2);

        // Fail if v1 and v2 are parallel, opposite, or either is zero-length.
        // Rearranging "A cross B = ||A|| * ||B|| * sin(theta) * n" (n is a unit vector perpendicular to A and B) gives
        // sin_theta below.
        const auto sin_theta = Math::abs(length(normal) / (length(v1) * length(v2)));
        if (Math::isnan(sin_theta) ||
            Math::isinf(sin_theta) ||
            sin_theta < epsilon) {
            return std::make_tuple(false, vec<T,3>::zero);
        } else {
            return std::make_tuple(true, normalize(normal));
        }
    }

    template <typename T>
    std::tuple<bool, plane<T,3>> fromPoints(const vec<T,3>& p1, const vec<T,3>& p2, const vec<T,3>& p3) {
        const auto [valid, normal] = planeNormal(p1, p2, p3);
        if (!valid) {
            return std::make_tuple(false, plane<T,3>());
        } else {
            return std::make_tuple(true, plane<T,3>(p1, normal));
        }
    }

    template <typename I, typename G>
    auto fromPoints(I cur, I end, const G& get) -> std::tuple<bool, plane<typename std::remove_reference<decltype(get(*cur))>::type::type,3>> {
        using T = typename std::remove_reference<decltype(get(*cur))>::type::type;

        if (cur == end) {
            return std::make_tuple(false, plane<T,3>());
        }
        const auto p1 = *cur; ++cur;
        if (cur == end) {
            return std::make_tuple(false, plane<T,3>());
        }
        const auto p2 = *cur; ++cur;
        if (cur == end) {
            return std::make_tuple(false, plane<T,3>());
        }
        const auto p3 = *cur;

        return fromPoints(p1, p2, p3);
    }

    template <typename T>
    plane<T,3> horizontalPlane(const vec<T, 3> &position) {
        return plane<T,3>(position, vec<T,3>::pos_z);
    }

    template <typename T>
    plane<T,3> orthogonalPlane(const vec<T, 3> &position, const vec<T, 3> &direction) {
        return plane<T,3>(position, normalize(direction));
    }

    template <typename T>
    plane<T,3> alignedOrthogonalPlane(const vec<T, 3> &position, const vec<T, 3> &direction) {
        return plane<T,3>(position, firstAxis(direction));
    }
}

#endif //TRENCHBROOM_PLANE_IMPL_H
