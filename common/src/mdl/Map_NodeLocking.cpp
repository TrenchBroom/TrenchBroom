/*
 Copyright (C) 2025 Kristian Duske

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

#include "Map.h"
#include "mdl/BrushFaceHandle.h" // IWYU pragma: keep
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/SetLockStateCommand.h"
#include "mdl/Transaction.h"

#include <algorithm>

namespace tb::mdl
{

void Map::lockNodes(const std::vector<Node*>& nodes)
{
  auto transaction = Transaction{*this, "Lock Objects"};

  // Deselect any selected nodes or faces inside `nodes`
  deselectNodes(collectSelectedNodes(nodes));
  deselectBrushFaces(collectSelectedBrushFaces(nodes));

  // Reset lock state of any forced unlocked children of `nodes`
  downgradeUnlockedToInherit(collectDescendants(nodes));

  executeAndStore(SetLockStateCommand::lock(nodes));
  transaction.commit();
}

void Map::unlockNodes(const std::vector<Node*>& nodes)
{
  executeAndStore(SetLockStateCommand::unlock(nodes));
}

void Map::ensureNodesUnlocked(const std::vector<Node*>& nodes)
{
  auto nodesToUnlock = std::vector<Node*>{};
  std::ranges::copy_if(
    nodes, std::back_inserter(nodesToUnlock), [](auto* node) { return node->locked(); });
  unlockNodes(nodesToUnlock);
}

void Map::resetNodeLockingState(const std::vector<Node*>& nodes)
{
  executeAndStore(SetLockStateCommand::reset(nodes));
}

void Map::downgradeUnlockedToInherit(const std::vector<Node*>& nodes)
{
  auto nodesToReset = std::vector<Node*>{};
  std::ranges::copy_if(nodes, std::back_inserter(nodesToReset), [](auto* node) {
    return node->lockState() == LockState::Unlocked;
  });
  resetNodeLockingState(nodesToReset);
}

} // namespace tb::mdl
