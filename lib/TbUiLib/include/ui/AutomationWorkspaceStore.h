/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QString>

#include "ui/AutomationWorkspaceManifest.h"

#include <filesystem>
#include <optional>
#include <vector>

namespace tb::ui
{

enum class AutomationWorkspaceStoreDiagnosticKind
{
  ManifestMissing,
  ManifestUnreadable,
  ManifestInvalid,
  UnsupportedSchemaVersion,
  WorkspaceIdMismatch,
  PathEscapesWorkspace,
  BaseMissing,
  BaseFingerprintMismatch,
  BranchMissing,
  BranchFingerprintMismatch,
  SnapshotIgnored,
};

struct AutomationWorkspaceStoreDiagnostic
{
  AutomationWorkspaceStoreDiagnosticKind kind;
  QString message;
  std::filesystem::path path;
};

struct AutomationWorkspaceStoreRecord
{
  std::filesystem::path directory;
  std::optional<AutomationWorkspaceManifest> manifest;
  std::vector<AutomationWorkspaceStoreDiagnostic> diagnostics;

  bool valid() const;
};

struct AutomationWorkspaceStoreResult
{
  std::optional<AutomationWorkspaceManifest> manifest;
  QString error;

  explicit operator bool() const { return manifest.has_value(); }
};

/**
 * Owns only durable workspace files. It never opens documents or mutates a map.
 *
 * A checkpoint is published in this order: branch bytes, generation manifest, then the
 * root manifest. Readers select the newest fully valid generation, so a crash before
 * root-manifest publication leaves the prior checkpoint usable.
 */
class AutomationWorkspaceStore
{
private:
  std::filesystem::path m_rootDirectory;

public:
  explicit AutomationWorkspaceStore(std::filesystem::path rootDirectory);

  const std::filesystem::path& rootDirectory() const;
  std::filesystem::path workspaceDirectory(const QString& workspaceId) const;

  /** Creates a new workspace and publishes generation zero. */
  AutomationWorkspaceStoreResult create(
    AutomationWorkspaceManifest manifest,
    const std::filesystem::path& baseMap,
    const std::filesystem::path& branchMap);

  /** Copies branchMap into the next generation and atomically advances workspace.json. */
  AutomationWorkspaceStoreResult publishCheckpoint(
    const AutomationWorkspaceManifest& manifest, const std::filesystem::path& branchMap);

  /**
   * Atomically updates durable user-facing/lifecycle metadata without changing the
   * checkpoint generation or any map artifact. The supplied manifest must have been
   * derived from the current published manifest.
   */
  AutomationWorkspaceStoreResult updateMetadata(
    const AutomationWorkspaceManifest& manifest);

  /** Read-only validation of one workspace directory. */
  AutomationWorkspaceStoreRecord read(
    const std::filesystem::path& workspaceDirectory) const;

  /** Read-only discovery of direct workspace children in deterministic name order. */
  std::vector<AutomationWorkspaceStoreRecord> scan() const;
};

} // namespace tb::ui
