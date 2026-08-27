/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "AutomationJson.h"
#include "mdl/ApplyAndSwap.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/MapFormat.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "mdl/WorldNode.h"
#include "ui/AutomationService.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"

#include <algorithm>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace tb::ui
{
namespace
{

struct FaceReference
{
  mdl::BrushFaceHandle handle;
};

struct FaceAttributeMapping
{
  FaceReference source;
  FaceReference target;
};

std::optional<FaceReference> resolveFaceReference(mdl::Map& map, const QJsonValue& value)
{
  if (!value.isObject())
  {
    return std::nullopt;
  }

  const auto object = value.toObject();
  const auto path = automation::nodePathFromJson(object.value("path"));
  const auto faceIndex = automation::sizeFromJson(object.value("faceIndex"));
  auto* const node = path ? map.worldNode().resolvePath(*path) : nullptr;
  auto* const brush = dynamic_cast<mdl::BrushNode*>(node);
  if (brush == nullptr || !faceIndex || *faceIndex >= brush->brush().faceCount())
  {
    return std::nullopt;
  }

  return FaceReference{mdl::BrushFaceHandle{brush, *faceIndex}};
}

std::optional<std::vector<FaceAttributeMapping>> resolveFaceAttributeMappings(
  mdl::Map& sourceMap, mdl::Map& targetMap, const QJsonValue& mappingsValue)
{
  if (!mappingsValue.isArray() || mappingsValue.toArray().empty())
  {
    return std::nullopt;
  }

  auto mappings = std::vector<FaceAttributeMapping>{};
  for (const auto& value : mappingsValue.toArray())
  {
    if (!value.isObject())
    {
      return std::nullopt;
    }

    const auto mapping = value.toObject();
    const auto source = resolveFaceReference(sourceMap, mapping.value("source"));
    const auto target = resolveFaceReference(targetMap, mapping.value("target"));
    if (!source || !target)
    {
      return std::nullopt;
    }

    const auto duplicateTarget = std::ranges::any_of(mappings, [&](const auto& existing) {
      return existing.target.handle.node() == target->handle.node()
             && existing.target.handle.faceIndex() == target->handle.faceIndex();
    });
    if (duplicateTarget)
    {
      return std::nullopt;
    }

    mappings.push_back({*source, *target});
  }
  return mappings;
}

/**
 * applyAndSwap intentionally leaves the selection alone, but swapping node contents
 * invalidates it. Preserve every selection kind and restore the current material last,
 * because selecting faces may otherwise replace it with the face material.
 */
class FaceAttributeCopyOperationState
{
private:
  mdl::Map& m_map;
  std::vector<mdl::Node*> m_nodes;
  std::vector<mdl::BrushFaceHandle> m_brushFaces;
  std::vector<mdl::EdgeHandle> m_edgeHandles;
  std::string m_currentMaterialName;

public:
  explicit FaceAttributeCopyOperationState(mdl::Map& map)
    : m_map{map}
    , m_nodes{map.selection().nodes}
    , m_brushFaces{map.selection().brushFaces}
    , m_currentMaterialName{map.currentMaterialName()}
  {
    for (const auto& edge : map.nodeHandles().selectedHandles<mdl::EdgeHandle>())
    {
      m_edgeHandles.push_back(edge);
    }
  }

  ~FaceAttributeCopyOperationState()
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
    handles.deselectAllHandles<mdl::EdgeHandle>();
    handles.selectHandles<mdl::EdgeHandle>(m_edgeHandles);
    m_map.setCurrentMaterialName(m_currentMaterialName);
  }
};

bool expectedRevisionMatches(const mdl::Map& map, const QJsonValue& value)
{
  const auto expectedRevision = automation::sizeFromJson(value);
  return expectedRevision && *expectedRevision == map.modificationCount();
}

struct FaceAttributeSource
{
  mdl::BrushFace face;
  std::optional<mdl::UvCoordSystemSnapshot> uvSnapshot;
};

FaceAttributeSource snapshotFaceAttributes(const mdl::BrushFace& face)
{
  // Paraxial UV axes are resolved from the face plane, not stored. Only transfer a
  // snapshot when the source face actually owns transferable axes; otherwise a
  // Standard-to-Valve copy would incorrectly turn its resolved axes into Valve axes.
  return {face, face.takeUvCoordSystemSnapshot()};
}

void copyFaceAttributes(
  const FaceAttributeSource& source, mdl::BrushFace& target, const bool copyUvAxes)
{
  target.copyAttributes(source.face);
  if (copyUvAxes && source.uvSnapshot)
  {
    target.restoreUvCoordSystemSnapshot(*source.uvSnapshot);
  }
}

} // namespace

JsonRpcResponse AutomationService::handleFaceRequest(
  const QString& method, const QJsonObject& params)
{
  if (method != "faces.copyAttributes")
  {
    return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
  }

  const auto targetDocumentId = params.value("documentId");
  const auto sourceDocumentId = params.value("sourceDocumentId");
  if (
    !targetDocumentId.isString() || targetDocumentId.toString().isEmpty()
    || !sourceDocumentId.isString() || sourceDocumentId.toString().isEmpty()
    || !automation::sizeFromJson(params.value("expectedRevision"))
    || !automation::sizeFromJson(params.value("sourceRevision")))
  {
    return automation::invalidParams(
      "documentId, expectedRevision, sourceDocumentId, and sourceRevision are required");
  }

  auto* const targetWindow = findWindow(QJsonObject{{"documentId", targetDocumentId}});
  auto* const sourceWindow = findWindow(QJsonObject{{"documentId", sourceDocumentId}});
  if (targetWindow == nullptr || sourceWindow == nullptr)
  {
    return automation::invalidParams("Unknown documentId or no map document is open");
  }

  auto& targetMap = targetWindow->document().map();
  auto& sourceMap = sourceWindow->document().map();
  if (!expectedRevisionMatches(targetMap, params.value("expectedRevision")))
  {
    return automation::revisionConflict(targetMap.modificationCount());
  }
  if (!expectedRevisionMatches(sourceMap, params.value("sourceRevision")))
  {
    return automation::revisionConflict(sourceMap.modificationCount());
  }

  const auto mappings =
    resolveFaceAttributeMappings(sourceMap, targetMap, params.value("mappings"));
  if (!mappings)
  {
    return automation::invalidParams(
      "mappings must be a non-empty array of valid source and distinct target faces");
  }

  auto sources = std::vector<FaceAttributeSource>{};
  auto targets = std::vector<mdl::BrushFaceHandle>{};
  sources.reserve(mappings->size());
  targets.reserve(mappings->size());
  for (const auto& mapping : *mappings)
  {
    sources.push_back(snapshotFaceAttributes(mapping.source.handle.face()));
    targets.push_back(mapping.target.handle);
  }

  auto state = FaceAttributeCopyOperationState{targetMap};
  const auto applied = mdl::applyAndSwap(
    targetMap,
    "Copy Face Attributes",
    targets,
    [index = size_t{0u},
     copyUvAxes = mdl::isParallelUvCoordSystem(targetMap.worldNode().mapFormat()),
     &sources](auto& face) mutable {
      copyFaceAttributes(sources[index++], face, copyUvAxes);
      return true;
    });
  if (!applied)
  {
    return automation::invalidParams(
      "The face attribute copy transaction could not be committed");
  }

  return JsonRpcResponse::success(
    QJsonObject{
      {"mappingCount", static_cast<qint64>(mappings->size())},
      {"revision", static_cast<qint64>(targetMap.modificationCount())},
      {"sourceRevision", static_cast<qint64>(sourceMap.modificationCount())},
    });
}

} // namespace tb::ui
