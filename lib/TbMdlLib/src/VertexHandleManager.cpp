/*
 Copyright (C) 2010 Kristian Duske

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

#include "mdl/VertexHandleManager.h"

#include "mdl/BrushFace.h"
#include "mdl/Grid.h"
#include "mdl/Polyhedron.h"

#include "vm/distance.h"
#include "vm/polygon.h"
#include "vm/ray.h"
#include "vm/vec.h"

namespace tb::mdl
{

VertexHandleManagerBase::~VertexHandleManagerBase() = default;

const HitType::Type VertexHandleManager::HandleHitType = HitType::freeType();

void VertexHandleManager::pick(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    if (const auto distance = camera.pickPointHandle(pickRay, position, handleRadius))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *distance);
      const auto error = vm::squared_distance(pickRay, position).distance;
      pickResult.addHit(Hit(HandleHitType, *distance, hitPoint, position, error));
    }
  }
}

void VertexHandleManager::addHandles(const BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* vertex : brush.vertices())
  {
    add(vertex->position());
  }
}

void VertexHandleManager::removeHandles(const BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* vertex : brush.vertices())
  {
    assertResult(remove(vertex->position()));
  }
}

HitType::Type VertexHandleManager::hitType() const
{
  return HandleHitType;
}

bool VertexHandleManager::isIncident(
  const Handle& handle, const BrushNode* brushNode) const
{
  const auto& brush = brushNode->brush();
  return brush.hasVertex(handle);
}

const HitType::Type EdgeHandleManager::HandleHitType = HitType::freeType();

void EdgeHandleManager::pickGridHandle(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  const Grid& grid,
  PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    if (
      const auto edgeDist = camera.pickLineSegmentHandle(pickRay, position, handleRadius))
    {
      if (
        const auto pointHandle =
          grid.snap(vm::point_at_distance(pickRay, *edgeDist), position))
      {
        if (
          const auto pointDist =
            camera.pickPointHandle(pickRay, *pointHandle, handleRadius))
        {
          const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
          pickResult.addHit(
            Hit{HandleHitType, *pointDist, hitPoint, HitData{position, *pointHandle}});
        }
      }
    }
  }
}

void EdgeHandleManager::pickCenterHandle(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    const auto pointHandle = position.center();

    if (const auto pointDist = camera.pickPointHandle(pickRay, pointHandle, handleRadius))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
      pickResult.addHit(Hit{HandleHitType, *pointDist, hitPoint, position});
    }
  }
}

void EdgeHandleManager::addHandles(const BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* edge : brush.edges())
  {
    add(vm::segment3d{edge->firstVertex()->position(), edge->secondVertex()->position()});
  }
}

void EdgeHandleManager::removeHandles(const BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* edge : brush.edges())
  {
    assertResult(remove(
      vm::segment3d{edge->firstVertex()->position(), edge->secondVertex()->position()}));
  }
}

HitType::Type EdgeHandleManager::hitType() const
{
  return HandleHitType;
}

bool EdgeHandleManager::isIncident(const Handle& handle, const BrushNode* brushNode) const
{
  const auto& brush = brushNode->brush();
  return brush.hasEdge(handle);
}

const HitType::Type FaceHandleManager::HandleHitType = HitType::freeType();

void FaceHandleManager::pickGridHandle(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  const Grid& grid,
  PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    if (
      const auto plane =
        vm::from_points(position.vertices().begin(), position.vertices().end()))
    {
      if (
        const auto distance = vm::intersect_ray_polygon(
          pickRay, *plane, position.vertices().begin(), position.vertices().end()))
      {
        const auto pointHandle =
          grid.snap(vm::point_at_distance(pickRay, *distance), *plane);

        if (
          const auto pointDist =
            camera.pickPointHandle(pickRay, pointHandle, handleRadius))
        {
          const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
          pickResult.addHit(
            Hit{HandleHitType, *pointDist, hitPoint, HitData{position, pointHandle}});
        }
      }
    }
  }
}

void FaceHandleManager::pickCenterHandle(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    const auto pointHandle = position.center();

    if (const auto pointDist = camera.pickPointHandle(pickRay, pointHandle, handleRadius))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
      pickResult.addHit(Hit{HandleHitType, *pointDist, hitPoint, position});
    }
  }
}

void FaceHandleManager::addHandles(const BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto& face : brush.faces())
  {
    add(face.polygon());
  }
}

void FaceHandleManager::removeHandles(const BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto& face : brush.faces())
  {
    assertResult(remove(face.polygon()));
  }
}

HitType::Type FaceHandleManager::hitType() const
{
  return HandleHitType;
}

bool FaceHandleManager::isIncident(const Handle& handle, const BrushNode* brushNode) const
{
  const auto& brush = brushNode->brush();
  return brush.hasFace(handle);
}

} // namespace tb::mdl
