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

#include "ui/ControlPointTool.h"

#include "mdl/Map.h"
#include "mdl/Map_Patches.h"
#include "mdl/NodeHandles.h"
#include "ui/ControlPointToolPage.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/string_format.h"

namespace tb::ui
{

ControlPointTool::ControlPointTool(MapDocument& document)
  : NodeHandleToolBase{document}
{
}

void ControlPointTool::pick(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  mdl::PickResult& pickResult) const
{
  m_document.map().nodeHandles().pick<mdl::ControlPointHandle>(
    pickResult, mdl::ControlPointHandle::HandleHitType, pickRay, camera, handleRadius);
}

std::tuple<vm::vec3d, vm::vec3d> ControlPointTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  contract_pre(!hits.empty());

  const auto& hit = hits.front();
  contract_assert(hit.hasType(mdl::ControlPointHandle::HandleHitType));

  return {hit.target<mdl::ControlPointHandle>().position, hit.hitPoint()};
}

ControlPointTool::MoveResult ControlPointTool::move(const vm::vec3d& delta)
{
  auto& map = m_document.map();

  const auto controlPointPositions = mdl::ControlPointHandle::getPositions(
    map.nodeHandles().selectedHandles<mdl::ControlPointHandle>());
  const auto transform = vm::translation_matrix(delta);
  if (transformControlPoints(map, controlPointPositions, transform))
  {
    m_dragHandlePosition = transform * m_dragHandlePosition;
    return MoveResult::Continue;
  }
  return MoveResult::Deny;
}

std::string ControlPointTool::actionName() const
{
  return kdl::str_plural(
    handleManager().selectedHandleCount<mdl::ControlPointHandle>(),
    "Move Control Point",
    "Move Control Points");
}

QWidget* ControlPointTool::doCreatePage(QWidget* parent)
{
  return new ControlPointToolPage{m_document, parent};
}

void ControlPointTool::addHandles(const std::vector<mdl::Node*>& nodes)
{
  m_document.map().nodeHandles().addHandles<mdl::ControlPointHandle>(nodes);
}

void ControlPointTool::removeHandles(const std::vector<mdl::Node*>& nodes)
{
  m_document.map().nodeHandles().removeHandles<mdl::ControlPointHandle>(nodes);
}

} // namespace tb::ui
