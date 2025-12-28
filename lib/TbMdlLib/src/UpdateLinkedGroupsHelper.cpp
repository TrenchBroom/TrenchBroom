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

#include "mdl/UpdateLinkedGroupsHelper.h"

#include "mdl/GroupNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/ModelUtils.h"

#include "kd/overload.h"
#include "kd/ranges/as_rvalue_view.h"
#include "kd/ranges/to.h"
#include "kd/result.h"
#include "kd/result_fold.h"
#include "kd/vector_utils.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <ranges>
#include <unordered_set>

namespace tb::mdl
{
namespace
{

// Order groups so that descendants will be updated before their ancestors
auto compareByAncestry(const GroupNode* lhs, const GroupNode* rhs)
{
  return rhs->isAncestorOf(lhs);
}

std::vector<Node*> collectOldChildren(
  const std::vector<std::pair<Node*, std::vector<std::unique_ptr<Node>>>>& nodes)
{
  auto result = std::vector<Node*>{};
  for (auto& [parent, newChildren] : nodes)
  {
    result = kdl::vec_concat(std::move(result), parent->children());
  }
  return result;
}

auto doReplaceChildren(
  std::vector<std::pair<Node*, std::vector<std::unique_ptr<Node>>>> nodes, Map& map)
{
  auto result = std::vector<std::pair<Node*, std::vector<std::unique_ptr<Node>>>>{};

  if (nodes.empty())
  {
    return result;
  }

  const auto allOldChildren = collectOldChildren(nodes);
  auto notifyChildren = NotifyBeforeAndAfter{
    map.nodesWillBeRemovedNotifier, map.nodesWereRemovedNotifier, allOldChildren};

  auto allNewChildren = std::vector<Node*>{};

  for (auto& [parent, newChildren] : nodes)
  {
    allNewChildren = kdl::vec_concat(
      std::move(allNewChildren), newChildren | std::views::transform([](auto& child) {
                                   return child.get();
                                 }) | kdl::ranges::to<std::vector>());

    auto oldChildren = parent->replaceChildren(std::move(newChildren));

    result.emplace_back(parent, std::move(oldChildren));
  }

  map.nodesWereAddedNotifier(allNewChildren);

  return result;
}

} // namespace

bool checkLinkedGroupsToUpdate(const std::vector<GroupNode*>& changedLinkedGroups)
{
  const auto linkedGroupIds = kdl::vec_sort(
    changedLinkedGroups
    | std::views::transform([](const auto* groupNode) { return groupNode->linkId(); })
    | kdl::ranges::to<std::vector>());

  return std::ranges::adjacent_find(linkedGroupIds) == std::end(linkedGroupIds);
}

UpdateLinkedGroupsHelper::UpdateLinkedGroupsHelper(
  ChangedLinkedGroups changedLinkedGroups)
  : m_state{kdl::vec_sort(std::move(changedLinkedGroups), compareByAncestry)}
{
}

UpdateLinkedGroupsHelper::~UpdateLinkedGroupsHelper() = default;

Result<void> UpdateLinkedGroupsHelper::applyLinkedGroupUpdates(Map& map)
{
  return computeLinkedGroupUpdates(map)
         | kdl::transform([&]() { doApplyOrUndoLinkedGroupUpdates(map); });
}

void UpdateLinkedGroupsHelper::undoLinkedGroupUpdates(Map& map)
{
  doApplyOrUndoLinkedGroupUpdates(map);
}

void UpdateLinkedGroupsHelper::collateWith(UpdateLinkedGroupsHelper& other)
{
  // Both helpers have already applied their changes at this point, so in both helpers,
  // m_linkedGroups contains pairs p where
  // - p.first is the group node to update
  // - p.second is a vector containing the group node's original children
  //
  // Let p_o be an update from the other helper. If p_o is an update for a linked group
  // node that was updated by this helper, then there is a pair p_t in this helper such
  // that p_t.first == p_o.first. In this case, we want to keep the old children of the
  // linked group node stored in this helper and discard those in the other helper. If
  // p_o is not an update for a linked group node that was updated by this helper, then
  // we will add p_o to our updates and remove it from the other helper's updates to
  // prevent the replaced node to be deleted with the other helper.

  auto& myLinkedGroupUpdates = std::get<LinkedGroupUpdates>(m_state);
  auto& theirLinkedGroupUpdates = std::get<LinkedGroupUpdates>(other.m_state);

  for (auto& [theirGroupNodeToUpdate_, theirOldChildren] : theirLinkedGroupUpdates)
  {
    const auto myIt = std::ranges::find_if(
      myLinkedGroupUpdates,
      [theirGroupNodeToUpdate = theirGroupNodeToUpdate_](const auto& p) {
        return p.first == theirGroupNodeToUpdate;
      });
    if (myIt == std::end(myLinkedGroupUpdates))
    {
      myLinkedGroupUpdates.emplace_back(
        theirGroupNodeToUpdate_, std::move(theirOldChildren));
    }
  }
}

Result<void> UpdateLinkedGroupsHelper::computeLinkedGroupUpdates(Map& map)
{
  return std::visit(
    kdl::overload(
      [&](const ChangedLinkedGroups& changedLinkedGroups) {
        return computeLinkedGroupUpdates(changedLinkedGroups, map)
               | kdl::transform([&](auto&& linkedGroupUpdates) {
                   m_state =
                     std::forward<decltype(linkedGroupUpdates)>(linkedGroupUpdates);
                 });
      },
      [](const LinkedGroupUpdates&) -> Result<void> { return kdl::void_success; }),
    m_state);
}

Result<UpdateLinkedGroupsHelper::LinkedGroupUpdates> UpdateLinkedGroupsHelper::
  computeLinkedGroupUpdates(const ChangedLinkedGroups& changedLinkedGroups, Map& map)
{
  if (!checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    return Error{"Cannot update multiple members of the same link set"};
  }

  const auto& worldBounds = map.worldBounds();
  return changedLinkedGroups | std::views::transform([&](const auto* groupNode) {
           const auto groupNodesToUpdate = kdl::vec_erase(
             collectGroupsWithLinkId({&map.worldNode()}, groupNode->linkId()), groupNode);

           return updateLinkedGroups(
             *groupNode, groupNodesToUpdate, worldBounds, map.taskManager());
         })
         | kdl::fold
         | kdl::and_then([&](auto nestedUpdateLists) -> Result<LinkedGroupUpdates> {
             return nestedUpdateLists | std::views::join | kdl::views::as_rvalue
                    | kdl::ranges::to<std::vector>();
           });
}

void UpdateLinkedGroupsHelper::doApplyOrUndoLinkedGroupUpdates(Map& map)
{
  std::visit(
    kdl::overload(
      [](const ChangedLinkedGroups&) {},
      [&](LinkedGroupUpdates&& linkedGroupUpdates) {
        m_state = doReplaceChildren(std::move(linkedGroupUpdates), map);
      }),
    std::move(m_state));
}

} // namespace tb::mdl
