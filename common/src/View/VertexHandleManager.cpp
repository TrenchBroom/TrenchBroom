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

#include "VertexHandleManager.h"

#include "Model/BrushFace.h"
#include "Model/Polyhedron.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/Grid.h"

#include "vm/distance.h"
#include "vm/polygon.h"
#include "vm/ray.h"
#include "vm/vec.h"

namespace tb::View
{

VertexHandleManagerBase::~VertexHandleManagerBase() = default;

const Model::HitType::Type VertexHandleManager::HandleHitType =
  Model::HitType::freeType();

void VertexHandleManager::pick(
  const vm::ray3d& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    if (
      const auto distance = camera.pickPointHandle(
        pickRay, position, double(pref(Preferences::HandleRadius))))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *distance);
      const auto error = vm::squared_distance(pickRay, position).distance;
      pickResult.addHit(Model::Hit(HandleHitType, *distance, hitPoint, position, error));
    }
  }
}

void VertexHandleManager::addHandles(const Model::BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* vertex : brush.vertices())
  {
    add(vertex->position());
  }
}

void VertexHandleManager::removeHandles(const Model::BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* vertex : brush.vertices())
  {
    assertResult(remove(vertex->position()));
  }
}

Model::HitType::Type VertexHandleManager::hitType() const
{
  return HandleHitType;
}

bool VertexHandleManager::isIncident(
  const Handle& handle, const Model::BrushNode* brushNode) const
{
  const auto& brush = brushNode->brush();
  return brush.hasVertex(handle);
}

const Model::HitType::Type EdgeHandleManager::HandleHitType = Model::HitType::freeType();

void EdgeHandleManager::pickGridHandle(
  const vm::ray3d& pickRay,
  const Renderer::Camera& camera,
  const Grid& grid,
  Model::PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    if (
      const auto edgeDist = camera.pickLineSegmentHandle(
        pickRay, position, double(pref(Preferences::HandleRadius))))
    {
      if (
        const auto pointHandle =
          grid.snap(vm::point_at_distance(pickRay, *edgeDist), position))
      {
        if (
          const auto pointDist = camera.pickPointHandle(
            pickRay, *pointHandle, double(pref(Preferences::HandleRadius))))
        {
          const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
          pickResult.addHit(Model::Hit{
            HandleHitType, *pointDist, hitPoint, HitType{position, *pointHandle}});
        }
      }
    }
  }
}

void EdgeHandleManager::pickCenterHandle(
  const vm::ray3d& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    const auto pointHandle = position.center();

    if (
      const auto pointDist = camera.pickPointHandle(
        pickRay, pointHandle, double(pref(Preferences::HandleRadius))))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
      pickResult.addHit(Model::Hit{HandleHitType, *pointDist, hitPoint, position});
    }
  }
}

void EdgeHandleManager::addHandles(const Model::BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* edge : brush.edges())
  {
    add(vm::segment3d{edge->firstVertex()->position(), edge->secondVertex()->position()});
  }
}

void EdgeHandleManager::removeHandles(const Model::BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto* edge : brush.edges())
  {
    assertResult(remove(
      vm::segment3d{edge->firstVertex()->position(), edge->secondVertex()->position()}));
  }
}

Model::HitType::Type EdgeHandleManager::hitType() const
{
  return HandleHitType;
}

bool EdgeHandleManager::isIncident(
  const Handle& handle, const Model::BrushNode* brushNode) const
{
  const auto& brush = brushNode->brush();
  return brush.hasEdge(handle);
}

const Model::HitType::Type FaceHandleManager::HandleHitType = Model::HitType::freeType();

void FaceHandleManager::pickGridHandle(
  const vm::ray3d& pickRay,
  const Renderer::Camera& camera,
  const Grid& grid,
  Model::PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    if (const auto plane = vm::from_points(std::begin(position), std::end(position)))
    {
      if (
        const auto distance = vm::intersect_ray_polygon(
          pickRay, *plane, std::begin(position), std::end(position)))
      {
        const auto pointHandle =
          grid.snap(vm::point_at_distance(pickRay, *distance), *plane);

        if (
          const auto pointDist = camera.pickPointHandle(
            pickRay, pointHandle, double(pref(Preferences::HandleRadius))))
        {
          const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
          pickResult.addHit(Model::Hit{
            HandleHitType, *pointDist, hitPoint, HitType{position, pointHandle}});
        }
      }
    }
  }
}

void FaceHandleManager::pickCenterHandle(
  const vm::ray3d& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  for (const auto& [position, info] : m_handles)
  {
    const auto pointHandle = position.center();

    if (
      const auto pointDist = camera.pickPointHandle(
        pickRay, pointHandle, double(pref(Preferences::HandleRadius))))
    {
      const auto hitPoint = vm::point_at_distance(pickRay, *pointDist);
      pickResult.addHit(Model::Hit{HandleHitType, *pointDist, hitPoint, position});
    }
  }
}

void FaceHandleManager::addHandles(const Model::BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto& face : brush.faces())
  {
    add(face.polygon());
  }
}

void FaceHandleManager::removeHandles(const Model::BrushNode* brushNode)
{
  const auto& brush = brushNode->brush();
  for (const auto& face : brush.faces())
  {
    assertResult(remove(face.polygon()));
  }
}

Model::HitType::Type FaceHandleManager::hitType() const
{
  return HandleHitType;
}

bool FaceHandleManager::isIncident(
  const Handle& handle, const Model::BrushNode* brushNode) const
{
  const auto& brush = brushNode->brush();
  return brush.hasFace(handle);
}

} // namespace tb::View
