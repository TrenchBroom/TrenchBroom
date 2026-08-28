/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationService.h"

#include "ui/AcceptanceAutomationService.h"
#include "ui/AcceptanceViewStore.h"
#include "ui/AppController.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/QPathUtils.h"

#include <variant>

namespace tb::ui
{

AutomationService::AutomationService(AppController& appController, QObject* parent)
  : QObject{parent}
  , m_appController{appController}
  , m_server{this, this}
  , m_documentRegistry{}
  , m_viewRegistry{}
  , m_cameraDocumentResolver{m_documentRegistry}
  , m_cameraHandles{m_cameraDocumentResolver}
  , m_cameraCapture{appController, m_documentRegistry}
  , m_cameraService{m_cameraHandles, m_cameraCapture}
  , m_workspaceManager{appController}
  , m_renderRpcAdapter{appController}
  , m_acceptanceCapture{appController, m_documentRegistry}
  , m_acceptanceMapResolver{
      m_documentRegistry,
      [this](const std::string_view documentId) {
        auto* document = m_acceptanceCapture.findDocument(documentId);
        return document != nullptr ? &document->map() : nullptr;
      }}
  , m_acceptanceGeometry{m_acceptanceMapResolver}
{
  start();
}

AutomationService::~AutomationService()
{
  m_server.close();
  removeDiscoveryFile();
}

bool AutomationService::isListening() const
{
  return m_server.isListening();
}

const QString& AutomationService::serverName() const
{
  return m_serverName;
}

const std::filesystem::path& AutomationService::discoveryPath() const
{
  return m_discoveryPath;
}

JsonRpcResponse AutomationService::handleRequest(
  const QString& method, const QJsonValue& paramsValue)
{
  const auto params = paramsValue.isObject() ? paramsValue.toObject() : QJsonObject{};

  if (
    method.startsWith("system.") || method.startsWith("documents.")
    || method.startsWith("reference.") || method == "document.save"
    || method == "document.reload")
  {
    return handleDocumentRequest(method, params);
  }
  if (
    method.startsWith("context.") || method.startsWith("view.") || method == "views.list")
  {
    return handleViewRequest(method, params);
  }
  if (method.startsWith("render."))
  {
    return m_renderRpcAdapter.handle(method, params, m_documentRegistry);
  }
  if (method.startsWith("cameras."))
  {
    const auto result = m_cameraService.handle(method, params);
    if (result.is_success())
    {
      return JsonRpcResponse::success(result.value());
    }
    const auto failure = std::get<AutomationCameraHandleError>(result.error());
    switch (failure.code)
    {
    case AutomationCameraHandleErrorCode::InvalidRequest:
      return JsonRpcResponse::error(
        {JsonRpcError::InvalidParams, QString::fromStdString(failure.message)});
    case AutomationCameraHandleErrorCode::MethodNotFound:
      return JsonRpcResponse::error(
        {JsonRpcError::MethodNotFound, QString::fromStdString(failure.message)});
    case AutomationCameraHandleErrorCode::UnknownDocument:
    case AutomationCameraHandleErrorCode::UnknownHandle:
      return JsonRpcResponse::error(
        {-32032,
         "Camera target is unavailable",
         QString::fromStdString(failure.message)});
    case AutomationCameraHandleErrorCode::CaptureFailed:
      return JsonRpcResponse::error(
        {-32033, "Camera capture failed", QString::fromStdString(failure.message)});
    }
    return JsonRpcResponse::error(
      {JsonRpcError::InternalError, "Unknown camera automation error"});
  }
  if (method.startsWith("document.") || method.startsWith("nodes."))
  {
    return handleNodeRequest(method, params);
  }
  if (method.startsWith("brushes."))
  {
    return handleBrushRequest(method, params);
  }
  if (method.startsWith("faces."))
  {
    return handleFaceRequest(method, params);
  }
  if (method.startsWith("geometry."))
  {
    return handleGeometryRequest(method, params);
  }
  if (method.startsWith("workspace."))
  {
    return handleWorkspaceRequest(method, params);
  }
  if (method.startsWith("acceptance."))
  {
    return handleAcceptanceRequest(method, params);
  }
  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

JsonRpcResponse AutomationService::handleAcceptanceRequest(
  const QString& method, const QJsonObject& params)
{
  const auto projectPathValue = params.value("projectPath");
  if (!projectPathValue.isString() || projectPathValue.toString().isEmpty())
  {
    return JsonRpcResponse::error(
      {JsonRpcError::InvalidParams,
       "Acceptance operations require an explicit projectPath"});
  }

  // Context roles are path-based and must resolve an already open document even when
  // this is the first automation request of the session. Registration is identity-only:
  // it does not activate, focus, or otherwise mutate any map window.
  for (auto* window : m_appController.mapWindowManager().mapWindows())
  {
    if (window != nullptr)
      documentId(*window);
  }

  auto store = AcceptanceViewStore{pathFromQString(projectPathValue.toString())};
  auto service = AcceptanceAutomationService{
    store,
    m_acceptanceCapture,
    m_acceptanceGeometry,
    m_acceptanceCapture,
    m_acceptanceCapture};
  const auto result = service.handle(method, params);
  if (result.is_success())
  {
    return JsonRpcResponse::success(result.value());
  }

  const auto failure = std::get<AcceptanceAutomationError>(result.error());
  switch (failure.code)
  {
  case AcceptanceAutomationErrorCode::MethodNotFound:
    return JsonRpcResponse::error(
      {JsonRpcError::MethodNotFound, QString::fromStdString(failure.message)});
  case AcceptanceAutomationErrorCode::InvalidParameters:
    return JsonRpcResponse::error(
      {JsonRpcError::InvalidParams, QString::fromStdString(failure.message)});
  case AcceptanceAutomationErrorCode::StoreFailed:
    return JsonRpcResponse::error(
      {-32030,
       "Acceptance store operation failed",
       QString::fromStdString(failure.message)});
  case AcceptanceAutomationErrorCode::CaptureFailed:
    return JsonRpcResponse::error(
      {-32031, "Acceptance capture failed", QString::fromStdString(failure.message)});
  case AcceptanceAutomationErrorCode::GeometryFailed:
    return JsonRpcResponse::error(
      {-32034,
       "Acceptance geometry comparison failed",
       QString::fromStdString(failure.message)});
  case AcceptanceAutomationErrorCode::EvidenceFailed:
    return JsonRpcResponse::error(
      {-32035,
       "Acceptance evidence run failed",
       QString::fromStdString(failure.message)});
  }
  return JsonRpcResponse::error(
    {JsonRpcError::InternalError, "Unknown acceptance error"});
}

QString AutomationService::documentId(const MapWindow& window) const
{
  return m_documentRegistry.registerDocument(const_cast<MapWindow&>(window));
}

MapWindow* AutomationService::findWindow(const QJsonObject& params) const
{
  const auto requestedId = params.value("documentId").toString();
  if (requestedId.isEmpty())
  {
    return m_appController.mapWindowManager().topMapWindow();
  }

  return m_documentRegistry.findWindow(requestedId);
}

void AutomationService::registerMapViews(const MapWindow& window) const
{
  const auto id = documentId(window);
  for (auto* view : window.mapViewBases())
  {
    m_viewRegistry.registerView(id, *view);
  }
}

std::optional<AutomationResolvedMapView> AutomationService::resolveMapView(
  const QJsonObject& params, QString* error) const
{
  const auto requestedDocumentId = params.value("documentId").toString();
  const auto requestedViewId = params.value("viewId").toString();
  if (requestedDocumentId.isEmpty() || requestedViewId.isEmpty())
  {
    if (error != nullptr)
    {
      *error = "documentId and viewId are required";
    }
    return std::nullopt;
  }

  auto* window = m_documentRegistry.findWindow(requestedDocumentId);
  if (window == nullptr)
  {
    if (error != nullptr)
    {
      *error = "Unknown documentId";
    }
    return std::nullopt;
  }

  const auto resolvedDocumentId = documentId(*window);
  registerMapViews(*window);
  auto* view = m_viewRegistry.findView(resolvedDocumentId, requestedViewId);
  if (view == nullptr)
  {
    if (error != nullptr)
    {
      *error = "Unknown viewId for the requested documentId";
    }
    return std::nullopt;
  }

  const auto resolvedViewId = m_viewRegistry.viewId(*view);
  if (resolvedViewId.isEmpty())
  {
    if (error != nullptr)
    {
      *error = "The requested view is no longer available";
    }
    return std::nullopt;
  }
  return AutomationResolvedMapView{window, view, resolvedDocumentId, resolvedViewId};
}

} // namespace tb::ui
