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
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/AutomationService.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"

#include <algorithm>
#include <optional>
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

QJsonArray pathToJson(const mdl::NodePath& path)
{
  auto result = QJsonArray{};
  for (const auto index : path.indices)
  {
    result.push_back(static_cast<qint64>(index));
  }
  return result;
}

struct CohortPreview
{
  std::vector<mdl::BrushNode*> brushNodes;
  std::vector<mdl::BrushOptimizationCandidate> candidates;
};

QJsonObject optimizationGuarantees()
{
  return {
    {"exactVolume", true},
    {"visibleSurfaceAttributes",
     QJsonObject{
       {"materials", true},
       {"surfaceFlags", true},
       {"uvAttributes", true},
       {"textureAxes", true},
     }},
  };
}

std::vector<CohortPreview> createCohortPreviews(
  const mdl::Map& map, const std::vector<mdl::BrushNode*>& brushNodes)
{
  auto result = std::vector<CohortPreview>{};
  for (auto& cohort : mdl::findBrushOptimizationCohorts(map, brushNodes))
  {
    auto candidates = mdl::createBrushOptimizationCandidates(map, cohort);
    if (!candidates.empty())
    {
      result.push_back({std::move(cohort), std::move(candidates)});
    }
  }
  return result;
}

QJsonObject cohortToJson(
  const CohortPreview& cohort, const mdl::WorldNode& world, const size_t index)
{
  const auto& best = cohort.candidates.front();
  auto paths = QJsonArray{};
  for (const auto* brushNode : cohort.brushNodes)
  {
    paths.push_back(pathToJson(brushNode->pathFrom(world)));
  }
  return {
    {"index", static_cast<qint64>(index)},
    {"paths", paths},
    {"sourceBrushCount", static_cast<qint64>(cohort.brushNodes.size())},
    {"candidateCount", static_cast<qint64>(cohort.candidates.size())},
    {"bestCandidateIndex", 0},
    {"bestBrushCount", static_cast<qint64>(best.brushCount())},
    {"reduction", static_cast<qint64>(cohort.brushNodes.size() - best.brushCount())},
    {"internalFaceArea", best.internalFaceArea},
    {"kind", best.brushes.empty() ? "axisAlignedCuboids" : "coplanarPrisms"},
    {"guarantees", optimizationGuarantees()},
  };
}

} // namespace

JsonRpcResponse AutomationService::handleBrushRequest(
  const QString& method, const QJsonObject& params)
{
  if (
    method != "brushes.optimize.batch.preview"
    && method != "brushes.optimize.batch.apply")
  {
    return handleNodeRequest(method, params);
  }

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

  auto cohorts = createCohortPreviews(map, *brushNodes);
  if (method == "brushes.optimize.batch.preview")
  {
    auto cohortsJson = QJsonArray{};
    auto optimizedBrushCount = size_t{0u};
    auto sourceBrushCount = size_t{0u};
    for (size_t index = 0u; index < cohorts.size(); ++index)
    {
      cohortsJson.push_back(cohortToJson(cohorts[index], map.worldNode(), index));
      sourceBrushCount += cohorts[index].brushNodes.size();
      optimizedBrushCount += cohorts[index].candidates.front().brushCount();
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"inputBrushCount", static_cast<qint64>(brushNodes->size())},
        {"cohortCount", static_cast<qint64>(cohorts.size())},
        {"optimizableBrushCount", static_cast<qint64>(sourceBrushCount)},
        {"optimizedBrushCount", static_cast<qint64>(optimizedBrushCount)},
        {"reduction", static_cast<qint64>(sourceBrushCount - optimizedBrushCount)},
        {"guarantees", optimizationGuarantees()},
        {"cohorts", cohortsJson},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (!automation::sizeFromJson(params.value("expectedRevision")))
  {
    return automation::invalidParams("expectedRevision is required for a mutation");
  }
  if (!expectedRevisionMatches(map, params))
  {
    return automation::revisionConflict(map.modificationCount());
  }
  if (cohorts.empty())
  {
    return automation::invalidParams("No optimizable brush cohorts were found");
  }

  auto sourceBrushCount = size_t{0u};
  auto optimizedBrushCount = size_t{0u};
  auto transaction = mdl::Transaction{map, "Optimize Brushwork Batch"};
  for (const auto& cohort : cohorts)
  {
    const auto& candidate = cohort.candidates.front();
    sourceBrushCount += cohort.brushNodes.size();
    optimizedBrushCount += candidate.brushCount();
    if (!mdl::applyBrushOptimizationCandidate(map, cohort.brushNodes, candidate))
    {
      transaction.cancel();
      return automation::invalidParams(
        "A brush optimization cohort could not be applied");
    }
  }
  if (!transaction.commit())
  {
    return automation::invalidParams(
      "The batch optimization transaction could not be committed");
  }
  return JsonRpcResponse::success(
    QJsonObject{
      {"cohortCount", static_cast<qint64>(cohorts.size())},
      {"sourceBrushCount", static_cast<qint64>(sourceBrushCount)},
      {"brushCount", static_cast<qint64>(optimizedBrushCount)},
      {"reduction", static_cast<qint64>(sourceBrushCount - optimizedBrushCount)},
      {"guarantees", optimizationGuarantees()},
      {"revision", static_cast<qint64>(map.modificationCount())},
    });
}

} // namespace tb::ui
