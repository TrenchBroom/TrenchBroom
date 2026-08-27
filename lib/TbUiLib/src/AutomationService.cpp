/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationService.h"

#include "ui/AppController.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"

#include <cstdint>

namespace tb::ui
{

AutomationService::AutomationService(AppController& appController, QObject* parent)
  : QObject{parent}
  , m_appController{appController}
  , m_server{this, this}
  , m_workspaceManager{appController}
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
  if (method.startsWith("context.") || method.startsWith("view."))
  {
    return handleViewRequest(method, params);
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
  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

QString AutomationService::documentId(const MapWindow& window) const
{
  return QString{"document-%1"}.arg(
    static_cast<qulonglong>(reinterpret_cast<std::uintptr_t>(&window)), 0, 16);
}

MapWindow* AutomationService::findWindow(const QJsonObject& params) const
{
  const auto requestedId = params.value("documentId").toString();
  if (requestedId.isEmpty())
  {
    return m_appController.mapWindowManager().topMapWindow();
  }

  for (auto* window : m_appController.mapWindowManager().mapWindows())
  {
    if (documentId(*window) == requestedId)
    {
      return window;
    }
  }
  return nullptr;
}

} // namespace tb::ui
