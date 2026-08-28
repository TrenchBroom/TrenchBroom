/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QJsonArray>
#include <QJsonDocument>

#include "ui/AcceptanceDivergencePolicy.h"
#include "ui/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceDivergencePolicy policy()
{
  auto result = AcceptanceDivergencePolicy{};
  result.rules = {
    {"export-repair",
     "Broken exporter door",
     AcceptanceDivergenceDomain::SolidSpace,
     AcceptanceDivergenceDirection::CandidateOnly,
     vm::bbox3d{{0.0, 0.0, 0.0}, {8.0, 8.0, 8.0}},
     AcceptanceDivergenceDisposition::AcceptedRepair,
     AcceptanceDivergenceProvenance::ExportDefect,
     "unrest-export-audit.md#door-7",
     "The export closes a player opening that is absent in the original client."},
    {"whole-map-waiver",
     "Known lighting limitation",
     AcceptanceDivergenceDomain::Visual,
     std::nullopt,
     std::nullopt,
     AcceptanceDivergenceDisposition::Waived,
     AcceptanceDivergenceProvenance::KnownLimitation,
     "issue-123",
     "The legacy palette cannot be reproduced by the current renderer."}};
  return result;
}

} // namespace

TEST_CASE("AcceptanceDivergencePolicy")
{
  SECTION(
    "classifies role-relative findings without treating the reference as authoritative")
  {
    const auto classifications = classifyAcceptanceDivergences(
      policy(),
      {{"door",
        AcceptanceDivergenceDomain::SolidSpace,
        AcceptanceDivergenceDirection::CandidateOnly,
        2u,
        vm::bbox3d{{2.0, 2.0, 2.0}, {4.0, 4.0, 4.0}}},
       {"unknown",
        AcceptanceDivergenceDomain::SolidSpace,
        AcceptanceDivergenceDirection::ReferenceOnly,
        1u,
        vm::bbox3d{{2.0, 2.0, 2.0}, {4.0, 4.0, 4.0}}}});

    REQUIRE(classifications.size() == 2u);
    CHECK(classifications[0].finding.id == "door");
    CHECK(
      classifications[0].disposition == AcceptanceDivergenceDisposition::AcceptedRepair);
    CHECK(classifications[0].severity == AcceptanceDivergenceSeverity::Info);
    CHECK(classifications[0].ruleId == "export-repair");
    const auto repairReport =
      acceptanceDivergenceClassificationToJson(classifications[0]);
    CHECK(repairReport.value("finding").toObject().value("direction") == "candidateOnly");
    CHECK(repairReport.value("disposition") == "acceptedRepair");
    CHECK(repairReport.value("severity") == "info");
    CHECK(repairReport.value("ruleId") == "export-repair");
    CHECK(classifications[1].finding.id == "unknown");
    CHECK(classifications[1].disposition == AcceptanceDivergenceDisposition::Review);
    CHECK(classifications[1].severity == AcceptanceDivergenceSeverity::Error);
    CHECK_FALSE(classifications[1].ruleId);
  }

  SECTION("does not waive an aggregate that only intersects a spatial rule")
  {
    const auto classifications = classifyAcceptanceDivergences(
      policy(),
      {{"overlap",
        AcceptanceDivergenceDomain::SolidSpace,
        AcceptanceDivergenceDirection::CandidateOnly,
        2u,
        vm::bbox3d{{4.0, 4.0, 4.0}, {12.0, 4.0, 4.0}}}});

    REQUIRE(classifications.size() == 1u);
    CHECK(classifications[0].disposition == AcceptanceDivergenceDisposition::Review);
  }

  SECTION("serializes rules in canonical id order and round trips provenance")
  {
    const auto json = acceptanceDivergencePolicyToJson(policy());
    const auto rules = json.value("rules").toArray();
    REQUIRE(rules.size() == 2);
    CHECK(rules[0].toObject().value("id") == "export-repair");
    CHECK(rules[0].toObject().value("provenance") == "exportDefect");
    CHECK(
      rules[0].toObject().value("sourceReference") == "unrest-export-audit.md#door-7");
    CHECK(rules[1].toObject().value("id") == "whole-map-waiver");

    const auto parsed = acceptanceDivergencePolicyFromJson(json);
    REQUIRE(parsed.is_success());
    CHECK(acceptanceDivergencePolicyToJson(parsed.value()) == json);
    CHECK(
      QJsonDocument{json}.toJson(QJsonDocument::Compact)
      == QJsonDocument{acceptanceDivergencePolicyToJson(parsed.value())}.toJson(
        QJsonDocument::Compact));
  }

  SECTION("rejects un-auditable rules")
  {
    auto invalid = policy();
    invalid.rules[0].rationale.clear();
    CHECK(validateAcceptanceDivergencePolicy(invalid).is_error());
    invalid = policy();
    invalid.rules[0].sourceReference.clear();
    CHECK(validateAcceptanceDivergencePolicy(invalid).is_error());
  }
}

} // namespace tb::ui
