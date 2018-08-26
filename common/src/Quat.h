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

#ifndef TrenchBroom_Quat_h
#define TrenchBroom_Quat_h

#include "MathUtils.h"
#include "vec_type.h"

#include <cassert>

template <typename T>
class Quat {
public:
    T r;
    vec<T,3> v;
    
    Quat() :
    r(static_cast<float>(0.0)),
    v(vec<T,3>::zero) {}
    
    // Copy and move constructors
    Quat(const Quat<T>& other) = default;
    Quat(Quat<T>&& other) = default;
    
    // Assignment operators
    Quat<T>& operator=(const Quat<T>& other) = default;
    Quat<T>& operator=(Quat<T>&& other) = default;
    
    // Conversion constructor
    template <typename U>
    Quat(const Quat<U>& other) :
    r(static_cast<T>(other.r)),
    v(other.v) {}

    Quat(const T i_r, const vec<T,3>& i_v) :
    r(i_r),
    v(i_v) {}
    
    /**
     * Creates a new quaternion that represent a clounter-clockwise rotation by the given angle (in radians) about
     * the given axis.
     */
    Quat(const vec<T,3>& axis, const T angle) {
        setRotation(axis, angle);
    }

    /**
     * Creates a new quaternion that rotates the 1st given vector onto the 2nd given vector. Both vectors are
     * expected to be normalized.
     */
    Quat(const vec<T,3>& from, const vec<T,3>& to) {
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

    const Quat<T> operator-() const {
        return Quat(-r, v);
    }
    
    const Quat<T> operator*(const T right) const {
        return Quat(r * right, v);
    }
    
    Quat<T>& operator*= (const T right) {
        r *= right;
        return *this;
    }
    
    const Quat<T> operator*(const Quat<T>& right) const {
        Quat<T> result = *this;
        return result *= right;
    }
    
    Quat<T>& operator*=(const Quat<T>& right) {
        const T& t = right.r;
        const vec<T,3>& w = right.v;
        
        const T nx = r * w.x() + t * v.x() + v.y() * w.z() - v.z() * w.y();
        const T ny = r * w.y() + t * v.y() + v.z() * w.x() - v.x() * w.z();
        const T nz = r * w.z() + t * v.z() + v.x() * w.y() - v.y() * w.x();
        
        r = r * t - dot(v, w);
        v[0] = nx;
        v[1] = ny;
        v[2] = nz;
        return *this;
    }
    
    const vec<T,3> operator*(const vec<T,3>& right) const {
        Quat<T> p;
        p.r = 0.0;
        p.v = right;
        p = *this * p * conjugated();
        return p.v;
    }
    
    Quat<T>& setRotation(const vec<T,3>& axis, const T angle) {
        assert(isUnit(axis));
        r = std::cos(angle / static_cast<T>(2.0));
        v = axis * std::sin(angle / static_cast<T>(2.0));
        return *this;
    }
    
    T angle() const {
        return static_cast<T>(2.0) * std::acos(r);
    }
    
    vec<T,3> axis() const {
        if (isNull(v)) {
            return v;
        } else {
            return v / std::sin(angle() / static_cast<T>(2.0));
        }
    }
    
    Quat<T>& conjugate() {
        v = -v;
        return *this;
    }
    
    const Quat conjugated() const {
        Quat<T> result;
        result.r = r;
        result.v = -v;
        return result;
    }
};

typedef Quat<float> Quatf;
typedef Quat<double> Quatd;

template <typename T>
Quat<T> operator*(const T left, const Quat<T>& right) {
    return Quat<T>(left * right.r, right.v);
}

#endif
