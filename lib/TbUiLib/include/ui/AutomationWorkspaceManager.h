/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QPointer>
#include <QString>

#include "mdl/MapWorkspace.h"
#include "ui/AutomationWorkspaceManifest.h"
#include "ui/AutomationWorkspaceStore.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tb::ui
{
class AppController;
class MapWindow;

struct AutomationWorkspaceInfo
{
  QString id;
  QPointer<MapWindow> sourceWindow;
  QPointer<MapWindow> branchWindow;
  std::filesystem::path directory;
  AutomationWorkspaceManifest manifest;
  AutomationWorkspaceRuntimeStatus runtimeStatus =
    AutomationWorkspaceRuntimeStatus::Dormant;
  bool sourceChanged = false;
  std::vector<AutomationWorkspaceStoreDiagnostic> diagnostics;
};

struct AutomationWorkspaceResult
{
  AutomationWorkspaceInfo* workspace = nullptr;
  std::string error;

  explicit operator bool() const { return workspace != nullptr; }
};

struct AutomationMergeResult
{
  mdl::WorkspaceMergePlan plan;
  bool applied = false;
  std::string error;
};

/** Owns recoverable copied-map workspaces used by local automation clients. */
class AutomationWorkspaceManager
{
private:
  struct Entry;

  AppController& m_appController;
  AutomationWorkspaceStore m_store;
  std::vector<std::unique_ptr<Entry>> m_entries;

public:
  explicit AutomationWorkspaceManager(AppController& appController);
  AutomationWorkspaceManager(
    AppController& appController, std::filesystem::path workspaceRoot);
  ~AutomationWorkspaceManager();

  AutomationWorkspaceResult fork(MapWindow& sourceWindow, const QString& name);
  /** Opens the current branch checkpoint using persisted loading metadata. */
  AutomationWorkspaceResult recover(const QString& id);
  /** Supplies loading context only for legacy metadata-free version 1 manifests. */
  AutomationWorkspaceResult recover(const QString& id, MapWindow& mapContextWindow);
  /** Recovers if necessary, then validates and attaches this exact live source window. */
  AutomationWorkspaceResult attachSource(const QString& id, MapWindow& sourceWindow);
  /** Publishes the current branch map and identity table as the next durable generation.
   */
  AutomationWorkspaceResult checkpoint(const QString& id);
  /** Updates the user-facing name without opening or changing a branch map. */
  AutomationWorkspaceResult rename(const QString& id, const QString& name);
  /** Persists a terminal abandoned state while retaining every workspace artifact. */
  AutomationWorkspaceResult abandon(const QString& id);
  /** Releases open documents after checkpointing while retaining the durable record. */
  AutomationWorkspaceResult close(const QString& id);
  std::vector<AutomationWorkspaceInfo*> workspaces();
  AutomationWorkspaceInfo* find(const QString& id);
  const mdl::MapWorkspace* model(const QString& id) const;
  AutomationMergeResult merge(const QString& id, bool apply);

private:
  Entry* findEntry(const QString& id) const;
  AutomationWorkspaceResult recover(Entry& entry, const MapWindow* mapContextWindow);
  AutomationWorkspaceResult checkpoint(Entry& entry);
  AutomationWorkspaceResult updateMetadata(
    Entry& entry, AutomationWorkspaceManifest manifest);
  void validateRecord(Entry& entry) const;
  void refreshRuntimeStatus(Entry& entry) const;
};

} // namespace tb::ui
