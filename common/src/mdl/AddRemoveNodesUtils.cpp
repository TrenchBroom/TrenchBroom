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

#include "AddRemoveNodesUtils.h"

#include "Notifier.h"
#include "mdl/Map.h"
#include "mdl/Node.h"
#include "mdl/NodeQueries.h"

#include "kdl/ranges/to.h"
#include "kdl/vector_utils.h"

namespace tb::mdl
{

void addNodesAndNotify(const std::map<Node*, std::vector<Node*>>& nodes, Map& map)
{
  const auto parents =
    collectNodesAndAncestors(nodes | std::views::keys | kdl::ranges::to<std::vector>());

  auto notifyParents = NotifyBeforeAndAfter{
    map.nodesWillChangeNotifier, map.nodesDidChangeNotifier, parents};

  auto addedNodes = std::vector<Node*>{};
  for (const auto& [parent, children] : nodes)
  {
    parent->addChildren(children);
    addedNodes = kdl::vec_concat(std::move(addedNodes), children);
  }

  map.nodesWereAddedNotifier(addedNodes);
}

void removeNodesAndNotify(const std::map<Node*, std::vector<Node*>>& nodes, Map& map)
{
  const auto parents =
    collectNodesAndAncestors(nodes | std::views::keys | kdl::ranges::to<std::vector>());

  auto notifyParents = NotifyBeforeAndAfter{
    map.nodesWillChangeNotifier, map.nodesDidChangeNotifier, parents};

  const auto allChildren =
    nodes | std::views::values | std::views::join | kdl::ranges::to<std::vector>();

  auto notifyChildren = NotifyBeforeAndAfter{
    map.nodesWillBeRemovedNotifier, map.nodesWereRemovedNotifier, allChildren};

  for (const auto& [parent, children] : nodes)
  {
    parent->removeChildren(std::begin(children), std::end(children));
  }
}

} // namespace tb::mdl
