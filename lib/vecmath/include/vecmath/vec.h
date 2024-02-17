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

#include "vecmath/constants.h"
#include "vecmath/constexpr_util.h"
#include "vecmath/scalar.h"

#include <cassert>
#include <cstddef>
#include <type_traits>

namespace vm
{
template <typename T, std::size_t S>
class vec
{
public:
  using float_type = vec<float, S>;
  using type = T;
  static constexpr std::size_t size = S;

public:
  /**
   * The vector components.
   */
  T v[S];

public:
  /* ========== constructors and assignment operators ========== */

  /**
   * Creates a new vector with all components initialized to 0.
   */
  constexpr vec()
    : v{}
  {
  }

  // Copy and move constructors
  vec(const vec<T, S>& other) = default;
  vec(vec<T, S>&& other) noexcept = default;

  // Assignment operators
  vec<T, S>& operator=(const vec<T, S>& other) = default;
  vec<T, S>& operator=(vec<T, S>&& other) noexcept = default;

  /**
   * Creates a new vector from the values in the given initializer list. If the given list
   * has fewer elements than the size of this vector, then the remaining components are
   * set to 0. If the given list has more elements than the size of this vector, then the
   * surplus elements are ignored.
   *
   * @param values the values
   */
  constexpr vec(std::initializer_list<T> values)
    : v{}
  {
    auto it = std::begin(values);
    for (std::size_t i = 0u; i < vm::min(values.size(), S); ++i)
    {
      v[i] = *it++;
    }
  }

  /**
   * Creates a new vector with the components initialized to the given values. The values
   * are converted to the component type T using static_cast. The number of values must
   * match the number of components S.
   *
   * @tparam A1 the type of the first value
   * @tparam Args the types of the remaining values
   * @param a1 the first value
   * @param args the remaining values
   */
  template <typename A1, typename... Args>
  constexpr explicit vec(const A1 a1, const Args... args)
    : v{static_cast<T>(a1), static_cast<T>(args)...}
  {
    static_assert(sizeof...(args) == S - 1u, "Wrong number of parameters");
  }

  /**
   * Creates a new vector with the components initialized to the values of the
   * corresponding components of the given vector. If the given vector has a different
   * component type, its components are converted using static_cast.
   *
   * @tparam U the component type of the given vector
   * @param other the vector to copy
   */
  template <typename U, std::size_t V>
  constexpr explicit vec(const vec<U, V>& other)
    : v{}
  {
    for (std::size_t i = 0u; i < vm::min(S, V); ++i)
    {
      v[i] = static_cast<T>(other[i]);
    }
  }

  /**
   * Creates a new vector by copying the values of the given vector of smaller size and
   * filling up the remaining members using the remaining given values. The components of
   * the given vector and the remaining given values are converted to the component type T
   * using static_cast.
   *
   * @tparam U the component type of the given vector
   * @tparam SS the number of components of the given vector
   * @tparam A1 the type of the first given remaining value
   * @tparam Args the types of the other given remaining values
   * @param other the vector whose values to copy
   * @param a1 the first given remaining value
   * @param args the other given remaining values
   */
  template <typename U, std::size_t SS, typename A1, typename... Args>
  constexpr vec(const vec<U, SS>& other, const A1 a1, const Args... args)
    : v{}
  {
    static_assert(SS + sizeof...(Args) + 1 == S, "Wrong number of parameters");
    for (std::size_t i = 0u; i < SS; ++i)
    {
      v[i] = static_cast<T>(other[i]);
    }

    const T t[S - SS]{static_cast<T>(a1), static_cast<T>(args)...};
    for (std::size_t i = 0u; i < S - SS; ++i)
    {
      v[i + SS] = t[i];
    }
  }

public:
  /* ========== factory methods ========== */
  /**
   * Returns a vector where all components are set to the given value.
   *
   * @param value the value to set
   * @return the newly created vector
   */
  static constexpr vec<T, S> fill(const T value)
  {
    vec<T, S> result;
    for (size_t i = 0; i < S; ++i)
    {
      result[i] = value;
    }
    return result;
  }

  /**
   * Returns a vector with the component at the given index set to 1, and all others set
   * to 0.
   *
   * @param index the index of the component to set to 1
   * @return the newly created vector
   */
  static constexpr vec<T, S> axis(const std::size_t index)
  {
    vec<T, S> axis;
    axis[index] = static_cast<T>(1.0);
    return axis;
  }

public:
  /* ========== accessors ========== */

  /**
   * Returns a copy of the component at the given index. The index is not checked at
   * runtime.
   *
   * @param i the index of the component
   * @return a copy of the compononent at the given index
   */
  constexpr T operator[](const std::size_t i) const { return v[i]; }

  /**
   * Returns a reference to the component at the given index. The index is not checked at
   * runtime.
   *
   * @param i the index of the component
   * @return a reference to the compononent at the given index
   */
  constexpr T& operator[](const std::size_t i) { return v[i]; }

  /**
   * Returns the value of the first component.
   *
   * @return the value of the first component
   */
  template <std::size_t SS = S>
  constexpr T x(typename std::enable_if<SS >= 1>::type* = nullptr) const
  {
    static_assert(S > 0);
    return v[0];
  }

  /**
   * Returns the value of the second component.
   *
   * @return the value of the second component
   */
  template <std::size_t SS = S>
  constexpr T y(typename std::enable_if<SS >= 2>::type* = nullptr) const
  {
    static_assert(S > 1);
    return v[1];
  }

  /**
   * Returns the value of the third component.
   *
   * @return the value of the third component
   */
  template <std::size_t SS = S>
  constexpr T z(typename std::enable_if<SS >= 3>::type* = nullptr) const
  {
    static_assert(S > 2);
    // cppcheck-suppress arrayIndexOutOfBounds
    return v[2];
  }

  /**
   * Returns the value of the fourth component.
   *
   * @return the value of the fourth component
   */
  template <std::size_t SS = S>
  constexpr T w(typename std::enable_if<SS >= 4>::type* = nullptr) const
  {
    static_assert(S > 3);
    // cppcheck-suppress arrayIndexOutOfBounds
    return v[3];
  }

  /**
   * Returns a vector with the values of the first and second component.
   *
   * @return a vector with the values of the first and second component
   */
  constexpr vec<T, 2> xy() const
  {
    static_assert(S > 1);
    return vec<T, 2>(x(), y());
  }

  /**
   * Returns a vector with the values of the first and third component.
   *
   * @return a vector with the values of the first and third component
   */
  constexpr vec<T, 2> xz() const
  {
    static_assert(S > 1);
    return vec<T, 2>(x(), z());
  }

  /**
   * Returns a vector with the values of the second and third component.
   *
   * @return a vector with the values of the second and third component
   */
  constexpr vec<T, 2> yz() const
  {
    static_assert(S > 1);
    return vec<T, 2>(y(), z());
  }

  /**
   * Returns a vector with the values of the first three components.
   *
   * @return a vector with the values of the first three components
   */
  constexpr vec<T, 3> xyz() const
  {
    static_assert(S > 2);
    return vec<T, 3>(x(), y(), z());
  }

  /**
   * Returns a vector with the values of the first four components.
   *
   * @return a vector with the values of the first four components
   */
  constexpr vec<T, 4> xyzw() const
  {
    static_assert(S > 3);
    return vec<T, 4>(x(), y(), z(), w());
  }

public:
  /**
   * Returns a vector with the first component set to 1 and all other components set to 0.
   */
  static constexpr vec<T, S> pos_x()
  {
    constexpr auto result = axis(0);
    return result;
  }

  /**
   * Returns a vector with the second component set to 1 and all other components set to
   * 0.
   */
  static constexpr vec<T, S> pos_y()
  {
    constexpr auto result = axis(1);
    return result;
  }

  /**
   * Returns a vector with the third component set to 1 and all other components set to 0.
   */
  static constexpr vec<T, S> pos_z()
  {
    constexpr auto result = axis(2);
    return result;
  }

  /**
   * Returns a vector with the first component set to -1 and all other components set to
   * 0.
   */
  static constexpr vec<T, S> neg_x()
  {
    constexpr auto result = -axis(0);
    return result;
  }

  /**
   * Returns a vector with the second component set to -1 and all other components set to
   * 0.
   */
  static constexpr vec<T, S> neg_y()
  {
    constexpr auto result = -axis(1);
    return result;
  }

  /**
   * Returns a vector with the third component set to -1 and all other components set to
   * 0.
   */
  static constexpr vec<T, S> neg_z()
  {
    constexpr auto result = -axis(2);
    return result;
  }

  /**
   * Returns a vector with all components set to 0.
   */
  static constexpr vec<T, S> zero()
  {
    constexpr auto result = fill(static_cast<T>(0));
    return result;
  }

  /**
   * Returns a vector with all components set to 1.
   */
  static constexpr vec<T, S> one()
  {
    constexpr auto result = fill(static_cast<T>(1));
    return result;
  }

  /**
   * Returns a vector with all components set to NaN.
   */
  static constexpr vec<T, S> nan()
  {
    constexpr auto result = fill(std::numeric_limits<T>::quiet_NaN());
    return result;
  }

  /**
   * Returns a vector with all components set to the minimal possible value.
   */
  static constexpr vec<T, S> min()
  {
    constexpr auto result = fill(std::numeric_limits<T>::min());
    return result;
  }

  /**
   * Returns a vector with all components set to the maximal possible value.
   */
  static constexpr vec<T, S> max()
  {
    constexpr auto result = fill(std::numeric_limits<T>::max());
    return result;
  }

  // FIXME: this is only here because TB's VertexToolBase needs it, it should be moved
  // elsewhere
  /**
   * Adds the given range of vertices to the given output iterator.
   *
   * @tparam I the range iterator type
   * @tparam O the output iterator type
   * @param cur the range start
   * @param end the range end
   * @param out the output iterator
   */
  template <typename I, typename O>
  static constexpr void get_vertices(I cur, I end, O out)
  {
    while (cur != end)
    {
      out++ = *cur++;
    }
  }
};

/* ========== comparison operators ========== */

/**
 * Lexicographically compares the given components of the vectors using the given epsilon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return -1 if the left hand size is less than the right hand size, +1 if the left hand
 * size is greater than the right hand size, and 0 if both sides are equal
 */
template <typename T, std::size_t S>
constexpr int compare(
  const vec<T, S>& lhs, const vec<T, S>& rhs, const T epsilon = T(0.0))
{
  for (size_t i = 0; i < S; ++i)
  {
    // NaN handling: sort NaN's above non-NaN's, otherwise they would compare equal to any
    // non-nan value since both the < and > tests below fail. Note that this function will
    // compare NaN and nan as equal.
    const bool lhsIsNaN = (lhs[i] != lhs[i]);
    const bool rhsIsNaN = (rhs[i] != rhs[i]);
    if (!lhsIsNaN && rhsIsNaN)
    {
      return -1;
    }
    else if (lhsIsNaN && !rhsIsNaN)
    {
      return 1;
    }
    else if (!lhsIsNaN && !rhsIsNaN)
    {
      if (lhs[i] < rhs[i] - epsilon)
      {
        return -1;
      }
      else if (lhs[i] > rhs[i] + epsilon)
      {
        return 1;
      }
    }
  }
  return 0;
}

/**
 * Performs a pairwise lexicographical comparison of the pairs of vectors given by the two
 * ranges. This function iterates over both ranges in a parallel fashion, and compares the
 * two current elements lexicagraphically until one range ends. If the end of a range is
 * reached, that range is considered less if the end of the other range has not yet been
 * reached. Otherwise, the two ranges of the same length, and are considered to be
 * identical.
 *
 * @tparam T the type of the epsilon parameter
 * @tparam I1 the iterator type of the first range
 * @tparam I2 the iterator type of the second range
 * @param lhsCur the beginning of the left hand range
 * @param lhsEnd the end of the left hand range
 * @param rhsCur the beginning of the right hand range
 * @param rhsEnd the end of the right hand range
 * @param epsilon the epsilon value for component wise comparison
 * @return -1 if the left hand range is less than the right hand range, +1 if the left
 * hand range is greater than the right hand range, and 0 if both ranges are equal
 */
template <typename T, typename I1, typename I2>
constexpr int compare(
  I1 lhsCur, I1 lhsEnd, I2 rhsCur, I2 rhsEnd, const T epsilon = static_cast<T>(0.0))
{
  while (lhsCur != lhsEnd && rhsCur != rhsEnd)
  {
    const auto cmp = compare(*lhsCur, *rhsCur, epsilon);
    if (cmp < 0)
    {
      return -1;
    }
    else if (cmp > 0)
    {
      return +1;
    }
    ++lhsCur;
    ++rhsCur;
  }

  if (rhsCur != rhsEnd)
  {
    return -1;
  }
  else if (lhsCur != lhsEnd)
  {
    return +1;
  }
  else
  {
    return 0;
  }
}

/**
 * Checks whether the given vectors are component wise equal up to the given epsilon.
 *
 * Unlike the equality operator ==, this function takes an epsilon value into account.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @param epsilon the epsilon value
 * @return true if the given vectors are component wise equal up to the given epsilon
 * value
 */
template <typename T, std::size_t S>
constexpr bool is_equal(const vec<T, S>& lhs, const vec<T, S>& rhs, const T epsilon)
{
  return compare(lhs, rhs, epsilon) == 0;
}

/**
 * Compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) == 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given vectors have equal values for each component, and false
 * otherwise
 */
template <typename T, std::size_t S>
constexpr bool operator==(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return compare(lhs, rhs) == 0;
}

/**
 * Compares the given vectors component wise. Equivalent to compare(lhs, rhs, 0.0) != 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given vectors do not have equal values for each component, and
 * false otherwise
 */
template <typename T, std::size_t S>
constexpr bool operator!=(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return compare(lhs, rhs) != 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs,
 * rhs, 0.0) < 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is less than the given right hand vector
 */
template <typename T, std::size_t S>
constexpr bool operator<(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return compare(lhs, rhs) < 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs,
 * rhs, 0.0)
 * <= 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is less than or equal to the given right
 * hand vector
 */
template <typename T, std::size_t S>
constexpr bool operator<=(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return compare(lhs, rhs) <= 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs,
 * rhs, 0.0) > 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is greater than than the given right hand
 * vector
 */
template <typename T, std::size_t S>
constexpr bool operator>(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return compare(lhs, rhs) > 0;
}

/**
 * Lexicographically compares the given vectors component wise. Equivalent to compare(lhs,
 * rhs, 0.0)
 * >= 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return true if the given left hand vector is greater than or equal to than the given
 * right hand vector
 */
template <typename T, std::size_t S>
constexpr bool operator>=(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return compare(lhs, rhs) >= 0;
}

/* ========== slicing ========== */

/**
 * Extracts a slice of the given vector.
 *
 * @tparam RS the number of components of the slice
 * @tparam T the component type
 * @tparam S the number of components of the given vector
 * @param vector the vector to slice
 * @param offset the index of the first element of the slice
 * @return the slice
 */
template <std::size_t RS, typename T, std::size_t S>
constexpr vec<T, RS> slice(const vec<T, S>& vector, const std::size_t offset)
{
  static_assert(RS <= S, "slice must not exceed vector");
  assert(offset <= S - RS);

  vec<T, RS> result;
  for (std::size_t i = 0u; i < RS; ++i)
  {
    result[i] = vector[i + offset];
  }
  return result;
}

/* ========== sorting and finding components ========== */

namespace detail
{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
#endif
template <typename T>
struct index_pair
{
  T element;
  std::size_t index;

  constexpr index_pair()
    : element{}
    , index{}
  {
  }
};
#ifdef __clang
#pragma clang diagnostic pop
#endif

template <typename T>
using vector_index_element = index_pair<T>;

template <typename T, std::size_t S>
using vector_index_elements = vector_index_element<T>[S];

template <typename T, std::size_t S, typename Cmp>
constexpr void sort_vector(
  const vec<T, S>& vector, const Cmp& cmp, vector_index_elements<T, S>& elements)
{
  for (std::size_t i = 0u; i < S; ++i)
  {
    elements[i].element = vector[i];
    elements[i].index = i;
  }
  sort(std::begin(elements), std::end(elements), cmp);
}
} // namespace detail

/**
 * Returns the index of the k-largest component of the given vector. If there are multiple
 * such components, then an index of any such component may be returned.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vector the vector
 * @param k the value of k
 * @return the index of a k-largest component of the given vector
 */
template <typename T, std::size_t S>
constexpr std::size_t find_max_component(
  const vec<T, S>& vector, const std::size_t k = 0u)
{
  assert(k < S);

  constexpr auto cmp = [](const auto& lhs, const auto& rhs) {
    return lhs.element < rhs.element;
  };

  detail::vector_index_elements<T, S> elements;
  detail::sort_vector(vector, cmp, elements);
  return elements[S - k - 1u].index;
}

/**
 * Returns the index of the k-largest absolute component of the given vector. If there are
 * multiple such components, then an index of any such component may be returned.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vector the vector
 * @param k the value of k
 * @return the index of a k-largest absolute component of the given vector
 */
template <typename T, std::size_t S>
constexpr std::size_t find_abs_max_component(
  const vec<T, S>& vector, const std::size_t k = 0u)
{
  assert(k < S);

  constexpr auto cmp = [](const auto& lhs, const auto& rhs) {
    return abs(lhs.element) < abs(rhs.element);
  };

  detail::vector_index_elements<T, S> elements;
  detail::sort_vector(vector, cmp, elements);
  return elements[S - k - 1u].index;
}

/**
 * Returns the coordinate system axis with the index of the k-largest absolute component
 * of the given vector. If there are multiple such components, then any such axis may be
 * returned.
 *
 * Example: Given vector (-1, 3, -5) and k=0 returns the negative Z axis. Given the same
 * vector and k=1 returns the positive Y axis.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vector the vector
 * @param k the value of k
 * @return the axis of the k-largest absolute component of the given vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> get_abs_max_component_axis(
  const vec<T, S>& vector, const std::size_t k = 0u)
{
  const auto index = find_abs_max_component(vector, k);
  const auto result = vec<T, S>::axis(index);
  return vector[index] < static_cast<T>(0) ? -result : result;
}

/**
 * Returns the value of the k-largest component of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vector the vector
 * @param k the value of k
 * @return the value of a k-largest component value of the given vector
 */
template <typename T, std::size_t S>
constexpr T get_max_component(vec<T, S> vector, const std::size_t k = 0u)
{
  assert(k < S);

  constexpr auto cmp = [](const auto& lhs, const auto& rhs) { return lhs < rhs; };

  detail::sort(std::begin(vector.v), std::end(vector.v), cmp);
  return vector[S - k - 1u];
}

/**
 * Returns the value of the k-largest absolute component of the given vector. If there are
 * multiple such components, then any such component value may be returned.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vector the vector
 * @param k the value of k
 * @return the value of a k-largest component of the given vector
 */
template <typename T, std::size_t S>
constexpr T get_abs_max_component(vec<T, S> vector, const std::size_t k = 0u)
{
  constexpr auto cmp = [](const auto& lhs, const auto& rhs) {
    return abs(lhs) < abs(rhs);
  };

  detail::sort(std::begin(vector.v), std::end(vector.v), cmp);
  return vector[S - k - 1u];
}

/* ========== arithmetic operators ========== */

/**
 * Returns a copy of this vector.
 *
 * @return the copy
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator+(const vec<T, S>& vector)
{
  return vector;
}

/**
 * Returns an inverted copy of this vector. The copy is inverted by negating every
 * component.
 *
 * @return the inverted copy
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator-(const vec<T, S>& vector)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = -vector[i];
  }
  return result;
}

/**
 * Returns the sum of the given vectors, which is computed by adding all of their
 * components.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the sum of the given two vectors
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator+(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = lhs[i] + rhs[i];
  }
  return result;
}

/**
 * Returns the difference of the given vectors, which is computed by subtracting the
 * corresponding components of the right hand vector from the components of the left hand
 * vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the difference of the given two vectors
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator-(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = lhs[i] - rhs[i];
  }
  return result;
}

/**
 * Returns the product of the given vectors, which is computed by multiplying the
 * corresponding components of the right hand vector with the components of the left hand
 * vector. Note that this does not compute either the inner (or dot) product or the outer
 * (or cross) product.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the product of the given two vectors
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator*(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = lhs[i] * rhs[i];
  }
  return result;
}

/**
 * Returns the product of the given vector and scalar factor, which is computed by
 * multiplying each component of the vector with the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar
 * @return the scalar product of the given vector with the given factor
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator*(const vec<T, S>& lhs, const T rhs)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = lhs[i] * rhs;
  }
  return result;
}

/**
 * Returns the product of the given vector and scalar factor, which is computed by
 * multiplying each component of the vector with the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar
 * @param rhs the vector
 * @return the scalar product of the given vector with the given factor
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator*(const T lhs, const vec<T, S>& rhs)
{
  return vec<T, S>(rhs) * lhs;
}

/**
 * Returns the division of the given vectors, which is computed by dividing the
 * corresponding components of the left hand vector by the components of the right hand
 * vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the division of the given two vectors
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator/(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = lhs[i] / rhs[i];
  }
  return result;
}

/**
 * Returns the division of the given vector and scalar factor, which is computed by
 * dividing each component of the vector by the factor.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the vector
 * @param rhs the scalar
 * @return the scalar division of the given vector with the given factor
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator/(const vec<T, S>& lhs, const T rhs)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = lhs[i] / rhs;
  }
  return result;
}

/**
 * Returns the division of the given vector and scalar factor, which is computed by
 * dividing the factor by each component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the scalar
 * @param rhs the vector
 * @return the scalar division of the given factor with the given vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> operator/(const T lhs, const vec<T, S>& rhs)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = lhs / rhs[i];
  }
  return result;
}

/* ========== arithmetic functions ========== */

/**
 * Returns a vector where each component is the minimum of the corresponding components of
 * the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first vector
 * @param rhs the second vector
 * @param rest the remaining vectors
 * @return the component wise minimum of the given vectors
 */
template <typename T, std::size_t S, typename... Rest>
constexpr vec<T, S> min(const vec<T, S>& lhs, const vec<T, S>& rhs, Rest... rest)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = min(lhs[i], rhs[i], rest[i]...);
  }
  return result;
}

/**
 * Returns a vector where each component is the maximum of the corresponding components of
 * the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first vector
 * @param rhs the second vector
 * @param rest the remaining vectors
 * @return the component wise maximum of the given vectors
 */
template <typename T, std::size_t S, typename... Rest>
constexpr vec<T, S> max(const vec<T, S>& lhs, const vec<T, S>& rhs, Rest... rest)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = max(lhs[i], rhs[i], rest[i]...);
  }
  return result;
}

/**
 * Returns a vector where each component is the absolute minimum of the corresponding
 * components of the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first vector
 * @param rhs the second vector
 * @param rest the remaining vectors
 * @return the component wise absolute minimum of the given vectors
 */
template <typename T, std::size_t S, typename... Rest>
constexpr vec<T, S> abs_min(const vec<T, S>& lhs, const vec<T, S>& rhs, Rest... rest)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = abs_min(lhs[i], rhs[i], rest[i]...);
  }
  return result;
}

/**
 * Returns a vector where each component is the absolute maximum of the corresponding
 * components of the given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @tparam Rest the types of the remaining arguments
 * @param lhs the first vector
 * @param rhs the second vector
 * @param rest the remaining vectors
 * @return the component wise absolute maximum of the given vectors
 */
template <typename T, std::size_t S, typename... Rest>
constexpr vec<T, S> abs_max(const vec<T, S>& lhs, const vec<T, S>& rhs, Rest... rest)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = abs_max(lhs[i], rhs[i], rest[i]...);
  }
  return result;
}

/**
 * Returns a vector with each component clamped to the ranges defined in by the
 * corresponding components of the given minimum and maximum vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the value to clamp
 * @param minVal the minimum values
 * @param maxVal the maximum values
 * @return the clamped vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> clamp(
  const vec<T, S>& v, const vec<T, S>& minVal, const vec<T, S>& maxVal)
{
  return min(max(v, minVal), maxVal);
}

/**
 * Returns a vector where each component is the absolute value of the corresponding
 * component of the the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to make absolute
 * @return the absolute vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> abs(const vec<T, S>& v)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = abs(v[i]);
  }
  return result;
}

/**
 * Returns a vector where each component indicates the sign of the corresponding
 * components of the given vector.
 *
 * For each component, the returned vector has a value of
 * - -1 if the corresponding component of the given vector is less than 0
 * - +1 if the corresponding component of the given vector is greater than 0
 * -  0 if the corresponding component of the given vector is equal to 0
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v a vector
 * @return a vector indicating the signs of the components of the given vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> sign(const vec<T, S>& v)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = sign(v[i]);
  }
  return result;
}

/**
 * Returns a vector where each component is set to 0 if the corresponding component of the
 * given vector is less than the corresponding component of the given edge vector, and 1
 * otherwise.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v a vector
 * @param e the edge vector
 * @return a vector indicating whether the given value is less than the given edge value
 * or not
 */
template <typename T, std::size_t S>
constexpr vec<T, S> step(const vec<T, S>& e, const vec<T, S>& v)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = step(e[i], v[i]);
  }
  return result;
}

/**
 * Performs performs smooth Hermite interpolation for each component x of the given vector
 * between 0 and 1 when e0[i] < v[i] < e1[i].
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param e0 the lower edge values
 * @param e1 the upper edge values
 * @param v the vector to interpolate
 * @return the interpolated vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> smoothstep(
  const vec<T, S>& e0, const vec<T, S>& e1, const vec<T, S>& v)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = smoothstep(e0[i], e1[i], v[i]);
  }
  return result;
}

/**
 * Returns the dot product (also called inner product) of the two given vectors.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the dot product of the given vectors
 */
template <typename T, std::size_t S>
constexpr T dot(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  auto result = static_cast<T>(0.0);
  for (size_t i = 0; i < S; ++i)
  {
    result += (lhs[i] * rhs[i]);
  }
  return result;
}

/**
 * Returns the cross product (also called outer product) of the two given 3d vectors.
 *
 * @tparam T the component type
 * @param lhs the left hand vector
 * @param rhs the right hand vector
 * @return the cross product of the given vectors
 */
template <typename T>
constexpr vec<T, 3> cross(const vec<T, 3>& lhs, const vec<T, 3>& rhs)
{
  return vec<T, 3>(
    lhs[1] * rhs[2] - lhs[2] * rhs[1],
    lhs[2] * rhs[0] - lhs[0] * rhs[2],
    lhs[0] * rhs[1] - lhs[1] * rhs[0]);
}

/**
 * Returns the squared length of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to normalize
 * @return the squared length of the given vector
 */
template <typename T, std::size_t S>
constexpr T squared_length(const vec<T, S>& vec)
{
  return dot(vec, vec);
}

/**
 * Returns the length of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to return the length of
 * @return the length of the given vector
 */
template <typename T, std::size_t S>
T length(const vec<T, S>& vec)
{
  return sqrt(squared_length(vec));
}

/**
 * Returns the length of the given vector at compile time.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to return the length of
 * @return the length of the given vector
 */
template <typename T, std::size_t S>
constexpr T length_c(const vec<T, S>& vec)
{
  return sqrt_c(squared_length(vec));
}

/**
 * Normalizes the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to return the squared length of
 * @return the normalized vector
 */
template <typename T, std::size_t S>
vec<T, S> normalize(const vec<T, S>& vec)
{
  return vec / length(vec);
}

/**
 * Normalizes the given vector at compile time.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param vec the vector to return the squared length of
 * @return the normalized vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> normalize_c(const vec<T, S>& vec)
{
  return vec / length_c(vec);
}

/**
 * Rearranges the components of the given vector depending on the value of the axis
 * parameter as follows:
 *
 * - 1: x y z -> y z x
 * - 2: x y z -> z x y
 * - 3: x y z -> x y z
 *
 * @tparam T the component type
 * @param point the point to swizzle
 * @param axis the axis
 * @return the swizzled point
 */
template <typename T>
constexpr vec<T, 3> swizzle(const vec<T, 3>& point, const std::size_t axis)
{
  assert(axis <= 3);
  switch (axis)
  {
  case 0: // x y z -> y z x
    return vec<T, 3>(point.y(), point.z(), point.x());
  case 1: // x y z -> z x y
    return vec<T, 3>(point.z(), point.x(), point.y());
  default:
    return point;
  }
}

/**
 * Rearranges the components of the given vector depending on the value of the axis
 * parameter so that it undoes the effect of calling swizzle.
 *
 * @tparam T the component type
 * @param point the point to swizzle
 * @param axis the axis
 * @return the unswizzled point
 */
template <typename T>
constexpr vec<T, 3> unswizzle(const vec<T, 3>& point, const std::size_t axis)
{
  assert(axis <= 3);
  switch (axis)
  {
  case 0:
    return vec<T, 3>(point.z(), point.x(), point.y());
  case 1:
    return vec<T, 3>(point.y(), point.z(), point.x());
  default:
    return point;
  }
}

/**
 * Checks whether the given vector has unit length (1).
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @param epsilon the epsilon value
 * @return true if the given vector has a length of 1 and false otherwise
 */
template <typename T, std::size_t S>
bool is_unit(const vec<T, S>& v, const T epsilon)
{
  return is_equal(length(v), T(1.0), epsilon);
}

/**
 * Checks whether the given vector has unit length (1) at compile time.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @param epsilon the epsilon value
 * @return true if the given vector has a length of 1 and false otherwise
 */
template <typename T, std::size_t S>
constexpr bool is_unit_c(const vec<T, S>& v, const T epsilon)
{
  return is_equal(length_c(v), T(1.0), epsilon);
}

/**
 * Checks whether the given vector has a length of 0.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @param epsilon the epsilon value
 * @return true if the given vector has a length of 0 and false otherwise
 */
template <typename T, std::size_t S>
constexpr bool is_zero(const vec<T, S>& v, const T epsilon)
{
  for (size_t i = 0; i < S; ++i)
  {
    if (!is_zero(v[i], epsilon))
    {
      return false;
    }
  }
  return true;
}

/**
 * Checks whether the given vector NaN as any component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @return true if the given vector has NaN as any component
 */
template <typename T, std::size_t S>
constexpr bool is_nan(const vec<T, S>& v)
{
  for (size_t i = 0; i < S; ++i)
  {
    if (is_nan(v[i]))
    {
      return true;
    }
  }
  return false;
}

/**
 * Checks whether each component of the given vector is within a distance of epsilon
 * around an integral value.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to check
 * @param epsilon the epsilon value
 * @return true if all components of the given vector are integral under the above
 * definition
 */
template <typename T, std::size_t S>
constexpr bool is_integral(const vec<T, S>& v, const T epsilon = static_cast<T>(0.0))
{
  for (size_t i = 0; i < S; ++i)
  {
    if (abs(v[i] - round(v[i])) > epsilon)
    {
      return false;
    }
  }
  return true;
}

/**
 * Mixes the given two vectors using the given factors. For each component i of the given
 * vectors, the corresponding component of the result is computed as
 *
 *   (1 - f[i]) * lhs[i] + f * rhs[i]
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @param f the mixing factors
 * @return the mixed vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> mix(const vec<T, S>& lhs, const vec<T, S>& rhs, const vec<T, S>& f)
{
  return (vec<T, S>::one() - f) * lhs + f * rhs;
}

/**
 * Returns a vector with each component set to the fractional part of the corresponding
 * component of the given vector.
 *
 * Note that this function differs from GLSL's fract, which behaves wrongly for negative
 * values. Return 0.9 for fract(-0.1). This function will correctly return -0.1 in this
 * case.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector
 * @return the fractional vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> fract(const vec<T, S>& v)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = fract(v[i]);
  }
  return result;
}

/**
 * Returns a vector with each component set to the floating point remainder of the
 * division of v over f. So for each component i, it holds that result[i] = mod(v[i],
 * f[i]).
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the dividend
 * @param f the divisor
 * @return the fractional remainder
 */
template <typename T, std::size_t S>
constexpr vec<T, S> mod(const vec<T, S>& v, const vec<T, S>& f)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = mod(v[i], f[i]);
  }
  return result;
}

/**
 * Computes the distance between two given points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first point
 * @param rhs the second point
 * @return the distance between the given points
 */
template <typename T, std::size_t S>
T distance(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return length(lhs - rhs);
}

/**
 * Computes the distance between two given points at compile time.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first point
 * @param rhs the second point
 * @return the distance between the given points
 */
template <typename T, std::size_t S>
constexpr T distance_c(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return length_c(lhs - rhs);
}

/**
 * Computes the squared distance between two given points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first point
 * @param rhs the second point
 * @return the squared distance between the given points
 */
template <typename T, std::size_t S>
constexpr T squared_distance(const vec<T, S>& lhs, const vec<T, S>& rhs)
{
  return squared_length(lhs - rhs);
}

/**
 * Converts the given point in cartesian coordinates to homogeneous coordinates by
 * embedding the point into a vector with a size increased by 1 and setting the last
 * component to 1.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param point the point in cartesian coordinates
 * @return the point in homogeneous coordinates
 */
template <typename T, std::size_t S>
constexpr vec<T, S + 1> to_homogeneous_coords(const vec<T, S>& point)
{
  return vec<T, S + 1>(point, static_cast<T>(1.0));
}

/**
 * Converts the given point in homogeneous coordinates to cartesian coordinates by
 * dividing all but the last component by the value of the last component.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param point the point in homogeneous coordinates
 * @return the point in cartesian coordinates
 */
template <typename T, std::size_t S>
constexpr vec<T, S - 1> to_cartesian_coords(const vec<T, S>& point)
{
  vec<T, S - 1> result;
  for (size_t i = 0; i < S - 1; ++i)
  {
    result[i] = point[i] / point[S - 1];
  }
  return result;
}

/**
 * Checks whether the given three points are colinear.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param a the first point
 * @param b the second point
 * @param c the third point
 * @param epsilon the epsilon value
 * @return true if the given three points are colinear, and false otherwise
 */
template <typename T, std::size_t S>
constexpr bool is_colinear(
  const vec<T, S>& a,
  const vec<T, S>& b,
  const vec<T, S>& c,
  const T epsilon = constants<T>::colinear_epsilon())
{
  // see http://math.stackexchange.com/a/1778739

  T j = 0.0;
  T k = 0.0;
  T l = 0.0;
  for (size_t i = 0; i < S; ++i)
  {
    const T ac = a[i] - c[i];
    const T ba = b[i] - a[i];
    j += ac * ba;
    k += ac * ac;
    l += ba * ba;
  }

  return is_zero(j * j - k * l, epsilon);
}

/**
 * Checks whether the given vectors are parallel. Two vectors are considered to be
 * parallel if and only if they point in the same or in opposite directions.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @param epsilon the epsilon value
 * @return true if the given vectors are parallel, and false otherwise
 */
template <typename T, std::size_t S>
bool is_parallel(
  const vec<T, S>& lhs,
  const vec<T, S>& rhs,
  const T epsilon = constants<T>::colinear_epsilon())
{
  const T cos = dot(normalize(lhs), normalize(rhs));
  return is_equal(abs(cos), T(1.0), epsilon);
}

/**
 * Checks whether the given vectors are parallel at compile time. Two vectors are
 * considered to be parallel if and only if they point in the same or in opposite
 * directions.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first vector
 * @param rhs the second vector
 * @param epsilon the epsilon value
 * @return true if the given vectors are parallel, and false otherwise
 */
template <typename T, std::size_t S>
constexpr bool is_parallel_c(
  const vec<T, S>& lhs,
  const vec<T, S>& rhs,
  const T epsilon = constants<T>::colinear_epsilon())
{
  const T cos = dot(normalize_c(lhs), normalize_c(rhs));
  return is_equal(abs(cos), T(1.0), epsilon);
}

/* ========== rounding and error correction ========== */

/**
 * Returns a vector with each component set to the largest integer value not greater than
 * the value of the corresponding component of the given vector.
 *
 * @tparam T the component type, which must be a floating point type
 * @tparam S the number of components
 * @param v the value
 * @return a vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> floor(const vec<T, S>& v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a float point type");
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = floor(v[i]);
  }
  return result;
}

/**
 * Returns a vector with each component set to the smallest integer value not less than
 * the value of the corresponding component of the given vector.
 *
 * @tparam T the component type, which must be a floating point type
 * @tparam S the number of components
 * @param v the value
 * @return a vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> ceil(const vec<T, S>& v)
{
  static_assert(std::is_floating_point<T>::value, "T must be a float point type");
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = ceil(v[i]);
  }
  return result;
}

/**
 * Returns a vector with each component set to the nearest integer which is not greater in
 * magnitude than the corresponding component of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to truncate
 * @return the truncated vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> trunc(const vec<T, S>& v)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = trunc(v[i]);
  }
  return result;
}

/**
 * Returns a vector where each component is the rounded value of the corresponding
 * component of the given vector.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round
 * @return the rounded vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> round(const vec<T, S>& v)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = round(v[i]);
  }
  return result;
}

/**
 * Rounds the components of the given vector down to multiples of the components of the
 * given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round down
 * @param m the multiples to round down to
 * @return the rounded vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> snapDown(const vec<T, S>& v, const vec<T, S>& m)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = snapDown(v[i], m[i]);
  }
  return result;
}

/**
 * Rounds the components of the given vector up to multiples of the components of the
 * given vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round down
 * @param m the multiples to round up to
 * @return the rounded vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> snapUp(const vec<T, S>& v, const vec<T, S>& m)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = snapUp(v[i], m[i]);
  }
  return result;
}

/**
 * Rounds the components of the given vector to multiples of the components of the given
 * vector m.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to round down
 * @param m the multiples to round to
 * @return the rounded vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> snap(const vec<T, S>& v, const vec<T, S>& m)
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = snap(v[i], m[i]);
  }
  return result;
}

/**
 * Corrects the given vector's components to the given number of decimal places.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param v the vector to correct
 * @param decimals the number of decimal places to keep
 * @param epsilon the epsilon value
 * @return the corrected vector
 */
template <typename T, std::size_t S>
constexpr vec<T, S> correct(
  const vec<T, S>& v,
  const std::size_t decimals = 0,
  const T epsilon = constants<T>::correct_epsilon())
{
  vec<T, S> result;
  for (size_t i = 0; i < S; ++i)
  {
    result[i] = correct(v[i], decimals, epsilon);
  }
  return result;
}

/**
 * Given three colinear points, this function checks whether the first point is contained
 * in a segment formed by the other two points.
 *
 * The result is undefined for the case of non-colinear points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param p the point to check
 * @param start the segment start
 * @param end the segment end
 * @return true if the given point is contained within the segment
 */
template <typename T, std::size_t S>
bool is_between(const vec<T, S>& p, const vec<T, S>& start, const vec<T, S>& end)
{
  assert(is_colinear(p, start, end));

  if (p == start || p == end)
  {
    return true;
  }
  else
  {
    const auto toStart = start - p;
    const auto toEnd = end - p;

    const auto d = dot(toEnd, normalize(toStart));
    return d < T(0.0);
  }
}

/**
 * Given three colinear points, this function checks whether the first point is contained
 * in a segment formed by the other two points at compile time.
 *
 * The result is undefined for the case of non-colinear points.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param p the point to check
 * @param start the segment start
 * @param end the segment end
 * @return true if the given point is contained within the segment
 */
template <typename T, std::size_t S>
constexpr bool is_between_c(
  const vec<T, S>& p, const vec<T, S>& start, const vec<T, S>& end)
{
  assert(is_colinear(p, start, end));

  if (p == start || p == end)
  {
    return true;
  }
  else
  {
    const auto toStart = start - p;
    const auto toEnd = end - p;

    const auto d = dot(toEnd, normalize_c(toStart));
    return d < T(0.0);
  }
}

/**
 * Computes the average of the given range of elements, using the given function to
 * transform an element into a vector.
 *
 * @tparam I the type of the range iterators
 * @tparam G the type of the transformation function from a range element to a vector type
 * @param cur the start of the range
 * @param end the end of the range
 * @param get the transformation function, defaults to identity
 * @return the average of the vectors obtained from the given range of elements
 */
template <typename I, typename G = identity>
constexpr auto average(I cur, I end, const G& get = G()) ->
  typename std::remove_reference<decltype(get(*cur))>::type
{
  assert(cur != end);

  using T = typename std::remove_reference<decltype(get(*cur))>::type::type;

  auto result = get(*cur++);
  auto count = T(1.0);
  while (cur != end)
  {
    result = result + get(*cur++);
    count = count + T(1.0);
  }
  return result / count;
}

/**
 * Computes the CCW angle between axis and vector in relation to the given up vector. All
 * vectors are expected to be normalized. The CCW angle is the angle by which the given
 * axis must be rotated in CCW direction about the given up vector so that it becomes
 * identical to the given vector.
 *
 * @tparam T the coordinate type
 * @param v the vector
 * @param axis the axis
 * @param up the up vector
 * @return the CCW angle
 */
template <typename T>
T measure_angle(const vec<T, 3>& v, const vec<T, 3>& axis, const vec<T, 3>& up)
{
  const auto cosAngle = clamp(dot(v, axis), T(-1), T(1));
  const auto angle = std::acos(cosAngle);

  const auto perp = cross(axis, v);
  if (dot(perp, up) >= T(0.0))
  {
    return angle;
  }
  else
  {
    return constants<T>::two_pi() - angle;
  }
}
} // namespace vm
