/*
 Copyright (C) 2010 Kristian Duske
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

#include "ScaleTool.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/Grid.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/PickResult.h"
#include "mdl/TransactionScope.h"
#include "render/Camera.h"
#include "ui/ScaleToolPage.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/reflection_impl.h"

#include "vm/bbox.h"
#include "vm/distance.h"
#include "vm/polygon.h"
#include "vm/vec.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <ranges>
#include <set>

namespace tb::ui
{
namespace
{

bool validSideNormal(const vm::vec3d& normal)
{
  return std::ranges::any_of(std::views::iota(0u, 3u), [&](const auto i) {
    return vm::abs(normal) == vm::vec3d::axis(i);
  });
}

bool validCorner(const vm::vec3d& corner)
{
  return std::ranges::all_of(
    std::views::iota(0u, 3u), [&](const auto i) { return vm::abs(corner[i]) == 1.0; });
}

/**
 * For dragging a corner retursn the 3 sides that touch that corner
 */
std::vector<BBoxSide> sidesForCornerSelection(const BBoxCorner& corner)
{
  return std::views::iota(0u, 3u) | std::views::transform([&](const auto i) {
           auto sideNormal = vm::vec3d{};
           sideNormal[i] = corner.corner[i];
           return BBoxSide{sideNormal};
         })
         | kdl::ranges::to<std::vector>();
}

/**
 * For dragging an edge, returns the 2 bbox sides that contain that edge
 */
std::vector<BBoxSide> sidesForEdgeSelection(const BBoxEdge& edge)
{
  auto result = std::vector<BBoxSide>{};

  auto visitor =
    [&](const auto& p0, const auto& p1, const auto& p2, const auto& p3, const auto& n) {
      const vm::vec3d verts[4] = {p0, p1, p2, p3};

      // look for the edge
      for (size_t i = 0; i < 4; ++i)
      {
        if (
          (verts[i] == edge.point0 && verts[(i + 1) % 4] == edge.point1)
          || (verts[i] == edge.point1 && verts[(i + 1) % 4] == edge.point0))
        {
          result.emplace_back(n);
        }
      }
    };

  vm::bbox3d{{-1, -1, -1}, {1, 1, 1}}.for_each_face(visitor);
  assert(result.size() == 2);

  return result;
}

std::vector<vm::polygon3f> polysForSides(
  const vm::bbox3d& box, const std::vector<BBoxSide>& sides)
{
  return sides | std::views::transform([&](const auto& side) {
           return vm::polygon3f{polygonForBBoxSide(box, side)};
         })
         | kdl::ranges::to<std::vector>();
}

std::vector<BBoxSide> sidesWithOppositeSides(const std::vector<BBoxSide>& sides)
{
  auto result = std::set<BBoxSide>{};
  for (const auto& side : sides)
  {
    result.insert(side);
    result.insert(oppositeSide(side));
  }

  return {result.begin(), result.end()};
}

} // namespace

const mdl::HitType::Type ScaleTool::ScaleToolSideHitType = mdl::HitType::freeType();
const mdl::HitType::Type ScaleTool::ScaleToolEdgeHitType = mdl::HitType::freeType();
const mdl::HitType::Type ScaleTool::ScaleToolCornerHitType = mdl::HitType::freeType();

// Scale tool helper functions

BBoxSide::BBoxSide(const vm::vec3d& n)
  : normal{n}
{
  if (!validSideNormal(n))
  {
    throw std::invalid_argument{
      fmt::format("BBoxSide created with invalid normal {}", fmt::streamed(n))};
  }
}

kdl_reflect_impl(BBoxSide);

// Corner

BBoxCorner::BBoxCorner(const vm::vec3d& c)
  : corner(c)
{
  if (!validCorner(c))
  {
    throw std::invalid_argument{
      fmt::format("Corner created with invalid corner {}", fmt::streamed(c))};
  }
}

kdl_reflect_impl(BBoxCorner);

// BBoxEdge

BBoxEdge::BBoxEdge(const vm::vec3d& p0, const vm::vec3d& p1)
  : point0{p0}
  , point1{p1}
{
  if (!validCorner(p0))
  {
    throw std::invalid_argument{
      fmt::format("BBoxEdge created with invalid corner {}", fmt::streamed(p0))};
  }
  if (!validCorner(p1))
  {
    throw std::invalid_argument{
      fmt::format("BBoxEdge created with invalid corner {}", fmt::streamed(p1))};
  }
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
  return {true, true, true};
}

ProportionalAxes ProportionalAxes::None()
{
  return {false, false, false};
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

kdl_reflect_impl(ProportionalAxes);

// Helper functions

std::vector<BBoxSide> allSides()
{
  auto result = std::vector<BBoxSide>{};
  result.reserve(6);

  vm::bbox3d{{-1, -1, -1}, {1, 1, 1}}.for_each_face(
    [&](const auto&, const auto&, const auto&, const auto&, const auto& n) {
      result.emplace_back(n);
    });

  assert(result.size() == 6);
  return result;
}

std::vector<BBoxEdge> allEdges()
{
  auto result = std::vector<BBoxEdge>{};
  result.reserve(12);

  vm::bbox3d{{-1, -1, -1}, {1, 1, 1}}.for_each_edge(
    [&](const auto& p0, const auto& p1) { result.emplace_back(p0, p1); });

  assert(result.size() == 12);
  return result;
}

std::vector<BBoxCorner> allCorners()
{
  auto result = std::vector<BBoxCorner>{};
  result.reserve(8);

  vm::bbox3d{{-1, -1, -1}, {1, 1, 1}}.for_each_vertex(
    [&](const auto& point) { result.emplace_back(point); });

  assert(result.size() == 8);
  return result;
}

vm::vec3d pointForBBoxCorner(const vm::bbox3d& box, const BBoxCorner& corner)
{
  vm::vec3d res;
  for (size_t i = 0; i < 3; ++i)
  {
    assert(corner.corner[i] == 1.0 || corner.corner[i] == -1.0);

    res[i] = (corner.corner[i] == 1.0) ? box.max[i] : box.min[i];
  }
  return res;
}

BBoxSide oppositeSide(const BBoxSide& side)
{
  return BBoxSide{side.normal * -1.0};
}

BBoxCorner oppositeCorner(const BBoxCorner& corner)
{
  return BBoxCorner{{-corner.corner.x(), -corner.corner.y(), -corner.corner.z()}};
}

BBoxEdge oppositeEdge(const BBoxEdge& edge)
{
  return BBoxEdge{
    oppositeCorner(BBoxCorner{edge.point0}).corner,
    oppositeCorner(BBoxCorner{edge.point1}).corner};
}

vm::segment3d pointsForBBoxEdge(const vm::bbox3d& box, const BBoxEdge& edge)
{
  return vm::segment3d{
    pointForBBoxCorner(box, BBoxCorner{edge.point0}),
    pointForBBoxCorner(box, BBoxCorner{edge.point1})};
}

vm::polygon3d polygonForBBoxSide(const vm::bbox3d& box, const BBoxSide& side)
{
  const auto wantedNormal = side.normal;

  auto result = vm::polygon3d{};
  box.for_each_face(
    [&](const auto& p0, const auto& p1, const auto& p2, const auto& p3, const auto& n) {
      if (n == wantedNormal)
      {
        result = vm::polygon3d{p0, p1, p2, p3};
      }
    });

  assert(result.vertexCount() == 4);
  return result;
}

vm::vec3d centerForBBoxSide(const vm::bbox3d& box, const BBoxSide& side)
{
  const auto wantedNormal = side.normal;

  auto result = std::optional<vm::vec3d>{};
  auto visitor =
    [&](const auto& p0, const auto& p1, const auto& p2, const auto& p3, const auto& n) {
      if (n == wantedNormal)
      {
        result = (p0 + p1 + p2 + p3) / 4.0;
      }
    };
  box.for_each_face(visitor);

  assert(result != std::nullopt);
  return *result;
}

// manipulating bboxes

vm::bbox3d moveBBoxSide(
  const vm::bbox3d& in,
  const BBoxSide& side,
  const vm::vec3d& delta,
  const ProportionalAxes& proportional,
  AnchorPos anchorType)
{
  auto sideLengthDelta = vm::dot(side.normal, delta);

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
    return vm::bbox3d{};
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

  return vm::bbox3d{matrix * in.min, matrix * in.max};
}

vm::bbox3d moveBBoxCorner(
  const vm::bbox3d& in,
  const BBoxCorner& corner,
  const vm::vec3d& delta,
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
      return vm::bbox3d{};
    }
    const bool oldPositive = oldCorner[i] > anchor[i];
    const bool newPositive = newCorner[i] > anchor[i];
    if (oldPositive != newPositive)
    {
      return vm::bbox3d{};
    }
  }

  if (anchorType == AnchorPos::Center)
  {
    const auto points = std::vector{anchor - (newCorner - anchor), newCorner};
    return vm::bbox3d::merge_all(std::begin(points), std::end(points));
  }
  else
  {
    const auto points = std::vector{oppositePoint, newCorner};
    return vm::bbox3d::merge_all(std::begin(points), std::end(points));
  }
}

vm::bbox3d moveBBoxEdge(
  const vm::bbox3d& in,
  const BBoxEdge& edge,
  const vm::vec3d& delta,
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
      return vm::bbox3d{};
    }
    if ((oldAnchorDist[i] < 0) && (newAnchorDist[i] > 0))
    {
      return vm::bbox3d{};
    }
  }

  const auto nonMovingAxis = vm::find_abs_max_component(oldAnchorDist, 2u);

  const auto corner1 =
    (anchorType == AnchorPos::Center) ? anchor - newAnchorDist : anchor;
  const auto corner2 = anchor + newAnchorDist;

  auto p1 = vm::min(corner1, corner2);
  auto p2 = vm::max(corner1, corner2);

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

  const auto result = vm::bbox3d{vm::min(p1, p2), vm::max(p1, p2)};

  // check for zero size
  return !result.is_empty() ? result : vm::bbox3d{};
}

vm::line3d handleLineForHit(const vm::bbox3d& bboxAtDragStart, const mdl::Hit& hit)
{
  auto handleLine = vm::line3d{};

  // NOTE: We don't need to check for the Alt modifier (moves the drag anchor to the
  // center of the bbox) because all of these lines go through the center of the box
  // anyway, so the resulting line would be the same.

  if (hit.type() == ScaleTool::ScaleToolSideHitType)
  {
    const auto draggingSide = hit.target<BBoxSide>();

    handleLine =
      vm::line3d{centerForBBoxSide(bboxAtDragStart, draggingSide), draggingSide.normal};
  }
  else if (hit.type() == ScaleTool::ScaleToolEdgeHitType)
  {
    const auto endEdge = hit.target<BBoxEdge>();
    const auto startEdge = oppositeEdge(endEdge);

    const vm::segment3d endEdgeActual = pointsForBBoxEdge(bboxAtDragStart, endEdge);
    const vm::segment3d startEdgeActual = pointsForBBoxEdge(bboxAtDragStart, startEdge);

    const vm::vec3d handleLineStart = startEdgeActual.center();
    const vm::vec3d handleLineEnd = endEdgeActual.center();

    handleLine = vm::line3d(handleLineStart, normalize(handleLineEnd - handleLineStart));
  }
  else if (hit.type() == ScaleTool::ScaleToolCornerHitType)
  {
    const auto endCorner = hit.target<BBoxCorner>();
    const auto startCorner = oppositeCorner(endCorner);

    const vm::vec3d handleLineStart = pointForBBoxCorner(bboxAtDragStart, startCorner);
    const vm::vec3d handleLineEnd = pointForBBoxCorner(bboxAtDragStart, endCorner);

    handleLine = vm::line3d(handleLineStart, normalize(handleLineEnd - handleLineStart));
  }
  else
  {
    assert(0);
  }

  return handleLine;
}

vm::bbox3d moveBBoxForHit(
  const vm::bbox3d& bboxAtDragStart,
  const mdl::Hit& dragStartHit,
  const vm::vec3d& delta,
  const ProportionalAxes& proportional,
  const AnchorPos anchor)
{
  if (dragStartHit.type() == ScaleTool::ScaleToolSideHitType)
  {
    const auto endSide = dragStartHit.target<BBoxSide>();

    return moveBBoxSide(bboxAtDragStart, endSide, delta, proportional, anchor);
  }
  else if (dragStartHit.type() == ScaleTool::ScaleToolEdgeHitType)
  {
    const auto endEdge = dragStartHit.target<BBoxEdge>();

    return moveBBoxEdge(bboxAtDragStart, endEdge, delta, proportional, anchor);
  }
  else if (dragStartHit.type() == ScaleTool::ScaleToolCornerHitType)
  {
    const auto endCorner = dragStartHit.target<BBoxCorner>();

    return moveBBoxCorner(bboxAtDragStart, endCorner, delta, anchor);
  }
  else
  {
    assert(0);
    return vm::bbox3d{};
  }
}

// ScaleTool

ScaleTool::ScaleTool(mdl::Map& map)
  : Tool{false}
  , m_map{map}
{
}

ScaleTool::~ScaleTool() = default;

bool ScaleTool::doActivate()
{
  m_toolPage->activate();
  return true;
}

const mdl::Grid& ScaleTool::grid() const
{
  return m_map.grid();
}

const mdl::Hit& ScaleTool::dragStartHit() const
{
  return m_dragStartHit;
}

bool ScaleTool::applies() const
{
  return m_map.selection().hasNodes();
}

BackSide pickBackSideOfBox(
  const vm::ray3d& pickRay, const render::Camera& /* camera */, const vm::bbox3d& box)
{
  auto closestDistToRay = std::numeric_limits<double>::max();
  auto bestDistAlongRay = std::numeric_limits<double>::max();
  vm::vec3d bestNormal;

  // idea is: find the closest point on an edge of the cube, belonging
  // to a face that's facing away from the pick ray.
  auto visitor =
    [&](const auto& p0, const auto& p1, const auto& p2, const auto& p3, const auto& n) {
      const auto cosAngle = vm::dot(n, pickRay.direction);
      if (cosAngle >= 0.0 && cosAngle < 1.0)
      {
        // the face is pointing away from the camera (or exactly perpendicular)
        // but not equal to the camera direction (important for 2D views)

        const vm::vec3d points[] = {p0, p1, p2, p3};
        for (size_t i = 0; i < 4; i++)
        {
          const auto result =
            vm::distance(pickRay, vm::segment3d(points[i], points[(i + 1) % 4]));
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
  assert(bestNormal != vm::vec3d(0, 0, 0));

  return {
    bestDistAlongRay,
    bestNormal,
  };
}

void ScaleTool::pickBackSides(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  // select back sides. Used for both 2D and 3D.
  if (pickResult.empty())
  {
    const auto result = pickBackSideOfBox(pickRay, camera, bounds());

    // The hit point is the closest point on the pick ray to one of the edges of the face.
    // For face dragging, we'll project the pick ray onto the line through this point and
    // having the face normal.
    assert(result.pickedSideNormal != vm::vec3d(0, 0, 0));
    pickResult.addHit(mdl::Hit{
      ScaleToolSideHitType,
      result.distAlongRay,
      vm::point_at_distance(pickRay, result.distAlongRay),
      BBoxSide{result.pickedSideNormal}});
  }
}

void ScaleTool::pick2D(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  using namespace mdl::HitFilters;

  const auto& myBounds = bounds();

  // origin in bbox
  if (myBounds.contains(pickRay.origin))
  {
    return;
  }

  auto localPickResult = mdl::PickResult{};

  // bbox corners in 2d views
  assert(camera.orthographicProjection());
  for (const auto& edge : allEdges())
  {
    const auto points = pointsForBBoxEdge(myBounds, edge);

    // in 2d views, only use edges that are parallel to the camera
    if (vm::is_parallel(points.direction(), vm::vec3d{camera.direction()}))
    {
      // could figure out which endpoint is closer to camera, or just test both.
      for (const auto& point : {points.start(), points.end()})
      {
        if (
          const auto dist = camera.pickPointHandle(
            pickRay, point, double(pref(Preferences::HandleRadius))))
        {
          const auto hitPoint = vm::point_at_distance(pickRay, *dist);
          localPickResult.addHit(mdl::Hit{ScaleToolEdgeHitType, *dist, hitPoint, edge});
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

void ScaleTool::pick3D(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  using namespace mdl::HitFilters;

  const auto& myBounds = bounds();

  // origin in bbox
  if (myBounds.contains(pickRay.origin))
  {
    return;
  }

  auto localPickResult = mdl::PickResult{};

  // these handles only work in 3D.
  assert(camera.perspectiveProjection());

  // corners
  for (const auto& corner : allCorners())
  {
    const auto point = pointForBBoxCorner(myBounds, corner);

    // make the spheres for the corner handles slightly larger than the
    // cylinders of the edge handles, so they take priority where they overlap.
    const auto cornerRadius = double(pref(Preferences::HandleRadius)) * 2.0;
    if (const auto dist = camera.pickPointHandle(pickRay, point, cornerRadius))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *dist);
      localPickResult.addHit(mdl::Hit{ScaleToolCornerHitType, *dist, hitPoint, corner});
    }
  }

  // edges
  for (const auto& edge : allEdges())
  {
    const auto points = pointsForBBoxEdge(myBounds, edge);

    if (
      const auto dist = camera.pickLineSegmentHandle(
        pickRay, points, double(pref(Preferences::HandleRadius))))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *dist);
      localPickResult.addHit(mdl::Hit{ScaleToolEdgeHitType, *dist, hitPoint, edge});
    }
  }

  // sides
  for (const auto& side : allSides())
  {
    const auto poly = polygonForBBoxSide(myBounds, side);

    if (
      const auto dist = vm::intersect_ray_polygon(
        pickRay, poly.vertices().begin(), poly.vertices().end()))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *dist);
      localPickResult.addHit(mdl::Hit{ScaleToolSideHitType, *dist, hitPoint, side});
    }
  }

  pickBackSides(pickRay, camera, localPickResult);

  if (!localPickResult.empty())
  {
    pickResult.addHit(localPickResult.all().front());
  }
}

vm::bbox3d ScaleTool::bounds() const
{
  const auto& bounds = m_map.selectionBounds();
  contract_assert(bounds != std::nullopt);

  return *bounds;
}

std::vector<vm::polygon3f> ScaleTool::polygonsHighlightedByDrag() const
{
  auto sides = std::vector<BBoxSide>{};

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
        auto side1 = vm::vec3d{};
        side1[i] = 1.0;
        sides.emplace_back(side1);

        auto side2 = vm::vec3d{};
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

bool ScaleTool::hasDragSide() const
{
  return dragSide().vertexCount() > 0;
}

vm::polygon3f ScaleTool::dragSide() const
{
  if (m_dragStartHit.type() == ScaleToolSideHitType)
  {
    const auto side = m_dragStartHit.target<BBoxSide>();
    return vm::polygon3f{polygonForBBoxSide(bounds(), side)};
  }

  return {};
}

bool ScaleTool::hasDragEdge() const
{
  return m_dragStartHit.type() == ScaleToolEdgeHitType;
}

vm::segment3f ScaleTool::dragEdge() const
{
  assert(hasDragEdge());
  auto whichEdge = m_dragStartHit.target<BBoxEdge>();
  return vm::segment3f{pointsForBBoxEdge(bounds(), whichEdge)};
}

bool ScaleTool::hasDragCorner() const
{
  return m_dragStartHit.type() == ScaleToolCornerHitType;
}

vm::vec3f ScaleTool::dragCorner() const
{
  assert(hasDragCorner());
  auto whichCorner = m_dragStartHit.target<BBoxCorner>();
  return vm::vec3f{pointForBBoxCorner(bounds(), whichCorner)};
}

bool ScaleTool::hasDragAnchor() const
{
  if (bounds().is_empty())
  {
    return false;
  }

  const auto type = m_dragStartHit.type();
  return type == ScaleToolEdgeHitType || type == ScaleToolCornerHitType
         || type == ScaleToolSideHitType;
}

vm::vec3f ScaleTool::dragAnchor() const
{
  if (m_anchorPos == AnchorPos::Center)
  {
    return vm::vec3f{bounds().center()};
  }

  if (m_dragStartHit.type() == ScaleToolSideHitType)
  {
    const auto endSide = m_dragStartHit.target<BBoxSide>();
    const auto startSide = oppositeSide(endSide);

    return vm::vec3f{centerForBBoxSide(bounds(), startSide)};
  }
  else if (m_dragStartHit.type() == ScaleToolEdgeHitType)
  {
    const auto endEdge = m_dragStartHit.target<BBoxEdge>();
    const auto startEdge = oppositeEdge(endEdge);

    const auto startEdgeActual = pointsForBBoxEdge(bounds(), startEdge);

    return vm::vec3f{startEdgeActual.center()};
  }
  else if (m_dragStartHit.type() == ScaleToolCornerHitType)
  {
    const auto endCorner = m_dragStartHit.target<BBoxCorner>();
    const auto startCorner = oppositeCorner(endCorner);

    const auto startCornerActual = pointForBBoxCorner(bounds(), startCorner);
    return vm::vec3f{startCornerActual};
  }

  assert(0);
  return vm::vec3f{};
}

vm::bbox3d ScaleTool::bboxAtDragStart() const
{
  contract_pre(m_resizing);

  return m_bboxAtDragStart;
}

std::vector<vm::vec3d> ScaleTool::cornerHandles() const
{
  auto result = std::vector<vm::vec3d>{};
  if (!bounds().is_empty())
  {
    result.reserve(8);
    auto op = [&](const auto& point) { result.push_back(point); };
    bounds().for_each_vertex(op);
  }
  return result;
}

void ScaleTool::updatePickedHandle(const mdl::PickResult& pickResult)
{
  using namespace mdl::HitFilters;

  const auto& hit = pickResult.first(
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

void ScaleTool::setAnchorPos(const AnchorPos pos)
{
  m_anchorPos = pos;
}

AnchorPos ScaleTool::anchorPos() const
{
  return m_anchorPos;
}

void ScaleTool::setProportionalAxes(const ProportionalAxes& proportionalAxes)
{
  m_proportionalAxes = proportionalAxes;
}

const ProportionalAxes& ScaleTool::proportionalAxes() const
{
  return m_proportionalAxes;
}

void ScaleTool::startScaleWithHit(const mdl::Hit& hit)
{
  contract_pre(hit.isMatch());
  contract_pre(
    hit.type() == ScaleToolCornerHitType || hit.type() == ScaleToolEdgeHitType
    || hit.type() == ScaleToolSideHitType);
  contract_pre(!m_resizing);

  m_bboxAtDragStart = bounds();
  m_dragStartHit = hit;
  m_dragCumulativeDelta = vm::vec3d{0, 0, 0};

  m_map.startTransaction("Scale Objects", mdl::TransactionScope::LongRunning);
  m_resizing = true;
}

void ScaleTool::scaleByDelta(const vm::vec3d& delta)
{
  contract_pre(m_resizing);

  m_dragCumulativeDelta = m_dragCumulativeDelta + delta;

  const auto newBox = moveBBoxForHit(
    m_bboxAtDragStart,
    m_dragStartHit,
    m_dragCumulativeDelta,
    m_proportionalAxes,
    m_anchorPos);

  if (!newBox.is_empty())
  {
    scaleSelection(m_map, bounds(), newBox);
  }
}

void ScaleTool::commitScale()
{
  if (vm::is_zero(m_dragCumulativeDelta, vm::Cd::almost_zero()))
  {
    m_map.cancelTransaction();
  }
  else
  {
    m_map.commitTransaction();
  }
  m_resizing = false;
}

void ScaleTool::cancelScale()
{
  m_map.cancelTransaction();
  m_resizing = false;
}

QWidget* ScaleTool::doCreatePage(QWidget* parent)
{
  assert(m_toolPage == nullptr);
  m_toolPage = new ScaleToolPage{m_map, parent};
  return m_toolPage;
}

} // namespace tb::ui
