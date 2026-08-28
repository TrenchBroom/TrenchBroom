/* Copyright (C) 2026 */
#pragma once

#include "ui/AcceptanceAssertions.h"
#include "ui/AcceptanceComparisonRunner.h"
#include "ui/AcceptanceViewStore.h"

#include <functional>
#include <memory>
#include <mutex>

namespace tb::ui
{
class AcceptanceGeometryProvider
{
public:
  virtual ~AcceptanceGeometryProvider() = default;
  virtual Result<std::shared_ptr<const AcceptanceGeometryQuery>, AcceptanceGeometryError>
  geometryFor(const AcceptanceCaptureDocumentIdentity& document) = 0;
};

enum class AcceptanceRunStatus
{
  Passed,
  Failed,
  Error,
  Cancelled
};

struct AcceptanceAssertionRunReport
{
  std::string assertionId;
  AcceptanceRunStatus status = AcceptanceRunStatus::Error;
  std::optional<AcceptanceAssertionReport> report;
  std::string error;
};

struct AcceptanceComparisonRunReport
{
  std::string comparisonId;
  AcceptanceRunStatus status = AcceptanceRunStatus::Error;
  std::optional<AcceptancePairedCaptureReport> capture;
  std::vector<AcceptanceAssertionRunReport> assertions;
  std::string error;
};

struct AcceptanceSuiteRunReport
{
  std::string suiteId;
  size_t storeRevision = 0u;
  AcceptanceRunStatus status = AcceptanceRunStatus::Error;
  std::vector<AcceptanceComparisonRunReport> comparisons;
};

QJsonObject acceptanceSuiteRunReportToJson(const AcceptanceSuiteRunReport& report);

struct AcceptanceSuiteRunOptions
{
  std::vector<std::string> comparisonFilter;
  size_t maxCpuConcurrency = 1u;
  std::function<bool()> cancelled;
};

class AcceptanceSuiteRunner
{
public:
  AcceptanceSuiteRunner(
    AcceptanceViewStore& store,
    AcceptanceComparisonRunner& comparisons,
    AcceptanceGeometryProvider& geometry);

  AcceptanceSuiteRunReport run(
    const std::string& suiteId, const AcceptanceSuiteRunOptions& options = {}) const;

private:
  AcceptanceViewStore& m_store;
  AcceptanceComparisonRunner& m_comparisons;
  AcceptanceGeometryProvider& m_geometry;
  mutable std::mutex m_captureMutex;
};
} // namespace tb::ui
