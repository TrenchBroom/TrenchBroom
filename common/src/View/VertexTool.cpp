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
#include "View/BrushVertexCommands.h"
#include "View/MapDocument.h"
#include "mdl/BrushNode.h"
#include "render/RenderBatch.h"

#include "kdl/string_format.h"

#include "vm/polygon.h"

#include <cassert>
#include <tuple>
#include <vector>

namespace tb::View
{

VertexTool::VertexTool(std::weak_ptr<MapDocument> document)
  : VertexToolBase{std::move(document)}
  , m_mode{Mode::Move}
  , m_vertexHandles{std::make_unique<VertexHandleManager>()}
  , m_edgeHandles{std::make_unique<EdgeHandleManager>()}
  , m_faceHandles{std::make_unique<FaceHandleManager>()}
  , m_guideRenderer{m_document}
{
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::vec3d& handle) const
{
  return findIncidentBrushes(*m_vertexHandles, handle);
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::segment3d& handle) const
{
  return findIncidentBrushes(*m_edgeHandles, handle);
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::polygon3d& handle) const
{
  return findIncidentBrushes(*m_faceHandles, handle);
}

void VertexTool::pick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();

  m_vertexHandles->pick(pickRay, camera, pickResult);
  m_edgeHandles->pickGridHandle(pickRay, camera, grid, pickResult);
  m_faceHandles->pickGridHandle(pickRay, camera, grid, pickResult);
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

VertexHandleManager& VertexTool::handleManager()
{
  return *m_vertexHandles;
}

const VertexHandleManager& VertexTool::handleManager() const
{
  return *m_vertexHandles;
}

std::tuple<vm::vec3d, vm::vec3d> VertexTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  assert(!hits.empty());

  const auto& hit = hits.front();
  assert(hit.hasType(
    VertexHandleManager::HandleHitType | EdgeHandleManager::HandleHitType
    | FaceHandleManager::HandleHitType));

  const auto position = hit.hasType(VertexHandleManager::HandleHitType)
                          ? hit.target<vm::vec3d>()
                        : hit.hasType(EdgeHandleManager::HandleHitType)
                          ? std::get<1>(hit.target<EdgeHandleManager::HitType>())
                          : std::get<1>(hit.target<FaceHandleManager::HitType>());

  return {position, hit.hitPoint()};
}

bool VertexTool::startMove(const std::vector<mdl::Hit>& hits)
{
  const auto& hit = hits.front();
  if (hit.hasType(EdgeHandleManager::HandleHitType | FaceHandleManager::HandleHitType))
  {
    m_vertexHandles->deselectAll();
    if (hit.hasType(EdgeHandleManager::HandleHitType))
    {
      const auto& handle = std::get<0>(hit.target<const EdgeHandleManager::HitType&>());
      m_edgeHandles->select(handle);
      m_mode = Mode::SplitEdge;
    }
    else
    {
      const auto& handle = std::get<0>(hit.target<const FaceHandleManager::HitType&>());
      m_faceHandles->select(handle);
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
  auto document = kdl::mem_lock(m_document);

  if (m_mode == Mode::Move)
  {
    auto handles = m_vertexHandles->selectedHandles();
    const auto result = document->moveVertices(std::move(handles), delta);
    if (result.success)
    {
      if (!result.hasRemainingVertices)
      {
        return MoveResult::Cancel;
      }
      m_dragHandlePosition = m_dragHandlePosition + delta;
      return MoveResult::Continue;
    }
    return MoveResult::Deny;
  }

  auto brushes = std::vector<mdl::BrushNode*>{};
  if (m_mode == Mode::SplitEdge)
  {
    if (m_edgeHandles->selectedHandleCount() == 1)
    {
      const vm::segment3d handle = m_edgeHandles->selectedHandles().front();
      brushes = findIncidentBrushes(handle);
    }
  }
  else
  {
    assert(m_mode == Mode::SplitFace);
    if (m_faceHandles->selectedHandleCount() == 1)
    {
      const vm::polygon3d handle = m_faceHandles->selectedHandles().front();
      brushes = findIncidentBrushes(handle);
    }
  }

  if (!brushes.empty())
  {
    const auto newVertexPosition = m_dragHandlePosition + delta;
    if (document->addVertex(newVertexPosition))
    {
      m_mode = Mode::Move;
      m_edgeHandles->deselectAll();
      m_faceHandles->deselectAll();
      m_dragHandlePosition = m_dragHandlePosition + delta;
      m_vertexHandles->select(m_dragHandlePosition);
    }
    return MoveResult::Continue;
  }

  // Catch all failure cases: no brushes were selected or vertices could not be added:
  return MoveResult::Deny;
}

void VertexTool::endMove()
{
  VertexToolBase::endMove();
  m_edgeHandles->deselectAll();
  m_faceHandles->deselectAll();
  m_mode = Mode::Move;
}
void VertexTool::cancelMove()
{
  VertexToolBase::cancelMove();
  m_edgeHandles->deselectAll();
  m_faceHandles->deselectAll();
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
    VertexHandleManager::HandleHitType | EdgeHandleManager::HandleHitType
    | FaceHandleManager::HandleHitType));

  return hit.hasType(VertexHandleManager::HandleHitType) ? hit.target<vm::vec3d>()
         : hit.hasType(EdgeHandleManager::HandleHitType)
           ? std::get<1>(hit.target<EdgeHandleManager::HitType>())
           : std::get<1>(hit.target<FaceHandleManager::HitType>());
}

std::string VertexTool::actionName() const
{
  switch (m_mode)
  {
  case Mode::Move:
    return kdl::str_plural(
      m_vertexHandles->selectedHandleCount(), "Move Vertex", "Move Vertices");
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

  auto handles = m_vertexHandles->selectedHandles();
  const auto commandName =
    kdl::str_plural(handles.size(), "Remove Brush Vertex", "Remove Brush Vertices");
  kdl::mem_lock(m_document)->removeVertices(commandName, std::move(handles));
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

  m_edgeHandles->clear();
  m_faceHandles->clear();

  const auto& brushes = selectedBrushes();
  m_edgeHandles->addHandles(std::begin(brushes), std::end(brushes));
  m_faceHandles->addHandles(std::begin(brushes), std::end(brushes));

  m_mode = Mode::Move;
  return true;
}

bool VertexTool::doDeactivate()
{
  VertexToolBase::doDeactivate();

  m_edgeHandles->clear();
  m_faceHandles->clear();
  return true;
}

void VertexTool::addHandles(const std::vector<mdl::Node*>& nodes)
{
  VertexToolBase::addHandles(nodes, *m_vertexHandles);
  VertexToolBase::addHandles(nodes, *m_edgeHandles);
  VertexToolBase::addHandles(nodes, *m_faceHandles);
}

void VertexTool::removeHandles(const std::vector<mdl::Node*>& nodes)
{
  VertexToolBase::removeHandles(nodes, *m_vertexHandles);
  VertexToolBase::removeHandles(nodes, *m_edgeHandles);
  VertexToolBase::removeHandles(nodes, *m_faceHandles);
}

void VertexTool::addHandles(BrushVertexCommandBase* command)
{
  command->addHandles(*m_vertexHandles);
  command->addHandles(*m_edgeHandles);
  command->addHandles(*m_faceHandles);
}

void VertexTool::removeHandles(BrushVertexCommandBase* command)
{
  command->removeHandles(*m_vertexHandles);
  command->removeHandles(*m_edgeHandles);
  command->removeHandles(*m_faceHandles);
}

void VertexTool::resetModeAfterDeselection()
{
  if (!m_vertexHandles->anySelected())
  {
    m_mode = Mode::Move;
  }
}

} // namespace tb::View
