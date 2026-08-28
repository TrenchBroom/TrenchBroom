/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceAutomationService.h"

#include <QJsonArray>

#include "AutomationJson.h"
#include "ui/AcceptanceComparisonAlignment.h"
#include "ui/AcceptanceComparisonContextResolver.h"
#include "ui/AcceptanceDivergencePolicy.h"
#include "ui/QPathUtils.h"

#include <cmath>
#include <limits>
#include <memory>
#include <ranges>
#include <utility>
#include <variant>

namespace tb::ui
{
namespace
{

AcceptanceAutomationError error(
  const AcceptanceAutomationErrorCode code, std::string message)
{
  return {code, std::move(message)};
}

template <typename Value>
AcceptanceAutomationError storeError(const Result<Value, AcceptanceError>& result)
{
  return error(
    AcceptanceAutomationErrorCode::StoreFailed,
    std::get<AcceptanceError>(result.error()).message);
}

QString singularKind(const QString& kind)
{
  if (kind == "contexts")
    return "context";
  if (kind == "views")
    return "view";
  if (kind == "comparisons")
    return "comparison";
  return "suite";
}

QString idKey(const QString& kind)
{
  return kind == "suites" ? "suiteId" : "id";
}

std::optional<size_t> expectedRevision(const QJsonObject& params)
{
  const auto value = params.value("expectedRevision");
  if (
    !value.isDouble() || value.toDouble() < 0.0
    || std::floor(value.toDouble()) != value.toDouble()
    || value.toDouble() > static_cast<double>(std::numeric_limits<size_t>::max()))
    return std::nullopt;
  return static_cast<size_t>(value.toDouble());
}

Result<AcceptanceSuiteRunOptions, AcceptanceAutomationError> suiteRunOptions(
  const QJsonObject& params)
{
  auto options = AcceptanceSuiteRunOptions{};
  if (params.contains("comparisonIds"))
  {
    const auto values = params.value("comparisonIds");
    if (!values.isArray())
    {
      return error(
        AcceptanceAutomationErrorCode::InvalidParameters,
        "comparisonIds must be an array of strings");
    }
    for (const auto& value : values.toArray())
    {
      if (!value.isString())
      {
        return error(
          AcceptanceAutomationErrorCode::InvalidParameters,
          "comparisonIds must be an array of strings");
      }
      options.comparisonFilter.push_back(value.toString().toStdString());
    }
  }
  if (params.contains("maxCpuConcurrency"))
  {
    const auto concurrency = params.value("maxCpuConcurrency");
    if (
      !concurrency.isDouble() || concurrency.toDouble() < 1.0
      || std::floor(concurrency.toDouble()) != concurrency.toDouble()
      || concurrency.toDouble() > static_cast<double>(std::numeric_limits<size_t>::max()))
    {
      return error(
        AcceptanceAutomationErrorCode::InvalidParameters,
        "maxCpuConcurrency must be a positive integer");
    }
    options.maxCpuConcurrency = static_cast<size_t>(concurrency.toDouble());
  }
  return options;
}

QJsonObject metadata(const AcceptanceViewStore& store, const AcceptanceProject& project)
{
  return {
    {"projectPath", QString::fromStdString(store.projectPath().generic_string())},
    {"revision", static_cast<qint64>(project.revision)},
  };
}

QJsonObject captureToJson(const AcceptancePairedCaptureReport& report)
{
  const auto capture = [](const AcceptanceVirtualCaptureResult& result) {
    auto json = QJsonObject{
      {"document",
       QJsonObject{
         {"path", QString::fromStdString(result.document.path.generic_string())},
         {"documentId", QString::fromStdString(result.document.documentId)},
         {"revision", static_cast<qint64>(result.document.revision)}}},
      {"colorPath", QString::fromStdString(result.colorPath.generic_string())},
      {"rendererVersion", QString::fromStdString(result.rendererVersion)}};
    if (result.depthPath)
      json.insert(
        "depthPath", QString::fromStdString(result.depthPath->generic_string()));
    return json;
  };
  return {
    {"comparisonId", QString::fromStdString(report.comparisonId)},
    {"reference", capture(report.reference)},
    {"target", capture(report.target)},
    {"imageComparison", acceptanceImageComparisonReportToJson(report.imageComparison)},
  };
}

Result<AcceptanceCaptureDocumentIdentity, AcceptanceAutomationError> documentFromJson(
  const QJsonValue& value)
{
  if (!value.isObject())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Assertion evaluation requires a document object with documentId");
  }
  const auto object = value.toObject();
  const auto id = object.value("documentId");
  if (!id.isString() || id.toString().isEmpty())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Assertion evaluation requires an explicit document.documentId");
  }
  if (object.contains("path") && !object.value("path").isString())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "document.path must be a string when supplied");
  }
  const auto revision = object.value("revision");
  if (
    !revision.isUndefined()
    && (!revision.isDouble() || revision.toDouble() < 0.0
        || std::floor(revision.toDouble()) != revision.toDouble()
        || revision.toDouble() > static_cast<double>(std::numeric_limits<size_t>::max())))
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "document.revision must be a non-negative integer when supplied");
  }
  return AcceptanceCaptureDocumentIdentity{
    object.contains("path") ? pathFromQString(object.value("path").toString())
                            : std::filesystem::path{},
    id.toString().toStdString(),
    revision.isUndefined() ? 0u : static_cast<size_t>(revision.toDouble())};
}

QJsonObject documentToJson(const AcceptanceCaptureDocumentIdentity& document)
{
  return {
    {"path", QString::fromStdString(document.path.generic_string())},
    {"documentId", QString::fromStdString(document.documentId)},
    {"revision", static_cast<qint64>(document.revision)},
  };
}

constexpr auto DefaultSolidSpaceMaxSamples = size_t{250000u};
constexpr auto MaximumSolidSpaceMaxSamples = size_t{1000000u};
constexpr auto DefaultSolidSpaceReportLimit = size_t{1000u};
constexpr auto MaximumSolidSpaceReportLimit = size_t{10000u};

std::optional<size_t> boundedSize(
  const QJsonObject& params,
  const char* key,
  const size_t defaultValue,
  const size_t maximum)
{
  if (!params.contains(key))
    return defaultValue;
  const auto value = params.value(key);
  if (
    !value.isDouble() || value.toDouble() < 1.0
    || std::floor(value.toDouble()) != value.toDouble()
    || value.toDouble() > static_cast<double>(maximum))
  {
    return std::nullopt;
  }
  return static_cast<size_t>(value.toDouble());
}

std::optional<vm::bbox3d> solidSpaceBounds(const QJsonValue& value)
{
  if (!value.isObject())
    return std::nullopt;
  const auto object = value.toObject();
  const auto min = automation::vec3FromJson(object.value("min"));
  const auto max = automation::vec3FromJson(object.value("max"));
  if (!min || !max)
    return std::nullopt;
  const auto result = vm::bbox3d{*min, *max};
  return result.is_valid() && !result.is_empty() ? std::optional{result} : std::nullopt;
}

QJsonObject solidSpaceBoundsToJson(const vm::bbox3d& bounds)
{
  return {
    {"min", QJsonArray{bounds.min.x(), bounds.min.y(), bounds.min.z()}},
    {"max", QJsonArray{bounds.max.x(), bounds.max.y(), bounds.max.z()}},
  };
}

class AlignedSolidSpaceQuery : public AcceptanceSolidSpaceQuery
{
public:
  AlignedSolidSpaceQuery(
    std::shared_ptr<const AcceptanceSolidSpaceQuery> query, AcceptanceAlignment alignment)
    : m_query{std::move(query)}
    , m_alignment{std::move(alignment)}
  {
  }

  Result<bool, AcceptanceSolidSpaceError> isSolid(
    const vm::vec3d& referencePoint) const override
  {
    const auto candidatePoint = alignAcceptanceTargetPoint(m_alignment, referencePoint);
    if (candidatePoint.is_error())
    {
      return AcceptanceSolidSpaceError{
        std::get<AcceptanceAlignmentError>(candidatePoint.error()).message};
    }
    return m_query->isSolid(candidatePoint.value());
  }

private:
  std::shared_ptr<const AcceptanceSolidSpaceQuery> m_query;
  AcceptanceAlignment m_alignment;
};

QJsonObject discrepancyToJson(
  const AcceptanceSolidSpaceDiscrepancy& discrepancy,
  const bool includeCells,
  const size_t reportLimit)
{
  auto result = QJsonObject{{"cellCount", static_cast<qint64>(discrepancy.cellCount)}};
  if (discrepancy.bounds)
    result.insert("bounds", solidSpaceBoundsToJson(*discrepancy.bounds));
  auto regions = QJsonArray{};
  const auto regionCount = std::min(reportLimit, discrepancy.regions.size());
  for (size_t i = 0u; i < regionCount; ++i)
  {
    regions.push_back(
      QJsonObject{
        {"cellCount", static_cast<qint64>(discrepancy.regions[i].cellCount)},
        {"bounds", solidSpaceBoundsToJson(discrepancy.regions[i].bounds)},
      });
  }
  result.insert("regionCount", static_cast<qint64>(discrepancy.regions.size()));
  result.insert("regions", regions);
  result.insert(
    "regionsTruncated", static_cast<qint64>(discrepancy.regions.size() - regionCount));
  if (includeCells)
  {
    auto cells = QJsonArray{};
    const auto count = std::min(reportLimit, discrepancy.cells.size());
    for (size_t i = 0u; i < count; ++i)
      cells.push_back(solidSpaceBoundsToJson(discrepancy.cells[i]));
    result.insert("cells", cells);
    result.insert(
      "cellsTruncated", static_cast<qint64>(discrepancy.cells.size() - count));
  }
  return result;
}

void incrementJsonCount(QJsonObject& counts, const QString& key)
{
  counts.insert(key, counts.value(key).toInteger() + 1);
}

QJsonObject classifySolidSpaceReport(
  const AcceptanceSolidSpaceComparisonReport& report,
  const AcceptanceDivergencePolicy& policy,
  const size_t reportLimit)
{
  auto findings = std::vector<AcceptanceDivergenceFinding>{};
  findings.reserve(report.newlyEmpty.cells.size() + report.newlySolid.cells.size());
  for (size_t i = 0u; i < report.newlyEmpty.cells.size(); ++i)
  {
    findings.push_back(
      {"reference-only-" + std::to_string(i),
       AcceptanceDivergenceDomain::SolidSpace,
       AcceptanceDivergenceDirection::ReferenceOnly,
       1u,
       report.newlyEmpty.cells[i]});
  }
  for (size_t i = 0u; i < report.newlySolid.cells.size(); ++i)
  {
    findings.push_back(
      {"candidate-only-" + std::to_string(i),
       AcceptanceDivergenceDomain::SolidSpace,
       AcceptanceDivergenceDirection::CandidateOnly,
       1u,
       report.newlySolid.cells[i]});
  }

  const auto classifications = classifyAcceptanceDivergences(policy, std::move(findings));
  auto reported = QJsonArray{};
  auto dispositionCounts = QJsonObject{};
  auto severityCounts = QJsonObject{};
  for (size_t i = 0u; i < classifications.size(); ++i)
  {
    const auto json = acceptanceDivergenceClassificationToJson(classifications[i]);
    incrementJsonCount(dispositionCounts, json.value("disposition").toString());
    incrementJsonCount(severityCounts, json.value("severity").toString());
    if (i < reportLimit)
      reported.push_back(json);
  }
  return {
    {"definition", acceptanceDivergencePolicyToJson(policy)},
    {"total", static_cast<qint64>(classifications.size())},
    {"reported", reported},
    {"reportedTruncated",
     static_cast<qint64>(classifications.size() - static_cast<size_t>(reported.size()))},
    {"dispositions", dispositionCounts},
    {"severities", severityCounts},
  };
}

} // namespace

AcceptanceAutomationService::AcceptanceAutomationService(
  AcceptanceViewStore& store,
  AcceptanceVirtualCapture& capture,
  AcceptanceGeometryProvider& geometry,
  AcceptanceSolidSpaceProvider& solidSpace,
  AcceptanceDocumentSnapshotProvider& snapshots)
  : m_store{store}
  , m_comparisons{store.projectPath(), capture}
  , m_suites{store, m_comparisons, geometry}
  , m_geometry{geometry}
  , m_solidSpace{solidSpace}
  , m_snapshots{snapshots}
{
}

const std::filesystem::path& AcceptanceAutomationService::projectPath() const
{
  return m_store.projectPath();
}

AcceptanceAutomationResult AcceptanceAutomationService::handle(
  const QString& method, const QJsonObject& params)
{
  static const auto kinds = std::array{
    QString{"contexts"}, QString{"views"}, QString{"comparisons"}, QString{"suites"}};
  for (const auto& kind : kinds)
  {
    const auto prefix = "acceptance." + kind + ".";
    if (method == prefix + "list")
      return list(kind);
    if (method == prefix + "create")
      return create(kind, params);
    if (method == prefix + "update")
      return update(kind, params);
    if (method == prefix + "delete")
      return erase(kind, params);
  }
  if (method == "acceptance.capture")
    return capture(params);
  if (method == "acceptance.run")
    return run(params);
  if (method == "acceptance.evidence.run")
    return runEvidence(params);
  if (method == "acceptance.assertions.evaluate")
    return evaluateAssertion(params);
  if (method == "acceptance.geometry.compare")
    return compareSolidSpace(params);
  return error(
    AcceptanceAutomationErrorCode::MethodNotFound, "Unknown acceptance method");
}

AcceptanceAutomationResult AcceptanceAutomationService::list(const QString& kind) const
{
  const auto project = m_store.load();
  if (project.is_error())
    return storeError(project);
  auto result = metadata(m_store, project.value());
  result.insert("items", acceptanceProjectToJson(project.value()).value(kind).toArray());
  return result;
}

AcceptanceAutomationResult AcceptanceAutomationService::create(
  const QString& kind, const QJsonObject& params)
{
  const auto revision = expectedRevision(params);
  const auto item = params.value(singularKind(kind));
  if (!revision || !item.isObject())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Create requires expectedRevision and an item object");
  }
  const auto updated = m_store.update(*revision, [&](AcceptanceProject& project) {
    auto json = acceptanceProjectToJson(project);
    auto values = json.value(kind).toArray();
    values.push_back(item);
    json.insert(kind, values);
    const auto parsed = acceptanceProjectFromJson(json);
    if (parsed.is_error())
      return AcceptanceValidationResult{std::get<AcceptanceError>(parsed.error())};
    project = parsed.value();
    return AcceptanceValidationResult{};
  });
  if (updated.is_error())
    return storeError(updated);
  auto result = metadata(m_store, updated.value());
  result.insert("item", item);
  return result;
}

AcceptanceAutomationResult AcceptanceAutomationService::update(
  const QString& kind, const QJsonObject& params)
{
  const auto revision = expectedRevision(params);
  const auto id = params.value("id");
  const auto item = params.value(singularKind(kind));
  if (!revision || !id.isString() || id.toString().isEmpty() || !item.isObject())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Update requires id, expectedRevision, and an item object");
  }
  if (item.toObject().value(idKey(kind)) != id)
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters, "Updated item id must match id");
  }
  const auto updated = m_store.update(*revision, [&](AcceptanceProject& project) {
    auto json = acceptanceProjectToJson(project);
    auto values = json.value(kind).toArray();
    const auto it = std::ranges::find_if(values, [&](const auto& value) {
      return value.isObject() && value.toObject().value(idKey(kind)) == id;
    });
    if (it == values.end())
    {
      return AcceptanceValidationResult{AcceptanceError{
        AcceptanceErrorCode::BrokenReference, "Acceptance item was not found"}};
    }
    *it = item;
    json.insert(kind, values);
    const auto parsed = acceptanceProjectFromJson(json);
    if (parsed.is_error())
      return AcceptanceValidationResult{std::get<AcceptanceError>(parsed.error())};
    project = parsed.value();
    return AcceptanceValidationResult{};
  });
  if (updated.is_error())
    return storeError(updated);
  auto result = metadata(m_store, updated.value());
  result.insert("item", item);
  return result;
}

AcceptanceAutomationResult AcceptanceAutomationService::erase(
  const QString& kind, const QJsonObject& params)
{
  const auto revision = expectedRevision(params);
  const auto id = params.value("id");
  if (!revision || !id.isString() || id.toString().isEmpty())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Delete requires id and expectedRevision");
  }
  const auto updated = m_store.update(*revision, [&](AcceptanceProject& project) {
    auto json = acceptanceProjectToJson(project);
    auto values = json.value(kind).toArray();
    auto index = -1;
    for (auto candidate = 0; candidate < values.size(); ++candidate)
    {
      const auto& value = values[candidate];
      if (value.isObject() && value.toObject().value(idKey(kind)) == id)
      {
        index = candidate;
        break;
      }
    }
    if (index == -1)
    {
      return AcceptanceValidationResult{AcceptanceError{
        AcceptanceErrorCode::BrokenReference, "Acceptance item was not found"}};
    }
    values.removeAt(index);
    json.insert(kind, values);
    const auto parsed = acceptanceProjectFromJson(json);
    if (parsed.is_error())
      return AcceptanceValidationResult{std::get<AcceptanceError>(parsed.error())};
    project = parsed.value();
    return AcceptanceValidationResult{};
  });
  if (updated.is_error())
    return storeError(updated);
  auto result = metadata(m_store, updated.value());
  result.insert("deletedId", id);
  return result;
}

AcceptanceAutomationResult AcceptanceAutomationService::capture(
  const QJsonObject& params) const
{
  const auto id = params.value("comparisonId");
  if (!id.isString() || id.toString().isEmpty())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters, "Capture requires comparisonId");
  }
  const auto project = m_store.load();
  if (project.is_error())
    return storeError(project);
  const auto report = m_comparisons.run(project.value(), id.toString().toStdString());
  if (report.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::CaptureFailed,
      std::get<AcceptanceComparisonError>(report.error()).message);
  }
  auto result = metadata(m_store, project.value());
  result.insert("capture", captureToJson(report.value()));
  return result;
}

AcceptanceAutomationResult AcceptanceAutomationService::run(
  const QJsonObject& params) const
{
  const auto id = params.value("suiteId");
  if (!id.isString() || id.toString().isEmpty())
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters, "Run requires suiteId");

  const auto options = suiteRunOptions(params);
  if (options.is_error())
    return std::get<AcceptanceAutomationError>(options.error());
  const auto report = m_suites.run(id.toString().toStdString(), options.value());
  auto result = QJsonObject{
    {"projectPath", QString::fromStdString(m_store.projectPath().generic_string())},
    {"report", acceptanceSuiteRunReportToJson(report)}};
  return result;
}

AcceptanceAutomationResult AcceptanceAutomationService::runEvidence(
  const QJsonObject& params) const
{
  const auto id = params.value("suiteId");
  const auto outputDirectory = params.value("outputDirectory");
  if (
    !id.isString() || id.toString().isEmpty() || !outputDirectory.isString()
    || outputDirectory.toString().isEmpty())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Evidence run requires suiteId and outputDirectory");
  }
  const auto options = suiteRunOptions(params);
  if (options.is_error())
    return std::get<AcceptanceAutomationError>(options.error());

  const auto report = m_suites.run(id.toString().toStdString(), options.value());
  const auto bundle = writeAcceptanceEvidenceBundle(
    m_store.projectPath(),
    report,
    pathFromQString(outputDirectory.toString()),
    m_snapshots);
  if (bundle.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::EvidenceFailed,
      std::get<AcceptanceEvidenceError>(bundle.error()).message);
  }
  return QJsonObject{
    {"projectPath", QString::fromStdString(m_store.projectPath().generic_string())},
    {"outputDirectory", QString::fromStdString(bundle.value().path.generic_string())},
    {"manifestPath",
     QString::fromStdString(bundle.value().manifestPath.generic_string())},
    {"manifestSha256", QString::fromStdString(bundle.value().manifestSha256)},
    {"report", acceptanceSuiteRunReportToJson(report)},
  };
}

AcceptanceAutomationResult AcceptanceAutomationService::evaluateAssertion(
  const QJsonObject& params) const
{
  const auto document = documentFromJson(params.value("document"));
  if (document.is_error())
    return std::get<AcceptanceAutomationError>(document.error());
  const auto assertion = params.value("assertion");
  if (!assertion.isObject())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Assertion evaluation requires an assertion object");
  }
  const auto parsedAssertion = acceptanceAssertionFromJson(assertion.toObject());
  if (parsedAssertion.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      std::get<AcceptanceAssertionError>(parsedAssertion.error()).message);
  }
  const auto context = params.value("context");
  if (!context.isUndefined() && !context.isObject())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Assertion evaluation context must be an object");
  }
  const auto parsedContext = acceptanceAssertionContextFromJson(
    context.isUndefined() ? QJsonObject{} : context.toObject());
  if (parsedContext.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      std::get<AcceptanceAssertionError>(parsedContext.error()).message);
  }

  auto result = QJsonObject{
    {"document", documentToJson(document.value())},
    {"assertionId", QString::fromStdString(parsedAssertion.value().id)},
  };
  const auto geometry = m_geometry.geometryFor(document.value());
  if (geometry.is_error())
  {
    result.insert("status", "error");
    result.insert(
      "error",
      QString::fromStdString(
        std::get<AcceptanceGeometryError>(geometry.error()).message));
    return result;
  }
  const auto evaluation = AcceptanceAssertionEvaluator{*geometry.value()}.evaluate(
    parsedAssertion.value(), parsedContext.value());
  if (evaluation.is_error())
  {
    result.insert("status", "error");
    result.insert(
      "error",
      QString::fromStdString(
        std::get<AcceptanceAssertionError>(evaluation.error()).message));
    return result;
  }
  result.insert("status", evaluation.value().passed ? "passed" : "failed");
  result.insert("report", acceptanceAssertionReportToJson(evaluation.value()));
  return result;
}

AcceptanceAutomationResult AcceptanceAutomationService::compareSolidSpace(
  const QJsonObject& params) const
{
  const auto contextId = params.value("contextId");
  const auto bounds = solidSpaceBounds(params.value("bounds"));
  const auto cellSize = params.value("cellSize");
  const auto maxSamples = boundedSize(
    params, "maxSamples", DefaultSolidSpaceMaxSamples, MaximumSolidSpaceMaxSamples);
  const auto reportLimit = boundedSize(
    params,
    "maxReportedFindings",
    DefaultSolidSpaceReportLimit,
    MaximumSolidSpaceReportLimit);
  const auto includeCellsValue = params.value("includeCells");
  if (
    !contextId.isString() || contextId.toString().isEmpty() || !bounds
    || !cellSize.isDouble() || !std::isfinite(cellSize.toDouble())
    || cellSize.toDouble() <= 0.0 || !maxSamples || !reportLimit
    || (!includeCellsValue.isUndefined() && !includeCellsValue.isBool()))
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      "Solid-space comparison requires contextId, finite nonempty bounds, a positive "
      "cellSize, and valid optional limits/includeCells");
  }

  auto policy = std::optional<AcceptanceDivergencePolicy>{};
  if (params.contains("policy"))
  {
    if (!params.value("policy").isObject())
    {
      return error(
        AcceptanceAutomationErrorCode::InvalidParameters,
        "Solid-space comparison policy must be an object");
    }
    const auto parsed =
      acceptanceDivergencePolicyFromJson(params.value("policy").toObject());
    if (parsed.is_error())
    {
      return error(
        AcceptanceAutomationErrorCode::InvalidParameters,
        std::get<AcceptanceDivergenceError>(parsed.error()).message);
    }
    policy = parsed.value();
  }

  const auto project = m_store.load();
  if (project.is_error())
    return storeError(project);
  const auto context = resolveAcceptanceComparisonContext(
    m_store.projectPath(), project.value(), contextId.toString().toStdString());
  if (context.is_error())
    return storeError(context);

  const auto alignedProbe =
    alignAcceptanceTargetPoint(context.value().alignment, bounds->center());
  if (alignedProbe.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::InvalidParameters,
      std::get<AcceptanceAlignmentError>(alignedProbe.error()).message);
  }

  const auto reference = m_solidSpace.queryFor(context.value().referencePath);
  if (reference.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::GeometryFailed,
      std::get<AcceptanceSolidSpaceError>(reference.error()).message);
  }
  const auto candidate = m_solidSpace.queryFor(context.value().candidatePath);
  if (candidate.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::GeometryFailed,
      std::get<AcceptanceSolidSpaceError>(candidate.error()).message);
  }

  const auto alignedCandidate =
    AlignedSolidSpaceQuery{candidate.value().query, context.value().alignment};
  const auto compared = AcceptanceSolidSpaceComparison{}.compare(
    *reference.value().query,
    alignedCandidate,
    {*bounds, cellSize.toDouble(), *maxSamples, {}});
  if (compared.is_error())
  {
    return error(
      AcceptanceAutomationErrorCode::GeometryFailed,
      std::get<AcceptanceSolidSpaceError>(compared.error()).message);
  }

  const auto includeCells = includeCellsValue.toBool(false);
  const auto& report = compared.value();
  auto comparison = QJsonObject{
    {"status",
     report.status == AcceptanceSolidSpaceComparisonStatus::Complete ? "complete"
                                                                     : "cancelled"},
    {"coordinateSpace", "reference"},
    {"occupancyModel", "brushVolumesV1"},
    {"referenceDocument",
     QJsonObject{
       {"id", QString::fromStdString(reference.value().documentId)},
       {"revision", static_cast<qint64>(reference.value().revision)}}},
    {"candidateDocument",
     QJsonObject{
       {"id", QString::fromStdString(candidate.value().documentId)},
       {"revision", static_cast<qint64>(candidate.value().revision)}}},
    {"bounds", solidSpaceBoundsToJson(*bounds)},
    {"cellSize", cellSize.toDouble()},
    {"totalCells", static_cast<qint64>(report.totalCells)},
    {"sampledCells", static_cast<qint64>(report.sampledCells)},
    {"referenceOnly", discrepancyToJson(report.newlyEmpty, includeCells, *reportLimit)},
    {"candidateOnly", discrepancyToJson(report.newlySolid, includeCells, *reportLimit)},
  };
  if (policy)
  {
    comparison.insert("policy", classifySolidSpaceReport(report, *policy, *reportLimit));
  }

  auto result = metadata(m_store, project.value());
  result.insert("contextId", contextId);
  result.insert(
    "referencePath",
    QString::fromStdString(context.value().referencePath.generic_string()));
  result.insert(
    "candidatePath",
    QString::fromStdString(context.value().candidatePath.generic_string()));
  result.insert("comparison", comparison);
  return result;
}

} // namespace tb::ui
