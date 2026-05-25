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

#include "ui/EdgeTool.h"

#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/NodeHandles.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/string_format.h"

namespace tb::ui
{

EdgeTool::EdgeTool(MapDocument& document)
  : VertexToolBase{document}
{
}

void EdgeTool::pick(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  mdl::PickResult& pickResult) const
{
  m_document.map().nodeHandles().pick<mdl::EdgeHandle>(
    pickResult, mdl::EdgeHandle::HandleHitType, pickRay, camera, handleRadius);
}

std::tuple<vm::vec3d, vm::vec3d> EdgeTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  contract_pre(!hits.empty());

  const auto& hit = hits.front();
  contract_assert(hit.hasType(mdl::EdgeHandle::HandleHitType));

  return {hit.target<mdl::EdgeHandle>().position.center(), hit.hitPoint()};
}

EdgeTool::MoveResult EdgeTool::move(const vm::vec3d& delta)
{
  auto& map = m_document.map();

  const auto edgePositions =
    mdl::EdgeHandle::getPositions(map.nodeHandles().selectedHandles<mdl::EdgeHandle>());
  const auto transform = vm::translation_matrix(delta);
  if (transformEdges(map, edgePositions, transform))
  {
    m_dragHandlePosition = m_dragHandlePosition.transform(transform);
    return MoveResult::Continue;
  }
  return MoveResult::Deny;
}

std::string EdgeTool::actionName() const
{
  return kdl::str_plural(
    handleManager().selectedHandleCount<mdl::EdgeHandle>(), "Move Edge", "Move Edges");
}

void EdgeTool::removeSelection()
{
  auto& map = m_document.map();

  auto handles = map.nodeHandles().selectedHandles<mdl::EdgeHandle>();
  const auto vertexPositions = mdl::EdgeHandle::getVertices(handles);

  const auto commandName = kdl::str_plural(
    vertexPositions.size() / 2, "Remove Brush Edge", "Remove Brush Edges");
  removeVertices(map, commandName, vertexPositions);
}

} // namespace tb::ui
