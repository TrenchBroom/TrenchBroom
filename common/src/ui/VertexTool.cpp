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
  auto document = kdl::mem_lock(m_document);
  return findIncidentBrushes(document->vertexHandles(), handle);
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::segment3d& handle) const
{
  auto document = kdl::mem_lock(m_document);
  return findIncidentBrushes(document->edgeHandles(), handle);
}

std::vector<mdl::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::polygon3d& handle) const
{
  auto document = kdl::mem_lock(m_document);
  return findIncidentBrushes(document->faceHandles(), handle);
}

void VertexTool::pick(
  const vm::ray3d& pickRay,
  const render::Camera& camera,
  mdl::PickResult& pickResult) const
{
  auto document = kdl::mem_lock(m_document);
  const auto& grid = document->grid();

  document->vertexHandles().pick(pickRay, camera, pickResult);
  document->edgeHandles().pickGridHandle(pickRay, camera, grid, pickResult);
  document->faceHandles().pickGridHandle(pickRay, camera, grid, pickResult);
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
  auto document = kdl::mem_lock(m_document);
  return document->vertexHandles();
}

const VertexHandleManager& VertexTool::handleManager() const
{
  auto document = kdl::mem_lock(m_document);
  return document->vertexHandles();
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
    auto document = kdl::mem_lock(m_document);

    document->vertexHandles().deselectAll();
    if (hit.hasType(EdgeHandleManager::HandleHitType))
    {
      const auto& handle = std::get<0>(hit.target<const EdgeHandleManager::HitType&>());
      document->edgeHandles().select(handle);
      m_mode = Mode::SplitEdge;
    }
    else
    {
      const auto& handle = std::get<0>(hit.target<const FaceHandleManager::HitType&>());
      document->faceHandles().select(handle);
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
  const auto transform = vm::translation_matrix(delta);

  if (m_mode == Mode::Move)
  {
    auto handles = document->vertexHandles().selectedHandles();
    const auto result = document->transformVertices(std::move(handles), transform);
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
    if (document->edgeHandles().selectedHandleCount() == 1)
    {
      const auto handle = document->edgeHandles().selectedHandles().front();
      brushes = findIncidentBrushes(handle);
    }
  }
  else
  {
    assert(m_mode == Mode::SplitFace);
    if (document->faceHandles().selectedHandleCount() == 1)
    {
      const auto handle = document->faceHandles().selectedHandles().front();
      brushes = findIncidentBrushes(handle);
    }
  }

  if (!brushes.empty())
  {
    const auto newVertexPosition = transform * m_dragHandlePosition;
    if (document->addVertex(newVertexPosition))
    {
      m_mode = Mode::Move;
      document->edgeHandles().deselectAll();
      document->faceHandles().deselectAll();
      m_dragHandlePosition = transform * m_dragHandlePosition;
      document->vertexHandles().select(m_dragHandlePosition);
    }
    return MoveResult::Continue;
  }

  // Catch all failure cases: no brushes were selected or vertices could not be added:
  return MoveResult::Deny;
}

void VertexTool::endMove()
{
  auto document = kdl::mem_lock(m_document);

  VertexToolBase::endMove();
  document->edgeHandles().deselectAll();
  document->faceHandles().deselectAll();
  m_mode = Mode::Move;
}
void VertexTool::cancelMove()
{
  auto document = kdl::mem_lock(m_document);

  VertexToolBase::cancelMove();
  document->edgeHandles().deselectAll();
  document->faceHandles().deselectAll();
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
  auto document = kdl::mem_lock(m_document);

  switch (m_mode)
  {
  case Mode::Move:
    return kdl::str_plural(
      document->vertexHandles().selectedHandleCount(), "Move Vertex", "Move Vertices");
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

  auto document = kdl::mem_lock(m_document);
  auto handles = document->vertexHandles().selectedHandles();

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

  auto document = kdl::mem_lock(m_document);

  document->edgeHandles().clear();
  document->faceHandles().clear();

  const auto& brushes = selectedBrushes();
  document->edgeHandles().addHandles(brushes);
  document->faceHandles().addHandles(brushes);

  m_mode = Mode::Move;
  return true;
}

bool VertexTool::doDeactivate()
{
  VertexToolBase::doDeactivate();

  auto document = kdl::mem_lock(m_document);

  document->edgeHandles().clear();
  document->faceHandles().clear();
  return true;
}

void VertexTool::addHandles(const std::vector<mdl::Node*>& nodes)
{
  auto document = kdl::mem_lock(m_document);

  VertexToolBase::addHandles(nodes, document->vertexHandles());
  VertexToolBase::addHandles(nodes, document->edgeHandles());
  VertexToolBase::addHandles(nodes, document->faceHandles());
}

void VertexTool::removeHandles(const std::vector<mdl::Node*>& nodes)
{
  auto document = kdl::mem_lock(m_document);

  VertexToolBase::removeHandles(nodes, document->vertexHandles());
  VertexToolBase::removeHandles(nodes, document->edgeHandles());
  VertexToolBase::removeHandles(nodes, document->faceHandles());
}

void VertexTool::addHandles(BrushVertexCommandBase* command)
{
  auto document = kdl::mem_lock(m_document);

  command->addHandles(document->vertexHandles());
  command->addHandles(document->edgeHandles());
  command->addHandles(document->faceHandles());
}

void VertexTool::removeHandles(BrushVertexCommandBase* command)
{
  auto document = kdl::mem_lock(m_document);

  command->removeHandles(document->vertexHandles());
  command->removeHandles(document->edgeHandles());
  command->removeHandles(document->faceHandles());
}

void VertexTool::resetModeAfterDeselection()
{
  auto document = kdl::mem_lock(m_document);
  if (!document->vertexHandles().anySelected())
  {
    m_mode = Mode::Move;
  }
}

} // namespace tb::ui
