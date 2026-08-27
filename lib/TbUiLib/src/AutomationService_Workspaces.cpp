/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QJsonArray>

#include "AutomationJson.h"
#include "mdl/Node.h"
#include "ui/AutomationService.h"
#include "ui/MapWindow.h"
#include "ui/QPathUtils.h"

namespace tb::ui
{
namespace
{

QString changeKindName(const mdl::WorkspaceChangeKind kind)
{
  switch (kind)
  {
  case mdl::WorkspaceChangeKind::Added:
    return "added";
  case mdl::WorkspaceChangeKind::Removed:
    return "removed";
  case mdl::WorkspaceChangeKind::Changed:
    return "changed";
  case mdl::WorkspaceChangeKind::Reparented:
    return "reparented";
  case mdl::WorkspaceChangeKind::BrushesOptimized:
    return "brushes_optimized";
  }
  return "unknown";
}

QJsonArray changesToJson(const mdl::MapWorkspace& workspace)
{
  auto result = QJsonArray{};
  for (const auto& change : workspace.changes())
  {
    auto json = QJsonObject{
      {"kind", changeKindName(change.kind)},
      {"nodeId", static_cast<qint64>(change.nodeId)},
      {"name",
       QString::fromStdString(
         change.branchNode != nullptr ? change.branchNode->name()
                                      : change.baseNode->name())},
    };
    if (change.kind == mdl::WorkspaceChangeKind::BrushesOptimized)
    {
      json.insert("baseBrushCount", static_cast<qint64>(change.baseBrushCount));
      json.insert("branchBrushCount", static_cast<qint64>(change.branchBrushCount));
    }
    result.push_back(std::move(json));
  }
  return result;
}

QJsonArray conflictsToJson(const mdl::WorkspaceMergePlan& plan)
{
  auto result = QJsonArray{};
  for (const auto& conflict : plan.conflicts)
  {
    result.push_back(QJsonObject{
      {"kind", static_cast<int>(conflict.kind)},
      {"nodeId", static_cast<qint64>(conflict.nodeId)},
    });
  }
  return result;
}

} // namespace

JsonRpcResponse AutomationService::handleWorkspaceRequest(
  const QString& method, const QJsonObject& params)
{
  if (method == "workspace.fork")
  {
    auto* sourceWindow = findWindow(params);
    if (sourceWindow == nullptr)
    {
      return automation::invalidParams("Unknown documentId or no map document is open");
    }
    const auto result =
      m_workspaceManager.fork(*sourceWindow, params.value("name").toString());
    if (!result)
    {
      return JsonRpcResponse::error(
        {JsonRpcError::InternalError, QString::fromStdString(result.error)});
    }
    return JsonRpcResponse::success(QJsonObject{
      {"workspaceId", result.workspace->id},
      {"sourceDocumentId", documentId(*result.workspace->sourceWindow)},
      {"branchDocumentId", documentId(*result.workspace->branchWindow)},
      {"directory", pathAsQString(result.workspace->directory)},
    });
  }

  if (method == "workspace.list")
  {
    auto workspaces = QJsonArray{};
    for (const auto* workspace : m_workspaceManager.workspaces())
    {
      workspaces.push_back(QJsonObject{
        {"workspaceId", workspace->id},
        {"sourceDocumentId",
         workspace->sourceWindow ? documentId(*workspace->sourceWindow) : QString{}},
        {"branchDocumentId",
         workspace->branchWindow ? documentId(*workspace->branchWindow) : QString{}},
        {"directory", pathAsQString(workspace->directory)},
      });
    }
    return JsonRpcResponse::success(workspaces);
  }

  if (method == "workspace.diff" || method == "workspace.merge")
  {
    const auto workspaceId = params.value("workspaceId").toString();
    const auto mergeResult =
      m_workspaceManager.merge(workspaceId, method == "workspace.merge");
    if (!mergeResult.error.empty())
    {
      return JsonRpcResponse::error(
        {JsonRpcError::InternalError, QString::fromStdString(mergeResult.error)});
    }

    const auto* model = m_workspaceManager.model(workspaceId);
    return JsonRpcResponse::success(QJsonObject{
      {"workspaceId", workspaceId},
      {"changes", model != nullptr ? changesToJson(*model) : QJsonArray{}},
      {"operationCount", static_cast<qint64>(mergeResult.plan.operations.size())},
      {"conflicts", conflictsToJson(mergeResult.plan)},
      {"applied", mergeResult.applied},
    });
  }

  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

} // namespace tb::ui
