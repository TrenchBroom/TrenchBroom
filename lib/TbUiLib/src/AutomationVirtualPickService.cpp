/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationVirtualPickService.h"

#include "gl/OrthographicCamera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/Map.h"
#include "ui/MapDocument.h"

#include "vm/util.h"

#include <cmath>
#include <exception>
#include <type_traits>
#include <utility>

namespace tb::ui
{
namespace
{

AutomationVirtualPickResult failure(
  const AutomationVirtualPickError error,
  const QString& message,
  const automation::AutomationRenderRequest& request,
  const size_t revision)
{
  return {error, message, request, {}, revision};
}

bool validRequest(const automation::AutomationRenderRequest& request)
{
  if (
    !automation::isValidImageSize(request.size) || request.camera.nearPlane <= 0.0
    || request.camera.farPlane <= request.camera.nearPlane
    || vm::is_zero(request.camera.direction, vm::Cd::almost_zero())
    || vm::is_zero(request.camera.up, vm::Cd::almost_zero()))
  {
    return false;
  }
  if (request.camera.projection == automation::AutomationProjection::Perspective)
  {
    return request.camera.verticalFov && *request.camera.verticalFov > 0.0
           && *request.camera.verticalFov < 180.0 && !request.camera.zoom;
  }
  return request.camera.zoom && *request.camera.zoom > 0.0 && !request.camera.verticalFov;
}

bool validPixel(
  const automation::AutomationRenderRequest& request, const double x, const double y)
{
  return std::isfinite(x) && std::isfinite(y) && x >= 0.0 && y >= 0.0
         && x < static_cast<double>(request.size.width)
         && y < static_cast<double>(request.size.height);
}

template <typename Camera>
void configureCamera(Camera& camera, const automation::AutomationRenderRequest& request)
{
  camera.setNearPlane(static_cast<float>(request.camera.nearPlane));
  camera.setFarPlane(static_cast<float>(request.camera.farPlane));
  camera.setViewport({0, 0, request.size.width, request.size.height});
  camera.moveTo(vm::vec3f{request.camera.position});
  camera.setDirection(vm::vec3f{request.camera.direction}, vm::vec3f{request.camera.up});
}

template <typename Camera>
MapViewPickResult pickMap(
  mdl::Map& map,
  Camera& camera,
  const automation::AutomationRenderRequest& request,
  const float x,
  const float y)
{
  configureCamera(camera, request);
  if constexpr (std::is_same_v<Camera, gl::PerspectiveCamera>)
  {
    camera.setFov(static_cast<float>(*request.camera.verticalFov));
  }
  else
  {
    camera.setZoom(static_cast<float>(*request.camera.zoom));
  }
  return pickMapView(map, camera, x, y);
}

} // namespace

AutomationVirtualPickResult AutomationVirtualPickService::pick(
  MapDocument& document,
  const automation::AutomationRenderRequest& request,
  const double x,
  const double y) const
{
  const auto revision = document.map().modificationCount();
  if (!validRequest(request))
  {
    return failure(
      AutomationVirtualPickError::InvalidRequest,
      "Invalid render request",
      request,
      revision);
  }
  if (!validPixel(request, x, y))
  {
    return failure(
      AutomationVirtualPickError::InvalidPixel,
      "Pixel coordinates must be finite and within the requested image",
      request,
      revision);
  }

  try
  {
    auto result = MapViewPickResult{};
    if (request.camera.projection == automation::AutomationProjection::Perspective)
    {
      auto camera = gl::PerspectiveCamera{};
      result = pickMap(
        document.map(), camera, request, static_cast<float>(x), static_cast<float>(y));
    }
    else
    {
      auto camera = gl::OrthographicCamera{};
      result = pickMap(
        document.map(), camera, request, static_cast<float>(x), static_cast<float>(y));
    }

    if (document.map().modificationCount() != revision)
    {
      return failure(
        AutomationVirtualPickError::DocumentChanged,
        "Document changed while picking",
        request,
        revision);
    }
    return {AutomationVirtualPickError::None, {}, request, std::move(result), revision};
  }
  catch (const std::exception& error)
  {
    return failure(
      AutomationVirtualPickError::PickFailed,
      QString::fromUtf8(error.what()),
      request,
      revision);
  }
}

} // namespace tb::ui
