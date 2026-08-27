/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationRenderRpcAdapter.h"

#include "AutomationJson.h"
#include "mdl/Map.h"
#include "ui/AppController.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/AutomationVirtualPickService.h"
#include "ui/AutomationVirtualRenderService.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/QPathUtils.h"

#include <cmath>
#include <cstddef>

namespace tb::ui
{
namespace
{

JsonRpcResponse invalidRenderRequest(const QString& message)
{
  return automation::invalidParams(message);
}

JsonRpcResponse renderFailure(const AutomationVirtualRenderResult& result)
{
  switch (result.error)
  {
  case AutomationVirtualRenderError::InvalidRequest:
    return invalidRenderRequest(result.message);
  case AutomationVirtualRenderError::DocumentChanged:
    return automation::revisionConflict(result.revision);
  case AutomationVirtualRenderError::ResourceNotReady:
    return JsonRpcResponse::error(
      {-32002,
       "Render resources not ready",
       QJsonObject{
         {"retryable", true}, {"revision", static_cast<qint64>(result.revision)}}});
  case AutomationVirtualRenderError::ContextUnavailable:
  case AutomationVirtualRenderError::RenderFailed:
  case AutomationVirtualRenderError::OutputUnavailable:
    return JsonRpcResponse::error(
      {JsonRpcError::InternalError,
       result.message.isEmpty() ? "Could not capture the virtual render"
                                : result.message});
  case AutomationVirtualRenderError::None:
    break;
  }
  return JsonRpcResponse::error(
    {JsonRpcError::InternalError, "Could not capture the virtual render"});
}

JsonRpcResponse pickFailure(const AutomationVirtualPickResult& result)
{
  switch (result.error)
  {
  case AutomationVirtualPickError::InvalidRequest:
  case AutomationVirtualPickError::InvalidPixel:
    return invalidRenderRequest(result.message);
  case AutomationVirtualPickError::DocumentChanged:
    return automation::revisionConflict(result.revision);
  case AutomationVirtualPickError::PickFailed:
    return JsonRpcResponse::error(
      {JsonRpcError::InternalError,
       result.message.isEmpty() ? "Could not perform the virtual pick" : result.message});
  case AutomationVirtualPickError::None:
    break;
  }
  return JsonRpcResponse::error(
    {JsonRpcError::InternalError, "Could not perform the virtual pick"});
}

QJsonObject renderResult(
  const QString& documentId,
  const automation::AutomationRenderRequest& request,
  const size_t revision)
{
  auto result = automation::renderRequestToJson(request);
  result.insert("documentId", documentId);
  result.insert("revision", static_cast<qint64>(revision));
  return result;
}

} // namespace

AutomationRenderRpcAdapter::AutomationRenderRpcAdapter(AppController& appController)
  : m_appController{appController}
{
}

JsonRpcResponse AutomationRenderRpcAdapter::handle(
  const QString& method,
  const QJsonObject& params,
  const AutomationDocumentRegistry& documentRegistry) const
{
  if (method != "render.capture" && method != "render.context" && method != "render.pick")
  {
    return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
  }

  const auto documentId = params.value("documentId").toString();
  if (documentId.isEmpty())
  {
    return invalidRenderRequest("documentId is required");
  }
  auto* window = documentRegistry.findWindow(documentId);
  if (window == nullptr)
  {
    return invalidRenderRequest("Unknown documentId");
  }

  const auto request = automation::renderRequestFromJson(params);
  if (!request)
  {
    return invalidRenderRequest(
      "A valid virtual camera, image size, render mode, and overlays are required");
  }

  auto& document = window->document();
  if (method == "render.context")
  {
    auto result = renderResult(documentId, *request, document.map().modificationCount());
    result.insert("path", pathAsQString(document.map().path()));
    result.insert("filename", QString::fromStdString(document.map().filename()));
    result.insert("modified", document.map().modified());
    return JsonRpcResponse::success(result);
  }

  if (method == "render.pick")
  {
    const auto xValue = params.value("x");
    const auto yValue = params.value("y");
    if (!xValue.isDouble() || !yValue.isDouble())
    {
      return invalidRenderRequest("x and y pixel coordinates are required");
    }
    const auto x = xValue.toDouble();
    const auto y = yValue.toDouble();
    if (!std::isfinite(x) || !std::isfinite(y))
    {
      return invalidRenderRequest("x and y pixel coordinates must be finite");
    }

    const auto picker = AutomationVirtualPickService{};
    const auto pick = picker.pick(document, *request, x, y);
    if (!pick)
    {
      return pickFailure(pick);
    }

    auto result = renderResult(documentId, pick.request, pick.revision);
    result.insert("pixel", QJsonObject{{"x", x}, {"y", y}});
    const auto pickJson = automation::pickToJson(pick.pick);
    for (auto it = pickJson.begin(); it != pickJson.end(); ++it)
    {
      result.insert(it.key(), it.value());
    }
    return JsonRpcResponse::success(result);
  }

  auto renderer = AutomationVirtualRenderService{m_appController};
  const auto capture = renderer.capture(document, *request);
  if (!capture)
  {
    return renderFailure(capture);
  }

  auto result = renderResult(documentId, capture.request, capture.revision);
  const auto output = automation::renderOutputToJson(capture.output);
  for (auto it = output.begin(); it != output.end(); ++it)
  {
    result.insert(it.key(), it.value());
  }
  return JsonRpcResponse::success(result);
}

} // namespace tb::ui
