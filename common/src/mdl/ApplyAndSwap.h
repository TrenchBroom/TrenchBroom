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

#pragma once

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/ModelUtils.h"
#include "mdl/Node.h"
#include "mdl/NodeContents.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

namespace tb::mdl
{

/**
 * Applies the given lambda to a copy of the contents of each of the given nodes and
 * returns a vector of pairs of the original node and the modified contents.
 *
 * The lambda L needs three overloads:
 * - bool operator()(Entity&);
 * - bool operator()(Brush&);
 * - bool operator()(BezierPatch&);
 *
 * The given node contents should be modified in place and the lambda should return true
 * if it was applied successfully and false otherwise.
 *
 * Returns a vector of pairs which map each node to its modified contents if the lambda
 * succeeded for every given node, or an empty optional otherwise.
 */
template <typename N, typename L>
std::optional<std::vector<std::pair<Node*, NodeContents>>> applyToNodeContents(
  const std::vector<N*>& nodes, L lambda)
{
  using NodeContent = std::variant<Layer, Group, Entity, Brush, BezierPatch>;
  auto getNodeContents = [](const Node* node) -> NodeContent {
    return node->accept(kdl::overload(
      [](const WorldNode* worldNode) -> NodeContent { return worldNode->entity(); },
      [](const LayerNode* layerNode) -> NodeContent { return layerNode->layer(); },
      [](const GroupNode* groupNode) -> NodeContent { return groupNode->group(); },
      [](const EntityNode* entityNode) -> NodeContent { return entityNode->entity(); },
      [](const BrushNode* brushNode) -> NodeContent { return brushNode->brush(); },
      [](const PatchNode* patchNode) -> NodeContent { return patchNode->patch(); }));
  };

  auto newNodes = std::vector<std::pair<Node*, NodeContents>>{};
  newNodes.reserve(nodes.size());

  for (auto* node : nodes)
  {
    auto nodeContents = getNodeContents(node);
    if (!std::visit(lambda, nodeContents))
    {
      return std::nullopt;
    }

    newNodes.emplace_back(node, std::move(nodeContents));
  }

  return newNodes;
}

/**
 * Applies the given lambda to a copy of the contents of each of the given nodes and
 * swaps the node contents if the given lambda succeeds for all node contents.
 *
 * The lambda L needs three overloads:
 * - bool operator()(Entity&);
 * - bool operator()(Brush&);
 * - bool operator()(BezierPatch&);
 *
 * The given node contents should be modified in place and the lambda should return true
 * if it was applied successfully and false otherwise.
 *
 * For each linked group in the given list of linked groups, its changes are distributed
 * to the connected members of its link set.
 *
 * Returns true if the given lambda could be applied successfully to all node contents
 * and false otherwise. If the lambda fails, then no node contents will be swapped, and
 * the original nodes remain unmodified.
 */
template <typename N, typename L>
bool applyAndSwap(
  Map& map,
  const std::string& commandName,
  const std::vector<N*>& nodes,
  std::vector<GroupNode*> changedLinkedGroups,
  L lambda)
{
  if (nodes.empty())
  {
    return true;
  }

  if (auto newNodes = applyToNodeContents(nodes, std::move(lambda)))
  {
    return map.updateNodeContents(
      commandName, std::move(*newNodes), std::move(changedLinkedGroups));
  }

  return false;
}

/**
 * Applies the given lambda to a copy of each of the given faces.
 *
 * Specifically, each brush node of the given faces has its contents copied and the
 * lambda applied to the copied faces. If the lambda succeeds for each face, the node
 * contents are subsequently swapped.
 *
 * The lambda L needs to accept brush faces:
 * - bool operator()(BrushFace&);
 *
 * The given node contents should be modified in place and the lambda should return true
 * if it was applied successfully and false otherwise.
 *
 * For each linked group in the given list of linked groups, its changes are distributed
 * to the connected members of its link set.
 *
 * Returns true if the given lambda could be applied successfully to each face and false
 * otherwise. If the lambda fails, then no node contents will be swapped, and the
 * original nodes remain unmodified.
 */
template <typename L>
bool applyAndSwap(
  Map& map,
  const std::string& commandName,
  const std::vector<BrushFaceHandle>& faces,
  L lambda)
{
  if (faces.empty())
  {
    return true;
  }

  auto brushes = std::unordered_map<BrushNode*, Brush>{};
  for (const auto& faceHandle : faces)
  {
    auto* brushNode = faceHandle.node();
    auto it = brushes.find(brushNode);
    if (it == std::end(brushes))
    {
      it = brushes.emplace(brushNode, brushNode->brush()).first;
    }

    auto& brush = it->second;
    if (!lambda(brush.face(faceHandle.faceIndex())))
    {
      return false;
    }
  }

  auto newNodes = std::vector<std::pair<Node*, NodeContents>>{};
  newNodes.reserve(brushes.size());

  for (auto& [brushNode, brush] : brushes)
  {
    newNodes.emplace_back(brushNode, NodeContents(std::move(brush)));
  }

  auto changedLinkedGroups = collectContainingGroups(
    kdl::vec_transform(newNodes, [](const auto& p) { return p.first; }));
  map.updateNodeContents(
    commandName, std::move(newNodes), std::move(changedLinkedGroups));

  return true;
}

} // namespace tb::mdl
