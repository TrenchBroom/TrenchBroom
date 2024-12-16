/*
 Copyright 2021 Kristian Duske

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

#include "vm/vec.h"

#include <array>

namespace vm
{
template <typename T, size_t C>
vec<T, C> evaluate_quadratic_bezier_surface(
  const std::array<std::array<vec<T, C>, 3>, 3>& controlPoints, const T u, const T v)
{
  const auto bernsteinPolynomial_0 = [](const auto x) {
    return static_cast<T>(1) - static_cast<T>(2) * x + (x * x);
  };

  const auto bernsteinPolynomial_1 = [](const auto x) {
    return static_cast<T>(2) * (x - (x * x));
  };

  const auto bernsteinPolynomial_2 = [](const auto x) { return x * x; };

  const auto interpolate = [&](const auto x, const std::array<vec<T, C>, 3>& p) {
    auto result = vec<T, C>{};
    result = result + bernsteinPolynomial_0(x) * p[0];
    result = result + bernsteinPolynomial_1(x) * p[1];
    result = result + bernsteinPolynomial_2(x) * p[2];
    return result;
  };

  return interpolate(
    v,
    {
      interpolate(u, controlPoints[0]),
      interpolate(u, controlPoints[1]),
      interpolate(u, controlPoints[2]),
    });
}
} // namespace vm
