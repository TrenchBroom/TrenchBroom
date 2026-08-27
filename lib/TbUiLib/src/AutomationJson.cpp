/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "AutomationJson.h"

#include <QJsonDocument>

#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/MapViewContext.h"
#include "ui/QPathUtils.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <tuple>

namespace tb::ui::automation
{
namespace
{

QJsonArray vec3ToJson(const vm::vec3f& value)
{
  return {value.x(), value.y(), value.z()};
}

QJsonArray vec3ToJson(const vm::vec3d& value)
{
  return {value.x(), value.y(), value.z()};
}

QJsonArray vec2ToJson(const vm::vec2f& value)
{
  return {value.x(), value.y()};
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

constexpr double FootprintEpsilon = 0.000001;

std::array<size_t, 2u> footprintComponents(const vm::axis::type axis)
{
  switch (axis)
  {
  case vm::axis::x:
    return {1u, 2u}; // Y x Z = X
  case vm::axis::y:
    return {2u, 0u}; // Z x X = Y
  case vm::axis::z:
    return {0u, 1u}; // X x Y = Z
  }
  return {0u, 1u};
}

bool lexicographicallyLess(
  const vm::vec3d& lhs, const vm::vec3d& rhs, const std::array<size_t, 2u>& components)
{
  if (lhs[components[0]] != rhs[components[0]])
  {
    return lhs[components[0]] < rhs[components[0]];
  }
  return lhs[components[1]] < rhs[components[1]];
}

double signedFootprintArea(
  const std::vector<vm::vec3d>& vertices, const std::array<size_t, 2u>& components)
{
  auto area = 0.0;
  for (size_t index = 0u; index < vertices.size(); ++index)
  {
    const auto& current = vertices[index];
    const auto& next = vertices[(index + 1u) % vertices.size()];
    area += current[components[0]] * next[components[1]]
            - current[components[1]] * next[components[0]];
  }
  return area;
}

std::vector<vm::vec3d> canonicalizeFootprint(
  std::vector<vm::vec3d> vertices, const vm::axis::type axis)
{
  const auto components = footprintComponents(axis);
  // The two components are chosen as a right-handed basis for +axis. This makes
  // the winding and first vertex independent of the source face normal and its
  // half-edge start point.
  if (signedFootprintArea(vertices, components) < 0.0)
  {
    std::reverse(vertices.begin(), vertices.end());
  }
  const auto first =
    std::ranges::min_element(vertices, [&](const auto& lhs, const auto& rhs) {
      return lexicographicallyLess(lhs, rhs, components);
    });
  std::rotate(vertices.begin(), first, vertices.end());
  return vertices;
}

bool sameFootprint(const std::vector<vm::vec3d>& lhs, const std::vector<vm::vec3d>& rhs)
{
  if (lhs.size() != rhs.size())
  {
    return false;
  }
  return std::ranges::equal(lhs, rhs, [](const auto& lhsVertex, const auto& rhsVertex) {
    return vm::squared_distance(lhsVertex, rhsVertex)
           <= FootprintEpsilon * FootprintEpsilon;
  });
}

bool footprintLess(const BrushFootprint& lhs, const BrushFootprint& rhs)
{
  const auto compareVertices = [&](const auto& left, const auto& right) {
    for (size_t component = 0u; component < 3u; ++component)
    {
      if (left[component] != right[component])
      {
        return left[component] < right[component];
      }
    }
    return false;
  };
  return std::lexicographical_compare(
    lhs.vertices.begin(),
    lhs.vertices.end(),
    rhs.vertices.begin(),
    rhs.vertices.end(),
    compareVertices);
}

bool sourceLess(const FootprintSource& lhs, const FootprintSource& rhs)
{
  if (lhs.path.indices != rhs.path.indices)
  {
    return lhs.path.indices < rhs.path.indices;
  }
  return lhs.faceIndex < rhs.faceIndex;
}

QJsonObject nodeReferenceToJson(const MapViewNodeReference& reference)
{
  return {
    {"path", nodePathToJson(reference.path)},
    {"name", QString::fromStdString(reference.name)}};
}

QString nodeTypeName(const mdl::Node& node)
{
  if (dynamic_cast<const mdl::WorldNode*>(&node))
    return "world";
  if (dynamic_cast<const mdl::LayerNode*>(&node))
    return "layer";
  if (dynamic_cast<const mdl::GroupNode*>(&node))
    return "group";
  if (dynamic_cast<const mdl::EntityNode*>(&node))
    return "entity";
  if (dynamic_cast<const mdl::BrushNode*>(&node))
    return "brush";
  if (dynamic_cast<const mdl::PatchNode*>(&node))
    return "patch";
  return "unknown";
}

QString nodeClassname(const mdl::Node& node)
{
  if (const auto* entity = dynamic_cast<const mdl::EntityNode*>(&node))
  {
    return QString::fromStdString(entity->entity().classname());
  }
  if (const auto* world = dynamic_cast<const mdl::WorldNode*>(&node))
  {
    return QString::fromStdString(world->entity().classname());
  }
  return {};
}

QJsonArray nodeMaterials(const mdl::Node& node)
{
  auto materials = QJsonArray{};
  if (const auto* brush = dynamic_cast<const mdl::BrushNode*>(&node))
  {
    for (const auto& face : brush->brush().faces())
    {
      const auto material = QString::fromStdString(face.materialName());
      if (!materials.contains(material))
      {
        materials.push_back(material);
      }
    }
  }
  else if (const auto* patch = dynamic_cast<const mdl::PatchNode*>(&node))
  {
    materials.push_back(QString::fromStdString(patch->patch().materialName()));
  }
  return materials;
}

bool matchesAny(const std::vector<QString>& values, const QString& candidate)
{
  return values.empty() || std::ranges::find(values, candidate) != values.end();
}

bool matchesAnyMaterial(const std::vector<QString>& materials, const mdl::Node& node)
{
  if (materials.empty())
  {
    return true;
  }
  for (const auto& material : nodeMaterials(node))
  {
    if (std::ranges::find(materials, material.toString()) != materials.end())
    {
      return true;
    }
  }
  return false;
}

QJsonObject nodeToJson(const mdl::Node& node, const mdl::WorldNode& world)
{
  const auto bounds = node.logicalBounds();
  auto result = QJsonObject{
    {"path", nodePathToJson(node.pathFrom(world))},
    {"type", nodeTypeName(node)},
    {"name", QString::fromStdString(node.name())},
    {"bounds",
     QJsonObject{{"min", vec3ToJson(bounds.min)}, {"max", vec3ToJson(bounds.max)}}},
  };

  const mdl::Entity* nodeEntity = nullptr;
  if (const auto* entity = dynamic_cast<const mdl::EntityNode*>(&node))
  {
    nodeEntity = &entity->entity();
  }
  else if (const auto* worldNode = dynamic_cast<const mdl::WorldNode*>(&node))
  {
    nodeEntity = &worldNode->entity();
  }
  if (nodeEntity != nullptr)
  {
    result.insert("classname", QString::fromStdString(nodeEntity->classname()));
    auto properties = QJsonObject{};
    for (const auto& property : nodeEntity->properties())
    {
      properties.insert(
        QString::fromStdString(property.key()), QString::fromStdString(property.value()));
    }
    result.insert("properties", properties);
  }

  const auto materials = nodeMaterials(node);
  if (!materials.empty())
  {
    result.insert("materials", materials);
  }
  return result;
}

bool matchesQuery(
  const mdl::Node& node,
  const mdl::WorldNode& world,
  const NodeQuery& query,
  QJsonObject& value)
{
  if (query.bounds && !query.bounds->intersects(node.logicalBounds()))
  {
    return false;
  }

  if (
    !matchesAny(query.types, nodeTypeName(node))
    || !matchesAny(query.classnames, nodeClassname(node))
    || !matchesAnyMaterial(query.materials, node))
  {
    return false;
  }

  value = nodeToJson(node, world);
  const auto searchable = QString{QJsonDocument{value}.toJson(QJsonDocument::Compact)};
  return query.pattern.isEmpty()
         || searchable.contains(query.pattern, Qt::CaseInsensitive);
}

void incrementCount(QJsonObject& counts, const QString& key)
{
  counts.insert(key, counts.value(key).toInt() + 1);
}

void addAggregate(const QJsonObject& value, QJsonObject& aggregate)
{
  auto byType = aggregate.value("byType").toObject();
  incrementCount(byType, value.value("type").toString());
  aggregate.insert("byType", byType);

  if (value.contains("classname"))
  {
    auto byClassname = aggregate.value("byClassname").toObject();
    incrementCount(byClassname, value.value("classname").toString());
    aggregate.insert("byClassname", byClassname);
  }

  auto byMaterial = aggregate.value("byMaterial").toObject();
  for (const auto& material : value.value("materials").toArray())
  {
    incrementCount(byMaterial, material.toString());
  }
  if (!byMaterial.empty())
  {
    aggregate.insert("byMaterial", byMaterial);
  }
}

void collectMatchingNodes(
  const mdl::Node& node,
  const mdl::WorldNode& world,
  const NodeQuery& query,
  NodeQueryResult& result)
{
  auto value = QJsonObject{};
  if (matchesQuery(node, world, query, value))
  {
    if (query.aggregate)
    {
      addAggregate(value, result.aggregate);
    }
    if (
      !query.aggregate && result.total >= query.offset
      && result.nodes.size() < query.limit)
    {
      result.nodes.push_back(
        query.pathsOnly ? QJsonValue{value.value("path").toArray()} : value);
    }
    ++result.total;
  }

  for (const auto* child : node.children())
  {
    collectMatchingNodes(*child, world, query, result);
  }
}

QString viewTypeName(const MapViewType viewType)
{
  switch (viewType)
  {
  case MapViewType::ThreeD:
    return "3d";
  case MapViewType::XY:
    return "xy";
  case MapViewType::XZ:
    return "xz";
  case MapViewType::YZ:
    return "yz";
  }
  return "unknown";
}

} // namespace

QJsonObject brushToJson(const mdl::BrushNode& brushNode, const mdl::WorldNode& world)
{
  const auto& brush = brushNode.brush();
  const auto bounds = brush.bounds();
  auto vertices = QJsonArray{};
  for (const auto& vertex : brush.vertexPositions())
  {
    vertices.push_back(vec3ToJson(vertex));
  }

  auto faces = QJsonArray{};
  for (size_t index = 0u; index < brush.faceCount(); ++index)
  {
    const auto& face = brush.face(index);
    auto faceVertices = QJsonArray{};
    for (const auto& vertex : face.vertexPositions())
    {
      faceVertices.push_back(vec3ToJson(vertex));
    }

    const auto& surface = face.surfaceAttributes();
    auto surfaceJson = QJsonObject{};
    if (surface.contents)
      surfaceJson.insert("contents", *surface.contents);
    if (surface.flags)
      surfaceJson.insert("flags", *surface.flags);
    if (surface.value)
      surfaceJson.insert("value", *surface.value);
    if (surface.color)
    {
      const auto color = surface.color->to<RgbaF>().toVec();
      surfaceJson.insert("color", QJsonArray{color.x(), color.y(), color.z(), color.w()});
    }

    const auto uv = face.uvAttributes();
    faces.push_back(
      QJsonObject{
        {"index", static_cast<qint64>(index)},
        {"boundary",
         QJsonObject{
           {"normal", vec3ToJson(face.boundary().normal)},
           {"distance", face.boundary().distance}}},
        {"vertices", faceVertices},
        {"material", QString::fromStdString(face.materialName())},
        {"surface", surfaceJson},
        {"uv",
         QJsonObject{
           {"offset", vec2ToJson(uv.offset)},
           {"scale", vec2ToJson(uv.scale)},
           {"rotation", uv.rotation},
           {"uAxis", vec3ToJson(face.uAxis())},
           {"vAxis", vec3ToJson(face.vAxis())}}},
      });
  }

  return {
    {"path", nodePathToJson(brushNode.pathFrom(world))},
    {"bounds",
     QJsonObject{{"min", vec3ToJson(bounds.min)}, {"max", vec3ToJson(bounds.max)}}},
    {"vertices", vertices},
    {"faces", faces},
  };
}

std::vector<BrushFootprint> extractFootprints(
  const mdl::WorldNode& world,
  const std::vector<const mdl::BrushNode*>& brushes,
  const FootprintFaceSelector& selector)
{
  auto result = std::vector<BrushFootprint>{};
  for (const auto* brushNode : brushes)
  {
    const auto path = brushNode->pathFrom(world);
    const auto& brush = brushNode->brush();
    for (size_t faceIndex = 0u; faceIndex < brush.faceCount(); ++faceIndex)
    {
      const auto& face = brush.face(faceIndex);
      if (
        std::abs(std::abs(face.normal()[selector.axis]) - 1.0) > FootprintEpsilon
        || (selector.material && face.materialName() != *selector.material))
      {
        continue;
      }

      auto vertices = face.vertexPositions();
      if (std::ranges::any_of(vertices, [&](const auto& vertex) {
            return std::abs(vertex[selector.axis] - selector.coordinate)
                   > FootprintEpsilon;
          }))
      {
        continue;
      }
      vertices = canonicalizeFootprint(std::move(vertices), selector.axis);

      const auto existing = std::ranges::find_if(result, [&](const auto& footprint) {
        return sameFootprint(footprint.vertices, vertices);
      });
      const auto source = FootprintSource{path, faceIndex};
      if (existing == result.end())
      {
        result.push_back({std::move(vertices), face.bounds(), {source}});
      }
      else
      {
        existing->sources.push_back(source);
      }
    }
  }

  for (auto& footprint : result)
  {
    std::sort(footprint.sources.begin(), footprint.sources.end(), sourceLess);
  }
  std::sort(result.begin(), result.end(), footprintLess);
  return result;
}

QJsonArray footprintsToJson(const std::vector<BrushFootprint>& footprints)
{
  auto result = QJsonArray{};
  for (const auto& footprint : footprints)
  {
    auto vertices = QJsonArray{};
    for (const auto& vertex : footprint.vertices)
    {
      vertices.push_back(vec3ToJson(vertex));
    }
    auto sources = QJsonArray{};
    for (const auto& source : footprint.sources)
    {
      sources.push_back(
        QJsonObject{
          {"path", nodePathToJson(source.path)},
          {"faceIndex", static_cast<qint64>(source.faceIndex)}});
    }
    result.push_back(
      QJsonObject{
        {"vertices", vertices},
        {"bounds",
         QJsonObject{
           {"min", vec3ToJson(footprint.bounds.min)},
           {"max", vec3ToJson(footprint.bounds.max)}}},
        {"sources", sources}});
  }
  return result;
}

JsonRpcResponse invalidParams(const QString& message)
{
  return JsonRpcResponse::error(
    {JsonRpcError::InvalidParams, "Invalid params", QJsonValue{message}});
}

JsonRpcResponse revisionConflict(const size_t actualRevision)
{
  return JsonRpcResponse::error(
    {-32001,
     "Revision conflict",
     QJsonObject{{"actualRevision", static_cast<qint64>(actualRevision)}}});
}

QJsonObject contextToJson(const MapViewContext& context)
{
  auto selectedNodes = QJsonArray{};
  for (const auto& node : context.selectedNodes)
  {
    selectedNodes.push_back(nodeReferenceToJson(node));
  }

  auto selectedFaces = QJsonArray{};
  for (const auto& face : context.selectedFaces)
  {
    auto value = nodeReferenceToJson(face.node);
    value.insert("faceIndex", static_cast<qint64>(face.faceIndex));
    selectedFaces.push_back(value);
  }

  auto layers = QJsonArray{};
  for (const auto& layer : context.layers)
  {
    auto value = nodeReferenceToJson(layer.node);
    value.insert("visible", layer.visible);
    value.insert("current", layer.current);
    layers.push_back(value);
  }

  const auto& viewport = context.camera.viewport;
  return {
    {"document",
     QJsonObject{
       {"path", pathAsQString(context.document.path)},
       {"filename", QString::fromStdString(context.document.filename)},
       {"revision", static_cast<qint64>(context.document.revision)},
       {"modified", context.document.modified}}},
    {"viewType", viewTypeName(context.viewType)},
    {"camera",
     QJsonObject{
       {"orthographic", context.camera.orthographicProjection},
       {"viewport",
        QJsonObject{
          {"x", viewport.x},
          {"y", viewport.y},
          {"width", viewport.width},
          {"height", viewport.height}}},
       {"near", context.camera.nearPlane},
       {"far", context.camera.farPlane},
       {"zoom", context.camera.zoom},
       {"position", vec3ToJson(context.camera.position)},
       {"direction", vec3ToJson(context.camera.direction)},
       {"up", vec3ToJson(context.camera.up)},
       {"right", vec3ToJson(context.camera.right)}}},
    {"grid",
     QJsonObject{
       {"size", context.grid.size},
       {"actualSize", context.grid.actualSize},
       {"snap", context.grid.snap},
       {"visible", context.grid.visible}}},
    {"currentMaterial", QString::fromStdString(context.currentMaterialName)},
    {"selectedNodes", selectedNodes},
    {"selectedFaces", selectedFaces},
    {"layers", layers},
  };
}

QJsonObject pickToJson(const MapViewPickResult& pick)
{
  auto hits = QJsonArray{};
  for (const auto& hit : pick.hits)
  {
    auto value = QJsonObject{
      {"type", static_cast<qint64>(hit.type)},
      {"distance", hit.distance},
      {"error", hit.error},
      {"point", vec3ToJson(hit.point)},
    };
    if (hit.node)
    {
      value.insert("node", nodeReferenceToJson(*hit.node));
    }
    if (hit.faceIndex)
    {
      value.insert("faceIndex", static_cast<qint64>(*hit.faceIndex));
    }
    hits.push_back(value);
  }

  return {
    {"ray",
     QJsonObject{
       {"origin", vec3ToJson(pick.ray.origin)},
       {"direction", vec3ToJson(pick.ray.direction)}}},
    {"hits", hits},
  };
}

QJsonArray queryNodes(
  const mdl::WorldNode& world, const QString& pattern, const int limit)
{
  return queryNodes(world, NodeQuery{.pattern = pattern, .limit = limit}).nodes;
}

NodeQueryResult queryNodes(const mdl::WorldNode& world, const NodeQuery& query)
{
  auto result = NodeQueryResult{};
  const auto* ancestor =
    query.ancestorPath ? world.resolvePath(*query.ancestorPath) : &world;
  if (ancestor == nullptr)
  {
    return result;
  }

  collectMatchingNodes(*ancestor, world, query, result);
  if (query.aggregate)
  {
    result.aggregate.insert("total", result.total);
  }
  else if (result.total > query.offset + result.nodes.size())
  {
    result.truncated = true;
    result.nextOffset = query.offset + result.nodes.size();
  }
  return result;
}

std::optional<mdl::NodePath> nodePathFromJson(const QJsonValue& value)
{
  if (!value.isArray())
    return std::nullopt;
  auto path = mdl::NodePath{};
  for (const auto& index : value.toArray())
  {
    const auto decodedIndex = sizeFromJson(index);
    if (!decodedIndex)
      return std::nullopt;
    path.indices.push_back(*decodedIndex);
  }
  return path;
}

std::optional<size_t> sizeFromJson(const QJsonValue& value)
{
  if (!value.isDouble())
  {
    return std::nullopt;
  }

  const auto number = value.toDouble();
  // The largest size_t cannot necessarily be represented exactly as a double. Reject
  // its rounded-up representation as well, so the cast is always defined.
  if (
    !std::isfinite(number) || number < 0.0 || std::trunc(number) != number
    || number >= static_cast<double>(std::numeric_limits<size_t>::max()))
  {
    return std::nullopt;
  }
  return static_cast<size_t>(number);
}

std::optional<vm::vec3d> vec3FromJson(const QJsonValue& value)
{
  const auto array = value.toArray();
  if (
    array.size() != 3 || !array[0].isDouble() || !array[1].isDouble()
    || !array[2].isDouble())
  {
    return std::nullopt;
  }
  return vm::vec3d{array[0].toDouble(), array[1].toDouble(), array[2].toDouble()};
}

std::optional<mdl::PlanarProfileSpec> planarProfileSpecFromJson(const QJsonObject& params)
{
  const auto axis =
    params.contains("axis") ? params.value("axis").toString().toLower() : QString{"z"};
  const auto gridSizeValue =
    params.contains("gridSize") ? params.value("gridSize") : QJsonValue{1.0};
  if (
    axis != "z" || !gridSizeValue.isDouble() || !std::isfinite(gridSizeValue.toDouble())
    || gridSizeValue.toDouble() <= 0.0 || !params.value("contour").isArray())
  {
    return std::nullopt;
  }
  const auto gridSize = gridSizeValue.toDouble();
  const auto snappedNumber = [&](const QJsonValue& value) -> std::optional<double> {
    if (!value.isDouble() || !std::isfinite(value.toDouble()))
    {
      return std::nullopt;
    }
    return std::round(value.toDouble() / gridSize) * gridSize;
  };
  const auto snappedPoint = [&](const QJsonValue& value) -> std::optional<vm::vec2d> {
    const auto array = value.toArray();
    if (array.size() != 2)
    {
      return std::nullopt;
    }
    const auto x = snappedNumber(array[0]);
    const auto y = snappedNumber(array[1]);
    return x && y ? std::optional<vm::vec2d>{vm::vec2d{*x, *y}} : std::nullopt;
  };
  auto contour = std::vector<vm::vec2d>{};
  for (const auto& pointValue : params.value("contour").toArray())
  {
    const auto point = snappedPoint(pointValue);
    if (!point)
    {
      return std::nullopt;
    }
    contour.push_back(*point);
  }
  if (contour.size() < 3u)
  {
    return std::nullopt;
  }

  const auto stringValue =
    [](const QJsonObject& object, const QString& name) -> std::optional<std::string> {
    const auto value = object.value(name);
    return value.isString() && !value.toString().isEmpty()
             ? std::optional<std::string>{value.toString().toStdString()}
             : std::nullopt;
  };
  const auto defaultMaterial = params.contains("material")
                                 ? stringValue(params, "material")
                                 : std::optional<std::string>{};
  if (params.contains("material") && !defaultMaterial)
  {
    return std::nullopt;
  }
  const auto materialFor = [&](const QJsonObject& part) -> std::optional<std::string> {
    return part.contains("material") ? stringValue(part, "material") : defaultMaterial;
  };
  const auto roleFor = [&](
                         const QJsonObject& part,
                         const std::string& defaultRole) -> std::optional<std::string> {
    return part.contains("role") ? stringValue(part, "role")
                                 : std::optional<std::string>{defaultRole};
  };
  const auto verticalRange =
    [&](const QJsonObject& part) -> std::optional<std::pair<double, double>> {
    const auto bottom = snappedNumber(part.value("bottom"));
    const auto top = snappedNumber(part.value("top"));
    return bottom && top && *top > *bottom ? std::optional{std::pair{*bottom, *top}}
                                           : std::nullopt;
  };

  auto bands = std::vector<mdl::PlanarProfileBand>{};
  if (params.contains("bands"))
  {
    if (!params.value("bands").isArray())
    {
      return std::nullopt;
    }
    const auto bandValues = params.value("bands").toArray();
    bands.reserve(static_cast<size_t>(bandValues.size()));
    for (auto i = 0; i < bandValues.size(); ++i)
    {
      if (!bandValues[i].isObject())
      {
        return std::nullopt;
      }
      const auto part = bandValues[i].toObject();
      const auto inset = snappedNumber(part.value("inset"));
      const auto range = verticalRange(part);
      const auto material = materialFor(part);
      const auto role = roleFor(part, QString{"band-%1"}.arg(i).toStdString());
      if (!inset || !range || !material || !role)
      {
        return std::nullopt;
      }
      bands.push_back({*inset, range->first, range->second, *material, *role});
    }
  }

  auto core = std::optional<mdl::PlanarProfileCore>{};
  if (params.contains("core"))
  {
    if (!params.value("core").isObject())
    {
      return std::nullopt;
    }
    const auto part = params.value("core").toObject();
    const auto range = verticalRange(part);
    const auto material = materialFor(part);
    const auto role = roleFor(part, "core");
    if (!range || !material || !role)
    {
      return std::nullopt;
    }
    core.emplace(range->first, range->second, *material, *role);
  }
  return bands.empty() && !core
           ? std::nullopt
           : std::optional<mdl::PlanarProfileSpec>{mdl::PlanarProfileSpec{
               std::move(contour), std::move(bands), std::move(core), gridSize}};
}

std::optional<mdl::ProfileExtrusionSpec> profileExtrusionSpecFromJson(
  const QJsonObject& params)
{
  const auto planeValue = params.value("plane");
  const auto plane = planeValue.isString() ? planeValue.toString().toLower() : QString{};
  const auto extrusionPlane =
    plane == "xy"   ? std::optional{mdl::ProfileExtrusionPlane::XY}
    : plane == "xz" ? std::optional{mdl::ProfileExtrusionPlane::XZ}
    : plane == "yz" ? std::optional{mdl::ProfileExtrusionPlane::YZ}
                    : std::optional<mdl::ProfileExtrusionPlane>{};
  const auto gridSizeValue =
    params.contains("gridSize") ? params.value("gridSize") : QJsonValue{1.0};
  const auto interval = params.value("interval").toArray();
  const auto material = params.value("material");
  const auto roleValue =
    params.contains("role") ? params.value("role") : QJsonValue{"profile"};
  if (
    !extrusionPlane || !gridSizeValue.isDouble()
    || !std::isfinite(gridSizeValue.toDouble()) || gridSizeValue.toDouble() <= 0.0
    || !params.value("profile").isArray() || interval.size() != 2
    || !interval[0].isDouble() || !interval[1].isDouble() || !material.isString()
    || material.toString().isEmpty() || !roleValue.isString()
    || roleValue.toString().isEmpty())
  {
    return std::nullopt;
  }

  const auto gridSize = gridSizeValue.toDouble();
  const auto snap = [&](const QJsonValue& value) -> std::optional<double> {
    if (!value.isDouble() || !std::isfinite(value.toDouble()))
    {
      return std::nullopt;
    }
    return std::round(value.toDouble() / gridSize) * gridSize;
  };
  const auto minimum = snap(interval[0]);
  const auto maximum = snap(interval[1]);
  if (!minimum || !maximum || *maximum <= *minimum)
  {
    return std::nullopt;
  }

  auto profile = std::vector<vm::vec2d>{};
  for (const auto& pointValue : params.value("profile").toArray())
  {
    const auto point = pointValue.toArray();
    if (point.size() != 2)
    {
      return std::nullopt;
    }
    const auto first = snap(point[0]);
    const auto second = snap(point[1]);
    if (!first || !second)
    {
      return std::nullopt;
    }
    profile.emplace_back(*first, *second);
  }
  if (profile.size() < 3u)
  {
    return std::nullopt;
  }

  return mdl::ProfileExtrusionSpec{
    *extrusionPlane,
    std::move(profile),
    *minimum,
    *maximum,
    gridSize,
    material.toString().toStdString(),
    roleValue.toString().toStdString()};
}

} // namespace tb::ui::automation
