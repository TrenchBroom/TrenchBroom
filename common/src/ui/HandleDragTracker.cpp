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

#include "ui/HandleDragTracker.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Grid.h"
#include "mdl/Hit.h"
#include "mdl/HitAdapter.h"

#include "kd/optional_utils.h"
#include "kd/reflection_impl.h"

#include "vm/distance.h"
#include "vm/intersection.h"
#include "vm/line.h"
#include "vm/plane.h"
#include "vm/quat.h"
#include "vm/vec_io.h" // IWYU pragma: keep

namespace tb::ui
{

kdl_reflect_impl(DragState);

std::optional<UpdateDragConfig> HandleDragTrackerDelegate::modifierKeyChange(
  const InputState&, const DragState&)
{
  return std::nullopt;
}

void HandleDragTrackerDelegate::mouseScroll(const InputState&, const DragState&) {}

void HandleDragTrackerDelegate::setRenderOptions(
  const InputState&, render::RenderContext&) const
{
}

void HandleDragTrackerDelegate::render(
  const InputState&, const DragState&, render::RenderContext&, render::RenderBatch&) const
{
}

DragHandlePicker makeLineHandlePicker(
  const vm::line3d& line_, const vm::vec3d& handleOffset)
{
  return [line = vm::line3d{line_.point - handleOffset, line_.direction},
          handleOffset](const InputState& inputState) -> std::optional<vm::vec3d> {
    const auto dist = vm::distance(inputState.pickRay(), line);
    if (dist.parallel)
    {
      return std::nullopt;
    }
    return line.point + line.direction * dist.position2 + handleOffset;
  };
}

DragHandlePicker makePlaneHandlePicker(
  const vm::plane3d& plane_, const vm::vec3d& handleOffset)
{
  return [plane = vm::plane3d{plane_.anchor() - handleOffset, plane_.normal},
          handleOffset](const InputState& inputState) -> std::optional<vm::vec3d> {
    return vm::intersect_ray_plane(inputState.pickRay(), plane)
           | kdl::optional_transform([&](const auto distance) {
               return vm::point_at_distance(inputState.pickRay(), distance)
                      + handleOffset;
             });
  };
}

DragHandlePicker makeCircleHandlePicker(
  const vm::vec3d& center_,
  const vm::vec3d& normal,
  const double radius,
  const vm::vec3d& handleOffset)
{
  return [center = center_ - handleOffset, normal, radius, handleOffset](
           const InputState& inputState) -> std::optional<vm::vec3d> {
    const auto plane = vm::plane3d{center, normal};
    return vm::intersect_ray_plane(inputState.pickRay(), plane)
           | kdl::optional_transform([&](const auto distance) {
               const auto hitPoint =
                 vm::point_at_distance(inputState.pickRay(), distance);
               const auto direction = vm::normalize(hitPoint - center);
               return center + radius * direction + handleOffset;
             });
  };
}

DragHandlePicker makeSurfaceHandlePicker(
  mdl::HitFilter filter_, const vm::vec3d& handleOffset)
{
  return [filter = std::move(filter_),
          handleOffset](const InputState& inputState) -> std::optional<vm::vec3d> {
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
  return
    [](const InputState&, const DragState&, const vm::vec3d& proposedHandlePosition) {
      return proposedHandlePosition;
    };
}

DragHandleSnapper makeRelativeHandleSnapper(const mdl::Grid& grid)
{
  return [&grid](
           const InputState&,
           const DragState& dragState,
           const vm::vec3d& proposedHandlePosition) {
    return dragState.initialHandlePosition
           + grid.snap(proposedHandlePosition - dragState.initialHandlePosition);
  };
}

DragHandleSnapper makeAbsoluteHandleSnapper(const mdl::Grid& grid)
{
  return [&grid](
           const InputState&, const DragState&, const vm::vec3d& proposedHandlePosition) {
    return grid.snap(proposedHandlePosition);
  };
}

DragHandleSnapper makeRelativeLineHandleSnapper(
  const mdl::Grid& grid, const vm::line3d& line)
{
  return [&grid, line](
           const InputState&,
           const DragState& dragState,
           const vm::vec3d& proposedHandlePosition) {
    const auto initialDistanceOnLine =
      vm::dot(dragState.initialHandlePosition - line.point, line.direction);
    const auto proposedDistanceOnLine =
      vm::dot(proposedHandlePosition - line.point, line.direction);
    const auto delta = grid.snap(proposedDistanceOnLine - initialDistanceOnLine);
    return vm::point_at_distance(line, initialDistanceOnLine + delta);
  };
}

DragHandleSnapper makeAbsoluteLineHandleSnapper(
  const mdl::Grid& grid, const vm::line3d& line)
{
  return [&grid, line](
           const InputState&, const DragState&, const vm::vec3d& proposedHandlePosition) {
    return grid.snap(proposedHandlePosition, line);
  };
}

DragHandleSnapper makeCircleHandleSnapper(
  const mdl::Grid& grid,
  double snapAngle,
  const vm::vec3d& center,
  const vm::vec3d& normal,
  const double radius)
{
  return [&grid, snapAngle, center, normal, radius](
           const InputState&,
           const DragState& dragState,
           const vm::vec3d& proposedHandlePosition) -> std::optional<vm::vec3d> {
    if (proposedHandlePosition == center)
    {
      return std::nullopt;
    }

    const auto ref = vm::normalize(dragState.initialHandlePosition - center);
    const auto vec = vm::normalize(proposedHandlePosition - center);
    const auto angle = vm::measure_angle(vec, ref, normal);
    const auto snapped = grid.snapAngle(angle, vm::abs(snapAngle));
    const auto canonical = snapped - vm::snapDown(snapped, vm::Cd::two_pi());
    const auto rotation = vm::quatd{normal, canonical};
    return center + radius * (rotation * ref);
  };
}

HandlePositionProposer makeBrushFaceHandleProposer(const mdl::Grid& grid)
{
  return
    [&grid](const InputState& inputState, const DragState&) -> std::optional<vm::vec3d> {
      using namespace mdl::HitFilters;

      const auto& hit = inputState.pickResult().first(type(mdl::BrushNode::BrushHitType));
      if (!hit.isMatch())
      {
        return std::nullopt;
      }

      const auto faceHandle = mdl::hitToFaceHandle(hit);
      ensure(faceHandle, "invalid hit type");

      return grid.snap(hit.hitPoint(), faceHandle->face().boundary());
    };
}

HandlePositionProposer makeHandlePositionProposer(
  DragHandlePicker pickHandlePosition_, DragHandleSnapper snapHandlePosition_)
{
  return [pickHandlePosition = std::move(pickHandlePosition_),
          snapHandlePosition = std::move(snapHandlePosition_)](
           const InputState& inputState,
           const DragState& dragState) -> std::optional<vm::vec3d> {
    if (const auto handlePosition = pickHandlePosition(inputState))
    {
      return snapHandlePosition(inputState, dragState, *handlePosition);
    }
    return std::nullopt;
  };
}

} // namespace tb::ui
