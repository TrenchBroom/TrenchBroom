/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationRenderRequest.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include "vm/util.h"

#include <cmath>

namespace tb::ui::automation
{
namespace
{

std::optional<vm::vec3d> vec3FromJson(const QJsonValue& value)
{
  if (!value.isArray())
  {
    return std::nullopt;
  }
  const auto values = value.toArray();
  if (
    values.size() != 3 || !values[0].isDouble() || !values[1].isDouble()
    || !values[2].isDouble())
  {
    return std::nullopt;
  }

  const auto result =
    vm::vec3d{values[0].toDouble(), values[1].toDouble(), values[2].toDouble()};
  return std::isfinite(result.x()) && std::isfinite(result.y())
             && std::isfinite(result.z())
           ? std::optional{result}
           : std::nullopt;
}

QJsonArray vec3ToJson(const vm::vec3d& value)
{
  return {value.x(), value.y(), value.z()};
}

std::optional<AutomationProjection> projectionFromJson(const QJsonValue& value)
{
  if (!value.isString())
  {
    return std::nullopt;
  }
  const auto projection = value.toString();
  if (projection == "perspective")
  {
    return AutomationProjection::Perspective;
  }
  if (projection == "orthographic")
  {
    return AutomationProjection::Orthographic;
  }
  return std::nullopt;
}

QString projectionToJson(const AutomationProjection projection)
{
  return projection == AutomationProjection::Perspective ? "perspective" : "orthographic";
}

std::optional<AutomationRenderMode> renderModeFromJson(const QJsonValue& value)
{
  return value.isString() && value.toString() == "textured"
           ? std::optional{AutomationRenderMode::Textured}
           : std::nullopt;
}

QString renderModeToJson(const AutomationRenderMode renderMode)
{
  switch (renderMode)
  {
  case AutomationRenderMode::Textured:
    return "textured";
  }
  return {};
}

std::optional<AutomationImageSize> imageSizeFromJson(const QJsonValue& value)
{
  if (!value.isArray())
  {
    return std::nullopt;
  }
  const auto values = value.toArray();
  if (values.size() != 2 || !values[0].isDouble() || !values[1].isDouble())
  {
    return std::nullopt;
  }
  const auto widthAsDouble = values[0].toDouble();
  const auto heightAsDouble = values[1].toDouble();
  if (
    !std::isfinite(widthAsDouble) || !std::isfinite(heightAsDouble)
    || std::floor(widthAsDouble) != widthAsDouble
    || std::floor(heightAsDouble) != heightAsDouble
    || widthAsDouble < AutomationMinImageDimension
    || widthAsDouble > AutomationMaxImageDimension
    || heightAsDouble < AutomationMinImageDimension
    || heightAsDouble > AutomationMaxImageDimension)
  {
    return std::nullopt;
  }
  const auto width = static_cast<int>(widthAsDouble);
  const auto height = static_cast<int>(heightAsDouble);
  const auto size = AutomationImageSize{width, height};
  return isValidImageSize(size) ? std::optional{size} : std::nullopt;
}

QJsonArray imageSizeToJson(const AutomationImageSize& size)
{
  return {size.width, size.height};
}

std::optional<AutomationOverlayOptions> overlaysFromJson(const QJsonValue& value)
{
  if (!value.isObject())
  {
    return std::nullopt;
  }
  const auto object = value.toObject();
  const auto valid = [](const QJsonObject& candidate, const QString& key) {
    return !candidate.contains(key) || candidate.value(key).isBool();
  };
  if (
    !valid(object, "brushEdges") || !valid(object, "selection") || !valid(object, "grid"))
  {
    return std::nullopt;
  }
  return AutomationOverlayOptions{
    object.value("brushEdges").toBool(false),
    object.value("selection").toBool(false),
    object.value("grid").toBool(false),
  };
}

std::optional<AutomationOutputOptions> outputsFromJson(const QJsonValue& value)
{
  if (value.isUndefined())
  {
    return AutomationOutputOptions{};
  }
  if (!value.isObject())
  {
    return std::nullopt;
  }
  const auto object = value.toObject();
  if (object.contains("depth") && !object.value("depth").isBool())
  {
    return std::nullopt;
  }
  return AutomationOutputOptions{object.value("depth").toBool(false)};
}

QJsonObject overlaysToJson(const AutomationOverlayOptions& overlays)
{
  return {
    {"brushEdges", overlays.brushEdges},
    {"selection", overlays.selection},
    {"grid", overlays.grid},
  };
}

QJsonObject outputsToJson(const AutomationOutputOptions& outputs)
{
  return {{"depth", outputs.depth}};
}

std::optional<std::optional<AutomationScenePreviewOptions>> scenePreviewFromJson(
  const QJsonValue& value)
{
  if (value.isUndefined())
  {
    return std::optional<AutomationScenePreviewOptions>{};
  }
  if (!value.isObject())
  {
    return std::nullopt;
  }
  const auto object = value.toObject();
  const auto visionValue = object.value("vision");
  const auto timeValue = object.value("timeOfDay");
  const auto lightsValue = object.value("entityLights");
  if (
    !visionValue.isString() || !timeValue.isDouble()
    || (!lightsValue.isUndefined() && !lightsValue.isBool()))
  {
    return std::nullopt;
  }

  const auto visionName = visionValue.toString();
  const auto vision =
    visionName == "human"         ? std::optional{render::PlayerVision::Human}
    : visionName == "infravision" ? std::optional{render::PlayerVision::Infravision}
    : visionName == "ultravision" ? std::optional{render::PlayerVision::Ultravision}
                                  : std::nullopt;
  const auto timeOfDay = timeValue.toDouble();
  if (!vision || !std::isfinite(timeOfDay) || timeOfDay < 0.0 || timeOfDay >= 24.0)
  {
    return std::nullopt;
  }
  return AutomationScenePreviewOptions{*vision, timeOfDay, lightsValue.toBool(true)};
}

QJsonObject scenePreviewToJson(const AutomationScenePreviewOptions& options)
{
  const auto vision = [&]() -> QString {
    switch (options.vision)
    {
    case render::PlayerVision::Human:
      return "human";
    case render::PlayerVision::Infravision:
      return "infravision";
    case render::PlayerVision::Ultravision:
      return "ultravision";
    }
    return "human";
  }();
  return {
    {"vision", vision},
    {"timeOfDay", options.timeOfDay},
    {"entityLights", options.entityLights},
  };
}

std::optional<double> finitePositiveDouble(const QJsonValue& value)
{
  if (!value.isDouble())
  {
    return std::nullopt;
  }
  const auto result = value.toDouble();
  return std::isfinite(result) && result > 0.0 ? std::optional{result} : std::nullopt;
}

} // namespace

bool isValidImageSize(const AutomationImageSize& size)
{
  return size.width >= AutomationMinImageDimension
         && size.width <= AutomationMaxImageDimension
         && size.height >= AutomationMinImageDimension
         && size.height <= AutomationMaxImageDimension;
}

std::optional<AutomationRenderRequest> renderRequestFromJson(const QJsonObject& json)
{
  const auto cameraObject = json.value("camera");
  const auto size = imageSizeFromJson(json.value("size"));
  const auto renderMode = renderModeFromJson(json.value("renderMode"));
  const auto overlays = overlaysFromJson(json.value("overlays"));
  const auto outputs = outputsFromJson(json.value("outputs"));
  const auto scenePreview = scenePreviewFromJson(json.value("scenePreview"));
  if (
    !cameraObject.isObject() || !size || !renderMode || !overlays || !outputs
    || !scenePreview)
  {
    return std::nullopt;
  }

  const auto cameraJson = cameraObject.toObject();
  const auto projection = projectionFromJson(cameraJson.value("projection"));
  const auto position = vec3FromJson(cameraJson.value("position"));
  const auto direction = vec3FromJson(cameraJson.value("direction"));
  const auto up = vec3FromJson(cameraJson.value("up"));
  const auto nearPlane = finitePositiveDouble(cameraJson.value("near"));
  const auto farPlane = finitePositiveDouble(cameraJson.value("far"));
  if (
    !projection || !position || !direction || !up || !nearPlane || !farPlane
    || *farPlane <= *nearPlane || vm::is_zero(*direction, vm::Cd::almost_zero())
    || vm::is_zero(*up, vm::Cd::almost_zero()))
  {
    return std::nullopt;
  }

  const auto normalizedDirection = vm::normalize(*direction);
  const auto orthogonalUp = *up - vm::dot(*up, normalizedDirection) * normalizedDirection;
  if (vm::is_zero(orthogonalUp, vm::Cd::almost_zero()))
  {
    return std::nullopt;
  }

  auto camera = AutomationCamera{
    *projection,
    *position,
    normalizedDirection,
    vm::normalize(orthogonalUp),
    std::nullopt,
    std::nullopt,
    *nearPlane,
    *farPlane,
  };

  if (*projection == AutomationProjection::Perspective)
  {
    const auto verticalFov = finitePositiveDouble(cameraJson.value("verticalFov"));
    if (!verticalFov || *verticalFov >= 180.0 || cameraJson.contains("zoom"))
    {
      return std::nullopt;
    }
    camera.verticalFov = *verticalFov;
  }
  else
  {
    const auto zoom = finitePositiveDouble(cameraJson.value("zoom"));
    if (!zoom || cameraJson.contains("verticalFov"))
    {
      return std::nullopt;
    }
    camera.zoom = *zoom;
  }

  return AutomationRenderRequest{
    camera, *size, *renderMode, *overlays, *outputs, *scenePreview};
}

QJsonObject renderRequestToJson(const AutomationRenderRequest& request)
{
  auto camera = QJsonObject{
    {"projection", projectionToJson(request.camera.projection)},
    {"position", vec3ToJson(request.camera.position)},
    {"direction", vec3ToJson(request.camera.direction)},
    {"up", vec3ToJson(request.camera.up)},
    {"near", request.camera.nearPlane},
    {"far", request.camera.farPlane},
  };
  if (request.camera.projection == AutomationProjection::Perspective)
  {
    if (request.camera.verticalFov)
    {
      camera.insert("verticalFov", *request.camera.verticalFov);
    }
  }
  else if (request.camera.zoom)
  {
    camera.insert("zoom", *request.camera.zoom);
  }
  auto result = QJsonObject{
    {"camera", camera},
    {"size", imageSizeToJson(request.size)},
    {"renderMode", renderModeToJson(request.renderMode)},
    {"overlays", overlaysToJson(request.overlays)},
    {"outputs", outputsToJson(request.outputs)},
  };
  if (request.scenePreview)
  {
    result.insert("scenePreview", scenePreviewToJson(*request.scenePreview));
  }
  return result;
}

QJsonObject renderOutputToJson(const AutomationRenderOutput& output)
{
  auto result = QJsonObject{
    {"imagePath", QString::fromStdString(output.imagePath.string())},
    {"size", imageSizeToJson(output.size)},
    {"captureMode", "offscreen"},
  };
  if (output.depth)
  {
    result.insert(
      "depth",
      QJsonObject{
        {"path", QString::fromStdString(output.depth->path.string())},
        {"size", imageSizeToJson(output.depth->size)},
        {"format", AutomationDepthFormat},
        {"noHit", "infinity"},
      });
  }
  return result;
}

} // namespace tb::ui::automation
