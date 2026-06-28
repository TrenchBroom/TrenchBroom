/*
 Copyright (C) 2026 Kristian Duske

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

#include "mdl/Hit.h"

#include "kd/ranges/to.h"
#include "kd/reflection_decl.h"

#include "vm/ray.h"
#include <vm/polygon.h>
#include <vm/segment.h>
#include <vm/vec.h>

#include <optional>
#include <tuple>

#pragma once

namespace tb
{
namespace gl
{
class Camera;
}

namespace mdl
{
class Grid;
class Node;

struct VertexHandle
{
  using Position = vm::vec3d;

  static const HitType::Type HandleHitType;

  Position position;

  static std::vector<VertexHandle> getHandles(const Node& node);

  template <std::ranges::range R>
  static std::vector<Position> getPositions(R&& range)
  {
    return range | std::views::transform(&VertexHandle::position)
           | kdl::ranges::to<std::vector>();
  }

  template <std::ranges::range R>
  static std::vector<vm::vec3d> getVertices(R&& range)
  {
    return range | std::views::transform(&VertexHandle::position)
           | kdl::ranges::to<std::vector>();
  }

  static double distance(const VertexHandle& lhs, const VertexHandle& rhs);

  /**
   * Pick this handle.
   */
  std::optional<Hit> pick(
    HitType::Type hitType,
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius) const;

  kdl_reflect_decl(VertexHandle, position);
};

struct EdgeHandle
{
  using Position = vm::segment3d;
  using GridHandleHitData = std::tuple<EdgeHandle, vm::vec3d>;

  static const HitType::Type HandleHitType;

  Position position;

  static std::vector<EdgeHandle> getHandles(const Node& node);

  template <std::ranges::range R>
  static std::vector<Position> getPositions(R&& range)
  {
    return range | std::views::transform(&EdgeHandle::position)
           | kdl::ranges::to<std::vector>();
  }

  template <std::ranges::range R>
  static std::vector<vm::vec3d> getVertices(R&& range)
  {
    auto result = std::vector<vm::vec3d>{};
    if constexpr (std::ranges::sized_range<R>)
    {
      result.reserve(2 * std::ranges::size(range));
    }

    for (const auto& edgeHandle : range)
    {
      result.push_back(edgeHandle.position.start());
      result.push_back(edgeHandle.position.end());
    }

    return result;
  }

  static double distance(const EdgeHandle& lhs, const EdgeHandle& rhs);

  /**
   * Pick the center point of this handle.
   */
  std::optional<Hit> pick(
    HitType::Type hitType,
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius) const;

  /**
   * Pick a point on this handle where it intersects a grid plane.
   * This is used for splitting edges in the vertex tool.
   */
  std::optional<Hit> pick(
    HitType::Type hitType,
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius,
    const Grid& grid) const;

  kdl_reflect_decl(EdgeHandle, position);
};

struct FaceHandle
{
  using Position = vm::polygon3d;
  using GridHandleHitData = std::tuple<FaceHandle, vm::vec3d>;

  static const HitType::Type HandleHitType;

  Position position;

  static std::vector<FaceHandle> getHandles(const Node& node);

  template <std::ranges::range R>
  static std::vector<Position> getPositions(R&& range)
  {
    return range | std::views::transform(&FaceHandle::position)
           | kdl::ranges::to<std::vector>();
  }

  template <std::ranges::range R>
  static std::vector<vm::vec3d> getVertices(R&& range)
  {
    return range
           | std::views::transform(
             [](const auto& faceHandle) -> const std::vector<vm::vec3d>& {
               return faceHandle.position.vertices();
             })
           | std::views::join | kdl::ranges::to<std::vector>();
  }

  static double distance(const FaceHandle& lhs, const FaceHandle& rhs);

  /**
   * Pick the center point of this handle.
   */
  std::optional<Hit> pick(
    HitType::Type hitType,
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius) const;

  /**
   * Pick a point on this handle, but snap it to the grid.
   * This is used for splitting edges in the vertex tool.
   */
  std::optional<Hit> pick(
    HitType::Type hitType,
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius,
    const Grid& grid) const;

  kdl_reflect_decl(FaceHandle, position);
};

struct ControlPointHandle
{
  using Position = vm::vec3d;

  static const HitType::Type HandleHitType;

  Position position;

  static std::vector<ControlPointHandle> getHandles(const Node& node);

  template <std::ranges::range R>
  static std::vector<Position> getPositions(R&& range)
  {
    return range | std::views::transform(&ControlPointHandle::position)
           | kdl::ranges::to<std::vector>();
  }

  template <std::ranges::range R>
  static std::vector<vm::vec3d> getVertices(R&& range)
  {
    return range | std::views::transform(&ControlPointHandle::position)
           | kdl::ranges::to<std::vector>();
  }

  static double distance(const ControlPointHandle& lhs, const ControlPointHandle& rhs);

  /**
   * Pick this handle.
   */
  std::optional<Hit> pick(
    HitType::Type hitType,
    const vm::ray3d& pickRay,
    const gl::Camera& camera,
    double handleRadius) const;

  kdl_reflect_decl(ControlPointHandle, position);
};

} // namespace mdl
} // namespace tb