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

#include "constants.h"
#include "vec.h"

#include <cassert>
#include <tuple>

namespace vm
{
template <typename T, std::size_t R, std::size_t C>
class mat
{
public:
  using type = T;
  using column_type = vec<T, R>;
  static const std::size_t rows = R;
  static const std::size_t cols = C;

  /**
   * The matrix components in column major format.
   */
  column_type v[C];

public:
  /* ========== constructors and assignment operators ========== */

  /**
   * Creates a new matrix with its values initialized to an identity matrix.
   */
  constexpr mat()
    : v{}
  {
    for (std::size_t c = 0; c < C; c++)
    {
      for (std::size_t r = 0; r < R; r++)
      {
        v[c][r] = c == r ? static_cast<T>(1.0) : static_cast<T>(0.0);
      }
    }
  }

  // Copy and move constructors
  mat(const mat<T, R, C>& other) = default;
  mat(mat<T, R, C>&& other) noexcept = default;

  // Assignment operators
  mat<T, R, C>& operator=(const mat<T, R, C>& other) = default;
  mat<T, R, C>& operator=(mat<T, R, C>&& other) noexcept = default;

  /**
   * Creates a matrix with the given values.
   *
   * @param values the initializer list with the matrix elements in row-major order
   */
  constexpr mat(std::initializer_list<T> values)
    : v{}
  {
    const auto* list = std::begin(values);
    for (std::size_t c = 0u; c < C; ++c)
    {
      for (std::size_t r = 0u; r < R; ++r)
      {
        v[c][r] = list[c + (C * r)];
      }
    }
  }

  /**
   * Creates a matrix with the given values.
   *
   * @tparam A11 the type of the first argument
   * @tparam Args the types of the remaining arguments
   * @param a11 the value at position 0,0
   * @param args the remaining values in row-major order
   */
  template <typename A11, typename... Args>
  constexpr explicit mat(const A11 a11, const Args... args)
    : v{}
  {
    static_assert(sizeof...(args) + 1u == R * C, "Wrong number of parameters");
    const T t[R * C]{static_cast<T>(a11), static_cast<T>(args)...};
    for (std::size_t c = 0u; c < C; ++c)
    {
      for (std::size_t r = 0u; r < R; ++r)
      {
        v[c][r] = t[c + (C * r)];
      }
    }
  }

  /**
   * Creates a matrix with the elements initialized to the values of the corresponding
   * elements of the given matrix. If the given matrix has a different element type, its
   * elements are converted using static_cast.
   *
   * @tparam U the element type of the given matrix
   * @param other the matrix to copy
   */
  template <typename U>
  constexpr explicit mat(const mat<U, R, C>& other)
    : v{}
  {
    for (std::size_t c = 0u; c < C; ++c)
    {
      for (std::size_t r = 0u; r < R; ++r)
      {
        v[c][r] = static_cast<T>(other[c][r]);
      }
    }
  }

public:
  /* ========== accessors ========== */

  /**
   * Returns the column at the given index.
   *
   * @param index the index of the column to return
   * @return the column at the given index
   */
  constexpr vec<T, R>& operator[](const std::size_t index)
  {
    assert(index < C);
    return v[index];
  }

  /**
   * Returns the column at the given index.
   *
   * @param index the index of the column to return
   * @return the column at the given index
   */
  constexpr const vec<T, R>& operator[](const std::size_t index) const
  {
    assert(index < C);
    return v[index];
  }

public:
  /* ========== factory methods ========== */
  /**
   * Returns a matrix where all elements are set to the given value.
   *
   * @param value the value to set
   * @return the newly created matrix
   */
  static constexpr mat<T, R, C> fill(const T value)
  {
    mat<T, R, C> result;
    for (std::size_t c = 0; c < C; c++)
    {
      for (std::size_t r = 0; r < R; r++)
      {
        result[c][r] = value;
      }
    }
    return result;
  }

public:
  /**
   * Returns the identity matrix.
   */
  static constexpr mat<T, R, C> identity()
  {
    constexpr auto result = mat<T, R, C>();
    return result;
  }

  /**
   * Returns a matrix with all elements set to 0.
   */
  static constexpr mat<T, R, C> zero()
  {
    constexpr auto result = mat<T, R, C>::fill(static_cast<T>(0));
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 90 degrees about the X axis in clockwise
   * direction.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_90_x_cw(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+1, +0, +0, +0, +0, +0, +1, +0, +0, -1, +0, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 90 degrees about the Y axis in clockwise
   * direction.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_90_y_cw(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+0, +0, -1, +0, +0, +1, +0, +0, +1, +0, +0, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 90 degrees about the Z axis in clockwise
   * direction.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_90_z_cw(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+0, +1, +0, +0, -1, +0, +0, +0, +0, +0, +1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 90 degrees about the X axis in counter-clockwise
   * direction.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_90_x_ccw(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+1, +0, +0, +0, +0, +0, -1, +0, +0, +1, +0, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 90 degrees about the Y axis in counter-clockwise
   * direction.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_90_y_ccw(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+0, +0, +1, +0, +0, +1, +0, +0, -1, +0, +0, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 90 degrees about the Z axis in counter-clockwise
   * direction.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_90_z_ccw(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+0, -1, +0, +0, +1, +0, +0, +0, +0, +0, +1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 180 degrees about the X axis.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_180_x(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+1, +0, +0, +0, +0, -1, +0, +0, +0, +0, -1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 180 degrees about the Y axis.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_180_y(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(-1, +0, +0, +0, +0, +1, +0, +0, +0, +0, -1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that rotates by 180 degrees about the Z axis.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> rot_180_z(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(-1, +0, +0, +0, +0, -1, +0, +0, +0, +0, +1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that mirrors along the X axis.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> mirror_x(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(-1, +0, +0, +0, +0, +1, +0, +0, +0, +0, +1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that mirrors along the Y axis.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> mirror_y(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+1, +0, +0, +0, +0, -1, +0, +0, +0, +0, +1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a 4x4 matrix that mirrors along the Z axis.
   */
  template <std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> mirror_z(
    typename std::enable_if<RR == 4 && CC == 4>::type* = nullptr)
  {
    constexpr auto result =
      mat<T, R, C>(+1, +0, +0, +0, +0, +1, +0, +0, +0, +0, -1, +0, +0, +0, +0, +1);
    return result;
  }

  /**
   * Returns a matrix that sets the given component of a vector to 0 if multiplied by it.
   *
   * @tparam E the index of the component to set to zero
   * @return the matrix
   */
  template <std::size_t E, std::size_t RR = R, std::size_t CC = C>
  static constexpr mat<T, R, C> zero_out(
    typename std::enable_if<RR == CC>::type* = nullptr)
  {
    static_assert(E < R, "Element index out of bounds");
    constexpr auto result = set(mat<T, R, C>::identity(), E, E, static_cast<T>(0));
    return result;
  }
};

/* ========== comparison operators ========== */

/**
 * Compares the given two matrices column wise.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @param epsilon the epsilon value
 * @return a negative value if there is a column in the left matrix that compares less
 * than its corresponding column of the right matrix, a positive value in the opposite
 * case, and 0 if all columns compare equal
 */
template <typename T, std::size_t R, std::size_t C>
constexpr int compare(
  const mat<T, R, C>& lhs, const mat<T, R, C>& rhs, const T epsilon = static_cast<T>(0.0))
{
  for (std::size_t c = 0; c < C; c++)
  {
    const auto cmp = compare(lhs[c], rhs[c], epsilon);
    if (cmp != 0)
    {
      return cmp;
    }
  }
  return 0;
}

/**
 * Checks whether the given matrices have equal components.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @param epsilon the epsilon value
 * @return true if all components of the given matrices are equal, and false otherwise
 */
template <typename T, std::size_t R, std::size_t C>
constexpr bool is_equal(const mat<T, R, C>& lhs, const mat<T, R, C>& rhs, const T epsilon)
{
  return compare(lhs, rhs, epsilon) == 0;
}

/**
 * Checks whether all columns of the given matrix are zero.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param m the matrix to check
 * @param epsilon the epsilon value
 * @return true if all columsn of the given matrix are zero
 */
template <typename T, std::size_t R, std::size_t C>
constexpr bool is_zero(const mat<T, R, C>& m, const T epsilon)
{
  for (size_t c = 0; c < C; ++c)
  {
    if (!is_zero(m[c], epsilon))
    {
      return false;
    }
  }
  return true;
}

/**
 * Checks whether the given matrices have identical components.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return true if all components of the given matrices are equal, and false otherwise
 */
template <typename T, std::size_t R, std::size_t C>
constexpr bool operator==(const mat<T, R, C>& lhs, const mat<T, R, C>& rhs)
{
  return compare(lhs, rhs, static_cast<T>(0.0)) == 0;
}

/**
 * Checks whether the given matrices have identical components.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return false if all components of the given matrices are equal, and true otherwise
 */
template <typename T, std::size_t R, std::size_t C>
constexpr bool operator!=(const mat<T, R, C>& lhs, const mat<T, R, C>& rhs)
{
  return compare(lhs, rhs, static_cast<T>(0.0)) != 0;
}

/* ========== arithmetic operators ========== */

/**
 * Returns a copy of the given matrix.
 *
 * @param m the matrix
 * @return a copy of the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> operator+(const mat<T, R, C>& m)
{
  return m;
}

/**
 * Returns a matrix with the negated components of the given matrix.
 *
 * @param m the matrix to negate
 * @return the negated matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> operator-(const mat<T, R, C>& m)
{
  mat<T, R, C> result;
  for (size_t c = 0; c < C; c++)
  {
    result[c] = -m[c];
  }
  return result;
}

/**
 * Computes the sum of two matrices by adding the corresponding components.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return a matrix where each component is the sum of the two corresponding components of
 * the given matrices
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> operator+(const mat<T, R, C>& lhs, const mat<T, R, C>& rhs)
{
  mat<T, R, C> result;
  for (size_t c = 0; c < C; c++)
  {
    result[c] = lhs[c] + rhs[c];
  }
  return result;
}

/**
 * Computes the difference of two matrices by subtracting the corresponding components.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return a matrix where each component is the difference of the two corresponding
 * components of the given matrices
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> operator-(const mat<T, R, C>& lhs, const mat<T, R, C>& rhs)
{
  mat<T, R, C> result;
  for (size_t c = 0; c < C; c++)
  {
    result[c] = lhs[c] - rhs[c];
  }
  return result;
}

/**
 * Computes the product of the given two matrices.
 *
 * @tparam T the element type
 * @tparam R1 the number of rows of the first matrix
 * @tparam C1R2 the number of columns of the first matrix and the number of rows of the
 * second matrix
 * @tparam C2 the number of columns of the second matrix
 * @param lhs the first matrix
 * @param rhs the second matrix
 * @return the product of the given matrices
 */
template <typename T, std::size_t R1, std::size_t C1R2, std::size_t C2>
constexpr mat<T, R1, C2> operator*(
  const mat<T, R1, C1R2>& lhs, const mat<T, C1R2, C2>& rhs)
{
  auto result = mat<T, R1, C2>::zero();
  for (size_t c = 0; c < C2; c++)
  {
    for (size_t r = 0; r < R1; r++)
    {
      for (size_t i = 0; i < C1R2; ++i)
      {
        result[c][r] += lhs[i][r] * rhs[c][i];
      }
    }
  }
  return result;
}

/**
 * Computes the scalar product of the given matrix and the given value.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the scalar
 * @return the product of the given matrix and the given value
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> operator*(const mat<T, R, C>& lhs, const T rhs)
{
  mat<T, R, C> result;
  for (size_t c = 0; c < C; ++c)
  {
    result[c] = lhs[c] * rhs;
  }
  return result;
}

/**
 * Computes the scalar product of the given matrix and the given value.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the scalar
 * @param rhs the matrix
 * @return the product of the given matrix and the given value
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> operator*(const T lhs, const mat<T, R, C>& rhs)
{
  return rhs * lhs;
}

/**
 * Computes the scalar division of the given matrix and the given value.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the scalar
 * @return the division of the given matrix and the given value
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> operator/(const mat<T, R, C>& lhs, const T rhs)
{
  mat<T, R, C> result;
  for (size_t c = 0; c < C; ++c)
  {
    result[c] = lhs[c] / rhs;
  }
  return result;
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the vector
 * @param rhs the matrix
 * @return the product of the given vector and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr vec<T, R> operator*(const vec<T, R>& lhs, const mat<T, R, C>& rhs)
{
  vec<T, R> result;
  for (size_t c = 0; c < C; c++)
  {
    result[c] = dot(lhs, rhs[c]);
  }
  return result;
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the vector
 * @return the product of the given vector and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr vec<T, R> operator*(const mat<T, R, C>& lhs, const vec<T, C>& rhs)
{
  vec<T, C> result;
  for (size_t r = 0; r < R; r++)
  {
    for (size_t c = 0; c < C; ++c)
    {
      result[r] += lhs[c][r] * rhs[c];
    }
  }
  return result;
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the matrix
 * @param rhs the vector
 * @return the product of the given vector and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr vec<T, C - 1> operator*(const mat<T, R, C>& lhs, const vec<T, C - 1>& rhs)
{
  return to_cartesian_coords(lhs * to_homogeneous_coords(rhs));
}

/**
 * Multiplies the given vector by the given matrix.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param lhs the vector
 * @param rhs the matrix
 * @return the product of the given vector and the given matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr vec<T, R - 1> operator*(const vec<T, R - 1>& lhs, const mat<T, R, C>& rhs)
{
  return to_cartesian_coords(to_homogeneous_coords(lhs) * rhs);
}

/**
 * Returns a matrix with the given element set to the given value.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param m the matrix
 * @param r the row index
 * @param c the column index
 * @param v the value
 *
 * @return m with the element set at the given row and column to the given value
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R, C> set(
  mat<T, R, C> m, const std::size_t r, const std::size_t c, const T v)
{
  assert(r < R && c < C);
  m[c][r] = v;
  return m;
}

/**
 * Transposes the given matrix.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param m the matrix to transpose
 * @return the transposed matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, C, R> transpose(const mat<T, R, C>& m)
{
  mat<T, C, R> result(m);
  for (std::size_t c = 0u; c < C; ++c)
  {
    for (std::size_t r = c + 1u; r < R; ++r)
    {
      detail::swap(result[c][r], result[r][c]);
    }
  }
  return result;
}

/**
 * Extracts a slice, or sub matrix, of the given matrix.
 *
 * @tparam RR the number of rows of the slice
 * @tparam RC the number of columns of the slice
 * @tparam T the element type
 * @tparam R the number of rows of the given matrix
 * @tparam C the number of columns of the given matrix
 * @param m the matrix to slice
 * @param r the first row of the slice
 * @param c the first column of the slice
 * @return the slice
 */
template <std::size_t RR, std::size_t RC, typename T, std::size_t R, std::size_t C>
constexpr mat<T, RR, RC> slice(const mat<T, R, C>& m, const size_t r, const size_t c)
{
  static_assert(RR <= R && RC <= R, "slice must not exceed matrix");
  assert(r <= R - RR && c <= C - RC);
  mat<T, RR, RC> result;
  for (std::size_t row = 0u; row < RR; ++row)
  {
    for (std::size_t col = 0u; col < RC; ++col)
    {
      result[col][row] = m[col + c][row + r];
    }
  }
  return result;
}

/**
 * Computes a minor of the given matrix. The minor of a matrix is obtained by erasing one
 * column and one row from that matrix. Thus, any minor matrix of an n*m matrix is a
 * (n-1)*(m-1) matrix.
 *
 * @tparam T the element type
 * @tparam R the number of rows
 * @tparam C the number of columns
 * @param m the matrix to compute a minor of
 * @param row the row to strike
 * @param col the column to strike
 * @return the minor matrix
 */
template <typename T, std::size_t R, std::size_t C>
constexpr mat<T, R - 1, C - 1> extract_minor(
  const mat<T, R, C>& m, const std::size_t row, const std::size_t col)
{
  assert(row < R && col < C);

  mat<T, R - 1, C - 1> min;
  std::size_t minC = 0u;
  for (std::size_t c = 0u; c < C; ++c)
  {
    if (c != col)
    {
      std::size_t minR = 0u;
      for (std::size_t r = 0u; r < R; ++r)
      {
        if (r != row)
        {
          min[minC][minR++] = m[c][r];
        }
      }
      ++minC;
    }
  }
  return min;
}

namespace detail
{
/**
 * Helper struct to compute a matrix determinant. This struct implements a method that
 * works for all S, but there are partial specializations of this template for specific
 * values of S for which faster algorithms exist.
 *
 * @tparam T the component type
 * @tparam S the number of components
 */
template <typename T, std::size_t S>
struct matrix_determinant
{
  constexpr T operator()(const mat<T, S, S>& m) const
  {
    // Laplace after first col
    matrix_determinant<T, S - 1> determinant{};

    auto result = static_cast<T>(0.0);
    for (std::size_t r = 0; r < S; r++)
    {
      const auto f = static_cast<T>(r % 2 == 0 ? 1.0 : -1.0);
      result += f * m[0][r] * determinant(extract_minor(m, r, 0));
    }
    return result;
  }
};

// TODO: implement faster block-matrix based method for NxN matrices where N = 2^n
/**
 * Partial specialization to optimize for the case of a 3x3 matrix.
 *
 * @tparam T the component type
 */
template <typename T>
struct matrix_determinant<T, 3>
{
  constexpr T operator()(const mat<T, 3, 3>& m) const
  {
    return (
      m[0][0] * m[1][1] * m[2][2] + m[1][0] * m[2][1] * m[0][2]
      + m[2][0] * m[0][1] * m[1][2] - m[2][0] * m[1][1] * m[0][2]
      - m[1][0] * m[0][1] * m[2][2] - m[0][0] * m[2][1] * m[1][2]);
  }
};

/**
 * Partial specialization to optimize for the case of a 2x2 matrix.
 *
 * @tparam T the component type
 */
template <typename T>
struct matrix_determinant<T, 2>
{
  constexpr T operator()(const mat<T, 2, 2>& m) const
  {
    return (m[0][0] * m[1][1] - m[1][0] * m[0][1]);
  }
};

/**
 * Partial specialization to optimize for the case of a 1x1 matrix.
 *
 * @tparam T the component type
 */
template <typename T>
struct matrix_determinant<T, 1>
{
  constexpr T operator()(const mat<T, 1, 1>& m) const { return m[0][0]; }
};
} // namespace detail

/**
 * Computes the determinant of the given square matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the matrix to compute the determinant of
 * @return the determinant of the given matrix
 */
template <typename T, std::size_t S>
constexpr T compute_determinant(const mat<T, S, S>& m)
{
  return detail::matrix_determinant<T, S>()(m);
}

/**
 * Computes the adjugate of the given square matrix.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the matrix to compute the adjugate of
 * @return the adjugate of the given matrix
 */
template <typename T, std::size_t S>
constexpr mat<T, S, S> compute_adjugate(const mat<T, S, S>& m)
{
  mat<T, S, S> result;
  for (size_t c = 0; c < S; c++)
  {
    for (size_t r = 0; r < S; r++)
    {
      const auto f = static_cast<T>((c + r) % 2 == 0 ? 1.0 : -1.0);
      result[r][c] =
        f
        * compute_determinant(extract_minor(m, r, c)); // transpose the matrix on the fly
    }
  }
  return result;
}

namespace detail
{
/**
 * Finds an LUP decomposition of matrix a.
 *
 * Given A, finds P,L,U satisfying PA=LU where P is a permutation matrix,
 * where L is lower-triangular with the diagonal elements set to 1,
 * U is upper-triangular.
 *
 * The permutation matrix is returned in a compressed form where each element of the
 * vector represents a row of the permutation matrix, and a value of `i` means the `i`th
 * column of that row is set to 1.
 *
 * From "LUP-Decomposition", Introduction to Algorithms by Cormen et. al., 2nd. ed. p752.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param a the matrix to decompose
 * @return {true, L and U packed into a single matrix, compressed permutation matrix}
 *         or {false, unspecified, unspecified} if a decomposition doesn't exist.
 */
template <typename T, std::size_t S>
constexpr std::tuple<bool, mat<T, S, S>, vec<size_t, S>> lup_find_decomposition(
  mat<T, S, S> a)
{
  vec<size_t, S> pi;
  for (std::size_t i = 0; i < S; ++i)
  {
    pi[i] = i;
  }
  for (std::size_t k = 0; k < S; ++k)
  {
    T p(0);
    std::size_t kPrime = 0;
    for (std::size_t i = k; i < S; ++i)
    {
      if (vm::abs(a[k][i]) > p)
      {
        p = vm::abs(a[k][i]);
        kPrime = i;
      }
    }
    if (p < 1.0e-15)
    {
      return {false, mat<T, S, S>(), vec<size_t, S>()};
    }
    swap(pi[k], pi[kPrime]);
    for (std::size_t i = 0; i < S; ++i)
    {
      swap(a[i][k], a[i][kPrime]);
    }
    for (std::size_t i = k + 1; i < S; ++i)
    {
      a[k][i] = a[k][i] / a[k][k];
      for (std::size_t j = k + 1; j < S; ++j)
      {
        a[j][i] = a[j][i] - a[k][i] * a[j][k];
      }
    }
  }
  return {true, a, pi};
}

/**
 * Solves a system of equations given an LUP factorization.
 *
 * From "LUP-Solve", Introduction to Algorithms by Cormen et. al., 2nd. ed. p745.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lu the LU factorization packed into a single matrix; see lupDecomposition()
 * @param pi the permutation matrix packed into a vector; see lupDecomposition()
 * @param b the target value in the system of equations a*x=b
 * @return the solution value x in the system of equations a*x=b
 */
template <typename T, std::size_t S>
constexpr vec<T, S> lup_solve_internal(
  const mat<T, S, S>& lu, const vec<size_t, S>& pi, const vec<T, S>& b)
{
  vec<T, S> x;
  vec<T, S> y;
  for (std::size_t i = 0; i < S; ++i)
  {
    T sum = T(0);
    for (std::size_t j = 0; j + 1 <= i; ++j)
    {
      sum += lu[j][i] * y[j];
    }
    y[i] = b[pi[i]] - sum;
  }
  for (std::size_t i = S - 1; i < S; --i)
  {
    T sum = T(0);
    for (std::size_t j = i + 1; j < S; ++j)
    {
      sum += lu[j][i] * x[j];
    }
    x[i] = (y[i] - sum) / lu[i][i];
  }
  return x;
}
} // namespace detail

/**
 * Solves a system of equations expressed as a*x=b, using LU factorization with pivoting.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param a square matrix
 * @param b column vector
 * @return either {true, x such that a*x=b} or {false, unspecified} if no solution could
 * be found
 */
template <typename T, std::size_t S>
constexpr std::tuple<bool, vec<T, S>> lup_solve(const mat<T, S, S>& a, const vec<T, S>& b)
{
  const auto decomp = detail::lup_find_decomposition(a);
  const bool success = std::get<0>(decomp);
  if (!success)
  {
    return std::make_tuple(false, vec<T, S>());
  }

  const auto lu = std::get<1>(decomp);
  const auto pi = std::get<2>(decomp);
  return std::make_tuple(true, detail::lup_solve_internal(lu, pi, b));
}

/**
 * Inverts the given square matrix if possible.
 *
 * Uses the technique from "Computing a matrix inverse from an LUP decomposition"
 * Introduction to Algorithms by Cormen et. al., 2nd. ed. p755.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param m the matrix to invert
 * @return a pair of a boolean and a matrix such that the boolean indicates whether the
 * matrix is invertible, and if so, the matrix is the inverted given matrix
 */
template <typename T, std::size_t S>
constexpr std::tuple<bool, mat<T, S, S>> invert(const mat<T, S, S>& m)
{
  const auto decomp = detail::lup_find_decomposition(m);
  const bool success = std::get<0>(decomp);
  if (!success)
  {
    return std::make_tuple(false, mat<T, S, S>::identity());
  }

  const auto lu = std::get<1>(decomp);
  const auto pi = std::get<2>(decomp);

  mat<T, S, S> result;
  for (std::size_t i = 0; i < S; ++i)
  {
    vec<T, S> targetColumn; // ith column of the S by S identity matrix
    targetColumn[i] = static_cast<T>(1);

    result[i] = detail::lup_solve_internal(lu, pi, targetColumn);
  }
  return std::make_tuple(true, result);
}
} // namespace vm
