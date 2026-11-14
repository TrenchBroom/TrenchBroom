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

#include "ui/ShearTool.h"

#include "gl/Camera.h"
#include "mdl/Grid.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/PickResult.h"
#include "mdl/TransactionScope.h"
#include "ui/MapDocument.h"
#include "ui/ScaleTool.h"

#include "kd/contracts.h"

#include "vm/intersection.h"

namespace tb::ui
{

const mdl::HitType::Type ShearTool::ShearToolSideHitType = mdl::HitType::freeType();

ShearTool::ShearTool(MapDocument& document)
  : Tool{false}
  , m_document{document}
{
}

ShearTool::~ShearTool() = default;

const mdl::Grid& ShearTool::grid() const
{
  return m_document.map().grid();
}

bool ShearTool::applies() const
{
  return m_document.map().selection().hasNodes();
}

void ShearTool::pickBackSides(
  const vm::ray3d& pickRay, const gl::Camera& camera, mdl::PickResult& pickResult) const
{
  // select back sides. Used for both 2D and 3D.
  if (pickResult.empty())
  {
    const auto result = pickBackSideOfBox(pickRay, camera, bounds());

    // The hit point is the closest point on the pick ray to one of the edges of the face.
    // For face dragging, we'll project the pick ray onto the line through this point and
    // having the face normal.
    contract_assert(result.pickedSideNormal != vm::vec3d(0, 0, 0));

    pickResult.addHit(mdl::Hit{
      ShearToolSideHitType,
      result.distAlongRay,
      vm::point_at_distance(pickRay, result.distAlongRay),
      BBoxSide{result.pickedSideNormal}});
  }
}

void ShearTool::pick2D(
  const vm::ray3d& pickRay, const gl::Camera& camera, mdl::PickResult& pickResult) const
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
  const vm::ray3d& pickRay, const gl::Camera& camera, mdl::PickResult& pickResult) const
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
  contract_assert(camera.perspectiveProjection());

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
  const auto& bounds = m_document.map().selectionBounds();
  contract_assert(bounds != std::nullopt);

  return *bounds;
}

bool ShearTool::dragging() const
{
  return m_dragging;
}

void ShearTool::beginShear()
{
  contract_pre(!m_dragging);

  m_document.map().startTransaction("Shear Objects", mdl::TransactionScope::LongRunning);
  m_dragging = true;
}

void ShearTool::commitShear(const ShearDragState& dragState)
{
  contract_pre(m_dragging);

  auto& map = m_document.map();
  if (vm::is_zero(dragState.totalDelta, vm::Cd::almost_zero()))
  {
    map.cancelTransaction();
  }
  else
  {
    map.commitTransaction();
  }
  m_dragging = false;
}

void ShearTool::cancelShear()
{
  contract_pre(m_dragging);

  m_document.map().cancelTransaction();
  m_dragging = false;
}

bool ShearTool::shearByDelta(const vm::vec3d& delta, ShearDragState& dragState)
{
  contract_pre(m_dragging);

  auto& map = m_document.map();
  const auto& bboxAtDragStart = dragState.bboxAtDragStart;
  const auto& normal = dragState.side.normal;
  const auto totalDelta = dragState.totalDelta + delta;

  map.rollbackTransaction();
  if (!shearSelection(map, bboxAtDragStart, normal, totalDelta))
  {
    shearSelection(map, bboxAtDragStart, normal, dragState.totalDelta);
    return false;
  }

  dragState.totalDelta = totalDelta;
  return true;
}

std::optional<vm::polygon3f> ShearTool::shearHandle() const
{
  if (m_pickedSide.type() != ShearToolSideHitType)
  {
    return std::nullopt;
  }

  const auto side = m_pickedSide.target<BBoxSide>();
  return vm::polygon3f{polygonForBBoxSide(bounds(), side)};
}

void ShearTool::updatePickedSide(const mdl::PickResult& pickResult)
{
  using namespace mdl::HitFilters;

  const auto& hit = pickResult.first(type(ShearToolSideHitType));

  // extract the highlighted handle from the hit here, and only refresh views if it
  // changed
  if (
    hit.type() == ShearToolSideHitType && m_pickedSide.type() == ShearToolSideHitType
    && hit.target<BBoxSide>() == m_pickedSide.target<BBoxSide>())
  {
    return;
  }

  m_pickedSide = hit;

  refreshViews();
}

} // namespace tb::ui
