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

#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/Node.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/vector_utils.h"

#include <vector>

namespace TrenchBroom::Model
{

struct TrueNodePredicate
{
  bool operator()(WorldNode*) const { return true; }
  bool operator()(LayerNode*) const { return true; }
  bool operator()(GroupNode*) const { return true; }
  bool operator()(EntityNode*) const { return true; }
  bool operator()(BrushNode*) const { return true; }
  bool operator()(PatchNode*) const { return true; }
};

template <typename T = Node*, typename Predicate = TrueNodePredicate>
auto collectNodes(const std::vector<T>& nodes, const Predicate& predicate = Predicate{})
{
  auto result = std::vector<Node*>{};
  auto visitor = kdl::overload(
    [&]([[maybe_unused]] WorldNode* worldNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, WorldNode*>)
      {
        if (predicate(worldNode))
        {
          result.push_back(worldNode);
        }
      }
    },
    [&]([[maybe_unused]] LayerNode* layerNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, LayerNode*>)
      {
        if (predicate(layerNode))
        {
          result.push_back(layerNode);
        }
      }
    },
    [&]([[maybe_unused]] GroupNode* groupNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, GroupNode*>)
      {
        if (predicate(groupNode))
        {
          result.push_back(groupNode);
        }
      }
    },
    [&]([[maybe_unused]] EntityNode* entityNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, EntityNode*>)
      {
        if (predicate(entityNode))
        {
          result.push_back(entityNode);
        }
      }
    },
    [&]([[maybe_unused]] BrushNode* brushNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, BrushNode*>)
      {
        if (predicate(brushNode))
        {
          result.push_back(brushNode);
        }
      }
    },
    [&]([[maybe_unused]] PatchNode* patchNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, PatchNode*>)
      {
        if (predicate(patchNode))
        {
          result.push_back(patchNode);
        }
      }
    });

  for (auto* node : nodes)
  {
    node->accept(visitor);
  }
  return result;
}

template <typename T = Node*, typename Predicate = TrueNodePredicate>
auto collectAncestors(
  const std::vector<T>& nodes, const Predicate& predicate = Predicate{})
{
  auto result = std::vector<Node*>{};
  auto visitor = kdl::overload(
    [&]([[maybe_unused]] WorldNode* worldNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, WorldNode*>)
      {
        if (predicate(worldNode))
        {
          result.push_back(worldNode);
        }
      }
    },
    [&](auto&& thisLambda, [[maybe_unused]] LayerNode* layerNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, LayerNode*>)
      {
        if (predicate(layerNode))
        {
          result.push_back(layerNode);
        }
      }
      layerNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] GroupNode* groupNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, GroupNode*>)
      {
        if (predicate(groupNode))
        {
          result.push_back(groupNode);
        }
      }
      groupNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] EntityNode* entityNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, EntityNode*>)
      {
        if (predicate(entityNode))
        {
          result.push_back(entityNode);
        }
      }
      entityNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] BrushNode* brushNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, BrushNode*>)
      {
        if (predicate(brushNode))
        {
          result.push_back(brushNode);
        }
      }
      brushNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] PatchNode* patchNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, PatchNode*>)
      {
        if (predicate(patchNode))
        {
          result.push_back(patchNode);
        }
      }
      patchNode->visitParent(thisLambda);
    });

  for (auto* node : nodes)
  {
    node->visitParent(visitor);
  }
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

template <typename T = Node*, typename Predicate = TrueNodePredicate>
auto collectNodesAndAncestors(
  const std::vector<T>& nodes, const Predicate& predicate = Predicate{})
{
  auto result = std::vector<Node*>{};
  auto visitor = kdl::overload(
    [&]([[maybe_unused]] WorldNode* worldNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, WorldNode*>)
      {
        if (predicate(worldNode))
        {
          result.push_back(worldNode);
        }
      }
    },
    [&](auto&& thisLambda, [[maybe_unused]] LayerNode* layerNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, LayerNode*>)
      {
        if (predicate(layerNode))
        {
          result.push_back(layerNode);
        }
      }
      layerNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] GroupNode* groupNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, GroupNode*>)
      {
        if (predicate(groupNode))
        {
          result.push_back(groupNode);
        }
      }
      groupNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] EntityNode* entityNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, EntityNode*>)
      {
        if (predicate(entityNode))
        {
          result.push_back(entityNode);
        }
      }
      entityNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] BrushNode* brushNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, BrushNode*>)
      {
        if (predicate(brushNode))
        {
          result.push_back(brushNode);
        }
      }
      brushNode->visitParent(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] PatchNode* patchNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, PatchNode*>)
      {
        if (predicate(patchNode))
        {
          result.push_back(patchNode);
        }
      }
      patchNode->visitParent(thisLambda);
    });

  Node::visitAll(nodes, visitor);
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

template <typename T = Node*, typename Predicate = TrueNodePredicate>
auto collectDescendants(
  const std::vector<T>& nodes, const Predicate& predicate = Predicate{})
{
  auto result = std::vector<Node*>{};
  auto visitor = kdl::overload(
    [&](auto&& thisLambda, [[maybe_unused]] WorldNode* worldNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, WorldNode*>)
      {
        if (predicate(worldNode))
        {
          result.push_back(worldNode);
        }
      }
      worldNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] LayerNode* layerNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, LayerNode*>)
      {
        if (predicate(layerNode))
        {
          result.push_back(layerNode);
        }
      }
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] GroupNode* groupNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, GroupNode*>)
      {
        if (predicate(groupNode))
        {
          result.push_back(groupNode);
        }
      }
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] EntityNode* entityNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, EntityNode*>)
      {
        if (predicate(entityNode))
        {
          result.push_back(entityNode);
        }
      }
      entityNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] BrushNode* brushNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, BrushNode*>)
      {
        if (predicate(brushNode))
        {
          result.push_back(brushNode);
        }
      }
      brushNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] PatchNode* patchNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, PatchNode*>)
      {
        if (predicate(patchNode))
        {
          result.push_back(patchNode);
        }
      }
      patchNode->visitChildren(thisLambda);
    });

  for (auto* node : nodes)
  {
    node->visitChildren(visitor);
  }
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

template <typename T = Node*, typename Predicate = TrueNodePredicate>
auto collectNodesAndDescendants(
  const std::vector<T>& nodes, const Predicate& predicate = Predicate{})
{
  auto result = std::vector<Node*>{};
  auto visitor = kdl::overload(
    [&](auto&& thisLambda, [[maybe_unused]] WorldNode* worldNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, WorldNode*>)
      {
        if (predicate(worldNode))
        {
          result.push_back(worldNode);
        }
      }
      worldNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] LayerNode* layerNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, LayerNode*>)
      {
        if (predicate(layerNode))
        {
          result.push_back(layerNode);
        }
      }
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] GroupNode* groupNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, GroupNode*>)
      {
        if (predicate(groupNode))
        {
          result.push_back(groupNode);
        }
      }
      groupNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] EntityNode* entityNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, EntityNode*>)
      {
        if (predicate(entityNode))
        {
          result.push_back(entityNode);
        }
      }
      entityNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] BrushNode* brushNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, BrushNode*>)
      {
        if (predicate(brushNode))
        {
          result.push_back(brushNode);
        }
      }
      brushNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, [[maybe_unused]] PatchNode* patchNode) {
      if constexpr (std::is_invocable_r_v<bool, Predicate, PatchNode*>)
      {
        if (predicate(patchNode))
        {
          result.push_back(patchNode);
        }
      }
      patchNode->visitChildren(thisLambda);
    });

  Node::visitAll(nodes, visitor);
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

struct TrueBrushFacePredicate
{
  bool operator()(const BrushNode&, const BrushFace&) const { return true; }
};

template <typename T = Node, typename Predicate = TrueBrushFacePredicate>
std::vector<BrushFaceHandle> collectBrushFaces(
  const std::vector<T*>& nodes, const Predicate& predicate = Predicate{})
{
  auto result = std::vector<BrushFaceHandle>{};
  Node::visitAll(
    nodes,
    kdl::overload(
      [&](auto&& thisLambda, const WorldNode* worldNode) {
        worldNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, const LayerNode* layerNode) {
        layerNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, const GroupNode* groupNode) {
        groupNode->visitChildren(thisLambda);
      },
      [&](auto&& thisLambda, const EntityNode* entityNode) {
        entityNode->visitChildren(thisLambda);
      },
      [&](BrushNode* brushNode) {
        const auto& brush = brushNode->brush();
        for (size_t i = 0; i < brush.faceCount(); ++i)
        {
          const auto& face = brush.face(i);
          if (predicate(*brushNode, face))
          {
            result.emplace_back(brushNode, i);
          }
        }
      },
      [&](const PatchNode*) {}));
  return kdl::vec_sort_and_remove_duplicates(std::move(result));
}

} // namespace TrenchBroom::Model
