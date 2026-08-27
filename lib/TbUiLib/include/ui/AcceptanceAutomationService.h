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

#include "ui/AcceptanceSuiteRunner.h"

namespace tb::ui
{

enum class AcceptanceAutomationErrorCode
{
  MethodNotFound,
  InvalidParameters,
  StoreFailed,
  CaptureFailed,
};

struct AcceptanceAutomationError
{
  AcceptanceAutomationErrorCode code = AcceptanceAutomationErrorCode::InvalidParameters;
  std::string message;
};

using AcceptanceAutomationResult = Result<QJsonObject, AcceptanceAutomationError>;

/**
 * Stateless-request facade for the acceptance data model. The caller owns the store
 * and adapters; this service neither resolves map windows nor retains GUI state.
 * AutomationService can delegate every `acceptance.*` method to handle() in one call.
 */
class AcceptanceAutomationService
{
public:
  AcceptanceAutomationService(
    AcceptanceViewStore& store,
    AcceptanceVirtualCapture& capture,
    AcceptanceGeometryProvider& geometry);

  const std::filesystem::path& projectPath() const;

  AcceptanceAutomationResult handle(const QString& method, const QJsonObject& params);

private:
  AcceptanceAutomationResult list(const QString& kind) const;
  AcceptanceAutomationResult create(const QString& kind, const QJsonObject& params);
  AcceptanceAutomationResult update(const QString& kind, const QJsonObject& params);
  AcceptanceAutomationResult erase(const QString& kind, const QJsonObject& params);
  AcceptanceAutomationResult capture(const QJsonObject& params) const;
  AcceptanceAutomationResult run(const QJsonObject& params) const;
  AcceptanceAutomationResult evaluateAssertion(const QJsonObject& params) const;

  AcceptanceViewStore& m_store;
  AcceptanceComparisonRunner m_comparisons;
  AcceptanceSuiteRunner m_suites;
  AcceptanceGeometryProvider& m_geometry;
};

} // namespace tb::ui
