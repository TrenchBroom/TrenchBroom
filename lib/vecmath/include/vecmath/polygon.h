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

#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/vec.h>
#include <vecmath/vec_ext.h>

#include <algorithm>
#include <cstddef>
#include <vector>

namespace vm
{
template <typename T, size_t S>
class polygon
{
public:
  using component_type = T;
  static const size_t size = S;
  using float_type = polygon<float, S>;

private:
  std::vector<vec<T, S>> m_vertices;

public:
  /**
   * Creates a new empty polygon.
   */
  polygon() = default;

  /**
   * Creates a new polygon with the given vertices. The given points are assumed to form a
   * convex polygon.
   *
   *  @param i_vertices the vertices
   */
  polygon(std::initializer_list<vec<T, S>> i_vertices)
    : m_vertices(i_vertices)
  {
    rotate_min_to_front();
  }

  /**
   * Creates a new polygon with the given vertices. The given points are assumed to form a
   * convex polygon.
   *
   * @param i_vertices the vertices
   */
  explicit polygon(const std::vector<vec<T, S>>& i_vertices)
    : m_vertices(i_vertices)
  {
    rotate_min_to_front();
  }

  /**
   * Creates a new polygon with the given vertices. The given points are assumed to form a
   * convex polygon.
   *
   * @param i_vertices the vertices
   */
  explicit polygon(std::vector<vec<T, S>>&& i_vertices)
    : m_vertices(std::move(i_vertices))
  {
    rotate_min_to_front();
  }

private:
  void rotate_min_to_front()
  {
    if (!m_vertices.empty())
    {
      const auto begin = std::begin(m_vertices);
      const auto end = std::end(m_vertices);
      const auto min = std::min_element(begin, end);
      // cppcheck-suppress ignoredReturnValue
      std::rotate(begin, min, end);
    }
  }

public:
  // Copy and move constructors
  polygon(const polygon<T, S>& other) = default;
  polygon(polygon<T, S>&& other) noexcept = default;

  // Assignment operators
  polygon<T, S>& operator=(const polygon<T, S>& other) = default;
  polygon<T, S>& operator=(polygon<T, S>&& other) noexcept = default;

  /**
   * Creates a new polygon by copying the values from the given polygon. If the given
   * polygon has a different component type, the values are converted using static_cast.
   *
   * @tparam U the component type of the given polygon
   * @param other the polygon to copy the values from
   */
  template <typename U>
  explicit polygon(const polygon<U, S>& other)
  {
    m_vertices.reserve(other.vertexCount());
    for (const auto& vertex : other.vertices())
    {
      m_vertices.push_back(vec<T, S>(vertex));
    }
  }

  /**
   * Checks whether this polygon has a vertex with the given coordinates.
   *
   * @param vertex the position to check
   * @return bool if this polygon has a vertex at the given position and false otherwise
   */
  bool hasVertex(const vec<T, S>& vertex) const
  {
    for (const auto& v : m_vertices)
    {
      if (v == vertex)
      {
        return true;
      }
    }
    return false;
  }

  /**
   * Returns the number of vertices of this polygon.
   *
   * @return the number of vertices
   */
  size_t vertexCount() const { return m_vertices.size(); }

  /**
   * Returns an iterator to the beginning of the vertices.
   *
   * @return an iterator to the beginning of the vertices
   */
  auto begin() const { return std::begin(m_vertices); }

  /**
   * Returns an iterator to the end of the vertices.
   *
   * @return an iterator to the end of the vertices
   */
  auto end() const { return std::end(m_vertices); }

  /**
   * Returns an iterator to the beginning of the vertices.
   *
   * @return an iterator to the beginning of the vertices
   */
  auto rbegin() const { return std::begin(m_vertices); }

  /**
   * Returns an iterator to the end of the vertices.
   *
   * @return an iterator to the end of the vertices
   */
  auto rend() const { return std::end(m_vertices); }

  /**
   * Returns the vertices of this polygon.
   *
   * @return the vertices
   */
  const std::vector<vec<T, S>>& vertices() const { return m_vertices; }

  /**
   * Computes the center of this polygon.
   *
   * @return the center of this polygon
   */
  vec<T, S> center() const
  {
    auto result = vec<T, S>::zero();
    for (const auto& v : m_vertices)
    {
      result = result + v;
    }
    return result / static_cast<T>(vertexCount());
  }

  /**
   * Inverts this polygon by reversing its vertices.
   *
   * @return the inverted polygon
   */
  polygon<T, S> invert() const
  {
    auto vertices = m_vertices;
    if (vertices.size() > 1)
    {
      std::reverse(std::next(std::begin(vertices)), std::end(vertices));
    }
    return polygon<T, S>(vertices);
  }

  /**
   * Translates this polygon by the given offset.
   *
   * @param offset the offset by which to translate
   * @return the translated polygon
   */
  polygon<T, S> translate(const vec<T, S>& offset) const
  {
    return polygon<T, S>(m_vertices + offset);
  }

  /**
   * Transforms this polygon using the given transformation matrix.
   *
   * @param mat the transformation to apply
   * @return the transformed polygon
   */
  polygon<T, S> transform(const mat<T, S + 1, S + 1>& mat) const
  {
    return polygon<T, S>(mat * vertices());
  }

  // FIXME: this is only here because TB's VertexToolBase needs it, it should be moved
  // elsewhere
  /**
   * Adds the vertices of the given range of polygons to the given output iterator.
   *
   * @tparam I the range iterator type
   * @tparam O the output iterator type
   * @param cur the range start
   * @param end the range end
   * @param out the output iterator
   */
  template <typename I, typename O>
  static void get_vertices(I cur, I end, O out)
  {
    while (cur != end)
    {
      const auto& polygon = *cur;
      for (const auto& vertex : polygon)
      {
        out++ = vertex;
      }
      ++cur;
    }
  }
};

/**
 * Compares the given polygons under the assumption that the first vertex of each polygon
 * is the smallest of all vertices of that polygon. A polygon is considered less than
 * another polygon if is has fewer vertices, and vice versa. If both polygons have the
 * same number of vertices, the vertices are compared lexicographically until a pair of
 * vertices is found which are not identical. The result of comparing these vertices is
 * returned. If no such pair exists, 0 is returned.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @param epsilon an epsilon value
 * @return -1 if the first polygon is less than the second polygon, +1 in the opposite
 * case, and 0 otherwise
 */
template <typename T, size_t S>
int compare(
  const polygon<T, S>& lhs,
  const polygon<T, S>& rhs,
  const T epsilon = static_cast<T>(0.0))
{
  const auto& lhsVerts = lhs.vertices();
  const auto& rhsVerts = rhs.vertices();

  return compare(
    std::begin(lhsVerts),
    std::end(lhsVerts),
    std::begin(rhsVerts),
    std::end(rhsVerts),
    epsilon);
}

/**
 * Checks whether the first given polygon is equal to the second polygon using the given
 * epsilon value.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @param epsilon an epsilon value
 * @return true if the polygons are equal and false otherwise
 */
template <typename T, size_t S>
bool isEqual(const polygon<T, S>& lhs, const polygon<T, S>& rhs, const T epsilon)
{
  return compare(lhs, rhs, epsilon) == 0;
}

/**
 * Checks whether the first given polygon is identical to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the polygons are identical and false otherwise
 */
template <typename T, size_t S>
bool operator==(const polygon<T, S>& lhs, const polygon<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) == 0;
}

/**
 * Checks whether the first given polygon is identical to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return false if the polygons are identical and true otherwise
 */
template <typename T, size_t S>
bool operator!=(const polygon<T, S>& lhs, const polygon<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) != 0;
}

/**
 * Checks whether the first given polygon is less than the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is less than the second polygon and false otherwise
 */
template <typename T, size_t S>
bool operator<(const polygon<T, S>& lhs, const polygon<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) < 0;
}

/**
 * Checks whether the first given polygon is less than or equal to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is less than or equal to the second polygon and false
 * otherwise
 */
template <typename T, size_t S>
bool operator<=(const polygon<T, S>& lhs, const polygon<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) <= 0;
}

/**
 * Checks whether the first given polygon is greater than the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is greater than the second polygon and false
 * otherwise
 */
template <typename T, size_t S>
bool operator>(const polygon<T, S>& lhs, const polygon<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) > 0;
}

/**
 * Checks whether the first given polygon is greater than or equal to the second polygon.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @return true if the first polygon is greater than or equal to the second polygon and
 * false otherwise
 */
template <typename T, size_t S>
bool operator>=(const polygon<T, S>& lhs, const polygon<T, S>& rhs)
{
  return compare(lhs, rhs, T(0.0)) >= 0;
}

/**
 * Compares the given polygons under the assumption that the first vertex of each polygon
 * is the smallest of all vertices of that polygon. A polygon is considered less than
 * another polygon if is has fewer vertices, and vice versa. If both polygons have the
 * same number of vertices, the vertices are compared lexicographically until a pair of
 * vertices is found which are not identical. The result of comparing these vertices is
 * returned. If no such pair exists, 0 is returned.
 *
 * In this comparison, the order of the vertices is relaxed. Two polygons can be identical
 * even if the order of the vertices is reversed.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first polygon
 * @param rhs the second polygon
 * @param epsilon an epsilon value
 * @return -1 if the first polygon is less than the second polygon, +1 in the opposite
 * case, and 0 otherwise
 */
template <typename T, size_t S>
int compareUnoriented(
  const polygon<T, S>& lhs,
  const polygon<T, S>& rhs,
  const T epsilon = static_cast<T>(0.0))
{
  const auto& lhsVerts = lhs.vertices();
  const auto& rhsVerts = rhs.vertices();

  if (lhsVerts.size() < rhsVerts.size())
  {
    return -1;
  }
  else if (lhsVerts.size() > rhsVerts.size())
  {
    return 1;
  }
  else
  {
    const auto count = lhsVerts.size();
    if (count == 0)
    {
      return 0;
    }

    // Compare first:
    const auto cmp0 = compare(lhsVerts[0], rhsVerts[0], epsilon);
    if (cmp0 < 0)
    {
      return -1;
    }
    else if (cmp0 > 0)
    {
      return +1;
    }

    if (count == 1)
    {
      return 0;
    }

    // First vertices are identical. Now compare my second with other's second.
    auto cmp1 = compare(lhsVerts[1], rhsVerts[1], epsilon);
    if (cmp1 == 0)
    {
      // The second vertices are also identical, so we just do a forward compare.
      return compare(
        std::next(std::begin(lhsVerts), 2),
        std::end(lhsVerts),
        std::next(std::begin(rhsVerts), 2),
        std::end(rhsVerts),
        epsilon);
    }
    else
    {
      // The second vertices are not identical, so we attempt a backward compare.
      size_t i = 1;
      while (i < count)
      {
        const auto j = count - i;
        const auto cmp = compare(lhsVerts[i], rhsVerts[j], epsilon);
        if (cmp != 0)
        {
          // Backward compare failed, so make a forward compare
          return compare(
            std::next(std::begin(lhsVerts), 2),
            std::end(lhsVerts),
            std::next(std::begin(rhsVerts), 2),
            std::end(rhsVerts),
            epsilon);
        }
        ++i;
      }
      return 0;
    }
  }
}
} // namespace vm
