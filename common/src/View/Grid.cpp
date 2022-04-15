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

#include "Grid.h"

#include "FloatType.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushGeometry.h"
#include "Model/BrushNode.h"
#include "Model/Polyhedron.h"

#include <vecmath/intersection.h>
#include <vecmath/ray.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>

#include <cmath>

namespace TrenchBroom {
namespace View {
Grid::Grid(const int size)
  : m_size(size)
  , m_snap(true)
  , m_visible(true) {}

FloatType Grid::actualSize(const int size) {
  return std::exp2(size);
}

int Grid::size() const {
  return m_size;
}

void Grid::setSize(const int size) {
  assert(size <= MaxSize);
  assert(size >= MinSize);
  m_size = size;
  gridDidChangeNotifier();
}

void Grid::incSize() {
  if (m_size < MaxSize) {
    ++m_size;
    gridDidChangeNotifier();
  }
}

void Grid::decSize() {
  if (m_size > MinSize) {
    --m_size;
    gridDidChangeNotifier();
  }
}

FloatType Grid::actualSize() const {
  if (snap()) {
    return actualSize(m_size);
  }
  return FloatType(1);
}

FloatType Grid::angle() const {
  return vm::to_radians(static_cast<FloatType>(15.0));
}

bool Grid::visible() const {
  return m_visible;
}

void Grid::toggleVisible() {
  m_visible = !m_visible;
  gridDidChangeNotifier();
}

bool Grid::snap() const {
  return m_snap;
}

void Grid::toggleSnap() {
  m_snap = !m_snap;
  gridDidChangeNotifier();
}

FloatType Grid::intersectWithRay(const vm::ray3& ray, const size_t skip) const {
  vm::vec3 planeAnchor;

  for (size_t i = 0; i < 3; ++i) {
    planeAnchor[i] =
      ray.direction[i] > 0.0
        ? snapUp(ray.origin[i], true) + static_cast<FloatType>(skip) * actualSize()
        : snapDown(ray.origin[i], true) - static_cast<FloatType>(skip) * actualSize();
  }

  const auto distX = vm::intersect_ray_plane(ray, vm::plane3(planeAnchor, vm::vec3::pos_x()));
  const auto distY = vm::intersect_ray_plane(ray, vm::plane3(planeAnchor, vm::vec3::pos_y()));
  const auto distZ = vm::intersect_ray_plane(ray, vm::plane3(planeAnchor, vm::vec3::pos_z()));

  auto dist = distX;
  if (!vm::is_nan(distY) && (vm::is_nan(dist) || std::abs(distY) < std::abs(dist))) {
    dist = distY;
  }
  if (!vm::is_nan(distZ) && (vm::is_nan(dist) || std::abs(distZ) < std::abs(dist))) {
    dist = distZ;
  }
  return dist;
}

vm::vec3 Grid::moveDeltaForPoint(const vm::vec3& point, const vm::vec3& delta) const {
  const auto newPoint = snap(point + delta);
  auto actualDelta = newPoint - point;

  for (size_t i = 0; i < 3; ++i) {
    if (
      (actualDelta[i] > static_cast<FloatType>(0.0)) != (delta[i] > static_cast<FloatType>(0.0))) {
      actualDelta[i] = static_cast<FloatType>(0.0);
    }
  }
  return actualDelta;
}

/**
 * Suggests a placement for a box of the given size following some heuristics described below.
 *
 * The placement is returned as a delta from bounds.min (which is not used, otherwise).
 * Intended to be used for placing objects (e.g. when pasting, or dragging from the entity browser)
 *
 * - One of the box corners is placed at the ray/targetPlane intersection, grid snapped
 *   (snapping towards the ray origin)
 * - Exception to the previous point: if the targetPlane is axial plane,
 *   we'll treat the plane's normal axis as "on grid" even if it's not. This allows, e.g. pasting on
 *   top of 1 unit thick floor detail on grid 8.
 * - The box is positioned so it's above the targetPlane (snapped to axial). It might
 *   clip into targetPlane.
 * - The box is positioned so it's on the opposite side of the ray/targetPlane intersection point
 *   from the pickRay source. The effect of this rule is, when dragging an entity from the entity
 * browser onto the map, the mouse is always grabbing the edge of the entity bbox that's closest to
 * the camera.
 */
vm::vec3 Grid::moveDeltaForBounds(
  const vm::plane3& targetPlane, const vm::bbox3& bounds, const vm::bbox3& /* worldBounds */,
  const vm::ray3& ray) const {
  // First, find the ray/plane intersection, and snap it to grid.
  // This will become one of the corners of our resulting bbox.
  // Note that this means we might let the box clip into the plane somewhat.
  const FloatType dist = vm::intersect_ray_plane(ray, targetPlane);
  const vm::vec3 hitPoint = vm::point_at_distance(ray, dist);

  // Local axis system where Z is the largest magnitude component of targetPlane.normal, and X and Y
  // are the other two axes.
  const size_t localZ = vm::find_abs_max_component(targetPlane.normal, 0);
  const size_t localX = vm::find_abs_max_component(targetPlane.normal, 1);
  const size_t localY = vm::find_abs_max_component(targetPlane.normal, 2);

  vm::vec3 firstCorner = snapTowards(hitPoint, -ray.direction);
  if (vm::is_equal(
        targetPlane.normal, vm::get_abs_max_component_axis(targetPlane.normal),
        vm::C::almost_zero())) {
    // targetPlane is axial. As a special case, only snap X and Y
    firstCorner[localZ] = hitPoint[localZ];
  }

  vm::vec3 newMinPos = firstCorner;

  // The remaining task is to decide which corner of the bbox firstCorner is.
  // Start with using firstCorner as the bbox min, and for each axis,
  // we'll either subtract the box size along that axis (or not) to shift the box position.

  // 1. Look at the component of targetPlane.normal with the greatest magnitude.
  if (targetPlane.normal[localZ] < 0.0) {
    // The plane normal we're snapping to is negative in localZ (e.g. a ceiling), so
    // align the box max with the snap point on that axis
    newMinPos[localZ] -= bounds.size()[localZ];
  }
  // else, the plane normal is positive in localZ (e.g. a floor), so newMinPos
  // is already the correct box min position on that axis.

  // 2. After dealing with localZ, we'll adjust the box position on the other
  // two axes so it's furthest from the source of the ray. See moveDeltaForBounds() docs
  // for the rationale.
  if (ray.direction[localX] < 0.0) {
    newMinPos[localX] -= bounds.size()[localX];
  }
  if (ray.direction[localY] < 0.0) {
    newMinPos[localY] -= bounds.size()[localY];
  }

  return newMinPos - bounds.min;
}

FloatType Grid::snapToGridPlane(const vm::line3& line, const FloatType distance) const {
  // the difference between the distance to X and the distance to Y
  auto difference = std::numeric_limits<FloatType>::max();

  // x is a point on the line and it is located in one grid cube
  const auto x = vm::point_at_distance(line, distance);

  // find the corner of the grid that is closest to x:
  const auto c = snap(x);

  // intersect l with every grid plane that meets at that corner
  for (size_t i = 0; i < 3; ++i) {
    const auto p = vm::plane3{c, vm::vec3::axis(i)};
    const auto y = vm::intersect_line_plane(line, p);
    if (!vm::is_nan(y)) {
      difference = vm::abs_min(difference, y - distance);
    }
  }

  assert(!vm::is_nan(difference));
  return distance + difference;
}

vm::vec3 Grid::snapMoveDeltaForFace(const Model::BrushFace& face, const vm::vec3& moveDelta) const {
  const auto isBoundaryEdge = [&](const Model::BrushEdge* edge) {
    return edge->firstFace() == face.geometry() || edge->secondFace() == face.geometry();
  };

  const auto moveDirection = vm::normalize(moveDelta);
  const auto moveDistance = vm::dot(moveDelta, moveDirection);
  auto difference = std::numeric_limits<FloatType>::max();

  for (const auto* vertex : face.vertices()) {
    const auto* currentHalfEdge = vertex->leaving();
    do {
      if (!isBoundaryEdge(currentHalfEdge->edge())) {
        // compute how far the vertex has to move along its edge vector to hit a grid plane
        const auto edgeDirection = vm::normalize(currentHalfEdge->vector());
        const auto distanceOnEdge = vm::dot(moveDelta, edgeDirection);
        const auto line = vm::line3{currentHalfEdge->origin()->position(), edgeDirection};
        const auto snappedDistance = snapToGridPlane(line, distanceOnEdge);

        // convert this to a movement along moveDirection and minimize the difference
        const auto snappedDeltaEdge = snappedDistance * edgeDirection;
        const auto snappedMoveDistance = vm::dot(snappedDeltaEdge, moveDirection);
        difference = vm::abs_min(difference, snappedMoveDistance - moveDistance);
      }
      currentHalfEdge = currentHalfEdge->nextIncident();
    } while (currentHalfEdge != vertex->leaving());
  }

  // difference is now minimal among all vertices and grid planes
  // we correct the move delta so that a vertex lands on a grid plane
  return moveDirection * (moveDistance + difference);
}

vm::vec3 Grid::referencePoint(const vm::bbox3& bounds) const {
  return snap(bounds.center());
}
} // namespace View
} // namespace TrenchBroom
