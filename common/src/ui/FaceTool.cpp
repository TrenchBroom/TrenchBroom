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

#include "FaceTool.h"

#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"

#include "kd/string_format.h"

namespace tb::ui
{

FaceTool::FaceTool(mdl::Map& map)
  : VertexToolBase{map}
{
}

std::vector<mdl::BrushNode*> FaceTool::findIncidentBrushes(
  const vm::polygon3d& handle) const
{
  return findIncidentBrushes(m_map.faceHandles(), handle);
}

void FaceTool::pick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  m_map.faceHandles().pickCenterHandle(pickRay, camera, pickResult);
}

mdl::FaceHandleManager& FaceTool::handleManager()
{
  return m_map.faceHandles();
}

const mdl::FaceHandleManager& FaceTool::handleManager() const
{
  return m_map.faceHandles();
}

std::tuple<vm::vec3d, vm::vec3d> FaceTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  assert(!hits.empty());

  const auto& hit = hits.front();
  assert(hit.hasType(mdl::FaceHandleManager::HandleHitType));

  return {hit.target<vm::polygon3d>().center(), hit.hitPoint()};
}

FaceTool::MoveResult FaceTool::move(const vm::vec3d& delta)
{
  auto handles = m_map.faceHandles().selectedHandles();
  const auto transform = vm::translation_matrix(delta);
  if (transformFaces(m_map, std::move(handles), transform))
  {
    m_dragHandlePosition = m_dragHandlePosition.transform(transform);
    return MoveResult::Continue;
  }
  return MoveResult::Deny;
}

std::string FaceTool::actionName() const
{
  return kdl::str_plural(
    m_map.faceHandles().selectedHandleCount(), "Move Face", "Move Faces");
}

void FaceTool::removeSelection()
{
  const auto handles = m_map.faceHandles().selectedHandles();
  auto vertexPositions = std::vector<vm::vec3d>{};
  vm::polygon3d::get_vertices(
    std::begin(handles), std::end(handles), std::back_inserter(vertexPositions));

  const auto commandName =
    kdl::str_plural(handles.size(), "Remove Brush Face", "Remove Brush Faces");
  removeVertices(m_map, commandName, std::move(vertexPositions));
}

} // namespace tb::ui
