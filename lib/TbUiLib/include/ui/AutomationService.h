/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QObject>
#include <QString>

#include "ui/AcceptanceMapGeometry.h"
#include "ui/AcceptanceVirtualCaptureAdapter.h"
#include "ui/AutomationCameraHandleService.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/AutomationRenderRpcAdapter.h"
#include "ui/AutomationViewRegistry.h"
#include "ui/AutomationWorkspaceManager.h"
#include "ui/LocalJsonRpcServer.h"

#include <filesystem>
#include <optional>

namespace tb::ui
{
class AppController;
class MapWindow;
class MapViewBase;

struct AutomationResolvedMapView
{
  MapWindow* window = nullptr;
  MapViewBase* view = nullptr;
  QString documentId;
  QString viewId;
};

/**
 * Publishes TrenchBroom's local automation API.
 *
 * The service listens only through QLocalServer. A small discovery document is written
 * into the user data directory for same-user clients such as tbctl. All model access is
 * dispatched synchronously on the Qt main thread by LocalJsonRpcServer.
 */
class AutomationService : public QObject, public JsonRpcRequestHandler
{
private:
  AppController& m_appController;
  LocalJsonRpcServer m_server;
  mutable AutomationDocumentRegistry m_documentRegistry;
  mutable AutomationViewRegistry m_viewRegistry;
  AutomationRegistryCameraDocumentResolver m_cameraDocumentResolver;
  AutomationCameraHandleRegistry m_cameraHandles;
  AutomationVirtualCameraCapture m_cameraCapture;
  AutomationCameraHandleService m_cameraService;
  AutomationWorkspaceManager m_workspaceManager;
  AutomationRenderRpcAdapter m_renderRpcAdapter;
  AcceptanceVirtualCaptureAdapter m_acceptanceCapture;
  AcceptanceAutomationMapResolver m_acceptanceMapResolver;
  AcceptanceMapGeometryProvider m_acceptanceGeometry;
  QString m_serverName;
  std::filesystem::path m_discoveryPath;

public:
  explicit AutomationService(AppController& appController, QObject* parent = nullptr);
  ~AutomationService() override;

  bool isListening() const;
  const QString& serverName() const;
  const std::filesystem::path& discoveryPath() const;

private:
  JsonRpcResponse handleRequest(const QString& method, const QJsonValue& params) override;
  JsonRpcResponse handleDocumentRequest(const QString& method, const QJsonObject& params);
  JsonRpcResponse handleViewRequest(const QString& method, const QJsonObject& params);
  JsonRpcResponse handleNodeRequest(const QString& method, const QJsonObject& params);
  JsonRpcResponse handleBrushRequest(const QString& method, const QJsonObject& params);
  JsonRpcResponse handleFaceRequest(const QString& method, const QJsonObject& params);
  JsonRpcResponse handleGeometryRequest(const QString& method, const QJsonObject& params);
  JsonRpcResponse handleWorkspaceRequest(
    const QString& method, const QJsonObject& params);
  JsonRpcResponse handleAcceptanceRequest(
    const QString& method, const QJsonObject& params);

  QString documentId(const MapWindow& window) const;
  MapWindow* findWindow(const QJsonObject& params) const;
  std::optional<AutomationResolvedMapView> resolveMapView(
    const QJsonObject& params, QString* error = nullptr) const;
  void registerMapViews(const MapWindow& window) const;

  bool start();
  void removeDiscoveryFile();
};

} // namespace tb::ui
