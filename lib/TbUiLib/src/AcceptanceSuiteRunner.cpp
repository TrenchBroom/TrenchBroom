/* Copyright (C) 2026 */
#include "ui/AcceptanceSuiteRunner.h"

#include <QJsonArray>

#include <algorithm>
#include <future>
#include <variant>

namespace tb::ui
{
namespace
{
template <typename Value, typename Error>
Error resultError(const Result<Value, Error>& result)
{
  return std::get<Error>(result.error());
}

QString statusName(const AcceptanceRunStatus status)
{
  switch (status)
  {
  case AcceptanceRunStatus::Passed:
    return "passed";
  case AcceptanceRunStatus::Failed:
    return "failed";
  case AcceptanceRunStatus::Error:
    return "error";
  case AcceptanceRunStatus::Cancelled:
    return "cancelled";
  }
  return "error";
}

AcceptanceRunStatus aggregate(
  const std::vector<AcceptanceComparisonRunReport>& comparisons)
{
  if (std::ranges::any_of(comparisons, [](const auto& report) {
        return report.status == AcceptanceRunStatus::Cancelled;
      }))
    return AcceptanceRunStatus::Cancelled;
  if (std::ranges::any_of(comparisons, [](const auto& report) {
        return report.status == AcceptanceRunStatus::Error;
      }))
    return AcceptanceRunStatus::Error;
  if (std::ranges::any_of(comparisons, [](const auto& report) {
        return report.status == AcceptanceRunStatus::Failed;
      }))
    return AcceptanceRunStatus::Failed;
  return AcceptanceRunStatus::Passed;
}

AcceptanceRunStatus aggregate(const std::vector<AcceptanceAssertionRunReport>& assertions)
{
  if (std::ranges::any_of(assertions, [](const auto& report) {
        return report.status == AcceptanceRunStatus::Cancelled;
      }))
    return AcceptanceRunStatus::Cancelled;
  if (std::ranges::any_of(assertions, [](const auto& report) {
        return report.status == AcceptanceRunStatus::Error;
      }))
    return AcceptanceRunStatus::Error;
  if (std::ranges::any_of(assertions, [](const auto& report) {
        return report.status == AcceptanceRunStatus::Failed;
      }))
    return AcceptanceRunStatus::Failed;
  return AcceptanceRunStatus::Passed;
}

AcceptanceRunStatus aggregate(
  const AcceptanceRunStatus lhs, const AcceptanceRunStatus rhs)
{
  const auto priority = [](const AcceptanceRunStatus status) {
    switch (status)
    {
    case AcceptanceRunStatus::Passed:
      return 0;
    case AcceptanceRunStatus::Failed:
      return 1;
    case AcceptanceRunStatus::Error:
      return 2;
    case AcceptanceRunStatus::Cancelled:
      return 3;
    }
    return 2;
  };
  return priority(lhs) >= priority(rhs) ? lhs : rhs;
}

bool imageComparisonPassed(const AcceptanceImageComparisonReport& report)
{
  return std::ranges::all_of(report.metrics, &AcceptanceImageMetricReport::passed);
}

} // namespace

QJsonObject acceptanceSuiteRunReportToJson(const AcceptanceSuiteRunReport& report)
{
  auto comparisons = QJsonArray{};
  for (const auto& comparison : report.comparisons)
  {
    auto assertions = QJsonArray{};
    for (const auto& assertion : comparison.assertions)
    {
      auto value = QJsonObject{
        {"id", QString::fromStdString(assertion.assertionId)},
        {"status", statusName(assertion.status)},
        {"error", QString::fromStdString(assertion.error)}};
      if (assertion.report)
        value.insert("coverage", assertion.report->coverage);
      assertions.push_back(value);
    }
    auto value = QJsonObject{
      {"id", QString::fromStdString(comparison.comparisonId)},
      {"status", statusName(comparison.status)},
      {"error", QString::fromStdString(comparison.error)},
      {"assertions", assertions}};
    if (comparison.capture)
    {
      value.insert(
        "imageComparison",
        acceptanceImageComparisonReportToJson(comparison.capture->imageComparison));
      value.insert(
        "referenceRevision",
        static_cast<qint64>(comparison.capture->reference.document.revision));
      value.insert(
        "targetRevision",
        static_cast<qint64>(comparison.capture->target.document.revision));
      value.insert(
        "referenceDocumentId",
        QString::fromStdString(comparison.capture->reference.document.documentId));
      value.insert(
        "targetDocumentId",
        QString::fromStdString(comparison.capture->target.document.documentId));
    }
    comparisons.push_back(value);
  }
  return {
    {"suiteId", QString::fromStdString(report.suiteId)},
    {"storeRevision", static_cast<qint64>(report.storeRevision)},
    {"status", statusName(report.status)},
    {"comparisons", comparisons}};
}

AcceptanceSuiteRunner::AcceptanceSuiteRunner(
  AcceptanceViewStore& store,
  AcceptanceComparisonRunner& comparisons,
  AcceptanceGeometryProvider& geometry)
  : m_store{store}
  , m_comparisons{comparisons}
  , m_geometry{geometry}
{
}

AcceptanceSuiteRunReport AcceptanceSuiteRunner::run(
  const std::string& suiteId, const AcceptanceSuiteRunOptions& options) const
{
  auto result = AcceptanceSuiteRunReport{};
  result.suiteId = suiteId;
  const auto loaded = m_store.load();
  if (loaded.is_error())
  {
    result.status = AcceptanceRunStatus::Error;
    result.comparisons.push_back(
      {"", AcceptanceRunStatus::Error, std::nullopt, {}, resultError(loaded).message});
    return result;
  }
  const auto& project = loaded.value();
  result.storeRevision = project.revision;
  const auto suiteIt =
    std::ranges::find(project.suites, suiteId, &AcceptanceSuite::suiteId);
  if (suiteIt == project.suites.end())
  {
    result.status = AcceptanceRunStatus::Error;
    result.comparisons.push_back(
      {"",
       AcceptanceRunStatus::Error,
       std::nullopt,
       {},
       "Acceptance suite was not found"});
    return result;
  }
  auto ids = suiteIt->comparisonIds;
  if (!options.comparisonFilter.empty())
  {
    std::erase_if(ids, [&](const auto& id) {
      return std::ranges::find(options.comparisonFilter, id)
             == options.comparisonFilter.end();
    });
  }
  std::ranges::sort(ids);
  for (size_t captureIndex = 0u; captureIndex < ids.size(); ++captureIndex)
  {
    const auto& id = ids[captureIndex];
    if (options.cancelled && options.cancelled())
    {
      result.comparisons.push_back(
        {id, AcceptanceRunStatus::Cancelled, std::nullopt, {}, {}});
      continue;
    }
    const auto capture = [&]() {
      const auto lock = std::lock_guard{m_captureMutex};
      return m_comparisons.run(project, id);
    }();
    if (capture.is_error())
    {
      result.comparisons.push_back(
        {id, AcceptanceRunStatus::Error, std::nullopt, {}, resultError(capture).message});
      continue;
    }
    const auto captureStatus = imageComparisonPassed(capture.value().imageComparison)
                                 ? AcceptanceRunStatus::Passed
                                 : AcceptanceRunStatus::Failed;
    result.comparisons.push_back({id, captureStatus, capture.value(), {}, {}});
    const auto current = m_store.load();
    if (current.is_error() || current.value().revision != result.storeRevision)
    {
      result.comparisons.back().status = AcceptanceRunStatus::Error;
      result.comparisons.back().error =
        current.is_error() ? resultError(current).message
                           : "Acceptance store revision changed during run";
      for (auto remaining = captureIndex + 1u; remaining < ids.size(); ++remaining)
      {
        result.comparisons.push_back(
          {ids[remaining],
           AcceptanceRunStatus::Error,
           std::nullopt,
           {},
           "Acceptance store revision changed during run"});
      }
      break;
    }
  }
  const auto workers = std::max(size_t{1u}, options.maxCpuConcurrency);
  auto futures = std::vector<
    std::pair<size_t, std::future<std::vector<AcceptanceAssertionRunReport>>>>{};
  const auto collect = [&]() {
    auto entry = std::move(futures.front());
    futures.erase(futures.begin());
    auto& report = result.comparisons[entry.first];
    report.assertions = entry.second.get();
    report.status = aggregate(report.status, aggregate(report.assertions));
  };
  for (size_t index = 0u; index < result.comparisons.size(); ++index)
  {
    auto& report = result.comparisons[index];
    if (
      !report.capture || report.status == AcceptanceRunStatus::Error
      || report.status == AcceptanceRunStatus::Cancelled)
      continue;
    if (options.cancelled && options.cancelled())
    {
      report.status = AcceptanceRunStatus::Cancelled;
      continue;
    }
    const auto comparisonIt = std::ranges::find(
      project.comparisons, report.comparisonId, &AcceptanceComparison::id);
    if (comparisonIt == project.comparisons.end())
    {
      report.status = AcceptanceRunStatus::Error;
      report.error = "Acceptance comparison was not found during run";
      continue;
    }
    const auto geometry = m_geometry.geometryFor(report.capture->target.document);
    if (geometry.is_error())
    {
      report.status = AcceptanceRunStatus::Error;
      report.error = resultError(geometry).message;
      continue;
    }
    const auto assertions = comparisonIt->assertions;
    const auto context = AcceptanceAssertionContext{
      comparisonIt->alignment,
      AcceptanceAssertionSpace::Target,
      report.capture->target.normalizedCamera,
      0.01};
    const auto query = geometry.value();
    const auto evaluate = [assertions, context, query, cancelled = options.cancelled]() {
      auto reports = std::vector<AcceptanceAssertionRunReport>{};
      AcceptanceAssertionEvaluator evaluator{*query};
      for (const auto& assertion : assertions)
      {
        if (cancelled && cancelled())
        {
          reports.push_back(
            {assertion.id, AcceptanceRunStatus::Cancelled, std::nullopt, {}});
          break;
        }
        const auto value = evaluator.evaluate(assertion, context);
        if (value.is_error())
          reports.push_back(
            {assertion.id,
             AcceptanceRunStatus::Error,
             std::nullopt,
             resultError(value).message});
        else
          reports.push_back(
            {assertion.id,
             value.value().passed ? AcceptanceRunStatus::Passed
                                  : AcceptanceRunStatus::Failed,
             value.value(),
             {}});
      }
      return reports;
    };
    if (query->isThreadSafe())
    {
      futures.emplace_back(index, std::async(std::launch::async, evaluate));
    }
    else
    {
      report.assertions = evaluate();
      report.status = aggregate(report.status, aggregate(report.assertions));
    }
    if (futures.size() >= workers)
      collect();
  }
  while (!futures.empty())
    collect();
  const auto final = m_store.load();
  if (final.is_error() || final.value().revision != result.storeRevision)
  {
    const auto error = final.is_error() ? resultError(final).message
                                        : "Acceptance store revision changed during run";
    for (auto& report : result.comparisons)
    {
      if (report.status != AcceptanceRunStatus::Cancelled)
      {
        report.status = AcceptanceRunStatus::Error;
        report.error = error;
      }
    }
  }
  result.status = aggregate(result.comparisons);
  return result;
}
} // namespace tb::ui
