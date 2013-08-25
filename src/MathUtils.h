/*
Copyright (C) 2010-2012 Kristian Duske

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
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TrenchBroom_MathUtils_h
#define TrenchBroom_MathUtils_h

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace Math {
    template <typename T>
    class Constants {
    public:
        static const T AlmostZero;
        static const T PointStatusEpsilon;
        static const T CorrectEpsilon;
        static const T ColinearEpsilon;
        static const T Pi;
        static const T TwoPi;
        static const T PiOverTwo;
        static const T PiOverFour;
        static const T PiOverStraightAngle;
        static const T StraightAngleOverPi;
        static const T E;
    };
    
    template <typename T>
    const T Constants<T>::AlmostZero           = static_cast<T>(0.001);
    template <typename T>
    const T Constants<T>::PointStatusEpsilon   = static_cast<T>(0.01);
    template <typename T>
    const T Constants<T>::CorrectEpsilon       = static_cast<T>(0.001); // this is what QBSP uses
    template <typename T>
    const T Constants<T>::ColinearEpsilon      = static_cast<T>(0.01);
    template <typename T>
    const T Constants<T>::Pi                   = static_cast<T>(3.141592653589793);
    template <typename T>
    const T Constants<T>::TwoPi                = static_cast<T>(2.0) * Pi;
    template <typename T>
    const T Constants<T>::PiOverTwo            = Pi / static_cast<T>(2.0);
    template <typename T>
    const T Constants<T>::PiOverFour           = Pi / static_cast<T>(4.0);
    template <typename T>
    const T Constants<T>::PiOverStraightAngle  = Pi / static_cast<T>(180.0);
    template <typename T>
    const T Constants<T>::StraightAngleOverPi  = static_cast<T>(180.0) / Pi;
    template <typename T>
    const T Constants<T>:: E                   = static_cast<T>(2.718281828459045);

    template <typename T>
    inline bool isnan(const T f) {
#ifdef _MSC_VER
        return _isnan(f) != 0;
#else
        return std::isnan(f);
#endif
    }
    
    template <typename T>
    inline T nan() {
        return std::numeric_limits<T>::quiet_NaN();
    }
    
    template <typename T>
    inline T radians(const T d) {
        return d * Constants<T>::PiOverStraightAngle;
    }

    template <typename T>
    inline T degrees(const T r) {
        return r * Constants<T>::StraightAngleOverPi;
    }
    
    template <typename T>
    inline T round(const T v) {
        return v > 0.0 ? std::floor(v + static_cast<T>(0.5)) : std::ceil(v - static_cast<T>(0.5));
    }
    
    template <typename T>
    inline T correct(const T v, const T epsilon = Constants<T>::CorrectEpsilon) {
        const T r = round(v);
        if (std::abs(v - r) <= epsilon)
            return r;
        return v;
    }

    template <typename T>
    inline bool zero(const T v, const T epsilon = Constants<T>::AlmostZero) {
        return std::abs(v) <= epsilon;
    }
    
    template <typename T>
    inline bool pos(const T v, const T epsilon = Constants<T>::AlmostZero) {
        return v > epsilon;
    }
    
    template <typename T>
    inline bool neg(const T v, const T epsilon = Constants<T>::AlmostZero) {
        return v < -epsilon;
    }

    template <typename T>
    inline bool relEq(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        const T absA = std::abs(v1);
        const T absB = std::abs(v2);
        const T diff = std::abs(v1 - v2);
        
        if (v1 == v2) { // shortcut, handles infinities
            return true;
        } else if (v1 == 0.0 || v2 == 0.0 || diff < std::numeric_limits<T>::min()) {
            // a or b is zero or both are extremely close to it
            // relative error is less meaningful here
            return diff < (epsilon * std::numeric_limits<T>::min());
        } else { // use relative error
            return diff / (absA + absB) < epsilon;
        }
    }

    template <typename T>
    inline bool eq(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return std::abs(v1 - v2) < epsilon;
    }
    
    template <typename T>
    inline bool gt(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return v1 > v2 + epsilon;
    }
    
    template <typename T>
    inline bool lt(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return v1 < v2 - epsilon;
    }
    
    template <typename T>
    inline bool gte(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return !lt(v1, v2, epsilon);
    }
    
    template <typename T>
    inline bool lte(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return !gt(v1, v2, epsilon);
    }

    template <typename T>
    inline bool between(const T v, const T s, const T e, const T epsilon = Constants<T>::AlmostZero) {
        if (eq(v, s, epsilon) || eq(v, e, epsilon))
            return true;
        if (lt(s, e, epsilon))
            return gt(v, s, epsilon) && lt(v, e, epsilon);
        return gt(v, e, epsilon) && lt(v, s, epsilon);
    }
    
    template <typename T>
    inline T selectMin(const T v1, const T v2) {
        if (isnan(v1))
            return v2;
        if (isnan(v2))
            return v1;
        return std::min(v1, v2);
    }

    inline size_t succ(size_t index, size_t count, size_t offset = 1) {
        return (index + offset) % count;
    }
    
    inline size_t pred(size_t index, size_t count, size_t offset = 1) {
        return ((index + count) - (offset % count)) % count;
    }
    
    namespace Axis {
        typedef size_t Type;
        static const Type AX = 0;
        static const Type AY = 1;
        static const Type AZ = 2;
    }
    
    namespace PointStatus {
        typedef size_t Type;
        static const Type PSAbove = 0;
        static const Type PSBelow = 1;
        static const Type PSInside = 2;
    }
};

#endif
