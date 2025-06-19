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

#include "UpdateLinkedGroupsHelper.h"

#include "mdl/GroupNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeQueries.h"
#include "ui/MapDocument.h"

#include "kdl/overload.h"
#include "kdl/range_to_vector.h"
#include "kdl/result.h"
#include "kdl/result_fold.h"
#include "kdl/vector_utils.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <ranges>
#include <unordered_set>

namespace tb::ui
{
namespace
{

// Order groups so that descendants will be updated before their ancestors
auto compareByAncestry(const mdl::GroupNode* lhs, const mdl::GroupNode* rhs)
{
  return rhs->isAncestorOf(lhs);
}

std::vector<mdl::Node*> collectOldChildren(
  const std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>>&
    nodes)
{
  auto result = std::vector<mdl::Node*>{};
  for (auto& [parent, newChildren] : nodes)
  {
    result = kdl::vec_concat(std::move(result), parent->children());
  }
  return result;
}

auto doReplaceChildren(
  std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>> nodes,
  MapDocument& document)
{
  auto result =
    std::vector<std::pair<mdl::Node*, std::vector<std::unique_ptr<mdl::Node>>>>{};

  if (nodes.empty())
  {
    return result;
  }

  const auto parents = collectNodesAndAncestors(kdl::map_keys(nodes));
  auto notifyParents = NotifyBeforeAndAfter{
    document.nodesWillChangeNotifier, document.nodesDidChangeNotifier, parents};

  const auto allOldChildren = collectOldChildren(nodes);
  auto notifyChildren = NotifyBeforeAndAfter{
    document.nodesWillBeRemovedNotifier,
    document.nodesWereRemovedNotifier,
    allOldChildren};

  auto allNewChildren = std::vector<mdl::Node*>{};

  for (auto& [parent, newChildren] : nodes)
  {
    allNewChildren = kdl::vec_concat(
      std::move(allNewChildren),
      kdl::vec_transform(newChildren, [](auto& child) { return child.get(); }));

    auto oldChildren = parent->replaceChildren(std::move(newChildren));

    result.emplace_back(parent, std::move(oldChildren));
  }

  document.nodesWereAddedNotifier(allNewChildren);

  return result;
}

} // namespace

bool checkLinkedGroupsToUpdate(const std::vector<mdl::GroupNode*>& changedLinkedGroups)
{
  const auto linkedGroupIds = kdl::vec_sort(
    changedLinkedGroups
    | std::views::transform([](const auto* groupNode) { return groupNode->linkId(); })
    | kdl::to_vector);

  return std::ranges::adjacent_find(linkedGroupIds) == std::end(linkedGroupIds);
}

UpdateLinkedGroupsHelper::UpdateLinkedGroupsHelper(
  ChangedLinkedGroups changedLinkedGroups)
  : m_state{kdl::vec_sort(std::move(changedLinkedGroups), compareByAncestry)}
{
}

UpdateLinkedGroupsHelper::~UpdateLinkedGroupsHelper() = default;

Result<void> UpdateLinkedGroupsHelper::applyLinkedGroupUpdates(MapDocument& document)
{
  return computeLinkedGroupUpdates(document)
         | kdl::transform([&]() { doApplyOrUndoLinkedGroupUpdates(document); });
}

void UpdateLinkedGroupsHelper::undoLinkedGroupUpdates(MapDocument& document)
{
  doApplyOrUndoLinkedGroupUpdates(document);
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

Result<void> UpdateLinkedGroupsHelper::computeLinkedGroupUpdates(MapDocument& document)
{
  return std::visit(
    kdl::overload(
      [&](const ChangedLinkedGroups& changedLinkedGroups) {
        return computeLinkedGroupUpdates(changedLinkedGroups, document)
               | kdl::transform([&](auto&& linkedGroupUpdates) {
                   m_state =
                     std::forward<decltype(linkedGroupUpdates)>(linkedGroupUpdates);
                 });
      },
      [](const LinkedGroupUpdates&) -> Result<void> { return kdl::void_success; }),
    m_state);
}

Result<UpdateLinkedGroupsHelper::LinkedGroupUpdates> UpdateLinkedGroupsHelper::
  computeLinkedGroupUpdates(
    const ChangedLinkedGroups& changedLinkedGroups, MapDocument& document)
{
  if (!checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    return Error{"Cannot update multiple members of the same link set"};
  }

  const auto& worldBounds = document.worldBounds();
  return changedLinkedGroups | std::views::transform([&](const auto* groupNode) {
           const auto groupNodesToUpdate = kdl::vec_erase(
             mdl::collectGroupsWithLinkId({document.world()}, groupNode->linkId()),
             groupNode);

           return mdl::updateLinkedGroups(
             *groupNode, groupNodesToUpdate, worldBounds, document.taskManager());
         })
         | kdl::fold
         | kdl::and_then([&](auto nestedUpdateLists) -> Result<LinkedGroupUpdates> {
             return kdl::vec_flatten(std::move(nestedUpdateLists));
           });
}

void UpdateLinkedGroupsHelper::doApplyOrUndoLinkedGroupUpdates(MapDocument& document)
{
  std::visit(
    kdl::overload(
      [](const ChangedLinkedGroups&) {},
      [&](LinkedGroupUpdates&& linkedGroupUpdates) {
        m_state = doReplaceChildren(std::move(linkedGroupUpdates), document);
      }),
    std::move(m_state));
}

} // namespace tb::ui
