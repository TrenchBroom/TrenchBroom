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

#pragma once

#include "FloatType.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/NodeVisitor.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "Result.h"
#include "Uuid.h"

#include "kdl/overload.h"
#include "kdl/vector_utils.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace TrenchBroom::Model
{

std::vector<Node*> collectNodesWithLinkId(
  const std::vector<Node*>& nodes, const std::string& linkId);

template <typename N>
std::vector<N*> collectLinkedNodes(const std::vector<Node*>& nodes, const N& node)
{
  return kdl::vec_static_cast<N*>(node.accept(kdl::overload(
    [](const WorldNode*) { return std::vector<Node*>{}; },
    [](const LayerNode*) { return std::vector<Node*>{}; },
    [&](const GroupNode* groupNode) {
      return collectNodesWithLinkId(nodes, groupNode->group().linkId());
    },
    [&](const EntityNode* entityNode) {
      return collectNodesWithLinkId(nodes, entityNode->entity().linkId());
    },
    [&](const BrushNode* brushNode) {
      return collectNodesWithLinkId(nodes, brushNode->brush().linkId());
    },
    [&](const PatchNode* patchNode) {
      return collectNodesWithLinkId(nodes, patchNode->patch().linkId());
    })));
}

std::vector<GroupNode*> collectGroupsWithLinkId(
  const std::vector<Node*>& nodes, const std::string& linkId);

std::vector<std::string> collectLinkedGroupIds(const std::vector<Node*>& nodes);
std::vector<std::string> collectLinkedGroupIds(const Node& node);

std::vector<std::string> collectParentLinkedGroupIds(const Node& parent);

struct SelectionResult
{
  std::vector<Node*> nodesToSelect;
  std::vector<GroupNode*> groupsToLock;
};

/**
 * Given a list of `nodes` the user wants to select, returns the subset that we should
 * allow selection of, as well as a list of linked groups to lock.
 *
 * - Attempting to select nodes inside a linked group will propose locking all other
 * groups in that link set. This is intended to prevent users from making conflicting
 * commands as well as communicate which specific linked group they are modifying.
 *
 * - If `nodes` contains members of different groups in the same link set,
 *  only those in the first group will be allowed to be selected ("first" in the order of
 * `nodes`).
 *
 * Note: no changes are made, just the proposed selection and locking is returned.
 */
SelectionResult nodeSelectionWithLinkedGroupConstraints(
  WorldNode& world, const std::vector<Node*>& nodes);

struct FaceSelectionResult
{
  std::vector<BrushFaceHandle> facesToSelect;
  std::vector<GroupNode*> groupsToLock;
};

/**
 * Given a list of `faces` the user wants to select, returns the subset that we should
 * allow selection of, as well as a list of linked groups to lock.
 *
 * @see nodeSelectionWithLinkedGroupConstraints()
 */
FaceSelectionResult faceSelectionWithLinkedGroupConstraints(
  WorldNode& world, const std::vector<BrushFaceHandle>& faces);

using UpdateLinkedGroupsResult =
  std::vector<std::pair<Node*, std::vector<std::unique_ptr<Node>>>>;

/**
 * Updates the given target group nodes from the given source group node.
 *
 * The children of the source node are cloned (recursively) and transformed into the
 * target nodes by means of the recorded transformations of the source group and the
 * corresponding target groups.
 *
 * Depending on the protected property keys of the cloned entities and their corresponding
 * entities in the target groups, some entity property changes may not be propagated from
 * the source group to the target groups. Specifically, if an entity property is protected
 * in either the cloned entity or its corresponding entity in a target group, then changes
 * to that entity property incl. removal are not propagated. This also applies to numbered
 * properties, i.e. properties whose names end in a number. So if the entity property
 * "target" is protected, then changes to the property "target2" are not propagated or
 * overwritten during propagation.
 *
 * If this operation fails for any child and target group, then an error is returned. The
 * operation can fail if any of the following conditions arises:
 *
 * - the transformation of the source group node is not invertible
 * - transforming any of the source node's children fails
 * - any of the transformed children is no longer within the world bounds
 *
 * If this operation succeeds, a vector of pairs is returned where each pair consists of
 * the target node that should be updated, and the new children that should replace the
 * target node's children.
 */
Result<UpdateLinkedGroupsResult> updateLinkedGroups(
  const GroupNode& sourceGroupNode,
  const std::vector<Model::GroupNode*>& targetGroupNodes,
  const vm::bbox3& worldBounds);

std::vector<Error> initializeLinkIds(const std::vector<Node*>& nodes);


Result<std::unordered_map<Node*, std::string>> copyAndReturnLinkIds(
  const GroupNode& sourceGroupNode, const std::vector<GroupNode*>& targetGroupNodes);

std::vector<Error> copyAndSetLinkIds(
  const GroupNode& sourceGroupNode, const std::vector<GroupNode*>& targetGroupNodes);

template <typename T>
T setNewLinkIdIf(T x, const bool setNewLinkId)
{
  if (setNewLinkId)
  {
    x.setLinkId(generateUuid());
  }
  return x;
}

} // namespace TrenchBroom::Model
