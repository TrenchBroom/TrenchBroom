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

#include "constants.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

namespace vm {
    /**
     * A function that just returns its argument.
     */
    struct Identity {
        template<typename U>
        constexpr auto operator()(U&& v) const noexcept -> decltype(std::forward<U>(v)) {
            return std::forward<U>(v);
        }
    };

    /**
     * Checks whether the given float is NaN.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param f the float to check
     * @return bool if the given float is NaN and false otherwise
     */
    template <typename T>
    bool isNan(const T f) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
#ifdef _MSC_VER
        return _isnan(f) != 0;
#else
        return std::isnan(f);
#endif
    }

    /**
     * Checks whether the given float is positive or negative infinity.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param f the float to check
     * @return true if the given float is positive or negative infinity
     */
    template <typename T>
    bool isInf(const T f) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return (f ==  std::numeric_limits<T>::infinity() ||
                f == -std::numeric_limits<T>::infinity());
    }

    /**
     * Returns a floating point value that represents NaN.
     *
     * @tparam T the result type, which must be a floating point type
     * @return a value that represents NaN
     */
    template <typename T>
    T nan() {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return std::numeric_limits<T>::quiet_NaN();
    }

    /**
     * Returns the absolute of the given value.
     *
     * @tparam T the argument type
     * @param v the value
     * @return the absolute of the given value
     */
    template <typename T>
    T abs(const T v) {
        return std::abs(v);
    }

    /**
     * Returns the floating point remainder of x/y.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param x the dividend
     * @param y the divisor
     * @return the remainder of x/y
     */
    template <typename T>
    T mod(const T x, const T y) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return std::fmod(x, y);
    }

    /**
     * Returns the minimum of the given values.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @return the minimum of the given values
     */
    template <typename T>
    T min(const T lhs, const T rhs) {
        if (lhs < rhs) {
            return lhs;
        } else {
            return rhs;
        }
    }

    /**
     * Returns the maximum of the given values.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @return the maximum of the given values
     */
    template <typename T>
    T max(const T lhs, const T rhs) {
        if (lhs > rhs) {
            return lhs;
        } else {
            return rhs;
        }
    }

    /**
     * Returns the minimum of the absolute given values. Note that this function does not return the absolute of the
     * minimal value.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @return the minimum of the absolute given values
     */
    template <typename T>
    T absMin(const T lhs, const T rhs) {
        if (abs(lhs) < abs(rhs)) {
            return lhs;
        } else {
            return rhs;
        }
    }

    /**
     * Returns the maximum of the absolute given values. Note that this function does not return the absolute of the
     * maximal value.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @return the maximum of the absolute given values
     */
    template <typename T>
    T absMax(const T lhs, const T rhs) {
        if (abs(lhs) > abs(rhs)) {
            return lhs;
        } else {
            return rhs;
        }
    }

    /**
     * Returns the absolute difference of the given values.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @return the absolute difference of the given values
     */
    template <typename T>
    T absDifference(const T lhs, const T rhs) {
        return abs(abs(lhs) - abs(rhs));
    }

    /**
     * Converts the given angle to radians.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param d the angle to convert, in degrees
     * @return the converted angle in radians
     */
    template <typename T>
    T radians(const T d) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return d * constants<T>::piOverStraightAngle();
    }

    /**
     * Converts the given angle to degrees.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param r the angle to convert, in radians
     * @return the converted angle in degrees
     */
    template <typename T>
    T degrees(const T r) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return r * constants<T>::straightAngleOverPi();
    }

    /**
     * Computes the largest integer value not greater than the given value.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value
     * @return the largest integer value not greater than the given value
     */
    template <typename T>
    T floor(const T v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return std::floor(v);
    }

    /**
     * Computes the smallest integer value not less than the given value.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value
     * @return the smallest integer value not less than the given value
     */
    template <typename T>
    T ceil(const T v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return std::ceil(v);
    }

    /**
     * Rounds the given value to the nearest integer value.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value
     * @return the nearest integer value to the given value
     */
    template <typename T>
    T round(const T v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return v > 0.0 ? floor(v + static_cast<T>(0.5)) : ceil(v - static_cast<T>(0.5));
    }

    /**
     * Computes the offset to the nearest integer value.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value
     * @return the offset to the nearest integer value
     */
    template <typename T>
    T integerOffset(const T v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return v - round(v);
    }

    /**
     * Rounds the given value away from 0. Given a positive value, this function returns the largester integer not
     * greater than the given value, and given a negative value, this function returns the smallest integer not less
     * than the given value.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value
     * @return the rounded value
     */
    template <typename T>
    T roundUp(const T v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return v < 0.0 ? floor(v) : ceil(v);
    }

    /**
     * Rounds the given value towards 0. Given a positive value, this function returns the largest integer not
     * greater than the given value, and given a negative value, this function returns the smallest integer not less
     * than the given value.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value
     * @return the rounded value
     */
    template <typename T>
    T roundDown(const T v) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return v > 0.0 ? floor(v) : ceil(v);
    }

    /**
     * Rounds the given value to the nearest multiple of the given grid size.
     *
     * @tparam T the argument type
     * @param v the value to round
     * @param grid the grid size to round to
     * @return the rounded value
     */
    template <typename T>
    T snap(const T v, const T grid) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        assert(grid != 0.0);
        return grid * round(v / grid);
    }

    /**
     * Rounds the given value away from 0 to the nearest multiple of the given grid size.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value to round
     * @param grid the grid size to round to
     * @return the rounded value
     */
    template <typename T>
    T snapUp(const T v, const T grid) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        assert(grid > 0.0);
        return grid * roundUp(v / grid);
    }

    /**
     * Rounds the given value towards 0 to the nearest multiple of the given grid size.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value to round
     * @param grid the grid size to round to
     * @return the rounded value
     */
    template <typename T>
    T snapDown(const T v, const T grid) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        assert(grid > 0.0);
        return grid * roundDown(v / grid);
    }

    /**
     * Rounds the given value to the nearest integer if its distance to that integer is less than the given epsilon.
     * Furthermore, the value is rounded such that at most the given number of decimals are retained.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value to round
     * @param decimals the number of decimals to retain
     * @param epsilon an epsilon value
     * @return the corrected value
     */
    template <typename T>
    T correct(const T v, const size_t decimals = 0, const T epsilon = constants<T>::correctEpsilon()) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        const T m = static_cast<T>(1 << decimals);
        const T r = round(v * m);
        if (abs(v - r) < epsilon) {
            return r / m;
        } else {
            return v;
        }
    }

    /**
     * Checks whether the first given value is greater than the second given value.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @param epsilon an epsilon value
     * @return true if the first given value is greater than the sum of the secon value and the epsilon value, and false otherwise
     */
     // TODO: The more I think about this, the more I wonder if it's even correct, because it amounts to 1.0 > 0.95 being false with an epsilon of 0.1.
    template <typename T>
    bool gt(const T lhs, const T rhs, const T epsilon = constants<T>::almostZero()) {
        return lhs > rhs + epsilon;
    }

    /**
     * Checks whether the first given value is less than the second given value.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @param epsilon an epsilon value
     * @return true if the first given value is less than the sum of the secon value and the negated epsilon value, and false otherwise
     */
    // TODO: The more I think about this, the more I wonder if it's even correct, because it amounts to 0.95 < 1.0 being false with an epsilon of 0.1.
    template <typename T>
    bool lt(const T lhs, const T rhs, const T epsilon = constants<T>::almostZero()) {
        return lhs < rhs - epsilon;
    }

    /**
     * Checks whether the first given value is greater than or equal to the second given value.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @param epsilon an epsilon value
     * @return true if the first given value is greater than or equal to the sum of the secon value and the epsilon value, and false otherwise
     */
     // TODO: likewise, is this correct at all?
    template <typename T>
    bool gte(const T lhs, const T rhs, const T epsilon = constants<T>::almostZero()) {
        return !lt(lhs, rhs, epsilon);
    }

    /**
     * Checks whether the first given value is less than or equal to the second given value.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @param epsilon an epsilon value
     * @return true if the first given value is less than or equal to the sum of the secon value and the negated epsilon value, and false otherwise
     */
    // TODO: likewise, is this correct at all?
    template <typename T>
    bool lte(const T lhs, const T rhs, const T epsilon = constants<T>::almostZero()) {
        return !gt(lhs, rhs, epsilon);
    }

    /**
     * Checks whether the given values are equal.
     *
     * @tparam T the argument type
     * @param lhs the first value
     * @param rhs the second value
     * @param epsilon an epsilon value
     * @return true if the distance of the given values is less than the given epsilon and false otherwise
     */
    template <typename T>
    bool isEqual(const T lhs, const T rhs, const T epsilon = constants<T>::almostZero()) {
        return abs(lhs - rhs) < epsilon;
    }

    /**
     * Checks whether the given argument is 0 using the given epsilon.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value to check
     * @param epsilon an epsilon value
     * @return true if the distance of the given argument to 0 is less than the given epsilon
     */
    template <typename T>
    bool isZero(const T v, const T epsilon = constants<T>::almostZero()) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return abs(v) <= epsilon;
    }

    /**
     * Checks whether the given argument is positive using the given epsilon.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value to check
     * @param epsilon an epsilon value
     * @return true if the given argument is greater than or equal to the given epsilon
     */
    template <typename T>
    bool isPositive(const T v, const T epsilon = constants<T>::almostZero()) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return gt(v, static_cast<T>(0.0), epsilon);
    }

    /**
     * Checks whether the given argument is negative using the given epsilon.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value to check
     * @param epsilon an epsilon value
     * @return true if the given argument is less than or equal to the negated given epsilon
     */
    template <typename T>
    bool isNegative(const T v, const T epsilon = constants<T>::almostZero()) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return lt(v, static_cast<T>(0.0), epsilon);
    }

    /**
     * Checks whether the given value is integral. To be considered integral, the distance of the given value to the
     * nearest integer must be less than the given epsilon value.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param v the value
     * @param epsilon an epsilon value
     * @return true if the given value is integer and false otherwise
     */
    template <typename T>
    bool isInteger(const T v, const T epsilon = constants<T>::almostZero()) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        return isEqual(v, round(v), epsilon);
    }

    /**
     * Checks whether the given value is in the given interval. The interval boundaries are inclusive, and need not be
     * ordered.
     *
     * @tparam T the argument type
     * @param v the value to check
     * @param s the interval start
     * @param e the interval end
     * @param epsilon an epsilon value
     * @return true if the given value is in the given interval and false otherwise
     */
    template <typename T>
    bool contains(const T v, const T s, const T e, const T epsilon = constants<T>::almostZero()) {
        if (s < e) {
            return gte(v, s, epsilon) && lte(v, e, epsilon);
        } else {
            return gte(v, e, epsilon) && lte(v, s, epsilon);
        }
    }

    /**
     * Clamps the given value to the given interval.
     *
     * @tparam T the argument type
     * @param v the value to clamp
     * @param minV the minimal value
     * @param maxV the maximal value
     * @return the clamped value
     */
    template <typename T>
    T clamp(const T v, const T minV = static_cast<T>(0.0), const T maxV = static_cast<T>(1.0)) {
        return max(min(v, maxV), minV);
    }

    /**
     * Selects the minimum of the given values. If the first value is NaN, the second value is returned. Conversely, if
     * the second value is NaN, the first is returned. Otherwise, the minimum of the values is returned.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param lhs the first value
     * @param rhs the second value
     * @return the minimum of the given values, or NaN if both values are NaN
     */
    template <typename T>
    T selectMin(const T lhs, const T rhs) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        if (isNan(lhs)) {
            return rhs;
        } else if (isNan(rhs)) {
            return lhs;
        } else {
            return min(lhs, rhs);
        }
    }

    /**
     * Normalizes the given angle by constraining it to the interval [0 - 2*PI[.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param angle the angle to normalize, in radians
     * @return the normalized angle
     */
    template <typename T>
    T normalizeRadians(T angle) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        static const T z = static_cast<T>(0.0);
        static const T o = constants<T>::twoPi();
        while (angle < z) {
            angle += o;
        }
        return mod(angle, o);
    }

    /**
     * Normalizes the given angle by constraining it to the interval [0 - 360[.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param angle the angle to normalize, in degrees
     * @return the normalized angle
     */
    template <typename T>
    T normalizeDegrees(T angle) {
        static_assert(std::is_floating_point<T>::value, "T must be a float point type");
        static const T z = static_cast<T>(0.0);
        static const T o = static_cast<T>(360.0);
        while (angle < z) {
            angle += o;
        }
        return mod(angle, o);
    }

    /**
     * Returns index + offset modulo count.
     *
     * @tparam T the argument type, which must be integral
     * @tparam U the argument type of count, which must be integral
     * @param index the index to increase
     * @param count the maximum value
     * @param offset the offset by which to increase the index
     * @return the succeeding value
     */
    template <typename T, typename U>
    T succ(const T index, const U count, const T offset = static_cast<T>(1)) {
        static_assert(std::is_integral<T>::value, "T must be an integer type");
        static_assert(std::is_integral<U>::value, "U must be an integer type");
        return (index + offset) % static_cast<T>(count);
    }

    /**
     * Returns (index + count - offset) modulo count.
     *
     * @tparam T the argument type, which must be integral
     * @tparam U the argument type of count, which must be integral
     * @param index the index to decrease
     * @param count the maximum value
     * @param offset the offset by which to decrease the index
     * @return the preceeding value
     */
    template <typename T, typename U>
    T pred(const T index, const U count, const T offset = static_cast<T>(1)) {
        static_assert(std::is_integral<T>::value, "T must be an integer type");
        static_assert(std::is_integral<U>::value, "U must be an integer type");
        const auto c = static_cast<T>(count);
        return ((index + c) - (offset % c)) % c;
    }

    /**
     * Returns the smallest floating point value greater than the given value, or infinity if no such value exists.
     *
     * @tparam T the argument type, which must be a floating point type
     * @param value the value
     * @return the smallest floating point value greater than the given value
     */
    template <typename T>
    T nextgreater(const T value) {
        static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
        // TODO: does MSC not implement cmath correctly?
#ifdef _MSC_VER
        return _nextafter(value, std::numeric_limits<T>::infinity());
#else
        return std::nextafter(value, std::numeric_limits<T>::infinity());
#endif
    }
}

#endif
