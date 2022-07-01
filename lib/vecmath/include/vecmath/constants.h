/*
 Copyright 2010-2019 Kristian Duske
 Copyright 2015-2019 Eric Wasylishen

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
 OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

namespace vm {
template <typename T> class constants {
public:
  /**
   * Returns an epsilon value to use for comparisons.
   *
   * @return an epsilon value
   */
  constexpr static T almost_zero() {
    constexpr T value = static_cast<T>(0.001);
    return value;
  }

  /**
   * Returns an epsilon value to use for determining the position of a point in relation to a point
   * and vector, e.g. a plane.
   *
   * @return an epsilon value
   */
  constexpr static T point_status_epsilon() {
    constexpr T value = static_cast<T>(0.0001); // this is what tyrbsp uses
    return value;
  }

  /**
   * Returns an epsilon value to use for correcting a value that is very close to an integral value.
   *
   * @return an epsilon value
   */
  constexpr static T correct_epsilon() {
    constexpr T value = static_cast<T>(0.001); // this is what QBSP uses
    return value;
  }

  /**
   * Returns an epislon value to use when testing colinearity.
   *
   * @return an epsilon value
   */
  constexpr static T colinear_epsilon() {
    constexpr T value = static_cast<T>(
      0.00001); // this value seems to hit a sweet spot in relation to the point status epsilon
    return value;
  }

  /**
   * Returns an epsilon value to use when comparing angles in radians.
   *
   * @return an epsilon value
   */
  constexpr static T angle_epsilon() {
    constexpr T value =
      static_cast<T>(0.00000001); // if abs(sin()) of the angle between two vectors is less than
                                  // this, they are considered to be parallel or opposite
    return value;
  }

  /**
   * Returns the value of PI.
   *
   * @return the value of PI
   */
  constexpr static T pi() {
    constexpr T value = static_cast<T>(3.141592653589793);
    return value;
  }

  /**
   * Returns the value of 2 * PI.
   *
   * @return the value of 2 * PI
   */
  constexpr static T two_pi() {
    constexpr T value = static_cast<T>(2.0) * pi();
    return value;
  }

  /**
   * Returns the value of PI / 2.
   *
   * @return the value of PI / 2
   */
  constexpr static T half_pi() {
    constexpr T value = pi() / static_cast<T>(2.0);
    return value;
  }

  /**
   * Returns the value of PI / 4.
   *
   * @return the value of PI / 4
   */
  constexpr static T quarter_pi() {
    constexpr T value = pi() / static_cast<T>(4.0);
    return value;
  }

  /**
   * Returns the value of 3 * PI / 2.
   *
   * @return the value of 3 * PI / 2
   */
  constexpr static T three_half_pi() {
    constexpr T value = static_cast<T>(3.0) * pi() / static_cast<T>(2.0);
    return value;
  }

  /**
   * Returns the value of e, the Euler constant.
   *
   * @return the value of e
   */
  constexpr static T e() {
    constexpr T value = static_cast<T>(2.718281828459045);
    return value;
  }
};

using Cf = constants<float>;
using Cd = constants<double>;
} // namespace vm
