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

#include "mdl/AddRemoveNodesUtils.h"

#include "base/Notifier.h"
#include "mdl/Map.h"
#include "mdl/Node.h"

#include "kd/ranges/raw_pointers_view.h"
#include "kd/ranges/to.h"

#include <ranges>

namespace tb::mdl
{

kdl::flat_map<Node*, std::vector<Node*>> addNodesAndNotify(
  kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>> nodes, Map& map)
{
  auto addedNodes = kdl::flat_map<Node*, std::vector<Node*>>{};
  for (auto&& [parent, children] : nodes)
  {
    addedNodes[parent] =
      children | kdl::views::raw_pointers | kdl::ranges::to<std::vector>();
    parent->addChildren(std::move(children));
  }

  map.nodesWereAddedNotifier(
    addedNodes.values() | std::views::join | kdl::ranges::to<std::vector>());
  return addedNodes;
}

kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>> removeNodesAndNotify(
  const kdl::flat_map<Node*, std::vector<Node*>>& nodes, Map& map)
{
  const auto allChildren =
    nodes | std::views::values | std::views::join | kdl::ranges::to<std::vector>();

  auto notifyChildren = NotifyBeforeAndAfter{
    map.nodesWillBeRemovedNotifier, map.nodesWereRemovedNotifier, allChildren};

  auto removedNodes = kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>>{};
  for (const auto& [parent, children] : nodes)
  {
    removedNodes[parent] = parent->removeChildren(children);
  }
  return removedNodes;
}

} // namespace tb::mdl
