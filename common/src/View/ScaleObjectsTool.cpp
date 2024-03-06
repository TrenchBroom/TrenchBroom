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

#include "ScaleObjectsTool.h"

#include "FloatType.h"
#include "Model/Hit.h"
#include "Model/HitFilter.h"
#include "Model/PickResult.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "View/Grid.h"
#include "View/MapDocument.h"
#include "View/ScaleObjectsToolPage.h"
#include "View/TransactionScope.h"

#include "kdl/memory_utils.h"
#include "kdl/string_utils.h"

#include "vm/bbox.h"
#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/line.h"
#include "vm/vec.h"
#include "vm/vec_io.h"

#include <set>

namespace TrenchBroom
{
namespace View
{
const Model::HitType::Type ScaleObjectsTool::ScaleToolSideHitType =
  Model::HitType::freeType();
const Model::HitType::Type ScaleObjectsTool::ScaleToolEdgeHitType =
  Model::HitType::freeType();
const Model::HitType::Type ScaleObjectsTool::ScaleToolCornerHitType =
  Model::HitType::freeType();

// Scale tool helper functions

bool BBoxSide::validSideNormal(const vm::vec3& n)
{
  for (size_t i = 0; i < 3; ++i)
  {
    vm::vec3 expected = vm::vec3::zero();
    expected[i] = 1.0;
    if (n == expected || n == -expected)
    {
      return true;
    }
  }
  return false;
}

BBoxSide::BBoxSide(const vm::vec3& n)
  : normal(n)
{
  if (!validSideNormal(n))
  {
    throw std::invalid_argument(
      "BBoxSide created with invalid normal " + kdl::str_to_string(n));
  }
}

bool BBoxSide::operator<(const BBoxSide& other) const
{
  return normal < other.normal;
}

bool BBoxSide::operator==(const BBoxSide& other) const
{
  return normal == other.normal;
}

// Corner

bool BBoxCorner::validCorner(const vm::vec3& c)
{
  // all components must be either +1 or -1
  for (size_t i = 0; i < 3; ++i)
  {
    if (!(c[i] == -1.0 || c[i] == 1.0))
    {
      return false;
    }
  }
  return true;
}

BBoxCorner::BBoxCorner(const vm::vec3& c)
  : corner(c)
{
  if (!validCorner(c))
  {
    throw std::invalid_argument(
      "Corner created with invalid corner " + kdl::str_to_string(c));
  }
}

bool BBoxCorner::operator==(const BBoxCorner& other) const
{
  return corner == other.corner;
}

// BBoxEdge

BBoxEdge::BBoxEdge(const vm::vec3& p0, const vm::vec3& p1)
  : point0(p0)
  , point1(p1)
{
  if (!BBoxCorner::validCorner(p0))
  {
    throw std::invalid_argument(
      "BBoxEdge created with invalid corner " + kdl::str_to_string(p0));
  }
  if (!BBoxCorner::validCorner(p1))
  {
    throw std::invalid_argument(
      "BBoxEdge created with invalid corner " + kdl::str_to_string(p1));
  }
}

bool BBoxEdge::operator==(const BBoxEdge& other) const
{
  return point0 == other.point0 && point1 == other.point1;
}

// ProportionalAxes

ProportionalAxes::ProportionalAxes(
  const bool xProportional, const bool yProportional, const bool zProportional)
{
  m_bits.set(0, xProportional);
  m_bits.set(1, yProportional);
  m_bits.set(2, zProportional);
}

ProportionalAxes ProportionalAxes::All()
{
  return ProportionalAxes(true, true, true);
}

ProportionalAxes ProportionalAxes::None()
{
  return ProportionalAxes(false, false, false);
}

void ProportionalAxes::setAxisProportional(const size_t axis, const bool proportional)
{
  m_bits.set(axis, proportional);
}

bool ProportionalAxes::isAxisProportional(const size_t axis) const
{
  return m_bits.test(axis);
}

bool ProportionalAxes::allAxesProportional() const
{
  return m_bits.all();
}

bool ProportionalAxes::operator==(const ProportionalAxes& other) const
{
  return m_bits == other.m_bits;
}

bool ProportionalAxes::operator!=(const ProportionalAxes& other) const
{
  return m_bits != other.m_bits;
}

// Helper functions

std::vector<BBoxSide> allSides()
{
  std::vector<BBoxSide> result;
  result.reserve(6);

  const vm::bbox3 box{{-1, -1, -1}, {1, 1, 1}};
  auto op = [&](
              const vm::vec3& /* p0 */,
              const vm::vec3& /* p1 */,
              const vm::vec3& /* p2 */,
              const vm::vec3& /* p3 */,
              const vm::vec3& normal) { result.push_back(BBoxSide(normal)); };
  box.for_each_face(op);

  assert(result.size() == 6);
  return result;
}

std::vector<BBoxEdge> allEdges()
{
  std::vector<BBoxEdge> result;
  result.reserve(12);

  const vm::bbox3 box{{-1, -1, -1}, {1, 1, 1}};
  auto op = [&](const vm::vec3& p0, const vm::vec3& p1) {
    result.push_back(BBoxEdge(p0, p1));
  };
  box.for_each_edge(op);

  assert(result.size() == 12);
  return result;
}

std::vector<BBoxCorner> allCorners()
{
  std::vector<BBoxCorner> result;
  result.reserve(8);

  const vm::bbox3 box{{-1, -1, -1}, {1, 1, 1}};
  auto op = [&](const vm::vec3& point) { result.push_back(BBoxCorner(point)); };
  box.for_each_vertex(op);

  assert(result.size() == 8);
  return result;
}

vm::vec3 pointForBBoxCorner(const vm::bbox3& box, const BBoxCorner& corner)
{
  vm::vec3 res;
  for (size_t i = 0; i < 3; ++i)
  {
    assert(corner.corner[i] == 1.0 || corner.corner[i] == -1.0);

    res[i] = (corner.corner[i] == 1.0) ? box.max[i] : box.min[i];
  }
  return res;
}

BBoxSide oppositeSide(const BBoxSide& side)
{
  return BBoxSide(side.normal * -1.0);
}

BBoxCorner oppositeCorner(const BBoxCorner& corner)
{
  return BBoxCorner(vm::vec3(-corner.corner.x(), -corner.corner.y(), -corner.corner.z()));
}

BBoxEdge oppositeEdge(const BBoxEdge& edge)
{
  return BBoxEdge(
    oppositeCorner(BBoxCorner(edge.point0)).corner,
    oppositeCorner(BBoxCorner(edge.point1)).corner);
}

vm::segment3 pointsForBBoxEdge(const vm::bbox3& box, const BBoxEdge& edge)
{
  return vm::segment3(
    pointForBBoxCorner(box, BBoxCorner(edge.point0)),
    pointForBBoxCorner(box, BBoxCorner(edge.point1)));
}

vm::polygon3 polygonForBBoxSide(const vm::bbox3& box, const BBoxSide& side)
{
  const auto wantedNormal = side.normal;

  vm::polygon3 res;
  auto visitor = [&](
                   const vm::vec3& p0,
                   const vm::vec3& p1,
                   const vm::vec3& p2,
                   const vm::vec3& p3,
                   const vm::vec3& n) {
    if (n == wantedNormal)
    {
      const vm::polygon3 poly{p0, p1, p2, p3};
      res = poly;
    }
  };
  box.for_each_face(visitor);

  assert(res.vertexCount() == 4);
  return res;
}

vm::vec3 centerForBBoxSide(const vm::bbox3& box, const BBoxSide& side)
{
  const auto wantedNormal = side.normal;

  vm::vec3 result;
  bool setResult = false;

  auto visitor = [&](
                   const vm::vec3& p0,
                   const vm::vec3& p1,
                   const vm::vec3& p2,
                   const vm::vec3& p3,
                   const vm::vec3& n) {
    if (n == wantedNormal)
    {
      result = (p0 + p1 + p2 + p3) / 4.0;
      setResult = true;
    }
  };
  box.for_each_face(visitor);
  assert(setResult);
  return result;
}

// manipulating bboxes

vm::bbox3 moveBBoxSide(
  const vm::bbox3& in,
  const BBoxSide& side,
  const vm::vec3& delta,
  const ProportionalAxes& proportional,
  AnchorPos anchorType)
{
  auto sideLengthDelta = dot(side.normal, delta);

  // when using a center anchor, we're stretching both sides
  // at once, so multiply the delta by 2.
  if (anchorType == AnchorPos::Center)
  {
    sideLengthDelta *= 2.0;
  }

  const auto axis = vm::find_abs_max_component(side.normal);
  const auto inSideLenth = in.max[axis] - in.min[axis];
  const auto sideLength = inSideLenth + sideLengthDelta;

  if (sideLength <= 0)
  {
    return vm::bbox3();
  }

  const auto n = side.normal;
  const auto axis1 = vm::find_abs_max_component(n, 0u);
  const auto axis2 = vm::find_abs_max_component(n, 1u);
  const auto axis3 = vm::find_abs_max_component(n, 2u);

  auto newSize = in.size();

  newSize[axis1] = sideLength;

  // optionally apply proportional scaling to axis2/axis3
  const auto ratio = sideLength / in.size()[axis1];
  if (proportional.isAxisProportional(axis2))
  {
    newSize[axis2] *= ratio;
  }
  if (proportional.isAxisProportional(axis3))
  {
    newSize[axis3] *= ratio;
  }

  const auto anchor = (anchorType == AnchorPos::Center)
                        ? in.center()
                        : centerForBBoxSide(in, oppositeSide(side));

  const auto matrix = vm::scale_bbox_matrix_with_anchor(in, newSize, anchor);

  return vm::bbox3(matrix * in.min, matrix * in.max);
}

vm::bbox3 moveBBoxCorner(
  const vm::bbox3& in,
  const BBoxCorner& corner,
  const vm::vec3& delta,
  const AnchorPos anchorType)
{

  const auto opposite = oppositeCorner(corner);
  const auto oppositePoint = pointForBBoxCorner(in, opposite);
  const auto anchor = (anchorType == AnchorPos::Center) ? in.center() : oppositePoint;
  const auto oldCorner = pointForBBoxCorner(in, corner);
  const auto newCorner = oldCorner + delta;

  // check for inverting the box
  for (size_t i = 0; i < 3; ++i)
  {
    if (newCorner[i] == anchor[i])
    {
      return vm::bbox3();
    }
    const bool oldPositive = oldCorner[i] > anchor[i];
    const bool newPositive = newCorner[i] > anchor[i];
    if (oldPositive != newPositive)
    {
      return vm::bbox3();
    }
  }

  if (anchorType == AnchorPos::Center)
  {
    const auto points = std::vector<vm::vec3>{anchor - (newCorner - anchor), newCorner};
    return vm::bbox3::merge_all(std::begin(points), std::end(points));
  }
  else
  {
    const auto points = std::vector<vm::vec3>{oppositePoint, newCorner};
    return vm::bbox3::merge_all(std::begin(points), std::end(points));
  }
}

vm::bbox3 moveBBoxEdge(
  const vm::bbox3& in,
  const BBoxEdge& edge,
  const vm::vec3& delta,
  const ProportionalAxes& proportional,
  const AnchorPos anchorType)
{

  const auto opposite = oppositeEdge(edge);
  const auto edgeMid = pointsForBBoxEdge(in, edge).center();
  const auto oppositeEdgeMid = pointsForBBoxEdge(in, opposite).center();

  const auto anchor = (anchorType == AnchorPos::Center) ? in.center() : oppositeEdgeMid;

  const auto oldAnchorDist = edgeMid - anchor;
  const auto newAnchorDist = oldAnchorDist + delta;

  // check for crossing over the anchor
  for (size_t i = 0; i < 3; ++i)
  {
    if ((oldAnchorDist[i] > 0) && (newAnchorDist[i] < 0))
    {
      return vm::bbox3();
    }
    if ((oldAnchorDist[i] < 0) && (newAnchorDist[i] > 0))
    {
      return vm::bbox3();
    }
  }

  const auto nonMovingAxis = vm::find_abs_max_component(oldAnchorDist, 2u);

  const auto corner1 =
    (anchorType == AnchorPos::Center) ? anchor - newAnchorDist : anchor;
  const auto corner2 = anchor + newAnchorDist;

  auto p1 = min(corner1, corner2);
  auto p2 = max(corner1, corner2);

  // the only type of proportional scaling we support is optionally
  // scaling the nonMovingAxis.
  if (proportional.isAxisProportional(nonMovingAxis))
  {
    const auto axis1 = vm::find_abs_max_component(oldAnchorDist);
    const auto ratio = (p2 - p1)[axis1] / in.size()[axis1];

    p1[nonMovingAxis] = anchor[nonMovingAxis] - (in.size()[nonMovingAxis] * ratio * 0.5);
    p2[nonMovingAxis] = anchor[nonMovingAxis] + (in.size()[nonMovingAxis] * ratio * 0.5);
  }
  else
  {
    p1[nonMovingAxis] = in.min[nonMovingAxis];
    p2[nonMovingAxis] = in.max[nonMovingAxis];
  }

  const auto result = vm::bbox3(min(p1, p2), max(p1, p2));

  // check for zero size
  if (result.is_empty())
  {
    return vm::bbox3();
  }
  else
  {
    return result;
  }
}

vm::line3 handleLineForHit(const vm::bbox3& bboxAtDragStart, const Model::Hit& hit)
{
  vm::line3 handleLine;

  // NOTE: We don't need to check for the Alt modifier (moves the drag anchor to the
  // center of the bbox) because all of these lines go through the center of the box
  // anyway, so the resulting line would be the same.

  if (hit.type() == ScaleObjectsTool::ScaleToolSideHitType)
  {
    const auto draggingSide = hit.target<BBoxSide>();

    handleLine =
      vm::line3{centerForBBoxSide(bboxAtDragStart, draggingSide), draggingSide.normal};
  }
  else if (hit.type() == ScaleObjectsTool::ScaleToolEdgeHitType)
  {
    const auto endEdge = hit.target<BBoxEdge>();
    const auto startEdge = oppositeEdge(endEdge);

    const vm::segment3 endEdgeActual = pointsForBBoxEdge(bboxAtDragStart, endEdge);
    const vm::segment3 startEdgeActual = pointsForBBoxEdge(bboxAtDragStart, startEdge);

    const vm::vec3 handleLineStart = startEdgeActual.center();
    const vm::vec3 handleLineEnd = endEdgeActual.center();

    handleLine = vm::line3(handleLineStart, normalize(handleLineEnd - handleLineStart));
  }
  else if (hit.type() == ScaleObjectsTool::ScaleToolCornerHitType)
  {
    const auto endCorner = hit.target<BBoxCorner>();
    const auto startCorner = oppositeCorner(endCorner);

    const vm::vec3 handleLineStart = pointForBBoxCorner(bboxAtDragStart, startCorner);
    const vm::vec3 handleLineEnd = pointForBBoxCorner(bboxAtDragStart, endCorner);

    handleLine = vm::line3(handleLineStart, normalize(handleLineEnd - handleLineStart));
  }
  else
  {
    assert(0);
  }

  return handleLine;
}

vm::bbox3 moveBBoxForHit(
  const vm::bbox3& bboxAtDragStart,
  const Model::Hit& dragStartHit,
  const vm::vec3& delta,
  const ProportionalAxes& proportional,
  const AnchorPos anchor)
{
  if (dragStartHit.type() == ScaleObjectsTool::ScaleToolSideHitType)
  {
    const auto endSide = dragStartHit.target<BBoxSide>();

    return moveBBoxSide(bboxAtDragStart, endSide, delta, proportional, anchor);
  }
  else if (dragStartHit.type() == ScaleObjectsTool::ScaleToolEdgeHitType)
  {
    const auto endEdge = dragStartHit.target<BBoxEdge>();

    return moveBBoxEdge(bboxAtDragStart, endEdge, delta, proportional, anchor);
  }
  else if (dragStartHit.type() == ScaleObjectsTool::ScaleToolCornerHitType)
  {
    const auto endCorner = dragStartHit.target<BBoxCorner>();

    return moveBBoxCorner(bboxAtDragStart, endCorner, delta, anchor);
  }
  else
  {
    assert(0);
    return vm::bbox3();
  }
}

// ScaleObjectsTool

ScaleObjectsTool::ScaleObjectsTool(std::weak_ptr<MapDocument> document)
  : Tool(false)
  , m_document(std::move(document))
  , m_toolPage(nullptr)
  , m_resizing(false)
  , m_anchorPos(AnchorPos::Opposite)
  , m_bboxAtDragStart()
  , m_dragStartHit(Model::Hit::NoHit)
  , m_dragCumulativeDelta(vm::vec3::zero())
  , m_proportionalAxes(ProportionalAxes::None())
{
}

ScaleObjectsTool::~ScaleObjectsTool() = default;

bool ScaleObjectsTool::doActivate()
{
  m_toolPage->activate();
  return true;
}

const Grid& ScaleObjectsTool::grid() const
{
  return kdl::mem_lock(m_document)->grid();
}

const Model::Hit& ScaleObjectsTool::dragStartHit() const
{
  return m_dragStartHit;
}

bool ScaleObjectsTool::applies() const
{
  auto document = kdl::mem_lock(m_document);
  return !document->selectedNodes().empty();
}

BackSide pickBackSideOfBox(
  const vm::ray3& pickRay, const Renderer::Camera& /* camera */, const vm::bbox3& box)
{
  auto closestDistToRay = std::numeric_limits<FloatType>::max();
  auto bestDistAlongRay = std::numeric_limits<FloatType>::max();
  vm::vec3 bestNormal;

  // idea is: find the closest point on an edge of the cube, belonging
  // to a face that's facing away from the pick ray.
  auto visitor = [&](
                   const vm::vec3& p0,
                   const vm::vec3& p1,
                   const vm::vec3& p2,
                   const vm::vec3& p3,
                   const vm::vec3& n) {
    const auto cosAngle = dot(n, pickRay.direction);
    if (cosAngle >= 0.0 && cosAngle < 1.0)
    {
      // the face is pointing away from the camera (or exactly perpendicular)
      // but not equal to the camera direction (important for 2D views)

      const vm::vec3 points[] = {p0, p1, p2, p3};
      for (size_t i = 0; i < 4; i++)
      {
        const auto result =
          vm::distance(pickRay, vm::segment3(points[i], points[(i + 1) % 4]));
        if (result.distance < closestDistToRay)
        {
          closestDistToRay = result.distance;
          bestNormal = n;
          bestDistAlongRay = result.position1;
        }
      }
    }
  };
  box.for_each_face(visitor);

  // The hit point is the closest point on the pick ray to one of the edges of the face.
  // For face dragging, we'll project the pick ray onto the line through this point and
  // having the face normal.
  assert(bestNormal != vm::vec3::zero());

  BackSide result;
  result.distAlongRay = bestDistAlongRay;
  result.pickedSideNormal = bestNormal;
  return result;
}

void ScaleObjectsTool::pickBackSides(
  const vm::ray3& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  // select back sides. Used for both 2D and 3D.
  if (pickResult.empty())
  {
    const auto result = pickBackSideOfBox(pickRay, camera, bounds());

    // The hit point is the closest point on the pick ray to one of the edges of the face.
    // For face dragging, we'll project the pick ray onto the line through this point and
    // having the face normal.
    assert(result.pickedSideNormal != vm::vec3::zero());
    pickResult.addHit(Model::Hit(
      ScaleToolSideHitType,
      result.distAlongRay,
      vm::point_at_distance(pickRay, result.distAlongRay),
      BBoxSide{result.pickedSideNormal}));
  }
}

void ScaleObjectsTool::pick2D(
  const vm::ray3& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  using namespace Model::HitFilters;

  const vm::bbox3& myBounds = bounds();

  // origin in bbox
  if (myBounds.contains(pickRay.origin))
  {
    return;
  }

  Model::PickResult localPickResult;

  // bbox corners in 2d views
  assert(camera.orthographicProjection());
  for (const BBoxEdge& edge : allEdges())
  {
    const vm::segment3 points = pointsForBBoxEdge(myBounds, edge);

    // in 2d views, only use edges that are parallel to the camera
    if (vm::is_parallel(points.direction(), vm::vec3(camera.direction())))
    {
      // could figure out which endpoint is closer to camera, or just test both.
      for (const vm::vec3& point : std::vector<vm::vec3>{points.start(), points.end()})
      {
        if (
          const auto dist = camera.pickPointHandle(
            pickRay, point, static_cast<FloatType>(pref(Preferences::HandleRadius))))
        {
          const auto hitPoint = vm::point_at_distance(pickRay, *dist);
          localPickResult.addHit(Model::Hit(ScaleToolEdgeHitType, *dist, hitPoint, edge));
        }
      }
    }
  }

  pickBackSides(pickRay, camera, localPickResult);

  if (!localPickResult.empty())
  {
    pickResult.addHit(localPickResult.all().front());
  }
}

void ScaleObjectsTool::pick3D(
  const vm::ray3& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  using namespace Model::HitFilters;

  const auto& myBounds = bounds();

  // origin in bbox
  if (myBounds.contains(pickRay.origin))
  {
    return;
  }

  Model::PickResult localPickResult;

  // these handles only work in 3D.
  assert(camera.perspectiveProjection());

  // corners
  for (const auto& corner : allCorners())
  {
    const auto point = pointForBBoxCorner(myBounds, corner);

    // make the spheres for the corner handles slightly larger than the
    // cylinders of the edge handles, so they take priority where they overlap.
    const auto cornerRadius =
      static_cast<FloatType>(pref(Preferences::HandleRadius)) * 2.0;
    if (const auto dist = camera.pickPointHandle(pickRay, point, cornerRadius))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *dist);
      localPickResult.addHit(Model::Hit(ScaleToolCornerHitType, *dist, hitPoint, corner));
    }
  }

  // edges
  for (const auto& edge : allEdges())
  {
    const vm::segment3 points = pointsForBBoxEdge(myBounds, edge);

    if (
      const auto dist = camera.pickLineSegmentHandle(
        pickRay, points, static_cast<FloatType>(pref(Preferences::HandleRadius))))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *dist);
      localPickResult.addHit(Model::Hit(ScaleToolEdgeHitType, *dist, hitPoint, edge));
    }
  }

  // sides
  for (const auto& side : allSides())
  {
    const auto poly = polygonForBBoxSide(myBounds, side);

    if (
      const auto dist =
        vm::intersect_ray_polygon(pickRay, std::begin(poly), std::end(poly)))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *dist);
      localPickResult.addHit(Model::Hit(ScaleToolSideHitType, *dist, hitPoint, side));
    }
  }

  pickBackSides(pickRay, camera, localPickResult);

  if (!localPickResult.empty())
  {
    pickResult.addHit(localPickResult.all().front());
  }
}

vm::bbox3 ScaleObjectsTool::bounds() const
{
  auto document = kdl::mem_lock(m_document);
  return document->selectionBounds();
}

// used for rendering

/**
 * For dragging a corner retursn the 3 sides that touch that corner
 */
static std::vector<BBoxSide> sidesForCornerSelection(const BBoxCorner& corner)
{
  std::vector<BBoxSide> result;
  for (size_t i = 0; i < 3; ++i)
  {
    vm::vec3 sideNormal = vm::vec3::zero();
    sideNormal[i] = corner.corner[i];

    result.push_back(BBoxSide(sideNormal));
  }
  assert(result.size() == 3);
  return result;
}

/**
 * For dragging an edge, returns the 2 bbox sides that contain that edge
 */
static std::vector<BBoxSide> sidesForEdgeSelection(const BBoxEdge& edge)
{
  std::vector<BBoxSide> result;

  const vm::bbox3 box{{-1, -1, -1}, {1, 1, 1}};

  auto visitor = [&](
                   const vm::vec3& p0,
                   const vm::vec3& p1,
                   const vm::vec3& p2,
                   const vm::vec3& p3,
                   const vm::vec3& n) {
    const vm::vec3 verts[4] = {p0, p1, p2, p3};

    // look for the edge
    for (size_t i = 0; i < 4; ++i)
    {
      if (
        (verts[i] == edge.point0 && verts[(i + 1) % 4] == edge.point1)
        || (verts[i] == edge.point1 && verts[(i + 1) % 4] == edge.point0))
      {
        result.push_back(BBoxSide(n));
      }
    }
  };
  box.for_each_face(visitor);
  assert(result.size() == 2);

  return result;
}

static std::vector<vm::polygon3f> polysForSides(
  const vm::bbox3& box, const std::vector<BBoxSide>& sides)
{
  std::vector<vm::polygon3f> result;
  for (const auto& side : sides)
  {
    result.push_back(vm::polygon3f(polygonForBBoxSide(box, side)));
  }
  return result;
}

static std::vector<BBoxSide> sidesWithOppositeSides(const std::vector<BBoxSide>& sides)
{
  std::set<BBoxSide> result;
  for (const auto& side : sides)
  {
    result.insert(side);
    result.insert(oppositeSide(side));
  }

  return std::vector<BBoxSide>(result.begin(), result.end());
}

std::vector<vm::polygon3f> ScaleObjectsTool::polygonsHighlightedByDrag() const
{
  std::vector<BBoxSide> sides;

  if (m_dragStartHit.type() == ScaleToolSideHitType)
  {
    const auto side = m_dragStartHit.target<BBoxSide>();
    sides = {side};

    // Add additional highlights when Shift is pressed, to indicate the other axes that
    // are being scaled proportionally.
    for (size_t i = 0; i < 3; ++i)
    {
      // Don't highlight `side` or its opposite
      if (i == vm::find_abs_max_component(side.normal))
      {
        continue;
      }

      if (m_proportionalAxes.isAxisProportional(i))
      {
        // Highlight the + and - sides on this axis
        vm::vec3 side1;
        side1[i] = 1.0;
        sides.emplace_back(side1);

        vm::vec3 side2;
        side2[i] = -1.0;
        sides.emplace_back(side2);
      }
    }
  }
  else if (m_dragStartHit.type() == ScaleToolEdgeHitType)
  {
    const auto edge = m_dragStartHit.target<BBoxEdge>();
    sides = sidesForEdgeSelection(edge);
  }
  else if (m_dragStartHit.type() == ScaleToolCornerHitType)
  {
    const auto corner = m_dragStartHit.target<BBoxCorner>();
    sides = sidesForCornerSelection(corner);
  }
  else
  {
    return {};
  }

  // When the anchor point is the center, highlight the opposite sides also.
  if (m_anchorPos == AnchorPos::Center)
  {
    sides = sidesWithOppositeSides(sides);
  }

  return polysForSides(bounds(), sides);
}

bool ScaleObjectsTool::hasDragSide() const
{
  return dragSide().vertexCount() > 0;
}

vm::polygon3f ScaleObjectsTool::dragSide() const
{
  if (m_dragStartHit.type() == ScaleToolSideHitType)
  {
    const auto side = m_dragStartHit.target<BBoxSide>();
    return vm::polygon3f(polygonForBBoxSide(bounds(), side));
  }

  return vm::polygon3f();
}

bool ScaleObjectsTool::hasDragEdge() const
{
  return m_dragStartHit.type() == ScaleToolEdgeHitType;
}

vm::segment3f ScaleObjectsTool::dragEdge() const
{
  assert(hasDragEdge());
  auto whichEdge = m_dragStartHit.target<BBoxEdge>();
  return vm::segment3f(pointsForBBoxEdge(bounds(), whichEdge));
}

bool ScaleObjectsTool::hasDragCorner() const
{
  return m_dragStartHit.type() == ScaleToolCornerHitType;
}

vm::vec3f ScaleObjectsTool::dragCorner() const
{
  assert(hasDragCorner());
  auto whichCorner = m_dragStartHit.target<BBoxCorner>();
  return vm::vec3f(pointForBBoxCorner(bounds(), whichCorner));
}

bool ScaleObjectsTool::hasDragAnchor() const
{
  if (bounds().is_empty())
  {
    return false;
  }

  const auto type = m_dragStartHit.type();
  return type == ScaleToolEdgeHitType || type == ScaleToolCornerHitType
         || type == ScaleToolSideHitType;
}

vm::vec3f ScaleObjectsTool::dragAnchor() const
{
  if (m_anchorPos == AnchorPos::Center)
  {
    return vm::vec3f(bounds().center());
  }

  if (m_dragStartHit.type() == ScaleToolSideHitType)
  {
    const auto endSide = m_dragStartHit.target<BBoxSide>();
    const auto startSide = oppositeSide(endSide);

    return vm::vec3f(centerForBBoxSide(bounds(), startSide));
  }
  else if (m_dragStartHit.type() == ScaleToolEdgeHitType)
  {
    const auto endEdge = m_dragStartHit.target<BBoxEdge>();
    const auto startEdge = oppositeEdge(endEdge);

    const vm::segment3 startEdgeActual = pointsForBBoxEdge(bounds(), startEdge);

    return vm::vec3f(startEdgeActual.center());
  }
  else if (m_dragStartHit.type() == ScaleToolCornerHitType)
  {
    const auto endCorner = m_dragStartHit.target<BBoxCorner>();
    const auto startCorner = oppositeCorner(endCorner);

    const auto startCornerActual = pointForBBoxCorner(bounds(), startCorner);
    return vm::vec3f(startCornerActual);
  }

  assert(0);
  return vm::vec3f::zero();
}

vm::bbox3 ScaleObjectsTool::bboxAtDragStart() const
{
  ensure(m_resizing, "bboxAtDragStart() can only be called while resizing");
  return m_bboxAtDragStart;
}

std::vector<vm::vec3> ScaleObjectsTool::cornerHandles() const
{
  if (bounds().is_empty())
  {
    return {};
  }

  std::vector<vm::vec3> result;
  result.reserve(8);
  auto op = [&](const vm::vec3& point) { result.push_back(point); };
  bounds().for_each_vertex(op);
  return result;
}

void ScaleObjectsTool::updatePickedHandle(const Model::PickResult& pickResult)
{
  using namespace Model::HitFilters;
  const Model::Hit& hit = pickResult.first(
    type(ScaleToolSideHitType | ScaleToolEdgeHitType | ScaleToolCornerHitType));

  // extract the highlighted handle from the hit here, and only refresh views if it
  // changed
  if (hit.type() == ScaleToolSideHitType && m_dragStartHit.type() == ScaleToolSideHitType)
  {
    if (hit.target<BBoxSide>() == m_dragStartHit.target<BBoxSide>())
    {
      return;
    }
  }
  else if (
    hit.type() == ScaleToolEdgeHitType && m_dragStartHit.type() == ScaleToolEdgeHitType)
  {
    if (hit.target<BBoxEdge>() == m_dragStartHit.target<BBoxEdge>())
    {
      return;
    }
  }
  else if (
    hit.type() == ScaleToolCornerHitType
    && m_dragStartHit.type() == ScaleToolCornerHitType)
  {
    if (hit.target<BBoxCorner>() == m_dragStartHit.target<BBoxCorner>())
    {
      return;
    }
  }

  // hack for highlighting on mouseover
  m_dragStartHit = hit;

  refreshViews();
}

void ScaleObjectsTool::setAnchorPos(const AnchorPos pos)
{
  m_anchorPos = pos;
}

AnchorPos ScaleObjectsTool::anchorPos() const
{
  return m_anchorPos;
}

void ScaleObjectsTool::setProportionalAxes(const ProportionalAxes& proportionalAxes)
{
  m_proportionalAxes = proportionalAxes;
}

const ProportionalAxes& ScaleObjectsTool::proportionalAxes() const
{
  return m_proportionalAxes;
}

void ScaleObjectsTool::startScaleWithHit(const Model::Hit& hit)
{
  ensure(hit.isMatch(), "must start with matching hit");
  ensure(
    hit.type() == ScaleToolCornerHitType || hit.type() == ScaleToolEdgeHitType
      || hit.type() == ScaleToolSideHitType,
    "wrong hit type");
  ensure(!m_resizing, "must not be resizing already");

  m_bboxAtDragStart = bounds();
  m_dragStartHit = hit;
  m_dragCumulativeDelta = vm::vec3::zero();

  auto document = kdl::mem_lock(m_document);
  document->startTransaction("Scale Objects", TransactionScope::LongRunning);
  m_resizing = true;
}

void ScaleObjectsTool::scaleByDelta(const vm::vec3& delta)
{
  ensure(m_resizing, "must be resizing already");

  m_dragCumulativeDelta = m_dragCumulativeDelta + delta;

  auto document = kdl::mem_lock(m_document);

  const auto newBox = moveBBoxForHit(
    m_bboxAtDragStart,
    m_dragStartHit,
    m_dragCumulativeDelta,
    m_proportionalAxes,
    m_anchorPos);

  if (!newBox.is_empty())
  {
    document->scaleObjects(bounds(), newBox);
  }
}

void ScaleObjectsTool::commitScale()
{
  auto document = kdl::mem_lock(m_document);
  if (vm::is_zero(m_dragCumulativeDelta, vm::C::almost_zero()))
  {
    document->cancelTransaction();
  }
  else
  {
    document->commitTransaction();
  }
  m_resizing = false;
}

void ScaleObjectsTool::cancelScale()
{
  auto document = kdl::mem_lock(m_document);
  document->cancelTransaction();
  m_resizing = false;
}

QWidget* ScaleObjectsTool::doCreatePage(QWidget* parent)
{
  assert(m_toolPage == nullptr);
  m_toolPage = new ScaleObjectsToolPage(m_document, parent);
  return m_toolPage;
}
} // namespace View
} // namespace TrenchBroom
