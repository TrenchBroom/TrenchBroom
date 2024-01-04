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

#include "Ensure.h"
#include "Error.h"
#include "Model/ModelUtils.h"
#include "Model/Node.h"
#include "Model/NodeContents.h"
#include "Model/NodeQueries.h"
#include "Uuid.h"

#include "kdl/grouped_range.h"
#include <kdl/parallel.h>
#include <kdl/result.h>
#include <kdl/result_fold.h>
#include <kdl/zip_iterator.h>

namespace TrenchBroom::Model
{

std::vector<Node*> collectNodesWithLinkId(
  const std::vector<Node*>& nodes, const std::string& linkId)
{
  return collectNodesAndDescendants(
    nodes,
    kdl::overload(
      [&](const GroupNode* groupNode) { return groupNode->group().linkId() == linkId; },
      [&](const EntityNode* entityNode) {
        return entityNode->entity().linkId() == linkId;
      },
      [&](const BrushNode* brushNode) { return brushNode->brush().linkId() == linkId; },
      [&](const PatchNode* patchNode) { return patchNode->patch().linkId() == linkId; }));
}

std::vector<GroupNode*> collectGroupsWithLinkId(
  const std::vector<Node*>& nodes, const std::string& linkId)
{
  return kdl::vec_static_cast<GroupNode*>(
    collectNodesAndDescendants(nodes, kdl::overload([&](const GroupNode* groupNode) {
                                 return groupNode->group().linkId() == linkId;
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
        result.push_back(groupNode->group().linkId());
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
      result.push_back(currentGroupNode->group().linkId());
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

    const bool isNodeInGroupsToLock = kdl::any_of(
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
    const auto areAncestorGroupsHandled = kdl::all_of(
      containingGroupNodes,
      [&](auto* groupNode) { return groupsToKeepUnlocked.count(groupNode) == 1u; });

    if (!areAncestorGroupsHandled)
    {
      // for each `group` in `linkedGroupsContainingNode`,
      // implicitly lock other groups in the link set of `groupNode`, but keep `groupNode`
      // itself unlocked.
      for (auto* groupNode : containingGroupNodes)
      {
        // find the others and add them to the lock list
        for (auto* otherGroup :
             collectGroupsWithLinkId({&world}, groupNode->group().linkId()))
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
  const std::vector<Node*> nodes =
    kdl::vec_transform(faces, [](auto handle) -> Node* { return handle.node(); });
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
  const vm::bbox3& worldBounds)
{
  // First, clone `n`, and move in the new (transformed) content which was
  // prepared for it above
  auto clone = nodeToClone->accept(kdl::overload(
    [](const WorldNode*) -> std::unique_ptr<Node> {
      ensure(false, "Linked group structure is valid");
    },
    [](const LayerNode*) -> std::unique_ptr<Node> {
      ensure(false, "Linked group structure is valid");
    },
    [&](const GroupNode* groupNode) -> std::unique_ptr<Node> {
      auto& group = std::get<Group>(origNodeToTransformedContents.at(groupNode).get());
      return std::make_unique<GroupNode>(std::move(group));
    },
    [&](const EntityNode* entityNode) -> std::unique_ptr<Node> {
      auto& entity = std::get<Entity>(origNodeToTransformedContents.at(entityNode).get());
      return std::make_unique<EntityNode>(std::move(entity));
    },
    [&](const BrushNode* brushNode) -> std::unique_ptr<Node> {
      auto& brush = std::get<Brush>(origNodeToTransformedContents.at(brushNode).get());
      return std::make_unique<BrushNode>(std::move(brush));
    },
    [&](const PatchNode* patchNode) -> std::unique_ptr<Node> {
      auto& patch =
        std::get<BezierPatch>(origNodeToTransformedContents.at(patchNode).get());
      return std::make_unique<PatchNode>(std::move(patch));
    }));

  if (!worldBounds.contains(clone->logicalBounds()))
  {
    return Error{"Updating a linked node would exceed world bounds"};
  }

  return kdl::fold_results(kdl::vec_transform(
                             nodeToClone->children(),
                             [&](const auto* childNode) {
                               return cloneAndTransformRecursive(
                                 childNode, origNodeToTransformedContents, worldBounds);
                             }))
    .transform([&](auto childClones) {
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
  const Node& node, const vm::bbox3& worldBounds, const vm::mat4x4& transformation)
{
  auto nodesToClone = collectDescendants(std::vector{&node});

  using TransformResult = Result<std::pair<const Node*, NodeContents>>;

  // In parallel, produce pairs { node pointer, transformed contents } from the nodes in
  // `nodesToClone`
  auto transformResults =
    kdl::vec_parallel_transform(nodesToClone, [&](const Node* nodeToTransform) {
      return nodeToTransform->accept(kdl::overload(
        [](const WorldNode*) -> TransformResult {
          ensure(false, "Linked group structure is valid");
        },
        [](const LayerNode*) -> TransformResult {
          ensure(false, "Linked group structure is valid");
        },
        [&](const GroupNode* groupNode) -> TransformResult {
          auto group = groupNode->group();
          group.transform(transformation);
          return std::make_pair(nodeToTransform, NodeContents{std::move(group)});
        },
        [&](const EntityNode* entityNode) -> TransformResult {
          auto entity = entityNode->entity();
          entity.transform(entityNode->entityPropertyConfig(), transformation);
          return std::make_pair(nodeToTransform, NodeContents{std::move(entity)});
        },
        [&](const BrushNode* brushNode) -> TransformResult {
          auto brush = brushNode->brush();
          return brush.transform(worldBounds, transformation, true)
            .and_then([&]() -> TransformResult {
              return std::make_pair(nodeToTransform, NodeContents{std::move(brush)});
            });
        },
        [&](const PatchNode* patchNode) -> TransformResult {
          auto patch = patchNode->patch();
          patch.transform(transformation);
          return std::make_pair(nodeToTransform, NodeContents{std::move(patch)});
        }));
    });

  return kdl::fold_results(std::move(transformResults))
    .or_else(
      [](const auto&) -> Result<std::vector<std::pair<const Node*, NodeContents>>> {
        return Error{"Failed to transform a linked node"};
      })
    .and_then(
      [&](auto origNodeAndTransformedContents)
        -> Result<std::vector<std::unique_ptr<Node>>> {
        // Move into map for easier lookup
        auto resultsMap = std::unordered_map<const Node*, NodeContents>{
          origNodeAndTransformedContents.begin(), origNodeAndTransformedContents.end()};
        origNodeAndTransformedContents.clear();

        // Do a recursive traversal of the input node tree again,
        // creating a matching tree structure, and move in the contents
        // we've transformed above.
        return kdl::fold_results(
          kdl::vec_transform(node.children(), [&](const auto* childNode) {
            return cloneAndTransformRecursive(childNode, resultsMap, worldBounds);
          }));
      });
}

template <typename T>
void preserveGroupNames(
  const std::vector<T>& clonedNodes, const std::vector<Node*>& correspondingNodes)
{
  auto clIt = std::begin(clonedNodes);
  auto coIt = std::begin(correspondingNodes);
  while (clIt != std::end(clonedNodes) && coIt != std::end(correspondingNodes))
  {
    auto& clonedNode = *clIt;
    const auto* correspondingNode = *coIt;

    clonedNode->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [&](GroupNode* clonedGroupNode) {
        if (
          const auto* correspondingGroupNode =
            dynamic_cast<const GroupNode*>(correspondingNode))
        {
          auto group = clonedGroupNode->group();
          group.setName(correspondingGroupNode->group().name());
          clonedGroupNode->setGroup(std::move(group));

          preserveGroupNames(
            clonedGroupNode->children(), correspondingGroupNode->children());
        }
      },
      [](EntityNode*) {},
      [](BrushNode*) {},
      [](PatchNode*) {}));

    ++clIt;
    ++coIt;
  }
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

  const auto entityPropertyConfig = clonedEntityNode.entityPropertyConfig();
  for (const auto& propertyKey : allProtectedProperties)
  {
    // this can change the order of properties
    clonedEntity.removeProperty(entityPropertyConfig, propertyKey);
    if (const auto* propertyValue = correspondingEntity.property(propertyKey))
    {
      clonedEntity.addOrUpdateProperty(entityPropertyConfig, propertyKey, *propertyValue);
    }
  }

  clonedEntityNode.setEntity(std::move(clonedEntity));
}

template <typename T>
void preserveEntityProperties(
  const std::vector<T>& clonedNodes, const std::vector<Node*>& correspondingNodes)
{
  auto clIt = std::begin(clonedNodes);
  auto coIt = std::begin(correspondingNodes);
  while (clIt != std::end(clonedNodes) && coIt != std::end(correspondingNodes))
  {
    auto& clonedNode =
      *clIt; // deduces either to std::unique_ptr<Node>& or Node*& depending on T
    const auto* correspondingNode = *coIt;

    clonedNode->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [&](GroupNode* clonedGroupNode) {
        if (
          const auto* correspondingGroupNode =
            dynamic_cast<const GroupNode*>(correspondingNode))
        {
          preserveEntityProperties(
            clonedGroupNode->children(), correspondingGroupNode->children());
        }
      },
      [&](EntityNode* clonedEntityNode) {
        if (
          const auto* correspondingEntityNode =
            dynamic_cast<const EntityNode*>(correspondingNode))
        {
          preserveEntityProperties(*clonedEntityNode, *correspondingEntityNode);
        }
      },
      [](BrushNode*) {},
      [](PatchNode*) {}));

    ++clIt;
    ++coIt;
  }
}
} // namespace

Result<UpdateLinkedGroupsResult> updateLinkedGroups(
  const GroupNode& sourceGroupNode,
  const std::vector<GroupNode*>& targetGroupNodes,
  const vm::bbox3& worldBounds)
{
  const auto& sourceGroup = sourceGroupNode.group();
  const auto [success, invertedSourceTransformation] =
    vm::invert(sourceGroup.transformation());
  if (!success)
  {
    return Error{"Group transformation is not invertible"};
  }

  const auto _invertedSourceTransformation = invertedSourceTransformation;
  const auto targetGroupNodesToUpdate =
    kdl::vec_erase(targetGroupNodes, &sourceGroupNode);
  return kdl::fold_results(
    kdl::vec_transform(targetGroupNodesToUpdate, [&](auto* targetGroupNode) {
      const auto transformation =
        targetGroupNode->group().transformation() * _invertedSourceTransformation;
      return cloneAndTransformChildren(sourceGroupNode, worldBounds, transformation)
        .transform([&](std::vector<std::unique_ptr<Node>>&& newChildren) {
          preserveGroupNames(newChildren, targetGroupNode->children());
          preserveEntityProperties(newChildren, targetGroupNode->children());

          return std::make_pair(
            static_cast<Node*>(targetGroupNode), std::move(newChildren));
        });
    }));
}

namespace
{

template <typename N1, typename N2>
Result<N1*> tryCast(N2& targetNode)
{
  auto* targetNodeCasted = dynamic_cast<N1*>(&targetNode);
  return targetNodeCasted ? Result<N1*>{targetNodeCasted}
                          : Result<N1*>{Error{"Inconsistent linked group structure"}};
}

template <typename SourceNode, typename TargetNode, typename F>
Result<void> visitNodesPerPosition(
  const SourceNode& sourceNode, TargetNode& targetNode, const F& f)
{
  return sourceNode
    .accept(kdl::overload(
      [&](const WorldNode* sourceWorldNode) {
        return tryCast<WorldNode>(targetNode).and_then([&](WorldNode* targetWorldNode) {
          return f(*sourceWorldNode, *targetWorldNode);
        });
      },
      [&](const LayerNode* sourceLayerNode) {
        return tryCast<LayerNode>(targetNode).and_then([&](LayerNode* targetLayerNode) {
          return f(*sourceLayerNode, *targetLayerNode);
        });
      },
      [&](const GroupNode* sourceGroupNode) {
        return tryCast<GroupNode>(targetNode).and_then([&](GroupNode* targetGroupNode) {
          return f(*sourceGroupNode, *targetGroupNode);
        });
      },
      [&](const EntityNode* sourceEntityNode) {
        return tryCast<EntityNode>(targetNode)
          .and_then([&](EntityNode* targetEntityNode) {
            return f(*sourceEntityNode, *targetEntityNode);
          });
      },
      [&](const BrushNode* sourceBrushNode) {
        return tryCast<BrushNode>(targetNode).and_then([&](BrushNode* targetBrushNode) {
          return f(*sourceBrushNode, *targetBrushNode);
        });
      },
      [&](const PatchNode* sourcePatchNode) {
        return tryCast<PatchNode>(targetNode).and_then([&](PatchNode* targetPatchNode) {
          return f(*sourcePatchNode, *targetPatchNode);
        });
      }))
    .and_then([&](const auto& recurse) {
      if (recurse)
      {
        return visitChildrenPerPosition(sourceNode, targetNode, f);
      }

      return Result<void>{};
    });
}

template <typename SourceNode, typename TargetNode, typename F>
Result<void> visitChildrenPerPosition(
  SourceNode& sourceNode, TargetNode& targetNode, const F& f)
{
  if (sourceNode.childCount() != targetNode.childCount())
  {
    return Error{"Inconsistent linked group structure"};
  }

  return kdl::fold_results(kdl::vec_transform(
    kdl::make_zip_range(sourceNode.children(), targetNode.children()),
    [&](auto childPair) {
      auto& [sourceChild, targetChild] = childPair;
      return visitNodesPerPosition(*sourceChild, *targetChild, f);
    }));
}

Result<void> copyLinkIds(
  const GroupNode& sourceRootNode,
  GroupNode& targetRootNode,
  std::unordered_map<Node*, std::string>& linkIds)
{
  return visitNodesPerPosition(
    sourceRootNode,
    targetRootNode,
    kdl::overload(
      [&](const WorldNode&, const WorldNode&) { return Result<bool>{true}; },
      [&](const LayerNode&, const LayerNode&) { return Result<bool>{true}; },
      [&](const GroupNode& sourceGroupNode, GroupNode& targetGroupNode) {
        linkIds[&targetGroupNode] = sourceGroupNode.group().linkId();
        return Result<bool>{true};
      },
      [&](const EntityNode& sourceEntityNode, EntityNode& targetEntityNode) {
        linkIds[&targetEntityNode] = sourceEntityNode.entity().linkId();
        return Result<bool>{true};
      },
      [&](const BrushNode& sourceBrushNode, BrushNode& targetBrushNode) {
        linkIds[&targetBrushNode] = sourceBrushNode.brush().linkId();
        return Result<bool>{false};
      },
      [&](const PatchNode& sourcePatchNode, PatchNode& targetPatchNode) {
        linkIds[&targetPatchNode] = sourcePatchNode.patch().linkId();
        return Result<bool>{false};
      }));
}

template <typename R>
Result<std::unordered_map<Node*, std::string>> copyLinkIds(
  const GroupNode& sourceGroupNode, const R& targetGroupNodes)
{
  auto linkIds = std::unordered_map<Node*, std::string>{};
  return kdl::fold_results(kdl::vec_transform(
                             targetGroupNodes,
                             [&](auto* targetGroupNode) {
                               return copyLinkIds(
                                 sourceGroupNode, *targetGroupNode, linkIds);
                             }))
    .transform([&]() { return std::move(linkIds); });
}

template <typename R>
Result<std::unordered_map<Node*, std::string>> copyLinkIds(const R& groupNodes)
{
  if (groupNodes.empty())
  {
    return Error{"Link set must contain at least one group"};
  }

  return copyLinkIds(
    *groupNodes.front(), kdl::range{std::next(groupNodes.begin()), groupNodes.end()});
}

template <typename R>
void setLinkIds(
  Result<std::unordered_map<Node*, std::string>> linkIdResult,
  const R& groups,
  std::vector<Error>& errors)
{
  std::move(linkIdResult)
    .transform([&](auto linkIds) {
      for (auto& [node, linkId] : linkIds)
      {
        node->accept(kdl::overload(
          [](const WorldNode*) {},
          [](const LayerNode*) {},
          [&linkId = linkId](GroupNode* groupNode) {
            auto group = groupNode->group();
            group.setLinkId(std::move(linkId));
            groupNode->setGroup(std::move(group));
          },
          [&linkId = linkId](EntityNode* entityNode) {
            auto entity = entityNode->entity();
            entity.setLinkId(std::move(linkId));
            entityNode->setEntity(std::move(entity));
          },
          [&linkId = linkId](BrushNode* brushNode) {
            auto brush = brushNode->brush();
            brush.setLinkId(std::move(linkId));
            brushNode->setBrush(std::move(brush));
          },
          [&linkId = linkId](PatchNode* patchNode) {
            auto patch = patchNode->patch();
            patch.setLinkId(std::move(linkId));
            patchNode->setPatch(std::move(patch));
          }));
      }
    })
    .transform_error([&](auto e) {
      for (auto* linkedGroupNode : groups)
      {
        auto group = linkedGroupNode->group();
        group.setLinkId(generateUuid());
        group.setTransformation(vm::mat4x4::identity());
        linkedGroupNode->setGroup(std::move(group));
      }
      errors.push_back(std::move(e));
    });
}

} // namespace

std::vector<Error> initializeLinkIds(const std::vector<Node*>& nodes)
{
  const auto allGroupNodes =
    kdl::vec_sort(collectGroups(nodes), [](const auto* lhs, const auto* rhs) {
      return lhs->group().linkId() < rhs->group().linkId();
    });
  const auto groupNodesByLinkId =
    kdl::make_grouped_range(allGroupNodes, [](const auto* lhs, const auto* rhs) {
      return lhs->group().linkId() == rhs->group().linkId();
    });

  auto errors = std::vector<Error>{};
  for (const auto groupNodesWithId : groupNodesByLinkId)
  {
    // skip any link IDs with only one group
    if (
      groupNodesWithId.begin() != groupNodesWithId.end()
      && std::next(groupNodesWithId.begin()) != groupNodesWithId.end())
    {
      setLinkIds(copyLinkIds(groupNodesWithId), groupNodesWithId, errors);
    }
  }
  return errors;
}

Result<std::unordered_map<Node*, std::string>> copyAndReturnLinkIds(
  const GroupNode& sourceGroupNode, const std::vector<GroupNode*>& targetGroupNodes)
{
  return copyLinkIds(sourceGroupNode, targetGroupNodes);
}

std::vector<Error> copyAndSetLinkIds(
  const GroupNode& sourceGroupNode, const std::vector<GroupNode*>& targetGroupNodes)
{
  auto errors = std::vector<Error>{};
  setLinkIds(copyLinkIds(sourceGroupNode, targetGroupNodes), targetGroupNodes, errors);
  return errors;
}

} // namespace TrenchBroom::Model
