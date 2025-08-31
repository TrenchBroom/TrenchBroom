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

#include "mdl/Map_Groups.h"

#include "Logger.h"
#include "Uuid.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/CurrentGroupCommand.h"
#include "mdl/EditorContext.h"
#include "mdl/GroupNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/ModelUtils.h"
#include "mdl/SetLinkIdsCommand.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateLinkedGroupsCommand.h"
#include "mdl/UpdateLinkedGroupsHelper.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep

#include "kdl/stable_remove_duplicates.h"
#include "kdl/string_format.h"

#include <algorithm>

namespace tb::mdl
{
namespace
{

std::vector<Node*> collectGroupableNodes(
  const std::vector<Node*>& selectedNodes, const EntityNodeBase* world)
{
  std::vector<Node*> result;
  const auto addNode = [&](auto&& thisLambda, auto* node) {
    if (node->entity() == world)
    {
      result.push_back(node);
    }
    else
    {
      node->visitParent(thisLambda);
    }
  };

  Node::visitAll(
    selectedNodes,
    kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [&](GroupNode* group) { result.push_back(group); },
      [&](EntityNode* entity) { result.push_back(entity); },
      [&](auto&& thisLambda, BrushNode* brush) { addNode(thisLambda, brush); },
      [&](auto&& thisLambda, PatchNode* patch) { addNode(thisLambda, patch); }));
  return kdl::col_stable_remove_duplicates(std::move(result));
}

std::vector<Node*> collectNodesToUnlink(const std::vector<GroupNode*>& groupNodes)
{
  auto result = std::vector<Node*>{};
  for (auto* groupNode : groupNodes)
  {
    result.push_back(groupNode);
    groupNode->visitChildren(kdl::overload(
      [](const WorldNode*) {},
      [](const LayerNode*) {},
      [](const GroupNode*) {},
      [&](EntityNode* entityNode) { result.push_back(entityNode); },
      [&](BrushNode* brushNode) { result.push_back(brushNode); },
      [&](PatchNode* patchNode) { result.push_back(patchNode); }));
  }
  return result;
}

void linkGroups(Map& map, const std::vector<GroupNode*>& groupNodes)
{
  if (groupNodes.size() > 1)
  {
    const auto& sourceGroupNode = *groupNodes.front();
    const auto targetGroupNodes =
      kdl::vec_slice_suffix(groupNodes, groupNodes.size() - 1);
    copyAndReturnLinkIds(sourceGroupNode, targetGroupNodes)
      | kdl::transform([&](auto linkIds) {
          auto linkIdVector = kdl::vec_transform(
            std::move(linkIds), [](auto pair) -> std::tuple<Node*, std::string> {
              return {std::move(pair)};
            });

          map.executeAndStore(
            std::make_unique<SetLinkIdsCommand>("Set Link ID", std::move(linkIdVector)));
        })
      | kdl::transform_error(
        [&](auto e) { map.logger().error() << "Could not link groups: " << e.msg; });
  }
}

void unlinkGroups(Map& map, const std::vector<GroupNode*>& groupNodes)
{
  const auto nodesToUnlink = collectNodesToUnlink(groupNodes);

  auto linkIds = kdl::vec_transform(
    nodesToUnlink,
    [](auto* node) -> std::tuple<Node*, std::string> { return {node, generateUuid()}; });

  map.executeAndStore(
    std::make_unique<SetLinkIdsCommand>("Reset Link ID", std::move(linkIds)));
}

} // namespace

Node* currentGroupOrWorld(const Map& map)
{
  Node* result = map.editorContext().currentGroup();
  return result ? result : map.world();
}

void openGroup(Map& map, GroupNode* groupNode)
{
  auto transaction = Transaction{map, "Open Group"};

  map.deselectAll();

  if (auto* previousGroupNode = map.editorContext().currentGroup())
  {
    map.resetNodeLockingState({previousGroupNode});
  }
  else
  {
    map.lockNodes({map.world()});
  }
  map.unlockNodes({groupNode});
  map.executeAndStore(CurrentGroupCommand::push(groupNode));

  transaction.commit();
}

void closeGroup(Map& map)
{
  auto transaction = Transaction{map, "Close Group"};

  map.deselectAll();
  auto* previousGroup = map.editorContext().currentGroup();
  map.resetNodeLockingState({previousGroup});
  map.executeAndStore(CurrentGroupCommand::pop());

  auto* newGroup = map.editorContext().currentGroup();
  if (newGroup != nullptr)
  {
    map.unlockNodes({newGroup});
  }
  else
  {
    map.unlockNodes({map.world()});
  }

  transaction.commit();
}

GroupNode* groupSelectedNodes(Map& map, const std::string& name)
{
  if (!map.selection().hasNodes())
  {
    return nullptr;
  }

  const auto nodes = collectGroupableNodes(map.selection().nodes, map.world());
  if (nodes.empty())
  {
    return nullptr;
  }

  auto* group = new GroupNode{Group{name}};

  auto transaction = Transaction{map, "Group Selected Objects"};
  map.deselectAll();
  if (
    addNodes(map, {{parentForNodes(map, nodes), {group}}}).empty()
    || !reparentNodes(map, {{group, nodes}}))
  {
    transaction.cancel();
    return nullptr;
  }
  map.selectNodes({group});

  if (!transaction.commit())
  {
    return nullptr;
  }

  return group;
}

void ungroupSelectedNodes(Map& map)
{
  if (!map.selection().hasNodes())
  {
    return;
  }

  auto transaction = Transaction{map, "Ungroup"};
  separateSelectedLinkedGroups(map, false);

  const auto selectedNodes = map.selection().nodes;
  auto nodesToReselect = std::vector<Node*>{};

  map.deselectAll();

  auto success = true;
  Node::visitAll(
    selectedNodes,
    kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [&](GroupNode* group) {
        auto* parent = group->parent();
        const auto children = group->children();
        success = success && reparentNodes(map, {{parent, children}});
        nodesToReselect = kdl::vec_concat(std::move(nodesToReselect), children);
      },
      [&](EntityNode* entity) { nodesToReselect.push_back(entity); },
      [&](BrushNode* brush) { nodesToReselect.push_back(brush); },
      [&](PatchNode* patch) { nodesToReselect.push_back(patch); }));

  if (!success)
  {
    transaction.cancel();
    return;
  }

  map.selectNodes(nodesToReselect);
  transaction.commit();
}

void mergeSelectedGroupsWithGroup(Map& map, GroupNode* group)
{
  if (!map.selection().hasNodes() || !map.selection().hasOnlyGroups())
  {
    return;
  }

  const auto groupsToMerge = map.selection().groups;

  auto transaction = Transaction{map, "Merge Groups"};
  map.deselectAll();

  for (auto groupToMerge : groupsToMerge)
  {
    if (groupToMerge != group)
    {
      const auto children = groupToMerge->children();
      if (!reparentNodes(map, {{group, children}}))
      {
        transaction.cancel();
        return;
      }
    }
  }
  map.selectNodes({group});

  transaction.commit();
}

void renameSelectedGroups(Map& map, const std::string& name)
{
  if (map.selection().hasNodes() && map.selection().hasOnlyGroups())
  {
    const auto commandName =
      kdl::str_plural("Rename ", map.selection().groups.size(), "Group", "Groups");
    applyAndSwap(
      map,
      commandName,
      map.selection().groups,
      {},
      kdl::overload(
        [](Layer&) { return true; },
        [&](Group& group) {
          group.setName(name);
          return true;
        },
        [](Entity&) { return true; },
        [](Brush&) { return true; },
        [](BezierPatch&) { return true; }));
  }
}

bool canCreateLinkedDuplicate(const Map& map)
{
  return map.selection().hasOnlyGroups() && map.selection().groups.size() == 1u;
}

GroupNode* createLinkedDuplicate(Map& map)
{
  if (!canCreateLinkedDuplicate(map))
  {
    return nullptr;
  }

  auto* groupNode = map.selection().groups.front();
  auto* groupNodeClone =
    static_cast<GroupNode*>(groupNode->cloneRecursively(map.worldBounds()));
  auto* suggestedParent = parentForNodes(map, {groupNode});

  auto transaction = Transaction{map, "Create Linked Duplicate"};
  if (addNodes(map, {{suggestedParent, {groupNodeClone}}}).empty())
  {
    transaction.cancel();
    return nullptr;
  }

  if (!transaction.commit())
  {
    return nullptr;
  }

  return groupNodeClone;
}

void separateSelectedLinkedGroups(Map& map, const bool relinkGroups)
{
  const auto selectedLinkIds = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
    map.selection().groups, [](const auto* groupNode) { return groupNode->linkId(); }));

  auto groupsToUnlink = std::vector<GroupNode*>{};
  auto groupsToRelink = std::vector<std::vector<GroupNode*>>{};

  for (const auto& linkedGroupId : selectedLinkIds)
  {
    auto linkedGroups = collectGroupsWithLinkId({map.world()}, linkedGroupId);

    // partition the linked groups into selected and unselected ones
    const auto it = std::partition(
      std::begin(linkedGroups), std::end(linkedGroups), [](const auto* linkedGroupNode) {
        return linkedGroupNode->selected();
      });

    auto selectedLinkedGroups = std::vector<GroupNode*>(std::begin(linkedGroups), it);

    assert(!selectedLinkedGroups.empty());
    if (linkedGroups.size() - selectedLinkedGroups.size() > 0)
    {
      if (relinkGroups)
      {
        groupsToRelink.push_back(selectedLinkedGroups);
      }
      groupsToUnlink =
        kdl::vec_concat(std::move(groupsToUnlink), std::move(selectedLinkedGroups));
    }
    else if (selectedLinkedGroups.size() > 1 && !relinkGroups)
    {
      // all members of a link group are being separated, and we don't want to relink
      // them, so we need to reset their linked group IDs
      groupsToUnlink =
        kdl::vec_concat(std::move(groupsToUnlink), std::move(selectedLinkedGroups));
    }
  }

  const auto changedLinkedGroups = kdl::vec_sort_and_remove_duplicates(kdl::vec_concat(
    collectContainingGroups(kdl::vec_static_cast<Node*>(groupsToUnlink)),
    collectContainingGroups(
      kdl::vec_static_cast<Node*>(kdl::vec_flatten(groupsToRelink)))));

  if (checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    auto transaction = Transaction{map, "Separate Selected Linked Groups"};

    unlinkGroups(map, groupsToUnlink);
    for (const auto& groupNodes : groupsToRelink)
    {
      linkGroups(map, groupNodes);
    }

    setHasPendingChanges(changedLinkedGroups, true);
    transaction.commit();
  }
}

bool canSeparateSelectedLinkedGroups(const Map& map)
{
  return std::ranges::any_of(map.selection().groups, [&](const auto* groupNode) {
    const auto linkedGroups = collectNodesWithLinkId({map.world()}, groupNode->linkId());
    return linkedGroups.size() > 1u
           && std::ranges::any_of(linkedGroups, [](const auto* linkedGroupNode) {
                return !linkedGroupNode->selected();
              });
  });
}

bool canUpdateLinkedGroups(const std::vector<Node*>& nodes)
{
  if (nodes.empty())
  {
    return false;
  }

  const auto changedLinkedGroups = collectContainingGroups(nodes);
  return checkLinkedGroupsToUpdate(changedLinkedGroups);
}

void setHasPendingChanges(
  const std::vector<GroupNode*>& groupNodes, const bool hasPendingChanges)
{
  for (auto* groupNode : groupNodes)
  {
    groupNode->setHasPendingChanges(hasPendingChanges);
  }
}

} // namespace tb::mdl
