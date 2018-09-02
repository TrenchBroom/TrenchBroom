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

#ifndef TRENCHBROOM_QUAT_IMPL_H
#define TRENCHBROOM_QUAT_IMPL_H

#include "quat_decl.h"

template <typename T>
quat<T>::quat() :
r(static_cast<T>(0.0)),
v(vec<T,3>::zero) {}

template <typename T>
quat<T>::quat(const T i_r, const vec<T,3>& i_v) :
r(i_r),
v(i_v) {}

template <typename T>
quat<T>::quat(const vec<T,3>& axis, const T angle) {
    setRotation(axis, angle);
}

template <typename T>
quat<T>::quat(const vec<T,3>& from, const vec<T,3>& to) {
    assert(isUnit(from));
    assert(isUnit(to));

    const auto cos = dot(from, to);
    if (Math::one(cos)) {
        // `from` and `to` are equal.
        setRotation(vec<T,3>::pos_z, static_cast<T>(0.0));
    } else if (Math::one(-cos)) {
        // `from` and `to` are opposite.
        // We need to find a rotation axis that is perpendicular to `from`.
        auto axis = cross(from, vec<T,3>::pos_z);
        if (Math::zero(squaredLength(axis))) {
            axis = cross(from, vec<T,3>::pos_x);
        }
        setRotation(normalize(axis), Math::radians(static_cast<T>(180)));
    } else {
        const auto axis = normalize(cross(from, to));
        const auto angle = std::acos(cos);
        setRotation(axis, angle);
    }
}

template <typename T>
void quat<T>::setRotation(const vec<T,3>& axis, const T angle) {
    assert(isUnit(axis));
    r = std::cos(angle / static_cast<T>(2.0));
    v = axis * std::sin(angle / static_cast<T>(2.0));
}

template <typename T>
T quat<T>::angle() const {
    return static_cast<T>(2.0) * std::acos(r);
}

template <typename T>
vec<T,3> quat<T>::axis() const {
    if (isZero(v)) {
        return v;
    } else {
        return v / std::sin(angle() / static_cast<T>(2.0));
    }
}

template <typename T>
quat<T> quat<T>::conjugate() const {
    return quat<T>(r, -v);
}

template <typename T>
quat<T> operator-(const quat<T>& q) {
    return quat<T>(-q.r, q.v);
}

template <typename T>
quat<T> operator*(const quat<T> lhs, const T rhs) {
    return quat<T>(lhs.r * rhs, lhs.v);
}


template <typename T>
quat<T> operator*(const T lhs, const quat<T>& rhs) {
    return quat<T>(lhs * rhs.r, rhs.v);
}

template <typename T>
quat<T> operator*(const quat<T>& lhs, const quat<T>& rhs) {
    const auto nr = lhs.r * rhs.r - dot(lhs.v, rhs.v);
    const auto nx = lhs.r * rhs.v.x() + lhs.v.x() * rhs.r + lhs.v.y() * rhs.v.z() - lhs.v.z() * rhs.v.y();
    const auto ny = lhs.r * rhs.v.y() + lhs.v.y() * rhs.r + lhs.v.z() * rhs.v.x() - lhs.v.x() * rhs.v.z();
    const auto nz = lhs.r * rhs.v.z() + lhs.v.z() * rhs.r + lhs.v.x() * rhs.v.y() - lhs.v.y() * rhs.v.x();

    return quat<T>(nr, vec<T,3>(nx, ny, nz));
}

template <typename T>
vec<T,3> operator*(const quat<T>& lhs, const vec<T,3>& rhs) {
    return (lhs * quat<T>(static_cast<T>(0.0), rhs) * lhs.conjugate()).v;
}

#endif //TRENCHBROOM_QUAT_IMPL_H
