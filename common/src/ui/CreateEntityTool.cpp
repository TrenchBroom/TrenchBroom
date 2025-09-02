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

#include "CreateEntityTool.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/Grid.h"
#include "mdl/HitAdapter.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/PickResult.h"
#include "mdl/TransactionScope.h"
#include "mdl/WorldNode.h"
#include "render/Camera.h"

#include "kdl/k.h"

#include <string>

namespace tb::ui
{

CreateEntityTool::CreateEntityTool(mdl::Map& map)
  : Tool{K(initiallyActive)}
  , m_map{map}
{
}

bool CreateEntityTool::createEntity(const std::string& classname)
{
  const auto& definitionManager = m_map.entityDefinitionManager();
  const auto* definition = definitionManager.definition(classname);
  if (!definition || getType(*definition) != mdl::EntityDefinitionType::Point)
  {
    return false;
  }

  m_referenceBounds = m_map.referenceBounds();

  m_map.startTransaction(
    "Create '" + definition->name + "'", mdl::TransactionScope::LongRunning);
  m_entity = m_map.createPointEntity(*definition, {0, 0, 0});

  return m_entity != nullptr;
}

void CreateEntityTool::removeEntity()
{
  ensure(m_entity != nullptr, "entity is not null");

  m_map.cancelTransaction();
  m_entity = nullptr;
}

void CreateEntityTool::commitEntity()
{
  ensure(m_entity != nullptr, "entity is not null");

  m_map.commitTransaction();
  m_entity = nullptr;
}

void CreateEntityTool::updateEntityPosition2D(const vm::ray3d& pickRay)
{
  ensure(m_entity != nullptr, "entity is not null");

  const auto toMin = m_referenceBounds.min - pickRay.origin;
  const auto toMax = m_referenceBounds.max - pickRay.origin;
  const auto anchor = dot(toMin, pickRay.direction) > dot(toMax, pickRay.direction)
                        ? m_referenceBounds.min
                        : m_referenceBounds.max;
  const auto dragPlane = vm::plane3d(anchor, -pickRay.direction);

  const auto& grid = m_map.grid();
  const auto delta = grid.moveDeltaForBounds(
    dragPlane, m_entity->logicalBounds(), m_map.worldBounds(), pickRay);

  if (!vm::is_zero(delta, vm::Cd::almost_zero()))
  {
    m_map.translateSelection(delta);
  }
}

void CreateEntityTool::updateEntityPosition3D(
  const vm::ray3d& pickRay, const mdl::PickResult& pickResult)
{
  using namespace mdl::HitFilters;

  ensure(m_entity != nullptr, "entity is not null");

  auto delta = vm::vec3d{};
  const auto& grid = m_map.grid();
  const auto& hit = pickResult.first(type(mdl::BrushNode::BrushHitType));
  if (const auto faceHandle = mdl::hitToFaceHandle(hit))
  {
    const auto& face = faceHandle->face();
    delta = grid.moveDeltaForBounds(
      face.boundary(), m_entity->logicalBounds(), m_map.worldBounds(), pickRay);
  }
  else
  {
    const auto newPosition = vm::point_at_distance(
      pickRay, static_cast<double>(render::Camera::DefaultPointDistance));
    const auto boundsCenter = m_entity->logicalBounds().center();
    delta = grid.moveDeltaForPoint(boundsCenter, newPosition - boundsCenter);
  }

  if (!vm::is_zero(delta, vm::Cd::almost_zero()))
  {
    m_map.translateSelection(delta);
  }
}

} // namespace tb::ui
