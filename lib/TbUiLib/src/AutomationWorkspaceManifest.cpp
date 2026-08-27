/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AutomationWorkspaceManifest.h"

#include <QJsonArray>
#include <QRegularExpression>
#include <QUuid>

#include "ui/QPathUtils.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <unordered_set>
#include <utility>

namespace tb::ui
{
namespace
{

const auto fingerprintPattern = QRegularExpression{"^sha256:[0-9a-fA-F]{64}$"};

void setError(QString* error, const QString& message)
{
  if (error != nullptr)
  {
    *error = message;
  }
}

bool validFingerprint(const QString& fingerprint)
{
  return fingerprintPattern.match(fingerprint).hasMatch();
}

QJsonArray vec3ToJson(const vm::vec3d& value)
{
  return {value.x(), value.y(), value.z()};
}

std::optional<vm::vec3d> vec3FromJson(
  const QJsonValue& value, const QString& field, QString* error)
{
  if (!value.isArray() || value.toArray().size() != 3)
  {
    setError(error, QString{"%1 must be an array of three finite numbers"}.arg(field));
    return std::nullopt;
  }

  const auto values = value.toArray();
  auto result = vm::vec3d{};
  for (size_t i = 0u; i < 3u; ++i)
  {
    if (!values[static_cast<qsizetype>(i)].isDouble())
    {
      setError(error, QString{"%1 must be an array of three finite numbers"}.arg(field));
      return std::nullopt;
    }
    const auto coordinate = values[static_cast<qsizetype>(i)].toDouble();
    if (!std::isfinite(coordinate))
    {
      setError(error, QString{"%1 must be an array of three finite numbers"}.arg(field));
      return std::nullopt;
    }
    result[i] = coordinate;
  }
  return result;
}

QJsonArray pathToJson(const AutomationWorkspaceNodePath& path)
{
  auto result = QJsonArray{};
  for (const auto component : path)
  {
    result.push_back(static_cast<qint64>(component));
  }
  return result;
}

std::optional<AutomationWorkspaceNodePath> pathFromJson(
  const QJsonValue& value, const QString& field, QString* error)
{
  if (!value.isArray())
  {
    setError(error, QString{"%1 must be an array"}.arg(field));
    return std::nullopt;
  }

  auto result = AutomationWorkspaceNodePath{};
  for (const auto& component : value.toArray())
  {
    if (!component.isDouble())
    {
      setError(error, QString{"%1 must contain non-negative integers"}.arg(field));
      return std::nullopt;
    }
    const auto number = component.toDouble();
    if (
      !std::isfinite(number) || number < 0.0 || std::floor(number) != number
      || number > static_cast<double>(std::numeric_limits<size_t>::max()))
    {
      setError(error, QString{"%1 must contain non-negative integers"}.arg(field));
      return std::nullopt;
    }
    result.push_back(static_cast<size_t>(number));
  }
  return result;
}

std::optional<size_t> sizeFromJson(
  const QJsonValue& value, const QString& field, QString* error)
{
  if (!value.isDouble())
  {
    setError(error, QString{"%1 must be a non-negative integer"}.arg(field));
    return std::nullopt;
  }
  const auto number = value.toDouble();
  if (
    !std::isfinite(number) || number < 0.0 || std::floor(number) != number
    || number > static_cast<double>(std::numeric_limits<size_t>::max()))
  {
    setError(error, QString{"%1 must be a non-negative integer"}.arg(field));
    return std::nullopt;
  }
  return static_cast<size_t>(number);
}

std::optional<QString> requiredString(
  const QJsonObject& object, const QString& field, QString* error)
{
  const auto value = object.value(field);
  if (!value.isString() || value.toString().isEmpty())
  {
    setError(error, QString{"%1 must be a non-empty string"}.arg(field));
    return std::nullopt;
  }
  return value.toString();
}

bool isRelativeArtifactPath(const std::filesystem::path& path)
{
  if (path.empty() || path.is_absolute() || path.has_root_name())
  {
    return false;
  }
  return std::ranges::none_of(
    path, [](const auto& component) { return component == ".."; });
}

bool pathHasPrefix(
  const std::filesystem::path& candidate, const std::filesystem::path& directory)
{
  const auto [candidateIt, directoryIt] =
    std::mismatch(candidate.begin(), candidate.end(), directory.begin(), directory.end());
  return directoryIt == directory.end();
}

QJsonObject artifactToJson(const AutomationWorkspaceArtifact& artifact)
{
  return {
    {"path", pathAsGenericQString(artifact.path)},
    {"fingerprint", artifact.fingerprint.toLower()},
  };
}

} // namespace

QString automationWorkspaceLifecycleStateName(
  const AutomationWorkspaceLifecycleState state)
{
  switch (state)
  {
  case AutomationWorkspaceLifecycleState::Active:
    return "active";
  case AutomationWorkspaceLifecycleState::Merged:
    return "merged";
  case AutomationWorkspaceLifecycleState::Abandoned:
    return "abandoned";
  }
  return "active";
}

std::optional<AutomationWorkspaceLifecycleState>
automationWorkspaceLifecycleStateFromName(const QString& name)
{
  if (name == "active")
  {
    return AutomationWorkspaceLifecycleState::Active;
  }
  if (name == "merged")
  {
    return AutomationWorkspaceLifecycleState::Merged;
  }
  if (name == "abandoned")
  {
    return AutomationWorkspaceLifecycleState::Abandoned;
  }
  return std::nullopt;
}

QString automationWorkspaceRuntimeStatusName(
  const AutomationWorkspaceRuntimeStatus status)
{
  switch (status)
  {
  case AutomationWorkspaceRuntimeStatus::Dormant:
    return "dormant";
  case AutomationWorkspaceRuntimeStatus::Attached:
    return "attached";
  case AutomationWorkspaceRuntimeStatus::SourceChanged:
    return "sourceChanged";
  case AutomationWorkspaceRuntimeStatus::Orphaned:
    return "orphaned";
  case AutomationWorkspaceRuntimeStatus::Invalid:
    return "invalid";
  }
  return "invalid";
}

QJsonObject AutomationWorkspaceManifest::toJson() const
{
  auto result = extraFields;
  for (const auto& field :
       {"schemaVersion",
        "workspaceId",
        "name",
        "createdAt",
        "state",
        "checkpointGeneration",
        "source",
        "mapMetadata",
        "base",
        "branch",
        "nodeIdentities"})
  {
    result.remove(field);
  }

  auto identities = QJsonArray{};
  for (const auto& identity : nodeIdentities)
  {
    auto value = QJsonObject{
      {"id", static_cast<qint64>(identity.id)},
      {"type", identity.type},
      {"basePath", pathToJson(identity.basePath)},
      {"baseParentId", static_cast<qint64>(identity.baseParentId)},
    };
    value.insert(
      "branchPath",
      identity.branchPath ? QJsonValue{pathToJson(*identity.branchPath)} : QJsonValue{});
    identities.push_back(std::move(value));
  }

  result.insert("schemaVersion", SchemaVersion);
  result.insert("workspaceId", workspaceId);
  result.insert("name", name);
  result.insert("createdAt", createdAt.toUTC().toString(Qt::ISODateWithMs));
  result.insert("state", automationWorkspaceLifecycleStateName(state));
  result.insert("checkpointGeneration", static_cast<qint64>(checkpointGeneration));
  result.insert(
    "source",
    QJsonObject{
      {"path", pathAsQString(source.path)},
      {"fingerprintAtFork", source.fingerprintAtFork.toLower()},
      {"revisionAtFork", static_cast<qint64>(source.revisionAtFork)},
    });
  if (mapMetadata)
  {
    result.insert(
      "mapMetadata",
      QJsonObject{
        {"gameName", mapMetadata->gameName},
        {"mapFormat", QString::fromStdString(mdl::formatName(mapMetadata->mapFormat))},
        {"worldBounds",
         QJsonObject{
           {"min", vec3ToJson(mapMetadata->worldBounds.min)},
           {"max", vec3ToJson(mapMetadata->worldBounds.max)},
         }},
      });
  }
  result.insert("base", artifactToJson(base));
  result.insert("branch", artifactToJson(branch));
  result.insert("nodeIdentities", identities);
  return result;
}

std::optional<AutomationWorkspaceManifest> AutomationWorkspaceManifest::fromJson(
  const QJsonObject& json, QString* error)
{
  const auto schemaVersion = json.value("schemaVersion");
  if (
    !schemaVersion.isDouble() || schemaVersion.toDouble() != SchemaVersion
    || std::floor(schemaVersion.toDouble()) != schemaVersion.toDouble())
  {
    setError(error, "Unsupported workspace manifest schemaVersion");
    return std::nullopt;
  }

  auto result = AutomationWorkspaceManifest{};
  if (const auto workspaceId = requiredString(json, "workspaceId", error))
  {
    if (QUuid{*workspaceId}.isNull())
    {
      setError(error, "workspaceId must be a UUID");
      return std::nullopt;
    }
    result.workspaceId = QUuid{*workspaceId}.toString(QUuid::WithoutBraces);
  }
  else
  {
    return std::nullopt;
  }
  if (const auto name = requiredString(json, "name", error))
  {
    result.name = *name;
  }
  else
  {
    return std::nullopt;
  }

  const auto createdAt = requiredString(json, "createdAt", error);
  if (!createdAt)
  {
    return std::nullopt;
  }
  result.createdAt = QDateTime::fromString(*createdAt, Qt::ISODate);
  if (!result.createdAt.isValid())
  {
    setError(error, "createdAt must be an ISO-8601 date/time");
    return std::nullopt;
  }

  const auto state = requiredString(json, "state", error);
  const auto parsedState =
    state ? automationWorkspaceLifecycleStateFromName(*state) : std::nullopt;
  if (!parsedState)
  {
    setError(error, "state must be active, merged, or abandoned");
    return std::nullopt;
  }
  result.state = *parsedState;

  const auto checkpointGeneration =
    sizeFromJson(json.value("checkpointGeneration"), "checkpointGeneration", error);
  if (!checkpointGeneration)
  {
    return std::nullopt;
  }
  result.checkpointGeneration = *checkpointGeneration;

  const auto source = json.value("source");
  if (!source.isObject())
  {
    setError(error, "source must be an object");
    return std::nullopt;
  }
  const auto sourceObject = source.toObject();
  const auto sourcePath = requiredString(sourceObject, "path", error);
  const auto sourceFingerprint = requiredString(sourceObject, "fingerprintAtFork", error);
  const auto sourceRevision =
    sizeFromJson(sourceObject.value("revisionAtFork"), "source.revisionAtFork", error);
  if (!sourcePath || !sourceFingerprint || !sourceRevision)
  {
    return std::nullopt;
  }
  result.source.path = pathFromQString(*sourcePath);
  result.source.fingerprintAtFork = sourceFingerprint->toLower();
  result.source.revisionAtFork = *sourceRevision;

  if (json.contains("mapMetadata"))
  {
    const auto metadata = json.value("mapMetadata");
    if (!metadata.isObject())
    {
      setError(error, "mapMetadata must be an object");
      return std::nullopt;
    }
    const auto metadataObject = metadata.toObject();
    const auto gameName = requiredString(metadataObject, "gameName", error);
    const auto formatName = requiredString(metadataObject, "mapFormat", error);
    const auto worldBounds = metadataObject.value("worldBounds");
    if (!gameName || !formatName || !worldBounds.isObject())
    {
      if (worldBounds.isUndefined() || !worldBounds.isObject())
      {
        setError(error, "mapMetadata.worldBounds must be an object");
      }
      return std::nullopt;
    }
    const auto mapFormat = mdl::formatFromName(formatName->toStdString());
    if (mapFormat == mdl::MapFormat::Unknown)
    {
      setError(error, "mapMetadata.mapFormat must be a known map format");
      return std::nullopt;
    }
    const auto boundsObject = worldBounds.toObject();
    const auto min =
      vec3FromJson(boundsObject.value("min"), "mapMetadata.worldBounds.min", error);
    const auto max =
      vec3FromJson(boundsObject.value("max"), "mapMetadata.worldBounds.max", error);
    if (!min || !max)
    {
      return std::nullopt;
    }
    if (!vm::bbox3d::is_valid(*min, *max))
    {
      setError(error, "mapMetadata.worldBounds min must not exceed max");
      return std::nullopt;
    }
    result.mapMetadata =
      AutomationWorkspaceMapMetadata{*gameName, mapFormat, {*min, *max}};
  }

  const auto parseArtifact =
    [&](const QString& field, AutomationWorkspaceArtifact& artifact) -> bool {
    const auto value = json.value(field);
    if (!value.isObject())
    {
      setError(error, QString{"%1 must be an object"}.arg(field));
      return false;
    }
    const auto object = value.toObject();
    const auto path = requiredString(object, "path", error);
    const auto fingerprint = requiredString(object, "fingerprint", error);
    if (!path || !fingerprint)
    {
      return false;
    }
    artifact.path = pathFromQString(*path);
    artifact.fingerprint = fingerprint->toLower();
    return true;
  };
  if (!parseArtifact("base", result.base) || !parseArtifact("branch", result.branch))
  {
    return std::nullopt;
  }

  const auto identities = json.value("nodeIdentities");
  if (!identities.isArray())
  {
    setError(error, "nodeIdentities must be an array");
    return std::nullopt;
  }
  for (const auto& value : identities.toArray())
  {
    if (!value.isObject())
    {
      setError(error, "nodeIdentities must contain objects");
      return std::nullopt;
    }
    const auto object = value.toObject();
    const auto id = sizeFromJson(object.value("id"), "nodeIdentities.id", error);
    const auto type = requiredString(object, "type", error);
    const auto basePath =
      pathFromJson(object.value("basePath"), "nodeIdentities.basePath", error);
    const auto baseParentId =
      sizeFromJson(object.value("baseParentId"), "nodeIdentities.baseParentId", error);
    if (!id || !type || !basePath || !baseParentId)
    {
      return std::nullopt;
    }
    auto identity =
      AutomationWorkspaceNodeIdentity{*id, *type, *basePath, *baseParentId, std::nullopt};
    if (!object.contains("branchPath"))
    {
      setError(error, "nodeIdentities.branchPath must be an array or null");
      return std::nullopt;
    }
    const auto branchPath = object.value("branchPath");
    if (!branchPath.isNull())
    {
      identity.branchPath = pathFromJson(branchPath, "nodeIdentities.branchPath", error);
      if (!identity.branchPath)
      {
        return std::nullopt;
      }
    }
    result.nodeIdentities.push_back(std::move(identity));
  }

  for (const auto& field : json.keys())
  {
    if (
      field != "schemaVersion" && field != "workspaceId" && field != "name"
      && field != "createdAt" && field != "state" && field != "checkpointGeneration"
      && field != "source" && field != "mapMetadata" && field != "base"
      && field != "branch" && field != "nodeIdentities")
    {
      result.extraFields.insert(field, json.value(field));
    }
  }

  if (!result.validate(error))
  {
    return std::nullopt;
  }
  return result;
}

bool AutomationWorkspaceManifest::validate(QString* error) const
{
  if (schemaVersion != SchemaVersion)
  {
    setError(error, "Unsupported workspace manifest schemaVersion");
    return false;
  }
  if (workspaceId.isEmpty() || QUuid{workspaceId}.isNull())
  {
    setError(error, "workspaceId must be a UUID");
    return false;
  }
  if (name.isEmpty())
  {
    setError(error, "name must be a non-empty string");
    return false;
  }
  if (!createdAt.isValid())
  {
    setError(error, "createdAt must be an ISO-8601 date/time");
    return false;
  }
  if (source.path.empty() || !source.path.is_absolute())
  {
    setError(error, "source.path must be absolute");
    return false;
  }
  if (!validFingerprint(source.fingerprintAtFork))
  {
    setError(error, "source.fingerprintAtFork must be a SHA-256 fingerprint");
    return false;
  }
  if (
    mapMetadata
    && (mapMetadata->gameName.isEmpty()
        || mapMetadata->mapFormat == mdl::MapFormat::Unknown
        || !mapMetadata->worldBounds.is_valid()))
  {
    setError(
      error, "mapMetadata must contain a game name, known map format, and valid bounds");
    return false;
  }
  for (const auto* artifact : {&base, &branch})
  {
    if (!isRelativeArtifactPath(artifact->path))
    {
      setError(error, "base.path and branch.path must be non-empty relative paths");
      return false;
    }
    if (!validFingerprint(artifact->fingerprint))
    {
      setError(
        error, "base.fingerprint and branch.fingerprint must be SHA-256 fingerprints");
      return false;
    }
  }

  auto ids = std::unordered_set<size_t>{};
  for (const auto& identity : nodeIdentities)
  {
    if (identity.id == 0u || identity.type.isEmpty() || !ids.insert(identity.id).second)
    {
      setError(error, "nodeIdentities must have unique non-zero IDs and non-empty types");
      return false;
    }
    if (identity.baseParentId == identity.id)
    {
      setError(error, "node identity cannot be its own base parent");
      return false;
    }
  }
  for (const auto& identity : nodeIdentities)
  {
    if (identity.baseParentId != 0u && !ids.contains(identity.baseParentId))
    {
      setError(error, "node identity base parent must be another fork-time identity");
      return false;
    }
  }
  return true;
}

std::optional<std::filesystem::path> AutomationWorkspaceManifest::resolveArtifactPath(
  const std::filesystem::path& workspaceDirectory,
  const std::filesystem::path& relativePath,
  QString* error)
{
  if (!isRelativeArtifactPath(relativePath))
  {
    setError(
      error, "Artifact path must be a non-empty relative path beneath the workspace");
    return std::nullopt;
  }

  auto ec = std::error_code{};
  const auto workspace =
    std::filesystem::absolute(workspaceDirectory, ec).lexically_normal();
  if (ec)
  {
    setError(error, "Could not resolve workspace directory");
    return std::nullopt;
  }
  const auto resolved = (workspace / relativePath).lexically_normal();
  if (!pathHasPrefix(resolved, workspace))
  {
    setError(error, "Artifact path escapes the workspace directory");
    return std::nullopt;
  }

  const auto canonicalWorkspace = std::filesystem::weakly_canonical(workspace, ec);
  if (ec)
  {
    setError(error, "Could not canonicalize workspace directory");
    return std::nullopt;
  }
  const auto canonicalArtifact = std::filesystem::weakly_canonical(resolved, ec);
  if (ec || !pathHasPrefix(canonicalArtifact, canonicalWorkspace))
  {
    setError(error, "Artifact path escapes the workspace directory through a symlink");
    return std::nullopt;
  }

  return resolved;
}

} // namespace tb::ui
