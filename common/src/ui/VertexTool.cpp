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

#include "VertexTool.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "mdl/BrushNode.h"
#include "mdl/BrushVertexCommands.h"
#include "render/RenderBatch.h"
#include "ui/MapDocument.h"

#include "kdl/const_overload.h"
#include "kdl/string_format.h"

#include "vm/polygon.h"

#include <cassert>
#include <tuple>
#include <vector>

namespace tb::ui
{

VertexTool::VertexTool(std::weak_ptr<MapDocument> document)
  : VertexToolBase{std::move(document)}
  , m_mode{Mode::Move}
  , m_guideRenderer{m_document}
{
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::vec3d& handle) const
{
  const auto& map = kdl::mem_lock(m_document)->map();
  return findIncidentBrushes(map.vertexHandles(), handle);
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::segment3d& handle) const
{
  const auto& map = kdl::mem_lock(m_document)->map();
  return findIncidentBrushes(map.edgeHandles(), handle);
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::polygon3d& handle) const
{
  const auto& map = kdl::mem_lock(m_document)->map();
  return findIncidentBrushes(map.faceHandles(), handle);
}

void VertexTool::pick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  const auto& map = kdl::mem_lock(m_document)->map();
  const auto& grid = map.grid();

  map.vertexHandles().pick(pickRay, camera, pickResult);
  map.edgeHandles().pickGridHandle(pickRay, camera, grid, pickResult);
  map.faceHandles().pickGridHandle(pickRay, camera, grid, pickResult);
}

bool VertexTool::deselectAll()
{
  if (VertexToolBase::deselectAll())
  {
    resetModeAfterDeselection();
    return true;
  }
  return false;
}

mdl::VertexHandleManager& VertexTool::handleManager()
{
  return KDL_CONST_OVERLOAD(handleManager());
}

const mdl::VertexHandleManager& VertexTool::handleManager() const
{
  const auto& map = kdl::mem_lock(m_document)->map();
  return map.vertexHandles();
}

std::tuple<vm::vec3d, vm::vec3d> VertexTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  assert(!hits.empty());

  const auto& hit = hits.front();
  assert(hit.hasType(
    mdl::VertexHandleManager::HandleHitType | mdl::EdgeHandleManager::HandleHitType
    | mdl::FaceHandleManager::HandleHitType));

  const auto position = hit.hasType(mdl::VertexHandleManager::HandleHitType)
                          ? hit.target<vm::vec3d>()
                        : hit.hasType(mdl::EdgeHandleManager::HandleHitType)
                          ? std::get<1>(hit.target<mdl::EdgeHandleManager::HitData>())
                          : std::get<1>(hit.target<mdl::FaceHandleManager::HitData>());

  return {position, hit.hitPoint()};
}

bool VertexTool::startMove(const std::vector<mdl::Hit>& hits)
{
  const auto& hit = hits.front();
  if (hit.hasType(
        mdl::EdgeHandleManager::HandleHitType | mdl::FaceHandleManager::HandleHitType))
  {
    auto& map = kdl::mem_lock(m_document)->map();

    map.vertexHandles().deselectAll();
    if (hit.hasType(mdl::EdgeHandleManager::HandleHitType))
    {
      const auto& handle =
        std::get<0>(hit.target<const mdl::EdgeHandleManager::HitData&>());
      map.edgeHandles().select(handle);
      m_mode = Mode::SplitEdge;
    }
    else
    {
      const auto& handle =
        std::get<0>(hit.target<const mdl::FaceHandleManager::HitData&>());
      map.faceHandles().select(handle);
      m_mode = Mode::SplitFace;
    }
    refreshViews();
  }
  else
  {
    m_mode = Mode::Move;
  }

  if (!VertexToolBase::startMove(hits))
  {
    m_mode = Mode::Move;
    return false;
  }
  return true;
}

VertexTool::MoveResult VertexTool::move(const vm::vec3d& delta)
{
  auto& map = kdl::mem_lock(m_document)->map();
  const auto transform = vm::translation_matrix(delta);

  if (m_mode == Mode::Move)
  {
    auto handles = map.vertexHandles().selectedHandles();
    const auto result = map.transformVertices(std::move(handles), transform);
    if (result.success)
    {
      if (!result.hasRemainingVertices)
      {
        return MoveResult::Cancel;
      }
      m_dragHandlePosition = transform * m_dragHandlePosition;
      return MoveResult::Continue;
    }
    return MoveResult::Deny;
  }

  auto brushes = std::vector<mdl::BrushNode*>{};
  if (m_mode == Mode::SplitEdge)
  {
    if (map.edgeHandles().selectedHandleCount() == 1)
    {
      const auto handle = map.edgeHandles().selectedHandles().front();
      brushes = findIncidentBrushes(handle);
    }
  }
  else
  {
    assert(m_mode == Mode::SplitFace);
    if (map.faceHandles().selectedHandleCount() == 1)
    {
      const auto handle = map.faceHandles().selectedHandles().front();
      brushes = findIncidentBrushes(handle);
    }
  }

  if (!brushes.empty())
  {
    const auto newVertexPosition = transform * m_dragHandlePosition;
    if (map.addVertex(newVertexPosition))
    {
      m_mode = Mode::Move;
      map.edgeHandles().deselectAll();
      map.faceHandles().deselectAll();
      m_dragHandlePosition = transform * m_dragHandlePosition;
      map.vertexHandles().select(m_dragHandlePosition);
    }
    return MoveResult::Continue;
  }

  // Catch all failure cases: no brushes were selected or vertices could not be added:
  return MoveResult::Deny;
}

void VertexTool::endMove()
{
  auto& map = kdl::mem_lock(m_document)->map();

  VertexToolBase::endMove();
  map.edgeHandles().deselectAll();
  map.faceHandles().deselectAll();
  m_mode = Mode::Move;
}
void VertexTool::cancelMove()
{
  auto& map = kdl::mem_lock(m_document)->map();

  VertexToolBase::cancelMove();
  map.edgeHandles().deselectAll();
  map.faceHandles().deselectAll();
  m_mode = Mode::Move;
}

bool VertexTool::allowAbsoluteSnapping() const
{
  return true;
}

vm::vec3d VertexTool::getHandlePosition(const mdl::Hit& hit) const
{
  assert(hit.isMatch());
  assert(hit.hasType(
    mdl::VertexHandleManager::HandleHitType | mdl::EdgeHandleManager::HandleHitType
    | mdl::FaceHandleManager::HandleHitType));

  return hit.hasType(mdl::VertexHandleManager::HandleHitType) ? hit.target<vm::vec3d>()
         : hit.hasType(mdl::EdgeHandleManager::HandleHitType)
           ? std::get<1>(hit.target<mdl::EdgeHandleManager::HitData>())
           : std::get<1>(hit.target<mdl::FaceHandleManager::HitData>());
}

std::string VertexTool::actionName() const
{
  const auto& map = kdl::mem_lock(m_document)->map();

  switch (m_mode)
  {
  case Mode::Move:
    return kdl::str_plural(
      map.vertexHandles().selectedHandleCount(), "Move Vertex", "Move Vertices");
  case Mode::SplitEdge:
    return "Split Edge";
  case Mode::SplitFace:
    return "Split Face";
    switchDefault();
  }
}

void VertexTool::removeSelection()
{
  assert(canRemoveSelection());

  auto& map = kdl::mem_lock(m_document)->map();
  auto handles = map.vertexHandles().selectedHandles();

  const auto commandName =
    kdl::str_plural(handles.size(), "Remove Brush Vertex", "Remove Brush Vertices");
  map.removeVertices(commandName, std::move(handles));
}

void VertexTool::renderGuide(
  render::RenderContext&,
  render::RenderBatch& renderBatch,
  const vm::vec3d& position) const
{
  m_guideRenderer.setPosition(position);
  m_guideRenderer.setColor(Color(pref(Preferences::HandleColor), 0.5f));
  renderBatch.add(&m_guideRenderer);
}

bool VertexTool::doActivate()
{
  VertexToolBase::doActivate();

  auto& map = kdl::mem_lock(m_document)->map();

  map.edgeHandles().clear();
  map.faceHandles().clear();

  const auto& brushes = selectedBrushes();
  map.edgeHandles().addHandles(brushes);
  map.faceHandles().addHandles(brushes);

  m_mode = Mode::Move;
  return true;
}

bool VertexTool::doDeactivate()
{
  VertexToolBase::doDeactivate();

  auto& map = kdl::mem_lock(m_document)->map();

  map.edgeHandles().clear();
  map.faceHandles().clear();
  return true;
}

void VertexTool::addHandles(const std::vector<mdl::Node*>& nodes)
{
  auto& map = kdl::mem_lock(m_document)->map();

  VertexToolBase::addHandles(nodes, map.vertexHandles());
  VertexToolBase::addHandles(nodes, map.edgeHandles());
  VertexToolBase::addHandles(nodes, map.faceHandles());
}

void VertexTool::removeHandles(const std::vector<mdl::Node*>& nodes)
{
  auto& map = kdl::mem_lock(m_document)->map();

  VertexToolBase::removeHandles(nodes, map.vertexHandles());
  VertexToolBase::removeHandles(nodes, map.edgeHandles());
  VertexToolBase::removeHandles(nodes, map.faceHandles());
}

void VertexTool::addHandles(mdl::BrushVertexCommandBase* command)
{
  auto& map = kdl::mem_lock(m_document)->map();

  command->addHandles(map.vertexHandles());
  command->addHandles(map.edgeHandles());
  command->addHandles(map.faceHandles());
}

void VertexTool::removeHandles(mdl::BrushVertexCommandBase* command)
{
  auto& map = kdl::mem_lock(m_document)->map();

  command->removeHandles(map.vertexHandles());
  command->removeHandles(map.edgeHandles());
  command->removeHandles(map.faceHandles());
}

void VertexTool::resetModeAfterDeselection()
{
  auto& map = kdl::mem_lock(m_document)->map();
  if (!map.vertexHandles().anySelected())
  {
    m_mode = Mode::Move;
  }
}

} // namespace tb::ui
