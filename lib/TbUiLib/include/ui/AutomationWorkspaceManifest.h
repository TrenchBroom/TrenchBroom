/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>

#include "mdl/MapFormat.h"

#include "vm/bbox.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <vector>

namespace tb::ui
{

enum class AutomationWorkspaceLifecycleState
{
  Active,
  Merged,
  Abandoned,
};

QString automationWorkspaceLifecycleStateName(AutomationWorkspaceLifecycleState state);
std::optional<AutomationWorkspaceLifecycleState>
automationWorkspaceLifecycleStateFromName(const QString& name);

/** Runtime-only status; it is intentionally not serialized in the manifest. */
enum class AutomationWorkspaceRuntimeStatus
{
  Dormant,
  Attached,
  SourceChanged,
  Orphaned,
  Invalid,
};

QString automationWorkspaceRuntimeStatusName(AutomationWorkspaceRuntimeStatus status);

using AutomationWorkspaceNodePath = std::vector<size_t>;

struct AutomationWorkspaceArtifact
{
  std::filesystem::path path;
  QString fingerprint;
};

struct AutomationWorkspaceSource
{
  std::filesystem::path path;
  QString fingerprintAtFork;
  size_t revisionAtFork = 0u;
};

/** Map-loading information required to recover a branch without an open source window. */
struct AutomationWorkspaceMapMetadata
{
  QString gameName;
  mdl::MapFormat mapFormat = mdl::MapFormat::Unknown;
  vm::bbox3d worldBounds;
};

struct AutomationWorkspaceNodeIdentity
{
  size_t id = 0u;
  QString type;
  AutomationWorkspaceNodePath basePath;
  size_t baseParentId = 0u;
  std::optional<AutomationWorkspaceNodePath> branchPath;
};

/**
 * The durable, process-independent description of an automation workspace.
 *
 * Runtime MapWindow pointers and document IDs deliberately do not occur here. Unknown
 * top-level JSON fields survive a read/write cycle so that version 1 does not discard
 * metadata added by a compatible producer.
 */
struct AutomationWorkspaceManifest
{
  static constexpr int SchemaVersion = 1;

  int schemaVersion = SchemaVersion;
  QString workspaceId;
  QString name;
  QDateTime createdAt;
  AutomationWorkspaceLifecycleState state = AutomationWorkspaceLifecycleState::Active;
  size_t checkpointGeneration = 0u;
  AutomationWorkspaceSource source;
  /** Missing only for pre-metadata v1 manifests, which require explicit recovery context.
   */
  std::optional<AutomationWorkspaceMapMetadata> mapMetadata;
  AutomationWorkspaceArtifact base;
  AutomationWorkspaceArtifact branch;
  std::vector<AutomationWorkspaceNodeIdentity> nodeIdentities;
  QJsonObject extraFields;

  QJsonObject toJson() const;

  /** Parses and validates schema version 1 without accessing the filesystem. */
  static std::optional<AutomationWorkspaceManifest> fromJson(
    const QJsonObject& json, QString* error = nullptr);

  /** Validates the value object, including relative artifact-path requirements. */
  bool validate(QString* error = nullptr) const;

  /**
   * Resolves a manifest-relative artifact path and verifies lexical and symlink-safe
   * containment when the artifact already exists.
   */
  static std::optional<std::filesystem::path> resolveArtifactPath(
    const std::filesystem::path& workspaceDirectory,
    const std::filesystem::path& relativePath,
    QString* error = nullptr);
};

} // namespace tb::ui
