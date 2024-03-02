/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "vm/constants.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace vm
{
/**
 * A function that just returns its argument.
 */
struct identity
{
  template <typename U>
  constexpr auto operator()(U&& v) const noexcept -> decltype(std::forward<U>(v))
  {
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
constexpr bool is_nan(const T f)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return f != f;
}

/**
 * Checks whether the given float is positive or negative infinity.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param f the float to check
 * @return true if the given float is positive or negative infinity
 */
template <typename T>
constexpr bool is_inf(const T f)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return (
    f == std::numeric_limits<T>::infinity() || f == -std::numeric_limits<T>::infinity());
}

/**
 * Returns a floating point value that represents NaN.
 *
 * @tparam T the result type, which must be a floating point type
 * @return a value that represents NaN
 */
template <typename T>
constexpr T nan()
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
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
constexpr T abs(const T v)
{
  return v < static_cast<T>(0) ? -v : v;
}

/**
 * Returns the given value. Used as the base case of the variadic min template.
 *
 * @tparam T the argument type
 * @param v the value to return
 * @return the given value
 */
template <typename T>
constexpr T min(const T v)
{
  return v;
}

/**
 * Returns the minimum of the given values.
 *
 * @tparam T the argument type
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first value
 * @param rhs the second value
 * @param rest the remaining values
 * @return the minimum of the given values
 */
template <typename T, typename... Rest>
constexpr T min(const T lhs, const T rhs, const Rest... rest)
{
  if (lhs < rhs)
  {
    return min(lhs, rest...);
  }
  else
  {
    return min(rhs, rest...);
  }
}

/**
 * Returns the given value. Used as the base case of the variadic max template.
 *
 * @tparam T the argument type
 * @param v the value to return
 * @return the given value
 */
template <typename T>
constexpr T max(const T v)
{
  return v;
}

/**
 * Returns the maximum of the given values.
 *
 * @tparam T the argument type
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first value
 * @param rhs the second value
 * @param rest the remaining values
 * @return the maximum of the given values
 */
template <typename T, typename... Rest>
constexpr T max(const T lhs, const T rhs, const Rest... rest)
{
  if (lhs > rhs)
  {
    return max(lhs, rest...);
  }
  else
  {
    return max(rhs, rest...);
  }
}

/**
 * Returns the given value. Used as the base case of the variadic abs_min template.
 *
 * @tparam T the argument type
 * @param v the value to return
 * @return the given value
 */
template <typename T>
constexpr T abs_min(const T v)
{
  return v;
}

/**
 * Returns the minimum of the absolute given values. Note that this function does not
 * return the absolute of the minimal value.
 *
 * @tparam T the argument type
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first value
 * @param rhs the second value
 * @param rest the remaining values
 * @return the minimum of the absolute given values
 */
template <typename T, typename... Rest>
constexpr T abs_min(const T lhs, const T rhs, const Rest... rest)
{
  if (abs(lhs) < abs(rhs))
  {
    return abs_min(lhs, rest...);
  }
  else
  {
    return abs_min(rhs, rest...);
  }
}

/**
 * Returns the given value. Used as the base case of the variadic abs_max template.
 *
 * @tparam T the argument type
 * @param v the value to return
 * @return the given value
 */
template <typename T>
constexpr T abs_max(const T v)
{
  return v;
}

/**
 * Returns the maximum of the absolute given values. Note that this function does not
 * return the absolute of the maximum value.
 *
 * @tparam T the argument type
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first value
 * @param rhs the second value
 * @param rest the remaining values
 * @return the maximum of the absolute given values
 */
template <typename T, typename... Rest>
constexpr T abs_max(const T lhs, const T rhs, const Rest... rest)
{
  if (abs(lhs) > abs(rhs))
  {
    return abs_max(lhs, rest...);
  }
  else
  {
    return abs_max(rhs, rest...);
  }
}

/**
 * Returns the given value. Used as the base case of the variadic safe_min template.
 *
 * @tparam T the argument type
 * @param v the value to return
 * @return the given value
 */
template <typename T>
constexpr T safe_min(const T v)
{
  return v;
}

/**
 * Returns the minimum of the given values, but checks if any of the given values is NaN,
 * in which case it is not considered in the result.
 *
 * @tparam T the argument type
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first value
 * @param rhs the second value
 * @param rest the remaining values
 * @return the minimum of the given values
 */
template <typename T, typename... Rest>
constexpr T safe_min(const T lhs, const T rhs, const Rest... rest)
{
  if (is_nan(lhs))
  {
    return safe_min(rhs, rest...);
  }
  else if (is_nan(rhs))
  {
    return safe_min(lhs, rest...);
  }
  else if (lhs < rhs)
  {
    return safe_min(lhs, rest...);
  }
  else
  {
    return safe_min(rhs, rest...);
  }
}

/**
 * Returns the given value. Used as the base case of the variadic safe_min template.
 *
 * @tparam T the argument type
 * @param v the value to return
 * @return the given value
 */
template <typename T>
constexpr std::optional<T> safe_min(const std::optional<T>& v)
{
  return v;
}

/**
 * Returns the minimum of the given values, but checks if any of the given values is
 * nullopt, in which case it is not considered in the result.
 *
 * @tparam T the argument type
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first value
 * @param rhs the second value
 * @param rest the remaining values
 * @return the minimum of the given values or nullopt if all values are nullopt
 */
template <typename T, typename... Rest>
constexpr std::optional<T> safe_min(
  const std::optional<T>& lhs, const std::optional<T>& rhs, const Rest... rest)
{
  if (!lhs)
  {
    return safe_min(rhs, rest...);
  }
  if (!rhs)
  {
    return safe_min(lhs, rest...);
  }
  if (*lhs < *rhs)
  {
    return safe_min(lhs, rest...);
  }
  return safe_min(rhs, rest...);
}

/**
 * Returns the given value. Used as the base case of the variadic safe_max template.
 *
 * @tparam T the argument type
 * @param v the value to return
 * @return the given value
 */
template <typename T>
constexpr T safe_max(const T v)
{
  return v;
}

/**
 * Returns the maximum of the given values, but checks if any of the given values is NaN,
 * in which case it is not considered in the result.
 *
 * @tparam T the argument type
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first value
 * @param rhs the second value
 * @param rest the remaining values
 * @return the maximum of the given values
 */
template <typename T, typename... Rest>
constexpr T safe_max(const T lhs, const T rhs, const Rest... rest)
{
  if (is_nan(lhs))
  {
    return safe_max(rhs, rest...);
  }
  else if (is_nan(rhs))
  {
    return safe_max(lhs, rest...);
  }
  else if (lhs > rhs)
  {
    return safe_max(lhs, rest...);
  }
  else
  {
    return safe_max(rhs, rest...);
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
constexpr T abs_difference(const T lhs, const T rhs)
{
  if constexpr (std::is_signed<T>::value)
  {
    return abs(abs(lhs) - abs(rhs));
  }
  else
  {
    return lhs > rhs ? lhs - rhs : rhs - lhs;
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
constexpr T clamp(
  const T v, const T minV = static_cast<T>(0.0), const T maxV = static_cast<T>(1.0))
{
  return max(min(v, maxV), minV);
}

/**
 * Returns a value indicating the sign of the given value.
 *
 * @tparam T the argument type
 * @param v the value
 * @return -1 if the given value is less then 0, +1 if the value is greater than 0, and 0
 * if the given value is 0
 */
template <typename T>
constexpr T sign(const T v)
{
  if (v < T(0))
  {
    return T(-1);
  }
  else if (v > T(0))
  {
    return T(+1);
  }
  else
  {
    return T(0);
  }
}

/**
 * Returns 0 if the given value is less than the given edge value, and 1 otherwise.
 *
 * @tparam T the argument type
 * @param v the value
 * @param e the edge value
 * @return 0 or 1 depending on whether the given value is less than the given edge value
 * or not
 */
template <typename T>
constexpr T step(const T e, const T v)
{
  return v < e ? T(0) : T(1);
}

/**
 * Performs performs smooth Hermite interpolation between 0 and 1 when e0 < v < e1.
 *
 * @tparam T the argument type
 * @param e0 the lower edge value
 * @param e1 the upper edge value
 * @param v the value to interpolate
 * @return the interpolated value
 */
template <typename T>
constexpr T smoothstep(const T e0, const T e1, const T v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  const auto t = clamp((v - e0) / (e1 - e0), T(0), T(1));
  return t * t * (T(3) - T(2) * t);
}

/**
 * Returns the nearest integer value not greater in magnitude than the given value, i.e.,
 * the given value is rounded towards 0.
 *
 * @tparam T the argument type
 * @param v the value to truncate
 * @return the truncated value
 */
template <typename T>
constexpr T trunc(const T v)
{
  return static_cast<T>(static_cast<int64_t>(v));
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
constexpr T mod(const T x, const T y)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return x - y * trunc(x / y);
}

/**
 * Computes the largest integer value not greater than the given value.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param v the value
 * @return the largest integer value not greater than the given value
 */
template <typename T>
constexpr T floor(const T v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  const auto r = trunc(v);
  return (v >= static_cast<T>(0.0)) ? r : (r == v ? v : r - static_cast<T>(1.0));
}

/**
 * Computes the smallest integer value not less than the given value.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param v the value
 * @return the smallest integer value not less than the given value
 */
template <typename T>
constexpr T ceil(const T v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  const auto r = trunc(v);
  return (v < static_cast<T>(0.0)) ? r : (r == v ? r : r + static_cast<T>(1.0));
}

/**
 * Linearly interpolates between the given values using the given weight.
 *
 * @param x the first value to interpolate
 * @param y the second value to interpolate
 * @param a the weight
 * @return the interpolated values (1-a)*x + a*y
 */
template <typename T>
constexpr T mix(const T x, const T y, const T a)
{
  return (static_cast<T>(1.0) - a) * x + a * y;
}

/**
 * Returns the fractional part of the given value.
 *
 * @tparam T argument type
 * @param v the value
 * @return the fractional part
 */
template <typename T>
constexpr T fract(const T v)
{
  if (v > T(0))
  {
    return v - floor(v);
  }
  else
  {
    return v - ceil(v);
  }
}

/**
 * Rounds the given value to the nearest integer value.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param v the value
 * @return the nearest integer value to the given value
 */
template <typename T>
constexpr T round(const T v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return v > static_cast<T>(0.0) ? floor(v + static_cast<T>(0.5))
                                 : ceil(v - static_cast<T>(0.5));
}

/**
 * Rounds the given value away from 0. Given a positive value, this function returns the
 * largester integer not greater than the given value, and given a negative value, this
 * function returns the smallest integer not less than the given value.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param v the value
 * @return the rounded value
 */
template <typename T>
constexpr T round_up(const T v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return v < static_cast<T>(0.0) ? floor(v) : ceil(v);
}

/**
 * Rounds the given value towards 0. Given a positive value, this function returns the
 * largest integer not greater than the given value, and given a negative value, this
 * function returns the smallest integer not less than the given value.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param v the value
 * @return the rounded value
 */
template <typename T>
constexpr T round_down(const T v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  // this is equivalent to calling trunc
  // we keep this function for consistency because there is no equivalent function to
  // roundUp
  return v > static_cast<T>(0.0) ? floor(v) : ceil(v);
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
constexpr T snap(const T v, const T grid)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  assert(grid != static_cast<T>(0.0));
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
constexpr T snapUp(const T v, const T grid)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  assert(grid > static_cast<T>(0.0));
  return grid * round_up(v / grid);
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
constexpr T snapDown(const T v, const T grid)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  assert(grid > static_cast<T>(0.0));
  return grid * round_down(v / grid);
}

/**
 * Rounds the given value to the nearest integer if its distance to that integer is less
 * than the given epsilon. Furthermore, the value is rounded such that at most the given
 * number of decimals are retained.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param v the value to round
 * @param decimals the number of decimals to retain
 * @param epsilon an epsilon value
 * @return the corrected value
 */
template <typename T>
constexpr T correct(
  const T v,
  const std::size_t decimals = 0u,
  const T epsilon = constants<T>::correct_epsilon())
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  const T m = static_cast<T>(1u << decimals);
  const T r = round(v * m);
  if (abs(v - r) < epsilon)
  {
    return r / m;
  }
  else
  {
    return v;
  }
}

/**
 * Checks whether the given values are equal.
 *
 * @tparam T the argument type
 * @param lhs the first value
 * @param rhs the second value
 * @param epsilon an epsilon value
 * @return true if the distance of the given values is less than the given epsilon and
 * false otherwise
 */
template <typename T>
constexpr bool is_equal(const T lhs, const T rhs, const T epsilon)
{
  return abs(lhs - rhs) <= epsilon;
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
constexpr bool is_zero(const T v, const T epsilon)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  // MSVC sometimes complains about a possible division by 0 when we use is_zero to check
  // that the denominator is not 0. The diagnostic does not understand that abs(v) >
  // epsilon implies that v != 0, so we make it explicit.
  return v == T(0) || abs(v) <= epsilon;
}

/**
 * Checks whether the given value is in the given interval. The interval boundaries are
 * inclusive, and need not be ordered.
 *
 * @tparam T the argument type
 * @param v the value to check
 * @param s the interval start
 * @param e the interval end
 * @return true if the given value is in the given interval and false otherwise
 */
template <typename T>
constexpr bool contains(const T v, const T s, const T e)
{
  if (s < e)
  {
    return v >= s && v <= e;
  }
  else
  {
    return v >= e && v <= s;
  }
}

/**
 * Converts the given angle to radians.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param d the angle to convert, in degrees
 * @return the converted angle in radians
 */
template <typename T>
constexpr T to_radians(const T d)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return d * constants<T>::pi() / static_cast<T>(180);
}

/**
 * Converts the given angle to degrees.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param r the angle to convert, in radians
 * @return the converted angle in degrees
 */
template <typename T>
constexpr T to_degrees(const T r)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return r * static_cast<T>(180) / constants<T>::pi();
}

/**
 * Normalizes the given angle by constraining it to the interval [0 - 2*PI[.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param angle the angle to normalize, in radians
 * @return the normalized angle
 */
template <typename T>
constexpr T normalize_radians(T angle)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  constexpr T z = static_cast<T>(0.0);
  constexpr T o = constants<T>::two_pi();
  while (angle < z)
  {
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
constexpr T normalize_degrees(T angle)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  constexpr T z = static_cast<T>(0.0);
  constexpr T o = static_cast<T>(360.0);
  while (angle < z)
  {
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
 * @param stride the offset by which to increase the index
 * @return the succeeding value
 */
template <typename T, typename U>
constexpr T succ(const T index, const U count, const T stride = static_cast<T>(1))
{
  static_assert(std::is_integral<T>::value, "T must be an integer type");
  static_assert(std::is_integral<U>::value, "U must be an integer type");
  return (index + stride) % static_cast<T>(count);
}

/**
 * Returns (index + count - offset) modulo count.
 *
 * @tparam T the argument type, which must be integral
 * @tparam U the argument type of count, which must be integral
 * @param index the index to decrease
 * @param count the maximum value
 * @param stride the offset by which to decrease the index
 * @return the preceeding value
 */
template <typename T, typename U>
constexpr T pred(const T index, const U count, const T stride = static_cast<T>(1))
{
  static_assert(std::is_integral<T>::value, "T must be an integer type");
  static_assert(std::is_integral<U>::value, "U must be an integer type");
  const auto c = static_cast<T>(count);
  return ((index + c) - (stride % c)) % c;
}

/**
 * Returns the smallest floating point value greater than the given value, or infinity if
 * no such value exists.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param value the value
 * @return the smallest floating point value greater than the given value
 */
template <typename T>
T nextgreater(const T value)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  // TODO: does MSC not implement cmath correctly?
#ifdef _MSC_VER
  return _nextafter(value, std::numeric_limits<T>::infinity());
#else
  return std::nextafter(value, std::numeric_limits<T>::infinity());
#endif
}

template <typename T>
constexpr T sqrt_c_nr(const T x, const T curr, const T prev)
{
  return curr == prev ? curr
                      : sqrt_c_nr(x, static_cast<T>(0.5) * (curr + x / curr), curr);
}

/**
 * Computes the square root of the given value at compile time.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param value the value or NaN if an error occurs
 * @return the square root of the value
 */
template <typename T>
constexpr T sqrt_c(const T value)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  if (is_nan(value) || value == std::numeric_limits<T>::infinity())
  {
    return value;
  }
  else if (value >= static_cast<T>(0.0))
  {
    return sqrt_c_nr(value, value, static_cast<T>(0.0));
  }
  else
  {
    return std::numeric_limits<T>::quiet_NaN();
  }
}

/**
 * Computes the square root of the given value.
 *
 * @tparam T the argument type, which must be a floating point type
 * @param value the value or NaN if an error occurs
 * @return the square root of the value
 */
template <typename T>
T sqrt(const T value)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
  return std::sqrt(value);
}

/**
 * Solves a quadratic polynomial with the given coefficients and returns up to two
 * solutions.
 *
 * The polynomial is of the form a*x^2 + b*x + c = 0.
 *
 * The first element of the returned tuple indicates the number of solutions (0, 1 or 2)
 * and the remaining elements contain the solutions.
 *
 * @tparam T the type of the coefficients
 * @param a the coefficient of the quadratic term
 * @param b the coefficient of the linear term
 * @param c the constant term
 * @param epsilon the epsilon value for floating point comparison
 * @return a tuple of the number of solutions and the solutions
 */
template <typename T>
std::tuple<std::size_t, T, T> solve_quadratic(
  const T a, const T b, const T c, const T epsilon)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

  // adapted from https://github.com/erich666/GraphicsGems/blob/master/gems/Roots3And4.c

  // normal form: x^2 + px + q = 0
  const auto p = b / (static_cast<T>(2.0) * a); // actually p/2
  const auto q = c / a;
  const auto D = p * p - q;

  if (is_zero(D, epsilon))
  {
    return {1u, -p, nan<T>()};
  }
  else if (D < T(0.0))
  {
    return {0u, nan<T>(), nan<T>()};
  }
  else
  {
    const auto D2 = sqrt(D);
    return {2u, D2 - p, -D2 - p};
  }
}

/**
 * Solves a cubic polynomial with the given coefficients and returns up to three
 * solutions.
 *
 * The polynomial is of the form a*x^3 + b*x^2 + c*x + d = 0.
 *
 * The first element of the returned tuple indicates the number of solutions (0, 1, 2 or
 * 3), and the remaining elements contain the solutions.
 *
 * @tparam T the type of the coefficients
 * @param a the coefficient of the cubic term
 * @param b the coefficient of the quadratic term
 * @param c the coefficient of the linear term
 * @param d the constant term
 * @param epsilon the epsilon value for floating point comparison
 * @return a tuple of the number of solutions and the solutions
 */
template <typename T>
std::tuple<std::size_t, T, T, T> solve_cubic(
  const T a, const T b, const T c, const T d, const T epsilon)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

  // adapted from https://github.com/erich666/GraphicsGems/blob/master/gems/Roots3And4.c

  // normal form: x^3 + A*x^2 + B*x + C = 0
  const auto A = b / a;
  const auto B = c / a;
  const auto C = d / a;

  // substitute x = y - A/3 to eliminate quadratic term: x^3 + px + q = 0
  const auto p = T(1.0 / 3.0) * (-T(1.0 / 3.0) * A * A + B);
  const auto q = T(1.0 / 2.0) * (T(2.0 / 27.0) * A * A * A - T(1.0 / 3.0) * A * B + C);

  // use Cardano's formula
  const auto p3 = p * p * p;
  const auto D = q * q + p3;

  std::size_t num = 0;
  T solutions[3] = {nan<T>(), nan<T>(), nan<T>()};
  if (is_zero(D, epsilon))
  {
    if (is_zero(q, epsilon))
    {
      // one triple solution
      num = 1u;
      solutions[0] = T(0);
    }
    else
    {
      // one single and one double solution
      const auto u = std::cbrt(-q);
      num = 2u;
      solutions[0] = u * T(2);
      solutions[1] = -u;
    }
  }
  else if (D < T(0.0))
  {
    // casus irreducibilis: three real solutions
    const auto phi = T(1.0 / 3.0) * std::acos(-q / sqrt(-p3));
    const auto t = T(2.0) * sqrt(-p);
    num = 3u;
    solutions[0] = t * std::cos(phi);
    solutions[1] = -t * std::cos(phi + constants<T>::pi() / T(3));
    solutions[2] = -t * std::cos(phi - constants<T>::pi() / T(3));
  }
  else
  {
    // one real solution
    const auto D2 = sqrt(D);
    const auto u = std::cbrt(D2 - q);
    const auto v = -std::cbrt(D2 + q);
    num = 1u;
    solutions[0] = u + v;
  }

  // resubstitute
  const auto sub = T(1.0 / 3.0) * A;
  for (std::size_t i = 0; i < num; ++i)
  {
    solutions[i] -= sub;
  }

  return {num, solutions[0], solutions[1], solutions[2]};
}

/**
 * Solves a quartic polynomial with the given coefficients and returns up to four
 * solutions.
 *
 * The polynomial is of the form a*x^4 + b*x^3 + c*x^2 + d*x + e = 0.
 *
 * The first element of the returned tuple indicates the number of solutions (0, 1, 2, 3
 * or 4) and the remaining elements contain the solutions.
 *
 * @tparam T the type of the coefficients
 * @param a the coefficient of the quartic term
 * @param b the coefficient of the cubic term
 * @param c the coefficient of the quadratic term
 * @param d the coefficient of the linear term
 * @param e the constant term
 * @param epsilon the epsilon value for floating point comparison
 * @return a tuple of the number of solutions and the solutions
 */
template <typename T>
std::tuple<size_t, T, T, T, T> solve_quartic(
  const T a, const T b, const T c, const T d, const T e, const T epsilon)
{
  static_assert(std::is_floating_point<T>::value, "T must be a floating point type");

  // adapted from https://github.com/erich666/GraphicsGems/blob/master/gems/Roots3And4.c

  // normal form: x^4 + A*x^3 + B*x^1 + C*x + D = 0
  const auto A = b / a;
  const auto B = c / a;
  const auto C = d / a;
  const auto D = e / a;

  // substitute x = y - A/4 to eliminate cubic term: x^4 + px^2 + qx + r = 0
  const auto p = -T(3.0 / 8.0) * A * A + B;
  const auto q = T(1.0 / 8.0) * A * A * A - T(1.0 / 2.0) * A * B + C;
  const auto r = -T(3.0 / 256.0) * A * A * A * A + T(1.0 / 16.0) * A * A * B
                 - T(1.0 / 4.0) * A * C + D;

  std::size_t num = 0;
  T solutions[4] = {nan<T>(), nan<T>(), nan<T>(), nan<T>()};
  if (is_zero(r, epsilon))
  {
    // no absolute term: y(y^3 + py + q) = 0
    const auto solutions3 = solve_cubic(T(1.0), T(0.0), p, q, epsilon);
    const auto num3 = std::get<0>(solutions3);
    num = num3 + 1u;
    solutions[0] = std::get<1>(solutions3);
    solutions[1] = std::get<2>(solutions3);
    solutions[2] = std::get<3>(solutions3);
    solutions[num - 1] = T(0.0);
  }
  else
  {
    // solve the resolvent cubic ...
    const auto solutions3 = solve_cubic(
      T(1.0),
      -T(1.0 / 2.0) * p,
      -r,
      T(1.0 / 2.0) * r * p - T(1.0 / 8.0) * q * q,
      epsilon);

    assert(std::get<0>(solutions3) > 0u);

    // ... and take the one real solution ...
    const auto z = std::get<1>(solutions3);

    // ... to build two quadratic equations
    auto u = z * z - r;
    auto v = T(2) * z - p;

    if (is_zero(u, epsilon))
    {
      u = T(0);
    }
    else if (u > T(0))
    {
      u = sqrt(u);
    }
    else
    {
      return {0u, nan<T>(), nan<T>(), nan<T>(), nan<T>()};
    }

    if (is_zero(v, epsilon))
    {
      v = T(0);
    }
    else if (v > T(0))
    {
      v = sqrt(v);
    }
    else
    {
      return {0u, nan<T>(), nan<T>(), nan<T>(), nan<T>()};
    }

    const auto solutions2_1 = solve_quadratic(T(1), q < T(0) ? -v : v, z - u, epsilon);
    const auto solutions2_2 = solve_quadratic(T(1), q < T(0) ? v : -v, z + u, epsilon);

    const auto num2_1 = std::get<0>(solutions2_1);
    const auto num2_2 = std::get<0>(solutions2_2);
    num = num2_1 + num2_2;

    if (num2_1 > 0u)
    {
      solutions[0] = std::get<1>(solutions2_1);
    }
    if (num2_1 > 1u)
    {
      solutions[1] = std::get<2>(solutions2_1);
    }

    if (num2_1 > 0u)
    {
      solutions[0 + num2_1] = std::get<1>(solutions2_2);
    }
    if (num2_1 > 1u)
    {
      solutions[1 + num2_1] = std::get<2>(solutions2_2);
    }
  }

  // resubstitute
  const auto sub = T(1.0 / 4.0) * A;
  for (std::size_t i = 0; i < num; ++i)
  {
    solutions[i] -= sub;
  }

  return {num, solutions[0], solutions[1], solutions[2], solutions[3]};
}
} // namespace vm
