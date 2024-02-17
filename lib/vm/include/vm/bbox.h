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

#include "vm/mat.h"
#include "vm/quat.h"
#include "vm/scalar.h"
#include "vm/vec.h"

#include <array>

namespace vm
{
/**
 * An axis aligned bounding box that is represented by a min point and a max point. The
 * min and max point are constrained by the following invariant:
 *
 * For each component i < S, it holds that min[i] <= max[i].
 *
 * @tparam T the component type
 * @tparam S the number of components of the min and max points
 */
template <typename T, std::size_t S>
class bbox
{
public:
  /**
   * Helper to build a bounding box from points or other bounding boxes.
   */
  class builder
  {
  private:
    bbox m_bounds;

  public:
    /**
     * Creates a new unitialized instance.
     */
    constexpr builder()
      : m_bounds(false)
    {
      // put the bounds to an invalid state to signal that its unitialized
    }

    /**
     * Returns the bounds. If the no point has been added, an empty bbox at the origin is
     * returned.
     */
    constexpr const bbox& bounds() const { return m_bounds; }

    /**
     * Returns whether anything has been added to this builder.
     */
    constexpr bool initialized() const { return m_bounds.is_valid(); }

    /**
     * Adds the given range of points.
     *
     * @tparam I the iterator type
     * @tparam G the type of the function that transforms the iterated type to a point
     * @param cur the start of the range
     * @param end the end of the range
     * @param get the function that transforms the iterated type to a point
     */
    template <typename I, typename G = vm::identity>
    constexpr void add(I cur, I end, G get = G())
    {
      while (cur != end)
      {
        add(get(*cur));
        ++cur;
      }
    }

    /**
     * Adds the given point.
     */
    constexpr void add(const vec<T, S>& point)
    {
      if (!initialized())
      {
        m_bounds.min = m_bounds.max = point;
      }
      else
      {
        m_bounds = merge(m_bounds, point);
      }
    }

    /**
     * Adds the given box.
     */
    constexpr void add(const bbox& box)
    {
      if (!initialized())
      {
        m_bounds = box;
      }
      else
      {
        m_bounds = merge(m_bounds, box);
      }
    }
  };

public:
  vec<T, S> min;
  vec<T, S> max;

public:
  /**
   * Creates a new bounding box at the origin with size 0.
   */
  constexpr bbox()
    : min(vec<T, S>::zero())
    , max(vec<T, S>::zero())
  {
  }

  // Copy and move constructors
  bbox(const bbox<T, S>& other) = default;
  bbox(bbox<T, S>&& other) noexcept = default;

  // Assignment operators
  bbox<T, S>& operator=(const bbox<T, S>& other) = default;
  bbox<T, S>& operator=(bbox<T, S>&& other) noexcept = default;

  /**
   * Creates a new bounding box by copying the values from the given bounding box. If the
   * given box has a different component type, the values are converted by calling the
   * appropriate conversion constructor of the two vectors.
   *
   * @tparam U the component type of the given bounding box
   * @param other the bounding box to convert
   */
  template <typename U>
  explicit constexpr bbox(const bbox<U, S>& other)
    : min(other.min)
    , max(other.max)
  {
    assert(is_valid());
  }

  /**
   * Creates a new bounding box with the given min and max values. The values are assumed
   * to be correct, that is, for each component, the corresponding value of the min point
   * is smaller than or equal to the corresponding value of the max point.
   *
   * @param i_min the min point of the bounding box
   * @param i_max the max point of the bounding box
   */
  constexpr bbox(const vec<T, S>& i_min, const vec<T, S>& i_max)
    : min(i_min)
    , max(i_max)
  {
    assert(is_valid());
  }

  /**
   * Creates a new bounding box by setting each component of the min point to the given
   * min value, and each component of the max point to the given max value. This
   * constructor assumes that the given min value does not exceed the given max value.
   *
   * @param i_min the min point of the bounding box
   * @param i_max the max point of the bounding box
   */
  constexpr bbox(const T i_min, const T i_max)
    : min(vec<T, S>::fill(i_min))
    , max(vec<T, S>::fill(i_max))
  {
    assert(is_valid());
  }

  /**
   * Creates a new bounding box with the coordinate system origin at its center by setting
   * the min point to the negated given value, and the max point to the given value.
   *
   * The value is assumed to be correct, that is, none of its components must have a
   * negative value.
   *
   * @param i_minMax the min and max point
   */
  explicit constexpr bbox(const T i_minMax)
    : min(vec<T, S>::fill(-i_minMax))
    , max(vec<T, S>::fill(+i_minMax))
  {
    assert(is_valid());
  }

private:
  /**
   * This constructor is used by the builder to create an invalid bbox.
   */
  constexpr bbox(const bool)
    : min(vec<T, S>::fill(T(1)))
    , max(vec<T, S>::fill(T(0)))
  {
    assert(!is_valid());
  }

public:
  /**
   * Creates the smallest bounding box that contains all points in the given range.
   * Optionally accepts a transformation that is applied to each element of the range. The
   * given range must not be empty.
   *
   * @tparam I the range iterator type
   * @tparam G type of the transformation
   * @param cur the start of the range
   * @param end the end of the range
   * @param get the transformation
   * @return the bounding box
   */
  template <typename I, typename G = identity>
  constexpr static bbox<T, S> merge_all(I cur, I end, const G& get = G())
  {
    assert(cur != end);
    const auto first = get(*cur++);
    bbox<T, S> result(first, first);
    while (cur != end)
    {
      result = merge(result, get(*cur++));
    }
    return result;
  }

public:
  /**
   * Checks whether a bounding box with the given min and max points satisfies its
   * invariant. The invariant states that for each component, the corresponding value of
   * the min point must not exceed the corresponding value of the max point.
   *
   * @return true if the bounding box with the given points is valid and false otherwise
   */
  constexpr static bool is_valid(const vec<T, S>& min, const vec<T, S>& max)
  {
    for (size_t i = 0; i < S; ++i)
    {
      if (min[i] > max[i])
      {
        return false;
      }
    }
    return true;
  }

  /**
   * Checks whether this bounding box satisfies its invariant. The invariant states that
   * for each component, the corresponding value of the min point must not exceed the
   * corresponding value of the max point.
   *
   * @return true if this bounding box is valid and false otherwise
   */
  constexpr bool is_valid() const { return is_valid(min, max); }

  /**
   * Checks whether this bounding box has an empty volume.
   *
   * @return true if this bounding box has an empty volume and false otherwise
   */
  constexpr bool is_empty() const
  {
    assert(is_valid());
    for (size_t i = 0; i < S; ++i)
    {
      if (min[i] >= max[i])
      {
        return true;
      }
    }
    return false;
  }

  /**
   * Computes the center of this bounding box.
   *
   * @return the center of this bounding box
   */
  constexpr vec<T, S> center() const
  {
    assert(is_valid());
    return (min + max) / static_cast<T>(2.0);
  }

  /**
   * Computes the size of this bounding box
   *
   * @return the size of this bounding box
   */
  constexpr vec<T, S> size() const
  {
    assert(is_valid());
    return max - min;
  }

  /**
   * Computes the volume of this bounding box.
   *
   * @return the volumen of this bounding box
   */
  constexpr T volume() const
  {
    assert(is_valid());
    const auto boxSize = size();
    T result = boxSize[0];
    for (size_t i = 1; i < S; ++i)
    {
      result *= boxSize[i];
    }
    return result;
  }

  /**
   * Checks whether the given point is cointained in this bounding box.
   *
   * @param point the point
   * @return true if the given point is contained in the given bounding box and false
   * otherwise
   */
  constexpr bool contains(const vec<T, S>& point) const
  {
    assert(is_valid());
    for (size_t i = 0; i < S; ++i)
    {
      if (point[i] < min[i] || point[i] > max[i])
      {
        return false;
      }
    }
    return true;
  }

  /**
   * Checks whether the given bounding box is contained in this bounding box.
   *
   * @param b the possibly contained bounding box
   * @return true if the given bounding box is contained in this bounding box
   */
  constexpr bool contains(const bbox<T, S>& b) const
  {
    assert(is_valid());
    for (size_t i = 0; i < S; ++i)
    {
      if (b.min[i] < min[i] || b.max[i] > max[i])
      {
        return false;
      }
    }
    return true;
  }

  /**
   * Checks whether the given bounding box is enclosed in this bounding box. This is
   * equivalent to checking whether given box is contained within this box such that the
   * boxes don't touch at all.
   *
   * @param b the possibly enclosed bounding box
   * @return true if the given bounding box is enclosed in this bounding box
   */
  constexpr bool encloses(const bbox<T, S>& b) const
  {
    assert(is_valid());
    for (size_t i = 0; i < S; ++i)
    {
      if (b.min[i] <= min[i] || b.max[i] >= max[i])
      {
        return false;
      }
    }
    return true;
  }

  /**
   * Checks whether the given bounding box intersects with this bounding box.

   * @param b the second bounding box
   * @return true if the given bounding box intersects with this bounding box and false
   otherwise
   */
  constexpr bool intersects(const bbox<T, S>& b) const
  {
    for (size_t i = 0; i < S; ++i)
    {
      if (b.max[i] < min[i] || b.min[i] > max[i])
      {
        return false;
      }
    }
    return true;
  }

  /**
   * Constrains the given point to the volume covered by this bounding box.
   *
   * @param point the point to constrain
   * @return the constrained point
   */
  constexpr vec<T, S> constrain(const vec<T, S>& point) const
  {
    assert(is_valid());
    return vm::max(min, vm::min(max, point));
  }

  enum class Corner
  {
    min,
    max
  };

  /**
   * Returns the position of a corner of this bounding box according to the given spec.
   *
   * @param c the corner to return
   * @return the position of the given corner
   */
  constexpr vec<T, S> corner(const Corner c[S]) const
  {
    assert(is_valid());
    vec<T, S> result;
    for (size_t i = 0; i < S; ++i)
    {
      result[i] = c[i] == Corner::min ? min[i] : max[i];
    }
    return result;
  }

  /**
   * Returns the position of a corner of this bounding box according to the given spec.
   *
   * @param x the X position of the corner
   * @param y the Y position of the corner
   * @param z the Z position of the corner
   * @return the position of the given corner
   */
  constexpr vec<T, 3> corner(Corner x, Corner y, Corner z) const
  {
    Corner c[] = {x, y, z};
    return corner(c);
  }

  enum class Range
  {
    less,
    within,
    greater
  };

  /**
   * Returns the relative position of the given point. For each component, the returned
   * array contains a value of the Range enum which indicates one of the following three
   * cases:
   *
   * - the component of the point is less than the corresponding component of the min
   * point
   * - the component of the point is greater than the corresponding component of the max
   * point
   * - the component of the point is in the range defined by the corresponding components
   * of the min and max point (inclusive)
   *
   * @param point the point to check
   * @return the relative position
   */
  constexpr std::array<Range, S> relative_position(const vec<T, S>& point) const
  {
    assert(is_valid());

    std::array<Range, S> result{};
    for (size_t i = 0; i < S; ++i)
    {
      if (point[i] < min[i])
      {
        result[i] = Range::less;
      }
      else if (point[i] > max[i])
      {
        result[i] = Range::greater;
      }
      else
      {
        result[i] = Range::within;
      }
    }

    return result;
  }

  /**
   * Expands this bounding box by the given delta.
   *
   * @param f the value by which to expand this bounding box
   * @return the expanded bounding box
   */
  constexpr bbox<T, S> expand(const T f) const
  {
    assert(is_valid());
    return bbox<T, S>(min - vec<T, S>::fill(f), max + vec<T, S>::fill(f));
  }

  /**
   * Translates this bounding box by the given offset.
   *
   * @param delta the offset by which to translate
   * @return the translated bounding box
   */
  constexpr bbox<T, S> translate(const vec<T, S>& delta) const
  {
    assert(is_valid());
    return bbox<T, S>(min + delta, max + delta);
  }

  /**
   * Transforms this bounding box by applying the given transformation to each corner
   * vertex. The result is the smallest bounding box that contains the transformed
   * vertices.
   *
   * @param transform the transformation
   * @return the transformed bounding box
   */
  constexpr bbox<T, S> transform(const mat<T, S + 1, S + 1>& transform) const
  {
    builder builder;
    const auto vertices = this->vertices();
    for (const auto& vertex : vertices)
    {
      builder.add(transform * vertex);
    }
    return builder.bounds();
  }

  /**
   * Executes the given operation on every face of this bounding box. For each face, its
   * four vertices are passed to the given operation in a clock wise manner.
   *
   * @tparam Op the type of the operation
   * @param op the operation
   */
  template <typename Op>
  constexpr void for_each_face(Op&& op) const
  {
    const vec<T, 3> boxSize = size();
    const vec<T, 3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
    const vec<T, 3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
    const vec<T, 3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());

    op(max, max - y, max - y - x, max - x, vec<T, 3>(0.0, 0.0, +1.0)); // top
    op(min, min + x, min + x + y, min + y, vec<T, 3>(0.0, 0.0, -1.0)); // bottom
    op(min, min + z, min + z + x, min + x, vec<T, 3>(0.0, -1.0, 0.0)); // front
    op(max, max - x, max - x - z, max - z, vec<T, 3>(0.0, +1.0, 0.0)); // back
    op(min, min + y, min + y + z, min + z, vec<T, 3>(-1.0, 0.0, 0.0)); // left
    op(max, max - z, max - z - y, max - y, vec<T, 3>(+1.0, 0.0, 0.0)); // right
  }

  /**
   * Executes the given operation for each edge of this bounding box. For each edge, the
   * two vertices which are connected by that edge are passed to the operation.
   *
   * @tparam Op the type of the operation
   * @param op the operation
   */
  template <typename Op>
  constexpr void for_each_edge(Op&& op) const
  {
    const vec<T, 3> boxSize = size();
    const vec<T, 3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
    const vec<T, 3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
    const vec<T, 3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());

    // top edges clockwise (viewed from above)
    op(max, max - y);
    op(max - y, max - y - x);
    op(max - y - x, max - x);
    op(max - x, max);

    // bottom edges clockwise (viewed from below)
    op(min, min + x);
    op(min + x, min + x + y);
    op(min + x + y, min + y);
    op(min + y, min);

    // side edges clockwise (viewed from above)
    op(min, min + z);
    op(min + y, min + y + z);
    op(min + x + y, min + x + y + z);
    op(min + x, min + x + z);
  }

  /**
   * Executes the given operation for each vertex of this bounding box.
   *
   * @tparam Op the type of the operation
   * @param op the operation
   */
  template <class Op>
  constexpr void for_each_vertex(Op&& op) const
  {
    const vec<T, 3> boxSize = size();
    const vec<T, 3> x(boxSize.x(), static_cast<T>(0.0), static_cast<T>(0.0));
    const vec<T, 3> y(static_cast<T>(0.0), boxSize.y(), static_cast<T>(0.0));
    const vec<T, 3> z(static_cast<T>(0.0), static_cast<T>(0.0), boxSize.z());

    // top vertices clockwise (viewed from above)
    op(max);
    op(max - y);
    op(min + z);
    op(max - x);

    // bottom vertices clockwise (viewed from below)
    op(min);
    op(min + x);
    op(max - z);
    op(min + y);
  }

  /**
   * Returns an array containing all 8 corner vertices of this bounding box.
   *
   * @return an array of vertices
   */
  constexpr std::array<vec<T, S>, 8> vertices() const
  {
    std::array<vec<T, S>, 8> result{};
    std::size_t i = 0;
    for_each_vertex([&](const vec<T, S>& v) { result[i++] = v; });
    return result;
  }
};

/**
 * Checks whether the two given bounding boxes are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return true if the two bounding boxes are identical, and false otherwise
 */
template <typename T, std::size_t S>
constexpr bool operator==(const bbox<T, S>& lhs, const bbox<T, S>& rhs)
{
  return lhs.min == rhs.min && lhs.max == rhs.max;
}

/**
 * Checks whether the two given bounding boxes are identical.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return false if the two bounding boxes are identical, and true otherwise
 */
template <typename T, std::size_t S>
constexpr bool operator!=(const bbox<T, S>& lhs, const bbox<T, S>& rhs)
{
  return lhs.min != rhs.min || lhs.max != rhs.max;
}

/**
 * Checks whether the given bounding boxes are component wise equal up to the given
 * epsilon.
 *
 * Unline the equality operator ==, this function takes an epsilon value into account.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @param epsilon the epsilon value
 * @return true if the given boundinb boxes are component wise equal up to the given
 * epsilon value
 */
template <typename T, std::size_t S>
constexpr bool is_equal(const bbox<T, S>& lhs, const bbox<T, S>& rhs, const T epsilon)
{
  return is_equal(lhs.min, rhs.min, epsilon) && is_equal(lhs.max, rhs.max, epsilon);
}

/**
 * Repairs the given bounding box by ensuring that the min corner is the component wise
 * minimum of the min and max corners, and likewise that the max corner is the component
 * wise maximum of the min and max corners.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param box the boundinb box to repair
 * @return the repaired bounding box
 */
template <typename T, std::size_t S>
constexpr bbox<T, S> repair(const bbox<T, S>& box)
{
  return bbox<T, S>(min(box.min, box.max), max(box.min, box.max));
}

/**
 * Returns the smallest bounding box that contains the two given bounding boxes.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return the smallest bounding box that contains the given bounding boxes
 */
template <typename T, std::size_t S>
constexpr bbox<T, S> merge(const bbox<T, S>& lhs, const bbox<T, S>& rhs)
{
  return bbox<T, S>(min(lhs.min, rhs.min), max(lhs.max, rhs.max));
}

/**
 * Returns the smallest bounding box that contains the given bounding box and the given
 * point.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the bounding box
 * @param rhs the point
 * @return the smallest bounding box that contains the given bounding box and point
 */
template <typename T, std::size_t S>
constexpr bbox<T, S> merge(const bbox<T, S>& lhs, const vec<T, S>& rhs)
{
  return bbox<T, S>(min(lhs.min, rhs), max(lhs.max, rhs));
}

/**
 * Returns the smallest bounding box that contains the intersection of the given bounding
 * boxes. If the intersection is empty, then an empty bounding box at the origin is
 * returned.
 *
 * @tparam T the component type
 * @tparam S the number of components
 * @param lhs the first bounding box
 * @param rhs the second bounding box
 * @return the smallest bounding box that contains the intersection of the given bounding
 * boxes
 */
template <typename T, std::size_t S>
constexpr bbox<T, S> intersect(const bbox<T, S>& lhs, const bbox<T, S>& rhs)
{
  const auto min = vm::max(lhs.min, rhs.min);
  const auto max = vm::min(lhs.max, rhs.max);
  if (bbox<T, S>::is_valid(min, max))
  {
    return bbox<T, S>(min, max);
  }
  else
  {
    return bbox<T, S>(vec<T, S>::zero(), vec<T, S>::zero());
  }
}
} // namespace vm
