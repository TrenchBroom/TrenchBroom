/*
 Copyright (C) 2021 Kristian Duske

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

#include "View/HandleDragTracker.h"

#include "Macros.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/Hit.h"
#include "Model/HitAdapter.h"
#include "View/Grid.h"

#include "kdl/reflection_impl.h"

#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/line.h"
#include "vm/plane.h"
#include "vm/quat.h"
#include "vm/vec_io.h"

#include <ostream>

namespace TrenchBroom
{
namespace View
{

kdl_reflect_impl(DragState);

std::optional<UpdateDragConfig> HandleDragTrackerDelegate::modifierKeyChange(
  const InputState&, const DragState&)
{
  return std::nullopt;
}

void HandleDragTrackerDelegate::mouseScroll(const InputState&, const DragState&) {}

void HandleDragTrackerDelegate::setRenderOptions(
  const InputState&, Renderer::RenderContext&) const
{
}

void HandleDragTrackerDelegate::render(
  const InputState&,
  const DragState&,
  Renderer::RenderContext&,
  Renderer::RenderBatch&) const
{
}

DragHandlePicker makeLineHandlePicker(const vm::line3& line, const vm::vec3& handleOffset)
{
  return [line = vm::line3{line.point - handleOffset, line.direction},
          handleOffset](const InputState& inputState) -> std::optional<vm::vec3> {
    const auto dist = vm::distance(inputState.pickRay(), line);
    if (dist.parallel)
    {
      return std::nullopt;
    }
    return line.point + line.direction * dist.position2 + handleOffset;
  };
}

DragHandlePicker makePlaneHandlePicker(
  const vm::plane3& plane, const vm::vec3& handleOffset)
{
  return [plane = vm::plane3{plane.anchor() - handleOffset, plane.normal},
          handleOffset](const InputState& inputState) -> std::optional<vm::vec3> {
    const auto distance = vm::intersect_ray_plane(inputState.pickRay(), plane);
    if (vm::is_nan(distance))
    {
      return std::nullopt;
    }
    return vm::point_at_distance(inputState.pickRay(), distance) + handleOffset;
  };
}

DragHandlePicker makeCircleHandlePicker(
  const vm::vec3& center,
  const vm::vec3& normal,
  const FloatType radius,
  const vm::vec3& handleOffset)
{
  return [center = center - handleOffset, normal, radius, handleOffset](
           const InputState& inputState) -> std::optional<vm::vec3> {
    const auto plane = vm::plane3{center, normal};
    const auto distance = vm::intersect_ray_plane(inputState.pickRay(), plane);
    if (vm::is_nan(distance))
    {
      return std::nullopt;
    }

    const auto hitPoint = vm::point_at_distance(inputState.pickRay(), distance);
    const auto direction = vm::normalize(hitPoint - center);
    return center + radius * direction + handleOffset;
  };
}

DragHandlePicker makeSurfaceHandlePicker(
  Model::HitFilter filter, const vm::vec3& handleOffset)
{
  return [filter = std::move(filter),
          handleOffset](const InputState& inputState) -> std::optional<vm::vec3> {
    const auto& hit = inputState.pickResult().first(filter);
    if (!hit.isMatch())
    {
      return std::nullopt;
    }
    return hit.hitPoint() + handleOffset;
  };
}

DragHandleSnapper makeIdentityHandleSnapper()
{
  return [](const InputState&, const DragState&, const vm::vec3& proposedHandlePosition) {
    return proposedHandlePosition;
  };
}

DragHandleSnapper makeRelativeHandleSnapper(const Grid& grid)
{
  return [&grid](
           const InputState&,
           const DragState& dragState,
           const vm::vec3& proposedHandlePosition) {
    return dragState.initialHandlePosition
           + grid.snap(proposedHandlePosition - dragState.initialHandlePosition);
  };
}

DragHandleSnapper makeAbsoluteHandleSnapper(const Grid& grid)
{
  return
    [&grid](const InputState&, const DragState&, const vm::vec3& proposedHandlePosition) {
      return grid.snap(proposedHandlePosition);
    };
}

DragHandleSnapper makeRelativeLineHandleSnapper(const Grid& grid, const vm::line3& line)
{
  return [&grid, line](
           const InputState&,
           const DragState& dragState,
           const vm::vec3& proposedHandlePosition) {
    const auto initialDistanceOnLine =
      vm::dot(dragState.initialHandlePosition - line.point, line.direction);
    const auto proposedDistanceOnLine =
      vm::dot(proposedHandlePosition - line.point, line.direction);
    const auto delta = grid.snap(proposedDistanceOnLine - initialDistanceOnLine);
    return vm::point_at_distance(line, initialDistanceOnLine + delta);
  };
}

DragHandleSnapper makeAbsoluteLineHandleSnapper(const Grid& grid, const vm::line3& line)
{
  return [&grid, line](
           const InputState&, const DragState&, const vm::vec3& proposedHandlePosition) {
    return grid.snap(proposedHandlePosition, line);
  };
}

DragHandleSnapper makeCircleHandleSnapper(
  const Grid& grid,
  FloatType snapAngle,
  const vm::vec3& center,
  const vm::vec3& normal,
  const FloatType radius)
{
  return [&grid, snapAngle, center, normal, radius](
           const InputState&,
           const DragState& dragState,
           const vm::vec3& proposedHandlePosition) -> std::optional<vm::vec3> {
    if (proposedHandlePosition == center)
    {
      return std::nullopt;
    }

    const auto ref = vm::normalize(dragState.initialHandlePosition - center);
    const auto vec = vm::normalize(proposedHandlePosition - center);
    const auto angle = vm::measure_angle(vec, ref, normal);
    const auto snapped = grid.snapAngle(angle, vm::abs(snapAngle));
    const auto canonical = snapped - vm::snapDown(snapped, vm::C::two_pi());
    const auto rotation = vm::quat3{normal, canonical};
    return center + radius * (rotation * ref);
  };
}

HandlePositionProposer makeBrushFaceHandleProposer(const Grid& grid)
{
  return [&grid](
           const InputState& inputState, const DragState&) -> std::optional<vm::vec3> {
    using namespace Model::HitFilters;

    const auto& hit = inputState.pickResult().first(type(Model::BrushNode::BrushHitType));
    if (!hit.isMatch())
    {
      return std::nullopt;
    }

    const auto faceHandle = Model::hitToFaceHandle(hit);
    ensure(faceHandle, "invalid hit type");

    return grid.snap(hit.hitPoint(), faceHandle->face().boundary());
  };
}

HandlePositionProposer makeHandlePositionProposer(
  DragHandlePicker pickHandlePosition, DragHandleSnapper snapHandlePosition)
{
  return [pickHandlePosition = std::move(pickHandlePosition),
          snapHandlePosition = std::move(snapHandlePosition)](
           const InputState& inputState,
           const DragState& dragState) -> std::optional<vm::vec3> {
    if (const auto handlePosition = pickHandlePosition(inputState))
    {
      return snapHandlePosition(inputState, dragState, *handlePosition);
    }
    return std::nullopt;
  };
}
} // namespace View
} // namespace TrenchBroom
