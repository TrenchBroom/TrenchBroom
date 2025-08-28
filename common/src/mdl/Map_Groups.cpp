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

std::vector<GroupNode*> collectGroupsWithPendingChanges(Node& node)
{
  auto result = std::vector<GroupNode*>{};

  node.accept(kdl::overload(
    [](auto&& thisLambda, const WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, GroupNode* groupNode) {
      if (groupNode->hasPendingChanges())
      {
        result.push_back(groupNode);
      }
      groupNode->visitChildren(thisLambda);
    },
    [](const EntityNode*) {},
    [](const BrushNode*) {},
    [](const PatchNode*) {}));

  return result;
}

} // namespace

GroupNode* Map::currentGroup() const
{
  return m_editorContext->currentGroup();
}

Node* Map::currentGroupOrWorld() const
{
  Node* result = currentGroup();
  return result ? result : m_world.get();
}

void Map::openGroup(GroupNode* groupNode)
{
  auto transaction = Transaction{*this, "Open Group"};

  deselectAll();

  if (auto* previousGroupNode = editorContext().currentGroup())
  {
    resetNodeLockingState({previousGroupNode});
  }
  else
  {
    lockNodes({world()});
  }
  unlockNodes({groupNode});
  executeAndStore(CurrentGroupCommand::push(groupNode));

  transaction.commit();
}

void Map::closeGroup()
{
  auto transaction = Transaction{*this, "Close Group"};

  deselectAll();
  auto* previousGroup = editorContext().currentGroup();
  resetNodeLockingState({previousGroup});
  executeAndStore(CurrentGroupCommand::pop());

  auto* currentGroup = editorContext().currentGroup();
  if (currentGroup != nullptr)
  {
    unlockNodes({currentGroup});
  }
  else
  {
    unlockNodes({world()});
  }

  transaction.commit();
}

GroupNode* Map::groupSelectedNodes(const std::string& name)
{
  if (!selection().hasNodes())
  {
    return nullptr;
  }

  const auto nodes = collectGroupableNodes(selection().nodes, world());
  if (nodes.empty())
  {
    return nullptr;
  }

  auto* group = new GroupNode{Group{name}};

  auto transaction = Transaction{*this, "Group Selected Objects"};
  deselectAll();
  if (
    addNodes(*this, {{parentForNodes(*this, nodes), {group}}}).empty()
    || !reparentNodes(*this, {{group, nodes}}))
  {
    transaction.cancel();
    return nullptr;
  }
  selectNodes({group});

  if (!transaction.commit())
  {
    return nullptr;
  }

  return group;
}

void Map::ungroupSelectedNodes()
{
  if (!selection().hasNodes())
  {
    return;
  }

  auto transaction = Transaction{*this, "Ungroup"};
  separateSelectedLinkedGroups(false);

  const auto selectedNodes = selection().nodes;
  auto nodesToReselect = std::vector<Node*>{};

  deselectAll();

  auto success = true;
  Node::visitAll(
    selectedNodes,
    kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [&](GroupNode* group) {
        auto* parent = group->parent();
        const auto children = group->children();
        success = success && reparentNodes(*this, {{parent, children}});
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

  selectNodes(nodesToReselect);
  transaction.commit();
}

void Map::mergeSelectedGroupsWithGroup(GroupNode* group)
{
  if (!selection().hasNodes() || !selection().hasOnlyGroups())
  {
    return;
  }

  const auto groupsToMerge = selection().groups;

  auto transaction = Transaction{*this, "Merge Groups"};
  deselectAll();

  for (auto groupToMerge : groupsToMerge)
  {
    if (groupToMerge != group)
    {
      const auto children = groupToMerge->children();
      if (!reparentNodes(*this, {{group, children}}))
      {
        transaction.cancel();
        return;
      }
    }
  }
  selectNodes({group});

  transaction.commit();
}

void Map::renameSelectedGroups(const std::string& name)
{
  if (selection().hasNodes() && selection().hasOnlyGroups())
  {
    const auto commandName =
      kdl::str_plural("Rename ", selection().groups.size(), "Group", "Groups");
    applyAndSwap(
      *this,
      commandName,
      selection().groups,
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

bool Map::canCreateLinkedDuplicate() const
{
  return selection().hasOnlyGroups() && selection().groups.size() == 1u;
}

GroupNode* Map::createLinkedDuplicate()
{
  if (!canCreateLinkedDuplicate())
  {
    return nullptr;
  }

  auto* groupNode = selection().groups.front();
  auto* groupNodeClone =
    static_cast<GroupNode*>(groupNode->cloneRecursively(worldBounds()));
  auto* suggestedParent = parentForNodes(*this, {groupNode});

  auto transaction = Transaction{*this, "Create Linked Duplicate"};
  if (addNodes(*this, {{suggestedParent, {groupNodeClone}}}).empty())
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

void Map::linkGroups(const std::vector<GroupNode*>& groupNodes)
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

          executeAndStore(
            std::make_unique<SetLinkIdsCommand>("Set Link ID", std::move(linkIdVector)));
        })
      | kdl::transform_error(
        [&](auto e) { logger().error() << "Could not link groups: " << e.msg; });
  }
}

void Map::unlinkGroups(const std::vector<GroupNode*>& groupNodes)
{
  const auto nodesToUnlink = collectNodesToUnlink(groupNodes);

  auto linkIds = kdl::vec_transform(
    nodesToUnlink,
    [](auto* node) -> std::tuple<Node*, std::string> { return {node, generateUuid()}; });

  executeAndStore(
    std::make_unique<SetLinkIdsCommand>("Reset Link ID", std::move(linkIds)));
}

void Map::separateSelectedLinkedGroups(const bool relinkGroups)
{
  const auto selectedLinkIds = kdl::vec_sort_and_remove_duplicates(kdl::vec_transform(
    selection().groups, [](const auto* groupNode) { return groupNode->linkId(); }));

  auto groupsToUnlink = std::vector<GroupNode*>{};
  auto groupsToRelink = std::vector<std::vector<GroupNode*>>{};

  for (const auto& linkedGroupId : selectedLinkIds)
  {
    auto linkedGroups = collectGroupsWithLinkId({world()}, linkedGroupId);

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
    auto transaction = Transaction{*this, "Separate Selected Linked Groups"};

    unlinkGroups(groupsToUnlink);
    for (const auto& groupNodes : groupsToRelink)
    {
      linkGroups(groupNodes);
    }

    setHasPendingChanges(changedLinkedGroups, true);
    transaction.commit();
  }
}

bool Map::canSeparateSelectedLinkedGroups() const
{
  return std::ranges::any_of(selection().groups, [&](const auto* groupNode) {
    const auto linkedGroups = collectNodesWithLinkId({world()}, groupNode->linkId());
    return linkedGroups.size() > 1u
           && std::ranges::any_of(linkedGroups, [](const auto* linkedGroupNode) {
                return !linkedGroupNode->selected();
              });
  });
}

void Map::setHasPendingChanges(
  const std::vector<GroupNode*>& groupNodes, const bool hasPendingChanges)
{
  for (auto* groupNode : groupNodes)
  {
    groupNode->setHasPendingChanges(hasPendingChanges);
  }
}

bool Map::canUpdateLinkedGroups(const std::vector<Node*>& nodes) const
{
  if (nodes.empty())
  {
    return false;
  }

  const auto changedLinkedGroups = collectContainingGroups(nodes);
  return checkLinkedGroupsToUpdate(changedLinkedGroups);
}

bool Map::updateLinkedGroups()
{
  if (isCurrentDocumentStateObservable())
  {
    if (const auto allChangedLinkedGroups = collectGroupsWithPendingChanges(*m_world);
        !allChangedLinkedGroups.empty())
    {
      setHasPendingChanges(allChangedLinkedGroups, false);

      auto command = std::make_unique<UpdateLinkedGroupsCommand>(allChangedLinkedGroups);
      const auto result = executeAndStore(std::move(command));
      return result->success();
    }
  }

  return true;
}

} // namespace tb::mdl
