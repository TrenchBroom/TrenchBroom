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

#include "SetLockStateCommand.h"

#include "Macros.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LockState.h"
#include "mdl/Node.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "kdl/overload.h"

#include <algorithm>
#include <string>

namespace tb::mdl
{
namespace
{

auto setLockState(
  const std::vector<Node*>& nodes, const LockState lockState, ui::MapDocument& document)
{
  auto result = std::vector<std::tuple<Node*, LockState>>{};
  result.reserve(nodes.size());

  auto changedNodes = std::vector<Node*>{};
  changedNodes.reserve(nodes.size());

  for (Node* node : nodes)
  {
    const auto oldState = node->lockState();
    if (node->setLockState(lockState))
    {
      changedNodes.push_back(node);
      result.emplace_back(node, oldState);
    }
  }

  document.nodeLockingDidChangeNotifier(changedNodes);

  return result;
}

void restoreLockState(
  const std::vector<std::tuple<Node*, LockState>>& nodes, ui::MapDocument& document)
{
  auto changedNodes = std::vector<Node*>{};
  changedNodes.reserve(nodes.size());

  for (const auto& [node, state] : nodes)
  {
    if (node->setLockState(state))
    {
      changedNodes.push_back(node);
    }
  }

  document.nodeLockingDidChangeNotifier(changedNodes);
}

} // namespace

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::lock(std::vector<Node*> nodes)
{
  return std::make_unique<SetLockStateCommand>(nodes, LockState::Locked);
}

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::unlock(std::vector<Node*> nodes)
{
  return std::make_unique<SetLockStateCommand>(nodes, LockState::Unlocked);
}

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::reset(std::vector<Node*> nodes)
{
  return std::make_unique<SetLockStateCommand>(nodes, LockState::Inherited);
}

static bool shouldUpdateModificationCount(const std::vector<Node*>& nodes)
{
  return std::ranges::any_of(nodes, [](const auto* node) {
    return node->accept(kdl::overload(
      [](const WorldNode*) { return false; },
      [](const LayerNode*) { return true; },
      [](const GroupNode*) { return false; },
      [](const EntityNode*) { return false; },
      [](const BrushNode*) { return false; },
      [](const PatchNode*) { return false; }));
  });
}

SetLockStateCommand::SetLockStateCommand(
  std::vector<Node*> nodes, const LockState lockState)
  : UndoableCommand(makeName(lockState), shouldUpdateModificationCount(nodes))
  , m_nodes{std::move(nodes)}
  , m_lockState{lockState}
{
}

std::string SetLockStateCommand::makeName(const LockState state)
{
  switch (state)
  {
  case LockState::Inherited:
    return "Reset Locking";
  case LockState::Locked:
    return "Lock Objects";
  case LockState::Unlocked:
    return "Unlock Objects";
    switchDefault();
  }
}

std::unique_ptr<CommandResult> SetLockStateCommand::doPerformDo(ui::MapDocument& document)
{
  m_oldLockState = setLockState(m_nodes, m_lockState, document);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetLockStateCommand::doPerformUndo(
  ui::MapDocument& document)
{
  restoreLockState(m_oldLockState, document);
  return std::make_unique<CommandResult>(true);
}

} // namespace tb::mdl
