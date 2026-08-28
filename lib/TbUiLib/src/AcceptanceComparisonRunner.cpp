/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceComparisonRunner.h"

#include <algorithm>
#include <ostream>
#include <ranges>
#include <utility>
#include <variant>

namespace tb::ui
{
namespace
{

AcceptanceComparisonError error(
  const AcceptanceComparisonErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

template <typename Value>
AcceptanceError acceptanceError(const Result<Value, AcceptanceError>& result)
{
  return std::get<AcceptanceError>(result.error());
}

template <typename Value>
AcceptanceAlignmentError alignmentError(
  const Result<Value, AcceptanceAlignmentError>& result)
{
  return std::get<AcceptanceAlignmentError>(result.error());
}

const AcceptanceNamedView* findView(
  const AcceptanceProject& project, const std::string& viewId)
{
  const auto found = std::ranges::find(project.views, viewId, &AcceptanceNamedView::id);
  return found == project.views.end() ? nullptr : &*found;
}

AcceptanceValidationResult validateCaptureEcho(
  const AcceptanceVirtualCaptureRequest& request,
  const AcceptanceVirtualCaptureResult& result,
  AcceptanceComparisonError& output)
{
  if (result.document.path.lexically_normal() != request.documentPath.lexically_normal())
  {
    output = error(
      AcceptanceComparisonErrorCode::CaptureEchoMismatch,
      "Virtual capture returned a different document path");
    return AcceptanceError{AcceptanceErrorCode::InvalidValue, output.message};
  }
  if (
    result.document.documentId.empty() || result.colorPath.empty()
    || result.rendererVersion.empty())
  {
    output = error(
      AcceptanceComparisonErrorCode::CaptureEchoMismatch,
      "Virtual capture did not echo document identity, output path, and renderer "
      "version");
    return AcceptanceError{AcceptanceErrorCode::InvalidValue, output.message};
  }
  if (
    result.size.width != request.size.width || result.size.height != request.size.height)
  {
    output = error(
      AcceptanceComparisonErrorCode::CaptureEchoMismatch,
      "Virtual capture returned a different image size");
    return AcceptanceError{AcceptanceErrorCode::InvalidValue, output.message};
  }
  return {};
}

} // namespace

std::ostream& operator<<(std::ostream& lhs, const AcceptanceComparisonError& rhs)
{
  return lhs << rhs.message;
}

AcceptancePairedRequestsResult makeAcceptancePairedCaptureRequests(
  const std::filesystem::path& projectPath,
  const AcceptanceProject& project,
  const AcceptanceComparison& comparison)
{
  const auto projectValidation = validateAcceptanceProject(project);
  if (projectValidation.is_error())
  {
    return error(
      AcceptanceComparisonErrorCode::InvalidProject,
      acceptanceError(projectValidation).message);
  }
  const auto* referenceView = findView(project, comparison.reference.viewId);
  const auto* targetView = findView(project, comparison.target.viewId);
  if (referenceView == nullptr || targetView == nullptr)
  {
    return error(
      AcceptanceComparisonErrorCode::MissingView,
      "Comparison references a missing named view");
  }
  const auto referencePath =
    resolveAcceptancePath(projectPath, comparison.reference.path);
  const auto targetPath = resolveAcceptancePath(projectPath, comparison.target.path);
  if (referencePath.is_error() || targetPath.is_error())
  {
    return error(
      AcceptanceComparisonErrorCode::PathResolutionFailed,
      referencePath.is_error() ? acceptanceError(referencePath).message
                               : acceptanceError(targetPath).message);
  }
  const auto alignedCamera = alignAcceptanceTargetCamera(
    comparison.alignment, referenceView->camera, targetView->camera);
  if (alignedCamera.is_error())
  {
    return error(
      AcceptanceComparisonErrorCode::AlignmentFailed,
      alignmentError(alignedCamera).message);
  }
  const auto needsDepth = std::ranges::any_of(comparison.metrics, [](const auto& metric) {
    return metric.type == AcceptanceMetricType::Depth
           || metric.type == AcceptanceMetricType::Silhouette;
  });
  return AcceptancePairedCaptureRequests{
    {referencePath.value(),
     referenceView->camera,
     referenceView->size,
     referenceView->renderMode,
     referenceView->overlays,
     needsDepth},
    {targetPath.value(),
     alignedCamera.value(),
     referenceView->size,
     referenceView->renderMode,
     referenceView->overlays,
     needsDepth},
  };
}

AcceptanceComparisonRunner::AcceptanceComparisonRunner(
  std::filesystem::path projectPath, AcceptanceVirtualCapture& capture)
  : m_projectPath{std::move(projectPath)}
  , m_capture{capture}
{
}

AcceptancePairedCaptureResult AcceptanceComparisonRunner::run(
  const AcceptanceProject& project, const std::string& comparisonId) const
{
  const auto found =
    std::ranges::find(project.comparisons, comparisonId, &AcceptanceComparison::id);
  if (found == project.comparisons.end())
  {
    return error(
      AcceptanceComparisonErrorCode::ComparisonNotFound,
      "Acceptance comparison was not found");
  }
  const auto requests =
    makeAcceptancePairedCaptureRequests(m_projectPath, project, *found);
  if (requests.is_error())
  {
    return std::get<AcceptanceComparisonError>(requests.error());
  }

  const auto reference = m_capture.capture(requests.value().reference);
  if (reference.is_error())
  {
    return error(
      AcceptanceComparisonErrorCode::CaptureFailed,
      "Reference capture failed: "
        + std::get<AcceptanceVirtualCaptureError>(reference.error()).message);
  }
  auto echoError = AcceptanceComparisonError{};
  if (validateCaptureEcho(requests.value().reference, reference.value(), echoError)
        .is_error())
  {
    return echoError;
  }

  const auto target = m_capture.capture(requests.value().target);
  if (target.is_error())
  {
    return error(
      AcceptanceComparisonErrorCode::CaptureFailed,
      "Target capture failed: "
        + std::get<AcceptanceVirtualCaptureError>(target.error()).message);
  }
  if (validateCaptureEcho(requests.value().target, target.value(), echoError).is_error())
  {
    return echoError;
  }
  const auto imageComparison = compareAcceptanceImages(
    {reference.value().colorPath, reference.value().depthPath},
    {target.value().colorPath, target.value().depthPath},
    *found);
  if (imageComparison.is_error())
  {
    return error(
      AcceptanceComparisonErrorCode::MetricEvaluationFailed,
      std::get<AcceptanceImageComparisonError>(imageComparison.error()).message);
  }
  return AcceptancePairedCaptureReport{
    found->id,
    found->alignment.type,
    requests.value(),
    reference.value(),
    target.value(),
    imageComparison.value(),
  };
}

} // namespace tb::ui
