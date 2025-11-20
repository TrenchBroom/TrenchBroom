/*
 Copyright (C) 2023 Kristian Duske

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

#include "LinkedGroupUtils.h"

#include "Contracts.h"
#include "Uuid.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/NodeContents.h"
#include "mdl/NodeQueries.h"

#include "kd/ranges/chunk_by_view.h"
#include "kd/ranges/to.h"
#include "kd/ranges/zip_transform_view.h"
#include "kd/result.h"
#include "kd/result_fold.h"
#include "kd/task_manager.h"

#include <algorithm>
#include <string_view>
#include <unordered_map>

namespace tb::mdl
{

std::vector<Node*> collectNodesWithLinkId(
  const std::vector<Node*>& nodes, const std::string& linkId)
{
  return collectNodesAndDescendants(
    nodes,
    kdl::overload(
      [&](const GroupNode* groupNode) { return groupNode->linkId() == linkId; },
      [&](const EntityNode* entityNode) { return entityNode->linkId() == linkId; },
      [&](const BrushNode* brushNode) { return brushNode->linkId() == linkId; },
      [&](const PatchNode* patchNode) { return patchNode->linkId() == linkId; }));
}

std::vector<GroupNode*> collectGroupsWithLinkId(
  const std::vector<Node*>& nodes, const std::string& linkId)
{
  return kdl::vec_static_cast<GroupNode*>(
    collectNodesAndDescendants(nodes, kdl::overload([&](const GroupNode* groupNode) {
                                 return groupNode->linkId() == linkId;
                               })));
}

std::vector<std::string> collectLinkedGroupIds(const std::vector<Node*>& nodes)
{
  auto result = std::vector<std::string>{};

  Node::visitAll(
    nodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, const GroupNode* groupNode) {
        result.push_back(groupNode->linkId());
        groupNode->visitChildren(thisLambda);
      },
      [](const EntityNode*) {},
      [](const BrushNode*) {},
      [](const PatchNode*) {}));
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

std::vector<std::string> collectLinkedGroupIds(const Node& node)
{
  return collectLinkedGroupIds({const_cast<Node*>(&node)});
}

std::vector<std::string> collectParentLinkedGroupIds(const Node& parentNode)
{
  auto result = std::vector<std::string>{};
  const auto* currentNode = &parentNode;
  while (currentNode)
  {
    if (const auto* currentGroupNode = dynamic_cast<const GroupNode*>(currentNode))
    {
      result.push_back(currentGroupNode->linkId());
    }
    currentNode = currentNode->parent();
  }
  return result;
}

namespace
{
std::vector<GroupNode*> collectContainingGroups(Node& node)
{
  auto result = std::vector<GroupNode*>{};

  auto* currentNode = findContainingGroup(&node);
  while (currentNode)
  {
    result.push_back(currentNode);
    currentNode = findContainingGroup(currentNode);
  }

  return result;
}
} // namespace

SelectionResult nodeSelectionWithLinkedGroupConstraints(
  WorldNode& world, const std::vector<Node*>& nodes)
{
  auto groupsToLock = kdl::vector_set<GroupNode*>{};
  auto groupsToKeepUnlocked = kdl::vector_set<GroupNode*>{};

  // collects subset of `nodes` which pass the constraints
  auto nodesToSelect = std::vector<Node*>{};

  for (auto* node : nodes)
  {
    const auto containingGroupNodes = collectContainingGroups(*node);

    const bool isNodeInGroupsToLock = std::ranges::any_of(
      containingGroupNodes,
      [&](auto* groupNode) { return groupsToLock.count(groupNode) == 1u; });

    if (isNodeInGroupsToLock)
    {
      // don't bother trying to select this node.
      continue;
    }

    // we will allow selection of `node`, but we need to implicitly lock
    // any other groups in the link sets of the groups listed in
    // `linkedGroupsContainingNode`.

    // first check if we've already processed all of these
    const auto areAncestorGroupsHandled = std::ranges::all_of(
      containingGroupNodes,
      [&](auto* groupNode) { return groupsToKeepUnlocked.count(groupNode) == 1u; });

    if (!areAncestorGroupsHandled)
    {
      // for each `group` in `linkedGroupsContainingNode`,
      // implicitly lock other groups in the link set of `groupNode`, but keep
      // `groupNode` itself unlocked.
      for (auto* groupNode : containingGroupNodes)
      {
        // find the others and add them to the lock list
        for (auto* otherGroup : collectGroupsWithLinkId({&world}, groupNode->linkId()))
        {
          if (otherGroup == groupNode)
          {
            continue;
          }
          groupsToLock.insert(otherGroup);
        }
        groupsToKeepUnlocked.insert(groupNode);
      }
    }

    nodesToSelect.push_back(node);
  }

  return {nodesToSelect, groupsToLock.release_data()};
}

FaceSelectionResult faceSelectionWithLinkedGroupConstraints(
  WorldNode& world, const std::vector<BrushFaceHandle>& faces)
{
  const auto nodes = faces
                     | std::views::transform([](auto handle) { return handle.node(); })
                     | kdl::ranges::to<std::vector<Node*>>();
  auto constrainedNodes = nodeSelectionWithLinkedGroupConstraints(world, nodes);

  const auto nodesToSelect = kdl::vector_set<Node*>{constrainedNodes.nodesToSelect};

  auto facesToSelect = std::vector<BrushFaceHandle>{};
  for (const auto& handle : faces)
  {
    if (nodesToSelect.count(handle.node()) != 0)
    {
      facesToSelect.push_back(handle);
    }
  }

  return {std::move(facesToSelect), std::move(constrainedNodes.groupsToLock)};
}

namespace
{
Result<std::unique_ptr<Node>> cloneAndTransformRecursive(
  const Node* nodeToClone,
  std::unordered_map<const Node*, NodeContents>& origNodeToTransformedContents,
  const vm::bbox3d& worldBounds)
{
  // First, clone `n`, and move in the new (transformed) content which was
  // prepared for it above
  auto clone = nodeToClone->accept(kdl::overload(
    [](const WorldNode*) -> std::unique_ptr<Node> { contract_assert(false); },
    [](const LayerNode*) -> std::unique_ptr<Node> { contract_assert(false); },
    [&](const GroupNode* groupNode) -> std::unique_ptr<Node> {
      auto& group = std::get<Group>(origNodeToTransformedContents.at(groupNode).get());
      auto newGroupNode = std::make_unique<GroupNode>(std::move(group));
      newGroupNode->setLinkId(groupNode->linkId());
      return newGroupNode;
    },
    [&](const EntityNode* entityNode) -> std::unique_ptr<Node> {
      auto& entity = std::get<Entity>(origNodeToTransformedContents.at(entityNode).get());
      auto newEntityNode = std::make_unique<EntityNode>(std::move(entity));
      newEntityNode->setLinkId(entityNode->linkId());
      return newEntityNode;
    },
    [&](const BrushNode* brushNode) -> std::unique_ptr<Node> {
      auto& brush = std::get<Brush>(origNodeToTransformedContents.at(brushNode).get());
      auto newBrushNode = std::make_unique<BrushNode>(std::move(brush));
      newBrushNode->setLinkId(brushNode->linkId());
      return newBrushNode;
    },
    [&](const PatchNode* patchNode) -> std::unique_ptr<Node> {
      auto& patch =
        std::get<BezierPatch>(origNodeToTransformedContents.at(patchNode).get());
      auto newPatchNode = std::make_unique<PatchNode>(std::move(patch));
      newPatchNode->setLinkId(patchNode->linkId());
      return newPatchNode;
    }));

  if (!worldBounds.contains(clone->logicalBounds()))
  {
    return Error{"Updating a linked node would exceed world bounds"};
  }

  return nodeToClone->children() | std::views::transform([&](const auto* childNode) {
           return cloneAndTransformRecursive(
             childNode, origNodeToTransformedContents, worldBounds);
         })
         | kdl::fold | kdl::transform([&](auto childClones) {
             for (auto& childClone : childClones)
             {
               clone->addChild(childClone.release());
             }

             return std::move(clone);
           });
}

/**
 * Given a node, clones its children recursively and applies the given transform.
 *
 * Returns a vector of the cloned direct children of `node`.
 */
Result<std::vector<std::unique_ptr<Node>>> cloneAndTransformChildren(
  const Node& node,
  const vm::bbox3d& worldBounds,
  const vm::mat4x4d& transformation,
  kdl::task_manager& taskManager)
{
  auto nodesToClone = collectDescendants(std::vector{&node});

  using TransformResult = Result<std::pair<const Node*, NodeContents>>;

  // In parallel, produce pairs { node pointer, transformed contents } from the nodes in
  // `nodesToClone`
  auto tasks =
    nodesToClone | std::views::transform([&](const auto& nodeToTransform) {
      return std::function{[&]() {
        return nodeToTransform->accept(kdl::overload(
          [](const WorldNode*) -> TransformResult { contract_assert(false); },
          [](const LayerNode*) -> TransformResult { contract_assert(false); },
          [&](const GroupNode* groupNode) -> TransformResult {
            auto group = groupNode->group();
            group.transform(transformation);
            return std::make_pair(nodeToTransform, NodeContents{std::move(group)});
          },
          [&](const EntityNode* entityNode) -> TransformResult {
            const auto updateAngleProperty =
              entityNode->entityPropertyConfig().updateAnglePropertyAfterTransform;
            auto entity = entityNode->entity();
            entity.transform(transformation, updateAngleProperty);
            return std::make_pair(nodeToTransform, NodeContents{std::move(entity)});
          },
          [&](const BrushNode* brushNode) -> TransformResult {
            auto brush = brushNode->brush();
            return brush.transform(worldBounds, transformation, true)
                   | kdl::and_then([&]() -> TransformResult {
                       return std::make_pair(
                         nodeToTransform, NodeContents{std::move(brush)});
                     });
          },
          [&](const PatchNode* patchNode) -> TransformResult {
            auto patch = patchNode->patch();
            patch.transform(transformation);
            return std::make_pair(nodeToTransform, NodeContents{std::move(patch)});
          }));
      }};
    });

  return taskManager.run_tasks_and_wait(tasks) | kdl::fold
         | kdl::or_else(
           [](const auto&) -> Result<std::vector<std::pair<const Node*, NodeContents>>> {
             return Error{"Failed to transform a linked node"};
           })
         | kdl::and_then(
           [&](auto origNodeAndTransformedContents)
             -> Result<std::vector<std::unique_ptr<Node>>> {
             // Move into map for easier lookup
             auto resultsMap = std::unordered_map<const Node*, NodeContents>{
               origNodeAndTransformedContents.begin(),
               origNodeAndTransformedContents.end()};
             origNodeAndTransformedContents.clear();

             // Do a recursive traversal of the input node tree again,
             // creating a matching tree structure, and move in the contents
             // we've transformed above.
             return node.children() | std::views::transform([&](const auto* childNode) {
                      return cloneAndTransformRecursive(
                        childNode, resultsMap, worldBounds);
                    })
                    | kdl::fold;
           });
}

auto makeLinkIdToNodeMap(const std::vector<Node*>& nodes)
{
  auto result = std::unordered_map<std::string_view, const Node*>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, const GroupNode* groupNode) {
        result[groupNode->linkId()] = groupNode;
        groupNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, const EntityNode* entityNode) {
        result[entityNode->linkId()] = entityNode;
        entityNode->visitChildren(thisLambda);
      },
      [&](const BrushNode* brushNode) { result[brushNode->linkId()] = brushNode; },
      [&](const PatchNode* patchNode) { result[patchNode->linkId()] = patchNode; }));
  return result;
}

template <typename N>
const N* getCorrespondingNode(
  const std::unordered_map<std::string_view, const Node*>& correspondingNodes,
  const std::string_view linkId)
{
  auto it = correspondingNodes.find(linkId);
  return it != correspondingNodes.end() ? dynamic_cast<const N*>(it->second) : nullptr;
}

template <typename T>
void preserveGroupNames(
  const std::vector<T>& clonedNodes,
  const std::unordered_map<std::string_view, const Node*>& correspondingNodes)
{
  return Node::visitAll(
    clonedNodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, GroupNode* groupNode) {
        if (
          const auto* correspondingNode =
            getCorrespondingNode<GroupNode>(correspondingNodes, groupNode->linkId()))
        {
          auto group = groupNode->group();
          group.setName(correspondingNode->group().name());
          groupNode->setGroup(std::move(group));
        }
        groupNode->visitChildren(thisLambda);
      },
      [](const EntityNode*) {},
      [](const BrushNode*) {},
      [](const PatchNode*) {}));
}

void preserveEntityProperties(
  EntityNode& clonedEntityNode, const EntityNode& correspondingEntityNode)
{
  if (
    clonedEntityNode.entity().protectedProperties().empty()
    && correspondingEntityNode.entity().protectedProperties().empty())
  {
    return;
  }

  auto clonedEntity = clonedEntityNode.entity();
  const auto& correspondingEntity = correspondingEntityNode.entity();

  const auto allProtectedProperties = kdl::vec_sort_and_remove_duplicates(kdl::vec_concat(
    clonedEntity.protectedProperties(), correspondingEntity.protectedProperties()));

  clonedEntity.setProtectedProperties(correspondingEntity.protectedProperties());

  for (const auto& propertyKey : allProtectedProperties)
  {
    // this can change the order of properties
    clonedEntity.removeProperty(propertyKey);
    if (const auto* propertyValue = correspondingEntity.property(propertyKey))
    {
      clonedEntity.addOrUpdateProperty(propertyKey, *propertyValue);
    }
  }

  clonedEntityNode.setEntity(std::move(clonedEntity));
}

template <typename T>
void preserveEntityProperties(
  const std::vector<T>& clonedNodes,
  const std::unordered_map<std::string_view, const Node*>& correspondingNodes)
{
  return Node::visitAll(
    clonedNodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [&](EntityNode* entityNode) {
        if (
          const auto* correspondingNode =
            getCorrespondingNode<EntityNode>(correspondingNodes, entityNode->linkId()))
        {
          preserveEntityProperties(*entityNode, *correspondingNode);
        }
      },
      [](const BrushNode*) {},
      [](const PatchNode*) {}));
}
} // namespace

Result<UpdateLinkedGroupsResult> updateLinkedGroups(
  const GroupNode& sourceGroupNode,
  const std::vector<GroupNode*>& targetGroupNodes,
  const vm::bbox3d& worldBounds,
  kdl::task_manager& taskManager)
{
  const auto& sourceGroup = sourceGroupNode.group();
  const auto invertedSourceTransformation = vm::invert(sourceGroup.transformation());
  if (!invertedSourceTransformation)
  {
    return Error{"Group transformation is not invertible"};
  }

  const auto targetGroupNodesToUpdate =
    kdl::vec_erase(targetGroupNodes, &sourceGroupNode);
  return targetGroupNodesToUpdate | std::views::transform([&](auto* targetGroupNode) {
           const auto transformation =
             targetGroupNode->group().transformation() * *invertedSourceTransformation;
           return cloneAndTransformChildren(
                    sourceGroupNode, worldBounds, transformation, taskManager)
                  | kdl::transform([&](auto newChildren) {
                      const auto linkIdToNodeMap =
                        makeLinkIdToNodeMap(targetGroupNode->children());
                      preserveGroupNames(newChildren, linkIdToNodeMap);
                      preserveEntityProperties(newChildren, linkIdToNodeMap);
                      return std::pair{
                        static_cast<Node*>(targetGroupNode), std::move(newChildren)};
                    });
         })
         | kdl::fold;
}

namespace
{

enum class GroupRecursionMode
{
  Shallow,
  Deep,
};

template <typename N1, typename N2>
Result<N1*> tryCast(N2& targetNode)
{
  auto* targetNodeCasted = dynamic_cast<N1*>(&targetNode);
  return targetNodeCasted ? Result<N1*>{targetNodeCasted}
                          : Result<N1*>{Error{"Inconsistent linked group structure"}};
}

template <typename SourceNode, typename TargetNode, typename F>
Result<void> visitNodesPerPosition(
  const SourceNode& sourceNode,
  TargetNode& targetNode,
  const F& f,
  const GroupRecursionMode recursionMode,
  const size_t depth = 0)
{
  return sourceNode.accept(kdl::overload(
    [&](const WorldNode* sourceWorldNode) {
      return tryCast<WorldNode>(targetNode)
             | kdl::transform(
               [&](WorldNode* targetWorldNode) { f(*sourceWorldNode, *targetWorldNode); })
             | kdl::and_then([&]() {
                 return visitChildrenPerPosition(
                   sourceNode, targetNode, f, recursionMode, depth);
               });
      ;
    },
    [&](const LayerNode* sourceLayerNode) {
      return tryCast<LayerNode>(targetNode)
             | kdl::transform(
               [&](LayerNode* targetLayerNode) { f(*sourceLayerNode, *targetLayerNode); })
             | kdl::and_then([&]() {
                 return visitChildrenPerPosition(
                   sourceNode, targetNode, f, recursionMode, depth);
               });
      ;
    },
    [&](const GroupNode* sourceGroupNode) {
      return depth == 0 || recursionMode == GroupRecursionMode::Deep
               ? tryCast<GroupNode>(targetNode)
                   | kdl::transform([&](GroupNode* targetGroupNode) {
                       f(*sourceGroupNode, *targetGroupNode);
                     })
                   | kdl::and_then([&]() {
                       return visitChildrenPerPosition(
                         sourceNode, targetNode, f, recursionMode, depth + 1);
                     })
               : Result<void>{};
      ;
    },
    [&](const EntityNode* sourceEntityNode) {
      return tryCast<EntityNode>(targetNode)
             | kdl::transform([&](EntityNode* targetEntityNode) {
                 f(*sourceEntityNode, *targetEntityNode);
               })
             | kdl::and_then([&]() {
                 return visitChildrenPerPosition(
                   sourceNode, targetNode, f, recursionMode, depth);
               });
      ;
    },
    [&](const BrushNode* sourceBrushNode) {
      return tryCast<BrushNode>(targetNode)
             | kdl::transform([&](BrushNode* targetBrushNode) {
                 f(*sourceBrushNode, *targetBrushNode);
               });
    },
    [&](const PatchNode* sourcePatchNode) {
      return tryCast<PatchNode>(targetNode)
             | kdl::transform([&](PatchNode* targetPatchNode) {
                 f(*sourcePatchNode, *targetPatchNode);
               });
    }));
}

template <typename SourceNode, typename TargetNode, typename F>
Result<void> visitChildrenPerPosition(
  SourceNode& sourceNode,
  TargetNode& targetNode,
  const F& f,
  const GroupRecursionMode recursionMode,
  const size_t depth)
{
  if (sourceNode.childCount() != targetNode.childCount())
  {
    return Error{"Inconsistent linked group structure"};
  }

  return kdl::views::zip_transform(
           [&](auto& sourceChild, auto& targetChild) {
             return visitNodesPerPosition(
               *sourceChild, *targetChild, f, recursionMode, depth);
           },
           sourceNode.children(),
           targetNode.children())
         | kdl::fold;
}

Result<void> copyLinkIds(
  const GroupNode& sourceRootNode,
  GroupNode& targetRootNode,
  const GroupRecursionMode recursionMode,
  std::unordered_map<Node*, std::string>& linkIds)
{
  return visitNodesPerPosition(
    sourceRootNode,
    targetRootNode,
    kdl::overload(
      [&](const WorldNode&, const WorldNode&) {},
      [&](const LayerNode&, const LayerNode&) {},
      [&](const GroupNode& sourceGroupNode, GroupNode& targetGroupNode) {
        linkIds[&targetGroupNode] = sourceGroupNode.linkId();
      },
      [&](const EntityNode& sourceEntityNode, EntityNode& targetEntityNode) {
        linkIds[&targetEntityNode] = sourceEntityNode.linkId();
      },
      [&](const BrushNode& sourceBrushNode, BrushNode& targetBrushNode) {
        linkIds[&targetBrushNode] = sourceBrushNode.linkId();
      },
      [&](const PatchNode& sourcePatchNode, PatchNode& targetPatchNode) {
        linkIds[&targetPatchNode] = sourcePatchNode.linkId();
      }),
    recursionMode);
}

template <typename R>
Result<std::unordered_map<Node*, std::string>> copyLinkIds(
  const GroupNode& sourceGroupNode,
  const R& targetGroupNodes,
  const GroupRecursionMode recursionMode)
{
  auto linkIds = std::unordered_map<Node*, std::string>{};
  return targetGroupNodes | std::views::transform([&](auto* targetGroupNode) {
           return copyLinkIds(sourceGroupNode, *targetGroupNode, recursionMode, linkIds);
         })
         | kdl::fold | kdl::transform([&]() { return std::move(linkIds); });
}

template <typename R>
Result<std::unordered_map<Node*, std::string>> copyLinkIds(
  const R& groupNodes, const GroupRecursionMode recursionMode)
{
  if (groupNodes.empty())
  {
    return Error{"Link set must contain at least one group"};
  }

  return copyLinkIds(
    *groupNodes.front(), groupNodes | std::views::drop(1), recursionMode);
}

template <typename R>
void setLinkIds(
  Result<std::unordered_map<Node*, std::string>> linkIdResult,
  const R& groups,
  std::vector<Error>& errors)
{
  std::move(linkIdResult) | kdl::transform([&](auto linkIds) {
    for (auto& [node, linkId] : linkIds)
    {
      node->accept(kdl::overload(
        [](const WorldNode*) {},
        [](const LayerNode*) {},
        [&](Object* object) { object->setLinkId(std::move(linkId)); }));
    }
  }) | kdl::transform_error([&](auto e) {
    for (auto* linkedGroupNode : groups)
    {
      auto group = linkedGroupNode->group();
      group.setTransformation(vm::mat4x4d::identity());
      linkedGroupNode->setGroup(std::move(group));
      linkedGroupNode->setLinkId(generateUuid());
    }
    errors.push_back(std::move(e));
  });
}

void resetLinkIds(GroupNode& rootNode)
{
  rootNode.setLinkId(generateUuid());
  rootNode.visitChildren(kdl::overload(
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

} // namespace

std::vector<Error> initializeLinkIds(const std::vector<Node*>& nodes)
{
  const auto allGroupNodes =
    kdl::vec_sort(collectGroups(nodes), compareGroupNodesByLinkId);
  auto groupNodesByLinkId =
    allGroupNodes | kdl::views::chunk_by([](const auto* lhs, const auto* rhs) {
      return lhs->linkId() == rhs->linkId();
    });

  auto errors = std::vector<Error>{};
  for (const auto groupNodesWithId : groupNodesByLinkId)
  {
    // skip any link IDs with only one group
    if (
      groupNodesWithId.begin() != groupNodesWithId.end()
      && std::next(groupNodesWithId.begin()) != groupNodesWithId.end())
    {
      setLinkIds(
        copyLinkIds(groupNodesWithId, GroupRecursionMode::Deep),
        groupNodesWithId,
        errors);
    }
  }
  return errors;
}

void resetLinkIds(const std::vector<GroupNode*>& groupNodes)
{
  for (auto* groupNode : groupNodes)
  {
    resetLinkIds(*groupNode);
  }
}

Result<std::unordered_map<Node*, std::string>> copyAndReturnLinkIds(
  const GroupNode& sourceGroupNode, const std::vector<GroupNode*>& targetGroupNodes)
{
  return copyLinkIds(sourceGroupNode, targetGroupNodes, GroupRecursionMode::Deep);
}

std::vector<Error> copyAndSetLinkIds(
  const GroupNode& sourceGroupNode, const std::vector<GroupNode*>& targetGroupNodes)
{
  auto errors = std::vector<Error>{};
  setLinkIds(
    copyLinkIds(sourceGroupNode, targetGroupNodes, GroupRecursionMode::Shallow),
    targetGroupNodes,
    errors);
  return errors;
}

std::vector<Error> copyAndSetLinkIdsBeforeAddingNodes(
  const std::map<Node*, std::vector<Node*>>& nodesToAdd, WorldNode& worldNode)
{
  // Recursively collect all groups to add
  const auto groupsToAdd = kdl::vec_sort(
    collectGroups(
      nodesToAdd | std::views::values | std::views::join
      | kdl::ranges::to<std::vector>()),
    compareGroupNodesByLinkId);

  auto groupNodesByLinkId =
    groupsToAdd | kdl::views::chunk_by([](const auto* lhs, const auto* rhs) {
      return lhs->linkId() == rhs->linkId();
    });

  auto errors = std::vector<Error>{};
  for (const auto& linkedGroupsToAdd : groupNodesByLinkId)
  {
    const auto& linkId = linkedGroupsToAdd.front()->linkId();
    const auto existingLinkedNodes = collectNodesWithLinkId({&worldNode}, linkId);

    if (existingLinkedNodes.size() == 1)
    {
      // Unlink the added nodes because we don't want to create linked duplicates
      resetLinkIds({linkedGroupsToAdd.front()});

      if (std::next(linkedGroupsToAdd.begin()) != linkedGroupsToAdd.end())
      {
        // But keep the added linked groups mutually linked
        copyAndSetLinkIds(
          *linkedGroupsToAdd.front(),
          std::vector(std::next(linkedGroupsToAdd.begin()), linkedGroupsToAdd.end()));
      }
    }
    else if (existingLinkedNodes.size() > 1)
    {
      // Keep the pasted nodes linked to their originals, but validate the structure
      if (
        auto* existingLinkedGroup = dynamic_cast<GroupNode*>(existingLinkedNodes.front()))
      {
        errors = kdl::vec_concat(
          std::move(errors),
          copyAndSetLinkIds(
            *existingLinkedGroup,
            std::vector(linkedGroupsToAdd.begin(), linkedGroupsToAdd.end())));
      }
    }
  }

  return errors;
}

} // namespace tb::mdl
