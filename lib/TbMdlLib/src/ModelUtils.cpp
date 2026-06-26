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

#include "mdl/ModelUtils.h"

#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/EditorContext.h"
#include "mdl/HitAdapter.h"
#include "mdl/NodeQueries.h"

#include "kd/contracts.h"
#include "kd/ranges/cartesian_product_view.h"
#include "kd/ranges/to.h"
#include "kd/stable_remove_duplicates.h"
#include "kd/vector_utils.h"

#include "vm/bbox.h"
#include "vm/intersection.h"
#include "vm/segment.h"

#include <algorithm>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

namespace tb::mdl
{

HitType::Type nodeHitType()
{
  return EntityNode::EntityHitType | BrushNode::BrushHitType | PatchNode::PatchHitType;
}

LayerNode* findContainingLayer(Node* node)
{
  return node->accept(kdl::overload(
    [](WorldNode&) -> LayerNode* { return nullptr; },
    [](LayerNode& layerNode) -> LayerNode* { return &layerNode; },
    [](auto&& thisLambda, GroupNode& groupNode) -> LayerNode* {
      return groupNode.visitParent(thisLambda).value_or(nullptr);
    },
    [](auto&& thisLambda, EntityNode& entityNode) -> LayerNode* {
      return entityNode.visitParent(thisLambda).value_or(nullptr);
    },
    [](auto&& thisLambda, BrushNode& brushNode) -> LayerNode* {
      return brushNode.visitParent(thisLambda).value_or(nullptr);
    },
    [](auto&& thisLambda, PatchNode& patchNode) -> LayerNode* {
      return patchNode.visitParent(thisLambda).value_or(nullptr);
    }));
}

std::vector<LayerNode*> collectContainingLayersUserSorted(const std::vector<Node*>& nodes)
{
  std::vector<LayerNode*> layers;
  for (auto* node : nodes)
  {
    if (auto* layer = findContainingLayer(node))
    {
      layers.push_back(layer);
    }
  }
  return kdl::vec_sort_and_remove_duplicates(std::move(layers));
}

GroupNode* findContainingGroup(Node* node)
{
  return node
    ->visitParent(kdl::overload(
      [](WorldNode&) -> GroupNode* { return nullptr; },
      [](LayerNode&) -> GroupNode* { return nullptr; },
      [](GroupNode& groupNode) -> GroupNode* { return &groupNode; },
      [](auto&& thisLambda, EntityNode& entityNode) -> GroupNode* {
        return entityNode.visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, BrushNode& brushNode) -> GroupNode* {
        return brushNode.visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, PatchNode& patchNode) -> GroupNode* {
        return patchNode.visitParent(thisLambda).value_or(nullptr);
      }))
    .value_or(nullptr);
}

const GroupNode* findContainingGroup(const Node* node)
{
  return findContainingGroup(const_cast<Node*>(node));
}

GroupNode* findOutermostClosedGroup(Node* node)
{
  return node
    ->visitParent(kdl::overload(
      [](WorldNode&) -> GroupNode* { return nullptr; },
      [](LayerNode&) -> GroupNode* { return nullptr; },
      [](auto&& thisLambda, GroupNode& groupNode) -> GroupNode* {
        if (auto* parentResult = groupNode.visitParent(thisLambda).value_or(nullptr))
        {
          return parentResult;
        }
        // we didn't find a result searching the parent chain, so either return
        // this group (if it's closed) or nullptr to indicate no result
        return groupNode.closed() ? &groupNode : nullptr;
      },
      [](auto&& thisLambda, EntityNode& entityNode) -> GroupNode* {
        return entityNode.visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, BrushNode& brushNode) -> GroupNode* {
        return brushNode.visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, PatchNode& patchNode) -> GroupNode* {
        return patchNode.visitParent(thisLambda).value_or(nullptr);
      }))
    .value_or(nullptr);
}

const GroupNode* findOutermostClosedGroup(const Node* node)
{
  return findOutermostClosedGroup(const_cast<Node*>(node));
}

Node* findOutermostClosedGroupOrNode(Node* node)
{
  if (auto* group = findOutermostClosedGroup(node))
  {
    return group;
  }

  return node;
}

std::vector<Node*> hitsToNodesWithGroupPicking(const std::vector<Hit>& hits)
{
  return kdl::col_stable_remove_duplicates(
    hits | std::views::transform([](const auto& hit) {
      return findOutermostClosedGroupOrNode(hitToNode(hit));
    })
    | kdl::ranges::to<std::vector>());
}

const Node* findOutermostClosedGroupOrNode(const Node* node)
{
  return findOutermostClosedGroupOrNode(const_cast<Node*>(node));
}

std::vector<GroupNode*> collectGroups(const std::vector<Node*>& nodes)
{
  auto result = std::vector<GroupNode*>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode& worldNode) {
        worldNode.visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode& layerNode) {
        layerNode.visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, GroupNode& groupNode) {
        result.push_back(&groupNode);
        groupNode.visitChildren(thisLambda);
      },
      [](const EntityNode&) {},
      [](const BrushNode&) {},
      [](const PatchNode&) {}));
  return result;
}


std::vector<GroupNode*> collectContainingGroups(const std::vector<Node*>& nodes)
{
  auto result = std::vector<GroupNode*>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [&](GroupNode& groupNode) {
        if (auto* containingGroupNode = groupNode.containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      },
      [&](EntityNode& entityNode) {
        if (auto* containingGroupNode = entityNode.containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      },
      [&](BrushNode& brushNode) {
        if (auto* containingGroupNode = brushNode.containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      },
      [&](PatchNode& patchNode) {
        if (auto* containingGroupNode = patchNode.containingGroup())
        {
          result.push_back(containingGroupNode);
        }
      }));

  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

std::map<Node*, std::vector<Node*>> parentChildrenMap(const std::vector<Node*>& nodes)
{
  auto result = std::map<Node*, std::vector<Node*>>{};

  for (auto* node : nodes)
  {
    auto* parent = node->parent();
    contract_assert(parent != nullptr);

    result[parent].push_back(node);
  }

  return result;
}

/**
 * Recursively collect brushes and entities from the given vector of node trees such that
 * the returned nodes match the given predicate. A matching brush is only returned if it
 * isn't in the given vector brushes. A node matches the given predicate if there is a
 * brush in the given vector of brushes such that the predicate evaluates to true for that
 * pair of node and brush.
 *
 * The given predicate must be a function that maps a node and a brush to true or false.
 */
template <typename P>
static std::vector<Node*> collectMatchingNodes(
  const std::vector<Node*>& nodes,
  const std::vector<BrushNode*>& brushes,
  const P& predicate)
{
  auto result = std::vector<Node*>{};

  const auto collectIfMatching = [&](auto& node) {
    for (const auto* brush : brushes)
    {
      if (predicate(node, brush))
      {
        result.push_back(&node);
        return;
      }
    }
  };

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](
        auto&& thisLambda, WorldNode& worldNode) { worldNode.visitChildren(thisLambda); },
      [](
        auto&& thisLambda, LayerNode& layerNode) { layerNode.visitChildren(thisLambda); },
      [&](auto&& thisLambda, GroupNode& groupNode) {
        if (groupNode.opened() || groupNode.hasOpenedDescendant())
        {
          groupNode.visitChildren(thisLambda);
        }
        else
        {
          collectIfMatching(groupNode);
        }
      },
      [&](auto&& thisLambda, EntityNode& entityNode) {
        if (entityNode.hasChildren())
        {
          entityNode.visitChildren(thisLambda);
        }
        else
        {
          collectIfMatching(entityNode);
        }
      },
      [&](BrushNode& brushNode) {
        // if `brush` is one of the search query nodes, don't count it as touching
        if (!kdl::vec_contains(brushes, &brushNode))
        {
          collectIfMatching(brushNode);
        }
      },
      [&](PatchNode& patchNode) {
        // if `patch` is one of the search query nodes, don't count it as touching
        collectIfMatching(patchNode);
      }));
  }

  return result;
}

std::vector<Node*> collectTouchingNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushNodes)
{
  return collectMatchingNodes(
    nodes, brushNodes, [](const auto& node, const auto& brushNode) {
      return brushNode->intersects(node);
    });
}

std::vector<Node*> collectContainedNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushNodes)
{
  return collectMatchingNodes(
    nodes, brushNodes, [](const auto& node, const auto& brushNode) {
      return brushNode->contains(node);
    });
}

std::vector<Node*> collectSelectedNodes(const std::vector<Node*>& nodes)
{
  return collectNodesAndDescendants(
    nodes, [](const auto& node) { return node.selected(); });
}

std::vector<Node*> collectSelectableNodes(
  const std::vector<Node*>& nodes, const EditorContext& editorContext)
{
  auto result = std::vector<Node*>{};

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](
        auto&& thisLambda, WorldNode& worldNode) { worldNode.visitChildren(thisLambda); },
      [&](
        auto&& thisLambda, LayerNode& layerNode) { layerNode.visitChildren(thisLambda); },
      [&](auto&& thisLambda, GroupNode& groupNode) {
        if (editorContext.selectable(groupNode))
        {
          // implies that any containing group is opened and that group itself is closed
          // therefore we don't need to visit the group's children
          result.push_back(&groupNode);
        }
        else
        {
          groupNode.visitChildren(thisLambda);
        }
      },
      [&](auto&& thisLambda, EntityNode& entityNode) {
        if (editorContext.selectable(entityNode))
        {
          result.push_back(&entityNode);
        }
        entityNode.visitChildren(thisLambda);
      },
      [&](BrushNode& brushNode) {
        if (editorContext.selectable(brushNode))
        {
          result.push_back(&brushNode);
        }
      },
      [&](PatchNode& patchNode) {
        if (editorContext.selectable(patchNode))
        {
          result.push_back(&patchNode);
        }
      }));
  }

  return result;
}

std::vector<BrushFaceHandle> collectSelectedBrushFaces(const std::vector<Node*>& nodes)
{
  return collectBrushFaces(nodes, [](const BrushNode&, const BrushFace& brushFace) {
    return brushFace.selected();
  });
}

std::vector<BrushFaceHandle> collectSelectableBrushFaces(
  const std::vector<Node*>& nodes, const EditorContext& editorContext)
{
  return collectBrushFaces(
    nodes, [&](const BrushNode& brushNode, const BrushFace& brushFace) {
      return editorContext.selectable(brushNode, brushFace);
    });
}

std::vector<BrushFaceHandle> collectConnectedCoplanarFaces(
  const BrushFaceHandle& startFace,
  const EditorContext& editorContext,
  const NodeTree& nodeTree)
{
  constexpr auto epsilon = vm::constants<double>::almost_zero();
  const auto& startPlane = startFace.face().boundary();

  struct Candidate
  {
    BrushFaceHandle handle;
    std::vector<vm::segment3d> edges;
    vm::bbox3d bounds;
  };

  const auto makeCandidate = [&](const BrushFaceHandle& handle) {
    const auto vertices = handle.face().vertexPositions();
    const auto edges =
      handle.face().edges()
      | std::views::transform([](const auto* edge) { return edge->segment(); });

    auto builder = vm::bbox3d::builder{};
    builder.add(std::begin(vertices), std::end(vertices));

    return Candidate{
      handle,
      edges | kdl::ranges::to<std::vector>(),
      builder.bounds().expand(epsilon),
    };
  };

  const auto facesShareEdgeSegment = [](const auto& lhs, const auto& rhs) {
    const auto allEdgePairs = kdl::views::cartesian_product(lhs, rhs);
    return std::ranges::any_of(allEdgePairs, [](const auto& edgePair) {
      const auto& [lhsEdge, rhsEdge] = edgePair;
      return vm::segments_overlap(lhsEdge, rhsEdge, vm::Cd::almost_zero());
    });
  };

  // Cache coplanar+selectable faces per brush node so a node hit from multiple flood
  // directions has its face list computed only once. Querying the node tree keeps the
  // flood local instead of scanning the whole map, and dropping unselectable faces stops
  // hidden or locked brushes from bridging the region.
  auto nodeCache = std::unordered_map<Node*, std::vector<BrushFaceHandle>>{};

  const auto coplanarFacesOf = [&](Node* node) -> const std::vector<BrushFaceHandle>& {
    auto [it, inserted] = nodeCache.emplace(node, std::vector<BrushFaceHandle>{});
    if (inserted)
    {
      it->second = collectSelectableBrushFaces(std::vector<Node*>{node}, editorContext)
                   | std::views::filter([&](const auto& handle) {
                       return handle.face().coplanarWith(startPlane);
                     })
                   | kdl::ranges::to<std::vector>();
    }
    return it->second;
  };

  const auto coplanarFacesNear = [&](const vm::bbox3d& bounds) {
    auto result = std::vector<BrushFaceHandle>{};
    for (auto* node : nodeTree.find_intersectors(bounds))
    {
      kdl::vec_append(result, coplanarFacesOf(node));
    }
    return result;
  };

  // Flood out from the start face, re-querying the tree around each face we reach so a
  // long row of brushes is followed without ever visiting the rest of the map.
  auto region = std::vector<BrushFaceHandle>{};
  auto visited = kdl::vector_set<BrushFaceHandle>{startFace};
  auto pending = std::vector<Candidate>{};
  pending.push_back(makeCandidate(startFace));

  while (!pending.empty())
  {
    const auto current = kdl::vec_pop_back(pending);
    region.push_back(current.handle);

    for (const auto& handle : coplanarFacesNear(current.bounds))
    {
      if (visited.count(handle) == 0)
      {
        auto candidate = makeCandidate(handle);
        if (
          current.bounds.intersects(candidate.bounds)
          && facesShareEdgeSegment(current.edges, candidate.edges))
        {
          visited.insert(handle);
          pending.push_back(std::move(candidate));
        }
      }
    }
  }

  return region;
}

vm::bbox3d computeLogicalBounds(
  const std::vector<Node*>& nodes, const vm::bbox3d& defaultBounds)
{
  vm::bbox3d::builder builder;
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [&](const GroupNode& groupNode) { builder.add(groupNode.logicalBounds()); },
      [&](const EntityNode& entityNode) { builder.add(entityNode.logicalBounds()); },
      [&](const BrushNode& brushNode) { builder.add(brushNode.logicalBounds()); },
      [&](const PatchNode& patchNode) { builder.add(patchNode.logicalBounds()); }));
  return builder.initialized() ? builder.bounds() : defaultBounds;
}

vm::bbox3d computePhysicalBounds(
  const std::vector<Node*>& nodes, const vm::bbox3d& defaultBounds)
{
  vm::bbox3d::builder builder;
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [&](const GroupNode& groupNode) { builder.add(groupNode.physicalBounds()); },
      [&](const EntityNode& entityNode) { builder.add(entityNode.physicalBounds()); },
      [&](const BrushNode& brushNode) { builder.add(brushNode.physicalBounds()); },
      [&](const PatchNode& patchNode) { builder.add(patchNode.physicalBounds()); }));
  return builder.initialized() ? builder.bounds() : defaultBounds;
}

std::vector<BrushNode*> filterBrushNodes(const std::vector<Node*>& nodes)
{
  auto result = std::vector<BrushNode*>{};
  result.reserve(nodes.size());
  for (Node* node : nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode&) {},
      [](LayerNode&) {},
      [](GroupNode&) {},
      [](EntityNode&) {},
      [&](BrushNode& brushNode) { result.push_back(&brushNode); },
      [](PatchNode&) {}));
  }
  return result;
}

std::vector<EntityNode*> filterEntityNodes(const std::vector<Node*>& nodes)
{
  auto result = std::vector<EntityNode*>{};
  result.reserve(nodes.size());
  for (Node* node : nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode&) {},
      [](LayerNode&) {},
      [](GroupNode&) {},
      [&](EntityNode& entityNode) { result.push_back(&entityNode); },
      [](BrushNode&) {},
      [](PatchNode&) {}));
  }
  return result;
}

} // namespace tb::mdl
