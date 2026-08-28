/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceEvidence.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QUuid>

#include "ui/QPathUtils.h"

#include <algorithm>
#include <cctype>
#include <system_error>
#include <utility>
#include <variant>

namespace tb::ui
{
namespace
{

AcceptanceEvidenceError error(std::string message)
{
  return {std::move(message)};
}

std::optional<std::string> sha256(const std::filesystem::path& path)
{
  auto file = QFile{pathAsQString(path)};
  if (!file.open(QIODevice::ReadOnly))
    return std::nullopt;
  auto hash = QCryptographicHash{QCryptographicHash::Sha256};
  while (!file.atEnd())
  {
    const auto chunk = file.read(64 * 1024);
    if (chunk.isEmpty() && file.error() != QFile::NoError)
      return std::nullopt;
    hash.addData(chunk);
  }
  return hash.result().toHex().toStdString();
}

std::string safeFilename(std::string value)
{
  std::ranges::transform(value, value.begin(), [](const unsigned char character) {
    return std::isalnum(character) || character == '-' || character == '_'
             ? static_cast<char>(character)
             : '_';
  });
  return value.empty() ? "comparison" : value;
}

QJsonArray vecToJson(const vm::vec3d& vector)
{
  return QJsonArray{vector.x(), vector.y(), vector.z()};
}

QJsonObject cameraToJson(const AcceptanceCamera& camera)
{
  auto result = QJsonObject{
    {"projection",
     camera.projection == AcceptanceProjection::Perspective ? "perspective"
                                                            : "orthographic"},
    {"position", vecToJson(camera.position)},
    {"direction", vecToJson(camera.direction)},
    {"up", vecToJson(camera.up)},
    {"near", camera.nearPlane},
    {"far", camera.farPlane},
  };
  if (camera.verticalFov)
    result.insert("verticalFov", *camera.verticalFov);
  if (camera.zoom)
    result.insert("zoom", *camera.zoom);
  return result;
}

QJsonObject overlaysToJson(const AcceptanceOverlays& overlays)
{
  return QJsonObject{
    {"brushEdges", overlays.brushEdges},
    {"selection", overlays.selection},
    {"grid", overlays.grid},
  };
}

Result<void, AcceptanceEvidenceError> copyFile(
  const std::filesystem::path& source, const std::filesystem::path& destination)
{
  if (!std::filesystem::is_regular_file(source))
    return error("Acceptance evidence source file does not exist: " + source.string());
  auto errorCode = std::error_code{};
  std::filesystem::copy_file(source, destination, errorCode);
  if (errorCode)
  {
    return error("Could not copy acceptance evidence artifact: " + errorCode.message());
  }
  return {};
}

Result<QJsonObject, AcceptanceEvidenceError> artifact(
  const std::filesystem::path& root, const std::filesystem::path& path)
{
  const auto hash = sha256(path);
  if (!hash)
    return error("Could not hash acceptance evidence artifact: " + path.string());
  auto errorCode = std::error_code{};
  const auto size = std::filesystem::file_size(path, errorCode);
  if (errorCode)
    return error("Could not size acceptance evidence artifact: " + path.string());
  return QJsonObject{
    {"path", QString::fromStdString(path.lexically_relative(root).generic_string())},
    {"sha256", QString::fromStdString(*hash)},
    {"bytes", static_cast<qint64>(size)},
  };
}

Result<QJsonObject, AcceptanceEvidenceError> publishCaptureRole(
  const std::filesystem::path& staging,
  const std::filesystem::path& comparisonDirectory,
  const QString& role,
  const AcceptanceVirtualCaptureRequest& request,
  const AcceptanceVirtualCaptureResult& capture,
  AcceptanceDocumentSnapshotProvider& snapshots)
{
  const auto roleName = role.toStdString();
  const auto mapPath = comparisonDirectory / (roleName + ".map");
  const auto snapshot = snapshots.snapshot(capture.document, mapPath);
  if (snapshot.is_error())
    return std::get<AcceptanceEvidenceError>(snapshot.error());

  const auto colorPath = comparisonDirectory / (roleName + ".png");
  const auto copiedColor = copyFile(capture.colorPath, colorPath);
  if (copiedColor.is_error())
    return std::get<AcceptanceEvidenceError>(copiedColor.error());

  const auto mapArtifact = artifact(staging, mapPath);
  const auto colorArtifact = artifact(staging, colorPath);
  if (mapArtifact.is_error())
    return std::get<AcceptanceEvidenceError>(mapArtifact.error());
  if (colorArtifact.is_error())
    return std::get<AcceptanceEvidenceError>(colorArtifact.error());

  auto result = QJsonObject{
    {"sourcePath", QString::fromStdString(capture.document.path.generic_string())},
    {"documentId", QString::fromStdString(capture.document.documentId)},
    {"revision", static_cast<qint64>(capture.document.revision)},
    {"rendererVersion", QString::fromStdString(capture.rendererVersion)},
    {"camera", cameraToJson(capture.normalizedCamera)},
    {"size", QJsonObject{{"width", capture.size.width}, {"height", capture.size.height}}},
    {"renderMode", QString::fromStdString(request.renderMode)},
    {"overlays", overlaysToJson(request.overlays)},
    {"map", mapArtifact.value()},
    {"color", colorArtifact.value()},
  };
  if (capture.depthPath)
  {
    const auto depthPath = comparisonDirectory / (roleName + ".pfm");
    const auto copiedDepth = copyFile(*capture.depthPath, depthPath);
    if (copiedDepth.is_error())
      return std::get<AcceptanceEvidenceError>(copiedDepth.error());
    const auto depthArtifact = artifact(staging, depthPath);
    if (depthArtifact.is_error())
      return std::get<AcceptanceEvidenceError>(depthArtifact.error());
    result.insert("depth", depthArtifact.value());
  }
  return result;
}

QJsonObject evidenceReportToJson(const AcceptanceSuiteRunReport& report)
{
  auto result = acceptanceSuiteRunReportToJson(report);
  auto comparisons = result.value("comparisons").toArray();
  for (auto i = 0; i < comparisons.size(); ++i)
  {
    if (!report.comparisons[static_cast<size_t>(i)].capture)
      continue;
    auto comparison = comparisons[i].toObject();
    auto imageComparison = comparison.value("imageComparison").toObject();
    const auto directory =
      std::filesystem::path{"comparisons"}
      / safeFilename(report.comparisons[static_cast<size_t>(i)].comparisonId);
    imageComparison.insert(
      "referencePath",
      QString::fromStdString((directory / "reference.png").generic_string()));
    imageComparison.insert(
      "targetPath",
      QString::fromStdString((directory / "candidate.png").generic_string()));
    comparison.insert("imageComparison", imageComparison);
    comparisons[i] = comparison;
  }
  result.insert("comparisons", comparisons);
  return result;
}

} // namespace

AcceptanceEvidenceResult writeAcceptanceEvidenceBundle(
  const std::filesystem::path& projectPath,
  const AcceptanceSuiteRunReport& report,
  const std::filesystem::path& outputDirectory,
  AcceptanceDocumentSnapshotProvider& snapshots)
{
  if (!outputDirectory.is_absolute() || outputDirectory.filename().empty())
    return error("Acceptance evidence outputDirectory must be an absolute directory");
  if (std::filesystem::exists(outputDirectory))
    return error("Acceptance evidence outputDirectory already exists");
  if (!std::filesystem::is_regular_file(projectPath))
    return error("Acceptance project file does not exist");

  const auto parent = outputDirectory.parent_path();
  auto errorCode = std::error_code{};
  std::filesystem::create_directories(parent, errorCode);
  if (errorCode)
    return error("Could not create acceptance evidence parent directory");

  const auto staging = parent
                       / ("." + outputDirectory.filename().string() + ".staging-"
                          + QUuid::createUuid()
                              .toString(QUuid::WithoutBraces)
                              .toStdString());
  std::filesystem::create_directory(staging, errorCode);
  if (errorCode)
    return error("Could not create acceptance evidence staging directory");
  const auto cleanup = [&]() {
    auto cleanupError = std::error_code{};
    std::filesystem::remove_all(staging, cleanupError);
  };

  const auto projectSnapshot = staging / "project.json";
  const auto copiedProject = copyFile(projectPath, projectSnapshot);
  if (copiedProject.is_error())
  {
    cleanup();
    return std::get<AcceptanceEvidenceError>(copiedProject.error());
  }
  const auto projectArtifact = artifact(staging, projectSnapshot);
  if (projectArtifact.is_error())
  {
    cleanup();
    return std::get<AcceptanceEvidenceError>(projectArtifact.error());
  }

  auto captureArtifacts = QJsonArray{};
  for (const auto& comparison : report.comparisons)
  {
    if (!comparison.capture)
      continue;
    const auto comparisonDirectory =
      staging / "comparisons" / safeFilename(comparison.comparisonId);
    std::filesystem::create_directories(comparisonDirectory, errorCode);
    if (errorCode)
    {
      cleanup();
      return error("Could not create acceptance comparison evidence directory");
    }
    const auto reference = publishCaptureRole(
      staging,
      comparisonDirectory,
      "reference",
      comparison.capture->requests.reference,
      comparison.capture->reference,
      snapshots);
    const auto candidate = publishCaptureRole(
      staging,
      comparisonDirectory,
      "candidate",
      comparison.capture->requests.target,
      comparison.capture->target,
      snapshots);
    if (reference.is_error() || candidate.is_error())
    {
      const auto failure = reference.is_error()
                             ? std::get<AcceptanceEvidenceError>(reference.error())
                             : std::get<AcceptanceEvidenceError>(candidate.error());
      cleanup();
      return failure;
    }
    captureArtifacts.push_back(
      QJsonObject{
        {"comparisonId", QString::fromStdString(comparison.comparisonId)},
        {"reference", reference.value()},
        {"candidate", candidate.value()},
      });
  }

  const auto manifestPath = staging / "manifest.json";
  const auto manifest = QJsonObject{
    {"schemaVersion", 1},
    {"createdAtUtc", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
    {"projectPath", QString::fromStdString(projectPath.generic_string())},
    {"projectRevision", static_cast<qint64>(report.storeRevision)},
    {"project", projectArtifact.value()},
    {"report", evidenceReportToJson(report)},
    {"captures", captureArtifacts},
  };
  auto output = QSaveFile{pathAsQString(manifestPath)};
  if (
    !output.open(QIODevice::WriteOnly)
    || output.write(QJsonDocument{manifest}.toJson(QJsonDocument::Indented)) < 0
    || !output.commit())
  {
    cleanup();
    return error("Could not publish acceptance evidence manifest");
  }
  const auto manifestHash = sha256(manifestPath);
  if (!manifestHash)
  {
    cleanup();
    return error("Could not hash acceptance evidence manifest");
  }

  std::filesystem::rename(staging, outputDirectory, errorCode);
  if (errorCode)
  {
    cleanup();
    return error("Could not atomically publish acceptance evidence directory");
  }
  return AcceptanceEvidenceBundle{
    outputDirectory, outputDirectory / "manifest.json", *manifestHash};
}

} // namespace tb::ui
