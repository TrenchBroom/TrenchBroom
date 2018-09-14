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

#ifndef TRENCHBROOM_QUAT_DECL_H
#define TRENCHBROOM_QUAT_DECL_H

#include "vec.h"
#include "scalar.h"

#include <cassert>

namespace vm {
    template <typename T>
    class quat {
    public:
        /**
         * The real component.
         */
        T r;

        /**
         * The imaginary component.
         */
        vec<T,3> v;

        /**
         * Creates a new quaternion initialized to 0.
         */
        quat() :
        r(T(0.0)),
        v(vec<T,3>::zero) {}

        // Copy and move constructors
        quat(const quat<T>& other) = default;
        quat(quat<T>&& other) = default;

        // Assignment operators
        quat<T>& operator=(const quat<T>& other) = default;
        quat<T>& operator=(quat<T>&& other) = default;

        /**
         * Converts the given quaternion by converting its members to this quaternion's component type.
         *
         * @tparam U the component type of the quaternion to convert
         * @param other the quaternion to convert
         */
        template <typename U>
        quat(const quat<U>& other) :
        r(T(other.r)),
        v(other.v) {}

        /**
         * Creates a new quaternion with the given real and imaginary components.
         *
         * @param i_r the real component
         * @param i_v the imaginary components
         */
        quat(const T i_r, const vec<T,3>& i_v) :
        r(i_r),
        v(i_v) {}


        /**
         * Creates a new quaternion that represent a clounter-clockwise rotation by the given angle (in radians) about
         * the given axis.
         *
         * @param axis the rotation axis
         * @param angle the rotation angle (in radians)
         */
        quat(const vec<T,3>& axis, const T angle) {
            setRotation(axis, angle);
        }

        /**
         * Creates a new quaternion that rotates the 1st given vector onto the 2nd given vector. Both vectors are
         * expected to be normalized.
         *
         * @param from the vector to rotate
         * @param to the vector to rotate onto
         */
        quat(const vec<T,3>& from, const vec<T,3>& to) {
            assert(isUnit(from));
            assert(isUnit(to));

            const auto cos = dot(from, to);
            if (isEqual(+cos, T(1.0))) {
                // `from` and `to` are equal.
                setRotation(vec<T,3>::pos_z, T(0.0));
            } else if (isEqual(-cos, T(1.0))) {
                // `from` and `to` are opposite.
                // We need to find a rotation axis that is perpendicular to `from`.
                auto axis = cross(from, vec<T,3>::pos_z);
                if (isZero(squaredLength(axis))) {
                    axis = cross(from, vec<T,3>::pos_x);
                }
                setRotation(normalize(axis), radians(T(180)));
            } else {
                const auto axis = normalize(cross(from, to));
                const auto angle = std::acos(cos);
                setRotation(axis, angle);
            }
        }
    private:
        void setRotation(const vec<T,3>& axis, const T angle) {
            assert(isUnit(axis));
            r = std::cos(angle / T(2.0));
            v = axis * std::sin(angle / T(2.0));
        }
    public:
        /**
         * Returns the angle by which this quaternion would rotate a vector.
         *
         * @return the rotation angle in radians
         */
        T angle() const {
            return T(2.0) * std::acos(r);
        }

        /**
         * Returns the rotation axis of this quaternion.
         *
         * @return the rotation axis
         */
        vec<T,3> axis() const {
            if (isZero(v)) {
                return v;
            } else {
                return v / std::sin(std::acos(r));
            }
        }

        /**
         * Conjugates this quaternion by negating its imaginary components.
         *
         * @return the conjugated quaternion
         */
        quat<T> conjugate() const {
            return quat<T>(r, -v);
        }
    };

    /**
     * Negates this quaternion by negating its real component.
     *
     * @tparam T the component type
     * @param q the quaternion to negate
     * @return the negated quaternion
     */
    template <typename T>
    quat<T> operator-(const quat<T>& q) {
        return quat<T>(-q.r, q.v);
    }

    /**
     * Multiplies the given quaternion with the given scalar value.
     *
     * @tparam T the component type
     * @param lhs the quaternion
     * @param rhs the scalar
     * @return the multiplied quaternion
     */
    template <typename T>
    quat<T> operator*(const quat<T> lhs, const T rhs) {
        return quat<T>(lhs.r * rhs, lhs.v);
    }

    /**
     * Multiplies the given quaternion with the given scalar value.
     *
     * @tparam T the component type
     * @param lhs the scalar
     * @param rhs the quaternion
     * @return the multiplied quaternion
     */
    template <typename T>
    quat<T> operator*(const T lhs, const quat<T>& rhs) {
        return quat<T>(lhs * rhs.r, rhs.v);
    }

    /**
     * Multiplies the given quaternions.
     *
     * @tparam T the component type
     * @param lhs the first quaternion
     * @param rhs the second quaternion
     * @return the product of the given quaternions.
     */
    template <typename T>
    quat<T> operator*(const quat<T>& lhs, const quat<T>& rhs) {
        const auto nr = lhs.r * rhs.r - dot(lhs.v, rhs.v);
        const auto nx = lhs.r * rhs.v.x() + lhs.v.x() * rhs.r + lhs.v.y() * rhs.v.z() - lhs.v.z() * rhs.v.y();
        const auto ny = lhs.r * rhs.v.y() + lhs.v.y() * rhs.r + lhs.v.z() * rhs.v.x() - lhs.v.x() * rhs.v.z();
        const auto nz = lhs.r * rhs.v.z() + lhs.v.z() * rhs.r + lhs.v.x() * rhs.v.y() - lhs.v.y() * rhs.v.x();

        return quat<T>(nr, vec<T,3>(nx, ny, nz));
    }

    /**
     * Applies the given quaternion to the given vector, in effect rotating it.
     *
     * @tparam T the component type
     * @param lhs the quaternion
     * @param rhs the vector
     * @return the rotated vector
     */
    template <typename T>
    vec<T,3> operator*(const quat<T>& lhs, const vec<T,3>& rhs) {
        return (lhs * quat<T>(T(0.0), rhs) * lhs.conjugate()).v;
    }
}

#endif
