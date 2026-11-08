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

#include "mdl/ReparentNodesCommand.h"

#include "mdl/AddRemoveNodesUtils.h"
#include "mdl/Node.h"

#include "kd/contracts.h"
#include "kd/ranges/to.h"

#include <ranges>
#include <unordered_map>

namespace tb::mdl
{
namespace
{

auto regroupNodes(
  kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>> nodes,
  const kdl::flat_map<Node*, std::vector<Node*>>& newParents)
{
  auto nodesByPointer = std::unordered_map<Node*, std::unique_ptr<Node>>{};
  for (auto&& [parent, children] : nodes)
  {
    for (auto& child : children)
    {
      auto* childPtr = child.get();
      nodesByPointer.emplace(childPtr, std::move(child));
    }
  }

  auto result = kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>>{};
  for (const auto& [newParent, children] : newParents)
  {
    result[newParent] = children | std::views::transform([&](auto* child) {
                          contract_assert(nodesByPointer.contains(child));
                          return std::move(nodesByPointer.extract(child).mapped());
                        })
                        | kdl::ranges::to<std::vector>();
  }

  contract_assert(nodesByPointer.empty());
  return result;
}

} // namespace

std::unique_ptr<ReparentNodesCommand> ReparentNodesCommand::reparent(
  kdl::flat_map<Node*, std::vector<Node*>> nodesToAdd,
  kdl::flat_map<Node*, std::vector<Node*>> nodesToRemove)
{
  return std::make_unique<ReparentNodesCommand>(
    std::move(nodesToAdd), std::move(nodesToRemove));
}

ReparentNodesCommand::ReparentNodesCommand(
  kdl::flat_map<Node*, std::vector<Node*>> nodesToAdd,
  kdl::flat_map<Node*, std::vector<Node*>> nodesToRemove)
  : UpdateLinkedGroupsCommandBase{"Reparent Objects", true}
  , m_nodesToAdd{std::move(nodesToAdd)}
  , m_nodesToRemove{std::move(nodesToRemove)}
{
}

bool ReparentNodesCommand::doPerformDo(Map& map)
{
  addNodesAndNotify(
    regroupNodes(removeNodesAndNotify(m_nodesToRemove, map), m_nodesToAdd), map);
  return true;
}

bool ReparentNodesCommand::doPerformUndo(Map& map)
{
  addNodesAndNotify(
    regroupNodes(removeNodesAndNotify(m_nodesToAdd, map), m_nodesToRemove), map);
  return true;
}

} // namespace tb::mdl
