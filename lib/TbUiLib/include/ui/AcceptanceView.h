/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include <QJsonObject>

#include "base/Result.h"

#include "vm/vec.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace tb::ui
{

inline constexpr size_t AcceptanceSchemaVersion = 2u;
inline constexpr size_t LegacyAcceptanceSchemaVersion = 1u;

enum class AcceptanceErrorCode
{
  InvalidJson,
  UnsupportedSchemaVersion,
  InvalidValue,
  BrokenReference,
  RevisionConflict,
  FileAccess,
  LockFailed,
};

struct AcceptanceError
{
  AcceptanceErrorCode code = AcceptanceErrorCode::InvalidValue;
  std::string message;

  bool operator==(const AcceptanceError&) const = default;
};

std::ostream& operator<<(std::ostream& lhs, const AcceptanceError& rhs);

enum class AcceptanceProjection
{
  Perspective,
  Orthographic,
};

/**
 * A persisted, renderer-independent subset of the EV virtual camera contract. It is
 * deliberately a value rather than a view or a camera handle: AV2 adapts it to the
 * eventual render request without ever storing process-local identifiers here.
 */
struct AcceptanceCamera
{
  AcceptanceProjection projection = AcceptanceProjection::Perspective;
  vm::vec3d position{0.0, 0.0, 0.0};
  vm::vec3d direction{0.0, 1.0, 0.0};
  vm::vec3d up{0.0, 0.0, 1.0};
  std::optional<double> verticalFov = 75.0;
  double nearPlane = 1.0;
  double farPlane = 65536.0;
  std::optional<double> zoom;
};

struct AcceptanceImageSize
{
  int width = 1600;
  int height = 900;
};

struct AcceptanceOverlays
{
  bool brushEdges = false;
  bool selection = false;
  bool grid = false;
};

struct AcceptanceNamedView
{
  std::string id;
  std::string name;
  AcceptanceCamera camera;
  AcceptanceImageSize size;
  std::string renderMode = "textured";
  AcceptanceOverlays overlays;
};

/** A document path as stored in an acceptance project. It must be relative. */
struct AcceptanceDocumentReference
{
  std::filesystem::path path;
  std::string viewId;
};

enum class AcceptanceAlignmentType
{
  Identity,
  Matrix,
  Landmarks,
  Independent,
};

struct AcceptanceLandmark
{
  vm::vec3d reference;
  vm::vec3d target;
};

struct AcceptanceAlignment
{
  AcceptanceAlignmentType type = AcceptanceAlignmentType::Identity;
  std::array<double, 16u> matrix{
    1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
  std::vector<AcceptanceLandmark> landmarks;
};

/**
 * A durable, asymmetric document pairing shared by many comparisons. Paths are
 * project-relative; alignment maps reference coordinates into candidate coordinates.
 * The reference role is read-only for context-aware operations.
 */
struct AcceptanceComparisonContext
{
  std::string id;
  std::string name;
  std::filesystem::path referencePath;
  std::filesystem::path candidatePath;
  AcceptanceAlignment alignment;
};

/** A normalized rectangle in [0, 1] image coordinates. */
struct AcceptanceMask
{
  std::string id;
  double x = 0.0;
  double y = 0.0;
  double width = 1.0;
  double height = 1.0;
};

enum class AcceptanceMetricType
{
  Color,
  Depth,
  Silhouette,
  EdgeMap,
  MaterialId,
  ObjectId,
};

struct AcceptanceMetric
{
  std::string id;
  AcceptanceMetricType type = AcceptanceMetricType::Silhouette;
  std::optional<std::string> maskId;
  QJsonObject configuration;
};

enum class AcceptanceAssertionType
{
  BoundsVisible,
  BoundsNotVisible,
  ClearSightline,
  OpeningClearance,
  PlayerClearance,
  MaterialCoverage,
  DepthRange,
};

struct AcceptanceBounds
{
  vm::vec3d min;
  vm::vec3d max;
};

/**
 * `configuration` is intentionally opaque to AV0. It gives AV4/AV5 room to add
 * assertion-specific parameters without changing the persisted envelope.
 */
struct AcceptanceAssertion
{
  std::string id;
  AcceptanceAssertionType type = AcceptanceAssertionType::BoundsVisible;
  std::optional<std::string> maskId;
  std::optional<AcceptanceBounds> bounds;
  QJsonObject configuration;
};

struct AcceptanceComparison
{
  std::string id;
  std::string name;
  AcceptanceDocumentReference reference;
  AcceptanceDocumentReference target;
  AcceptanceAlignment alignment;
  std::vector<AcceptanceMask> masks;
  std::vector<AcceptanceMetric> metrics;
  std::vector<AcceptanceAssertion> assertions;
  std::optional<std::string> contextId = std::nullopt;
};

struct AcceptanceSuite
{
  size_t schemaVersion = AcceptanceSchemaVersion;
  std::string suiteId;
  std::string name;
  std::vector<std::string> comparisonIds;
};

/** The complete project-side payload. `revision` belongs to the store, not a map. */
struct AcceptanceProject
{
  size_t schemaVersion = AcceptanceSchemaVersion;
  size_t revision = 0u;
  std::vector<AcceptanceNamedView> views;
  std::vector<AcceptanceComparison> comparisons;
  std::vector<AcceptanceSuite> suites;
  std::vector<AcceptanceComparisonContext> contexts;
};

using AcceptanceProjectResult = Result<AcceptanceProject, AcceptanceError>;
using AcceptanceValidationResult = Result<void, AcceptanceError>;

AcceptanceValidationResult validateAcceptanceProject(const AcceptanceProject& project);

/** Serializes canonical key and array order, suitable for stable commits and tests. */
QJsonObject acceptanceProjectToJson(const AcceptanceProject& project);
AcceptanceProjectResult acceptanceProjectFromJson(const QJsonObject& json);

/**
 * Turns an explicit document path into a portable reference relative to the project
 * file. Both input paths may be relative; callers choose the project path explicitly.
 */
Result<std::filesystem::path, AcceptanceError> makePortableAcceptancePath(
  const std::filesystem::path& projectPath, const std::filesystem::path& documentPath);

/** Resolves a stored relative reference against the configured project file. */
Result<std::filesystem::path, AcceptanceError> resolveAcceptancePath(
  const std::filesystem::path& projectPath, const std::filesystem::path& portablePath);

} // namespace tb::ui
