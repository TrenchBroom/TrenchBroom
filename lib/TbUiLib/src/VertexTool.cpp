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

#include "ui/VertexTool.h"

#include "Preferences.h"
#include "base/Macros.h"
#include "base/PreferenceManager.h"
#include "mdl/BrushVertexCommands.h"
#include "mdl/HitFilter.h"
#include "mdl/Map_Geometry.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "render/RenderBatch.h"
#include "ui/MapDocument.h"

#include "kd/contracts.h"
#include "kd/string_format.h"

#include <tuple>
#include <vector>

namespace tb::ui
{

VertexTool::VertexTool(MapDocument& document)
  : BrushHandleToolBase{document}
  , m_mode{Mode::Move}
{
}

void VertexTool::pick(
  const vm::ray3d& pickRay,
  const gl::Camera& camera,
  const double handleRadius,
  mdl::PickResult& pickResult) const
{
  auto& map = m_document.map();
  const auto& grid = map.grid();

  map.nodeHandles().pick<mdl::VertexHandle>(
    pickResult, mdl::VertexHandle::HandleHitType, pickRay, camera, handleRadius);
  map.nodeHandles().pick<mdl::EdgeHandle>(
    pickResult, mdl::EdgeHandle::HandleHitType, pickRay, camera, handleRadius, grid);
  map.nodeHandles().pick<mdl::FaceHandle>(
    pickResult, mdl::FaceHandle::HandleHitType, pickRay, camera, handleRadius, grid);
}

bool VertexTool::deselectAll()
{
  if (NodeHandleToolBase::deselectAll())
  {
    resetModeAfterDeselection();
    return true;
  }
  return false;
}

mdl::Hit VertexTool::findDraggableHandle(
  const InputState& inputState, const mdl::HitType::Type hitType) const
{
  using namespace mdl::HitFilters;

  if (hitType != mdl::VertexHandle::HandleHitType)
  {
    return NodeHandleToolBase::findDraggableHandle(inputState, hitType);
  }

  if (const auto vertexHit = NodeHandleToolBase::findDraggableHandle(inputState, hitType);
      vertexHit.isMatch())
  {
    return vertexHit;
  }

  if (
    inputState.modifierKeysDown(ModifierKeys::Shift) && !inputState.pickResult().empty())
  {
    const auto& anyHit = inputState.pickResult().all().front();
    if (anyHit.hasType(mdl::EdgeHandle::HandleHitType | mdl::FaceHandle::HandleHitType))
    {
      return anyHit;
    }

    // If the top hit is not a split-handle hit (for example, a brush-face hit),
    // fall back to split handles and prefer faces over edges.
    if (const auto faceHit =
          inputState.pickResult().first(type(mdl::FaceHandle::HandleHitType));
        faceHit.isMatch())
    {
      return faceHit;
    }
    if (const auto edgeHit =
          inputState.pickResult().first(type(mdl::EdgeHandle::HandleHitType));
        edgeHit.isMatch())
    {
      return edgeHit;
    }
  }

  return mdl::Hit::NoHit;
}

std::vector<mdl::Hit> VertexTool::collectDraggableHandles(
  const InputState& inputState, const mdl::HitType::Type hitType) const
{
  using namespace mdl::HitFilters;

  if (hitType != mdl::VertexHandle::HandleHitType)
  {
    return NodeHandleToolBase::collectDraggableHandles(inputState, hitType);
  }

  if (const auto vertexHits =
        NodeHandleToolBase::collectDraggableHandles(inputState, hitType);
      !vertexHits.empty())
  {
    return vertexHits;
  }

  if (
    inputState.modifierKeysDown(ModifierKeys::Shift) && !inputState.pickResult().empty())
  {
    const auto& anyHit = inputState.pickResult().all().front();
    if (anyHit.hasType(mdl::EdgeHandle::HandleHitType))
    {
      if (const auto edgeHits =
            inputState.pickResult().all(type(mdl::EdgeHandle::HandleHitType));
          !edgeHits.empty())
      {
        return edgeHits;
      }
    }
    else if (anyHit.hasType(mdl::FaceHandle::HandleHitType))
    {
      if (const auto faceHits =
            inputState.pickResult().all(type(mdl::FaceHandle::HandleHitType));
          !faceHits.empty())
      {
        return faceHits;
      }
    }

    // If the top hit is not a split-handle hit, prefer face split handles
    // over edge split handles.
    if (const auto faceHits =
          inputState.pickResult().all(type(mdl::FaceHandle::HandleHitType));
        !faceHits.empty())
    {
      return faceHits;
    }
    if (const auto edgeHits =
          inputState.pickResult().all(type(mdl::EdgeHandle::HandleHitType));
        !edgeHits.empty())
    {
      return edgeHits;
    }
  }

  return {};
}

std::tuple<vm::vec3d, vm::vec3d> VertexTool::handlePositionAndHitPoint(
  const std::vector<mdl::Hit>& hits) const
{
  contract_pre(!hits.empty());

  const auto& hit = hits.front();
  contract_assert(hit.hasType(
    mdl::VertexHandle::HandleHitType | mdl::EdgeHandle::HandleHitType
    | mdl::FaceHandle::HandleHitType));

  const auto position = hit.hasType(mdl::VertexHandle::HandleHitType)
                          ? hit.target<mdl::VertexHandle>().position
                        : hit.hasType(mdl::EdgeHandle::HandleHitType)
                          ? std::get<1>(hit.target<mdl::EdgeHandle::GridHandleHitData>())
                          : std::get<1>(hit.target<mdl::FaceHandle::GridHandleHitData>());

  return {position, hit.hitPoint()};
}

bool VertexTool::startMove(const std::vector<mdl::Hit>& hits)
{
  auto& map = m_document.map();

  const auto& hit = hits.front();
  if (hit.hasType(mdl::EdgeHandle::HandleHitType | mdl::FaceHandle::HandleHitType))
  {
    map.nodeHandles().deselectAllHandles<mdl::VertexHandle>();
    if (hit.hasType(mdl::EdgeHandle::HandleHitType))
    {
      const auto handle = std::get<0>(hit.target<mdl::EdgeHandle::GridHandleHitData>());
      map.nodeHandles().selectHandle<mdl::EdgeHandle>(handle);
      m_mode = Mode::SplitEdge;
    }
    else
    {
      const auto handle = std::get<0>(hit.target<mdl::FaceHandle::GridHandleHitData>());
      map.nodeHandles().selectHandle<mdl::FaceHandle>(handle);
      m_mode = Mode::SplitFace;
    }
    refreshViews();
  }
  else
  {
    m_mode = Mode::Move;
  }

  if (!NodeHandleToolBase::startMove(hits))
  {
    m_mode = Mode::Move;
    return false;
  }
  return true;
}

VertexTool::MoveResult VertexTool::move(const vm::vec3d& delta)
{
  auto& map = m_document.map();

  const auto transform = vm::translation_matrix(delta);

  if (m_mode == Mode::Move)
  {
    const auto vertexPositions = mdl::VertexHandle::getPositions(
      map.nodeHandles().selectedHandles<mdl::VertexHandle>());
    const auto result = transformVertices(map, vertexPositions, transform);
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

  auto incidentNodes = std::vector<mdl::Node*>{};
  if (m_mode == Mode::SplitEdge)
  {
    if (map.nodeHandles().selectedHandleCount<mdl::EdgeHandle>() == 1)
    {
      incidentNodes =
        findIncidentNodes(map.nodeHandles().selectedHandles<mdl::EdgeHandle>().front());
    }
  }
  else
  {
    contract_assert(m_mode == Mode::SplitFace);
    if (map.nodeHandles().selectedHandleCount<mdl::FaceHandle>() == 1)
    {
      incidentNodes =
        findIncidentNodes(map.nodeHandles().selectedHandles<mdl::FaceHandle>().front());
    }
  }

  if (!incidentNodes.empty())
  {
    const auto newVertexPosition = transform * m_dragHandlePosition;
    if (addVertex(map, newVertexPosition))
    {
      m_mode = Mode::Move;
      map.nodeHandles().deselectAllHandles<mdl::EdgeHandle>();
      map.nodeHandles().deselectAllHandles<mdl::FaceHandle>();
      m_dragHandlePosition = transform * m_dragHandlePosition;
      map.nodeHandles().selectHandle<mdl::VertexHandle>(
        mdl::VertexHandle{m_dragHandlePosition});
    }
    return MoveResult::Continue;
  }

  // Catch all failure cases: no brushes were selected or vertices could not be added:
  return MoveResult::Deny;
}

void VertexTool::endMove()
{
  auto& map = m_document.map();

  NodeHandleToolBase::endMove();
  map.nodeHandles().deselectAllHandles<mdl::EdgeHandle>();
  map.nodeHandles().deselectAllHandles<mdl::FaceHandle>();
  m_mode = Mode::Move;
}
void VertexTool::cancelMove()
{
  auto& map = m_document.map();

  NodeHandleToolBase::cancelMove();
  map.nodeHandles().deselectAllHandles<mdl::EdgeHandle>();
  map.nodeHandles().deselectAllHandles<mdl::FaceHandle>();
  m_mode = Mode::Move;
}

bool VertexTool::allowAbsoluteSnapping() const
{
  return true;
}

vm::vec3d VertexTool::getHandlePosition(const mdl::Hit& hit) const
{
  contract_pre(hit.isMatch());
  contract_pre(hit.hasType(
    mdl::VertexHandle::HandleHitType | mdl::EdgeHandle::HandleHitType
    | mdl::FaceHandle::HandleHitType));

  return hit.hasType(mdl::VertexHandle::HandleHitType)
           ? hit.target<mdl::VertexHandle>().position
         : hit.hasType(mdl::EdgeHandle::HandleHitType)
           ? std::get<1>(hit.target<mdl::EdgeHandle::GridHandleHitData>())
           : std::get<1>(hit.target<mdl::FaceHandle::GridHandleHitData>());
}

std::string VertexTool::actionName() const
{
  switch (m_mode)
  {
  case Mode::Move:
    return kdl::str_plural(
      m_document.map().nodeHandles().selectedHandleCount<mdl::VertexHandle>(),
      "Move Vertex",
      "Move Vertices");
  case Mode::SplitEdge:
    return "Split Edge";
  case Mode::SplitFace:
    return "Split Face";
    switchDefault();
  }
}

void VertexTool::removeSelection()
{
  contract_pre(canRemoveSelection());

  auto& map = m_document.map();
  auto handlePositions = mdl::VertexHandle::getPositions(
    map.nodeHandles().selectedHandles<mdl::VertexHandle>());

  const auto commandName = kdl::str_plural(
    handlePositions.size(), "Remove Brush Vertex", "Remove Brush Vertices");
  removeVertices(map, commandName, std::move(handlePositions));
}

void VertexTool::renderGuide(
  render::RenderContext&,
  render::RenderBatch& renderBatch,
  const vm::vec3d& position) const
{
  m_guideRenderer.setPosition(position);
  m_guideRenderer.setColor(RgbaF{pref(Preferences::HandleColor).to<RgbF>(), 0.5f});
  renderBatch.add(&m_guideRenderer);
}

bool VertexTool::doActivate()
{
  NodeHandleToolBase::doActivate();

  const auto& nodes = selectedNodes();

  auto& map = m_document.map();
  map.nodeHandles().addHandles<mdl::EdgeHandle>(nodes);
  map.nodeHandles().addHandles<mdl::FaceHandle>(nodes);

  m_mode = Mode::Move;
  return true;
}

bool VertexTool::doDeactivate()
{
  NodeHandleToolBase::doDeactivate();

  auto& map = m_document.map();
  map.nodeHandles().clear<mdl::EdgeHandle>();
  map.nodeHandles().clear<mdl::FaceHandle>();
  return true;
}

void VertexTool::addHandles(const std::vector<mdl::Node*>& nodes)
{
  auto& map = m_document.map();

  map.nodeHandles().addHandles<mdl::VertexHandle>(nodes);
  map.nodeHandles().addHandles<mdl::EdgeHandle>(nodes);
  map.nodeHandles().addHandles<mdl::FaceHandle>(nodes);
}

void VertexTool::removeHandles(const std::vector<mdl::Node*>& nodes)
{
  auto& map = m_document.map();

  map.nodeHandles().removeHandles<mdl::VertexHandle>(nodes);
  map.nodeHandles().removeHandles<mdl::EdgeHandle>(nodes);
  map.nodeHandles().removeHandles<mdl::FaceHandle>(nodes);
}

void VertexTool::addHandles(mdl::BrushVertexCommand& command)
{
  auto& map = m_document.map();

  command.addHandles<mdl::EdgeHandle, mdl::FaceHandle>(map.nodeHandles());
}

void VertexTool::removeHandles(mdl::BrushVertexCommand& command)
{
  auto& map = m_document.map();

  command.removeHandles<mdl::EdgeHandle, mdl::FaceHandle>(map.nodeHandles());
}

void VertexTool::resetModeAfterDeselection()
{
  if (!m_document.map().nodeHandles().anyHandleSelected<mdl::VertexHandle>())
  {
    m_mode = Mode::Move;
  }
}

} // namespace tb::ui
