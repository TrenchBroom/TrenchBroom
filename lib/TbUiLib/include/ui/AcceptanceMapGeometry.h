/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#pragma once

#include "ui/AcceptanceSuiteRunner.h"

#include <functional>
#include <string_view>

namespace tb::mdl
{
class Map;
}

namespace tb::ui
{
class AutomationDocumentRegistry;

/** A structural ray query over one explicitly supplied model map. */
class AcceptanceMapGeometryQuery : public AcceptanceGeometryQuery
{
public:
  explicit AcceptanceMapGeometryQuery(mdl::Map& map);

  Result<std::vector<AcceptanceGeometryHit>, AcceptanceGeometryError> cast(
    const AcceptanceStructuralRay& ray) const override;

  Result<bool, AcceptanceGeometryError> intersects(
    const vm::bbox3d& bounds) const override;

private:
  mdl::Map& m_map;
};

/**
 * The application-level bridge resolves an already-open document by its capture
 * identity. It deliberately has no foreground-document fallback.
 */
class AcceptanceMapResolver
{
public:
  virtual ~AcceptanceMapResolver() = default;
  virtual Result<mdl::Map*, AcceptanceGeometryError> resolve(
    const AcceptanceCaptureDocumentIdentity& document) = 0;
};

/** Geometry provider suitable for AcceptanceSuiteRunner and AcceptanceAutomationService.
 */
class AcceptanceMapGeometryProvider : public AcceptanceGeometryProvider
{
public:
  explicit AcceptanceMapGeometryProvider(AcceptanceMapResolver& resolver);

  Result<std::shared_ptr<const AcceptanceGeometryQuery>, AcceptanceGeometryError>
  geometryFor(const AcceptanceCaptureDocumentIdentity& document) override;

private:
  AcceptanceMapResolver& m_resolver;
};

/** Resolves only the exact live AutomationDocumentRegistry identity supplied by capture.
 */
class AcceptanceAutomationMapResolver : public AcceptanceMapResolver
{
public:
  using HiddenMapResolver = std::function<mdl::Map*(std::string_view documentId)>;

  explicit AcceptanceAutomationMapResolver(
    const AutomationDocumentRegistry& documents, HiddenMapResolver hiddenMap = {});

  Result<mdl::Map*, AcceptanceGeometryError> resolve(
    const AcceptanceCaptureDocumentIdentity& document) override;

private:
  const AutomationDocumentRegistry& m_documents;
  HiddenMapResolver m_hiddenMap;
};

} // namespace tb::ui
