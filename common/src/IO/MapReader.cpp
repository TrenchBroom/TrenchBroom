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

#include "MapReader.h"

#include "IO/ParserStatus.h"
#include "Model/BrushError.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/EntityProperties.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/MapFormat.h"
#include "Model/PatchNode.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"

#include <vecmath/mat.h>
#include <vecmath/mat_io.h>

#include <kdl/parallel.h>
#include <kdl/result.h>
#include <kdl/result_for_each.h>
#include <kdl/string_format.h>
#include <kdl/string_utils.h>
#include <kdl/vector_utils.h>

#include <cassert>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace TrenchBroom
{
namespace IO
{
MapReader::MapReader(
  std::string_view str,
  const Model::MapFormat sourceMapFormat,
  const Model::MapFormat targetMapFormat,
  const Model::EntityPropertyConfig& entityPropertyConfig)
  : StandardMapParser(std::move(str), sourceMapFormat, targetMapFormat)
  , m_entityPropertyConfig{entityPropertyConfig}
{
}

void MapReader::readEntities(const vm::bbox3& worldBounds, ParserStatus& status)
{
  m_worldBounds = worldBounds;
  parseEntities(status);
  createNodes(status);
}

void MapReader::readBrushes(const vm::bbox3& worldBounds, ParserStatus& status)
{
  m_worldBounds = worldBounds;
  parseBrushesOrPatches(status);
  createNodes(status);
}

void MapReader::readBrushFaces(const vm::bbox3& worldBounds, ParserStatus& status)
{
  m_worldBounds = worldBounds;
  parseBrushFaces(status);
}

// implement MapParser interface

void MapReader::onBeginEntity(
  const size_t /* line */,
  std::vector<Model::EntityProperty> properties,
  ParserStatus& /* status */)
{
  m_currentEntityInfo = m_objectInfos.size();
  m_objectInfos.push_back(EntityInfo{std::move(properties), 0, 0});
}

void MapReader::onEndEntity(
  const size_t startLine, const size_t lineCount, ParserStatus& /* status */)
{
  assert(m_currentEntityInfo != std::nullopt);
  assert(std::holds_alternative<EntityInfo>(m_objectInfos[*m_currentEntityInfo]));

  EntityInfo& entity = std::get<EntityInfo>(m_objectInfos[*m_currentEntityInfo]);
  entity.startLine = startLine;
  entity.lineCount = lineCount;

  m_currentEntityInfo = std::nullopt;
}

void MapReader::onBeginBrush(const size_t /* line */, ParserStatus& /* status */)
{
  m_objectInfos.push_back(BrushInfo{{}, 0, 0, m_currentEntityInfo});
}

void MapReader::onEndBrush(
  const size_t startLine, const size_t lineCount, ParserStatus& /* status */)
{
  assert(std::holds_alternative<BrushInfo>(m_objectInfos.back()));

  BrushInfo& brush = std::get<BrushInfo>(m_objectInfos.back());
  brush.startLine = startLine;
  brush.lineCount = lineCount;
}

void MapReader::onStandardBrushFace(
  const size_t line,
  const Model::MapFormat targetMapFormat,
  const vm::vec3& point1,
  const vm::vec3& point2,
  const vm::vec3& point3,
  const Model::BrushFaceAttributes& attribs,
  ParserStatus& status)
{
  Model::BrushFace::createFromStandard(point1, point2, point3, attribs, targetMapFormat)
    .and_then([&](Model::BrushFace&& face) {
      face.setFilePosition(line, 1u);
      onBrushFace(std::move(face), status);
    })
    .handle_errors([&](const Model::BrushError e) {
      status.error(line, kdl::str_to_string("Skipping face: ", e));
    });
}

void MapReader::onValveBrushFace(
  const size_t line,
  const Model::MapFormat targetMapFormat,
  const vm::vec3& point1,
  const vm::vec3& point2,
  const vm::vec3& point3,
  const Model::BrushFaceAttributes& attribs,
  const vm::vec3& texAxisX,
  const vm::vec3& texAxisY,
  ParserStatus& status)
{
  Model::BrushFace::createFromValve(
    point1, point2, point3, attribs, texAxisX, texAxisY, targetMapFormat)
    .and_then([&](Model::BrushFace&& face) {
      face.setFilePosition(line, 1u);
      onBrushFace(std::move(face), status);
    })
    .handle_errors([&](const Model::BrushError e) {
      status.error(line, kdl::str_to_string("Skipping face: ", e));
    });
}

void MapReader::onPatch(
  const size_t startLine,
  const size_t lineCount,
  Model::MapFormat,
  const size_t rowCount,
  const size_t columnCount,
  std::vector<vm::vec<FloatType, 5>> controlPoints,
  std::string textureName,
  ParserStatus&)
{
  m_objectInfos.push_back(PatchInfo{
    rowCount,
    columnCount,
    std::move(controlPoints),
    std::move(textureName),
    startLine,
    lineCount,
    m_currentEntityInfo});
}

// helper methods

namespace
{
/** The type of a node's container. */
enum class ContainerType
{
  Layer,
  Group,
};

/** Records the container of a group or entity node. */
struct ContainerInfo
{
  ContainerType type;
  Model::IdType id;
};

/**
 * Represents the parent information stored in ObjectInfo, which is either the index of
 * the object info for the parent, or container info read from entity properties.
 */
using ParentInfo = std::variant<size_t, ContainerInfo>;

/** A linked group node has a missing or malformed transformation. */
struct MalformedTransformationIssue
{
  std::string transformationStr;
};

/** A group or entity node contained a malformed container ID. */
struct InvalidContainerId
{
  ContainerType type;
  std::string idStr;
};

/**
 * Records issues that occured during node creation. These issues did not prevent node
 * creation, but they must be logged nevertheless.
 */
using NodeIssue = std::variant<MalformedTransformationIssue, InvalidContainerId>;

/**
 * The data returned by the functions that create nodes.
 */
struct NodeInfo
{
  std::unique_ptr<Model::Node> node;
  std::optional<ParentInfo> parentInfo;
  std::vector<NodeIssue> issues;
};

/** Records errors that occur during node creation. These errors did prevent node
 * creation. */
struct NodeError
{
  size_t line;
  std::string msg;
};
} // namespace

/** This is the result returned from functions that create nodes. */
using CreateNodeResult = kdl::result<NodeInfo, NodeError>;

/**
 * Extracts container info (either a layer or a group ID) from the given entity properties
 * if present. In case of a malformed ID, an issue is added to the given vector of node
 * issues and an empty optional is returned.
 */
static std::optional<ContainerInfo> extractContainerInfo(
  const std::vector<Model::EntityProperty>& properties,
  std::vector<NodeIssue>& nodeIssues)
{
  const std::string& parentLayerIdStr =
    findEntityPropertyOrDefault(properties, Model::EntityPropertyKeys::Layer);
  if (!kdl::str_is_blank(parentLayerIdStr))
  {
    const auto rawParentLayerId = kdl::str_to_long(parentLayerIdStr);
    if (rawParentLayerId && *rawParentLayerId >= 0)
    {
      const Model::IdType parentLayerId = static_cast<Model::IdType>(*rawParentLayerId);
      return ContainerInfo{ContainerType::Layer, parentLayerId};
    }

    nodeIssues.emplace_back(InvalidContainerId{ContainerType::Layer, parentLayerIdStr});
    return std::nullopt;
  }

  const std::string& parentGroupIdStr =
    findEntityPropertyOrDefault(properties, Model::EntityPropertyKeys::Group);
  if (!kdl::str_is_blank(parentGroupIdStr))
  {
    const auto rawParentGroupId = kdl::str_to_long(parentGroupIdStr);
    if (rawParentGroupId && *rawParentGroupId >= 0)
    {
      const Model::IdType parentGroupId = static_cast<Model::IdType>(*rawParentGroupId);
      return ContainerInfo{ContainerType::Group, parentGroupId};
    }

    nodeIssues.emplace_back(InvalidContainerId{ContainerType::Group, parentGroupIdStr});
    return std::nullopt;
  }

  return std::nullopt;
}

/**
 * Creates a world node for the given entity info and configures its default layer
 * according to the information in the entity attributes.
 */
static CreateNodeResult createWorldNode(
  MapReader::EntityInfo entityInfo,
  const Model::EntityPropertyConfig& entityPropertyConfig,
  const Model::MapFormat mapFormat)
{
  auto entity = Model::Entity{entityPropertyConfig, std::move(entityInfo.properties)};
  auto worldNode =
    std::make_unique<Model::WorldNode>(entityPropertyConfig, Model::Entity{}, mapFormat);
  worldNode->setFilePosition(entityInfo.startLine, entityInfo.lineCount);

  // handle default layer attributes, which are stored in worldspawn
  auto* defaultLayerNode = worldNode->defaultLayer();
  auto defaultLayer = defaultLayerNode->layer();
  if (const auto* colorStr = entity.property(Model::EntityPropertyKeys::LayerColor))
  {
    if (const auto color = Color::parse(*colorStr))
    {
      defaultLayer.setColor(*color);
    }
    entity.removeProperty(entityPropertyConfig, Model::EntityPropertyKeys::LayerColor);
  }
  if (
    const auto* omitFromExportStr =
      entity.property(Model::EntityPropertyKeys::LayerOmitFromExport))
  {
    if (*omitFromExportStr == Model::EntityPropertyValues::LayerOmitFromExportValue)
    {
      defaultLayer.setOmitFromExport(true);
    }
    entity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::LayerOmitFromExport);
  }
  defaultLayerNode->setLayer(std::move(defaultLayer));

  if (const auto* lockedStr = entity.property(Model::EntityPropertyKeys::LayerLocked))
  {
    if (*lockedStr == Model::EntityPropertyValues::LayerLockedValue)
    {
      defaultLayerNode->setLockState(Model::LockState::Locked);
    }
    entity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::LayerOmitFromExport);
  }
  if (const auto* hiddenStr = entity.property(Model::EntityPropertyKeys::LayerHidden))
  {
    if (*hiddenStr == Model::EntityPropertyValues::LayerHiddenValue)
    {
      defaultLayerNode->setVisibilityState(Model::VisibilityState::Hidden);
    }
    entity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::LayerOmitFromExport);
  }

  worldNode->setEntity(std::move(entity));

  return NodeInfo{
    std::move(worldNode),
    {}, // parent info
    {}  // issues
  };
}

/**
 * Creates a layer node for the given entity info. Returns an error if the entity
 * attributes contain missing or invalid information.
 */
static CreateNodeResult createLayerNode(const MapReader::EntityInfo& entityInfo)
{
  const auto& properties = entityInfo.properties;

  const std::string& name =
    findEntityPropertyOrDefault(properties, Model::EntityPropertyKeys::LayerName);
  if (kdl::str_is_blank(name))
  {
    return NodeError{entityInfo.startLine, "Skipping layer entity: missing name"};
  }

  const std::string& idStr =
    findEntityPropertyOrDefault(properties, Model::EntityPropertyKeys::LayerId);
  if (kdl::str_is_blank(idStr))
  {
    return NodeError{entityInfo.startLine, "Skipping layer entity: missing id"};
  }

  const auto rawId = kdl::str_to_size(idStr);
  if (!rawId || *rawId <= 0u)
  {
    return NodeError{
      entityInfo.startLine,
      kdl::str_to_string("Skipping layer entity: '", idStr, "' is not a valid id")};
  }

  Model::Layer layer = Model::Layer(name);
  // This is optional (not present on maps saved in TB 2020.1 and earlier)
  if (
    const auto layerSortIndex = kdl::str_to_int(
      findEntityPropertyOrDefault(properties, Model::EntityPropertyKeys::LayerSortIndex)))
  {
    layer.setSortIndex(*layerSortIndex);
  }

  if (
    findEntityPropertyOrDefault(
      properties, Model::EntityPropertyKeys::LayerOmitFromExport)
    == Model::EntityPropertyValues::LayerOmitFromExportValue)
  {
    layer.setOmitFromExport(true);
  }

  auto layerNode = std::make_unique<Model::LayerNode>(std::move(layer));
  layerNode->setFilePosition(entityInfo.startLine, entityInfo.lineCount);

  const Model::IdType layerId = static_cast<Model::IdType>(*rawId);
  layerNode->setPersistentId(layerId);

  if (
    findEntityPropertyOrDefault(properties, Model::EntityPropertyKeys::LayerLocked)
    == Model::EntityPropertyValues::LayerLockedValue)
  {
    layerNode->setLockState(Model::LockState::Locked);
  }
  if (
    findEntityPropertyOrDefault(properties, Model::EntityPropertyKeys::LayerHidden)
    == Model::EntityPropertyValues::LayerHiddenValue)
  {
    layerNode->setVisibilityState(Model::VisibilityState::Hidden);
  }

  return NodeInfo{
    std::move(layerNode),
    {}, // parent info
    {}  // issues
  };
}

/**
 * Creates a group node for the given entity info. Returns an error if the entity
 * attributes contain missing or invalid information.
 */
static CreateNodeResult createGroupNode(const MapReader::EntityInfo& entityInfo)
{
  const std::string& name = findEntityPropertyOrDefault(
    entityInfo.properties, Model::EntityPropertyKeys::GroupName);
  if (kdl::str_is_blank(name))
  {
    return NodeError{entityInfo.startLine, "Skipping group entity: missing name"};
  }

  const std::string& idStr = findEntityPropertyOrDefault(
    entityInfo.properties, Model::EntityPropertyKeys::GroupId);
  if (kdl::str_is_blank(idStr))
  {
    return NodeError{entityInfo.startLine, "Skipping group entity: missing id"};
  }

  const auto rawId = kdl::str_to_size(idStr);
  if (!rawId || *rawId <= 0)
  {
    return NodeError{
      entityInfo.startLine,
      kdl::str_to_string("Skipping group entity: '", idStr, "' is not a valid id")};
  }

  auto group = Model::Group{name};
  auto nodeIssues = std::vector<NodeIssue>{};

  const std::string& linkedGroupId = findEntityPropertyOrDefault(
    entityInfo.properties, Model::EntityPropertyKeys::LinkedGroupId);
  if (!linkedGroupId.empty())
  {
    const std::string& transformationStr = findEntityPropertyOrDefault(
      entityInfo.properties, Model::EntityPropertyKeys::GroupTransformation);
    if (const auto transformation = vm::parse<FloatType, 4u, 4u>(transformationStr))
    {
      group.setLinkedGroupId(linkedGroupId);
      group.setTransformation(*transformation);
    }
    else
    {
      nodeIssues.emplace_back(MalformedTransformationIssue{transformationStr});
    }
  }

  auto groupNode = std::make_unique<Model::GroupNode>(std::move(group));
  groupNode->setFilePosition(entityInfo.startLine, entityInfo.lineCount);

  const Model::IdType groupId = static_cast<Model::IdType>(*rawId);
  groupNode->setPersistentId(groupId);

  auto containerInfo = extractContainerInfo(entityInfo.properties, nodeIssues);

  return NodeInfo{
    std::move(groupNode),
    std::move(containerInfo),
    std::move(nodeIssues),
  };
}

/**
 * Creates an entity node for the given entity info.
 */
static CreateNodeResult createEntityNode(
  const Model::EntityPropertyConfig& entityPropertyConfig,
  MapReader::EntityInfo entityInfo)
{
  auto entity = Model::Entity{entityPropertyConfig, std::move(entityInfo.properties)};
  if (
    const auto* protectedPropertiesStr =
      entity.property(Model::EntityPropertyKeys::ProtectedEntityProperties))
  {
    auto protectedProperties = kdl::str_split(*protectedPropertiesStr, ";");
    entity.setProtectedProperties(std::move(protectedProperties));
    entity.removeProperty(
      entityPropertyConfig, Model::EntityPropertyKeys::ProtectedEntityProperties);
  }

  auto nodeIssues = std::vector<NodeIssue>{};
  auto containerInfo = extractContainerInfo(entity.properties(), nodeIssues);

  // strip container properties
  entity.removeProperty(entityPropertyConfig, Model::EntityPropertyKeys::Layer);
  entity.removeProperty(entityPropertyConfig, Model::EntityPropertyKeys::Group);

  auto entityNode = std::make_unique<Model::EntityNode>(std::move(entity));
  entityNode->setFilePosition(entityInfo.startLine, entityInfo.lineCount);

  return NodeInfo{std::move(entityNode), std::move(containerInfo), std::move(nodeIssues)};
}

/**
 * Creates a world, layer, group or entity node depending on the information stored in the
 * given entity info.
 *
 * Returns an error if the node could not be created.
 */
static CreateNodeResult createNodeFromEntityInfo(
  const Model::EntityPropertyConfig& entityPropertyConfig,
  MapReader::EntityInfo entityInfo,
  const Model::MapFormat mapFormat)
{
  const auto& classname = findEntityPropertyOrDefault(
    entityInfo.properties, Model::EntityPropertyKeys::Classname);
  if (isWorldspawn(classname, entityInfo.properties))
  {
    return createWorldNode(std::move(entityInfo), entityPropertyConfig, mapFormat);
  }
  else if (isLayer(classname, entityInfo.properties))
  {
    return createLayerNode(entityInfo);
  }
  else if (isGroup(classname, entityInfo.properties))
  {
    return createGroupNode(entityInfo);
  }
  else
  {
    return createEntityNode(entityPropertyConfig, std::move(entityInfo));
  }
}

/**
 * Creates a brush node from the given brush info. Returns an error if the brush could not
 * be created.
 */
static CreateNodeResult createBrushNode(
  MapReader::BrushInfo brushInfo, const vm::bbox3& worldBounds)
{
  return Model::Brush::create(worldBounds, std::move(brushInfo.faces))
    .and_then([&](Model::Brush&& brush) {
      auto brushNode = std::make_unique<Model::BrushNode>(std::move(brush));
      brushNode->setFilePosition(brushInfo.startLine, brushInfo.lineCount);

      auto parentInfo = brushInfo.parentIndex ? ParentInfo{*brushInfo.parentIndex}
                                              : std::optional<ParentInfo>{};
      return NodeInfo{
        std::move(brushNode), std::move(parentInfo), {} // issues
      };
    })
    .map_errors([&](const Model::BrushError e) {
      return CreateNodeResult{NodeError{brushInfo.startLine, kdl::str_to_string(e)}};
    });
}

/**
 * Creates a patch node from the given patch info.
 */
static CreateNodeResult createPatchNode(MapReader::PatchInfo patchInfo)
{
  auto patchNode = std::make_unique<Model::PatchNode>(Model::BezierPatch{
    patchInfo.rowCount,
    patchInfo.columnCount,
    std::move(patchInfo.controlPoints),
    std::move(patchInfo.textureName)});
  patchNode->setFilePosition(patchInfo.startLine, patchInfo.lineCount);

  auto parentInfo = patchInfo.parentIndex ? ParentInfo{*patchInfo.parentIndex}
                                          : std::optional<ParentInfo>{};

  return NodeInfo{
    std::move(patchNode), std::move(parentInfo), {} // issues
  };
}

/**
 * Transforms the given object infos into a vector of node infos. The returned vector is
 * sparse, that is, it contains empty optionals in place of nodes that we failed to
 * create. We need the indices to remain correct because we use them to refer to parent
 * nodes later.
 */
static std::vector<std::optional<NodeInfo>> createNodesFromObjectInfos(
  const Model::EntityPropertyConfig& entityPropertyConfig,
  std::vector<MapReader::ObjectInfo> objectInfos,
  const vm::bbox3& worldBounds,
  const Model::MapFormat mapFormat,
  ParserStatus& status)
{
  // create nodes in parallel, moving data out of objectInfos
  // we store optionals in the result vector to make the elements default constructible,
  // which is a requirement for parallel transform
  std::vector<CreateNodeResult> createNodeResults = kdl::vec_parallel_transform(
    std::move(objectInfos), [&](auto&& objectInfo) -> CreateNodeResult {
      return std::visit(
        kdl::overload(
          [&](MapReader::EntityInfo&& entityInfo) {
            return createNodeFromEntityInfo(
              entityPropertyConfig, std::move(entityInfo), mapFormat);
          },
          [&](MapReader::BrushInfo&& brushInfo) {
            return createBrushNode(std::move(brushInfo), worldBounds);
          },
          [&](MapReader::PatchInfo&& patchInfo) {
            return createPatchNode(std::move(patchInfo));
          }),
        std::move(objectInfo));
    });

  return kdl::vec_transform(
    std::move(createNodeResults),
    [&](std::optional<CreateNodeResult>&& createNodeResult) -> std::optional<NodeInfo> {
      assert(createNodeResult.has_value());

      return std::move(*createNodeResult)
        .visit(kdl::overload(
          [&](NodeInfo&& nodeInfo) -> std::optional<NodeInfo> {
            return std::move(nodeInfo);
          },
          [&](const NodeError& e) -> std::optional<NodeInfo> {
            status.error(e.line, e.msg);
            return std::nullopt;
          }));
    });
}

/**
 * Validates the given node infos.
 *
 * - Removes layer and group nodes with duplicate IDs.
 * - Logs issues that occured during node creation.
 */
static void validateNodes(
  std::vector<std::optional<NodeInfo>>& nodeInfos, ParserStatus& status)
{
  auto layerIds = std::unordered_set<Model::IdType>{};
  auto groupIds = std::unordered_set<Model::IdType>{};

  for (auto& nodeInfo : nodeInfos)
  {
    if (nodeInfo)
    {
      const bool clearNode = nodeInfo->node->accept(kdl::overload(
        [](Model::WorldNode*) { return false; },
        [&](Model::LayerNode* layerNode) {
          const auto persistentId = *layerNode->persistentId();
          if (!layerIds.emplace(persistentId).second)
          {
            status.error(
              layerNode->lineNumber(),
              kdl::str_to_string(
                "Skipping duplicate layer with ID '", persistentId, "'"));
            return true;
          }
          return false;
        },
        [&](Model::GroupNode* groupNode) {
          const auto persistentId = *groupNode->persistentId();
          if (!groupIds.emplace(persistentId).second)
          {
            status.error(
              groupNode->lineNumber(),
              kdl::str_to_string(
                "Skipping duplicate group with ID '", persistentId, "'"));
            return true;
          }
          return false;
        },
        [](Model::EntityNode*) { return false; },
        [](Model::BrushNode*) { return false; },
        [](Model::PatchNode*) { return false; }));

      if (clearNode)
      {
        nodeInfo.reset();
      }
      else
      {
        // log issues
        for (const auto& issue : nodeInfo->issues)
        {
          std::visit(
            kdl::overload(
              [&](const MalformedTransformationIssue& m) {
                status.warn(
                  nodeInfo->node->lineNumber(),
                  kdl::str_to_string(
                    "Not linking group: malformed transformation '",
                    m.transformationStr,
                    "'"));
              },
              [&](const InvalidContainerId& c) {
                const auto containerName =
                  c.type == ContainerType::Layer ? "layer" : "group";
                status.warn(
                  nodeInfo->node->lineNumber(),
                  kdl::str_to_string(
                    "Adding object to default layer: Invalid ",
                    containerName,
                    " ID '",
                    c.idStr,
                    "'"));
              }),
            issue);
        }
        nodeInfo->issues.clear();
      }
    }
  }
}

using NodeToParentMap = std::unordered_map<Model::Node*, Model::Node*>;

/**
 * Builds a map of nodes to their intended parents using the parent info stored in each
 * node info object (this either refers to a parent node by index or a parent layer or
 * group by ID).
 *
 * Not every node comes with parent information, so the returned map does not contain
 * entries for each of the given nodes.
 */
static NodeToParentMap buildNodeToParentMap(
  std::vector<std::optional<NodeInfo>>& nodeInfos, ParserStatus& status)
{
  auto layerIdMap = std::unordered_map<Model::IdType, Model::LayerNode*>{};
  auto groupIdMap = std::unordered_map<Model::IdType, Model::GroupNode*>{};

  for (const auto& nodeInfo : nodeInfos)
  {
    if (nodeInfo)
    {
      nodeInfo->node->accept(kdl::overload(
        [](Model::WorldNode*) {},
        [&](Model::LayerNode* layerNode) {
          const auto persistentId = *layerNode->persistentId();
          assertResult(layerIdMap.emplace(persistentId, layerNode).second);
        },
        [&](Model::GroupNode* groupNode) {
          const auto persistentId = *groupNode->persistentId();
          assertResult(groupIdMap.emplace(persistentId, groupNode).second);
        },
        [](Model::EntityNode*) {},
        [](Model::BrushNode*) {},
        [](Model::PatchNode*) {}));
    }
  }

  const auto findContainerNode = [&](const ContainerInfo& containerInfo) -> Model::Node* {
    switch (containerInfo.type)
    {
    case ContainerType::Layer:
      if (auto layerIt = layerIdMap.find(containerInfo.id);
          layerIt != std::end(layerIdMap))
      {
        return layerIt->second;
      }
      break;
    case ContainerType::Group:
      if (auto groupIt = groupIdMap.find(containerInfo.id);
          groupIt != std::end(groupIdMap))
      {
        return groupIt->second;
      }
      break;
    }
    return nullptr;
  };

  // maps a node to its intended parent
  auto nodeToParentMap = NodeToParentMap{};
  for (const auto& nodeInfo : nodeInfos)
  {
    if (nodeInfo && nodeInfo->parentInfo)
    {
      std::visit(
        kdl::overload(
          [&](const size_t parentIndex) {
            if (const auto& parentNodeInfo = nodeInfos[parentIndex])
            {
              nodeToParentMap.emplace(nodeInfo->node.get(), parentNodeInfo->node.get());
            }
          },
          [&](const ContainerInfo& containerInfo) {
            if (auto* containerNode = findContainerNode(containerInfo))
            {
              nodeToParentMap.emplace(nodeInfo->node.get(), containerNode);
            }
            else
            {
              if (containerInfo.type == ContainerType::Layer)
              {
                status.warn(
                  nodeInfo->node->lineNumber(),
                  kdl::str_to_string(
                    "Entity references missing layer '",
                    containerInfo.id,
                    "', adding to default layer"));
              }
              else
              {
                status.warn(
                  nodeInfo->node->lineNumber(),
                  kdl::str_to_string(
                    "Entity references missing group '",
                    containerInfo.id,
                    "', adding to default layer"));
              }
            }
          }),
        *nodeInfo->parentInfo);
    }
  }
  return nodeToParentMap;
}

/**
 * Creates nodes from the recorded object infos and resolves parent / child relationships.
 *
 * Brushes should be added to the node corresponding to the preceding recorded entity
 * info. We stored the index of the preceding entity info for each brush, so we can
 * determine the parent node for a brush using that index.
 *
 * Group and entity nodes can belong to the default layer, a custom layer or another
 * group. If such a node belongs to a custom layer or a group, the ID of the containing
 * layer or group is stored in the entity properties of the entity info from which the
 * node was created. Since the entity properties of these nodes are discarded when the
 * node is created, we record this information in a separate map before creating we create
 * nodes. We later use it to find the parent layer or group of a group or entity node.
 *
 * Nodes for which the parent node is not known (e.g. when parsing only brushes) are added
 * to a default parent, which is returned from the `onWorldNode` callback.
 */
void MapReader::createNodes(ParserStatus& status)
{
  // create nodes from the recorded object infos
  auto nodeInfos = createNodesFromObjectInfos(
    m_entityPropertyConfig,
    std::move(m_objectInfos),
    m_worldBounds,
    m_targetMapFormat,
    status);

  // call onWorldNode for the first world node, remember the default parent and clear out
  // all other world nodes the brushes belonging to redundant world nodes will be added to
  // the default parent
  Model::Node* defaultParent = nullptr;
  for (auto& nodeInfo : nodeInfos)
  {
    if (nodeInfo)
    {
      if (dynamic_cast<Model::WorldNode*>(nodeInfo->node.get()))
      {
        if (!defaultParent)
        {
          auto worldNode = std::unique_ptr<Model::WorldNode>{
            static_cast<Model::WorldNode*>(nodeInfo->node.release())};
          defaultParent = onWorldNode(std::move(worldNode), status);
        }
        nodeInfo.reset();
      }
    }
  }

  validateNodes(nodeInfos, status);

  // build a map that maps nodes to their intended parents
  // if a node is not in this map, we will pass defaultParent to the callbacks for the
  // parent
  const auto nodeToParentMap = buildNodeToParentMap(nodeInfos, status);

  // call the callbacks now
  for (auto& nodeInfo : nodeInfos)
  {
    if (nodeInfo)
    {
      auto& node = nodeInfo->node;
      const auto parentNodeIt = nodeToParentMap.find(node.get());
      auto* parentNode =
        parentNodeIt != std::end(nodeToParentMap) ? parentNodeIt->second : defaultParent;

      node->accept(kdl::overload(
        [&](Model::WorldNode*) {
          // this should not happen since we already cleared out any world nodes
        },
        [&](Model::LayerNode*) { onLayerNode(std::move(node), status); },
        [&](Model::GroupNode*) { onNode(parentNode, std::move(node), status); },
        [&](Model::EntityNode*) { onNode(parentNode, std::move(node), status); },
        [&](Model::BrushNode*) { onNode(parentNode, std::move(node), status); },
        [&](Model::PatchNode*) { onNode(parentNode, std::move(node), status); }));
    }
  }
}

/**
 * Default implementation adds it to the current BrushInfo
 * Overridden in BrushFaceReader (which doesn't use m_brushInfos) to collect the faces
 * directly
 */
void MapReader::onBrushFace(Model::BrushFace face, ParserStatus& /* status */)
{
  assert(std::holds_alternative<BrushInfo>(m_objectInfos.back()));

  BrushInfo& brush = std::get<BrushInfo>(m_objectInfos.back());
  brush.faces.push_back(std::move(face));
}
} // namespace IO
} // namespace TrenchBroom
