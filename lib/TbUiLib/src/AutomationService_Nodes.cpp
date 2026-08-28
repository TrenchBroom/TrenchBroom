/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "AutomationJson.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandles.h"
#include "mdl/PasteType.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/AutomationService.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace tb::ui
{
namespace
{

bool expectedRevisionMatches(const mdl::Map& map, const QJsonObject& params)
{
  const auto expectedRevision =
    automation::sizeFromJson(params.value("expectedRevision"));
  return expectedRevision && *expectedRevision == map.modificationCount();
}

std::optional<std::vector<mdl::BrushNode*>> resolveBrushPaths(
  mdl::Map& map, const QJsonValue& pathsValue)
{
  if (!pathsValue.isArray())
  {
    return std::nullopt;
  }

  auto brushes = std::vector<mdl::BrushNode*>{};
  for (const auto& value : pathsValue.toArray())
  {
    const auto path = automation::nodePathFromJson(value);
    auto* node = path ? map.worldNode().resolvePath(*path) : nullptr;
    auto* brush = dynamic_cast<mdl::BrushNode*>(node);
    if (brush == nullptr || std::ranges::find(brushes, brush) != brushes.end())
    {
      return std::nullopt;
    }
    brushes.push_back(brush);
  }
  return brushes;
}

std::optional<std::vector<QString>> stringListFromJson(const QJsonValue& value)
{
  if (!value.isArray())
  {
    return std::nullopt;
  }

  auto result = std::vector<QString>{};
  for (const auto& element : value.toArray())
  {
    if (!element.isString() || element.toString().isEmpty())
    {
      return std::nullopt;
    }
    if (std::ranges::find(result, element.toString()) == result.end())
    {
      result.push_back(element.toString());
    }
  }
  return result;
}

QJsonArray optimizationCandidatesToJson(
  const std::vector<mdl::BrushOptimizationCandidate>& candidates,
  const size_t sourceBrushCount)
{
  auto result = QJsonArray{};
  for (size_t index = 0u; index < candidates.size(); ++index)
  {
    const auto& candidate = candidates[index];
    result.push_back(QJsonObject{
      {"index", static_cast<qint64>(index)},
      {"kind", candidate.brushes.empty() ? "axisAlignedCuboids" : "coplanarPrisms"},
      {"brushCount", static_cast<qint64>(candidate.brushCount())},
      {"reduction", static_cast<qint64>(sourceBrushCount - candidate.brushCount())},
      {"internalFaceArea", candidate.internalFaceArea},
    });
  }
  return result;
}

QJsonArray nodePathToJson(const mdl::NodePath& path)
{
  auto result = QJsonArray{};
  for (const auto index : path.indices)
  {
    result.push_back(static_cast<qint64>(index));
  }
  return result;
}

std::optional<std::vector<mdl::EntityProperty>> entityPropertiesFromJson(
  const QJsonValue& value)
{
  if (!value.isUndefined() && !value.isObject())
  {
    return std::nullopt;
  }

  auto result = std::vector<mdl::EntityProperty>{};
  const auto properties = value.toObject();
  result.reserve(static_cast<size_t>(properties.size()));
  for (auto it = properties.begin(); it != properties.end(); ++it)
  {
    if (it.key().isEmpty() || !it.value().isString() || it.key() == "classname")
    {
      return std::nullopt;
    }
    result.emplace_back(it.key().toStdString(), it.value().toString().toStdString());
  }
  return result;
}

/** Keeps editor state unchanged for automation insertions which do not need selection. */
class SelectionAndMaterialState
{
private:
  mdl::Map& m_map;
  std::vector<mdl::Node*> m_nodes;
  std::vector<mdl::BrushFaceHandle> m_brushFaces;
  std::vector<mdl::VertexHandle> m_vertexHandles;
  std::vector<mdl::EdgeHandle> m_edgeHandles;
  std::string m_currentMaterialName;

public:
  explicit SelectionAndMaterialState(mdl::Map& map)
    : m_map{map}
    , m_nodes{map.selection().nodes}
    , m_brushFaces{map.selection().brushFaces}
    , m_currentMaterialName{map.currentMaterialName()}
  {
    for (const auto& vertex : map.nodeHandles().selectedHandles<mdl::VertexHandle>())
    {
      m_vertexHandles.push_back(vertex);
    }
    for (const auto& edge : map.nodeHandles().selectedHandles<mdl::EdgeHandle>())
    {
      m_edgeHandles.push_back(edge);
    }
  }

  ~SelectionAndMaterialState()
  {
    mdl::deselectAll(m_map);
    if (!m_nodes.empty())
    {
      mdl::selectNodes(m_map, m_nodes);
    }
    if (!m_brushFaces.empty())
    {
      mdl::selectBrushFaces(m_map, m_brushFaces);
    }
    auto& handles = m_map.nodeHandles();
    handles.deselectAllHandles<mdl::VertexHandle>();
    handles.selectHandles<mdl::VertexHandle>(m_vertexHandles);
    handles.deselectAllHandles<mdl::EdgeHandle>();
    handles.selectHandles<mdl::EdgeHandle>(m_edgeHandles);
    m_map.setCurrentMaterialName(m_currentMaterialName);
  }
};

} // namespace

JsonRpcResponse AutomationService::handleNodeRequest(
  const QString& method, const QJsonObject& params)
{
  if (method == "document.exportSelection")
  {
    auto* window = findWindow(params);
    if (window == nullptr)
    {
      return automation::invalidParams("Unknown documentId or no map document is open");
    }
    auto& map = window->document().map();
    return JsonRpcResponse::success(QJsonObject{
      {"mapText", QString::fromStdString(mdl::serializeSelectedNodes(map))},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
  }

  if (method == "nodes.query")
  {
    auto* window = findWindow(params);
    if (window == nullptr)
    {
      return automation::invalidParams("Unknown documentId or no map document is open");
    }
    auto& map = window->document().map();
    auto query = automation::NodeQuery{
      .pattern = params.value("pattern").toString(),
      .limit = std::clamp(params.value("limit").toInt(200), 1, 5000),
      .aggregate = params.value("aggregate").toBool(false),
    };
    for (const auto& [name, destination] : {
           std::pair{"types", &query.types},
           std::pair{"materials", &query.materials},
           std::pair{"classnames", &query.classnames},
         })
    {
      if (params.contains(name))
      {
        const auto values = stringListFromJson(params.value(name));
        if (!values)
        {
          return automation::invalidParams(
            QString{"%1 must be an array of non-empty strings"}.arg(name));
        }
        *destination = *values;
      }
    }
    if (params.contains("projection"))
    {
      const auto projection = params.value("projection").toString();
      if (
        !params.value("projection").isString()
        || (projection != "full" && projection != "paths"))
      {
        return automation::invalidParams("projection must be 'full' or 'paths'");
      }
      query.pathsOnly = projection == "paths";
    }
    if (params.contains("offset"))
    {
      if (!params.value("offset").isDouble() || params.value("offset").toDouble() < 0.0)
      {
        return automation::invalidParams("offset must be a non-negative number");
      }
      query.offset = params.value("offset").toInt();
    }
    if (params.contains("bounds"))
    {
      const auto bounds = params.value("bounds").toObject();
      const auto min = automation::vec3FromJson(bounds.value("min"));
      const auto max = automation::vec3FromJson(bounds.value("max"));
      if (
        !params.value("bounds").isObject() || !min || !max
        || !vm::bbox3d::is_valid(*min, *max))
      {
        return automation::invalidParams("bounds must contain valid min and max vectors");
      }
      query.bounds = vm::bbox3d{*min, *max};
    }
    if (params.contains("ancestorPath"))
    {
      query.ancestorPath = automation::nodePathFromJson(params.value("ancestorPath"));
      if (
        !query.ancestorPath
        || map.worldNode().resolvePath(*query.ancestorPath) == nullptr)
      {
        return automation::invalidParams("ancestorPath must identify an existing node");
      }
    }

    const auto result = automation::queryNodes(map.worldNode(), query);
    auto response = QJsonObject{
      {"total", result.total},
      {"truncated", result.truncated},
      {"revision", static_cast<qint64>(map.modificationCount())},
    };
    if (query.aggregate)
    {
      response.insert("aggregate", result.aggregate);
    }
    else
    {
      response.insert(query.pathsOnly ? "paths" : "nodes", result.nodes);
      response.insert("offset", query.offset);
      if (result.nextOffset)
      {
        response.insert("nextOffset", *result.nextOffset);
      }
    }
    return JsonRpcResponse::success(response);
  }

  if (method == "nodes.describe")
  {
    auto* window = findWindow(params);
    if (
      window == nullptr || params.value("detail").toString() != "brushFaces"
      || !params.value("paths").isArray())
    {
      return automation::invalidParams(
        "documentId, paths, and detail: brushFaces are required");
    }

    auto& map = window->document().map();
    auto brushes = std::vector<mdl::BrushNode*>{};
    for (const auto& value : params.value("paths").toArray())
    {
      const auto path = automation::nodePathFromJson(value);
      auto* node = path ? map.worldNode().resolvePath(*path) : nullptr;
      auto* brush = dynamic_cast<mdl::BrushNode*>(node);
      if (brush == nullptr || std::ranges::find(brushes, brush) != brushes.end())
      {
        return automation::invalidParams(
          "paths must identify distinct brush nodes for detail: brushFaces");
      }
      brushes.push_back(brush);
    }

    auto result = QJsonArray{};
    for (const auto* brush : brushes)
    {
      result.push_back(automation::brushToJson(*brush, map.worldNode()));
    }
    return JsonRpcResponse::success(QJsonObject{
      {"detail", "brushFaces"},
      {"brushes", result},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
  }

  if (method == "nodes.group.create")
  {
    auto* window = findWindow(params);
    if (
      window == nullptr || !params.value("name").isString()
      || params.value("name").toString().isEmpty()
      || !automation::sizeFromJson(params.value("expectedRevision")))
    {
      return automation::invalidParams(
        "documentId, a non-empty name, and expectedRevision are required for a mutation");
    }

    auto& map = window->document().map();
    if (!expectedRevisionMatches(map, params))
    {
      return automation::revisionConflict(map.modificationCount());
    }

    auto* parent = static_cast<mdl::Node*>(map.editorContext().currentLayer());
    if (params.contains("parentPath"))
    {
      const auto parentPath = automation::nodePathFromJson(params.value("parentPath"));
      parent = parentPath ? map.worldNode().resolvePath(*parentPath) : nullptr;
    }
    if (parent == nullptr)
    {
      return automation::invalidParams(
        "parentPath must identify an existing node, or the document must have a current "
        "layer");
    }

    auto prospectiveGroup =
      mdl::GroupNode{mdl::Group{params.value("name").toString().toStdString()}};
    if (!parent->canAddChild(prospectiveGroup))
    {
      return automation::invalidParams(
        "parentPath must identify a node that can contain a group");
    }

    auto* const group =
      mdl::createGroup(map, *parent, params.value("name").toString().toStdString());
    if (group == nullptr)
    {
      return automation::invalidParams("The group could not be created");
    }
    return JsonRpcResponse::success(QJsonObject{
      {"path", nodePathToJson(group->pathFrom(map.worldNode()))},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
  }

  if (method == "nodes.entity.create")
  {
    auto* window = findWindow(params);
    const auto classname = params.value("classname");
    const auto entityType = params.value("entityType");
    if (
      window == nullptr || !classname.isString() || classname.toString().isEmpty()
      || !entityType.isString()
      || !automation::sizeFromJson(params.value("expectedRevision")))
    {
      return automation::invalidParams(
        "documentId, non-empty classname, entityType, and expectedRevision are required "
        "for a mutation");
    }

    const auto properties = entityPropertiesFromJson(params.value("properties"));
    if (!properties)
    {
      return automation::invalidParams(
        "properties must be an object with non-empty string keys and string values, and "
        "must not include classname");
    }

    auto& map = window->document().map();
    if (!expectedRevisionMatches(map, params))
    {
      return automation::revisionConflict(map.modificationCount());
    }

    const auto* definition =
      map.entityDefinitionManager().definition(classname.toString().toStdString());
    if (definition == nullptr)
    {
      return automation::invalidParams(
        "classname must identify a known entity definition");
    }
    const auto requestedType = entityType.toString();
    const auto definitionType = mdl::getType(*definition);
    if (
      (requestedType != "point" && requestedType != "brush")
      || (requestedType == "point" && definitionType != mdl::EntityDefinitionType::Point)
      || (requestedType == "brush" && definitionType != mdl::EntityDefinitionType::Brush))
    {
      return automation::invalidParams(
        "entityType must be point or brush and must match the classname definition");
    }

    auto* parent = static_cast<mdl::Node*>(map.editorContext().currentLayer());
    if (params.contains("parentPath"))
    {
      const auto parentPath = automation::nodePathFromJson(params.value("parentPath"));
      parent = parentPath ? map.worldNode().resolvePath(*parentPath) : nullptr;
    }
    if (parent == nullptr)
    {
      return automation::invalidParams(
        "parentPath must identify an existing node, or the document must have a current "
        "layer");
    }

    auto entityProperties = *properties;
    entityProperties.emplace_back(
      mdl::EntityPropertyKeys::Classname, classname.toString().toStdString());
    auto entity = mdl::Entity{std::move(entityProperties)};
    entity.setPointEntity(definitionType == mdl::EntityDefinitionType::Point);
    auto prospectiveEntity = mdl::EntityNode{entity};
    if (!parent->canAddChild(prospectiveEntity))
    {
      return automation::invalidParams(
        "parentPath must identify a node that can contain an entity");
    }

    auto state = SelectionAndMaterialState{map};
    auto* const entityNode = mdl::createEmptyEntity(map, *parent, std::move(entity));
    if (entityNode == nullptr)
    {
      return automation::invalidParams("The entity could not be created");
    }
    return JsonRpcResponse::success(QJsonObject{
      {"path", nodePathToJson(entityNode->pathFrom(map.worldNode()))},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
  }

  if (method == "nodes.select" || method == "nodes.delete")
  {
    auto* window = findWindow(params);
    if (window == nullptr || !params.value("paths").isArray())
    {
      return automation::invalidParams(
        "documentId and an array of node paths are required");
    }
    auto& map = window->document().map();
    if (
      method == "nodes.delete"
      && !automation::sizeFromJson(params.value("expectedRevision")))
    {
      return automation::invalidParams("expectedRevision is required for a mutation");
    }
    if (method == "nodes.delete" && !expectedRevisionMatches(map, params))
    {
      return automation::revisionConflict(map.modificationCount());
    }

    auto nodes = std::vector<mdl::Node*>{};
    for (const auto& value : params.value("paths").toArray())
    {
      const auto path = automation::nodePathFromJson(value);
      auto* node = path ? map.worldNode().resolvePath(*path) : nullptr;
      if (node == nullptr || node == &map.worldNode())
      {
        return automation::invalidParams(
          "A node path was invalid or referred to the world root");
      }
      nodes.push_back(node);
    }
    if (method == "nodes.select")
    {
      mdl::deselectAll(map);
      mdl::selectNodes(map, nodes);
    }
    else
    {
      mdl::removeNodes(map, nodes);
    }
    return JsonRpcResponse::success(QJsonObject{
      {"count", static_cast<qint64>(nodes.size())},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
  }

  if (method == "brushes.optimize.preview" || method == "brushes.optimize.apply")
  {
    auto* window = findWindow(params);
    if (window == nullptr)
    {
      return automation::invalidParams("Unknown documentId or no map document is open");
    }
    auto& map = window->document().map();
    const auto brushNodes = resolveBrushPaths(map, params.value("paths"));
    if (!brushNodes)
    {
      return automation::invalidParams(
        "paths must be an array of distinct brush node paths");
    }

    const auto eligible = mdl::canOptimizeBrushes(*brushNodes);
    const auto candidates = eligible
                              ? mdl::createBrushOptimizationCandidates(map, *brushNodes)
                              : std::vector<mdl::BrushOptimizationCandidate>{};
    if (method == "brushes.optimize.preview")
    {
      return JsonRpcResponse::success(QJsonObject{
        {"sourceBrushCount", static_cast<qint64>(brushNodes->size())},
        {"eligible", eligible},
        {"candidates", optimizationCandidatesToJson(candidates, brushNodes->size())},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
    }

    if (
      !params.value("candidateIndex").isDouble()
      || !automation::sizeFromJson(params.value("expectedRevision")))
    {
      return automation::invalidParams(
        "candidateIndex and expectedRevision are required for a mutation");
    }
    if (!expectedRevisionMatches(map, params))
    {
      return automation::revisionConflict(map.modificationCount());
    }
    const auto candidateIndex = params.value("candidateIndex").toInt(-1);
    if (candidateIndex < 0 || static_cast<size_t>(candidateIndex) >= candidates.size())
    {
      return automation::invalidParams("candidateIndex does not identify a candidate");
    }

    const auto& candidate = candidates[static_cast<size_t>(candidateIndex)];
    auto transaction = mdl::Transaction{map, "Optimize Brushwork"};
    if (!mdl::applyBrushOptimizationCandidate(map, *brushNodes, candidate))
    {
      transaction.cancel();
      return automation::invalidParams("The optimization candidate could not be applied");
    }
    if (!transaction.commit())
    {
      return automation::invalidParams(
        "The optimization transaction could not be committed");
    }
    return JsonRpcResponse::success(QJsonObject{
      {"candidateIndex", candidateIndex},
      {"brushCount", static_cast<qint64>(candidate.brushCount())},
      {"reduction", static_cast<qint64>(brushNodes->size() - candidate.brushCount())},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
  }

  if (method == "document.paste")
  {
    auto* window = findWindow(params);
    if (
      window == nullptr || !params.value("mapText").isString()
      || !automation::sizeFromJson(params.value("expectedRevision")))
    {
      return automation::invalidParams(
        "documentId, mapText, and expectedRevision are required for a mutation");
    }
    auto& map = window->document().map();
    if (!expectedRevisionMatches(map, params))
    {
      return automation::revisionConflict(map.modificationCount());
    }
    const auto pasteType =
      mdl::paste(map, params.value("mapText").toString().toStdString());
    if (pasteType == mdl::PasteType::Failed)
    {
      return automation::invalidParams("mapText could not be parsed or inserted");
    }
    return JsonRpcResponse::success(QJsonObject{
      {"pasteType", pasteType == mdl::PasteType::Node ? "node" : "brushFace"},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
  }

  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

} // namespace tb::ui
