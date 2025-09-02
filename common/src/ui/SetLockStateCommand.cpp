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

namespace tb::ui
{
namespace
{
auto setLockState(
  const std::vector<mdl::Node*>& nodes,
  const mdl::LockState lockState,
  MapDocument& document)
{
  auto result = std::vector<std::tuple<mdl::Node*, mdl::LockState>>{};
  result.reserve(nodes.size());

  auto changedNodes = std::vector<mdl::Node*>{};
  changedNodes.reserve(nodes.size());

  for (mdl::Node* node : nodes)
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
  const std::vector<std::tuple<mdl::Node*, mdl::LockState>>& nodes, MapDocument& document)
{
  auto changedNodes = std::vector<mdl::Node*>{};
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

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::lock(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SetLockStateCommand>(nodes, mdl::LockState::Locked);
}

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::unlock(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SetLockStateCommand>(nodes, mdl::LockState::Unlocked);
}

std::unique_ptr<SetLockStateCommand> SetLockStateCommand::reset(
  std::vector<mdl::Node*> nodes)
{
  return std::make_unique<SetLockStateCommand>(nodes, mdl::LockState::Inherited);
}

static bool shouldUpdateModificationCount(const std::vector<mdl::Node*>& nodes)
{
  return std::ranges::any_of(nodes, [](const auto* node) {
    return node->accept(kdl::overload(
      [](const mdl::WorldNode*) { return false; },
      [](const mdl::LayerNode*) { return true; },
      [](const mdl::GroupNode*) { return false; },
      [](const mdl::EntityNode*) { return false; },
      [](const mdl::BrushNode*) { return false; },
      [](const mdl::PatchNode*) { return false; }));
  });
}

SetLockStateCommand::SetLockStateCommand(
  std::vector<mdl::Node*> nodes, const mdl::LockState lockState)
  : UndoableCommand(makeName(lockState), shouldUpdateModificationCount(nodes))
  , m_nodes{std::move(nodes)}
  , m_lockState{lockState}
{
}

std::string SetLockStateCommand::makeName(const mdl::LockState state)
{
  switch (state)
  {
  case mdl::LockState::Inherited:
    return "Reset Locking";
  case mdl::LockState::Locked:
    return "Lock Objects";
  case mdl::LockState::Unlocked:
    return "Unlock Objects";
    switchDefault();
  }
}

std::unique_ptr<CommandResult> SetLockStateCommand::doPerformDo(MapDocument& document)
{
  m_oldLockState = setLockState(m_nodes, m_lockState, document);
  return std::make_unique<CommandResult>(true);
}

std::unique_ptr<CommandResult> SetLockStateCommand::doPerformUndo(MapDocument& document)
{
  restoreLockState(m_oldLockState, document);
  return std::make_unique<CommandResult>(true);
}

} // namespace tb::ui
