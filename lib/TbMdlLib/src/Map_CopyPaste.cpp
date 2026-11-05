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

#include "mdl/Map_CopyPaste.h"

#include "base/Logger.h"
#include "base/SimpleParserStatus.h"
#include "base/Uuid.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushFaceReader.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LinkedGroupUtils.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/NodeReader.h"
#include "mdl/NodeWriter.h"
#include "mdl/PasteType.h"
#include "mdl/PatchNode.h"
#include "mdl/Transaction.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/WorldNode.h"

#include "kd/contracts.h"
#include "kd/flat_map.h"
#include "kd/flat_set.h"
#include "kd/ranges/to.h"
#include "kd/vector_utils.h"

#include <algorithm>
#include <memory>
#include <ranges>

namespace tb::mdl
{
namespace
{

auto extractNodesToPaste(std::vector<std::unique_ptr<Node>> nodes, Node& parent)
{
  auto nodesToExtract = std::vector<Node*>{};

  for (const auto& node : nodes)
  {
    node->accept(kdl::overload(
      [&](
        auto&& thisLambda, WorldNode& worldNode) { worldNode.visitChildren(thisLambda); },
      [&](
        auto&& thisLambda, LayerNode& layerNode) { layerNode.visitChildren(thisLambda); },
      [&](GroupNode& groupNode) { nodesToExtract.push_back(&groupNode); },
      [&](auto&& thisLambda, EntityNode& entityNode) {
        if (isWorldspawn(entityNode.entity().classname()))
        {
          entityNode.visitChildren(thisLambda);
        }
        else
        {
          nodesToExtract.push_back(&entityNode);
        }
      },
      [&](BrushNode& brushNode) { nodesToExtract.push_back(&brushNode); },
      [&](PatchNode& patchNode) { nodesToExtract.push_back(&patchNode); }));
  }

  auto nodesToAdd = kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>>{};
  for (auto* node : nodesToExtract)
  {
    auto* nodeParent = node->parent();
    auto extractedNode =
      nodeParent
        ? nodeParent->removeChild(node)
        : std::move(*std::ranges::find(
            nodes, node, [](const auto& topLevelNode) { return topLevelNode.get(); }));
    nodesToAdd[&parent].push_back(std::move(extractedNode));
  }

  return nodesToAdd;
}

std::vector<IdType> allPersistentGroupIds(const Node& root)
{
  auto result = std::vector<IdType>{};
  root.accept(kdl::overload(
    [](auto&& thisLambda, const WorldNode& worldNode) {
      worldNode.visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const LayerNode& layerNode) {
      layerNode.visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const GroupNode& groupNode) {
      if (const auto persistentId = groupNode.persistentId())
      {
        result.push_back(*persistentId);
      }
      groupNode.visitChildren(thisLambda);
    },
    [](const EntityNode&) {},
    [](const BrushNode&) {},
    [](const PatchNode&) {}));
  return result;
}

void fixRedundantPersistentIds(
  const kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>>& nodesToAdd,
  const std::vector<IdType>& existingPersistentGroupIds)
{
  // parentheses (not braces) are required to select the container constructor instead of
  // the initializer_list constructor, which takes priority for braced argument lists
  auto persistentGroupIds = kdl::flat_set(existingPersistentGroupIds);
  for (const auto& [newParent, nodesToAddToParent] : nodesToAdd)
  {
    for (const auto& node : nodesToAddToParent)
    {
      node->accept(kdl::overload(
        [&](auto&& thisLambda, WorldNode& worldNode) {
          worldNode.visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, LayerNode& layerNode) {
          layerNode.visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, GroupNode& groupNode) {
          if (const auto persistentGroupId = groupNode.persistentId())
          {
            if (!persistentGroupIds.insert(*persistentGroupId).second)
            {
              // a group with this ID is already in the map or being pasted
              groupNode.resetPersistentId();
            }
          }
          groupNode.visitChildren(thisLambda);
        },
        [](EntityNode&) {},
        [](BrushNode&) {},
        [](PatchNode&) {}));
    }
  }
}

void fixRecursiveLinkedGroups(
  const kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>>& nodesToAdd,
  Logger& logger)
{
  for (const auto& [newParent, nodesToAddToParent] : nodesToAdd)
  {
    const auto linkedGroupIds = kdl::vec_sort(collectParentLinkedGroupIds(*newParent));
    for (const auto& node : nodesToAddToParent)
    {
      node->accept(kdl::overload(
        [&](auto&& thisLambda, WorldNode& worldNode) {
          worldNode.visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, LayerNode& layerNode) {
          layerNode.visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, GroupNode& groupNode) {
          const auto& linkId = groupNode.linkId();
          if (std::ranges::binary_search(linkedGroupIds, linkId))
          {
            logger.warn() << "Unlinking recursive linked group with ID '" << linkId
                          << "'";

            auto group = groupNode.group();
            group.setTransformation(vm::mat4x4d::identity());
            groupNode.setGroup(std::move(group));
            groupNode.setLinkId(generateUuid());
          }
          groupNode.visitChildren(thisLambda);
        },
        [](EntityNode&) {},
        [](BrushNode&) {},
        [](PatchNode&) {}));
    }
  }
}

void copyAndSetLinkIds(
  const kdl::flat_map<Node*, std::vector<std::unique_ptr<Node>>>& nodesToAdd,
  WorldNode& worldNode,
  Logger& logger)
{
  const auto errors = copyAndSetLinkIdsBeforeAddingNodes(nodesToAdd, worldNode);
  for (const auto& error : errors)
  {
    logger.warn() << "Could not paste linked groups: " + error.msg;
  }
}

bool pasteNodes(Map& map, std::vector<std::unique_ptr<Node>> nodes)
{
  auto nodesToAdd = extractNodesToPaste(std::move(nodes), parentForNodes(map));
  fixRedundantPersistentIds(nodesToAdd, allPersistentGroupIds(map.worldNode()));
  fixRecursiveLinkedGroups(nodesToAdd, map.logger());
  copyAndSetLinkIds(nodesToAdd, map.worldNode(), map.logger());

  auto transaction = Transaction{map, "Paste Nodes"};

  const auto addedNodes = addNodes(map, std::move(nodesToAdd));
  if (addedNodes.empty())
  {
    transaction.cancel();
    return false;
  }

  deselectAll(map);
  selectNodes(map, collectSelectableNodes(addedNodes, map.editorContext()));
  transaction.commit();

  return true;
}

bool pasteBrushFaces(Map& map, const std::vector<BrushFace>& faces)
{
  contract_pre(!faces.empty());

  const auto update = copyAllExceptContentFlags(faces.back());
  return setBrushFaceAttributes(map, update);
}

} // namespace

std::string serializeSelectedNodes(Map& map)
{
  auto stream = std::stringstream{};
  auto writer = NodeWriter{map.worldNode(), stream};
  writer.writeNodes(map.selection().nodes, map.taskManager());
  return stream.str();
}

std::string serializeSelectedBrushFaces(Map& map)
{
  auto stream = std::stringstream{};
  auto writer = NodeWriter{map.worldNode(), stream};
  writer.writeBrushFaces(
    map.selection().brushFaces | std::views::transform([](const auto& h) {
      return h.face();
    }) | kdl::ranges::to<std::vector>(),
    map.taskManager());
  return stream.str();
}

PasteType paste(Map& map, const std::string& str)
{
  auto parserStatus = SimpleParserStatus{map.logger()};

  // Try parsing as entities, then as brushes, in all compatible formats
  return NodeReader::read(
           str,
           map.worldNode().mapFormat(),
           map.worldBounds(),
           map.worldNode().entityPropertyConfig(),
           parserStatus,
           map.taskManager())
         | kdl::transform([&](auto nodes) {
             return pasteNodes(map, std::move(nodes)) ? PasteType::Node
                                                      : PasteType::Failed;
           })
         | kdl::or_else([&](const auto& nodeError) {
             // Try parsing as brush faces
             auto reader = BrushFaceReader{str, map.worldNode().mapFormat()};
             return reader.read(map.worldBounds(), parserStatus)
                    | kdl::transform([&](const auto& faces) {
                        return !faces.empty() && pasteBrushFaces(map, faces)
                                 ? PasteType::BrushFace
                                 : PasteType::Failed;
                      })
                    | kdl::transform_error([&](const auto& faceError) {
                        map.logger().error()
                          << "Could not parse clipboard contents as nodes: "
                          << nodeError.msg;
                        map.logger().error()
                          << "Could not parse clipboard contents as faces: "
                          << faceError.msg;
                        return PasteType::Failed;
                      });
           })
         | kdl::value();
}

} // namespace tb::mdl
