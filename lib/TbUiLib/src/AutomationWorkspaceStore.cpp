/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationWorkspaceStore.h"

#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>
#include <QUuid>

#include "ui/QPathUtils.h"

#include <algorithm>
#include <system_error>
#include <utility>

namespace tb::ui
{
namespace
{

constexpr auto ManifestFileName = "workspace.json";
constexpr auto BaseFileName = "base.map";
constexpr auto SnapshotsDirectoryName = "snapshots";

void addDiagnostic(
  AutomationWorkspaceStoreRecord& record,
  const AutomationWorkspaceStoreDiagnosticKind kind,
  const QString& message,
  const std::filesystem::path& path)
{
  record.diagnostics.push_back({kind, message, path});
}

QString formatFileError(const QString& operation, const std::filesystem::path& path)
{
  return QString{"%1: %2"}.arg(operation, pathAsQString(path));
}

std::optional<QString> fingerprint(const std::filesystem::path& path, QString* error)
{
  auto file = QFile{pathAsQString(path)};
  if (!file.open(QIODevice::ReadOnly))
  {
    if (error != nullptr)
    {
      *error = formatFileError("Could not read workspace artifact", path);
    }
    return std::nullopt;
  }

  auto hash = QCryptographicHash{QCryptographicHash::Sha256};
  while (!file.atEnd())
  {
    const auto chunk = file.read(64 * 1024);
    if (chunk.isEmpty() && file.error() != QFile::NoError)
    {
      if (error != nullptr)
      {
        *error = formatFileError("Could not hash workspace artifact", path);
      }
      return std::nullopt;
    }
    hash.addData(chunk);
  }
  return "sha256:" + QString::fromLatin1(hash.result().toHex());
}

bool copyFileAtomically(
  const std::filesystem::path& source,
  const std::filesystem::path& destination,
  QString* error)
{
  auto input = QFile{pathAsQString(source)};
  if (!input.open(QIODevice::ReadOnly))
  {
    if (error != nullptr)
    {
      *error = formatFileError("Could not read source map", source);
    }
    return false;
  }

  auto output = QSaveFile{pathAsQString(destination)};
  if (!output.open(QIODevice::WriteOnly))
  {
    if (error != nullptr)
    {
      *error = formatFileError("Could not create workspace artifact", destination);
    }
    return false;
  }

  while (!input.atEnd())
  {
    const auto chunk = input.read(64 * 1024);
    if (chunk.isEmpty() && input.error() != QFile::NoError)
    {
      if (error != nullptr)
      {
        *error = formatFileError("Could not read source map", source);
      }
      output.cancelWriting();
      return false;
    }
    if (output.write(chunk) != chunk.size())
    {
      if (error != nullptr)
      {
        *error = formatFileError("Could not write workspace artifact", destination);
      }
      output.cancelWriting();
      return false;
    }
  }
  if (!output.commit())
  {
    if (error != nullptr)
    {
      *error = formatFileError("Could not publish workspace artifact", destination);
    }
    return false;
  }
  return true;
}

bool writeManifestAtomically(
  const std::filesystem::path& path,
  const AutomationWorkspaceManifest& manifest,
  QString* error)
{
  auto output = QSaveFile{pathAsQString(path)};
  if (!output.open(QIODevice::WriteOnly))
  {
    if (error != nullptr)
    {
      *error = formatFileError("Could not create workspace manifest", path);
    }
    return false;
  }
  const auto json = QJsonDocument{manifest.toJson()}.toJson(QJsonDocument::Indented);
  if (output.write(json) != json.size() || !output.commit())
  {
    if (error != nullptr)
    {
      *error = formatFileError("Could not publish workspace manifest", path);
    }
    return false;
  }
  return true;
}

std::optional<AutomationWorkspaceManifest> readManifest(
  const std::filesystem::path& path, AutomationWorkspaceStoreRecord& record)
{
  auto file = QFile{pathAsQString(path)};
  if (!file.exists())
  {
    addDiagnostic(
      record,
      AutomationWorkspaceStoreDiagnosticKind::ManifestMissing,
      "Workspace manifest is missing",
      path);
    return std::nullopt;
  }
  if (!file.open(QIODevice::ReadOnly))
  {
    addDiagnostic(
      record,
      AutomationWorkspaceStoreDiagnosticKind::ManifestUnreadable,
      "Workspace manifest could not be read",
      path);
    return std::nullopt;
  }

  auto jsonError = QJsonParseError{};
  const auto document = QJsonDocument::fromJson(file.readAll(), &jsonError);
  if (jsonError.error != QJsonParseError::NoError || !document.isObject())
  {
    addDiagnostic(
      record,
      AutomationWorkspaceStoreDiagnosticKind::ManifestInvalid,
      "Workspace manifest is not a JSON object",
      path);
    return std::nullopt;
  }

  QString error;
  const auto manifest = AutomationWorkspaceManifest::fromJson(document.object(), &error);
  if (!manifest)
  {
    const auto kind =
      document.object().contains("schemaVersion")
          && error.startsWith("Unsupported workspace manifest schemaVersion")
        ? AutomationWorkspaceStoreDiagnosticKind::UnsupportedSchemaVersion
        : AutomationWorkspaceStoreDiagnosticKind::ManifestInvalid;
    addDiagnostic(record, kind, error, path);
  }
  return manifest;
}

bool validateArtifact(
  const std::filesystem::path& directory,
  const AutomationWorkspaceArtifact& artifact,
  const bool base,
  AutomationWorkspaceStoreRecord& record)
{
  QString error;
  const auto path =
    AutomationWorkspaceManifest::resolveArtifactPath(directory, artifact.path, &error);
  if (!path)
  {
    addDiagnostic(
      record,
      AutomationWorkspaceStoreDiagnosticKind::PathEscapesWorkspace,
      error,
      directory / artifact.path);
    return false;
  }

  auto ec = std::error_code{};
  if (!std::filesystem::is_regular_file(*path, ec) || ec)
  {
    addDiagnostic(
      record,
      base ? AutomationWorkspaceStoreDiagnosticKind::BaseMissing
           : AutomationWorkspaceStoreDiagnosticKind::BranchMissing,
      base ? "Workspace base map is missing" : "Workspace branch checkpoint is missing",
      *path);
    return false;
  }
  const auto actual = fingerprint(*path, &error);
  if (!actual)
  {
    addDiagnostic(
      record,
      base ? AutomationWorkspaceStoreDiagnosticKind::BaseMissing
           : AutomationWorkspaceStoreDiagnosticKind::BranchMissing,
      error,
      *path);
    return false;
  }
  if (*actual != artifact.fingerprint)
  {
    addDiagnostic(
      record,
      base ? AutomationWorkspaceStoreDiagnosticKind::BaseFingerprintMismatch
           : AutomationWorkspaceStoreDiagnosticKind::BranchFingerprintMismatch,
      base ? "Workspace base map fingerprint does not match the manifest"
           : "Workspace branch checkpoint fingerprint does not match the manifest",
      *path);
    return false;
  }
  return true;
}

bool validateManifestArtifacts(
  const AutomationWorkspaceManifest& manifest,
  const std::filesystem::path& directory,
  AutomationWorkspaceStoreRecord& record)
{
  return validateArtifact(directory, manifest.base, true, record)
         && validateArtifact(directory, manifest.branch, false, record);
}

std::optional<size_t> generationFromDirectoryName(const std::filesystem::path& path)
{
  const auto name = QString::fromStdString(path.filename().string());
  auto ok = false;
  const auto generation = name.toULongLong(&ok);
  if (!ok || QString::number(generation) != name)
  {
    return std::nullopt;
  }
  return static_cast<size_t>(generation);
}

bool snapshotMatchesDirectory(
  const AutomationWorkspaceManifest& manifest,
  const std::filesystem::path& workspaceDirectory,
  const size_t generation)
{
  return manifest.checkpointGeneration == generation
         && manifest.branch.path
              == std::filesystem::path{SnapshotsDirectoryName}
                   / std::to_string(generation) / "branch.map"
         && AutomationWorkspaceManifest::resolveArtifactPath(
              workspaceDirectory, manifest.branch.path)
              .has_value();
}

bool sameArtifact(
  const AutomationWorkspaceArtifact& lhs, const AutomationWorkspaceArtifact& rhs)
{
  return lhs.path == rhs.path && lhs.fingerprint == rhs.fingerprint;
}

bool sameNodeIdentities(
  const std::vector<AutomationWorkspaceNodeIdentity>& lhs,
  const std::vector<AutomationWorkspaceNodeIdentity>& rhs)
{
  return lhs.size() == rhs.size()
         && std::ranges::equal(lhs, rhs, [](const auto& left, const auto& right) {
              return left.id == right.id && left.type == right.type
                     && left.basePath == right.basePath
                     && left.baseParentId == right.baseParentId
                     && left.branchPath == right.branchPath;
            });
}

bool sameMapMetadata(
  const std::optional<AutomationWorkspaceMapMetadata>& lhs,
  const std::optional<AutomationWorkspaceMapMetadata>& rhs)
{
  if (lhs.has_value() != rhs.has_value())
  {
    return false;
  }
  return !lhs
         || (lhs->gameName == rhs->gameName && lhs->mapFormat == rhs->mapFormat
             && lhs->worldBounds == rhs->worldBounds);
}

bool onlyMetadataChanged(
  const AutomationWorkspaceManifest& current, const AutomationWorkspaceManifest& updated)
{
  return current.schemaVersion == updated.schemaVersion
         && current.workspaceId == updated.workspaceId
         && current.createdAt == updated.createdAt
         && current.checkpointGeneration == updated.checkpointGeneration
         && current.source.fingerprintAtFork == updated.source.fingerprintAtFork
         && current.source.revisionAtFork == updated.source.revisionAtFork
         && sameMapMetadata(current.mapMetadata, updated.mapMetadata)
         && sameArtifact(current.base, updated.base)
         && sameArtifact(current.branch, updated.branch)
         && sameNodeIdentities(current.nodeIdentities, updated.nodeIdentities)
         && current.extraFields == updated.extraFields;
}

bool validMetadataStateTransition(
  const AutomationWorkspaceLifecycleState current,
  const AutomationWorkspaceLifecycleState updated)
{
  return current == updated
         || (current == AutomationWorkspaceLifecycleState::Active && updated == AutomationWorkspaceLifecycleState::Abandoned);
}

} // namespace

bool AutomationWorkspaceStoreRecord::valid() const
{
  return manifest.has_value();
}

AutomationWorkspaceStore::AutomationWorkspaceStore(std::filesystem::path rootDirectory)
  : m_rootDirectory{std::move(rootDirectory)}
{
}

const std::filesystem::path& AutomationWorkspaceStore::rootDirectory() const
{
  return m_rootDirectory;
}

std::filesystem::path AutomationWorkspaceStore::workspaceDirectory(
  const QString& workspaceId) const
{
  return m_rootDirectory / workspaceId.toStdString();
}

AutomationWorkspaceStoreResult AutomationWorkspaceStore::create(
  AutomationWorkspaceManifest manifest,
  const std::filesystem::path& baseMap,
  const std::filesystem::path& branchMap)
{
  const auto workspaceUuid = QUuid{manifest.workspaceId};
  if (workspaceUuid.isNull())
  {
    return {{}, "workspaceId must be a UUID before creating a workspace"};
  }
  manifest.workspaceId = workspaceUuid.toString(QUuid::WithoutBraces);

  const auto directory = workspaceDirectory(manifest.workspaceId);
  auto ec = std::error_code{};
  if (std::filesystem::exists(directory, ec) || ec)
  {
    return {{}, "A workspace with this ID already exists"};
  }
  std::filesystem::create_directories(m_rootDirectory, ec);
  if (ec)
  {
    return {{}, "Could not create the workspace root directory"};
  }

  const auto stagingDirectory =
    m_rootDirectory
    / ("." + manifest.workspaceId.toStdString() + ".tmp-"
       + QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString());
  std::filesystem::create_directories(
    stagingDirectory / SnapshotsDirectoryName / "0", ec);
  if (ec)
  {
    return {{}, "Could not create a staging workspace directory"};
  }

  QString error;
  const auto baseDestination = stagingDirectory / BaseFileName;
  const auto branchDestination =
    stagingDirectory / SnapshotsDirectoryName / "0" / "branch.map";
  if (
    !copyFileAtomically(baseMap, baseDestination, &error)
    || !copyFileAtomically(branchMap, branchDestination, &error))
  {
    return {{}, error};
  }

  const auto baseFingerprint = fingerprint(baseDestination, &error);
  const auto branchFingerprint = fingerprint(branchDestination, &error);
  if (!baseFingerprint || !branchFingerprint)
  {
    return {{}, error};
  }
  manifest.checkpointGeneration = 0u;
  manifest.base = {BaseFileName, *baseFingerprint};
  manifest.branch = {
    std::filesystem::path{SnapshotsDirectoryName} / "0" / "branch.map",
    *branchFingerprint};
  if (!manifest.validate(&error))
  {
    return {{}, error};
  }
  if (
    !writeManifestAtomically(
      stagingDirectory / SnapshotsDirectoryName / "0" / ManifestFileName,
      manifest,
      &error)
    || !writeManifestAtomically(stagingDirectory / ManifestFileName, manifest, &error))
  {
    return {{}, error};
  }

  std::filesystem::rename(stagingDirectory, directory, ec);
  if (ec)
  {
    return {{}, "Could not publish the workspace directory"};
  }
  return {manifest, {}};
}

AutomationWorkspaceStoreResult AutomationWorkspaceStore::publishCheckpoint(
  const AutomationWorkspaceManifest& manifest, const std::filesystem::path& branchMap)
{
  if (QUuid{manifest.workspaceId}.isNull())
  {
    return {{}, "workspaceId must be a UUID before publishing a checkpoint"};
  }
  const auto directory = workspaceDirectory(manifest.workspaceId);
  const auto current = read(directory);
  if (!current.manifest)
  {
    return {{}, "Workspace has no valid published checkpoint"};
  }
  if (
    current.manifest->workspaceId != manifest.workspaceId
    || current.manifest->checkpointGeneration != manifest.checkpointGeneration)
  {
    return {
      {}, "Workspace checkpoint is stale; read the current manifest before publishing"};
  }

  const auto generation = manifest.checkpointGeneration + 1u;
  const auto generationDirectory =
    directory / SnapshotsDirectoryName / std::to_string(generation);
  auto ec = std::error_code{};
  if (std::filesystem::exists(generationDirectory, ec) || ec)
  {
    return {{}, "The next workspace checkpoint generation already exists"};
  }
  std::filesystem::create_directories(generationDirectory, ec);
  if (ec)
  {
    return {{}, "Could not create the next workspace checkpoint directory"};
  }

  QString error;
  const auto branchDestination = generationDirectory / "branch.map";
  if (!copyFileAtomically(branchMap, branchDestination, &error))
  {
    return {{}, error};
  }
  const auto branchFingerprint = fingerprint(branchDestination, &error);
  if (!branchFingerprint)
  {
    return {{}, error};
  }

  auto published = manifest;
  published.checkpointGeneration = generation;
  published.branch = {
    std::filesystem::path{SnapshotsDirectoryName} / std::to_string(generation)
      / "branch.map",
    *branchFingerprint};
  if (!published.validate(&error))
  {
    return {{}, error};
  }
  if (!writeManifestAtomically(generationDirectory / ManifestFileName, published, &error))
  {
    return {{}, error};
  }
  if (!writeManifestAtomically(directory / ManifestFileName, published, &error))
  {
    return {{}, error};
  }
  return {published, {}};
}

AutomationWorkspaceStoreResult AutomationWorkspaceStore::updateMetadata(
  const AutomationWorkspaceManifest& manifest)
{
  if (QUuid{manifest.workspaceId}.isNull())
  {
    return {{}, "workspaceId must be a UUID before updating workspace metadata"};
  }
  const auto directory = workspaceDirectory(manifest.workspaceId);
  const auto current = read(directory);
  if (!current.manifest)
  {
    return {{}, "Workspace has no valid published checkpoint"};
  }

  QString error;
  if (!manifest.validate(&error))
  {
    return {{}, error};
  }
  if (!onlyMetadataChanged(*current.manifest, manifest))
  {
    return {{}, "Workspace metadata update would alter immutable checkpoint data"};
  }
  if (!validMetadataStateTransition(current.manifest->state, manifest.state))
  {
    return {{}, "Workspace metadata update has an invalid lifecycle state transition"};
  }
  if (!writeManifestAtomically(directory / ManifestFileName, manifest, &error))
  {
    return {{}, error};
  }
  return {manifest, {}};
}

AutomationWorkspaceStoreRecord AutomationWorkspaceStore::read(
  const std::filesystem::path& directory) const
{
  auto record = AutomationWorkspaceStoreRecord{};
  record.directory = directory;

  auto ec = std::error_code{};
  if (!std::filesystem::is_directory(directory, ec) || ec)
  {
    addDiagnostic(
      record,
      AutomationWorkspaceStoreDiagnosticKind::ManifestMissing,
      "Workspace directory is missing",
      directory);
    return record;
  }

  const auto rootManifest = readManifest(directory / ManifestFileName, record);
  auto candidates = std::vector<std::pair<size_t, AutomationWorkspaceManifest>>{};
  if (rootManifest && validateManifestArtifacts(*rootManifest, directory, record))
  {
    candidates.emplace_back(rootManifest->checkpointGeneration, *rootManifest);
  }

  const auto snapshotsDirectory = directory / SnapshotsDirectoryName;
  if (std::filesystem::is_directory(snapshotsDirectory, ec) && !ec)
  {
    for (const auto& entry : std::filesystem::directory_iterator{snapshotsDirectory, ec})
    {
      if (ec)
      {
        break;
      }
      if (!entry.is_directory())
      {
        continue;
      }
      const auto generation = generationFromDirectoryName(entry.path());
      if (!generation)
      {
        continue;
      }

      const auto diagnosticCount = record.diagnostics.size();
      const auto manifest = readManifest(entry.path() / ManifestFileName, record);
      if (
        manifest && snapshotMatchesDirectory(*manifest, directory, *generation)
        && validateManifestArtifacts(*manifest, directory, record))
      {
        candidates.emplace_back(*generation, *manifest);
      }
      else
      {
        if (record.diagnostics.size() == diagnosticCount)
        {
          addDiagnostic(
            record,
            AutomationWorkspaceStoreDiagnosticKind::SnapshotIgnored,
            "Workspace snapshot manifest does not match its generation directory",
            entry.path() / ManifestFileName);
        }
      }
    }
  }

  if (!candidates.empty())
  {
    const auto selected = std::ranges::max_element(
      candidates, {}, [](const auto& candidate) { return candidate.first; });
    record.manifest = selected->second;
    if (!rootManifest || rootManifest->checkpointGeneration != selected->first)
    {
      addDiagnostic(
        record,
        AutomationWorkspaceStoreDiagnosticKind::SnapshotIgnored,
        "Recovered the latest complete workspace checkpoint instead of the root manifest",
        directory / ManifestFileName);
    }
  }

  if (
    record.manifest
    && record.manifest->workspaceId
         != QString::fromStdString(directory.filename().string()))
  {
    addDiagnostic(
      record,
      AutomationWorkspaceStoreDiagnosticKind::WorkspaceIdMismatch,
      "Workspace manifest ID does not match its directory name",
      directory / ManifestFileName);
    record.manifest.reset();
  }
  return record;
}

std::vector<AutomationWorkspaceStoreRecord> AutomationWorkspaceStore::scan() const
{
  auto records = std::vector<AutomationWorkspaceStoreRecord>{};
  auto ec = std::error_code{};
  if (!std::filesystem::is_directory(m_rootDirectory, ec) || ec)
  {
    return records;
  }

  auto directories = std::vector<std::filesystem::path>{};
  for (const auto& entry : std::filesystem::directory_iterator{m_rootDirectory, ec})
  {
    if (ec)
    {
      break;
    }
    if (entry.is_directory() && entry.path().filename().string().front() != '.')
    {
      directories.push_back(entry.path());
    }
  }
  std::ranges::sort(directories);
  for (const auto& directory : directories)
  {
    records.push_back(read(directory));
  }
  return records;
}

} // namespace tb::ui
