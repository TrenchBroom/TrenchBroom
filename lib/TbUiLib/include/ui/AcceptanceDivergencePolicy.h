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

#include "vm/bbox.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace tb::ui
{

/** The comparison modality producing a finding. More modalities may be added safely. */
enum class AcceptanceDivergenceDomain
{
  SolidSpace,
  Visual,
  Traversal,
};

/**
 * The two occupancy directions are deliberately role-relative, not value-laden:
 * neither direction asserts that the reference or candidate is correct.
 */
enum class AcceptanceDivergenceDirection
{
  ReferenceOnly,
  CandidateOnly,
  Different,
};

enum class AcceptanceDivergenceDisposition
{
  Review,
  IntendedChange,
  AcceptedRepair,
  Waived,
};

enum class AcceptanceDivergenceSeverity
{
  Info,
  Warning,
  Error,
};

enum class AcceptanceDivergenceProvenance
{
  ManualReview,
  ExportDefect,
  DesignDecision,
  KnownLimitation,
};

struct AcceptanceDivergenceError
{
  std::string message;
};

/** A measurement-only result passed in by a solid-space, visual, or traversal engine. */
struct AcceptanceDivergenceFinding
{
  std::string id;
  AcceptanceDivergenceDomain domain = AcceptanceDivergenceDomain::SolidSpace;
  AcceptanceDivergenceDirection direction = AcceptanceDivergenceDirection::Different;
  size_t count = 0u;
  std::optional<vm::bbox3d> bounds;
};

/**
 * A durable exception/expectation. A spatial rule applies only when its scope wholly
 * contains a finding. Engines should partition findings when they need partial scope
 * classification, rather than silently treating an intersecting aggregate as waived.
 */
struct AcceptanceDivergenceRule
{
  std::string id;
  std::string name;
  AcceptanceDivergenceDomain domain = AcceptanceDivergenceDomain::SolidSpace;
  std::optional<AcceptanceDivergenceDirection> direction;
  std::optional<vm::bbox3d> scope;
  AcceptanceDivergenceDisposition disposition = AcceptanceDivergenceDisposition::Review;
  AcceptanceDivergenceProvenance provenance =
    AcceptanceDivergenceProvenance::ManualReview;
  std::string sourceReference;
  std::string rationale;
};

/** Severity is policy, never an intrinsic property of the measurement. */
struct AcceptanceDivergenceSeverityPolicy
{
  AcceptanceDivergenceSeverity review = AcceptanceDivergenceSeverity::Error;
  AcceptanceDivergenceSeverity intendedChange = AcceptanceDivergenceSeverity::Info;
  AcceptanceDivergenceSeverity acceptedRepair = AcceptanceDivergenceSeverity::Info;
  AcceptanceDivergenceSeverity waived = AcceptanceDivergenceSeverity::Warning;
};

struct AcceptanceDivergencePolicy
{
  AcceptanceDivergenceSeverityPolicy severity;
  std::vector<AcceptanceDivergenceRule> rules;
};

struct AcceptanceDivergenceClassification
{
  AcceptanceDivergenceFinding finding;
  AcceptanceDivergenceDisposition disposition = AcceptanceDivergenceDisposition::Review;
  AcceptanceDivergenceSeverity severity = AcceptanceDivergenceSeverity::Error;
  std::optional<std::string> ruleId;
};

using AcceptanceDivergencePolicyResult =
  Result<AcceptanceDivergencePolicy, AcceptanceDivergenceError>;
using AcceptanceDivergencePolicyValidationResult =
  Result<void, AcceptanceDivergenceError>;

AcceptanceDivergencePolicyValidationResult validateAcceptanceDivergencePolicy(
  const AcceptanceDivergencePolicy& policy);

/** Returns an explicit review finding for every unmatched or spatially ambiguous input.
 */
std::vector<AcceptanceDivergenceClassification> classifyAcceptanceDivergences(
  const AcceptanceDivergencePolicy& policy,
  std::vector<AcceptanceDivergenceFinding> findings);

/** Serializes a measurement result in a stable form for gate reports. */
QJsonObject acceptanceDivergenceFindingToJson(const AcceptanceDivergenceFinding& finding);

/** Serializes the policy decision alongside its unmodified measurement. */
QJsonObject acceptanceDivergenceClassificationToJson(
  const AcceptanceDivergenceClassification& classification);

/** Serializes canonical key and rule order, suitable for durable acceptance projects. */
QJsonObject acceptanceDivergencePolicyToJson(const AcceptanceDivergencePolicy& policy);
AcceptanceDivergencePolicyResult acceptanceDivergencePolicyFromJson(
  const QJsonObject& json);

} // namespace tb::ui
