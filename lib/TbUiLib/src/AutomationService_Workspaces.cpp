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
    result.push_back(
      QJsonObject{
        {"kind", static_cast<int>(conflict.kind)},
        {"nodeId", static_cast<qint64>(conflict.nodeId)},
      });
  }
  return result;
}

JsonRpcResponse workspaceFailure(const QString& workspaceId, const QString& reason)
{
  return JsonRpcResponse::error(
    {-32020,
     "Workspace operation failed",
     QJsonObject{{"workspaceId", workspaceId}, {"reason", reason}}});
}

QJsonArray diagnosticsToJson(
  const std::vector<AutomationWorkspaceStoreDiagnostic>& diagnostics)
{
  auto result = QJsonArray{};
  for (const auto& diagnostic : diagnostics)
  {
    result.push_back(
      QJsonObject{
        {"kind", static_cast<int>(diagnostic.kind)},
        {"message", diagnostic.message},
        {"path", pathAsQString(diagnostic.path)},
      });
  }
  return result;
}

QJsonObject workspaceToJson(const AutomationWorkspaceInfo& workspace)
{
  return {
    {"workspaceId", workspace.id},
    {"name", workspace.manifest.name},
    {"state", automationWorkspaceLifecycleStateName(workspace.manifest.state)},
    {"runtimeStatus", automationWorkspaceRuntimeStatusName(workspace.runtimeStatus)},
    {"sourcePath", pathAsQString(workspace.manifest.source.path)},
    {"sourceChanged", workspace.sourceChanged},
    {"mergeable", workspace.manifest.state == AutomationWorkspaceLifecycleState::Active},
    {"checkpointGeneration",
     static_cast<qint64>(workspace.manifest.checkpointGeneration)},
    {"directory", pathAsQString(workspace.directory)},
    {"diagnostics", diagnosticsToJson(workspace.diagnostics)},
  };
}

} // namespace

JsonRpcResponse AutomationService::handleWorkspaceRequest(
  const QString& method, const QJsonObject& params)
{
  const auto requireWorkspaceId = [&]() -> std::optional<QString> {
    const auto workspaceId = params.value("workspaceId").toString();
    return workspaceId.isEmpty() ? std::nullopt : std::optional{workspaceId};
  };
  const auto explicitWindow = [&](const QString& parameterName) -> MapWindow* {
    const auto documentId = params.value(parameterName).toString();
    if (documentId.isEmpty())
    {
      return nullptr;
    }
    return findWindow(QJsonObject{{"documentId", documentId}});
  };
  const auto resultToJson = [&](const AutomationWorkspaceInfo& workspace) {
    auto result = workspaceToJson(workspace);
    result.insert(
      "sourceDocumentId",
      workspace.sourceWindow ? documentId(*workspace.sourceWindow) : QString{});
    result.insert(
      "branchDocumentId",
      workspace.branchWindow ? documentId(*workspace.branchWindow) : QString{});
    return result;
  };

  if (method == "workspace.fork")
  {
    auto* sourceWindow = explicitWindow("documentId");
    if (sourceWindow == nullptr)
    {
      return automation::invalidParams("workspace.fork requires a known documentId");
    }
    const auto result =
      m_workspaceManager.fork(*sourceWindow, params.value("name").toString());
    if (!result)
    {
      return workspaceFailure({}, QString::fromStdString(result.error));
    }
    return JsonRpcResponse::success(resultToJson(*result.workspace));
  }

  if (method == "workspace.list")
  {
    auto workspaces = QJsonArray{};
    for (const auto* workspace : m_workspaceManager.workspaces())
    {
      workspaces.push_back(resultToJson(*workspace));
    }
    return JsonRpcResponse::success(workspaces);
  }

  if (method == "workspace.status")
  {
    const auto workspaceId = requireWorkspaceId();
    if (!workspaceId)
    {
      return automation::invalidParams("workspace.status requires workspaceId");
    }
    const auto* workspace = m_workspaceManager.find(*workspaceId);
    if (workspace == nullptr)
    {
      return workspaceFailure(*workspaceId, "Unknown workspace");
    }
    return JsonRpcResponse::success(resultToJson(*workspace));
  }

  if (method == "workspace.recover")
  {
    const auto workspaceId = requireWorkspaceId();
    if (!workspaceId)
    {
      return automation::invalidParams("workspace.recover requires workspaceId");
    }
    auto* contextWindow = static_cast<MapWindow*>(nullptr);
    if (params.contains("documentId"))
    {
      contextWindow = explicitWindow("documentId");
      if (contextWindow == nullptr)
      {
        return automation::invalidParams("workspace.recover requires a known documentId");
      }
    }
    const auto result = contextWindow != nullptr
                          ? m_workspaceManager.recover(*workspaceId, *contextWindow)
                          : m_workspaceManager.recover(*workspaceId);
    if (!result)
    {
      return workspaceFailure(*workspaceId, QString::fromStdString(result.error));
    }
    return JsonRpcResponse::success(resultToJson(*result.workspace));
  }

  if (method == "workspace.attachSource")
  {
    const auto workspaceId = requireWorkspaceId();
    if (!workspaceId)
    {
      return automation::invalidParams("workspace.attachSource requires workspaceId");
    }
    auto* sourceWindow = explicitWindow("documentId");
    if (sourceWindow == nullptr)
    {
      return automation::invalidParams(
        "workspace.attachSource requires a known documentId");
    }
    const auto result = m_workspaceManager.attachSource(*workspaceId, *sourceWindow);
    if (!result)
    {
      return workspaceFailure(*workspaceId, QString::fromStdString(result.error));
    }
    return JsonRpcResponse::success(resultToJson(*result.workspace));
  }

  if (method == "workspace.checkpoint" || method == "workspace.close")
  {
    const auto workspaceId = requireWorkspaceId();
    if (!workspaceId)
    {
      return automation::invalidParams(method + " requires workspaceId");
    }
    const auto result = method == "workspace.checkpoint"
                          ? m_workspaceManager.checkpoint(*workspaceId)
                          : m_workspaceManager.close(*workspaceId);
    if (!result)
    {
      return workspaceFailure(*workspaceId, QString::fromStdString(result.error));
    }
    return JsonRpcResponse::success(resultToJson(*result.workspace));
  }

  if (method == "workspace.rename")
  {
    const auto workspaceId = requireWorkspaceId();
    if (!workspaceId)
    {
      return automation::invalidParams("workspace.rename requires workspaceId");
    }
    const auto name = params.value("name");
    if (!name.isString() || name.toString().trimmed().isEmpty())
    {
      return automation::invalidParams("workspace.rename requires a non-empty name");
    }
    const auto result = m_workspaceManager.rename(*workspaceId, name.toString());
    if (!result)
    {
      return workspaceFailure(*workspaceId, QString::fromStdString(result.error));
    }
    return JsonRpcResponse::success(resultToJson(*result.workspace));
  }

  if (method == "workspace.abandon")
  {
    const auto workspaceId = requireWorkspaceId();
    if (!workspaceId)
    {
      return automation::invalidParams("workspace.abandon requires workspaceId");
    }
    const auto result = m_workspaceManager.abandon(*workspaceId);
    if (!result)
    {
      return workspaceFailure(*workspaceId, QString::fromStdString(result.error));
    }
    return JsonRpcResponse::success(resultToJson(*result.workspace));
  }

  if (method == "workspace.diff" || method == "workspace.merge")
  {
    const auto workspaceId = requireWorkspaceId();
    if (!workspaceId)
    {
      return automation::invalidParams(method + " requires workspaceId");
    }
    const auto mergeResult =
      m_workspaceManager.merge(*workspaceId, method == "workspace.merge");
    if (!mergeResult.error.empty())
    {
      return workspaceFailure(*workspaceId, QString::fromStdString(mergeResult.error));
    }

    const auto* model = m_workspaceManager.model(*workspaceId);
    return JsonRpcResponse::success(
      QJsonObject{
        {"workspaceId", *workspaceId},
        {"changes", model != nullptr ? changesToJson(*model) : QJsonArray{}},
        {"operationCount", static_cast<qint64>(mergeResult.plan.operations.size())},
        {"conflicts", conflictsToJson(mergeResult.plan)},
        {"applied", mergeResult.applied},
      });
  }

  return JsonRpcResponse::error({JsonRpcError::MethodNotFound, "Method not found"});
}

} // namespace tb::ui
