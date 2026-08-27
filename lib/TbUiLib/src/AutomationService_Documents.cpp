/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QCoreApplication>
#include <QJsonArray>

#include "AutomationJson.h"
#include "mdl/Map.h"
#include "mdl/WorldNode.h"
#include "ui/AppController.h"
#include "ui/AutomationService.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/QPathUtils.h"

namespace tb::ui
{
namespace
{

bool expectedRevisionMatches(const mdl::Map& map, const QJsonObject& params)
{
  const auto expectedRevision = automation::sizeFromJson(params.value("expectedRevision"));
  return expectedRevision && *expectedRevision == map.modificationCount();
}

QJsonObject documentState(const MapWindow& window)
{
  const auto& map = window.document().map();
  return QJsonObject{
    {"path", pathAsQString(map.path())},
    {"filename", QString::fromStdString(map.filename())},
    {"revision", static_cast<qint64>(map.modificationCount())},
    {"modified", map.modified()},
  };
}

} // namespace

JsonRpcResponse AutomationService::handleDocumentRequest(
  const QString& method, const QJsonObject& params)
{
  if (method == "system.ping")
  {
    return JsonRpcResponse::success(
      QJsonObject{
        {"name", "TrenchBroom"},
        {"automationApiVersion", 1},
        {"pid", QCoreApplication::applicationPid()},
      });
  }

  if (method == "documents.list")
  {
    const auto* activeWindow = m_appController.mapWindowManager().topMapWindow();
    auto documents = QJsonArray{};
    for (const auto* window : m_appController.mapWindowManager().mapWindows())
    {
      const auto& map = window->document().map();
      documents.push_back(
        QJsonObject{
          {"id", documentId(*window)},
          {"path", pathAsQString(map.path())},
          {"filename", QString::fromStdString(map.filename())},
          {"revision", static_cast<qint64>(map.modificationCount())},
          {"modified", map.modified()},
          {"active", window == activeWindow},
        });
    }
    return JsonRpcResponse::success(documents);
  }

  if (method == "reference.open")
  {
    auto* sourceWindow = findWindow(params);
    const auto path = pathFromQString(params.value("path").toString());
    if (sourceWindow == nullptr || path.empty())
    {
      return automation::invalidParams("A valid source documentId and path are required");
    }
    auto& sourceMap = sourceWindow->document().map();
    auto loaded = m_appController.mapWindowManager().loadDocumentInNewWindow(
      sourceMap.gameInfo(),
      sourceMap.worldNode().mapFormat(),
      sourceMap.worldBounds(),
      path);
    if (loaded.is_error())
    {
      return JsonRpcResponse::error(
        {JsonRpcError::InternalError, "Could not open the reference map"});
    }
    return JsonRpcResponse::success(
      QJsonObject{
        {"documentId", documentId(*loaded.value())},
        {"path", pathAsQString(path)},
      });
  }

  if (method == "document.save")
  {
    auto* window = findWindow(params);
    if (window == nullptr || !automation::sizeFromJson(params.value("expectedRevision")))
    {
      return automation::invalidParams(
        "documentId and expectedRevision are required for a mutation");
    }
    auto& map = window->document().map();
    if (!expectedRevisionMatches(map, params))
    {
      return automation::revisionConflict(map.modificationCount());
    }
    if (!map.persistent())
    {
      return automation::invalidParams("Cannot save a transient document without a path");
    }
    const auto saved = map.save();
    if (saved.is_error())
    {
      return JsonRpcResponse::error(
        {JsonRpcError::InternalError, "Could not save the document"});
    }
    auto result = documentState(*window);
    result.insert("documentId", documentId(*window));
    return JsonRpcResponse::success(result);
  }

  if (method == "document.reload")
  {
    auto* window = findWindow(params);
    if (window == nullptr || !automation::sizeFromJson(params.value("expectedRevision")))
    {
      return automation::invalidParams(
        "documentId and expectedRevision are required for a mutation");
    }
    auto& map = window->document().map();
    if (!expectedRevisionMatches(map, params))
    {
      return automation::revisionConflict(map.modificationCount());
    }
    if (map.modified() && !params.value("discardChanges").toBool(false))
    {
      return automation::invalidParams(
        "The document has unsaved changes; set discardChanges to reload it");
    }
    const auto reloaded = window->document().reload();
    if (reloaded.is_error())
    {
      return JsonRpcResponse::error(
        {JsonRpcError::InternalError, "Could not reload the document"});
    }
    auto result = documentState(*window);
    result.insert("documentId", documentId(*window));
    return JsonRpcResponse::success(result);
  }

  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

} // namespace tb::ui
