/*
 Copyright (C) 2010-2017 Kristian Duske

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
#include "Model/BrushNode.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/RenderBatch.h"
#include "View/BrushVertexCommands.h"
#include "View/Grid.h"
#include "View/MapDocument.h"

#include <kdl/string_format.h>

#include <vecmath/polygon.h>

#include <cassert>
#include <tuple>
#include <vector>

namespace TrenchBroom
{
namespace View
{
VertexTool::VertexTool(const std::weak_ptr<MapDocument>& document)
  : VertexToolBase(document)
  , m_mode(Mode::Move)
  , m_vertexHandles(std::make_unique<VertexHandleManager>())
  , m_edgeHandles(std::make_unique<EdgeHandleManager>())
  , m_faceHandles(std::make_unique<FaceHandleManager>())
  , m_guideRenderer(document)
{
}

std::vector<Model::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::vec3& handle) const
{
  return findIncidentBrushes(*m_vertexHandles, handle);
}

std::vector<Model::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::segment3& handle) const
{
  return findIncidentBrushes(*m_edgeHandles, handle);
}

std::vector<Model::BrushNode*> VertexTool::findIncidentBrushes(
  const vm::polygon3& handle) const
{
  return findIncidentBrushes(*m_faceHandles, handle);
}

void VertexTool::pick(
  const vm::ray3& pickRay,
  const Renderer::Camera& camera,
  Model::PickResult& pickResult) const
{
  auto document = kdl::mem_lock(m_document);
  const Grid& grid = document->grid();

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

std::tuple<vm::vec3, vm::vec3> VertexTool::handlePositionAndHitPoint(
  const std::vector<Model::Hit>& hits) const
{
  assert(!hits.empty());

  const auto& hit = hits.front();
  assert(hit.hasType(
    VertexHandleManager::HandleHitType | EdgeHandleManager::HandleHitType
    | FaceHandleManager::HandleHitType));

  const auto position = hit.hasType(VertexHandleManager::HandleHitType)
                          ? hit.target<vm::vec3>()
                        : hit.hasType(EdgeHandleManager::HandleHitType)
                          ? std::get<1>(hit.target<EdgeHandleManager::HitType>())
                          : std::get<1>(hit.target<FaceHandleManager::HitType>());

  return {position, hit.hitPoint()};
}

bool VertexTool::startMove(const std::vector<Model::Hit>& hits)
{
  const auto& hit = hits.front();
  if (hit.hasType(EdgeHandleManager::HandleHitType | FaceHandleManager::HandleHitType))
  {
    m_vertexHandles->deselectAll();
    if (hit.hasType(EdgeHandleManager::HandleHitType))
    {
      const vm::segment3& handle =
        std::get<0>(hit.target<const EdgeHandleManager::HitType&>());
      m_edgeHandles->select(handle);
      m_mode = Mode::SplitEdge;
    }
    else
    {
      const vm::polygon3& handle =
        std::get<0>(hit.target<const FaceHandleManager::HitType&>());
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
  else
  {
    return true;
  }
}

VertexTool::MoveResult VertexTool::move(const vm::vec3& delta)
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
      else
      {
        m_dragHandlePosition = m_dragHandlePosition + delta;
        return MoveResult::Continue;
      }
    }
    else
    {
      return MoveResult::Deny;
    }
  }
  else
  {
    std::vector<Model::BrushNode*> brushes;
    if (m_mode == Mode::SplitEdge)
    {
      if (m_edgeHandles->selectedHandleCount() == 1)
      {
        const vm::segment3 handle = m_edgeHandles->selectedHandles().front();
        brushes = findIncidentBrushes(handle);
      }
    }
    else
    {
      assert(m_mode == Mode::SplitFace);
      if (m_faceHandles->selectedHandleCount() == 1)
      {
        const vm::polygon3 handle = m_faceHandles->selectedHandles().front();
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

vm::vec3 VertexTool::getHandlePosition(const Model::Hit& hit) const
{
  assert(hit.isMatch());
  assert(hit.hasType(
    VertexHandleManager::HandleHitType | EdgeHandleManager::HandleHitType
    | FaceHandleManager::HandleHitType));

  if (hit.hasType(VertexHandleManager::HandleHitType))
  {
    return hit.target<vm::vec3>();
  }
  else if (hit.hasType(EdgeHandleManager::HandleHitType))
  {
    return std::get<1>(hit.target<EdgeHandleManager::HitType>());
  }
  else
  {
    return std::get<1>(hit.target<FaceHandleManager::HitType>());
  }
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
  Renderer::RenderContext&,
  Renderer::RenderBatch& renderBatch,
  const vm::vec3& position) const
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

  const std::vector<Model::BrushNode*>& brushes = selectedBrushes();
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

void VertexTool::addHandles(const std::vector<Model::Node*>& nodes)
{
  VertexToolBase::addHandles(nodes, *m_vertexHandles);
  VertexToolBase::addHandles(nodes, *m_edgeHandles);
  VertexToolBase::addHandles(nodes, *m_faceHandles);
}

void VertexTool::removeHandles(const std::vector<Model::Node*>& nodes)
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
} // namespace View
} // namespace TrenchBroom
