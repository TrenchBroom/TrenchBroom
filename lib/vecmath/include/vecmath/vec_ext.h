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

#include "vec.h"

#include <array>
#include <vector>

namespace vm {
/**
 * Adds the given vector to each of the vectors in the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector of vectors
 * @param rhs the right hand vector
 * @return a vector containing the sum of each of the vectors in the given vector with the right
 * hand vector
 */
template <typename T, std::size_t S>
std::vector<vec<T, S>> operator+(const std::vector<vec<T, S>>& lhs, const vec<T, S>& rhs) {
  std::vector<vec<T, S>> result;
  result.reserve(lhs.size());
  for (const auto& vec : lhs) {
    result.push_back(vec + rhs);
  }
  return result;
}

/**
 * Adds the given vector to each of the vectors in the given array.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam N the number of elements in the given array
 * @param lhs the array of vectors
 * @param rhs the right hand vector
 * @return an array containing the sum of each of the vectors in the given array with the right hand
 * vector
 */
template <typename T, std::size_t S, std::size_t N>
constexpr std::array<vec<T, S>, N> operator+(
  const std::array<vec<T, S>, N>& lhs, const vec<T, S>& rhs) {
  auto result = std::array<vec<T, S>, N>{};
  for (std::size_t i = 0; i < N; ++i) {
    result[i] = lhs[i] + rhs;
  }
  return result;
}

/**
 * Adds the given vector to each of the vectors in the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the vector of vectors
 * @return a vector containing the sum of each of the vectors in the given vector with the left hand
 * vector
 */
template <typename T, std::size_t S>
std::vector<vec<T, S>> operator+(const vec<T, S>& lhs, const std::vector<vec<T, S>>& rhs) {
  return rhs + lhs;
}

/**
 * Adds the given vector to each of the vectors in the given array.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam N the number of elements in the given array
 * @param lhs the left hand vector
 * @param rhs the array of vectors
 * @return an array containing the sum of each of the vectors in the given array with the left hand
 * vector
 */
template <typename T, std::size_t S, std::size_t N>
constexpr std::array<vec<T, S>, N> operator+(
  const vec<T, S>& lhs, const std::array<vec<T, S>, N>& rhs) {
  return rhs + lhs;
}

/**
 * Multiplies each vector in the given range by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the range of vectors
 * @param rhs the scalar factor
 * @return a range containing the scalar product of each vector in the given vector with the given
 * scalar
 */
template <typename T, std::size_t S>
std::vector<vec<T, S>> operator*(const std::vector<vec<T, S>>& lhs, const T rhs) {
  std::vector<vec<T, S>> result;
  result.reserve(lhs.size());
  for (const auto& vec : lhs) {
    result.push_back(vec * rhs);
  }
  return result;
}

/**
 * Multiplies each vector in the given array by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the array of vectors
 * @param rhs the scalar factor
 * @return an array containing the scalar product of each vector in the given array with the given
 * scalar
 */
template <typename T, std::size_t S, std::size_t N>
constexpr std::array<vec<T, S>, N> operator*(const std::array<vec<T, S>, N>& lhs, const T rhs) {
  auto result = std::array<vec<T, S>, N>{};
  for (std::size_t i = 0u; i < N; ++i) {
    result[i] = lhs[i] * rhs;
  }
  return result;
}

/**
 * Multiplies each vector in the given vector by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar factor
 * @param rhs the vector of vectors
 * @return a vector containing the scalar product of each vector in the given vector with the given
 * scalar
 */
template <typename T, std::size_t S>
std::vector<vec<T, S>> operator*(const T lhs, const std::vector<vec<T, S>>& rhs) {
  return rhs * lhs;
}

/**
 * Multiplies each vector in the given array by the given scalar.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar factor
 * @param rhs the array of vectors
 * @return an array containing the scalar product of each vector in the given array with the given
 * scalar
 */
template <typename T, std::size_t S, std::size_t N>
constexpr std::array<vec<T, S>, N> operator*(const T lhs, const std::array<vec<T, S>, N>& rhs) {
  return rhs * lhs;
}
} // namespace vm
