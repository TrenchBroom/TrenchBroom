/*
 Copyright (C) 2020 Kristian Duske

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

#include "BrushVertexCommands.h"

#include "ui/SwapNodeContentsCommand.h"
#include "ui/VertexTool.h"

#include "kdl/range_to_vector.h"

#include <ranges>

namespace tb::ui
{

BrushVertexCommandBase::BrushVertexCommandBase(
  std::string name, std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes)
  : SwapNodeContentsCommand{std::move(name), std::move(nodes)}
{
}

std::unique_ptr<CommandResult> BrushVertexCommandBase::doPerformDo(
  MapDocumentCommandFacade& document)
{
  return createCommandResult(SwapNodeContentsCommand::doPerformDo(document));
}

std::unique_ptr<CommandResult> BrushVertexCommandBase::createCommandResult(
  std::unique_ptr<CommandResult> swapResult)
{
  return swapResult;
}

static auto collectBrushNodes(
  const std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes)
{
  return nodes | std::views::filter([](const auto& pair) {
           return dynamic_cast<mdl::BrushNode*>(pair.first) != nullptr;
         })
         | std::views::transform(
           [](const auto& pair) { return static_cast<mdl::BrushNode*>(pair.first); })
         | kdl::to_vector;
}

void BrushVertexCommandBase::removeHandles(VertexHandleManagerBase& manager)
{
  const auto nodes = collectBrushNodes(m_nodes);
  manager.removeHandles(nodes);
}

void BrushVertexCommandBase::addHandles(VertexHandleManagerBase& manager)
{
  const auto nodes = collectBrushNodes(m_nodes);
  manager.addHandles(nodes);
}

void BrushVertexCommandBase::selectNewHandlePositions(
  VertexHandleManagerBaseT<vm::vec3d>&) const
{
}
void BrushVertexCommandBase::selectOldHandlePositions(
  VertexHandleManagerBaseT<vm::vec3d>&) const
{
}
void BrushVertexCommandBase::selectNewHandlePositions(
  VertexHandleManagerBaseT<vm::segment3d>&) const
{
}
void BrushVertexCommandBase::selectOldHandlePositions(
  VertexHandleManagerBaseT<vm::segment3d>&) const
{
}
void BrushVertexCommandBase::selectNewHandlePositions(
  VertexHandleManagerBaseT<vm::polygon3d>&) const
{
}
void BrushVertexCommandBase::selectOldHandlePositions(
  VertexHandleManagerBaseT<vm::polygon3d>&) const
{
}

BrushVertexCommandResult::BrushVertexCommandResult(
  const bool success, const bool hasRemainingVertices)
  : CommandResult{success}
  , m_hasRemainingVertices{hasRemainingVertices}
{
}

bool BrushVertexCommandResult::hasRemainingVertices() const
{
  return m_hasRemainingVertices;
}

BrushVertexCommand::BrushVertexCommand(
  std::string name,
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes,
  std::vector<vm::vec3d> oldVertexPositions,
  std::vector<vm::vec3d> newVertexPositions)
  : BrushVertexCommandBase{std::move(name), std::move(nodes)}
  , m_oldVertexPositions{std::move(oldVertexPositions)}
  , m_newVertexPositions{std::move(newVertexPositions)}
{
}

std::unique_ptr<CommandResult> BrushVertexCommand::createCommandResult(
  std::unique_ptr<CommandResult> swapResult)
{
  return std::make_unique<BrushVertexCommandResult>(
    swapResult->success(), !m_newVertexPositions.empty());
}

bool BrushVertexCommand::doCollateWith(UndoableCommand& command)
{
  if (auto* other = dynamic_cast<BrushVertexCommand*>(&command);
      other && m_newVertexPositions == other->m_oldVertexPositions
      && SwapNodeContentsCommand::doCollateWith(command))
  {
    m_newVertexPositions = std::move(other->m_newVertexPositions);
    return true;
  }

  return false;
}

void BrushVertexCommand::selectNewHandlePositions(
  VertexHandleManagerBaseT<vm::vec3d>& manager) const
{
  manager.select(m_newVertexPositions);
}

void BrushVertexCommand::selectOldHandlePositions(
  VertexHandleManagerBaseT<vm::vec3d>& manager) const
{
  manager.select(m_oldVertexPositions);
}

BrushEdgeCommand::BrushEdgeCommand(
  std::string name,
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes,
  std::vector<vm::segment3d> oldEdgePositions,
  std::vector<vm::segment3d> newEdgePositions)
  : BrushVertexCommandBase{std::move(name), std::move(nodes)}
  , m_oldEdgePositions{std::move(oldEdgePositions)}
  , m_newEdgePositions{std::move(newEdgePositions)}
{
}

bool BrushEdgeCommand::doCollateWith(UndoableCommand& command)
{
  if (auto* other = dynamic_cast<BrushEdgeCommand*>(&command);
      other && m_newEdgePositions == other->m_oldEdgePositions
      && SwapNodeContentsCommand::doCollateWith(command))
  {
    m_newEdgePositions = std::move(other->m_newEdgePositions);
    return true;
  }

  return false;
}

void BrushEdgeCommand::selectNewHandlePositions(
  VertexHandleManagerBaseT<vm::segment3d>& manager) const
{
  manager.select(m_newEdgePositions);
}

void BrushEdgeCommand::selectOldHandlePositions(
  VertexHandleManagerBaseT<vm::segment3d>& manager) const
{
  manager.select(m_oldEdgePositions);
}

BrushFaceCommand::BrushFaceCommand(
  std::string name,
  std::vector<std::pair<mdl::Node*, mdl::NodeContents>> nodes,
  std::vector<vm::polygon3d> oldFacePositions,
  std::vector<vm::polygon3d> newFacePositions)
  : BrushVertexCommandBase{std::move(name), std::move(nodes)}
  , m_oldFacePositions{std::move(oldFacePositions)}
  , m_newFacePositions{std::move(newFacePositions)}
{
}

bool BrushFaceCommand::doCollateWith(UndoableCommand& command)
{
  if (auto* other = dynamic_cast<BrushFaceCommand*>(&command);
      other && m_newFacePositions == other->m_oldFacePositions
      && SwapNodeContentsCommand::doCollateWith(command))
  {
    m_newFacePositions = std::move(other->m_newFacePositions);
    return true;
  }
  return false;
}

void BrushFaceCommand::selectNewHandlePositions(
  VertexHandleManagerBaseT<vm::polygon3d>& manager) const
{
  manager.select(m_newFacePositions);
}

void BrushFaceCommand::selectOldHandlePositions(
  VertexHandleManagerBaseT<vm::polygon3d>& manager) const
{
  manager.select(m_oldFacePositions);
}

} // namespace tb::ui
