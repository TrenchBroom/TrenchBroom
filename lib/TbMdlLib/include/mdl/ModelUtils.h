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

#pragma once

#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Hit.h"
#include "mdl/HitType.h"
#include "mdl/LayerNode.h"
#include "mdl/Node.h"
#include "mdl/NodeTree.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/flat_map.h"
#include "kd/overload.h"

#include "vm/bbox.h"

#include <ranges>
#include <vector>

namespace tb::mdl
{

class BrushFaceHandle;
class EntityNodeBase;

template <typename T, typename U>
class octree;

HitType::Type nodeHitType();

LayerNode* findContainingLayer(Node& node);
const LayerNode* findContainingLayer(const Node& node);

std::vector<LayerNode*> collectContainingLayersUserSorted(
  const std::vector<Node*>& nodes);

GroupNode* findContainingGroup(Node& node);
const GroupNode* findContainingGroup(const Node& node);

/**
 * Returns the entity that owns the given node, i.e. the closest ancestor entity or
 * world node, treating layers and groups as pass-through. Only brush and patch nodes
 * can be owned by an entity; returns nullptr for every other node type.
 */
EntityNodeBase* findContainingEntity(Node& node);
const EntityNodeBase* findContainingEntity(const Node& node);

/**
 * Searches the ancestor chain of `node` for the outermost closed group and returns
 * it if one is found, otherwise returns nullptr.
 */
GroupNode* findOutermostClosedGroup(Node& node);
const GroupNode* findOutermostClosedGroup(const Node& node);

/**
 * Implements the Group picking logic: if `node` is inside a (possibly nested chain of)
 * closed group(s), the outermost closed group is returned. Otherwise, `node` itself is
 * returned.
 *
 * This is used to implement the UI where clicking on a brush inside a group selects the
 * group.
 */
Node* findOutermostClosedGroupOrNode(Node& node);

/**
 * Applies the group picking logic of findOutermostClosedGroupOrNode() to a list of hits.
 * The order of the hits is preserved, but if multiple hits map to the same group, that
 * group will only be listed once in the output.
 *
 * All hits must be node hits, i.e. their type must match nodeHitType().
 */
std::vector<mdl::Node*> hitsToNodesWithGroupPicking(const std::vector<Hit>& hits);

const Node* findOutermostClosedGroupOrNode(const Node& node);

std::vector<GroupNode*> collectGroups(const std::vector<Node*>& nodes);

std::vector<GroupNode*> collectContainingGroups(const std::vector<Node*>& nodes);

kdl::flat_map<Node*, std::vector<Node*>> parentChildrenMap(
  const std::vector<Node*>& nodes);

std::vector<Node*> collectTouchingNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes);
std::vector<Node*> collectContainedNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes);

std::vector<Node*> collectSelectedNodes(const std::vector<Node*>& nodes);

template <std::ranges::range R = std::vector<Node*>>
std::vector<Node*> collectSelectableNodes(R&& nodes, const EditorContext& editorContext)
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

std::vector<BrushFaceHandle> collectSelectedBrushFaces(const std::vector<Node*>& nodes);
std::vector<BrushFaceHandle> collectSelectableBrushFaces(
  const std::vector<Node*>& nodes, const EditorContext& editorContext);

/**
 * Floods out from the given face and returns every face that forms one connected,
 * coplanar surface with it, including across touching brushes. Faces only join where they
 * share an edge, so corners and gaps don't bridge the region.
 *
 * Candidate brushes are gathered by querying `nodeTree` with the bounds of each face the
 * flood reaches, so only brushes near the surface are visited rather than the whole map.
 * `editorContext` filters out faces that aren't selectable (e.g. on hidden or locked
 * brushes), which also keeps those brushes from bridging the region.
 */
std::vector<BrushFaceHandle> collectConnectedCoplanarFaces(
  const BrushFaceHandle& startFace,
  const EditorContext& editorContext,
  const NodeTree& nodeTree);

template <std::ranges::range R = std::vector<Node*>>
vm::bbox3d computeLogicalBounds(R&& nodes, const vm::bbox3d& defaultBounds = vm::bbox3d())
{
  auto bounds = std::vector<vm::bbox3d>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [&](const GroupNode& groupNode) { bounds.push_back(groupNode.logicalBounds()); },
      [&](const EntityNode& entityNode) { bounds.push_back(entityNode.logicalBounds()); },
      [&](const BrushNode& brushNode) { bounds.push_back(brushNode.logicalBounds()); },
      [&](const PatchNode& patchNode) { bounds.push_back(patchNode.logicalBounds()); }));
  return vm::bbox3d::build(bounds).value_or(defaultBounds);
}

template <std::ranges::range R = std::vector<Node*>>
vm::bbox3d computePhysicalBounds(
  R&& nodes, const vm::bbox3d& defaultBounds = vm::bbox3d())
{
  auto bounds = std::vector<vm::bbox3d>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode&) {},
      [](const LayerNode&) {},
      [&](const GroupNode& groupNode) { bounds.push_back(groupNode.physicalBounds()); },
      [&](const EntityNode& entityNode) {
        bounds.push_back(entityNode.physicalBounds());
      },
      [&](const BrushNode& brushNode) { bounds.push_back(brushNode.physicalBounds()); },
      [&](const PatchNode& patchNode) { bounds.push_back(patchNode.physicalBounds()); }));
  return vm::bbox3d::build(bounds).value_or(defaultBounds);
}

template <std::ranges::sized_range R = std::vector<Node*>>
std::vector<BrushNode*> filterBrushNodes(R&& nodes)
{
  auto result = std::vector<BrushNode*>{};
  result.reserve(std::ranges::size(nodes));
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

std::vector<EntityNode*> filterEntityNodes(const std::vector<Node*>& nodes);

} // namespace tb::mdl
