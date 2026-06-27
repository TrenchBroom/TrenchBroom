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

#include "mdl/NodeHandles.h"

#include "gl/Camera.h"
#include "mdl/Brush.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/Grid.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/ranges/zip_transform_view.h"
#include "kd/reflection_impl.h"

#include "vm/polygon_io.h" // IWYU pragma: keep
#include "vm/segment_io.h" // IWYU pragma: keep
#include "vm/vec_io.h"     // IWYU pragma: keep

#include <limits>

namespace tb::mdl
{

const HitType::Type VertexHandle::HandleHitType = HitType::freeType();
const HitType::Type EdgeHandle::HandleHitType = HitType::freeType();
const HitType::Type FaceHandle::HandleHitType = HitType::freeType();

kdl_reflect_impl(VertexHandle);
kdl_reflect_impl(EdgeHandle);
kdl_reflect_impl(FaceHandle);

std::vector<VertexHandle> VertexHandle::getHandles(const Node& node)
{
  auto result = std::vector<VertexHandle>{};

  node.accept(kdl::overload(
    [](const WorldNode&) {},
    [](const LayerNode&) {},
    [](const GroupNode&) {},
    [](const EntityNode&) {},
    [&](const BrushNode& brushNode) {
      for (const auto& vertexPosition : brushNode.brush().vertexPositions())
      {
        result.emplace_back(vertexPosition);
      }
    },
    [](const PatchNode&) {}));

  return result;
}

double VertexHandle::distance(const VertexHandle& lhs, const VertexHandle& rhs)
{
  return vm::distance(lhs.position, rhs.position);
}

std::optional<Hit> VertexHandle::pick(
  const HitType::Type hitType,
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  double handleRadius) const
{
  return camera.pickPointHandle(pickRay, position, handleRadius)
         | kdl::optional_transform([&](const auto distance) {
             return Hit{
               hitType, distance, vm::point_at_distance(pickRay, distance), *this};
           });
}

std::vector<EdgeHandle> EdgeHandle::getHandles(const Node& node)
{
  auto result = std::vector<EdgeHandle>{};

  node.accept(kdl::overload(
    [](const WorldNode&) {},
    [](const LayerNode&) {},
    [](const GroupNode&) {},
    [](const EntityNode&) {},
    [&](const BrushNode& brushNode) {
      for (const auto* edge : brushNode.brush().edges())
      {
        result.emplace_back(edge->segment());
      }
    },
    [](const PatchNode&) {}));

  return result;
}

double EdgeHandle::distance(const EdgeHandle& lhs, const EdgeHandle& rhs)
{
  return vm::max(
    vm::distance(lhs.position.start(), rhs.position.start()),
    vm::distance(lhs.position.end(), rhs.position.end()));
}

std::optional<Hit> EdgeHandle::pick(
  const HitType::Type hitType,
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  double handleRadius) const
{
  return camera.pickPointHandle(pickRay, position.center(), handleRadius)
         | kdl::optional_transform([&](const auto distance) {
             return Hit{
               hitType, distance, vm::point_at_distance(pickRay, distance), *this};
           });
}

std::optional<Hit> EdgeHandle::pick(
  const HitType::Type hitType,
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  double handleRadius,
  const Grid& grid) const
{
  return camera.pickLineSegmentHandle(pickRay, position, handleRadius)
         | kdl::optional_and_then([&](const auto distance) {
             return grid.snap(vm::point_at_distance(pickRay, distance), position);
           })
         | kdl::optional_and_then([&](const auto& pointHandle) {
             return camera.pickPointHandle(pickRay, pointHandle, handleRadius)
                    | kdl::optional_transform([&](const auto distance) {
                        const auto hitPoint = vm::point_at_distance(pickRay, distance);
                        return Hit{
                          hitType,
                          distance,
                          hitPoint,
                          GridHandleHitData{*this, pointHandle}};
                      });
           });
}

std::vector<FaceHandle> FaceHandle::getHandles(const Node& node)
{
  auto result = std::vector<FaceHandle>{};

  node.accept(kdl::overload(
    [](const WorldNode&) {},
    [](const LayerNode&) {},
    [](const GroupNode&) {},
    [](const EntityNode&) {},
    [&](const BrushNode& brushNode) {
      for (const auto& face : brushNode.brush().faces())
      {
        result.emplace_back(face.polygon());
      }
    },
    [](const PatchNode&) {}));

  return result;
}

double FaceHandle::distance(const FaceHandle& lhs, const FaceHandle& rhs)
{
  if (lhs.position.vertexCount() != rhs.position.vertexCount())
  {
    return std::numeric_limits<double>::max();
  }

  const auto distances = kdl::views::zip_transform(
    [](const auto& lhsVertex, const auto& rhsVertex) {
      return vm::distance(lhsVertex, rhsVertex);
    },
    lhs.position.vertices(),
    rhs.position.vertices());

  return *std::ranges::max_element(distances);
}

std::optional<Hit> FaceHandle::pick(
  const HitType::Type hitType,
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  double handleRadius) const
{
  return camera.pickPointHandle(pickRay, position.center(), handleRadius)
         | kdl::optional_transform([&](const auto distance) {
             const auto hitPoint = vm::point_at_distance(pickRay, distance);
             return Hit{hitType, distance, hitPoint, *this};
           });
}

std::optional<Hit> FaceHandle::pick(
  const HitType::Type hitType,
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  double handleRadius,
  const Grid& grid) const
{
  const auto iBegin = position.vertices().begin();
  const auto iEnd = position.vertices().end();

  return vm::from_points(iBegin, iEnd) | kdl::optional_and_then([&](const auto& plane) {
           return vm::intersect_ray_polygon(pickRay, plane, iBegin, iEnd)
                  | kdl::optional_transform([&](const auto distance) {
                      return grid.snap(vm::point_at_distance(pickRay, distance), plane);
                    });
         })
         | kdl::optional_and_then([&](const auto& pointHandle) {
             return camera.pickPointHandle(pickRay, pointHandle, handleRadius)
                    | kdl::optional_transform([&](const auto& distance) {
                        const auto hitPoint = vm::point_at_distance(pickRay, distance);
                        return Hit{
                          hitType,
                          distance,
                          hitPoint,
                          GridHandleHitData{*this, pointHandle}};
                      });
           });
}

} // namespace tb::mdl
