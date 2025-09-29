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

#include "Map_CopyPaste.h"

#include "Logger.h"
#include "Uuid.h"
#include "io/BrushFaceReader.h"
#include "io/NodeReader.h"
#include "io/NodeWriter.h"
#include "io/SimpleParserStatus.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
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
#include "mdl/PasteType.h"
#include "mdl/PatchNode.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"

#include "kdl/ranges/to.h"
#include "kdl/vector_utils.h"

#include <ranges>

namespace tb::mdl
{
namespace
{

auto extractNodesToPaste(const std::vector<Node*>& nodes, Node* parent)
{
  auto nodesToDetach = std::vector<Node*>{};
  auto nodesToDelete = std::vector<Node*>{};
  auto nodesToAdd = std::map<Node*, std::vector<Node*>>{};

  for (auto* node : nodes)
  {
    node->accept(kdl::overload(
      [&](auto&& thisLambda, WorldNode* world) {
        world->visitChildren(thisLambda);
        nodesToDelete.push_back(world);
      },
      [&](auto&& thisLambda, LayerNode* layer) {
        layer->visitChildren(thisLambda);
        nodesToDetach.push_back(layer);
        nodesToDelete.push_back(layer);
      },
      [&](GroupNode* group) {
        nodesToDetach.push_back(group);
        nodesToAdd[parent].push_back(group);
      },
      [&](auto&& thisLambda, EntityNode* entityNode) {
        if (isWorldspawn(entityNode->entity().classname()))
        {
          entityNode->visitChildren(thisLambda);
          nodesToDetach.push_back(entityNode);
          nodesToDelete.push_back(entityNode);
        }
        else
        {
          nodesToDetach.push_back(entityNode);
          nodesToAdd[parent].push_back(entityNode);
        }
      },
      [&](BrushNode* brush) {
        nodesToDetach.push_back(brush);
        nodesToAdd[parent].push_back(brush);
      },
      [&](PatchNode* patch) {
        nodesToDetach.push_back(patch);
        nodesToAdd[parent].push_back(patch);
      }));
  }

  for (auto* node : nodesToDetach)
  {
    if (auto* nodeParent = node->parent())
    {
      nodeParent->removeChild(node);
    }
  }
  kdl::vec_clear_and_delete(nodesToDelete);

  return nodesToAdd;
}

std::vector<IdType> allPersistentGroupIds(const Node& root)
{
  auto result = std::vector<IdType>{};
  root.accept(kdl::overload(
    [](auto&& thisLambda, const WorldNode* worldNode) {
      worldNode->visitChildren(thisLambda);
    },
    [](auto&& thisLambda, const LayerNode* layerNode) {
      layerNode->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, const GroupNode* groupNode) {
      if (const auto persistentId = groupNode->persistentId())
      {
        result.push_back(*persistentId);
      }
      groupNode->visitChildren(thisLambda);
    },
    [](const EntityNode*) {},
    [](const BrushNode*) {},
    [](const PatchNode*) {}));
  return result;
}

void fixRedundantPersistentIds(
  const std::map<Node*, std::vector<Node*>>& nodesToAdd,
  const std::vector<IdType>& existingPersistentGroupIds)
{
  auto persistentGroupIds = kdl::vector_set{existingPersistentGroupIds};
  for (auto& [newParent, nodesToAddToParent] : nodesToAdd)
  {
    for (auto* node : nodesToAddToParent)
    {
      node->accept(kdl::overload(
        [&](auto&& thisLambda, WorldNode* worldNode) {
          worldNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, LayerNode* layerNode) {
          layerNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, GroupNode* groupNode) {
          if (const auto persistentGroupId = groupNode->persistentId())
          {
            if (!persistentGroupIds.insert(*persistentGroupId).second)
            {
              // a group with this ID is already in the map or being pasted
              groupNode->resetPersistentId();
            }
          }
          groupNode->visitChildren(thisLambda);
        },
        [](EntityNode*) {},
        [](BrushNode*) {},
        [](PatchNode*) {}));
    }
  }
}

void fixRecursiveLinkedGroups(
  const std::map<Node*, std::vector<Node*>>& nodesToAdd, Logger& logger)
{
  for (auto& [newParent, nodesToAddToParent] : nodesToAdd)
  {
    const auto linkedGroupIds = kdl::vec_sort(collectParentLinkedGroupIds(*newParent));
    for (auto* node : nodesToAddToParent)
    {
      node->accept(kdl::overload(
        [&](auto&& thisLambda, WorldNode* worldNode) {
          worldNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, LayerNode* layerNode) {
          layerNode->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, GroupNode* groupNode) {
          const auto& linkId = groupNode->linkId();
          if (std::binary_search(linkedGroupIds.begin(), linkedGroupIds.end(), linkId))
          {
            logger.warn() << "Unlinking recursive linked group with ID '" << linkId
                          << "'";

            auto group = groupNode->group();
            group.setTransformation(vm::mat4x4d::identity());
            groupNode->setGroup(std::move(group));
            groupNode->setLinkId(generateUuid());
          }
          groupNode->visitChildren(thisLambda);
        },
        [](EntityNode*) {},
        [](BrushNode*) {},
        [](PatchNode*) {}));
    }
  }
}

void copyAndSetLinkIds(
  const std::map<Node*, std::vector<Node*>>& nodesToAdd,
  WorldNode& worldNode,
  Logger& logger)
{
  const auto errors = copyAndSetLinkIdsBeforeAddingNodes(nodesToAdd, worldNode);
  for (const auto& error : errors)
  {
    logger.warn() << "Could not paste linked groups: " + error.msg;
  }
}

bool pasteNodes(Map& map, const std::vector<Node*>& nodes)
{
  const auto nodesToAdd = extractNodesToPaste(nodes, parentForNodes(map));
  fixRedundantPersistentIds(nodesToAdd, allPersistentGroupIds(*map.world()));
  fixRecursiveLinkedGroups(nodesToAdd, map.logger());
  copyAndSetLinkIds(nodesToAdd, *map.world(), map.logger());

  auto transaction = Transaction{map, "Paste Nodes"};

  const auto addedNodes = addNodes(map, nodesToAdd);
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
  assert(!faces.empty());
  return setBrushFaceAttributesExceptContentFlags(map, faces.back().attributes());
}

} // namespace

std::string serializeSelectedNodes(Map& map)
{
  auto stream = std::stringstream{};
  auto writer = io::NodeWriter{*map.world(), stream};
  writer.writeNodes(map.selection().nodes, map.taskManager());
  return stream.str();
}

std::string serializeSelectedBrushFaces(Map& map)
{
  auto stream = std::stringstream{};
  auto writer = io::NodeWriter{*map.world(), stream};
  writer.writeBrushFaces(
    map.selection().brushFaces | std::views::transform([](const auto& h) {
      return h.face();
    }) | kdl::ranges::to<std::vector>(),
    map.taskManager());
  return stream.str();
}

PasteType paste(Map& map, const std::string& str)
{
  auto parserStatus = io::SimpleParserStatus{map.logger()};

  // Try parsing as entities, then as brushes, in all compatible formats
  return io::NodeReader::read(
           str,
           map.world()->mapFormat(),
           map.worldBounds(),
           map.world()->entityPropertyConfig(),
           parserStatus,
           map.taskManager())
         | kdl::transform([&](auto nodes) {
             return pasteNodes(map, nodes) ? PasteType::Node : PasteType::Failed;
           })
         | kdl::or_else([&](const auto& nodeError) {
             // Try parsing as brush faces
             auto reader = io::BrushFaceReader{str, map.world()->mapFormat()};
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
