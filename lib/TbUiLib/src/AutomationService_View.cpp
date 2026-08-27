/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QDir>
#include <QUuid>

#include "AutomationJson.h"
#include "mdl/Map.h"
#include "ui/AutomationService.h"
#include "ui/MapDocument.h"
#include "ui/MapViewBase.h"
#include "ui/MapWindow.h"
#include "ui/QPathUtils.h"
#include "ui/SystemPaths.h"

namespace tb::ui
{

JsonRpcResponse AutomationService::handleViewRequest(
  const QString& method, const QJsonObject& params)
{
  if (
    method != "context.capture" && method != "view.pick" && method != "view.camera.set"
    && method != "view.frame")
  {
    return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
  }

  auto* window = findWindow(params);
  if (window == nullptr)
  {
    return automation::invalidParams("Unknown documentId or no map document is open");
  }
  auto* view = window->currentMapViewBase();

  if (method == "context.capture")
  {
    auto result = automation::contextToJson(view->captureContext());
    result.insert("documentId", documentId(*window));

    if (params.value("screenshot").toBool(true))
    {
      const auto outputDirectory = SystemPaths::tempDirectory() / "TrenchBroomAutomation";
      QDir{}.mkpath(pathAsQString(outputDirectory));
      const auto outputPath =
        outputDirectory
        / ("context-" + QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString()
           + ".png");
      if (!view->captureImage().save(pathAsQString(outputPath), "PNG"))
      {
        return JsonRpcResponse::error(
          {JsonRpcError::InternalError, "Could not capture the map view"});
      }
      result.insert("screenshotPath", pathAsQString(outputPath));
    }
    return JsonRpcResponse::success(result);
  }

  if (method == "view.pick")
  {
    if (!params.value("x").isDouble() || !params.value("y").isDouble())
    {
      return automation::invalidParams("x and y must be numbers in view-local pixels");
    }

    auto result = automation::pickToJson(view->pickAt(
      static_cast<float>(params.value("x").toDouble()),
      static_cast<float>(params.value("y").toDouble())));
    result.insert("documentId", documentId(*window));
    result.insert(
      "revision", static_cast<qint64>(window->document().map().modificationCount()));
    return JsonRpcResponse::success(result);
  }

  if (method == "view.camera.set")
  {
    const auto position = automation::vec3FromJson(params.value("position"));
    const auto direction = automation::vec3FromJson(params.value("direction"));
    const auto up = automation::vec3FromJson(params.value("up"));
    if (
      !position || !direction || !up
      || !view->setCameraState(
        vm::vec3f{*position}, vm::vec3f{*direction}, vm::vec3f{*up}))
    {
      return automation::invalidParams(
        "documentId and valid position, direction, and up vectors are required");
    }
    return JsonRpcResponse::success(
      automation::contextToJson(view->captureContext()).value("camera"));
  }

  if (method == "view.frame")
  {
    if (params.value("bounds").isObject())
    {
      const auto bounds = params.value("bounds").toObject();
      const auto min = automation::vec3FromJson(bounds.value("min"));
      const auto max = automation::vec3FromJson(bounds.value("max"));
      if (!min || !max || !vm::bbox3d::is_valid(*min, *max))
      {
        return automation::invalidParams("bounds must contain valid min and max vectors");
      }
      view->frameBounds(vm::bbox3d{*min, *max});
    }
    else if (!view->frameSelection())
    {
      return automation::invalidParams("The document has no selected nodes to frame");
    }
    return JsonRpcResponse::success(
      automation::contextToJson(view->captureContext()).value("camera"));
  }

  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

} // namespace tb::ui
