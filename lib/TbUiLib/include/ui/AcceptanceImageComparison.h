/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "ui/AcceptanceView.h"

namespace tb::ui
{

enum class AcceptanceImageComparisonErrorCode
{
  ImageLoadFailed,
  ImageSizeMismatch,
  InvalidConfiguration,
  EmptyRegion,
  MissingAuxiliaryBuffer,
};

struct AcceptanceImageComparisonError
{
  AcceptanceImageComparisonErrorCode code =
    AcceptanceImageComparisonErrorCode::InvalidConfiguration;
  std::string message;
};

std::ostream& operator<<(std::ostream& lhs, const AcceptanceImageComparisonError& rhs);

struct AcceptanceImageDifferenceDiagnostic
{
  size_t comparedPixels = 0u;
  size_t changedPixels = 0u;
  double changedFraction = 0.0;
  double maximumAbsoluteError = 0.0;
  double maximumRelativeError = 0.0;
  std::optional<AcceptanceBounds> changedBounds;
};

struct AcceptanceImageMetricReport
{
  std::string metricId;
  AcceptanceMetricType metricType = AcceptanceMetricType::Silhouette;
  bool passed = false;
  AcceptanceImageDifferenceDiagnostic diagnostic;
};

struct AcceptanceImageComparisonReport
{
  std::filesystem::path referencePath;
  std::filesystem::path targetPath;
  std::vector<AcceptanceImageMetricReport> metrics;
};

/** Real auxiliary buffers supplied by EV6; silhouette is finite depth coverage. */
struct AcceptanceImageBuffers
{
  std::filesystem::path colorPath;
  std::optional<std::filesystem::path> depthPath;
};

using AcceptanceImageComparisonResult =
  Result<AcceptanceImageComparisonReport, AcceptanceImageComparisonError>;

/**
 * Evaluates the configured per-pixel comparison metrics for one captured pair. The
 * metric configuration accepts `absoluteError`, `relativeError`, and
 * `maxChangedFraction`. Color tolerances and fractions are in [0, 1]; depth's
 * absolute tolerance is in camera-space map units. A metric mask selects an optional
 * normalized ROI.
 */
AcceptanceImageComparisonResult compareAcceptanceImages(
  const std::filesystem::path& referencePath,
  const std::filesystem::path& targetPath,
  const AcceptanceComparison& comparison);

AcceptanceImageComparisonResult compareAcceptanceImages(
  const AcceptanceImageBuffers& reference,
  const AcceptanceImageBuffers& target,
  const AcceptanceComparison& comparison);

QJsonObject acceptanceImageComparisonReportToJson(
  const AcceptanceImageComparisonReport& report);

} // namespace tb::ui
