/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationWorkspaceManager.h"

#include <QUuid>

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
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
#include "ui/SystemPaths.h"

#include "kd/overload.h"

#include <map>
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
  mdl::Map& map,
  const mdl::MapWorkspace& workspace,
  const mdl::GroupNode& branchGroup)
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
         || map.executeAndStore(std::make_unique<mdl::SetLinkIdsCommand>(
           "Merge Workspace Link IDs", std::move(linkIds)));
}

} // namespace

AutomationWorkspaceManager::AutomationWorkspaceManager(AppController& appController)
  : m_appController{appController}
{
}

AutomationWorkspaceResult AutomationWorkspaceManager::fork(
  MapWindow& sourceWindow, const QString& /* name */)
{
  auto& sourceMap = sourceWindow.document().map();
  const auto id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  const auto directory =
    SystemPaths::userDataDirectory() / "automation" / "workspaces" / id.toStdString();
  const auto basePath = directory / "base.map";
  const auto branchPath = directory / "branch.map";

  auto ec = std::error_code{};
  std::filesystem::create_directories(directory, ec);
  if (ec)
  {
    return {nullptr, "Could not create workspace directory: " + ec.message()};
  }
  const auto baseSnapshot = sourceMap.saveTo(basePath);
  const auto branchSnapshot = sourceMap.saveTo(branchPath);
  auto baseCheckError = std::error_code{};
  const auto baseExists = std::filesystem::is_regular_file(basePath, baseCheckError);
  auto branchCheckError = std::error_code{};
  const auto branchExists =
    std::filesystem::is_regular_file(branchPath, branchCheckError);
  if (
    baseSnapshot.is_error() || branchSnapshot.is_error() || !baseExists || !branchExists
    || baseCheckError || branchCheckError)
  {
    return {nullptr, "Could not snapshot the source map"};
  }

  auto loaded = m_appController.mapWindowManager().loadDocumentInNewWindow(
    sourceMap.gameInfo(),
    sourceMap.worldNode().mapFormat(),
    sourceMap.worldBounds(),
    branchPath);
  if (loaded.is_error())
  {
    return {nullptr, "Could not open the branch map"};
  }

  auto* branchWindow = loaded.value();
  auto entry = std::make_unique<Entry>();
  entry->id = id;
  entry->sourceWindow = &sourceWindow;
  entry->branchWindow = branchWindow;
  entry->directory = directory;
  try
  {
    entry->model = std::make_unique<mdl::MapWorkspace>(
      sourceMap.worldNode(),
      branchWindow->document().map().worldNode(),
      sourceMap.worldBounds());
  }
  catch (const std::exception& e)
  {
    branchWindow->close();
    return {nullptr, std::string{"Could not attach branch map: "} + e.what()};
  }

  auto* result = entry.get();
  m_entries.push_back(std::move(entry));
  return {result, {}};
}

std::vector<AutomationWorkspaceInfo*> AutomationWorkspaceManager::workspaces()
{
  auto result = std::vector<AutomationWorkspaceInfo*>{};
  for (auto& entry : m_entries)
  {
    result.push_back(entry.get());
  }
  return result;
}

AutomationWorkspaceInfo* AutomationWorkspaceManager::find(const QString& id)
{
  return findEntry(id);
}

const mdl::MapWorkspace* AutomationWorkspaceManager::model(const QString& id) const
{
  const auto* entry = findEntry(id);
  return entry != nullptr ? entry->model.get() : nullptr;
}

AutomationMergeResult AutomationWorkspaceManager::merge(
  const QString& id, const bool apply)
{
  auto* entry = findEntry(id);
  if (entry == nullptr || entry->sourceWindow.isNull() || entry->branchWindow.isNull())
  {
    return {{}, false, "Unknown workspace or one of its windows was closed"};
  }
  if (entry->merged)
  {
    return {{}, false, "This workspace has already been merged; fork a new workspace"};
  }

  auto& map = entry->sourceWindow->document().map();
  if (
    entry->model->sourceNode(1u) != &map.worldNode()
    || &entry->model->branchWorld() != &entry->branchWindow->document().map().worldNode())
  {
    return {{}, false, "A workspace window loaded a different document"};
  }
  auto plan = entry->model->planMerge(map.worldNode());
  if (!apply || !plan.canApply())
  {
    return {std::move(plan), false, {}};
  }
  if (plan.operations.empty())
  {
    entry->merged = true;
    return {std::move(plan), true, {}};
  }

  auto transaction = mdl::Transaction{map, "Merge Automation Workspace"};
  for (const auto& operation : plan.operations)
  {
    auto* liveNode = const_cast<mdl::Node*>(entry->model->sourceNode(operation.nodeId));
    switch (operation.kind)
    {
    case mdl::WorkspaceMergeOperationKind::Add: {
      auto* parent =
        operation.parentId
          ? const_cast<mdl::Node*>(entry->model->sourceNode(*operation.parentId))
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
      if (const auto* branchGroup = dynamic_cast<const mdl::GroupNode*>(operation.branchNode))
      {
        if (!copyLinkedGroupIds(map, *entry->model, *branchGroup))
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
      auto* parent =
        const_cast<mdl::Node*>(entry->model->sourceNode(*operation.parentId));
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
  entry->merged = true;
  return {std::move(plan), true, {}};
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

} // namespace tb::ui
