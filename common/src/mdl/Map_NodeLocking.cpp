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

#include "mdl/Map_NodeLocking.h"

#include "Map.h"
#include "mdl/BrushFaceHandle.h" // IWYU pragma: keep
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "mdl/SetLockStateCommand.h"
#include "mdl/Transaction.h"

#include "kdl/range_to_vector.h"

#include <ranges>

namespace tb::mdl
{

void lockNodes(Map& map, const std::vector<Node*>& nodes)
{
  auto transaction = Transaction{map, "Lock Objects"};

  // Deselect any selected nodes or faces inside `nodes`
  deselectNodes(map, collectSelectedNodes(nodes));
  deselectBrushFaces(map, collectSelectedBrushFaces(nodes));

  // Reset lock state of any forced unlocked children of `nodes`
  downgradeUnlockedToInherit(map, collectDescendants(nodes));

  map.executeAndStore(SetLockStateCommand::lock(nodes));
  transaction.commit();
}

void unlockNodes(Map& map, const std::vector<Node*>& nodes)
{
  map.executeAndStore(SetLockStateCommand::unlock(nodes));
}

void ensureNodesUnlocked(Map& map, const std::vector<Node*>& nodes)
{
  const auto nodesToUnlock =
    nodes | std::views::filter([](auto* node) { return node->locked(); })
    | kdl::to_vector;
  unlockNodes(map, nodesToUnlock);
}

void resetNodeLockingState(Map& map, const std::vector<Node*>& nodes)
{
  map.executeAndStore(SetLockStateCommand::reset(nodes));
}

void downgradeUnlockedToInherit(Map& map, const std::vector<Node*>& nodes)
{
  const auto nodesToReset = nodes | std::views::filter([](auto* node) {
                              return node->lockState() == LockState::Unlocked;
                            })
                            | kdl::to_vector;
  resetNodeLockingState(map, nodesToReset);
}

} // namespace tb::mdl
