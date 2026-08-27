/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "ui/AcceptanceComparisonAlignment.h"
#include "ui/AcceptanceImageComparison.h"

#include <filesystem>
#include <string>

namespace tb::ui
{

struct AcceptanceCaptureDocumentIdentity
{
  std::filesystem::path path;
  std::string documentId;
  size_t revision = 0u;
};

/** The request AV3 sends to EV4; it is an immutable virtual request, never a pane. */
struct AcceptanceVirtualCaptureRequest
{
  std::filesystem::path documentPath;
  AcceptanceCamera camera;
  AcceptanceImageSize size;
  std::string renderMode;
  AcceptanceOverlays overlays;
  bool depth = false;
};

/** Minimum EV4 response used by AV3. The document identity must be echoed exactly. */
struct AcceptanceVirtualCaptureResult
{
  AcceptanceCaptureDocumentIdentity document;
  AcceptanceCamera normalizedCamera;
  AcceptanceImageSize size;
  std::filesystem::path colorPath;
  std::optional<std::filesystem::path> depthPath;
  std::string rendererVersion;
};

struct AcceptanceVirtualCaptureError
{
  std::string message;
};

/**
 * EV4 implements this narrow boundary. It may queue GPU work internally, but the
 * runner intentionally invokes it in deterministic reference-then-target order.
 */
class AcceptanceVirtualCapture
{
public:
  virtual ~AcceptanceVirtualCapture() = default;

  virtual Result<AcceptanceVirtualCaptureResult, AcceptanceVirtualCaptureError> capture(
    const AcceptanceVirtualCaptureRequest& request) = 0;
};

enum class AcceptanceComparisonErrorCode
{
  ComparisonNotFound,
  MissingView,
  InvalidProject,
  PathResolutionFailed,
  AlignmentFailed,
  CaptureFailed,
  CaptureEchoMismatch,
  MetricEvaluationFailed,
};

struct AcceptanceComparisonError
{
  AcceptanceComparisonErrorCode code = AcceptanceComparisonErrorCode::InvalidProject;
  std::string message;
};

std::ostream& operator<<(std::ostream& lhs, const AcceptanceComparisonError& rhs);

struct AcceptancePairedCaptureRequests
{
  AcceptanceVirtualCaptureRequest reference;
  AcceptanceVirtualCaptureRequest target;
};

struct AcceptancePairedCaptureReport
{
  std::string comparisonId;
  AcceptanceAlignmentType alignmentType = AcceptanceAlignmentType::Identity;
  AcceptancePairedCaptureRequests requests;
  AcceptanceVirtualCaptureResult reference;
  AcceptanceVirtualCaptureResult target;
  AcceptanceImageComparisonReport imageComparison;
};

using AcceptancePairedCaptureResult =
  Result<AcceptancePairedCaptureReport, AcceptanceComparisonError>;
using AcceptancePairedRequestsResult =
  Result<AcceptancePairedCaptureRequests, AcceptanceComparisonError>;

AcceptancePairedRequestsResult makeAcceptancePairedCaptureRequests(
  const std::filesystem::path& projectPath,
  const AcceptanceProject& project,
  const AcceptanceComparison& comparison);

class AcceptanceComparisonRunner
{
public:
  AcceptanceComparisonRunner(
    std::filesystem::path projectPath, AcceptanceVirtualCapture& capture);

  AcceptancePairedCaptureResult run(
    const AcceptanceProject& project, const std::string& comparisonId) const;

private:
  std::filesystem::path m_projectPath;
  AcceptanceVirtualCapture& m_capture;
};

} // namespace tb::ui
