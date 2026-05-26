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

#include "ui/FaceTool.h"

#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/NodeHandles.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"
#include "kd/string_format.h"

namespace tb::ui
{

FaceTool::FaceTool(MapDocument& document)
  : NodeHandleToolBase{document}
{
}

void FaceTool::pick(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  mdl::PickResult& pickResult) const
{
  m_document.map().nodeHandles().pick<mdl::FaceHandle>(
    pickResult, mdl::FaceHandle::HandleHitType, pickRay, camera, handleRadius);
}

std::tuple<vm::vec3d, vm::vec3d> FaceTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  contract_pre(!hits.empty());

  const auto& hit = hits.front();
  contract_assert(hit.hasType(mdl::FaceHandle::HandleHitType));

  return {hit.target<mdl::FaceHandle>().position.center(), hit.hitPoint()};
}

FaceTool::MoveResult FaceTool::move(const vm::vec3d& delta)
{
  auto& map = m_document.map();

  const auto facePositions =
    mdl::FaceHandle::getPositions(map.nodeHandles().selectedHandles<mdl::FaceHandle>());
  const auto transform = vm::translation_matrix(delta);
  if (transformFaces(map, facePositions, transform))
  {
    m_dragHandlePosition = m_dragHandlePosition.transform(transform);
    return MoveResult::Continue;
  }
  return MoveResult::Deny;
}

std::string FaceTool::actionName() const
{
  return kdl::str_plural(
    m_document.map().nodeHandles().selectedHandleCount<mdl::FaceHandle>(),
    "Move Face",
    "Move Faces");
}

void FaceTool::removeSelection()
{
  auto& map = m_document.map();

  const auto handles =
    map.nodeHandles().selectedHandles<mdl::FaceHandle>() | kdl::ranges::to<std::vector>();
  const auto vertexPositions = mdl::FaceHandle::getVertices(handles);

  const auto commandName =
    kdl::str_plural(handles.size(), "Remove Brush Face", "Remove Brush Faces");
  removeVertices(map, commandName, vertexPositions);
}

} // namespace tb::ui
