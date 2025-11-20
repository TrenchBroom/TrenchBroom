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

#include "ShearTool.h"

#include "mdl/Grid.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/PickResult.h"
#include "mdl/TransactionScope.h"
#include "render/Camera.h"
#include "ui/ScaleTool.h"

#include "kd/contracts.h"

#include "vm/intersection.h"

namespace tb::ui
{

const mdl::HitType::Type ShearTool::ShearToolSideHitType = mdl::HitType::freeType();

ShearTool::ShearTool(mdl::Map& map)
  : Tool{false}
  , m_map{map}
{
}

ShearTool::~ShearTool() = default;

const mdl::Grid& ShearTool::grid() const
{
  return m_map.grid();
}

bool ShearTool::applies() const
{
  return m_map.selection().hasNodes();
}

void ShearTool::pickBackSides(
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
      ShearToolSideHitType,
      result.distAlongRay,
      vm::point_at_distance(pickRay, result.distAlongRay),
      BBoxSide{result.pickedSideNormal}});
  }
}

void ShearTool::pick2D(
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
  pickBackSides(pickRay, camera, localPickResult);

  if (!localPickResult.empty())
  {
    pickResult.addHit(localPickResult.all().front());
  }
}

void ShearTool::pick3D(
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

  // sides
  for (const auto& side : allSides())
  {
    const auto poly = polygonForBBoxSide(myBounds, side);

    if (
      const auto dist = vm::intersect_ray_polygon(
        pickRay, poly.vertices().begin(), poly.vertices().end()))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *dist);
      localPickResult.addHit(mdl::Hit{ShearToolSideHitType, *dist, hitPoint, side});
    }
  }

  pickBackSides(pickRay, camera, localPickResult);

  if (!localPickResult.empty())
  {
    pickResult.addHit(localPickResult.all().front());
  }
}

vm::bbox3d ShearTool::bounds() const
{
  const auto& bounds = m_map.selectionBounds();
  contract_assert(bounds != std::nullopt);

  return *bounds;
}

// for rendering sheared bbox
vm::bbox3d ShearTool::bboxAtDragStart() const
{
  return m_resizing ? m_bboxAtDragStart : bounds();
}

void ShearTool::startShearWithHit(const mdl::Hit& hit)
{
  contract_pre(hit.isMatch());
  contract_pre(hit.type() == ShearToolSideHitType);
  contract_pre(!m_resizing);

  m_bboxAtDragStart = bounds();
  m_dragStartHit = hit;
  m_dragCumulativeDelta = vm::vec3d{0, 0, 0};

  m_map.startTransaction("Shear Objects", mdl::TransactionScope::LongRunning);
  m_resizing = true;
}

void ShearTool::commitShear()
{
  contract_pre(m_resizing);

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

void ShearTool::cancelShear()
{
  contract_pre(m_resizing);

  m_map.cancelTransaction();
  m_resizing = false;
}

void ShearTool::shearByDelta(const vm::vec3d& delta)
{
  contract_pre(m_resizing);

  m_dragCumulativeDelta = m_dragCumulativeDelta + delta;

  if (!vm::is_zero(delta, vm::Cd::almost_zero()))
  {
    const auto side = m_dragStartHit.target<BBoxSide>();
    shearSelection(m_map, bounds(), side.normal, delta);
  }
}

const mdl::Hit& ShearTool::dragStartHit() const
{
  return m_dragStartHit;
}

vm::mat4x4d ShearTool::bboxShearMatrix() const
{
  // happens if you cmd+drag on an edge or corner
  if (!m_resizing || m_dragStartHit.type() != ShearToolSideHitType)
  {
    return vm::mat4x4d::identity();
  }

  const auto side = m_dragStartHit.target<BBoxSide>();
  return vm::shear_bbox_matrix(m_bboxAtDragStart, side.normal, m_dragCumulativeDelta);
}

std::optional<vm::polygon3f> ShearTool::shearHandle() const
{
  // happens if you cmd+drag on an edge or corner
  if (m_dragStartHit.type() != ShearToolSideHitType)
  {
    return std::nullopt;
  }

  const auto side = m_dragStartHit.target<BBoxSide>();
  // use the bboxAtDragStart() function so we get bounds() if we're not currently inside a
  // drag.
  const auto polyAtDragStart = polygonForBBoxSide(bboxAtDragStart(), side);

  const auto handle = polyAtDragStart.transform(bboxShearMatrix());
  return vm::polygon3f{handle};
}

void ShearTool::updatePickedSide(const mdl::PickResult& pickResult)
{
  using namespace mdl::HitFilters;

  const auto& hit = pickResult.first(type(ShearToolSideHitType));

  // extract the highlighted handle from the hit here, and only refresh views if it
  // changed
  if (
    hit.type() == ShearToolSideHitType && m_dragStartHit.type() == ShearToolSideHitType
    && hit.target<BBoxSide>() == m_dragStartHit.target<BBoxSide>())
  {
    return;
  }

  // hack for highlighting on mouseover
  m_dragStartHit = hit;

  refreshViews();
}

bool ShearTool::constrainVertical() const
{
  return m_constrainVertical;
}

void ShearTool::setConstrainVertical(const bool constrainVertical)
{
  m_constrainVertical = constrainVertical;
}

} // namespace tb::ui
