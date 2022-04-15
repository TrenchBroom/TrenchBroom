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

#pragma once

#include "Ensure.h"
#include "FloatType.h"
#include "Macros.h"
#include "Notifier.h"

#include <vecmath/forward.h>
#include <vecmath/intersection.h>
#include <vecmath/plane.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>

#include <array>

// FIXME: should this be moved to Model?
namespace TrenchBroom {
namespace Model {
class BrushFace;
}

namespace View {
class Grid {
public:
  static const int MaxSize = 8;
  static const int MinSize = -3;

private:
  int m_size;
  bool m_snap;
  bool m_visible;

public:
  Notifier<> gridDidChangeNotifier;

public:
  explicit Grid(int size);

  static FloatType actualSize(int size);

  int size() const;
  void setSize(int size);
  void incSize();
  void decSize();
  FloatType actualSize() const;
  /**
   * Snap increment in radians for angle snapping
   */
  FloatType angle() const;

  bool visible() const;
  void toggleVisible();

  bool snap() const;
  void toggleSnap();

  template <typename T> T snapAngle(const T a) const { return snapAngle(a, angle()); }

  /**
   * Snaps the given angle `a` to the nearest multiple of `snapAngle`, if grid snapping is enabled.
   */
  template <typename T> T snapAngle(const T a, const T snapAngle) const {
    if (!snap()) {
      return a;
    } else {
      return snapAngle * vm::round(a / snapAngle);
    }
  }

public: // Snap scalars.
  template <typename T> T snap(const T f) const { return snap(f, SnapDir_None); }

  template <typename T> T offset(const T f) const {
    if (!snap()) {
      return static_cast<T>(0.0);
    } else {
      return f - snap(f);
    }
  }

  template <typename T> T snapUp(const T f, const bool skip) const {
    return snap(f, SnapDir_Up, skip);
  }

  template <typename T> T snapDown(const T f, const bool skip) const {
    return snap(f, SnapDir_Down, skip);
  }

private:
  typedef enum {
    /**
     * Snap to nearest grid increment (rounding away from 0 if the input is half way between two
     * multiples of the grid size).
     */
    SnapDir_None,
    /**
     * If off-grid, snap to the next larger grid increment.
     */
    SnapDir_Up,
    /**
     * If off-grid, snap to the next smaller grid increment.
     */
    SnapDir_Down
  } SnapDir;

  /**
   * Snaps a scalar to the grid.
   *
   * @tparam T scalar type
   * @param f scalar to snap
   * @param snapDir snap direction, see SnapDir
   * @param skip If true, SnapDir_Up/SnapDir_Down snap to the next larger/smaller grid increment
   * even if the input is already on-grid (within almost_zero()). If false, on-grid inputs stay at
   * the same grid increment.
   * @return snapped scalar
   */
  template <typename T> T snap(const T f, const SnapDir snapDir, const bool skip = false) const {
    if (!snap()) {
      return f;
    }

    const T actSize = static_cast<T>(actualSize());
    switch (snapDir) {
      case SnapDir_None:
        return vm::snap(f, actSize);
      case SnapDir_Up: {
        const T s = actSize * std::ceil(f / actSize);
        return (skip && vm::is_equal(s, f, vm::constants<T>::almost_zero()))
                 ? s + static_cast<T>(actualSize())
                 : s;
      }
      case SnapDir_Down: {
        const T s = actSize * std::floor(f / actSize);
        return (skip && vm::is_equal(s, f, vm::constants<T>::almost_zero()))
                 ? s - static_cast<T>(actualSize())
                 : s;
      }
        switchDefault();
    }
  }

public: // Snap vectors.
  /**
   * Snap each component to the nearest grid increment.
   */
  template <typename T, size_t S> vm::vec<T, S> snap(const vm::vec<T, S>& p) const {
    return snap(p, SnapDir_None);
  }

  template <typename T, size_t S> vm::vec<T, S> offset(const vm::vec<T, S>& p) const {
    if (!snap()) {
      return vm::vec<T, S>::zero();
    } else {
      return p - snap(p);
    }
  }

  template <typename T, size_t S>
  vm::vec<T, S> snapUp(const vm::vec<T, S>& p, const bool skip = false) const {
    return snap(p, SnapDir_Up, skip);
  }

  template <typename T, size_t S>
  vm::vec<T, S> snapDown(const vm::vec<T, S>& p, const bool skip = false) const {
    return snap(p, SnapDir_Down, skip);
  }

private:
  template <typename T, size_t S>
  vm::vec<T, S> snap(const vm::vec<T, S>& p, const SnapDir snapDir, const bool skip = false) const {
    if (!snap()) {
      return p;
    }
    vm::vec<T, S> result;
    for (size_t i = 0; i < S; ++i) {
      result[i] = snap(p[i], snapDir, skip);
    }
    return result;
  }

public: // Snap towards an arbitrary direction.
  template <typename T, size_t S>
  vm::vec<T, S> snapTowards(
    const vm::vec<T, S>& p, const vm::vec<T, S>& d, const bool skip = false) const {
    if (!snap()) {
      return p;
    }
    vm::vec3 result;
    for (size_t i = 0; i < S; ++i) {
      if (d[i] > T(0.0)) {
        result[i] = snapUp(p[i], skip);
      } else if (d[i] < T(0.0)) {
        result[i] = snapDown(p[i], skip);
      } else {
        result[i] = snap(p[i]);
      }
    }
    return result;
  }

public: // Snapping on a plane.
  template <typename T>
  vm::vec<T, 3> snap(const vm::vec<T, 3>& p, const vm::plane<T, 3>& onPlane) const {
    return snap(p, onPlane, SnapDir_None, false);
  }

  template <typename T>
  vm::vec<T, 3> snapUp(
    const vm::vec<T, 3>& p, const vm::plane<T, 3>& onPlane, const bool skip = false) const {
    return snap(p, onPlane, SnapDir_Up, skip);
  }

  template <typename T>
  vm::vec<T, 3> snapDown(
    const vm::vec<T, 3>& p, const vm::plane<T, 3>& onPlane, const bool skip = false) const {
    return snap(p, onPlane, SnapDir_Down, skip);
  }

  template <typename T, size_t S>
  vm::vec<T, S> snapTowards(
    const vm::vec<T, S>& p, const vm::plane<T, 3>& onPlane, const vm::vec<T, S>& d,
    const bool skip = false) const {

    SnapDir snapDirs[S];
    for (size_t i = 0; i < S; ++i) {
      snapDirs[i] = (d[i] < 0.0 ? SnapDir_Down : (d[i] > 0.0 ? SnapDir_Up : SnapDir_None));
    }

    return snap(p, onPlane, snapDirs, skip);
  }

private:
  template <typename T, size_t S>
  vm::vec<T, 3> snap(
    const vm::vec<T, S>& p, const vm::plane<T, S>& onPlane, const SnapDir snapDir,
    const bool skip = false) const {
    SnapDir snapDirs[S];
    for (size_t i = 0; i < S; ++i) {
      snapDirs[i] = snapDir;
    }

    return snap(p, onPlane, snapDirs, skip);
  }

  /**
   * Snaps p to grid on the two axes that aren't onPlane's major axis, then projects
   * these two coordinates onto the plane to get the third axis. The resulting point will be on the
   * plane and have two axes snapped to grid.
   */
  template <typename T, size_t S>
  vm::vec<T, S> snap(
    const vm::vec<T, S>& p, const vm::plane<T, 3>& onPlane, const SnapDir snapDirs[],
    const bool skip = false) const {

    vm::vec<T, 3> result;
    switch (vm::find_abs_max_component(onPlane.normal)) {
      case vm::axis::x:
        result[1] = snap(p.y(), snapDirs[1], skip);
        result[2] = snap(p.z(), snapDirs[2], skip);
        result[0] = onPlane.xAt(result.yz());
        break;
      case vm::axis::y:
        result[0] = snap(p.x(), snapDirs[0], skip);
        result[2] = snap(p.z(), snapDirs[2], skip);
        result[1] = onPlane.yAt(result.xz());
        break;
      case vm::axis::z:
        result[0] = snap(p.x(), snapDirs[0], skip);
        result[1] = snap(p.y(), snapDirs[1], skip);
        result[2] = onPlane.zAt(result.xy());
        break;
    }
    return result;
  }

public:
  // Snapping on an a line means finding the closest point on a line such that at least one
  // coordinate is on the grid, ignoring a coordinate if the line direction is identical to the
  // corresponding axis.
  template <typename T>
  vm::vec<T, 3> snap(const vm::vec<T, 3>& p, const vm::line<T, 3> line) const {
    // Project the point onto the line.
    const auto pr = vm::project_point(line, p);
    const auto prDist = vm::distance_to_projected_point(line, pr);

    auto result = pr;
    auto bestDiff = std::numeric_limits<T>::max();
    for (size_t i = 0; i < 3; ++i) {
      if (line.direction[i] != 0.0) {
        const std::array<T, 2> v = {
          {snapDown(pr[i], false) - line.point[i], snapUp(pr[i], false) - line.point[i]}};
        for (size_t j = 0; j < 2; ++j) {
          const auto s = v[j] / line.direction[i];
          const auto diff = vm::abs_difference(s, prDist);
          if (diff < bestDiff) {
            result = vm::point_at_distance(line, s);
            bestDiff = diff;
          }
        }
      }
    }

    return result;
  }

  template <typename T>
  vm::vec<T, 3> snap(const vm::vec<T, 3>& p, const vm::segment<T, 3> edge) const {
    const auto v = edge.end() - edge.start();
    const auto len = length(v);

    const auto orig = edge.start();
    const auto dir = v / len;

    const auto snapped = snap(p, vm::line<T, 3>(orig, dir));
    const auto dist = vm::dot(dir, snapped - orig);

    if (dist < 0.0 || dist > len) {
      return vm::vec<T, 3>::nan();
    } else {
      return snapped;
    }
  }

  template <typename T>
  vm::vec<T, 3> snap(
    const vm::vec<T, 3>& p, const vm::polygon<T, 3>& polygon, const vm::vec<T, 3>& normal) const {
    ensure(polygon.vertexCount() >= 3, "polygon has too few vertices");

    const auto plane = vm::plane<T, 3>(polygon.vertices().front(), normal);
    auto ps = snap(p, plane);
    auto err = vm::squared_length(p - ps);

    if (!vm::polygon_contains_point(ps, plane.normal, std::begin(polygon), std::end(polygon))) {
      ps = vm::vec<T, 3>::nan();
      err = std::numeric_limits<T>::max();
    }

    auto last = std::begin(polygon);
    auto cur = std::next(last);
    auto end = std::end(polygon);

    while (cur != end) {
      const auto cand = snap(p, vm::segment<T, 3>(*last, *cur));
      if (!vm::is_nan(cand)) {
        const auto cerr = vm::squared_length(p - cand);
        if (cerr < err) {
          err = cerr;
          ps = cand;
        }
      }

      last = cur;
      ++cur;
    }

    return ps;
  }

public:
  FloatType intersectWithRay(const vm::ray3& ray, size_t skip) const;

  /**
   * Returns a copy of `delta` that snaps the result to grid, if the grid snapping moves the result
   * in the same direction as delta (tested on each axis). Otherwise, returns the original point for
   * that axis.
   */
  vm::vec3 moveDeltaForPoint(const vm::vec3& point, const vm::vec3& delta) const;
  vm::vec3 moveDeltaForBounds(
    const vm::plane3& targetPlane, const vm::bbox3& bounds, const vm::bbox3& worldBounds,
    const vm::ray3& ray) const;

  /**
   * Given a line and a point X on the line (via the distance from the line's origin), returns the
   * distance to a point Y on the line such that Y is on the intersection of the line with a grid
   * plane, and the distance between X and Y is minimal among all such points.
   */
  FloatType snapToGridPlane(const vm::line3& line, FloatType distance) const;

  vm::vec3 snapMoveDeltaForFace(const Model::BrushFace& face, const vm::vec3& delta) const;

  vm::vec3 referencePoint(const vm::bbox3& bounds) const;
};
} // namespace View
} // namespace TrenchBroom
