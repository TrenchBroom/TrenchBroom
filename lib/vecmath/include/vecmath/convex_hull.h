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

#include "util.h"
#include "vec.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace vm {
namespace detail {
/**
 * Helper struct for computing a convex hull.
 *
 * @tparam T the component type
 */
template <typename T> class convex_hull {
private:
  static int is_left(const vec<T, 3>& p1, const vec<T, 3>& p2, const vec<T, 3>& p3) {
    const T result =
      ((p2.x() - p1.x()) * (p3.y() - p1.y()) - (p3.x() - p1.x()) * (p2.y() - p1.y()));
    if (result < 0.0) {
      return -1;
    } else if (result > 0.0) {
      return 1;
    } else {
      return 0;
    }
  }

private:
  class less_than_by_angle {
  private:
    const vec<T, 3>& m_anchor;

  public:
    explicit less_than_by_angle(const vec<T, 3>& anchor)
      : m_anchor(anchor) {}

  public:
    bool operator()(const vec<T, 3>& lhs, const vec<T, 3>& rhs) const {
      const auto side = is_left(m_anchor, lhs, rhs);
      if (side > 0) {
        return true;
      } else if (side < 0) {
        return false;
      } else {
        // the points are colinear, the one that is further from the anchor is considered less
        const auto dxl = abs(lhs.x() - m_anchor.x());
        const auto dxr = abs(rhs.x() - m_anchor.x());
        if (dxl == dxr) {
          const auto dyl = abs(lhs.y() - m_anchor.y());
          const auto dyr = abs(rhs.y() - m_anchor.y());
          return dyl > dyr;
        }
        return dxl > dxr;
      }
    }
  };

  std::vector<vec<T, 3>> m_points;

public:
  explicit convex_hull(const std::vector<vec<T, 3>>& points)
    : m_points(points) {
    if (m_points.size() <= 2u) {
      m_points.clear();
      return;
    }

    const auto thirdPointIndex = find_linearly_independent_point();
    if (thirdPointIndex >= m_points.size()) {
      m_points.clear();
      return;
    }

    const auto axis = compute_axis(thirdPointIndex);
    swizzle(axis);

    find_anchor();
    sort_points();
    if (m_points.size() <= 2u) {
      m_points.clear();
      return;
    }

    build_hull();
    unswizzle(axis);
  }

  const std::vector<vec<T, 3>>& result() const { return m_points; }

private:
  std::size_t find_linearly_independent_point() const {
    std::size_t index = 2;
    while (index < m_points.size() && is_colinear(m_points[0], m_points[1], m_points[index])) {
      ++index;
    }
    return index;
  }

  axis::type compute_axis(const std::size_t thirdPointIndex) const {
    const auto axis = cross(m_points[thirdPointIndex] - m_points[0], m_points[1] - m_points[0]);
    return find_abs_max_component(axis);
  }

  void swizzle(const axis::type axis) {
    for (auto& p : m_points) {
      p = vm::swizzle(p, axis);
    }
  }

  void unswizzle(const axis::type axis) {
    for (auto& p : m_points) {
      p = vm::unswizzle(p, axis);
    }
  }

  void find_anchor() {
    std::size_t anchor = 0;
    for (std::size_t i = 1; i < m_points.size(); ++i) {
      if (
        (m_points[i].y() < m_points[anchor].y()) ||
        (m_points[i].y() == m_points[anchor].y() && m_points[i].x() > m_points[anchor].x())) {
        anchor = i;
      }
    }

    if (anchor > 0) {
      using std::swap;
      swap(m_points[0], m_points[anchor]);
    }
  }

  void sort_points() {
    const auto& anchor = m_points[0];
    std::sort(std::next(std::begin(m_points)), std::end(m_points), less_than_by_angle(anchor));

    // now remove the duplicates
    auto i = std::begin(m_points) + 1;
    while (i != std::end(m_points)) {
      const auto& p1 = *(i++);
      while (i != std::end(m_points)) {
        const auto& p2 = *i;
        if (is_left(anchor, p1, p2) == 0) {
          i = m_points.erase(i);
        } else {
          break;
        }
      }
    }
  }

  void build_hull() {
    auto stack = std::vector<vec<T, 3>>();
    stack.reserve(m_points.size());
    stack.push_back(m_points[0]);
    stack.push_back(m_points[1]);

    for (std::size_t i = 2; i < m_points.size(); ++i) {
      const auto& p = m_points[i];
      pop_stale_points(stack, p);
      stack.push_back(p);
    }

    using std::swap;
    swap(m_points, stack);
    assert(m_points.size() > 2);
  }

  void pop_stale_points(std::vector<vec<T, 3>>& stack, const vec<T, 3>& p) {
    if (stack.size() > 1) {
      const auto& t1 = stack[stack.size() - 2];
      const auto& t2 = stack[stack.size() - 1];
      const auto side = is_left(t1, t2, p);
      if (side < 0) {
        stack.pop_back();
        pop_stale_points(stack, p);
      }
    }
  }
};
} // namespace detail

/**
 * Computes the convex hull of the given points. Returns the list of vertices of the polygon which
 * is formed by the convex hull. Note that if the given points are all colinear, or less than 3
 * points are given, then no convex hull exists and the function returns an empty list.
 *
 * @tparam T the component type
 * @param points the points
 * @return the convex hull of the points, or an empty list if no convex hull exists
 */
template <typename T> std::vector<vec<T, 3>> convex_hull(const std::vector<vec<T, 3>>& points) {
  // see http://geomalgorithms.com/a10-_hull-1.html
  const detail::convex_hull<T> hull(points);
  return hull.result();
}
} // namespace vm
