/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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

#include "FloatType.h"
#include "Model/Hit.h"
#include "Model/HitType.h"
#include "View/Tool.h"

#include <vecmath/bbox.h>
#include <vecmath/vec.h>

#include <bitset>
#include <memory>
#include <vector>

namespace TrenchBroom {
namespace Model {
class PickResult;
}

namespace Renderer {
class Camera;
}

namespace View {
class Grid;
class MapDocument;
class ScaleObjectsToolPage;

/**
 * Identifies the side of a bbox using a normal. The normal will be one of +/- 1.0 along X, Y, or Z.
 */
class BBoxSide {
public:
  vm::vec3 normal;

  static bool validSideNormal(const vm::vec3& n);
  explicit BBoxSide(const vm::vec3& n);
  bool operator<(const BBoxSide& other) const;
  bool operator==(const BBoxSide& other) const;
};

/**
 * Identifies a bbox corner, using a point on a bbox whose corners are at +/- 1.0
 * (i.e. a 2x2x2 box centered at 0, 0, 0).
 */
class BBoxCorner {
public:
  vm::vec3 corner;

  static bool validCorner(const vm::vec3& c);
  explicit BBoxCorner(const vm::vec3& c);

  bool operator==(const BBoxCorner& other) const;
};

/**
 * Identifies a directed edge of a bbox, using points on a bbox whose corners are at +/- 1.0
 * (i.e. a 2x2x2 box centered at 0, 0, 0).
 */
class BBoxEdge {
public:
  vm::vec3 point0;
  vm::vec3 point1;

  explicit BBoxEdge(const vm::vec3& p0, const vm::vec3& p1);

  bool operator==(const BBoxEdge& other) const;
};

enum class AnchorPos
{
  Opposite,
  Center
};

/**
 * A set containing a subset of (X, Y, Z). Identifies which axes will be scaled.
 * Used for recording which axes the Shift key affects in 2D views.
 */
class ProportionalAxes {
private:
  std::bitset<3> m_bits;

public:
  ProportionalAxes(bool xProportional, bool yProportional, bool zProportional);

  static ProportionalAxes All();
  static ProportionalAxes None();

  void setAxisProportional(size_t axis, bool proportional);
  bool isAxisProportional(size_t axis) const;
  bool allAxesProportional() const;

  bool operator==(const ProportionalAxes& other) const;
  bool operator!=(const ProportionalAxes& other) const;
};

std::vector<BBoxSide> allSides();
std::vector<BBoxEdge> allEdges();
std::vector<BBoxCorner> allCorners();
vm::vec3 pointForBBoxCorner(const vm::bbox3& box, const BBoxCorner& corner);
BBoxSide oppositeSide(const BBoxSide& side);
BBoxCorner oppositeCorner(const BBoxCorner& corner);
BBoxEdge oppositeEdge(const BBoxEdge& edge);
vm::segment3 pointsForBBoxEdge(const vm::bbox3& box, const BBoxEdge& edge);
vm::polygon3 polygonForBBoxSide(const vm::bbox3& box, const BBoxSide& side);
vm::vec3 centerForBBoxSide(const vm::bbox3& box, const BBoxSide& side);

/**
 * Computes a new bbox after moving the given side by the given delta.
 *
 * Only the component of `delta` matching the axis of `side` is used.
 *
 * `proportional` controls which other axes are scaled.
 *
 * Returns BBox3(Vec3::Null, Vec3::Null) if the move could not be completed
 * because the specified delta either collapses the bbox, or inverts it.
 */
vm::bbox3 moveBBoxSide(
  const vm::bbox3& in, const BBoxSide& side, const vm::vec3& delta,
  const ProportionalAxes& proportional, AnchorPos anchor);

/**
 * Computes a new bbox after moving the given corner by the given delta.
 *
 * All components of the `delta` are used.
 *
 * Returns BBox3(Vec3::Null, Vec3::Null) if the move could not be completed
 * because the specified delta either collapses the bbox, or inverts it.
 */
vm::bbox3 moveBBoxCorner(
  const vm::bbox3& in, const BBoxCorner& corner, const vm::vec3& delta, AnchorPos anchor);

/**
 * Computes a new bbox after moving the specified edge by the specified delta.
 *
 * If `edge` points along an axis i, the ith component of `delta` is ignored.
 * `proportional` only controls whether the bbox grows along axis `i`.
 *
 * Returns BBox3(Vec3::Null, Vec3::Null) if the move could not be completed
 * because the specified delta either collapses the bbox, or inverts it.
 */
vm::bbox3 moveBBoxEdge(
  const vm::bbox3& in, const BBoxEdge& edge, const vm::vec3& delta,
  const ProportionalAxes& proportional, AnchorPos anchor);

/**
 * Returns the line through the bbox that an invisible handle should be dragged, assuming
 * proportional dragging on all 3 axes.
 *
 * Only looks at the hit type (corner/edge/side), and which particular corner/edge/side.
 */
vm::line3 handleLineForHit(const vm::bbox3& bboxAtDragStart, const Model::Hit& hit);

/**
 * Wrapper around moveBBoxSide/moveBBoxEdge/moveBBoxCorner.
 *
 * Looks in the `dragStartHit` and calls the appropriate move function based on whether a side,
 * edge, or corner handle was grabbed.
 */
vm::bbox3 moveBBoxForHit(
  const vm::bbox3& bboxAtDragStart, const Model::Hit& dragStartHit, const vm::vec3& delta,
  const ProportionalAxes& proportional, AnchorPos anchor);

struct BackSide {
  FloatType distAlongRay;
  vm::vec3 pickedSideNormal;
};

/**
 * Picks a "back side" of the given box; this is used when the mouse is not over the selection.
 *
 * Among the faces of the box that are facing away from the camera, finds the one that comes closest
 * to the pick ray.
 *
 * Returns the point on the pick ray (stored as a distance along the ray) that is closest to the
 * selected face, as well as that face's normal.
 */
BackSide pickBackSideOfBox(
  const vm::ray3& pickRay, const Renderer::Camera& camera, const vm::bbox3& box);

class ScaleObjectsTool : public Tool {
public:
  static const Model::HitType::Type ScaleToolSideHitType;
  static const Model::HitType::Type ScaleToolEdgeHitType;
  static const Model::HitType::Type ScaleToolCornerHitType;

private:
  std::weak_ptr<MapDocument> m_document;
  ScaleObjectsToolPage* m_toolPage;
  bool m_resizing;
  AnchorPos m_anchorPos;
  vm::bbox3 m_bboxAtDragStart;
  Model::Hit m_dragStartHit; // contains the drag type (side/edge/corner)
  vm::vec3 m_dragCumulativeDelta;
  ProportionalAxes m_proportionalAxes;

public:
  explicit ScaleObjectsTool(std::weak_ptr<MapDocument> document);
  ~ScaleObjectsTool() override;

  bool doActivate() override;

  const Grid& grid() const;

  const Model::Hit& dragStartHit() const;
  bool applies() const;

  void pickBackSides(
    const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;
  void pick2D(
    const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;
  void pick3D(
    const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult) const;

public:
  vm::bbox3 bounds() const;

public:
  std::vector<vm::polygon3f> polygonsHighlightedByDrag() const;

  bool hasDragSide() const;
  vm::polygon3f dragSide() const;

  bool hasDragEdge() const;
  vm::segment3f dragEdge() const;

  bool hasDragCorner() const;
  vm::vec3f dragCorner() const;

  bool hasDragAnchor() const;
  vm::vec3f dragAnchor() const;

  /**
   * Returns the bbox at the start of the drag. Only allowed to call while m_resizing is true.
   */
  vm::bbox3 bboxAtDragStart() const;

  std::vector<vm::vec3> cornerHandles() const;

  void updatePickedHandle(const Model::PickResult& pickResult);

  void setAnchorPos(AnchorPos pos);
  AnchorPos anchorPos() const;

  void setProportionalAxes(const ProportionalAxes& proportionalAxes);
  const ProportionalAxes& proportionalAxes() const;

public:
  void startScaleWithHit(const Model::Hit& hit);
  void scaleByDelta(const vm::vec3& delta);
  void commitScale();
  void cancelScale();

private:
  QWidget* doCreatePage(QWidget* parent) override;
};
} // namespace View
} // namespace TrenchBroom
