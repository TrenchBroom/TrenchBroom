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

#include "EdgeTool.h"

#include "kdl/memory_utils.h"
#include "kdl/string_format.h"

namespace tb::ui
{

EdgeTool::EdgeTool(std::weak_ptr<MapDocument> document)
  : VertexToolBase{std::move(document)}
{
}

std::vector<mdl::BrushNode*> EdgeTool::findIncidentBrushes(
  const vm::segment3d& handle) const
{
  auto document = kdl::mem_lock(m_document);
  return findIncidentBrushes(document->edgeHandles(), handle);
}

void EdgeTool::pick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  auto document = kdl::mem_lock(m_document);
  document->edgeHandles().pickCenterHandle(pickRay, camera, pickResult);
}

mdl::EdgeHandleManager& EdgeTool::handleManager()
{
  auto document = kdl::mem_lock(m_document);
  return document->edgeHandles();
}

const mdl::EdgeHandleManager& EdgeTool::handleManager() const
{
  auto document = kdl::mem_lock(m_document);
  return document->edgeHandles();
}

std::tuple<vm::vec3d, vm::vec3d> EdgeTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  assert(!hits.empty());

  const auto& hit = hits.front();
  assert(hit.hasType(mdl::EdgeHandleManager::HandleHitType));

  return {hit.target<vm::segment3d>().center(), hit.hitPoint()};
}

EdgeTool::MoveResult EdgeTool::move(const vm::vec3d& delta)
{
  auto document = kdl::mem_lock(m_document);

  auto handles = document->edgeHandles().selectedHandles();
  const auto transform = vm::translation_matrix(delta);
  if (document->transformEdges(std::move(handles), transform))
  {
    m_dragHandlePosition = m_dragHandlePosition.transform(transform);
    return MoveResult::Continue;
  }
  return MoveResult::Deny;
}

std::string EdgeTool::actionName() const
{
  auto document = kdl::mem_lock(m_document);
  return kdl::str_plural(
    document->edgeHandles().selectedHandleCount(), "Move Edge", "Move Edges");
}

void EdgeTool::removeSelection()
{
  auto document = kdl::mem_lock(m_document);

  const auto handles = document->edgeHandles().selectedHandles();
  auto vertexPositions = std::vector<vm::vec3d>{};
  vertexPositions.reserve(2 * vertexPositions.size());
  vm::segment3d::get_vertices(
    std::begin(handles), std::end(handles), std::back_inserter(vertexPositions));

  const auto commandName =
    kdl::str_plural(handles.size(), "Remove Brush Edge", "Remove Brush Edges");
  kdl::mem_lock(m_document)->removeVertices(commandName, std::move(vertexPositions));
}

} // namespace tb::ui
