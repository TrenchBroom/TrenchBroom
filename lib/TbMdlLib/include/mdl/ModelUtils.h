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

#include "mdl/Hit.h"
#include "mdl/HitType.h"
#include "mdl/NodeTree.h"

#include "vm/bbox.h"

#include <map>
#include <vector>

namespace tb::mdl
{

class BrushFaceHandle;
class Node;
class GroupNode;
class BrushNode;
class EntityNode;
class LayerNode;
class EditorContext;

template <typename T, typename U>
class octree;

HitType::Type nodeHitType();

LayerNode* findContainingLayer(Node* node);

std::vector<LayerNode*> collectContainingLayersUserSorted(
  const std::vector<Node*>& nodes);

GroupNode* findContainingGroup(Node* node);
const GroupNode* findContainingGroup(const Node* node);

/**
 * Searches the ancestor chain of `node` for the outermost closed group and returns
 * it if one is found, otherwise returns nullptr.
 */
GroupNode* findOutermostClosedGroup(Node* node);
const GroupNode* findOutermostClosedGroup(const Node* node);

/**
 * Implements the Group picking logic: if `node` is inside a (possibly nested chain of)
 * closed group(s), the outermost closed group is returned. Otherwise, `node` itself is
 * returned.
 *
 * This is used to implement the UI where clicking on a brush inside a group selects the
 * group.
 */
Node* findOutermostClosedGroupOrNode(Node* node);

/**
 * Applies the group picking logic of findOutermostClosedGroupOrNode() to a list of hits.
 * The order of the hits is preserved, but if multiple hits map to the same group, that
 * group will only be listed once in the output.
 */
std::vector<mdl::Node*> hitsToNodesWithGroupPicking(const std::vector<Hit>& hits);

const Node* findOutermostClosedGroupOrNode(const Node* node);

std::vector<GroupNode*> collectGroups(const std::vector<Node*>& nodes);

std::vector<GroupNode*> collectContainingGroups(const std::vector<Node*>& nodes);

std::map<Node*, std::vector<Node*>> parentChildrenMap(const std::vector<Node*>& nodes);

std::vector<Node*> collectTouchingNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes);
std::vector<Node*> collectContainedNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes);

std::vector<Node*> collectSelectedNodes(const std::vector<Node*>& nodes);

std::vector<Node*> collectSelectableNodes(
  const std::vector<Node*>& nodes, const EditorContext& editorContext);

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

vm::bbox3d computeLogicalBounds(
  const std::vector<Node*>& nodes, const vm::bbox3d& defaultBounds = vm::bbox3d());
vm::bbox3d computePhysicalBounds(
  const std::vector<Node*>& nodes, const vm::bbox3d& defaultBounds = vm::bbox3d());

std::vector<BrushNode*> filterBrushNodes(const std::vector<Node*>& nodes);
std::vector<EntityNode*> filterEntityNodes(const std::vector<Node*>& nodes);

} // namespace tb::mdl
