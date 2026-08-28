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

#include "ui/QPathUtils.h"

#include <cmath>
#include <limits>
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

} // namespace

AcceptanceAutomationService::AcceptanceAutomationService(
  AcceptanceViewStore& store,
  AcceptanceVirtualCapture& capture,
  AcceptanceGeometryProvider& geometry)
  : m_store{store}
  , m_comparisons{store.projectPath(), capture}
  , m_suites{store, m_comparisons, geometry}
  , m_geometry{geometry}
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
  if (method == "acceptance.assertions.evaluate")
    return evaluateAssertion(params);
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
  const auto report = m_suites.run(id.toString().toStdString(), options);
  auto result = QJsonObject{
    {"projectPath", QString::fromStdString(m_store.projectPath().generic_string())},
    {"report", acceptanceSuiteRunReportToJson(report)}};
  return result;
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

} // namespace tb::ui
