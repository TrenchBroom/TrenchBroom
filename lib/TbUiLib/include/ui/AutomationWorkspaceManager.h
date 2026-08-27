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
  struct Entry : AutomationWorkspaceInfo
  {
    std::unique_ptr<mdl::MapWorkspace> model;
    bool merged = false;
  };

  AppController& m_appController;
  std::vector<std::unique_ptr<Entry>> m_entries;

public:
  explicit AutomationWorkspaceManager(AppController& appController);

  AutomationWorkspaceResult fork(MapWindow& sourceWindow, const QString& name);
  std::vector<AutomationWorkspaceInfo*> workspaces();
  AutomationWorkspaceInfo* find(const QString& id);
  const mdl::MapWorkspace* model(const QString& id) const;
  AutomationMergeResult merge(const QString& id, bool apply);

private:
  Entry* findEntry(const QString& id) const;
};

} // namespace tb::ui
