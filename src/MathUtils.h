/*
Copyright (C) 2010-2014 Kristian Duske

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
        static const T ThreePiOverTwo;
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
    const T Constants<T>::ThreePiOverTwo       = Pi * static_cast<T>(3.0 / 2.0);
    template <typename T>
    const T Constants<T>::PiOverStraightAngle  = Pi / static_cast<T>(180.0);
    template <typename T>
    const T Constants<T>::StraightAngleOverPi  = static_cast<T>(180.0) / Pi;
    template <typename T>
    const T Constants<T>:: E                   = static_cast<T>(2.718281828459045);

    typedef Constants<double> Cd;
    typedef Constants<float> Cf;
    
    template <typename T>
    bool isnan(const T f) {
#ifdef _MSC_VER
        return _isnan(f) != 0;
#else
        return std::isnan(f);
#endif
    }
    
    template <typename T>
    T nan() {
        return std::numeric_limits<T>::quiet_NaN();
    }
    
    template <typename T>
    T abs(const T v) {
        return std::abs(v);
    }
    
    template <typename T>
    T mod(const T v1, const T v2) {
        return std::fmod(v1, v2);
    }
    
    template <typename T>
    T remainder(const T v1, const T v2) {
        const T n = round(v1 / v2);
        return v1 - n * v2;
    }
    
    template <typename T>
    T min(const T v1, const T v2) {
        return std::min(v1, v2);
    }
    
    template <typename T>
    T max(const T v1, const T v2) {
        return std::max(v1, v2);
    }

    template <typename T>
    T radians(const T d) {
        return d * Constants<T>::PiOverStraightAngle;
    }

    template <typename T>
    T degrees(const T r) {
        return r * Constants<T>::StraightAngleOverPi;
    }
    
    template <typename T>
    T floor(const T v) {
        return std::floor(v);
    }
    
    template <typename T>
    T ceil(const T v) {
        return std::ceil(v);
    }
    
    template <typename T>
    T round(const T v) {
        return v > 0.0 ? floor(v + static_cast<T>(0.5)) : ceil(v - static_cast<T>(0.5));
    }
    
    template <typename T>
    T correct(const T v, const T epsilon = Constants<T>::CorrectEpsilon) {
        const T r = round(v);
        if (abs(v - r) <= epsilon)
            return r;
        return v;
    }

    template <typename T>
    bool zero(const T v, const T epsilon = Constants<T>::AlmostZero) {
        return abs(v) <= epsilon;
    }
    
    template <typename T>
    bool pos(const T v, const T epsilon = Constants<T>::AlmostZero) {
        return v > epsilon;
    }
    
    template <typename T>
    bool neg(const T v, const T epsilon = Constants<T>::AlmostZero) {
        return v < -epsilon;
    }

    template <typename T>
    bool relEq(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        const T absA = abs(v1);
        const T absB = abs(v2);
        const T diff = abs(v1 - v2);
        
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
    bool eq(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return abs(v1 - v2) < epsilon;
    }
    
    template <typename T>
    bool gt(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return v1 > v2 + epsilon;
    }
    
    template <typename T>
    bool lt(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return v1 < v2 - epsilon;
    }
    
    template <typename T>
    bool gte(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return !lt(v1, v2, epsilon);
    }
    
    template <typename T>
    bool lte(const T v1, const T v2, const T epsilon = Constants<T>::AlmostZero) {
        return !gt(v1, v2, epsilon);
    }

    template <typename T>
    bool isInteger(const T v) {
        return eq(v, round(v));
    }
    
    template <typename T>
    bool between(const T v, const T s, const T e, const T epsilon = Constants<T>::AlmostZero) {
        if (eq(v, s, epsilon) || eq(v, e, epsilon))
            return true;
        if (lt(s, e, epsilon))
            return gt(v, s, epsilon) && lt(v, e, epsilon);
        return gt(v, e, epsilon) && lt(v, s, epsilon);
    }
    
    template <typename T, typename C, typename O>
    T succ(const T index, const C count, const O offset) {
        return (index + offset) % count;
    }
    
    template <typename T, typename C>
    T succ(const T index, const C count) {
        return succ(index, count, 1);
    }
    
    template <typename T, typename O>
    T pred(const T index, const T count, const O offset) {
        return ((index + count) - (offset % count)) % count;
    }
    
    template <typename T>
    T pred(const T index, const T count) {
        return pred(index, count, 1);
    }

    template <typename T>
    T clamp(const T v, const T minV = 0.0f, const T maxV = 1.0f) {
        return max(min(v, maxV), minV);
    }

    template <typename T>
    T selectMin(const T v1, const T v2) {
        if (isnan(v1))
            return v2;
        if (isnan(v2))
            return v1;
        return min(v1, v2);
    }
    
    template <typename T>
    T nextPOT(T n) {
        // see https://en.wikipedia.org/wiki/Power_of_two
        if (!(n & (n-1)))
            return n;
        
        
        while (n & (n-1))
            n = n & (n-1);
        
        n = n << 1;
        return n;
    }
    
    template <typename T, bool Abs>
    struct Cmp {
        Cmp() {}
        int operator()(const T lhs, const T rhs) const {
            const T l = Abs ? abs(lhs) : lhs;
            const T r = Abs ? abs(rhs) : rhs;
            if (Math::lt(l, r))
                return -1;
            if (Math::gt(l, r))
                return 1;
            return 0;
        }
    };
    
    template <typename T, bool Abs>
    struct Less {
    private:
        Cmp<T, Abs> m_cmp;
    public:
        Less() {}
        bool operator()(const T lhs, const T rhs) const {
            return m_cmp(lhs, rhs) < 0;
        }
    };
    
    typedef enum {
        DForward,
        DBackward,
        DLeft,
        DRight,
        DUp,
        DDown
    } Direction;

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
}

#endif
