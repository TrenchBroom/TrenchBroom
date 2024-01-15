/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "ModelUtils.h"

#include "Ensure.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/EditorContext.h"
#include "Model/NodeQueries.h"
#include "Polyhedron.h"

#include <kdl/vector_utils.h>

#include <vector>

namespace TrenchBroom::Model
{

HitType::Type nodeHitType()
{
  return EntityNode::EntityHitType | BrushNode::BrushHitType | PatchNode::PatchHitType;
}

LayerNode* findContainingLayer(Node* node)
{
  return node->accept(kdl::overload(
    [](WorldNode*) -> LayerNode* { return nullptr; },
    [](LayerNode* layer) -> LayerNode* { return layer; },
    [](auto&& thisLambda, GroupNode* group) -> LayerNode* {
      return group->visitParent(thisLambda).value_or(nullptr);
    },
    [](auto&& thisLambda, EntityNode* entity) -> LayerNode* {
      return entity->visitParent(thisLambda).value_or(nullptr);
    },
    [](auto&& thisLambda, BrushNode* brush) -> LayerNode* {
      return brush->visitParent(thisLambda).value_or(nullptr);
    },
    [](auto&& thisLambda, PatchNode* patch) -> LayerNode* {
      return patch->visitParent(thisLambda).value_or(nullptr);
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
      [](WorldNode*) -> GroupNode* { return nullptr; },
      [](LayerNode*) -> GroupNode* { return nullptr; },
      [](GroupNode* group) -> GroupNode* { return group; },
      [](auto&& thisLambda, EntityNode* entity) -> GroupNode* {
        return entity->visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, BrushNode* brush) -> GroupNode* {
        return brush->visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, PatchNode* patch) -> GroupNode* {
        return patch->visitParent(thisLambda).value_or(nullptr);
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
      [](WorldNode*) -> GroupNode* { return nullptr; },
      [](LayerNode*) -> GroupNode* { return nullptr; },
      [](auto&& thisLambda, GroupNode* group) -> GroupNode* {
        if (GroupNode* parentResult = group->visitParent(thisLambda).value_or(nullptr))
        {
          return parentResult;
        }
        // we didn't find a result searching the parent chain, so either return
        // this group (if it's closed) or nullptr to indicate no result
        return group->closed() ? group : nullptr;
      },
      [](auto&& thisLambda, EntityNode* entity) -> GroupNode* {
        return entity->visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, BrushNode* brush) -> GroupNode* {
        return brush->visitParent(thisLambda).value_or(nullptr);
      },
      [](auto&& thisLambda, PatchNode* patch) -> GroupNode* {
        return patch->visitParent(thisLambda).value_or(nullptr);
      }))
    .value_or(nullptr);
}

const GroupNode* findOutermostClosedGroup(const Node* node)
{
  return findOutermostClosedGroup(const_cast<Node*>(node));
}

std::vector<GroupNode*> collectGroups(const std::vector<Node*>& nodes)
{
  auto result = std::vector<GroupNode*>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [](auto&& thisLambda, const WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [](auto&& thisLambda, const LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, GroupNode* groupNode) {
        result.push_back(groupNode);
        groupNode->visitChildren(thisLambda);
      },
      [](const EntityNode*) {},
      [](const BrushNode*) {},
      [](const PatchNode*) {}));
  return result;
}

std::map<Node*, std::vector<Node*>> parentChildrenMap(const std::vector<Node*>& nodes)
{
  std::map<Node*, std::vector<Node*>> result;

  for (Node* node : nodes)
  {
    Node* parent = node->parent();
    ensure(parent != nullptr, "parent is null");
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

  const auto collectIfMatching = [&](auto* node) {
    for (const auto* brush : brushes)
    {
      if (predicate(node, brush))
      {
        result.push_back(node);
        return;
      }
    }
  };

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
      [](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
      [&](auto&& thisLambda, GroupNode* group) {
        if (group->opened() || group->hasOpenedDescendant())
        {
          group->visitChildren(thisLambda);
        }
        else
        {
          collectIfMatching(group);
        }
      },
      [&](auto&& thisLambda, EntityNode* entity) {
        if (entity->hasChildren())
        {
          entity->visitChildren(thisLambda);
        }
        else
        {
          collectIfMatching(entity);
        }
      },
      [&](BrushNode* brush) {
        // if `brush` is one of the search query nodes, don't count it as touching
        if (!kdl::vec_contains(brushes, brush))
        {
          collectIfMatching(brush);
        }
      },
      [&](PatchNode* patch) {
        // if `patch` is one of the search query nodes, don't count it as touching
        collectIfMatching(patch);
      }));
  }

  return result;
}

std::vector<Node*> collectTouchingNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes)
{
  return collectMatchingNodes(nodes, brushes, [](const auto* node, const auto* brush) {
    return brush->intersects(node);
  });
}

std::vector<Node*> collectContainedNodes(
  const std::vector<Node*>& nodes, const std::vector<BrushNode*>& brushes)
{
  return collectMatchingNodes(nodes, brushes, [](const auto* node, const auto* brush) {
    return brush->contains(node);
  });
}

std::vector<Node*> collectSelectedNodes(const std::vector<Node*>& nodes)
{
  return collectNodesAndDescendants(
    nodes, [](const auto* node) { return node->selected(); });
}

std::vector<Node*> collectSelectableNodes(
  const std::vector<Node*>& nodes, const EditorContext& editorContext)
{
  auto result = std::vector<Node*>{};

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* world) { world->visitChildren(thisLambda); },
      [&](auto&& thisLambda, LayerNode* layer) { layer->visitChildren(thisLambda); },
      [&](auto&& thisLambda, GroupNode* group) {
        if (editorContext.selectable(group))
        {
          // implies that any containing group is opened and that group itself is closed
          // therefore we don't need to visit the group's children
          result.push_back(group);
        }
        else
        {
          group->visitChildren(thisLambda);
        }
      },
      [&](auto&& thisLambda, EntityNode* entity) {
        if (editorContext.selectable(entity))
        {
          result.push_back(entity);
        }
        entity->visitChildren(thisLambda);
      },
      [&](BrushNode* brush) {
        if (editorContext.selectable(brush))
        {
          result.push_back(brush);
        }
      },
      [&](PatchNode* patch) {
        if (editorContext.selectable(patch))
        {
          result.push_back(patch);
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
      return editorContext.selectable(&brushNode, brushFace);
    });
}

vm::bbox3 computeLogicalBounds(
  const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds)
{
  vm::bbox3::builder builder;
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode*) {},
      [](const LayerNode*) {},
      [&](const GroupNode* group) { builder.add(group->logicalBounds()); },
      [&](const EntityNode* entity) { builder.add(entity->logicalBounds()); },
      [&](const BrushNode* brush) { builder.add(brush->logicalBounds()); },
      [&](const PatchNode* patch) { builder.add(patch->logicalBounds()); }));
  return builder.initialized() ? builder.bounds() : defaultBounds;
}

vm::bbox3 computePhysicalBounds(
  const std::vector<Node*>& nodes, const vm::bbox3& defaultBounds)
{
  vm::bbox3::builder builder;
  Node::visitAll(
    nodes,
    kdl::overload(
      [](const WorldNode*) {},
      [](const LayerNode*) {},
      [&](const GroupNode* group) { builder.add(group->physicalBounds()); },
      [&](const EntityNode* entity) { builder.add(entity->physicalBounds()); },
      [&](const BrushNode* brush) { builder.add(brush->physicalBounds()); },
      [&](const PatchNode* patch) { builder.add(patch->physicalBounds()); }));
  return builder.initialized() ? builder.bounds() : defaultBounds;
}

std::vector<BrushNode*> filterBrushNodes(const std::vector<Node*>& nodes)
{
  auto result = std::vector<BrushNode*>{};
  result.reserve(nodes.size());
  for (Node* node : nodes)
  {
    node->accept(kdl::overload(
      [](WorldNode*) {},
      [](LayerNode*) {},
      [](GroupNode*) {},
      [](EntityNode*) {},
      [&](BrushNode* brushNode) { result.push_back(brushNode); },
      [](PatchNode*) {}));
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
      [](WorldNode*) {},
      [](LayerNode*) {},
      [](GroupNode*) {},
      [&](EntityNode* entityNode) { result.push_back(entityNode); },
      [](BrushNode*) {},
      [](PatchNode*) {}));
  }
  return result;
}

} // namespace TrenchBroom::Model
