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

#ifndef TrenchBroom_MathUtils_h
#define TrenchBroom_MathUtils_h

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace Math {
    struct Identity {
        template<typename U>
        constexpr auto operator()(U&& v) const noexcept -> decltype(std::forward<U>(v)) {
            return std::forward<U>(v);
        }
    };

    template <typename T>
    class Constants {
    public:
        static T almostZero() {
            static const T value = static_cast<T>(0.001);
            return value;
        }
        
        static T pointStatusEpsilon() {
            // static const T value = static_cast<T>(0.01);
            static const T value = static_cast<T>(0.0001); // this is what tyrbsp uses
            return value;
        }
        
        static T correctEpsilon() {
            static const T value = static_cast<T>(0.001); // this is what QBSP uses
            return value;
        }
        
        static T colinearEpsilon() {
            static const T value = static_cast<T>(0.00001); // this value seems to hit a sweet spot in relation to the point status epsilon
            return value;
        }
        
        static T angleEpsilon() {
            static const T value = static_cast<T>(0.00000001); // if abs(sin()) of the angle between two vectors is less than this, they are considered to be parallel or opposite
            return value;
        }
        
        static T pi() {
            static const T value = static_cast<T>(3.141592653589793);
            return value;
        }
        
        static T twoPi() {
            static const T value = static_cast<T>(2.0) * pi();
            return value;
        }
        
        static T piOverTwo() {
            static const T value = pi() / static_cast<T>(2.0);
            return value;
        }
        
        static T piOverFour() {
            static const T value = pi() / static_cast<T>(4.0);
            return value;
        }
        
        static T threePiOverTwo() {
            static const T value = static_cast<T>(3.0) * pi() / static_cast<T>(2.0);
            return value;
        }
        
        static T piOverStraightAngle() {
            static const T value = pi() / static_cast<T>(180.0);
            return value;
        }
        
        static T straightAngleOverPi() {
            static const T value = static_cast<T>(180.0) / pi();
            return value;
        }
        
        static T e() {
            static const T value = static_cast<T>(2.718281828459045);
            return value;
        }
    };
    
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
    bool isinf(const T f) {
        return (f ==  std::numeric_limits<T>::infinity() ||
                f == -std::numeric_limits<T>::infinity());
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
    T min(const T v1, const T v2) {
        return std::min(v1, v2);
    }
    
    template <typename T>
    T max(const T v1, const T v2) {
        return std::max(v1, v2);
    }

    template <typename T>
    T absMin(const T v1, const T v2) {
        if (abs(v1) < abs(v2))
            return v1;
        return v2;
    }
    
    template <typename T>
    T absMax(const T v1, const T v2) {
        if (abs(v1) > abs(v2))
            return v1;
        return v2;
    }
    
    template <typename T>
    T absDifference(const T v1, const T v2) {
        return Math::abs(Math::abs(v1) - Math::abs(v2));
    }
    
    template <typename T>
    T radians(const T d) {
        return d * Constants<T>::piOverStraightAngle();
    }

    template <typename T>
    T degrees(const T r) {
        return r * Constants<T>::straightAngleOverPi();
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
    T integerOffset(const T v) {
        return v - round(v);
    }
    
    template <typename T>
    T down(const T v) {
        return v > 0.0 ? floor(v) : ceil(v);
    }
    
    template <typename T>
    T up(const T v) {
        return v < 0.0 ? floor(v) : ceil(v);
    }

    template <typename T>
    T snap(const T v, const T grid) {
        assert(grid > 0.0);
        return grid * Math::round(v / grid);
    }

    template <typename T>
    T correct(const T v, const size_t decimals = 0, const T epsilon = Constants<T>::correctEpsilon()) {
        const T m = static_cast<T>(1 << decimals);
        const T r = round(v * m);
        if (abs(v - r) < epsilon)
            return r / m;
        return v;
    }

    template <typename T>
    T roundDownToMultiple(const T v, const T m) {
        return down(v / m) * m;
    }
    
    template <typename T>
    T roundUpToMultiple(const T v, const T m) {
        return up(v / m) * m;
    }
    
    template <typename T>
    T roundToMultiple(const T v, const T m) {
        const T d = roundDownToMultiple(v, m);
        const T u = roundUpToMultiple(v, m);
        if (Math::abs(d - v) < Math::abs(u - v))
            return d;
        return u;
    }

    template <typename T>
    bool one(const T v, const T epsilon = Constants<T>::almostZero()) {
        return Math::abs(v - static_cast<T>(1.0)) <= epsilon;
    }
    
    template <typename T>
    bool zero(const T v, const T epsilon = Constants<T>::almostZero()) {
        return abs(v) <= epsilon;
    }
    
    template <typename T>
    bool pos(const T v, const T epsilon = Constants<T>::almostZero()) {
        return v > epsilon;
    }
    
    template <typename T>
    bool neg(const T v, const T epsilon = Constants<T>::almostZero()) {
        return v < -epsilon;
    }

    template <typename T>
    bool relEq(const T v1, const T v2, const T epsilon = Constants<T>::almostZero()) {
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
    bool eq(const T v1, const T v2, const T epsilon = Constants<T>::almostZero()) {
        return abs(v1 - v2) < epsilon;
    }
    
    template <typename T>
    bool gt(const T v1, const T v2, const T epsilon = Constants<T>::almostZero()) {
        return v1 > v2 + epsilon;
    }
    
    template <typename T>
    bool lt(const T v1, const T v2, const T epsilon = Constants<T>::almostZero()) {
        return v1 < v2 - epsilon;
    }
    
    template <typename T>
    bool gte(const T v1, const T v2, const T epsilon = Constants<T>::almostZero()) {
        return !lt(v1, v2, epsilon);
    }
    
    template <typename T>
    bool lte(const T v1, const T v2, const T epsilon = Constants<T>::almostZero()) {
        return !gt(v1, v2, epsilon);
    }

    template <typename T>
    bool isInteger(const T v) {
        return eq(v, round(v));
    }
    
    template <typename T>
    bool between(const T v, const T s, const T e, const T epsilon = Constants<T>::almostZero()) {
        if (eq(v, s, epsilon) || eq(v, e, epsilon))
            return true;
        if (lt(s, e, epsilon))
            return gt(v, s, epsilon) && lt(v, e, epsilon);
        return gt(v, e, epsilon) && lt(v, s, epsilon);
    }
   
    template <typename T>
    bool within(const T v, const T s, const T e, const T epsilon = Constants<T>::almostZero()) {
        if (eq(v, s, epsilon) || eq(v, e, epsilon))
            return true;
        if (lte(s, e, epsilon))
            return gte(v, s, epsilon) && lte(v, e, epsilon);
        return gte(v, e, epsilon) && lte(v, s, epsilon);
    }
    
    size_t succ(size_t index, size_t count, size_t offset = 1);

    size_t pred(size_t index, size_t count, size_t offset = 1);
    
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

    /**
     * Returns a bit mask where only the bits up to, but not including, the given index are set.
     *
     * @tparam T the type of the parameter and result
     * @param index the index of the lowest bit which should not be set
     * @return the bit mask
     */
    template <typename T>
    T fillMask(const size_t index) {
        static_assert(std::is_integral<T>::value, "type must be integral");
        if (index == sizeof(T)*8) {
            return ~static_cast<T>(0);
        } else {
            return (static_cast<T>(1) << index) - 1;
        }
    }

    /**
     * Returns a bit mask with only the bits in the given index range set.
     *
     * @param end the 0-based end index, exclusive
     * @param start the 0-based start index, inclusive
     * @return the bit mask
     */
    template <typename T>
    T bitMask(const size_t end, const size_t start) {
        static_assert(std::is_integral<T>::value, "type must be integral");
        const auto   endMask = fillMask<T>(end);
        const auto startMask = fillMask<T>(start);
        const auto result = endMask & ~startMask;
        return result;
    }

    /**
     * Returns a bit mask with only the bit at the given index set.
     *
     * @tparam T the type of the bit mask
     * @param index the index of the set bit
     * @return the bit mask
     */
    template <typename T>
    T bitMask(const size_t index) {
        static_assert(std::is_integral<T>::value, "type must be integral");
        return static_cast<T>(1) << index;
    }

    /**
     * Tests whether the bit of the given value at the given index is set.
     *
     * @tparam T the value type
     * @param value the value to test
     * @param index the index of the bit to test
     * @return  true if the bit at the given index is set and false otherwise
     */
    template <typename T>
    bool testBit(const T value, const size_t index) {
        static_assert(std::is_integral<T>::value, "type must be integral");
        return value & bitMask<T>(index);
    }

    /**
     * Computes the prefix of the given value up until and including the given index.
     *
     * @tparam T the type of the given value
     * @param value the value
     * @param index the index, inclusive
     * @return the prefix, i.e. all bits of the given value from the highest bit up
     *   until and including the bit at the given index
     */
    template <typename T>
    T bitPrefix(const T value, const size_t index) {
        static_assert(std::is_integral<T>::value, "type must be integral");
        const auto mask = bitMask<T>(sizeof(T)*8, index);
        return value & mask;
    }

    /**
     * Finds the highest set bit in the given value. The search starts at the given
     * index, which is 0-based from the right. The function returns the index of the
     * highest set bit which is not higher than the given index, or the number of bits
     * of type T if no such bit is found.
     *
     * If the given start index is greater than the number of bits in T, the search will
     * start at the highest bit of T regardless.
     *
     * @tparam T the type of the argument
     * @param x the value
     * @param i the 0-based index of the bit to start the search at
     * @return the 0-based index of the highest set bit or the number of bits of T of no such bit is found
     */
    template <typename T>
    size_t findHighestOrderBit(T x, size_t i = sizeof(T)*8 - 1) {
        static_assert(std::is_integral<T>::value, "x is integral");

        i = min(sizeof(T)*8 - 1, i);

        while (true) {
            const T test = static_cast<T>(1) << static_cast<T>(i);
            if (x & test) {
                return i;
            }
            if (i == 0) {
                break;
            }
            --i;
        }

        return sizeof(T)*8;
    }

    /**
     * Finds the highest bit in which the given values differ. The search starts at the given
     * index, which is 0-based from the right. The function returns the index of the
     * highest differing bit which is not higher than the given index, or the number of bits
     * of type T if no such bit is found.
     *
     * If the given start index is greater than the number of bits in T, the search will
     * start at the highest bit of T regardless.
     *
     *
     * @tparam T the type of the arguments
     * @param x the first value
     * @param y the second value
     * @param i the 0-based index of the bit to start the search at
     * @return the 0-based index of the highest set in which x and y differ or the number of bits of T of no such bit is found
     */
    template <typename T>
    size_t findHighestDifferingBit(const T x, const T y, size_t i = sizeof(T)*8 - 1) {
        static_assert(std::is_integral<T>::value, "x and y are integral");
        return findHighestOrderBit(x ^ y, i);
    }

    template <typename T>
    T normalizeRadians(T angle) {
        static const T z = static_cast<T>(0.0);
        static const T o = Constants<T>::twoPi();
        while (angle < z)
            angle += o;
        return mod(angle, o);
    }
    
    template <typename T>
    T normalizeDegrees(T angle) {
        static const T z = static_cast<T>(0.0);
        static const T o = static_cast<T>(360.0);
        while (angle < z)
            angle += o;
        return mod(angle, o);
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
        Side_Front = 1,
        Side_Back  = 2,
        Side_Both  = 3
    } Side;
    
    typedef enum {
        Direction_Forward,
        Direction_Backward,
        Direction_Left,
        Direction_Right,
        Direction_Up,
        Direction_Down
    } Direction;

    namespace Axis {
        typedef size_t Type;
        static const Type AX = 0;
        static const Type AY = 1;
        static const Type AZ = 2;
    }
    
    typedef enum {
        RotationAxis_Roll,
        RotationAxis_Pitch,
        RotationAxis_Yaw
    } RotationAxis;

    // TODO 2201: make this an enum
    namespace PointStatus {
        typedef size_t Type;
        static const Type PSAbove = 0;
        static const Type PSBelow = 1;
        static const Type PSInside = 2;
    }
    
    double nextgreater(double value);
}

#endif
