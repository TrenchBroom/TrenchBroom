/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "AutomationJson.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Selection.h"
#include "mdl/Node.h"
#include "mdl/NodeHandles.h"
#include "mdl/WorldNode.h"
#include "ui/AutomationService.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"

#include <cmath>
#include <limits>
#include <optional>
#include <string>
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

struct ResolvedFootprintBrushes
{
  std::vector<const mdl::BrushNode*> brushes;
  size_t duplicatePathCount = 0u;
};

std::optional<ResolvedFootprintBrushes> resolveFootprintBrushPaths(
  const mdl::Map& map, const QJsonValue& pathsValue)
{
  if (!pathsValue.isArray() || pathsValue.toArray().empty())
  {
    return std::nullopt;
  }

  auto result = ResolvedFootprintBrushes{};
  for (const auto& value : pathsValue.toArray())
  {
    const auto path = automation::nodePathFromJson(value);
    const auto* node = path ? map.worldNode().resolvePath(*path) : nullptr;
    const auto* brush = dynamic_cast<const mdl::BrushNode*>(node);
    if (brush == nullptr)
    {
      return std::nullopt;
    }
    if (std::ranges::find(result.brushes, brush) != result.brushes.end())
    {
      ++result.duplicatePathCount;
    }
    else
    {
      result.brushes.push_back(brush);
    }
  }
  return result;
}

std::optional<std::vector<mdl::BrushFaceHandle>> resolveFaces(
  mdl::Map& map, const QJsonValue& facesValue)
{
  if (!facesValue.isArray())
  {
    return std::nullopt;
  }

  auto faces = std::vector<mdl::BrushFaceHandle>{};
  for (const auto& value : facesValue.toArray())
  {
    if (!value.isObject())
    {
      return std::nullopt;
    }
    const auto face = value.toObject();
    const auto path = automation::nodePathFromJson(face.value("path"));
    auto* node = path ? map.worldNode().resolvePath(*path) : nullptr;
    auto* brush = dynamic_cast<mdl::BrushNode*>(node);
    const auto indexValue = face.value("faceIndex");
    const auto index = automation::sizeFromJson(indexValue);
    if (brush == nullptr || !index || *index >= brush->brush().faceCount())
    {
      return std::nullopt;
    }
    const auto duplicate = std::ranges::any_of(faces, [&](const auto& existing) {
      return existing.node() == brush && existing.faceIndex() == *index;
    });
    if (duplicate)
    {
      return std::nullopt;
    }
    faces.emplace_back(brush, *index);
  }
  return faces;
}

std::vector<mdl::Node*> nodesFromBrushes(const std::vector<mdl::BrushNode*>& brushes)
{
  auto nodes = std::vector<mdl::Node*>{};
  nodes.reserve(brushes.size());
  for (auto* brush : brushes)
  {
    nodes.push_back(brush);
  }
  return nodes;
}

template <typename HandleType>
std::vector<HandleType> selectedHandles(const mdl::NodeHandleManager& handles)
{
  auto result = std::vector<HandleType>{};
  for (const auto& handle : handles.selectedHandles<HandleType>())
  {
    result.push_back(handle);
  }
  return result;
}

/**
 * Geometry operations temporarily drive the editor selection because the underlying map
 * commands use it as their input. Keep a snapshot until the command succeeds so an
 * invalid RPC is observationally a no-op for the user.
 */
class GeometryOperationState
{
private:
  mdl::Map& m_map;
  std::vector<mdl::Node*> m_nodes;
  std::vector<mdl::BrushFaceHandle> m_brushFaces;
  std::vector<mdl::EdgeHandle> m_edgeHandles;
  std::string m_currentMaterialName;
  bool m_restore = true;

public:
  explicit GeometryOperationState(mdl::Map& map)
    : m_map{map}
    , m_nodes{map.selection().nodes}
    , m_brushFaces{map.selection().brushFaces}
    , m_edgeHandles{selectedHandles<mdl::EdgeHandle>(map.nodeHandles())}
    , m_currentMaterialName{map.currentMaterialName()}
  {
  }

  ~GeometryOperationState()
  {
    if (!m_restore)
    {
      return;
    }

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
    // Face selection updates the current material, so restore it last.
    m_map.setCurrentMaterialName(m_currentMaterialName);
  }

  void keepChanges() { m_restore = false; }
};

bool samePosition(const vm::vec3d& lhs, const vm::vec3d& rhs)
{
  constexpr auto epsilon = 0.000001;
  return vm::squared_distance(lhs, rhs) <= epsilon * epsilon;
}

std::optional<std::vector<mdl::EdgeHandle>> resolveEdges(
  mdl::Map& map, const QJsonValue& edgesValue, std::vector<mdl::BrushNode*>& brushNodes)
{
  if (!edgesValue.isArray())
  {
    return std::nullopt;
  }

  auto edges = std::vector<mdl::EdgeHandle>{};
  for (const auto& value : edgesValue.toArray())
  {
    if (!value.isObject())
    {
      return std::nullopt;
    }
    const auto edge = value.toObject();
    const auto path = automation::nodePathFromJson(edge.value("path"));
    auto* node = path ? map.worldNode().resolvePath(*path) : nullptr;
    auto* brush = dynamic_cast<mdl::BrushNode*>(node);
    const auto start = automation::vec3FromJson(edge.value("start"));
    const auto end = automation::vec3FromJson(edge.value("end"));
    if (brush == nullptr || !start || !end || samePosition(*start, *end))
    {
      return std::nullopt;
    }

    const auto brushEdges = mdl::EdgeHandle::getHandles(*brush);
    const auto matching = std::ranges::find_if(brushEdges, [&](const auto& handle) {
      const auto& position = handle.position;
      return (samePosition(position.start(), *start)
              && samePosition(position.end(), *end))
             || (samePosition(position.start(), *end) && samePosition(position.end(), *start));
    });
    if (matching == brushEdges.end())
    {
      return std::nullopt;
    }

    const auto duplicate = std::ranges::any_of(edges, [&](const auto& existing) {
      return (samePosition(existing.position.start(), matching->position.start())
              && samePosition(existing.position.end(), matching->position.end()))
             || (samePosition(existing.position.start(), matching->position.end())
                 && samePosition(existing.position.end(), matching->position.start()));
    });
    if (duplicate)
    {
      return std::nullopt;
    }
    edges.push_back(*matching);
    if (std::ranges::find(brushNodes, brush) == brushNodes.end())
    {
      brushNodes.push_back(brush);
    }
  }
  return edges;
}

std::optional<vm::axis::type> axisFromJson(const QJsonValue& value)
{
  const auto axis = value.toString().toLower();
  if (axis == "x")
    return vm::axis::x;
  if (axis == "y")
    return vm::axis::y;
  if (axis == "z")
    return vm::axis::z;
  return std::nullopt;
}

std::optional<mdl::BridgeSurfaceDirection> bridgeDirectionFromJson(
  const QJsonValue& value)
{
  const auto direction = value.toString().toLower();
  if (direction == "below")
    return mdl::BridgeSurfaceDirection::Below;
  if (direction == "above")
    return mdl::BridgeSurfaceDirection::Above;
  if (direction == "centered")
    return mdl::BridgeSurfaceDirection::Centered;
  return std::nullopt;
}

std::optional<std::string> requiredString(const QJsonObject& params, const QString& name)
{
  const auto value = params.value(name);
  if (!value.isString() || value.toString().isEmpty())
  {
    return std::nullopt;
  }
  return value.toString().toStdString();
}

QJsonObject mutationResult(const mdl::Map& map, const QString& operation)
{
  return {
    {"operation", operation},
    {"selectedBrushCount", static_cast<qint64>(map.selection().brushes.size())},
    {"revision", static_cast<qint64>(map.modificationCount())},
  };
}

std::optional<std::vector<mdl::BrushCreationSpec>> resolveBrushCreationSpecs(
  const QJsonValue& brushesValue)
{
  if (!brushesValue.isArray() || brushesValue.toArray().empty())
  {
    return std::nullopt;
  }

  auto specs = std::vector<mdl::BrushCreationSpec>{};
  for (const auto& value : brushesValue.toArray())
  {
    if (!value.isObject())
    {
      return std::nullopt;
    }
    const auto brush = value.toObject();
    const auto material = requiredString(brush, "material");
    if (!material || !brush.value("points").isArray())
    {
      return std::nullopt;
    }
    auto points = std::vector<vm::vec3d>{};
    for (const auto& pointValue : brush.value("points").toArray())
    {
      const auto point = automation::vec3FromJson(pointValue);
      if (!point)
      {
        return std::nullopt;
      }
      points.push_back(*point);
    }
    if (points.empty())
    {
      return std::nullopt;
    }
    specs.push_back({std::move(points), *material});
  }
  return specs;
}

QJsonArray createdBrushesToJson(
  const std::vector<mdl::BrushNode*>& brushes, const mdl::WorldNode& world)
{
  auto result = QJsonArray{};
  for (const auto* brush : brushes)
  {
    const auto bounds = brush->logicalBounds();
    auto path = QJsonArray{};
    for (const auto index : brush->pathFrom(world).indices)
    {
      path.push_back(static_cast<qint64>(index));
    }
    result.push_back(
      QJsonObject{
        {"path", path},
        {"bounds",
         QJsonObject{
           {"min", QJsonArray{bounds.min.x(), bounds.min.y(), bounds.min.z()}},
           {"max", QJsonArray{bounds.max.x(), bounds.max.y(), bounds.max.z()}}}},
      });
  }
  return result;
}

std::optional<vm::vec2d> planarPointFromJson(
  const QJsonValue& value, const double gridSize)
{
  const auto array = value.toArray();
  if (
    array.size() != 2 || !array[0].isDouble() || !array[1].isDouble()
    || !std::isfinite(array[0].toDouble()) || !std::isfinite(array[1].toDouble()))
  {
    return std::nullopt;
  }
  return vm::vec2d{
    std::round(array[0].toDouble() / gridSize) * gridSize,
    std::round(array[1].toDouble() / gridSize) * gridSize};
}

std::optional<std::vector<std::vector<vm::vec2d>>> planarPathsFromJson(
  const QJsonObject& params, const double gridSize)
{
  const auto pathsValue = params.value("paths");
  const auto segmentsValue = params.value("segments");
  if (pathsValue.isArray() == segmentsValue.isArray())
  {
    return std::nullopt;
  }
  const auto input =
    pathsValue.isArray() ? pathsValue.toArray() : segmentsValue.toArray();
  auto result = std::vector<std::vector<vm::vec2d>>{};
  for (const auto& pathValue : input)
  {
    if (!pathValue.isArray() || pathValue.toArray().size() < 2)
    {
      return std::nullopt;
    }
    auto path = std::vector<vm::vec2d>{};
    for (const auto& pointValue : pathValue.toArray())
    {
      const auto point = planarPointFromJson(pointValue, gridSize);
      if (!point)
      {
        return std::nullopt;
      }
      path.push_back(*point);
    }
    result.push_back(std::move(path));
  }
  return result.empty()
           ? std::nullopt
           : std::optional<std::vector<std::vector<vm::vec2d>>>{std::move(result)};
}

std::optional<mdl::PlanarPathSweepSpec> planarPathSweepSpecFromJson(
  const QJsonObject& params)
{
  const auto profile = params.value("profile").toObject();
  const auto field = [&](const QString& name) {
    return profile.contains(name) ? profile.value(name) : params.value(name);
  };
  const auto width = field("width");
  const auto bottom = field("bottom");
  const auto top = field("top");
  const auto height = field("height");
  const auto material = requiredString(params, "material");
  const auto gridSize =
    params.contains("gridSize") ? params.value("gridSize").toDouble() : 1.0;
  const auto join = params.contains("join") ? params.value("join").toString().toLower()
                                            : QString{"miter"};
  const auto cap =
    params.contains("cap") ? params.value("cap").toString().toLower() : QString{"butt"};
  if (
    !width.isDouble() || !bottom.isDouble() || !material
    || !std::isfinite(width.toDouble()) || !std::isfinite(bottom.toDouble())
    || !std::isfinite(gridSize) || gridSize <= 0.0
    || (top.isDouble() == height.isDouble()) || join != "miter" || cap != "butt")
  {
    return std::nullopt;
  }
  const auto resolvedTop =
    top.isDouble() ? top.toDouble() : bottom.toDouble() + height.toDouble();
  if (
    !std::isfinite(resolvedTop)
    || (height.isDouble() && !std::isfinite(height.toDouble()))
    || resolvedTop <= bottom.toDouble() || width.toDouble() <= 0.0)
  {
    return std::nullopt;
  }
  const auto paths = planarPathsFromJson(params, gridSize);
  if (!paths)
  {
    return std::nullopt;
  }
  return mdl::PlanarPathSweepSpec{
    .chains = *paths,
    .bottom = bottom.toDouble(),
    .top = resolvedTop,
    .thickness = width.toDouble(),
    .materialName = *material,
  };
}

QJsonObject planarSweepSpecToJson(const mdl::BrushCreationSpec& spec)
{
  auto min = vm::vec3d{
    std::numeric_limits<double>::max(),
    std::numeric_limits<double>::max(),
    std::numeric_limits<double>::max()};
  auto max = vm::vec3d{
    std::numeric_limits<double>::lowest(),
    std::numeric_limits<double>::lowest(),
    std::numeric_limits<double>::lowest()};
  auto points = QJsonArray{};
  for (const auto& point : spec.points)
  {
    min = vm::vec3d{
      std::min(min.x(), point.x()),
      std::min(min.y(), point.y()),
      std::min(min.z(), point.z())};
    max = vm::vec3d{
      std::max(max.x(), point.x()),
      std::max(max.y(), point.y()),
      std::max(max.z(), point.z())};
    points.push_back(QJsonArray{point.x(), point.y(), point.z()});
  }
  return {
    {"material", QString::fromStdString(spec.materialName)},
    {"points", points},
    {"bounds",
     QJsonObject{
       {"min", QJsonArray{min.x(), min.y(), min.z()}},
       {"max", QJsonArray{max.x(), max.y(), max.z()}}}},
  };
}

QJsonObject planarProfileSpecToJson(const mdl::PlanarProfileBrushSpec& spec)
{
  auto result = planarSweepSpecToJson(spec.brush);
  result.insert("role", QString::fromStdString(spec.role));
  return result;
}

QJsonArray createdPlanarProfileBrushesToJson(
  const std::vector<mdl::BrushNode*>& brushes,
  const std::vector<mdl::PlanarProfileBrushSpec>& specs,
  const mdl::WorldNode& world)
{
  auto result = createdBrushesToJson(brushes, world);
  for (auto i = 0; i < result.size(); ++i)
  {
    auto brush = result[i].toObject();
    brush.insert("role", QString::fromStdString(specs[static_cast<size_t>(i)].role));
    result[i] = brush;
  }
  return result;
}

} // namespace

JsonRpcResponse AutomationService::handleGeometryRequest(
  const QString& method, const QJsonObject& params)
{
  auto* window = findWindow(params);
  if (window == nullptr)
  {
    return automation::invalidParams("Unknown documentId or no map document is open");
  }
  auto& map = window->document().map();

  if (method == "geometry.sweepPath.preview")
  {
    const auto sweep = planarPathSweepSpecFromJson(params);
    const auto specs = sweep ? mdl::createPlanarPathSweepBrushSpecs(*sweep)
                             : std::optional<std::vector<mdl::BrushCreationSpec>>{};
    if (!specs)
    {
      return automation::invalidParams(
        "paths (or segments) of [x,y] points, a rectangular profile with width and "
        "bottom plus top or height, and material are required. Paths must avoid "
        "reversing or excessively acute turns.");
    }
    auto brushes = QJsonArray{};
    for (const auto& spec : *specs)
    {
      brushes.push_back(planarSweepSpecToJson(spec));
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"brushCount", static_cast<qint64>(specs->size())},
        {"brushes", brushes},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (method == "geometry.planarProfile.preview")
  {
    const auto profile = automation::planarProfileSpecFromJson(params);
    const auto specs = profile
                         ? mdl::createPlanarProfileBrushSpecs(*profile)
                         : std::optional<std::vector<mdl::PlanarProfileBrushSpec>>{};
    if (!specs)
    {
      return automation::invalidParams(
        "contour must be a closed simple [x,y] polygon with valid grid-snapped mitered "
        "insets; every band/core needs bottom, top, and a material (or top-level "
        "material)");
    }
    auto brushes = QJsonArray{};
    for (const auto& spec : *specs)
    {
      brushes.push_back(planarProfileSpecToJson(spec));
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"axis", "z"},
        {"brushCount", static_cast<qint64>(specs->size())},
        {"brushes", brushes},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (method == "geometry.extrudeProfile.preview")
  {
    const auto extrusion = automation::profileExtrusionSpecFromJson(params);
    const auto specs = extrusion
                         ? mdl::createProfileExtrusionBrushSpecs(*extrusion)
                         : std::optional<std::vector<mdl::PlanarProfileBrushSpec>>{};
    if (!specs)
    {
      return automation::invalidParams(
        "plane (xy, xz, or yz), a closed simple profile of [u,v] points, a positive "
        "interval, and material are required. Concave profiles are triangulated into "
        "convex prisms.");
    }
    auto brushes = QJsonArray{};
    for (const auto& spec : *specs)
    {
      brushes.push_back(planarProfileSpecToJson(spec));
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"plane", params.value("plane").toString().toLower()},
        {"brushCount", static_cast<qint64>(specs->size())},
        {"brushes", brushes},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (method == "geometry.extractFootprints")
  {
    const auto brushes = resolveFootprintBrushPaths(map, params.value("paths"));
    const auto axis = axisFromJson(params.value("axis"));
    const auto coordinate = params.value("coordinate");
    if (!brushes || !axis || !coordinate.isDouble())
    {
      return automation::invalidParams(
        "paths, axis, and coordinate are required for footprint extraction");
    }
    if (params.contains("material") && !requiredString(params, "material"))
    {
      return automation::invalidParams(
        "material must be a non-empty string when provided");
    }

    const auto material = params.contains("material") ? requiredString(params, "material")
                                                      : std::optional<std::string>{};
    const auto footprints = automation::extractFootprints(
      map.worldNode(),
      brushes->brushes,
      automation::FootprintFaceSelector{*axis, coordinate.toDouble(), material});
    auto bounds = vm::bbox3d::builder{};
    auto sourceFaceCount = 0u;
    for (const auto& footprint : footprints)
    {
      bounds.add(footprint.bounds);
      sourceFaceCount += footprint.sources.size();
    }

    auto result = QJsonObject{
      {"axis", params.value("axis").toString().toLower()},
      {"coordinate", coordinate.toDouble()},
      {"footprints", automation::footprintsToJson(footprints)},
      {"footprintCount", static_cast<qint64>(footprints.size())},
      {"sourceFaceCount", static_cast<qint64>(sourceFaceCount)},
      {"duplicatePathCount", static_cast<qint64>(brushes->duplicatePathCount)},
      {"duplicateFaceCount", static_cast<qint64>(sourceFaceCount - footprints.size())},
      {"revision", static_cast<qint64>(map.modificationCount())},
    };
    if (material)
    {
      result.insert("material", QString::fromStdString(*material));
    }
    if (bounds.initialized())
    {
      result.insert(
        "bounds",
        QJsonObject{
          {"min",
           QJsonArray{
             bounds.bounds().min.x(), bounds.bounds().min.y(), bounds.bounds().min.z()}},
          {"max",
           QJsonArray{
             bounds.bounds().max.x(),
             bounds.bounds().max.y(),
             bounds.bounds().max.z()}}});
    }
    return JsonRpcResponse::success(result);
  }

  if (!automation::sizeFromJson(params.value("expectedRevision")))
  {
    return automation::invalidParams(
      "A map document and expectedRevision are required for a geometry mutation");
  }
  if (!expectedRevisionMatches(map, params))
  {
    return automation::revisionConflict(map.modificationCount());
  }

  if (method == "geometry.sweepPath.apply")
  {
    const auto sweep = planarPathSweepSpecFromJson(params);
    const auto specs = sweep ? mdl::createPlanarPathSweepBrushSpecs(*sweep)
                             : std::optional<std::vector<mdl::BrushCreationSpec>>{};
    if (!specs)
    {
      return automation::invalidParams(
        "paths (or segments) of [x,y] points, a rectangular profile with width and "
        "bottom plus top or height, and material are required. Paths must avoid "
        "reversing or excessively acute turns.");
    }
    auto* parent = static_cast<mdl::Node*>(map.editorContext().currentLayer());
    if (params.contains("parentPath"))
    {
      const auto parentPath = automation::nodePathFromJson(params.value("parentPath"));
      parent = parentPath ? map.worldNode().resolvePath(*parentPath) : nullptr;
    }
    if (parent == nullptr)
    {
      return automation::invalidParams("parentPath must identify an existing node");
    }
    auto created = std::vector<mdl::BrushNode*>{};
    if (!mdl::createBrushes(map, *parent, *specs, created))
    {
      return automation::invalidParams(
        "The generated sweep brushes are outside map bounds or parentPath cannot accept "
        "brushes");
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"operation", "sweepPath"},
        {"count", static_cast<qint64>(created.size())},
        {"brushes", createdBrushesToJson(created, map.worldNode())},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (method == "geometry.planarProfile.apply")
  {
    const auto profile = automation::planarProfileSpecFromJson(params);
    const auto specs = profile
                         ? mdl::createPlanarProfileBrushSpecs(*profile)
                         : std::optional<std::vector<mdl::PlanarProfileBrushSpec>>{};
    if (!specs)
    {
      return automation::invalidParams(
        "contour must be a closed simple [x,y] polygon with valid grid-snapped mitered "
        "insets; every band/core needs bottom, top, and a material (or top-level "
        "material)");
    }
    auto* parent = static_cast<mdl::Node*>(map.editorContext().currentLayer());
    if (params.contains("parentPath"))
    {
      const auto parentPath = automation::nodePathFromJson(params.value("parentPath"));
      parent = parentPath ? map.worldNode().resolvePath(*parentPath) : nullptr;
    }
    if (parent == nullptr)
    {
      return automation::invalidParams("parentPath must identify an existing node");
    }
    auto brushSpecs = std::vector<mdl::BrushCreationSpec>{};
    brushSpecs.reserve(specs->size());
    for (const auto& spec : *specs)
    {
      brushSpecs.push_back(spec.brush);
    }
    auto created = std::vector<mdl::BrushNode*>{};
    if (!mdl::createBrushes(map, *parent, brushSpecs, created))
    {
      return automation::invalidParams(
        "The generated profile brushes are outside map bounds or parentPath cannot "
        "accept "
        "brushes");
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"operation", "planarProfile"},
        {"count", static_cast<qint64>(created.size())},
        {"brushes", createdPlanarProfileBrushesToJson(created, *specs, map.worldNode())},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (method == "geometry.extrudeProfile.apply")
  {
    const auto extrusion = automation::profileExtrusionSpecFromJson(params);
    const auto specs = extrusion
                         ? mdl::createProfileExtrusionBrushSpecs(*extrusion)
                         : std::optional<std::vector<mdl::PlanarProfileBrushSpec>>{};
    if (!specs)
    {
      return automation::invalidParams(
        "plane (xy, xz, or yz), a closed simple profile of [u,v] points, a positive "
        "interval, and material are required. Concave profiles are triangulated into "
        "convex prisms.");
    }
    auto* parent = static_cast<mdl::Node*>(map.editorContext().currentLayer());
    if (params.contains("parentPath"))
    {
      const auto parentPath = automation::nodePathFromJson(params.value("parentPath"));
      parent = parentPath ? map.worldNode().resolvePath(*parentPath) : nullptr;
    }
    if (parent == nullptr)
    {
      return automation::invalidParams("parentPath must identify an existing node");
    }
    auto brushSpecs = std::vector<mdl::BrushCreationSpec>{};
    brushSpecs.reserve(specs->size());
    for (const auto& spec : *specs)
    {
      brushSpecs.push_back(spec.brush);
    }
    auto created = std::vector<mdl::BrushNode*>{};
    if (!mdl::createBrushes(map, *parent, brushSpecs, created))
    {
      return automation::invalidParams(
        "The generated profile-extrusion brushes are outside map bounds or parentPath "
        "cannot accept brushes");
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"operation", "extrudeProfile"},
        {"plane", params.value("plane").toString().toLower()},
        {"count", static_cast<qint64>(created.size())},
        {"brushes", createdPlanarProfileBrushesToJson(created, *specs, map.worldNode())},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (method == "geometry.bridgeEdgeChains")
  {
    const auto edges = [&]() {
      auto sourceBrushes = std::vector<mdl::BrushNode*>{};
      const auto resolved = resolveEdges(map, params.value("edges"), sourceBrushes);
      return std::pair{resolved, std::move(sourceBrushes)};
    }();
    const auto thickness = params.value("thickness");
    const auto direction = bridgeDirectionFromJson(params.value("direction"));
    const auto material = requiredString(params, "material");
    if (
      !edges.first || edges.first->size() < 2u || !thickness.isDouble()
      || thickness.toDouble() <= 0.0 || !direction || !material)
    {
      return automation::invalidParams(
        "edges, positive thickness, direction, and material are required");
    }

    auto state = GeometryOperationState{map};
    mdl::deselectAll(map);
    mdl::selectNodes(map, nodesFromBrushes(edges.second));
    auto& handles = map.nodeHandles();
    handles.deselectAllHandles<mdl::EdgeHandle>();
    handles.addHandles<mdl::EdgeHandle>(edges.second);
    handles.selectHandles<mdl::EdgeHandle>(*edges.first);
    const auto success =
      mdl::bridgeSelectedEdgeChains(map, thickness.toDouble(), *direction, *material);
    handles.deselectAllHandles<mdl::EdgeHandle>();
    if (!success)
    {
      return automation::invalidParams(
        "The selected edges must form two connected, coplanar, open chains");
    }
    state.keepChanges();
    return JsonRpcResponse::success(mutationResult(map, "bridgeEdgeChains"));
  }

  if (method == "geometry.volumeToPlane")
  {
    const auto axis = axisFromJson(params.value("axis"));
    const auto coordinate = params.value("coordinate");
    const auto material = requiredString(params, "material");
    if (!axis || !coordinate.isDouble() || !material)
    {
      return automation::invalidParams("axis, coordinate, and material are required");
    }

    auto state = std::optional<GeometryOperationState>{};
    if (params.contains("faces"))
    {
      const auto faces = resolveFaces(map, params.value("faces"));
      if (!faces || faces->empty())
      {
        return automation::invalidParams(
          "faces must be a non-empty array of brush faces");
      }
      state.emplace(map);
      mdl::deselectAll(map);
      mdl::selectBrushFaces(map, *faces);
    }
    else
    {
      const auto brushes = resolveBrushPaths(map, params.value("paths"));
      if (!brushes || brushes->empty())
      {
        return automation::invalidParams("paths must be a non-empty array of brushes");
      }
      state.emplace(map);
      mdl::deselectAll(map);
      mdl::selectNodes(map, nodesFromBrushes(*brushes));
    }

    if (!mdl::createVolumeToPlane(map, *axis, coordinate.toDouble(), *material))
    {
      return automation::invalidParams(
        "The target plane must be outside every selected brush or selected outward face");
    }
    state->keepChanges();
    return JsonRpcResponse::success(mutationResult(map, "volumeToPlane"));
  }

  if (method == "geometry.eqWater")
  {
    const auto brushes = resolveBrushPaths(map, params.value("paths"));
    const auto surfaceHeight = params.value("surfaceHeight");
    const auto surfaceThickness = params.value("surfaceThickness");
    const auto waterMaterial = requiredString(params, "waterMaterial");
    const auto surfaceMaterial = requiredString(params, "surfaceMaterial");
    if (
      !brushes || brushes->empty() || !surfaceHeight.isDouble()
      || !surfaceThickness.isDouble() || surfaceThickness.toDouble() <= 0.0
      || !waterMaterial || !surfaceMaterial)
    {
      return automation::invalidParams(
        "paths, surfaceHeight, positive surfaceThickness, waterMaterial, and "
        "surfaceMaterial are required");
    }
    auto state = GeometryOperationState{map};
    mdl::deselectAll(map);
    mdl::selectNodes(map, nodesFromBrushes(*brushes));
    if (!mdl::createEqWater(
          map,
          surfaceHeight.toDouble(),
          surfaceThickness.toDouble(),
          *waterMaterial,
          *surfaceMaterial))
    {
      return automation::invalidParams(
        "Selected brushes must be world riverbeds with a horizontal top face below the "
        "surface");
    }
    state.keepChanges();
    return JsonRpcResponse::success(mutationResult(map, "eqWater"));
  }

  if (method == "geometry.createBrushes")
  {
    const auto specs = resolveBrushCreationSpecs(params.value("brushes"));
    if (!specs)
    {
      return automation::invalidParams(
        "brushes must be a non-empty array of {points, material} objects");
    }

    auto* parent = static_cast<mdl::Node*>(map.editorContext().currentLayer());
    if (params.contains("parentPath"))
    {
      const auto parentPath = automation::nodePathFromJson(params.value("parentPath"));
      parent = parentPath ? map.worldNode().resolvePath(*parentPath) : nullptr;
      if (parent == nullptr)
      {
        return automation::invalidParams("parentPath must identify an existing node");
      }
    }
    if (parent == nullptr)
    {
      return automation::invalidParams("The document has no current layer");
    }

    auto createdBrushes = std::vector<mdl::BrushNode*>{};
    if (!mdl::createBrushes(map, *parent, *specs, createdBrushes))
    {
      return automation::invalidParams(
        "Every brush must be a valid convex polyhedron and parentPath must accept "
        "brushes");
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"operation", "createBrushes"},
        {"count", static_cast<qint64>(createdBrushes.size())},
        {"brushes", createdBrushesToJson(createdBrushes, map.worldNode())},
        {"revision", static_cast<qint64>(map.modificationCount())},
      });
  }

  if (method == "geometry.csg")
  {
    const auto brushes = resolveBrushPaths(map, params.value("paths"));
    const auto operation = params.value("operation").toString();
    if (!brushes || brushes->empty())
    {
      return automation::invalidParams("paths must be a non-empty array of brushes");
    }
    if (
      operation != "convexMerge" && operation != "subtract" && operation != "intersect"
      && operation != "hollow")
    {
      return automation::invalidParams(
        "operation must be convexMerge, subtract, intersect, or hollow");
    }
    const auto hasTargetPaths = params.contains("targetPaths");
    const auto targetBrushes = hasTargetPaths
                                 ? resolveBrushPaths(map, params.value("targetPaths"))
                                 : std::optional<std::vector<mdl::BrushNode*>>{};
    if (hasTargetPaths && operation != "subtract")
    {
      return automation::invalidParams("targetPaths is only supported by subtract");
    }
    if (hasTargetPaths && (!targetBrushes || targetBrushes->empty()))
    {
      return automation::invalidParams(
        "targetPaths must be a non-empty array of distinct brushes");
    }
    if (
      targetBrushes && std::ranges::any_of(*targetBrushes, [&](const auto* targetBrush) {
        return std::ranges::find(*brushes, targetBrush) != brushes->end();
      }))
    {
      return automation::invalidParams("paths and targetPaths must not overlap");
    }
    if ((operation == "convexMerge" || operation == "intersect") && brushes->size() < 2u)
    {
      return automation::invalidParams(
        "convexMerge and intersect require at least two brushes");
    }

    const auto material = params.contains("material") ? requiredString(params, "material")
                                                      : std::optional<std::string>{};
    if (params.contains("material"))
    {
      if (!material)
      {
        return automation::invalidParams(
          "material must be a non-empty string when supplied");
      }
    }
    auto state = GeometryOperationState{map};
    if (material)
    {
      map.setCurrentMaterialName(*material);
    }
    mdl::deselectAll(map);
    mdl::selectNodes(map, nodesFromBrushes(*brushes));
    auto replacementNodes = std::vector<mdl::BrushNode*>{};
    const auto success =
      operation == "convexMerge" ? mdl::csgConvexMerge(map)
      : operation == "subtract"
        ? targetBrushes
            ? mdl::csgSubtract(map, *brushes, *targetBrushes, replacementNodes)
            : mdl::csgSubtract(map)
      : operation == "intersect" ? mdl::csgIntersect(map)
                                 : mdl::csgHollow(map);
    if (!success)
    {
      return automation::invalidParams(
        "The CSG operation could not be applied to these brushes");
    }
    state.keepChanges();
    auto result = mutationResult(map, operation);
    if (targetBrushes)
    {
      result.insert("targetBrushCount", static_cast<qint64>(targetBrushes->size()));
      result.insert(
        "replacementBrushCount", static_cast<qint64>(replacementNodes.size()));
      result.insert(
        "replacementBrushes", createdBrushesToJson(replacementNodes, map.worldNode()));
    }
    return JsonRpcResponse::success(result);
  }

  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

} // namespace tb::ui
