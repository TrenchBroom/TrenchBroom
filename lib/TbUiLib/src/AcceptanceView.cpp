/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceView.h"

#include <QJsonArray>

#include "ui/QPathUtils.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <ostream>
#include <ranges>
#include <set>
#include <variant>

namespace tb::ui
{
namespace
{

AcceptanceError error(const AcceptanceErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

template <typename Value>
AcceptanceError resultError(const Result<Value, AcceptanceError>& result)
{
  return std::get<AcceptanceError>(result.error());
}

bool finite(const double value)
{
  return std::isfinite(value);
}

bool finite(const vm::vec3d& value)
{
  return finite(value.x()) && finite(value.y()) && finite(value.z());
}

bool validId(const std::string& id)
{
  return !id.empty() && std::ranges::all_of(id, [](const unsigned char c) {
    return std::isalnum(c) || c == '-' || c == '_' || c == '.';
  });
}

bool validRelativePath(const std::filesystem::path& path)
{
  return !path.empty() && path.is_relative() && path != ".";
}

QJsonArray vecToJson(const vm::vec3d& vector)
{
  return {vector.x(), vector.y(), vector.z()};
}

Result<vm::vec3d, AcceptanceError> vecFromJson(const QJsonValue& value, const char* field)
{
  if (!value.isArray())
  {
    return error(
      AcceptanceErrorCode::InvalidJson, std::string{"'"} + field + "' must be an array");
  }
  const auto array = value.toArray();
  if (
    array.size() != 3 || !array[0].isDouble() || !array[1].isDouble()
    || !array[2].isDouble())
  {
    return error(
      AcceptanceErrorCode::InvalidJson,
      std::string{"'"} + field + "' must contain exactly three numbers");
  }
  const auto vector =
    vm::vec3d{array[0].toDouble(), array[1].toDouble(), array[2].toDouble()};
  if (!finite(vector.x()) || !finite(vector.y()) || !finite(vector.z()))
  {
    return error(
      AcceptanceErrorCode::InvalidValue, std::string{"'"} + field + "' must be finite");
  }
  return vector;
}

QJsonObject sizeToJson(const AcceptanceImageSize& size)
{
  return {{"width", size.width}, {"height", size.height}};
}

Result<AcceptanceImageSize, AcceptanceError> sizeFromJson(const QJsonValue& value)
{
  if (!value.isObject())
  {
    return error(AcceptanceErrorCode::InvalidJson, "'size' must be an object");
  }
  const auto object = value.toObject();
  const auto width = object.value("width");
  const auto height = object.value("height");
  if (
    !width.isDouble() || !height.isDouble() || width.toInt() != width.toDouble()
    || height.toInt() != height.toDouble())
  {
    return error(
      AcceptanceErrorCode::InvalidJson, "'size' must contain integer width and height");
  }
  return AcceptanceImageSize{width.toInt(), height.toInt()};
}

QJsonObject cameraToJson(const AcceptanceCamera& camera)
{
  auto result = QJsonObject{
    {"projection",
     camera.projection == AcceptanceProjection::Perspective ? "perspective"
                                                            : "orthographic"},
    {"position", vecToJson(camera.position)},
    {"direction", vecToJson(camera.direction)},
    {"up", vecToJson(camera.up)},
    {"near", camera.nearPlane},
    {"far", camera.farPlane},
  };
  if (camera.verticalFov)
    result.insert("verticalFov", *camera.verticalFov);
  if (camera.zoom)
    result.insert("zoom", *camera.zoom);
  return result;
}

Result<AcceptanceCamera, AcceptanceError> cameraFromJson(const QJsonValue& value)
{
  if (!value.isObject())
  {
    return error(AcceptanceErrorCode::InvalidJson, "'camera' must be an object");
  }
  const auto object = value.toObject();
  const auto projection = object.value("projection");
  if (!projection.isString())
  {
    return error(
      AcceptanceErrorCode::InvalidJson, "'camera.projection' must be a string");
  }
  auto camera = AcceptanceCamera{};
  const auto projectionName = projection.toString();
  if (projectionName == "perspective")
  {
    camera.projection = AcceptanceProjection::Perspective;
  }
  else if (projectionName == "orthographic")
  {
    camera.projection = AcceptanceProjection::Orthographic;
  }
  else
  {
    return error(AcceptanceErrorCode::InvalidValue, "Unknown camera projection");
  }
  for (const auto& [key, destination] : {
         std::pair{"position", &camera.position},
         std::pair{"direction", &camera.direction},
         std::pair{"up", &camera.up},
       })
  {
    const auto parsed = vecFromJson(object.value(key), key);
    if (parsed.is_error())
    {
      return resultError(parsed);
    }
    *destination = parsed.value();
  }
  for (const auto& [key, destination] : {
         std::pair{"near", &camera.nearPlane},
         std::pair{"far", &camera.farPlane},
       })
  {
    const auto number = object.value(key);
    if (!number.isDouble() || !finite(number.toDouble()))
    {
      return error(
        AcceptanceErrorCode::InvalidJson,
        "'camera." + std::string{key} + "' must be finite");
    }
    *destination = number.toDouble();
  }
  const auto optionalNumber =
    [&](const char* key) -> Result<std::optional<double>, AcceptanceError> {
    if (!object.contains(key))
      return std::nullopt;
    const auto number = object.value(key);
    if (!number.isDouble() || !finite(number.toDouble()))
      return error(
        AcceptanceErrorCode::InvalidJson,
        "'camera." + std::string{key} + "' must be finite");
    return number.toDouble();
  };
  const auto verticalFov = optionalNumber("verticalFov");
  const auto zoom = optionalNumber("zoom");
  if (verticalFov.is_error())
    return resultError(verticalFov);
  if (zoom.is_error())
    return resultError(zoom);
  camera.verticalFov = verticalFov.value();
  camera.zoom = zoom.value();
  return camera;
}

QJsonObject overlaysToJson(const AcceptanceOverlays& overlays)
{
  return {
    {"brushEdges", overlays.brushEdges},
    {"selection", overlays.selection},
    {"grid", overlays.grid},
  };
}

Result<AcceptanceOverlays, AcceptanceError> overlaysFromJson(const QJsonValue& value)
{
  if (!value.isObject())
  {
    return error(AcceptanceErrorCode::InvalidJson, "'overlays' must be an object");
  }
  const auto object = value.toObject();
  AcceptanceOverlays overlays;
  for (const auto& [key, destination] : {
         std::pair{"brushEdges", &overlays.brushEdges},
         std::pair{"selection", &overlays.selection},
         std::pair{"grid", &overlays.grid},
       })
  {
    const auto boolean = object.value(key);
    if (!boolean.isBool())
    {
      return error(
        AcceptanceErrorCode::InvalidJson,
        "'overlays." + std::string{key} + "' must be boolean");
    }
    *destination = boolean.toBool();
  }
  return overlays;
}

template <typename Enum>
std::optional<Enum> enumFromString(const QString& value);

template <>
std::optional<AcceptanceAlignmentType> enumFromString(const QString& value)
{
  if (value == "identity")
    return AcceptanceAlignmentType::Identity;
  if (value == "matrix")
    return AcceptanceAlignmentType::Matrix;
  if (value == "landmarks")
    return AcceptanceAlignmentType::Landmarks;
  if (value == "independent")
    return AcceptanceAlignmentType::Independent;
  return std::nullopt;
}

template <>
std::optional<AcceptanceMetricType> enumFromString(const QString& value)
{
  if (value == "color")
    return AcceptanceMetricType::Color;
  if (value == "depth")
    return AcceptanceMetricType::Depth;
  if (value == "silhouette")
    return AcceptanceMetricType::Silhouette;
  if (value == "edgeMap")
    return AcceptanceMetricType::EdgeMap;
  if (value == "materialId")
    return AcceptanceMetricType::MaterialId;
  if (value == "objectId")
    return AcceptanceMetricType::ObjectId;
  return std::nullopt;
}

template <>
std::optional<AcceptanceAssertionType> enumFromString(const QString& value)
{
  if (value == "boundsVisible")
    return AcceptanceAssertionType::BoundsVisible;
  if (value == "boundsNotVisible")
    return AcceptanceAssertionType::BoundsNotVisible;
  if (value == "clearSightline")
    return AcceptanceAssertionType::ClearSightline;
  if (value == "openingClearance")
    return AcceptanceAssertionType::OpeningClearance;
  if (value == "materialCoverage")
    return AcceptanceAssertionType::MaterialCoverage;
  if (value == "depthRange")
    return AcceptanceAssertionType::DepthRange;
  return std::nullopt;
}

QString toString(const AcceptanceAlignmentType type)
{
  switch (type)
  {
  case AcceptanceAlignmentType::Identity:
    return "identity";
  case AcceptanceAlignmentType::Matrix:
    return "matrix";
  case AcceptanceAlignmentType::Landmarks:
    return "landmarks";
  case AcceptanceAlignmentType::Independent:
    return "independent";
  }
  return {};
}

QString toString(const AcceptanceMetricType type)
{
  switch (type)
  {
  case AcceptanceMetricType::Color:
    return "color";
  case AcceptanceMetricType::Depth:
    return "depth";
  case AcceptanceMetricType::Silhouette:
    return "silhouette";
  case AcceptanceMetricType::EdgeMap:
    return "edgeMap";
  case AcceptanceMetricType::MaterialId:
    return "materialId";
  case AcceptanceMetricType::ObjectId:
    return "objectId";
  }
  return {};
}

QString toString(const AcceptanceAssertionType type)
{
  switch (type)
  {
  case AcceptanceAssertionType::BoundsVisible:
    return "boundsVisible";
  case AcceptanceAssertionType::BoundsNotVisible:
    return "boundsNotVisible";
  case AcceptanceAssertionType::ClearSightline:
    return "clearSightline";
  case AcceptanceAssertionType::OpeningClearance:
    return "openingClearance";
  case AcceptanceAssertionType::MaterialCoverage:
    return "materialCoverage";
  case AcceptanceAssertionType::DepthRange:
    return "depthRange";
  }
  return {};
}

QJsonObject alignmentToJson(const AcceptanceAlignment& alignment)
{
  auto result = QJsonObject{{"type", toString(alignment.type)}};
  if (alignment.type == AcceptanceAlignmentType::Matrix)
  {
    auto matrix = QJsonArray{};
    for (const auto value : alignment.matrix)
      matrix.push_back(value);
    result.insert("matrix", matrix);
  }
  if (alignment.type == AcceptanceAlignmentType::Landmarks)
  {
    auto landmarks = QJsonArray{};
    for (const auto& landmark : alignment.landmarks)
    {
      landmarks.push_back(
        QJsonObject{
          {"reference", vecToJson(landmark.reference)},
          {"target", vecToJson(landmark.target)},
        });
    }
    result.insert("landmarks", landmarks);
  }
  return result;
}

Result<AcceptanceAlignment, AcceptanceError> alignmentFromJson(const QJsonValue& value)
{
  if (!value.isObject() || !value.toObject().value("type").isString())
  {
    return error(AcceptanceErrorCode::InvalidJson, "'alignment.type' must be a string");
  }
  const auto object = value.toObject();
  const auto type =
    enumFromString<AcceptanceAlignmentType>(object.value("type").toString());
  if (!type)
  {
    return error(AcceptanceErrorCode::InvalidValue, "Unknown alignment type");
  }
  auto alignment = AcceptanceAlignment{};
  alignment.type = *type;
  if (*type == AcceptanceAlignmentType::Matrix)
  {
    const auto matrix = object.value("matrix");
    if (!matrix.isArray() || matrix.toArray().size() != 16)
    {
      return error(
        AcceptanceErrorCode::InvalidJson, "Matrix alignment requires 16 matrix values");
    }
    for (size_t i = 0u; i < alignment.matrix.size(); ++i)
    {
      const auto number = matrix.toArray().at(static_cast<qsizetype>(i));
      if (!number.isDouble() || !finite(number.toDouble()))
      {
        return error(
          AcceptanceErrorCode::InvalidValue, "Matrix values must be finite numbers");
      }
      alignment.matrix[i] = number.toDouble();
    }
  }
  if (*type == AcceptanceAlignmentType::Landmarks)
  {
    const auto landmarks = object.value("landmarks");
    if (!landmarks.isArray() || landmarks.toArray().size() < 3)
    {
      return error(
        AcceptanceErrorCode::InvalidJson,
        "Landmark alignment requires at least three landmarks");
    }
    for (const auto& landmarkValue : landmarks.toArray())
    {
      if (!landmarkValue.isObject())
      {
        return error(AcceptanceErrorCode::InvalidJson, "Each landmark must be an object");
      }
      const auto landmarkObject = landmarkValue.toObject();
      const auto reference =
        vecFromJson(landmarkObject.value("reference"), "landmark.reference");
      const auto target = vecFromJson(landmarkObject.value("target"), "landmark.target");
      if (reference.is_error())
        return resultError(reference);
      if (target.is_error())
        return resultError(target);
      alignment.landmarks.push_back({reference.value(), target.value()});
    }
  }
  return alignment;
}

QJsonObject boundsToJson(const AcceptanceBounds& bounds)
{
  return {{"min", vecToJson(bounds.min)}, {"max", vecToJson(bounds.max)}};
}

Result<AcceptanceBounds, AcceptanceError> boundsFromJson(const QJsonValue& value)
{
  if (!value.isObject())
    return error(AcceptanceErrorCode::InvalidJson, "'bounds' must be an object");
  const auto object = value.toObject();
  const auto min = vecFromJson(object.value("min"), "bounds.min");
  const auto max = vecFromJson(object.value("max"), "bounds.max");
  if (min.is_error())
    return resultError(min);
  if (max.is_error())
    return resultError(max);
  return AcceptanceBounds{min.value(), max.value()};
}

template <typename Item>
void sortById(std::vector<Item>& items)
{
  std::ranges::sort(items, {}, &Item::id);
}

QJsonObject namedViewToJson(const AcceptanceNamedView& view)
{
  return {
    {"id", QString::fromStdString(view.id)},
    {"name", QString::fromStdString(view.name)},
    {"camera", cameraToJson(view.camera)},
    {"size", sizeToJson(view.size)},
    {"renderMode", QString::fromStdString(view.renderMode)},
    {"overlays", overlaysToJson(view.overlays)},
  };
}

Result<AcceptanceNamedView, AcceptanceError> namedViewFromJson(const QJsonValue& value)
{
  if (!value.isObject())
    return error(AcceptanceErrorCode::InvalidJson, "Each view must be an object");
  const auto object = value.toObject();
  if (
    !object.value("id").isString() || !object.value("name").isString()
    || !object.value("renderMode").isString())
  {
    return error(
      AcceptanceErrorCode::InvalidJson, "View id, name, and renderMode must be strings");
  }
  const auto camera = cameraFromJson(object.value("camera"));
  const auto size = sizeFromJson(object.value("size"));
  const auto overlays = overlaysFromJson(object.value("overlays"));
  if (camera.is_error())
    return resultError(camera);
  if (size.is_error())
    return resultError(size);
  if (overlays.is_error())
    return resultError(overlays);
  return AcceptanceNamedView{
    object.value("id").toString().toStdString(),
    object.value("name").toString().toStdString(),
    camera.value(),
    size.value(),
    object.value("renderMode").toString().toStdString(),
    overlays.value(),
  };
}

QJsonObject documentReferenceToJson(const AcceptanceDocumentReference& reference)
{
  return {
    {"documentPath", pathAsGenericQString(reference.path)},
    {"viewId", QString::fromStdString(reference.viewId)},
  };
}

Result<AcceptanceDocumentReference, AcceptanceError> documentReferenceFromJson(
  const QJsonValue& value)
{
  if (!value.isObject())
    return error(
      AcceptanceErrorCode::InvalidJson, "Document reference must be an object");
  const auto object = value.toObject();
  if (!object.value("documentPath").isString() || !object.value("viewId").isString())
  {
    return error(
      AcceptanceErrorCode::InvalidJson,
      "Document reference needs documentPath and viewId strings");
  }
  return AcceptanceDocumentReference{
    pathFromQString(object.value("documentPath").toString()),
    object.value("viewId").toString().toStdString(),
  };
}

QJsonObject maskToJson(const AcceptanceMask& mask)
{
  return {
    {"id", QString::fromStdString(mask.id)},
    {"x", mask.x},
    {"y", mask.y},
    {"width", mask.width},
    {"height", mask.height}};
}

Result<AcceptanceMask, AcceptanceError> maskFromJson(const QJsonValue& value)
{
  if (!value.isObject())
    return error(AcceptanceErrorCode::InvalidJson, "Each mask must be an object");
  const auto object = value.toObject();
  if (!object.value("id").isString())
    return error(AcceptanceErrorCode::InvalidJson, "Mask id must be a string");
  auto mask = AcceptanceMask{};
  mask.id = object.value("id").toString().toStdString();
  for (const auto& [key, destination] :
       {std::pair{"x", &mask.x},
        std::pair{"y", &mask.y},
        std::pair{"width", &mask.width},
        std::pair{"height", &mask.height}})
  {
    const auto number = object.value(key);
    if (!number.isDouble() || !finite(number.toDouble()))
      return error(
        AcceptanceErrorCode::InvalidJson, "Mask coordinates must be finite numbers");
    *destination = number.toDouble();
  }
  return mask;
}

QJsonObject metricToJson(const AcceptanceMetric& metric)
{
  auto result = QJsonObject{
    {"id", QString::fromStdString(metric.id)},
    {"type", toString(metric.type)},
    {"configuration", metric.configuration}};
  if (metric.maskId)
    result.insert("maskId", QString::fromStdString(*metric.maskId));
  return result;
}

Result<AcceptanceMetric, AcceptanceError> metricFromJson(const QJsonValue& value)
{
  if (!value.isObject())
    return error(AcceptanceErrorCode::InvalidJson, "Each metric must be an object");
  const auto object = value.toObject();
  if (
    !object.value("id").isString() || !object.value("type").isString()
    || !object.value("configuration").isObject())
  {
    return error(
      AcceptanceErrorCode::InvalidJson, "Metric needs id, type, and configuration");
  }
  const auto type = enumFromString<AcceptanceMetricType>(object.value("type").toString());
  if (!type)
    return error(AcceptanceErrorCode::InvalidValue, "Unknown metric type");
  if (object.contains("maskId") && !object.value("maskId").isString())
    return error(AcceptanceErrorCode::InvalidJson, "Metric maskId must be a string");
  return AcceptanceMetric{
    object.value("id").toString().toStdString(),
    *type,
    object.contains("maskId")
      ? std::optional{object.value("maskId").toString().toStdString()}
      : std::nullopt,
    object.value("configuration").toObject()};
}

QJsonObject assertionToJson(const AcceptanceAssertion& assertion)
{
  auto result = QJsonObject{
    {"id", QString::fromStdString(assertion.id)},
    {"type", toString(assertion.type)},
    {"configuration", assertion.configuration}};
  if (assertion.maskId)
    result.insert("maskId", QString::fromStdString(*assertion.maskId));
  if (assertion.bounds)
    result.insert("bounds", boundsToJson(*assertion.bounds));
  return result;
}

Result<AcceptanceAssertion, AcceptanceError> assertionFromJson(const QJsonValue& value)
{
  if (!value.isObject())
    return error(AcceptanceErrorCode::InvalidJson, "Each assertion must be an object");
  const auto object = value.toObject();
  if (
    !object.value("id").isString() || !object.value("type").isString()
    || !object.value("configuration").isObject())
  {
    return error(
      AcceptanceErrorCode::InvalidJson, "Assertion needs id, type, and configuration");
  }
  const auto type =
    enumFromString<AcceptanceAssertionType>(object.value("type").toString());
  if (!type)
    return error(AcceptanceErrorCode::InvalidValue, "Unknown assertion type");
  if (object.contains("maskId") && !object.value("maskId").isString())
    return error(AcceptanceErrorCode::InvalidJson, "Assertion maskId must be a string");
  auto assertion = AcceptanceAssertion{
    object.value("id").toString().toStdString(),
    *type,
    object.contains("maskId")
      ? std::optional{object.value("maskId").toString().toStdString()}
      : std::nullopt,
    std::nullopt,
    object.value("configuration").toObject()};
  if (object.contains("bounds"))
  {
    const auto bounds = boundsFromJson(object.value("bounds"));
    if (bounds.is_error())
      return resultError(bounds);
    assertion.bounds = bounds.value();
  }
  return assertion;
}

template <typename Item, typename Parse>
Result<std::vector<Item>, AcceptanceError> parseArray(
  const QJsonObject& object, const QString& key, Parse parse)
{
  const auto value = object.value(key);
  if (!value.isArray())
    return error(
      AcceptanceErrorCode::InvalidJson, "'" + key.toStdString() + "' must be an array");
  auto result = std::vector<Item>{};
  for (const auto& item : value.toArray())
  {
    const auto parsed = parse(item);
    if (parsed.is_error())
      return resultError(parsed);
    result.push_back(parsed.value());
  }
  return result;
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const AcceptanceError& rhs)
{
  return lhs << rhs.message;
}

AcceptanceValidationResult validateAcceptanceProject(const AcceptanceProject& project)
{
  if (project.schemaVersion != AcceptanceSchemaVersion)
  {
    return error(
      AcceptanceErrorCode::UnsupportedSchemaVersion,
      "Unsupported acceptance schema version");
  }

  const auto validRenderMode = [](const std::string& mode) {
    return mode == "textured" || mode == "flat" || mode == "wireframe" || mode == "depth"
           || mode == "surfaceNormals" || mode == "materialId" || mode == "objectId";
  };
  auto viewIds = std::set<std::string>{};
  for (const auto& view : project.views)
  {
    if (!validId(view.id) || !viewIds.insert(view.id).second)
      return error(
        AcceptanceErrorCode::InvalidValue, "View ids must be unique portable ids");
    if (
      view.name.empty() || !validRenderMode(view.renderMode) || view.size.width <= 0
      || view.size.height <= 0 || view.size.width > 16384 || view.size.height > 16384
      || !finite(view.camera.nearPlane) || !finite(view.camera.farPlane)
      || view.camera.nearPlane <= 0.0 || view.camera.farPlane <= view.camera.nearPlane
      || !finite(view.camera.position) || !finite(view.camera.direction)
      || !finite(view.camera.up) || vm::is_zero(view.camera.direction, 0.000001)
      || vm::is_zero(view.camera.up, 0.000001))
      return error(
        AcceptanceErrorCode::InvalidValue,
        "View has an invalid camera, size, or render mode");
    if (
      (view.camera.projection == AcceptanceProjection::Perspective
       && (!view.camera.verticalFov || view.camera.zoom
           || !finite(*view.camera.verticalFov) || *view.camera.verticalFov <= 0.0
           || *view.camera.verticalFov >= 180.0))
      || (view.camera.projection == AcceptanceProjection::Orthographic
          && (view.camera.verticalFov || !view.camera.zoom || !finite(*view.camera.zoom)
              || *view.camera.zoom <= 0.0)))
      return error(
        AcceptanceErrorCode::InvalidValue,
        "View camera projection parameters are invalid");
  }

  auto comparisonIds = std::set<std::string>{};
  for (const auto& comparison : project.comparisons)
  {
    if (!validId(comparison.id) || !comparisonIds.insert(comparison.id).second)
      return error(
        AcceptanceErrorCode::InvalidValue, "Comparison ids must be unique portable ids");
    for (const auto& endpoint :
         {std::cref(comparison.reference), std::cref(comparison.target)})
    {
      if (
        !validRelativePath(endpoint.get().path)
        || !viewIds.contains(endpoint.get().viewId))
        return error(
          AcceptanceErrorCode::BrokenReference,
          "Comparison references a missing view or non-portable document path");
    }
    auto maskIds = std::set<std::string>{};
    for (const auto& mask : comparison.masks)
    {
      if (
        !validId(mask.id) || !maskIds.insert(mask.id).second || !finite(mask.x)
        || !finite(mask.y) || !finite(mask.width) || !finite(mask.height) || mask.x < 0.0
        || mask.y < 0.0 || mask.width <= 0.0 || mask.height <= 0.0
        || mask.x + mask.width > 1.0 || mask.y + mask.height > 1.0)
        return error(AcceptanceErrorCode::InvalidValue, "Comparison has an invalid mask");
    }
    auto itemIds = std::set<std::string>{};
    for (const auto& metric : comparison.metrics)
    {
      if (
        !validId(metric.id) || !itemIds.insert(metric.id).second
        || (metric.maskId && !maskIds.contains(*metric.maskId)))
        return error(
          AcceptanceErrorCode::BrokenReference,
          "Metric has an invalid id or missing mask");
    }
    for (const auto& assertion : comparison.assertions)
    {
      if (
        !validId(assertion.id) || !itemIds.insert(assertion.id).second
        || (assertion.maskId && !maskIds.contains(*assertion.maskId)))
        return error(
          AcceptanceErrorCode::BrokenReference,
          "Assertion has an invalid id or missing mask");
      if (assertion.bounds && (assertion.bounds->min.x() > assertion.bounds->max.x()
                               || assertion.bounds->min.y() > assertion.bounds->max.y()
                               || assertion.bounds->min.z() > assertion.bounds->max.z()))
        return error(
          AcceptanceErrorCode::InvalidValue, "Assertion bounds min must not exceed max");
    }
    if (
      comparison.alignment.type == AcceptanceAlignmentType::Landmarks
      && comparison.alignment.landmarks.size() < 3u)
      return error(
        AcceptanceErrorCode::InvalidValue,
        "Landmark alignment requires at least three landmarks");
    if (
      comparison.alignment.type == AcceptanceAlignmentType::Matrix
      && !std::ranges::all_of(
        comparison.alignment.matrix, [](const auto value) { return finite(value); }))
      return error(AcceptanceErrorCode::InvalidValue, "Matrix alignment must be finite");
    if (
      comparison.alignment.type == AcceptanceAlignmentType::Landmarks
      && !std::ranges::all_of(comparison.alignment.landmarks, [](const auto& landmark) {
           return finite(landmark.reference) && finite(landmark.target);
         }))
      return error(AcceptanceErrorCode::InvalidValue, "Landmarks must be finite");
  }

  auto suiteIds = std::set<std::string>{};
  for (const auto& suite : project.suites)
  {
    if (
      suite.schemaVersion != AcceptanceSchemaVersion || !validId(suite.suiteId)
      || !suiteIds.insert(suite.suiteId).second || suite.name.empty())
      return error(
        AcceptanceErrorCode::InvalidValue, "Suite has an invalid id or schema version");
    auto seen = std::set<std::string>{};
    for (const auto& comparisonId : suite.comparisonIds)
    {
      if (!comparisonIds.contains(comparisonId) || !seen.insert(comparisonId).second)
        return error(
          AcceptanceErrorCode::BrokenReference,
          "Suite references a missing or duplicate comparison");
    }
  }
  return {};
}

QJsonObject acceptanceProjectToJson(const AcceptanceProject& project)
{
  auto canonical = project;
  sortById(canonical.views);
  sortById(canonical.comparisons);
  std::ranges::sort(canonical.suites, {}, &AcceptanceSuite::suiteId);
  auto views = QJsonArray{};
  for (const auto& view : canonical.views)
    views.push_back(namedViewToJson(view));
  auto comparisons = QJsonArray{};
  for (auto comparison : canonical.comparisons)
  {
    sortById(comparison.masks);
    sortById(comparison.metrics);
    sortById(comparison.assertions);
    auto masks = QJsonArray{};
    for (const auto& mask : comparison.masks)
      masks.push_back(maskToJson(mask));
    auto metrics = QJsonArray{};
    for (const auto& metric : comparison.metrics)
      metrics.push_back(metricToJson(metric));
    auto assertions = QJsonArray{};
    for (const auto& assertion : comparison.assertions)
      assertions.push_back(assertionToJson(assertion));
    comparisons.push_back(
      QJsonObject{
        {"id", QString::fromStdString(comparison.id)},
        {"name", QString::fromStdString(comparison.name)},
        {"reference", documentReferenceToJson(comparison.reference)},
        {"target", documentReferenceToJson(comparison.target)},
        {"alignment", alignmentToJson(comparison.alignment)},
        {"masks", masks},
        {"metrics", metrics},
        {"assertions", assertions},
      });
  }
  auto suites = QJsonArray{};
  for (auto suite : canonical.suites)
  {
    std::ranges::sort(suite.comparisonIds);
    auto comparisonIds = QJsonArray{};
    for (const auto& id : suite.comparisonIds)
      comparisonIds.push_back(QString::fromStdString(id));
    suites.push_back(
      QJsonObject{
        {"schemaVersion", static_cast<qint64>(suite.schemaVersion)},
        {"suiteId", QString::fromStdString(suite.suiteId)},
        {"name", QString::fromStdString(suite.name)},
        {"comparisons", comparisonIds}});
  }
  return {
    {"schemaVersion", static_cast<qint64>(canonical.schemaVersion)},
    {"revision", static_cast<qint64>(canonical.revision)},
    {"views", views},
    {"comparisons", comparisons},
    {"suites", suites}};
}

AcceptanceProjectResult acceptanceProjectFromJson(const QJsonObject& json)
{
  const auto schemaVersion = json.value("schemaVersion");
  const auto revision = json.value("revision");
  if (
    !schemaVersion.isDouble() || !revision.isDouble()
    || schemaVersion.toDouble() != AcceptanceSchemaVersion || revision.toDouble() < 0.0
    || revision.toDouble() != std::floor(revision.toDouble()))
    return error(
      AcceptanceErrorCode::UnsupportedSchemaVersion,
      "Unsupported or missing acceptance schema version/revision");
  auto project = AcceptanceProject{};
  project.schemaVersion = static_cast<size_t>(schemaVersion.toDouble());
  project.revision = static_cast<size_t>(revision.toDouble());
  const auto views = parseArray<AcceptanceNamedView>(json, "views", namedViewFromJson);
  if (views.is_error())
    return resultError(views);
  project.views = views.value();
  const auto comparisonValues = json.value("comparisons");
  if (!comparisonValues.isArray())
    return error(AcceptanceErrorCode::InvalidJson, "'comparisons' must be an array");
  for (const auto& value : comparisonValues.toArray())
  {
    if (!value.isObject())
      return error(AcceptanceErrorCode::InvalidJson, "Each comparison must be an object");
    const auto object = value.toObject();
    if (!object.value("id").isString() || !object.value("name").isString())
      return error(AcceptanceErrorCode::InvalidJson, "Comparison needs id and name");
    const auto reference = documentReferenceFromJson(object.value("reference"));
    const auto target = documentReferenceFromJson(object.value("target"));
    const auto alignment = alignmentFromJson(object.value("alignment"));
    const auto masks = parseArray<AcceptanceMask>(object, "masks", maskFromJson);
    const auto metrics = parseArray<AcceptanceMetric>(object, "metrics", metricFromJson);
    const auto assertions =
      parseArray<AcceptanceAssertion>(object, "assertions", assertionFromJson);
    if (reference.is_error())
      return resultError(reference);
    if (target.is_error())
      return resultError(target);
    if (alignment.is_error())
      return resultError(alignment);
    if (masks.is_error())
      return resultError(masks);
    if (metrics.is_error())
      return resultError(metrics);
    if (assertions.is_error())
      return resultError(assertions);
    project.comparisons.push_back(
      {object.value("id").toString().toStdString(),
       object.value("name").toString().toStdString(),
       reference.value(),
       target.value(),
       alignment.value(),
       masks.value(),
       metrics.value(),
       assertions.value()});
  }
  const auto suiteValues = json.value("suites");
  if (!suiteValues.isArray())
    return error(AcceptanceErrorCode::InvalidJson, "'suites' must be an array");
  for (const auto& value : suiteValues.toArray())
  {
    if (!value.isObject())
      return error(AcceptanceErrorCode::InvalidJson, "Each suite must be an object");
    const auto object = value.toObject();
    if (
      !object.value("schemaVersion").isDouble() || !object.value("suiteId").isString()
      || !object.value("name").isString() || !object.value("comparisons").isArray())
      return error(AcceptanceErrorCode::InvalidJson, "Suite fields are invalid");
    auto ids = std::vector<std::string>{};
    for (const auto& id : object.value("comparisons").toArray())
    {
      if (!id.isString())
        return error(
          AcceptanceErrorCode::InvalidJson, "Suite comparison ids must be strings");
      ids.push_back(id.toString().toStdString());
    }
    project.suites.push_back(
      {static_cast<size_t>(object.value("schemaVersion").toDouble()),
       object.value("suiteId").toString().toStdString(),
       object.value("name").toString().toStdString(),
       std::move(ids)});
  }
  const auto validation = validateAcceptanceProject(project);
  if (validation.is_error())
    return resultError(validation);
  return project;
}

Result<std::filesystem::path, AcceptanceError> makePortableAcceptancePath(
  const std::filesystem::path& projectPath, const std::filesystem::path& documentPath)
{
  if (projectPath.empty() || documentPath.empty())
    return error(
      AcceptanceErrorCode::InvalidValue, "Project and document paths are required");
  const auto relative = documentPath.lexically_relative(projectPath.parent_path());
  if (!validRelativePath(relative))
    return error(
      AcceptanceErrorCode::InvalidValue,
      "Document path cannot be made portable relative to the project path");
  return relative.lexically_normal();
}

Result<std::filesystem::path, AcceptanceError> resolveAcceptancePath(
  const std::filesystem::path& projectPath, const std::filesystem::path& portablePath)
{
  if (projectPath.empty() || !validRelativePath(portablePath))
    return error(
      AcceptanceErrorCode::InvalidValue,
      "Project path and relative document path are required");
  return (projectPath.parent_path() / portablePath).lexically_normal();
}

} // namespace tb::ui
