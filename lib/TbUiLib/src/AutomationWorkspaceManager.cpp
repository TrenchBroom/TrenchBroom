/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationWorkspaceManager.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QUuid>

#include "gl/GlManager.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameManager.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/NodeContents.h"
#include "mdl/Object.h"
#include "mdl/PatchNode.h"
#include "mdl/SetLinkIdsCommand.h"
#include "mdl/SetLockStateCommand.h"
#include "mdl/SetVisibilityCommand.h"
#include "mdl/Transaction.h"
#include "mdl/WorldNode.h"
#include "ui/AppController.h"
#include "ui/MapDocument.h"
#include "ui/MapWindow.h"
#include "ui/MapWindowManager.h"
#include "ui/QPathUtils.h"

#include "kd/overload.h"

#include <system_error>

namespace tb::ui
{
namespace
{

mdl::NodeContents copyContents(const mdl::Node& node)
{
  return node.accept(
    kdl::overload(
      [](const mdl::WorldNode& n) { return mdl::NodeContents{n.entity()}; },
      [](const mdl::LayerNode& n) { return mdl::NodeContents{n.layer()}; },
      [](const mdl::GroupNode& n) { return mdl::NodeContents{n.group()}; },
      [](const mdl::EntityNode& n) { return mdl::NodeContents{n.entity()}; },
      [](const mdl::BrushNode& n) { return mdl::NodeContents{n.brush()}; },
      [](const mdl::PatchNode& n) { return mdl::NodeContents{n.patch()}; }));
}

bool containsNode(const mdl::Node& root, const mdl::Node* target)
{
  if (&root == target)
  {
    return true;
  }
  for (const auto* child : root.children())
  {
    if (containsNode(*child, target))
    {
      return true;
    }
  }
  return false;
}

bool copyNodeState(mdl::Map& map, mdl::Node& liveNode, const mdl::Node& branchNode)
{
  if (liveNode.visibilityState() != branchNode.visibilityState())
  {
    auto command = [&]() {
      switch (branchNode.visibilityState())
      {
      case mdl::VisibilityState::Inherited:
        return mdl::SetVisibilityCommand::reset({&liveNode});
      case mdl::VisibilityState::Hidden:
        return mdl::SetVisibilityCommand::hide({&liveNode});
      case mdl::VisibilityState::Shown:
        return mdl::SetVisibilityCommand::show({&liveNode});
      }
      return mdl::SetVisibilityCommand::reset({&liveNode});
    }();
    if (!map.executeAndStore(std::move(command)))
    {
      return false;
    }
  }

  if (liveNode.lockState() != branchNode.lockState())
  {
    auto command = [&]() {
      switch (branchNode.lockState())
      {
      case mdl::LockState::Inherited:
        return mdl::SetLockStateCommand::reset({&liveNode});
      case mdl::LockState::Locked:
        return mdl::SetLockStateCommand::lock({&liveNode});
      case mdl::LockState::Unlocked:
        return mdl::SetLockStateCommand::unlock({&liveNode});
      }
      return mdl::SetLockStateCommand::reset({&liveNode});
    }();
    if (!map.executeAndStore(std::move(command)))
    {
      return false;
    }
  }

  return true;
}

void collectNodes(const mdl::Node& node, std::vector<const mdl::Node*>& result)
{
  result.push_back(&node);
  for (const auto* child : node.children())
  {
    collectNodes(*child, result);
  }
}

bool copyLinkedGroupIds(
  mdl::Map& map, const mdl::MapWorkspace& workspace, const mdl::GroupNode& branchGroup)
{
  auto branchNodes = std::vector<const mdl::Node*>{};
  collectNodes(branchGroup, branchNodes);

  auto linkIds = std::vector<std::tuple<mdl::Node*, std::string>>{};
  for (const auto* branchNode : branchNodes)
  {
    const auto id = workspace.nodeId(*branchNode);
    if (!id)
    {
      // Added descendants are cloned recursively and already retain their link IDs.
      continue;
    }
    auto* liveNode = const_cast<mdl::Node*>(workspace.sourceNode(*id));
    const auto* branchObject = dynamic_cast<const mdl::Object*>(branchNode);
    auto* liveObject = dynamic_cast<mdl::Object*>(liveNode);
    if (
      branchObject != nullptr && liveObject != nullptr
      && branchObject->linkId() != liveObject->linkId())
    {
      linkIds.emplace_back(liveNode, branchObject->linkId());
    }
  }

  return linkIds.empty()
         || map.executeAndStore(
           std::make_unique<mdl::SetLinkIdsCommand>(
             "Merge Workspace Link IDs", std::move(linkIds)));
}

std::optional<QString> fingerprint(const std::filesystem::path& path)
{
  auto file = QFile{pathAsQString(path)};
  if (!file.open(QIODevice::ReadOnly))
  {
    return std::nullopt;
  }

  auto hash = QCryptographicHash{QCryptographicHash::Sha256};
  while (!file.atEnd())
  {
    const auto chunk = file.read(64 * 1024);
    if (chunk.isEmpty() && file.error() != QFile::NoError)
    {
      return std::nullopt;
    }
    hash.addData(chunk);
  }
  return "sha256:" + QString::fromLatin1(hash.result().toHex());
}

std::vector<AutomationWorkspaceNodeIdentity> toManifestIdentities(
  const std::vector<mdl::WorkspaceNodeIdentity>& identities)
{
  auto result = std::vector<AutomationWorkspaceNodeIdentity>{};
  result.reserve(identities.size());
  for (const auto& identity : identities)
  {
    result.push_back(
      {identity.id,
       QString::fromStdString(identity.type),
       identity.basePath,
       identity.baseParentId,
       identity.branchPath});
  }
  return result;
}

std::vector<mdl::WorkspaceNodeIdentity> toModelIdentities(
  const std::vector<AutomationWorkspaceNodeIdentity>& identities)
{
  auto result = std::vector<mdl::WorkspaceNodeIdentity>{};
  result.reserve(identities.size());
  for (const auto& identity : identities)
  {
    result.push_back(
      {identity.id,
       identity.type.toStdString(),
       identity.basePath,
       identity.baseParentId,
       identity.branchPath});
  }
  return result;
}

std::filesystem::path liveBranchPath(const AutomationWorkspaceInfo& entry)
{
  return entry.directory / "branch-live.map";
}

std::optional<std::filesystem::path> artifactPath(
  const AutomationWorkspaceInfo& entry, const AutomationWorkspaceArtifact& artifact)
{
  return AutomationWorkspaceManifest::resolveArtifactPath(entry.directory, artifact.path);
}

} // namespace

struct AutomationWorkspaceManager::Entry : AutomationWorkspaceInfo
{
  struct Session
  {
    std::unique_ptr<MapDocument> baseDocument;
    std::unique_ptr<mdl::MapWorkspace> model;
  };

  std::unique_ptr<Session> session;
};

AutomationWorkspaceManager::AutomationWorkspaceManager(AppController& appController)
  : AutomationWorkspaceManager{
      appController,
      appController.environmentConfig().userDataFolderPath / "automation" / "workspaces"}
{
}

AutomationWorkspaceManager::AutomationWorkspaceManager(
  AppController& appController, std::filesystem::path workspaceRoot)
  : m_appController{appController}
  , m_store{std::move(workspaceRoot)}
{
  for (const auto& record : m_store.scan())
  {
    auto entry = std::make_unique<Entry>();
    entry->directory = record.directory;
    entry->diagnostics = record.diagnostics;
    if (record.manifest)
    {
      entry->id = record.manifest->workspaceId;
      entry->manifest = *record.manifest;
      entry->runtimeStatus = AutomationWorkspaceRuntimeStatus::Dormant;
    }
    else
    {
      entry->id = QString::fromStdString(record.directory.filename().string());
      entry->runtimeStatus = AutomationWorkspaceRuntimeStatus::Invalid;
    }
    m_entries.push_back(std::move(entry));
  }
}

AutomationWorkspaceManager::~AutomationWorkspaceManager()
{
  for (auto& entry : m_entries)
  {
    if (!entry || !entry->session || entry->branchWindow.isNull())
    {
      continue;
    }

    try
    {
      if (entry->branchWindow->document().map().modified())
      {
        static_cast<void>(checkpoint(*entry));
      }
    }
    catch (...)
    {
      // Teardown must not allow a failed persistence attempt to leave a hidden window
      // to present an interactive save prompt later.
    }

    if (!entry->branchWindow.isNull())
    {
      static_cast<void>(entry->branchWindow->closeWithoutSaving());
    }
    entry->session.reset();
    entry->sourceWindow = nullptr;
    entry->branchWindow = nullptr;
  }
}

AutomationWorkspaceResult AutomationWorkspaceManager::fork(
  MapWindow& sourceWindow, const QString& name)
{
  auto& sourceMap = sourceWindow.document().map();
  if (!sourceMap.persistent())
  {
    return {nullptr, "A workspace source document must have an absolute map path"};
  }

  const auto sourceFingerprint = fingerprint(sourceMap.path());
  if (!sourceFingerprint)
  {
    return {nullptr, "Could not fingerprint the saved source map"};
  }

  const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const auto inputDirectory =
    m_store.rootDirectory()
    / ("." + id.toStdString() + ".fork-input-"
       + QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString());
  auto ec = std::error_code{};
  std::filesystem::create_directories(inputDirectory, ec);
  if (ec)
  {
    return {nullptr, "Could not create a temporary workspace snapshot directory"};
  }
  const auto baseInput = inputDirectory / "base.map";
  const auto branchInput = inputDirectory / "branch.map";
  const auto savedBase = sourceMap.saveTo(baseInput);
  const auto savedBranch = sourceMap.saveTo(branchInput);
  if (savedBase.is_error() || savedBranch.is_error())
  {
    std::filesystem::remove_all(inputDirectory, ec);
    return {nullptr, "Could not snapshot the source map"};
  }

  auto initialModel = mdl::MapWorkspace{sourceMap.worldNode(), sourceMap.worldBounds()};
  auto manifest = AutomationWorkspaceManifest{};
  manifest.workspaceId = id;
  manifest.name = name.isEmpty() ? QString::fromStdString(sourceMap.filename()) : name;
  manifest.createdAt = QDateTime::currentDateTimeUtc();
  manifest.source = {sourceMap.path(), *sourceFingerprint, sourceMap.modificationCount()};
  manifest.mapMetadata = {
    QString::fromStdString(sourceMap.gameInfo().gameConfig.name),
    sourceMap.worldNode().mapFormat(),
    sourceMap.worldBounds(),
  };
  manifest.nodeIdentities = toManifestIdentities(initialModel.exportNodeIdentities());
  const auto created = m_store.create(std::move(manifest), baseInput, branchInput);
  std::filesystem::remove_all(inputDirectory, ec);
  if (!created)
  {
    return {nullptr, created.error.toStdString()};
  }

  auto entry = std::make_unique<Entry>();
  entry->id = created.manifest->workspaceId;
  entry->directory = m_store.workspaceDirectory(entry->id);
  entry->manifest = *created.manifest;
  m_entries.push_back(std::move(entry));
  auto* result = m_entries.back().get();
  const auto recovered = recover(*result, &sourceWindow);
  if (!recovered)
  {
    return recovered;
  }
  try
  {
    result->session->model->attachSource(sourceMap.worldNode());
    result->sourceWindow = &sourceWindow;
    refreshRuntimeStatus(*result);
  }
  catch (const std::exception& e)
  {
    return {nullptr, std::string{"Could not attach the fork source: "} + e.what()};
  }
  return {result, {}};
}

AutomationWorkspaceResult AutomationWorkspaceManager::recover(const QString& id)
{
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return {nullptr, "Unknown workspace"};
  }
  return recover(*entry, nullptr);
}

AutomationWorkspaceResult AutomationWorkspaceManager::recover(
  const QString& id, MapWindow& mapContextWindow)
{
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return {nullptr, "Unknown workspace"};
  }
  return recover(*entry, &mapContextWindow);
}

AutomationWorkspaceResult AutomationWorkspaceManager::recover(
  Entry& entry, const MapWindow* mapContextWindow)
{
  validateRecord(entry);
  refreshRuntimeStatus(entry);
  if (entry.runtimeStatus == AutomationWorkspaceRuntimeStatus::Invalid)
  {
    return {nullptr, "Workspace has no valid durable manifest"};
  }
  if (entry.session && entry.branchWindow)
  {
    return {&entry, {}};
  }
  entry.session.reset();
  entry.sourceWindow = nullptr;
  entry.branchWindow = nullptr;

  const auto basePath = artifactPath(entry, entry.manifest.base);
  const auto branchPath = artifactPath(entry, entry.manifest.branch);
  if (!basePath || !branchPath)
  {
    entry.runtimeStatus = AutomationWorkspaceRuntimeStatus::Invalid;
    return {nullptr, "Workspace artifact paths are invalid"};
  }

  const auto* gameInfo = static_cast<const mdl::GameInfo*>(nullptr);
  auto mapFormat = mdl::MapFormat::Unknown;
  auto worldBounds = vm::bbox3d{};
  if (entry.manifest.mapMetadata)
  {
    const auto& metadata = *entry.manifest.mapMetadata;
    gameInfo = m_appController.gameManager().gameInfo(metadata.gameName.toStdString());
    if (gameInfo == nullptr)
    {
      return {nullptr, "The workspace game configuration is unavailable"};
    }
    mapFormat = metadata.mapFormat;
    worldBounds = metadata.worldBounds;
  }
  else
  {
    if (mapContextWindow == nullptr)
    {
      return {
        nullptr,
        "This legacy version 1 workspace lacks map metadata; recover requires documentId "
        "context",
      };
    }
    const auto& contextMap = mapContextWindow->document().map();
    gameInfo = &contextMap.gameInfo();
    mapFormat = contextMap.worldNode().mapFormat();
    worldBounds = contextMap.worldBounds();
  }

  auto baseDocument = MapDocument::loadDocument(
    m_appController.environmentConfig(),
    *gameInfo,
    mapFormat,
    worldBounds,
    *basePath,
    m_appController.taskManager(),
    m_appController.glManager().resourceManager());
  if (baseDocument.is_error())
  {
    return {nullptr, "Could not load the workspace base map"};
  }

  const auto livePath = liveBranchPath(entry);
  auto ec = std::error_code{};
  std::filesystem::copy_file(
    *branchPath, livePath, std::filesystem::copy_options::overwrite_existing, ec);
  if (ec)
  {
    return {nullptr, "Could not prepare the live workspace branch map"};
  }
  auto branchWindow = m_appController.mapWindowManager().loadDocumentInNewWindow(
    *gameInfo, mapFormat, worldBounds, livePath, false);
  if (branchWindow.is_error())
  {
    return {nullptr, "Could not open the workspace branch map"};
  }

  auto session = std::make_unique<Entry::Session>();
  session->baseDocument = std::move(baseDocument).value();
  try
  {
    session->model = std::make_unique<mdl::MapWorkspace>(
      session->baseDocument->map().worldNode(),
      branchWindow.value()->document().map().worldNode(),
      worldBounds,
      toModelIdentities(entry.manifest.nodeIdentities));
  }
  catch (const std::exception& e)
  {
    static_cast<void>(branchWindow.value()->closeWithoutSaving());
    return {
      nullptr, std::string{"Could not reconstruct the workspace model: "} + e.what()};
  }

  entry.branchWindow = branchWindow.value();
  entry.session = std::move(session);
  refreshRuntimeStatus(entry);
  return {&entry, {}};
}

AutomationWorkspaceResult AutomationWorkspaceManager::attachSource(
  const QString& id, MapWindow& sourceWindow)
{
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return {nullptr, "Unknown workspace"};
  }
  validateRecord(*entry);
  refreshRuntimeStatus(*entry);
  if (entry->runtimeStatus == AutomationWorkspaceRuntimeStatus::Invalid)
  {
    return {nullptr, "Workspace has no valid durable manifest"};
  }
  if (entry->manifest.state != AutomationWorkspaceLifecycleState::Active)
  {
    return {nullptr, "Only active workspaces can attach a source document"};
  }
  const auto recovered = recover(*entry, &sourceWindow);
  if (!recovered)
  {
    return recovered;
  }
  try
  {
    if (entry->session->model->hasAttachedSource() && entry->sourceWindow.isNull())
    {
      const auto checkpointed = checkpoint(*entry);
      if (!checkpointed)
      {
        return checkpointed;
      }
      entry->session->model = std::make_unique<mdl::MapWorkspace>(
        entry->session->baseDocument->map().worldNode(),
        entry->branchWindow->document().map().worldNode(),
        sourceWindow.document().map().worldBounds(),
        toModelIdentities(entry->manifest.nodeIdentities));
    }
    entry->session->model->attachSource(sourceWindow.document().map().worldNode());
    entry->sourceWindow = &sourceWindow;
    refreshRuntimeStatus(*entry);
    return {entry, {}};
  }
  catch (const std::exception& e)
  {
    entry->runtimeStatus = AutomationWorkspaceRuntimeStatus::SourceChanged;
    return {nullptr, std::string{"Could not attach the source document: "} + e.what()};
  }
}

AutomationWorkspaceResult AutomationWorkspaceManager::checkpoint(const QString& id)
{
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return {nullptr, "Unknown workspace"};
  }
  validateRecord(*entry);
  refreshRuntimeStatus(*entry);
  if (entry->runtimeStatus == AutomationWorkspaceRuntimeStatus::Invalid)
  {
    return {nullptr, "Workspace has no valid durable manifest"};
  }
  return checkpoint(*entry);
}

AutomationWorkspaceResult AutomationWorkspaceManager::rename(
  const QString& id, const QString& name)
{
  const auto trimmedName = name.trimmed();
  if (trimmedName.isEmpty())
  {
    return {nullptr, "Workspace name must not be empty"};
  }
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return {nullptr, "Unknown workspace"};
  }
  validateRecord(*entry);
  refreshRuntimeStatus(*entry);
  if (entry->runtimeStatus == AutomationWorkspaceRuntimeStatus::Invalid)
  {
    return {nullptr, "Workspace has no valid durable manifest"};
  }

  auto manifest = entry->manifest;
  manifest.name = trimmedName;
  return updateMetadata(*entry, std::move(manifest));
}

AutomationWorkspaceResult AutomationWorkspaceManager::abandon(const QString& id)
{
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return {nullptr, "Unknown workspace"};
  }
  validateRecord(*entry);
  refreshRuntimeStatus(*entry);
  if (entry->runtimeStatus == AutomationWorkspaceRuntimeStatus::Invalid)
  {
    return {nullptr, "Workspace has no valid durable manifest"};
  }
  if (entry->manifest.state != AutomationWorkspaceLifecycleState::Active)
  {
    return {nullptr, "Only active workspaces can be abandoned"};
  }
  if (
    entry->session && entry->branchWindow
    && entry->branchWindow->document().map().modified())
  {
    const auto checkpointed = checkpoint(*entry);
    if (!checkpointed)
    {
      return checkpointed;
    }
  }

  auto manifest = entry->manifest;
  manifest.state = AutomationWorkspaceLifecycleState::Abandoned;
  return updateMetadata(*entry, std::move(manifest));
}

AutomationWorkspaceResult AutomationWorkspaceManager::checkpoint(Entry& entry)
{
  refreshRuntimeStatus(entry);
  if (entry.manifest.state == AutomationWorkspaceLifecycleState::Abandoned)
  {
    return {nullptr, "An abandoned workspace cannot publish another checkpoint"};
  }
  if (!entry.session || entry.branchWindow.isNull())
  {
    return {nullptr, "The workspace branch is not open"};
  }

  const auto branchPath = liveBranchPath(entry);
  if (entry.branchWindow->document().map().saveAs(branchPath).is_error())
  {
    return {nullptr, "Could not write the live workspace branch map"};
  }
  auto manifest = entry.manifest;
  manifest.nodeIdentities =
    toManifestIdentities(entry.session->model->exportNodeIdentities());
  const auto published = m_store.publishCheckpoint(manifest, branchPath);
  if (!published)
  {
    return {nullptr, published.error.toStdString()};
  }
  entry.manifest = *published.manifest;
  entry.diagnostics.clear();
  refreshRuntimeStatus(entry);
  return {&entry, {}};
}

AutomationWorkspaceResult AutomationWorkspaceManager::updateMetadata(
  Entry& entry, AutomationWorkspaceManifest manifest)
{
  const auto published = m_store.updateMetadata(manifest);
  if (!published)
  {
    return {nullptr, published.error.toStdString()};
  }
  entry.manifest = *published.manifest;
  entry.diagnostics.clear();
  refreshRuntimeStatus(entry);
  return {&entry, {}};
}

AutomationWorkspaceResult AutomationWorkspaceManager::close(const QString& id)
{
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return {nullptr, "Unknown workspace"};
  }
  if (entry->session && entry->branchWindow)
  {
    const auto releaseBranch = [&]() {
      if (!entry->branchWindow->closeWithoutSaving())
      {
        return false;
      }
      entry->session.reset();
      entry->sourceWindow = nullptr;
      entry->branchWindow = nullptr;
      refreshRuntimeStatus(*entry);
      return true;
    };
    if (entry->branchWindow->document().map().modified())
    {
      const auto checkpointed = checkpoint(*entry);
      if (!checkpointed)
      {
        if (releaseBranch())
        {
          return {
            nullptr,
            checkpointed.error
              + "; discarded unsaved hidden branch changes while closing the workspace",
          };
        }
        return checkpointed;
      }
    }
    if (!releaseBranch())
    {
      return {
        nullptr,
        "The workspace branch is unexpectedly visible and cannot be closed without "
        "confirmation",
      };
    }
    return {entry, {}};
  }
  entry->session.reset();
  entry->sourceWindow = nullptr;
  entry->branchWindow = nullptr;
  refreshRuntimeStatus(*entry);
  return {entry, {}};
}

std::vector<AutomationWorkspaceInfo*> AutomationWorkspaceManager::workspaces()
{
  auto result = std::vector<AutomationWorkspaceInfo*>{};
  for (auto& entry : m_entries)
  {
    validateRecord(*entry);
    refreshRuntimeStatus(*entry);
    result.push_back(entry.get());
  }
  return result;
}

AutomationWorkspaceInfo* AutomationWorkspaceManager::find(const QString& id)
{
  auto* entry = findEntry(id);
  if (entry != nullptr)
  {
    validateRecord(*entry);
    refreshRuntimeStatus(*entry);
  }
  return entry;
}

const mdl::MapWorkspace* AutomationWorkspaceManager::model(const QString& id) const
{
  auto* entry = findEntry(id);
  if (entry == nullptr)
  {
    return nullptr;
  }
  refreshRuntimeStatus(*entry);
  return entry->session && entry->branchWindow ? entry->session->model.get() : nullptr;
}

AutomationMergeResult AutomationWorkspaceManager::merge(
  const QString& id, const bool apply)
{
  auto* entry = findEntry(id);
  if (
    entry == nullptr || !entry->session || entry->sourceWindow.isNull()
    || entry->branchWindow.isNull())
  {
    return {{}, false, "Workspace source and branch documents must be attached"};
  }
  if (entry->manifest.state != AutomationWorkspaceLifecycleState::Active)
  {
    return {{}, false, "Only active workspaces can be merged; fork a new workspace"};
  }

  auto& model = *entry->session->model;
  auto& map = entry->sourceWindow->document().map();
  if (
    model.sourceNode(1u) != &map.worldNode()
    || &model.branchWorld() != &entry->branchWindow->document().map().worldNode())
  {
    return {
      {}, false, "A workspace session no longer owns its source or branch document"};
  }
  auto plan = model.planMerge(map.worldNode());
  if (!apply || !plan.canApply())
  {
    return {std::move(plan), false, {}};
  }
  if (plan.operations.empty())
  {
    entry->manifest.state = AutomationWorkspaceLifecycleState::Merged;
    const auto checkpointed = checkpoint(*entry);
    return checkpointed
             ? AutomationMergeResult{std::move(plan), true, {}}
             : AutomationMergeResult{std::move(plan), false, checkpointed.error};
  }

  auto transaction = mdl::Transaction{map, "Merge Automation Workspace"};
  for (const auto& operation : plan.operations)
  {
    auto* liveNode = const_cast<mdl::Node*>(model.sourceNode(operation.nodeId));
    switch (operation.kind)
    {
    case mdl::WorkspaceMergeOperationKind::Add: {
      auto* parent = operation.parentId
                       ? const_cast<mdl::Node*>(model.sourceNode(*operation.parentId))
                       : static_cast<mdl::Node*>(&map.worldNode());
      if (!containsNode(map.worldNode(), parent))
      {
        transaction.cancel();
        return {std::move(plan), false, "The destination parent no longer exists"};
      }
      auto* clone = operation.branchNode->cloneRecursively(map.worldBounds());
      if (mdl::addNodes(map, {{parent, {clone}}}).empty())
      {
        transaction.cancel();
        return {std::move(plan), false, "Could not add branch nodes"};
      }
      break;
    }
    case mdl::WorkspaceMergeOperationKind::Replace:
      if (!containsNode(map.worldNode(), liveNode))
      {
        transaction.cancel();
        return {std::move(plan), false, "A node to update no longer exists"};
      }
      if (!mdl::updateNodeContents(
            map,
            "Merge Workspace Change",
            {{liveNode, copyContents(*operation.branchNode)}}))
      {
        transaction.cancel();
        return {std::move(plan), false, "Could not update node contents"};
      }
      if (!copyNodeState(map, *liveNode, *operation.branchNode))
      {
        transaction.cancel();
        return {std::move(plan), false, "Could not update node state"};
      }
      if (
        const auto* branchGroup =
          dynamic_cast<const mdl::GroupNode*>(operation.branchNode))
      {
        if (!copyLinkedGroupIds(map, model, *branchGroup))
        {
          transaction.cancel();
          return {std::move(plan), false, "Could not update linked group IDs"};
        }
      }
      break;
    case mdl::WorkspaceMergeOperationKind::Reparent: {
      if (!operation.parentId)
      {
        transaction.cancel();
        return {std::move(plan), false, "The destination parent is branch-only"};
      }
      auto* parent = const_cast<mdl::Node*>(model.sourceNode(*operation.parentId));
      if (
        !containsNode(map.worldNode(), liveNode)
        || !containsNode(map.worldNode(), parent))
      {
        transaction.cancel();
        return {std::move(plan), false, "A node to reparent no longer exists"};
      }
      if (!mdl::reparentNodes(map, {{parent, {liveNode}}}))
      {
        transaction.cancel();
        return {std::move(plan), false, "Could not reparent a node"};
      }
      break;
    }
    case mdl::WorkspaceMergeOperationKind::Remove:
      // Reparenting can remove an empty group or entity as a side effect. A later
      // planned removal of that same node is already satisfied and must not dereference
      // its stale source pointer.
      if (containsNode(map.worldNode(), liveNode))
      {
        mdl::removeNodes(map, {liveNode});
      }
      break;
    }
  }

  if (!transaction.commit())
  {
    return {std::move(plan), false, "Could not commit the merge transaction"};
  }
  entry->manifest.state = AutomationWorkspaceLifecycleState::Merged;
  const auto checkpointed = checkpoint(*entry);
  return checkpointed ? AutomationMergeResult{std::move(plan), true, {}}
                      : AutomationMergeResult{std::move(plan), false, checkpointed.error};
}

AutomationWorkspaceManager::Entry* AutomationWorkspaceManager::findEntry(
  const QString& id) const
{
  for (const auto& entry : m_entries)
  {
    if (entry->id == id)
    {
      return entry.get();
    }
  }
  return nullptr;
}

void AutomationWorkspaceManager::validateRecord(Entry& entry) const
{
  const auto record = m_store.read(entry.directory);
  entry.diagnostics = record.diagnostics;
  if (record.manifest)
  {
    entry.manifest = *record.manifest;
  }
  else
  {
    entry.manifest = {};
  }
}

void AutomationWorkspaceManager::refreshRuntimeStatus(Entry& entry) const
{
  if (entry.manifest.workspaceId.isEmpty())
  {
    entry.runtimeStatus = AutomationWorkspaceRuntimeStatus::Invalid;
    entry.sourceChanged = false;
    return;
  }
  const auto currentSourceFingerprint = fingerprint(entry.manifest.source.path);
  entry.sourceChanged =
    !currentSourceFingerprint
    || *currentSourceFingerprint != entry.manifest.source.fingerprintAtFork;
  if (!entry.session || entry.branchWindow.isNull())
  {
    entry.runtimeStatus = AutomationWorkspaceRuntimeStatus::Dormant;
    return;
  }
  if (entry.session->model->hasAttachedSource() && entry.sourceWindow.isNull())
  {
    entry.runtimeStatus = AutomationWorkspaceRuntimeStatus::Orphaned;
    return;
  }
  entry.runtimeStatus = AutomationWorkspaceRuntimeStatus::Attached;
}

} // namespace tb::ui
