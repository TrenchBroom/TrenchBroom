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

#include "Map_Nodes.h"

#include "Ensure.h"
#include "Logger.h"
#include "Map.h"
#include "Uuid.h"
#include "mdl/AddRemoveNodesCommand.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityModelManager.h"
#include "mdl/EntityNode.h"
#include "mdl/Game.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/NodeQueries.h"
#include "mdl/PatchNode.h"
#include "mdl/ReparentNodesCommand.h"
#include "mdl/SetLinkIdsCommand.h"
#include "mdl/SwapNodeContentsCommand.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kdl/map_utils.h"
#include "kdl/overload.h"

namespace tb::mdl
{
namespace
{
std::vector<GroupNode*> collectGroupsOrContainers(const std::vector<Node*>& nodes)
{
  auto result = std::vector<GroupNode*>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode*) {},
      [](const LayerNode*) {},
      [&](GroupNode* groupNode) { result.push_back(groupNode); },
      [&](EntityNode* entityNode) {
        if (auto* containingGroup = entityNode->containingGroup())
        {
          result.push_back(containingGroup);
        }
      },
      [&](BrushNode* brushNode) {
        if (auto* containingGroup = brushNode->containingGroup())
        {
          result.push_back(containingGroup);
        }
      },
      [&](PatchNode* patchNode) {
        if (auto* containingGroup = patchNode->containingGroup())
        {
          result.push_back(containingGroup);
        }
      }));
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

void copyAndSetLinkIds(
  const std::map<Node*, std::vector<Node*>>& nodesToAdd,
  WorldNode& worldNode,
  Logger& logger)
{
  const auto errors = copyAndSetLinkIdsBeforeAddingNodes(nodesToAdd, worldNode);
  for (const auto& error : errors)
  {
    logger.warn() << "Could not paste linked groups: " + error.msg;
  }
}

/**
 * Returns whether, for UI reasons, duplicating the given node should also cause its
 * parent to be duplicated.
 *
 * Applies when duplicating a brush inside a brush entity.
 */
bool shouldCloneParentWhenCloningNode(const Node* node)
{
  return node->parent()->accept(kdl::overload(
    [](const WorldNode*) { return false; },
    [](const LayerNode*) { return false; },
    [](const GroupNode*) { return false; },
    [&](const EntityNode*) { return true; },
    [](const BrushNode*) { return false; },
    [](const PatchNode*) { return false; }));
}

void resetLinkIdsOfNonGroupedNodes(const std::map<Node*, std::vector<Node*>>& nodes)
{
  for (const auto& [parent, children] : nodes)
  {
    Node::visitAll(
      children,
      kdl::overload(
        [](const WorldNode*) {},
        [](const LayerNode*) {},
        [](const GroupNode*) {},
        [](auto&& thisLambda, EntityNode* entityNode) {
          entityNode->setLinkId(generateUuid());
          entityNode->visitChildren(thisLambda);
        },
        [](BrushNode* brushNode) { brushNode->setLinkId(generateUuid()); },
        [](PatchNode* patchNode) { patchNode->setLinkId(generateUuid()); }));
  }
}

bool checkReparenting(const std::map<Node*, std::vector<Node*>>& nodesToAdd)
{
  for (const auto& [newParent, children] : nodesToAdd)
  {
    if (!newParent->canAddChildren(std::begin(children), std::end(children)))
    {
      return false;
    }
  }
  return true;
}

auto setLinkIdsForReparentingNodes(
  const std::map<Node*, std::vector<Node*>>& nodesToReparent)
{
  auto result = std::vector<std::tuple<Node*, std::string>>{};
  for (const auto& [newParent_, nodes] : nodesToReparent)
  {
    Node::visitAll(
      nodes,
      kdl::overload(
        [](const WorldNode*) {},
        [](const LayerNode*) {},
        [](const GroupNode*) {
          // group nodes can keep their ID because they should remain in their link set
        },
        [&, newParent = newParent_](auto&& thisLambda, EntityNode* entityNode) {
          if (newParent->isAncestorOf(entityNode->parent()))
          {
            result.emplace_back(entityNode, generateUuid());
            entityNode->visitChildren(thisLambda);
          }
        },
        [&, newParent = newParent_](BrushNode* brushNode) {
          if (newParent->isAncestorOf(brushNode->parent()))
          {
            result.emplace_back(brushNode, generateUuid());
          }
        },
        [&, newParent = newParent_](PatchNode* patchNode) {
          if (newParent->isAncestorOf(patchNode->parent()))
          {
            result.emplace_back(patchNode, generateUuid());
          }
        }));
  }
  return result;
}

std::vector<Node*> removeImplicitelyRemovedNodes(std::vector<Node*> nodes)
{
  if (nodes.empty())
  {
    return nodes;
  }

  nodes = kdl::vec_sort(std::move(nodes), [](const auto* lhs, const auto* rhs) {
    return lhs->isAncestorOf(rhs);
  });

  auto result = std::vector<Node*>{};
  result.reserve(nodes.size());
  result.push_back(nodes.front());

  for (size_t i = 1; i < nodes.size(); ++i)
  {
    auto* node = nodes[i];
    if (!node->isDescendantOf(result))
    {
      result.push_back(node);
    }
  }

  return result;
}

void closeRemovedGroups(Map& map, const std::map<Node*, std::vector<Node*>>& toRemove)
{
  const auto& editorContext = map.editorContext();
  for (const auto& [parent, nodes] : toRemove)
  {
    for (const Node* node : nodes)
    {
      if (node == editorContext.currentGroup())
      {
        closeGroup(map);
        closeRemovedGroups(map, toRemove);
        return;
      }
    }
  }
}

auto collectRemovableParents(const std::map<Node*, std::vector<Node*>>& nodes)
{
  auto result = std::map<Node*, std::vector<Node*>>{};
  for (const auto& [node, children] : nodes)
  {
    if (node->removeIfEmpty() && !node->hasChildren())
    {
      auto* parent = node->parent();
      ensure(parent != nullptr, "parent is not null");
      result[parent].push_back(node);
    }
  }
  return result;
}

} // namespace

Node* parentForNodes(const Map& map, const std::vector<Node*>& nodes)
{
  if (nodes.empty())
  {
    // No reference nodes, so return either the current group (if open) or current layer
    auto* result = static_cast<Node*>(map.editorContext().currentGroup());
    if (!result)
    {
      result = map.editorContext().currentLayer();
    }
    return result;
  }

  if (auto* parentGroup = findContainingGroup(nodes.at(0)))
  {
    return parentGroup;
  }

  auto* parentLayer = findContainingLayer(nodes.at(0));
  ensure(parentLayer != nullptr, "no parent layer");
  return parentLayer;
}

std::vector<Node*> addNodes(Map& map, const std::map<Node*, std::vector<Node*>>& nodes)
{
  for (const auto& [parent, children] : nodes)
  {
    assert(parent == map.world() || parent->isDescendantOf(map.world()));
    unused(parent);
  }

  auto transaction = Transaction{map, "Add Objects"};
  const auto result = map.executeAndStore(AddRemoveNodesCommand::add(nodes));
  if (!result->success())
  {
    transaction.cancel();
    return {};
  }

  setHasPendingChanges(collectGroupsOrContainers(kdl::map_keys(nodes)), true);

  const auto addedNodes = kdl::vec_flatten(kdl::map_values(nodes));
  ensureNodesVisible(map, addedNodes);
  ensureNodesUnlocked(map, addedNodes);
  if (!transaction.commit())
  {
    return {};
  }

  return addedNodes;
}

void duplicateSelectedNodes(Map& map)
{
  auto nodesToAdd = std::map<Node*, std::vector<Node*>>{};
  auto nodesToSelect = std::vector<Node*>{};
  auto newParentMap = std::map<Node*, Node*>{};

  for (auto* original : map.selection().nodes)
  {
    auto* suggestedParent = parentForNodes(map, {original});
    auto* clone = original->cloneRecursively(map.worldBounds());

    if (shouldCloneParentWhenCloningNode(original))
    {
      // e.g. original is a brush in a brush entity, so we need to clone the entity
      // (parent) see if the parent was already cloned and if not, clone it and store it
      auto* originalParent = original->parent();
      auto* newParent = static_cast<Node*>(nullptr);
      if (const auto it = newParentMap.find(originalParent); it != std::end(newParentMap))
      {
        // parent was already cloned
        newParent = it->second;
      }
      else
      {
        // parent was not cloned yet
        newParent = originalParent->clone(map.worldBounds());
        newParentMap.emplace(originalParent, newParent);
        nodesToAdd[suggestedParent].push_back(newParent);
      }

      // hierarchy will look like (parent -> child): suggestedParent -> newParent -> clone
      newParent->addChild(clone);
    }
    else
    {
      nodesToAdd[suggestedParent].push_back(clone);
    }

    nodesToSelect.push_back(clone);
  }

  resetLinkIdsOfNonGroupedNodes(nodesToAdd);
  copyAndSetLinkIds(nodesToAdd, *map.world(), map.logger());

  {
    auto transaction = Transaction{map, "Duplicate Objects"};
    map.deselectAll();

    if (addNodes(map, nodesToAdd).empty())
    {
      transaction.cancel();
      return;
    }

    map.selectNodes(nodesToSelect);
    if (!transaction.commit())
    {
      return;
    }
  }

  map.pushRepeatableCommand([&]() { duplicateSelectedNodes(map); });
}

bool reparentNodes(Map& map, const std::map<Node*, std::vector<Node*>>& nodesToAdd)
{
  if (!checkReparenting(nodesToAdd))
  {
    return false;
  }

  const auto nodesToRemove =
    parentChildrenMap(kdl::vec_flatten(kdl::map_values(nodesToAdd)));

  const auto changedLinkedGroups = collectGroupsOrContainers(
    kdl::vec_concat(kdl::map_keys(nodesToAdd), kdl::map_keys(nodesToRemove)));

  if (!checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    return false;
  }

  auto transaction = mdl::Transaction{map, "Reparent Objects"};

  // This handles two main cases:
  // - creating brushes in a hidden layer, and then grouping / ungrouping them keeps
  //   them visible
  // - creating brushes in a hidden layer, then moving them to a hidden layer, should
  //   downgrade them to inherited and hide them
  for (auto& [newParent, nodes] : nodesToAdd)
  {
    auto* newParentLayer = mdl::findContainingLayer(newParent);

    const auto nodesToDowngrade = mdl::collectNodesAndDescendants(
      nodes,
      [&](mdl::Object* node) { return node->containingLayer() != newParentLayer; });

    downgradeUnlockedToInherit(map, nodesToDowngrade);
    downgradeShownToInherit(map, nodesToDowngrade);
  }

  // Reset link IDs of nodes being reparented, but don't recurse into nested groups
  map.executeAndStore(std::make_unique<mdl::SetLinkIdsCommand>(
    "Set Link ID", setLinkIdsForReparentingNodes(nodesToAdd)));

  const auto result =
    map.executeAndStore(ReparentNodesCommand::reparent(nodesToAdd, nodesToRemove));
  if (!result->success())
  {
    transaction.cancel();
    return false;
  }

  setHasPendingChanges(changedLinkedGroups, true);

  auto removableNodes = collectRemovableParents(nodesToRemove);
  while (!removableNodes.empty())
  {
    setHasPendingChanges(
      collectContainingGroups(kdl::vec_flatten(kdl::map_values(removableNodes))), true);

    closeRemovedGroups(map, removableNodes);
    map.executeAndStore(mdl::AddRemoveNodesCommand::remove(removableNodes));

    removableNodes = collectRemovableParents(removableNodes);
  }

  return transaction.commit();
}

void removeNodes(Map& map, const std::vector<Node*>& nodes)
{
  auto removableNodes = parentChildrenMap(removeImplicitelyRemovedNodes(nodes));

  auto transaction = Transaction{map, "Remove Objects"};
  while (!removableNodes.empty())
  {
    setHasPendingChanges(collectGroupsOrContainers(kdl::map_keys(removableNodes)), true);

    closeRemovedGroups(map, removableNodes);
    map.executeAndStore(AddRemoveNodesCommand::remove(removableNodes));

    removableNodes = collectRemovableParents(removableNodes);
  }

  assertResult(transaction.commit());
}

void removeSelectedNodes(Map& map)
{
  const auto nodes = map.selection().nodes;

  auto transaction = Transaction{map, "Delete Objects"};
  map.deselectAll();
  removeNodes(map, nodes);
  assertResult(transaction.commit());
}

bool Map::updateNodeContents(
  const std::string& commandName,
  std::vector<std::pair<Node*, NodeContents>> nodesToSwap,
  std::vector<GroupNode*> changedLinkedGroups)
{

  if (!checkLinkedGroupsToUpdate(changedLinkedGroups))
  {
    return false;
  }

  auto transaction = Transaction{*this};
  const auto result = executeAndStore(
    std::make_unique<SwapNodeContentsCommand>(commandName, std::move(nodesToSwap)));

  if (!result->success())
  {
    transaction.cancel();
    return false;
  }

  setHasPendingChanges(changedLinkedGroups, true);
  return transaction.commit();
}

bool Map::updateNodeContents(
  const std::string& commandName, std::vector<std::pair<Node*, NodeContents>> nodesToSwap)
{
  auto changedLinkedGroups = collectContainingGroups(
    kdl::vec_transform(nodesToSwap, [](const auto& p) { return p.first; }));

  return updateNodeContents(
    commandName, std::move(nodesToSwap), std::move(changedLinkedGroups));
}

} // namespace tb::mdl
