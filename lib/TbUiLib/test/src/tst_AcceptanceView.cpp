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

#include "ui/AcceptanceView.h"
#include "ui/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceProject makeProject()
{
  auto project = AcceptanceProject{};
  project.views = {
    {"source-view", "Source", {}, {1600, 900}, "textured", {true, false, false}},
    {"target-view", "Target", {}, {1600, 900}, "textured", {true, false, false}},
  };
  project.comparisons = {{
    "entry",
    "Entry",
    {"../maps/source.map", "source-view"},
    {"../maps/target.map", "target-view"},
    {},
    {{"portal", 0.1, 0.2, 0.5, 0.4}},
    {{"silhouette", AcceptanceMetricType::Silhouette, "portal", {}}},
    {{"visible", AcceptanceAssertionType::BoundsVisible, "portal", std::nullopt, {}},
     {"entry-clearance",
      AcceptanceAssertionType::PlayerClearance,
      std::nullopt,
      std::nullopt,
      {{"start", QJsonArray{0.0, 0.0, 0.0}}, {"radius", 16.0}, {"height", 56.0}}}},
  }};
  project.suites = {
    {AcceptanceSchemaVersion, "front-gardens", "Front gardens", {"entry"}}};
  return project;
}

AcceptanceProject makeContextProject()
{
  auto project = makeProject();
  project.contexts = {{
    "unrest-rebuild",
    "Unrest rebuild",
    "../maps/source.map",
    "../maps/target.map",
    {},
  }};
  project.comparisons.front().contextId = "unrest-rebuild";
  return project;
}

} // namespace

TEST_CASE("AcceptanceView")
{
  SECTION("round trips a complete schema-v2 project deterministically")
  {
    const auto project = makeProject();
    REQUIRE(validateAcceptanceProject(project).is_success());

    const auto json = acceptanceProjectToJson(project);
    const auto parsed = acceptanceProjectFromJson(json);
    REQUIRE(parsed.is_success());
    CHECK(parsed.value().views.size() == 2u);
    CHECK(parsed.value().comparisons.size() == 1u);
    CHECK(
      parsed.value().suites.front().comparisonIds == std::vector<std::string>{"entry"});
    CHECK(
      QJsonDocument{json}.toJson(QJsonDocument::Compact)
      == QJsonDocument{acceptanceProjectToJson(parsed.value())}.toJson(
        QJsonDocument::Compact));
  }

  SECTION("persists a reusable context without duplicating its binding in comparisons")
  {
    const auto project = makeContextProject();
    REQUIRE(validateAcceptanceProject(project).is_success());

    const auto json = acceptanceProjectToJson(project);
    REQUIRE(json.value("contexts").toArray().size() == 1);
    const auto comparison = json.value("comparisons").toArray().first().toObject();
    CHECK(comparison.value("contextId") == "unrest-rebuild");
    CHECK_FALSE(comparison.value("reference").toObject().contains("documentPath"));
    CHECK_FALSE(comparison.value("target").toObject().contains("documentPath"));
    CHECK_FALSE(comparison.contains("alignment"));

    const auto parsed = acceptanceProjectFromJson(json);
    REQUIRE(parsed.is_success());
    REQUIRE(parsed.value().comparisons.front().contextId);
    CHECK(*parsed.value().comparisons.front().contextId == "unrest-rebuild");
    CHECK(parsed.value().comparisons.front().reference.path == "../maps/source.map");
    CHECK(parsed.value().comparisons.front().target.path == "../maps/target.map");
  }

  SECTION("migrates a schema-v1 project to standalone schema-v2 comparisons")
  {
    auto json = acceptanceProjectToJson(makeProject());
    json.insert("schemaVersion", static_cast<qint64>(LegacyAcceptanceSchemaVersion));
    json.remove("contexts");
    auto suites = json.value("suites").toArray();
    auto suite = suites.first().toObject();
    suite.insert("schemaVersion", static_cast<qint64>(LegacyAcceptanceSchemaVersion));
    suites.replace(0, suite);
    json.insert("suites", suites);

    const auto parsed = acceptanceProjectFromJson(json);
    REQUIRE(parsed.is_success());
    CHECK(parsed.value().schemaVersion == AcceptanceSchemaVersion);
    CHECK(parsed.value().contexts.empty());
    CHECK_FALSE(parsed.value().comparisons.front().contextId);
    CHECK(
      acceptanceProjectToJson(parsed.value()).value("schemaVersion")
      == static_cast<qint64>(AcceptanceSchemaVersion));
  }

  SECTION("rejects unsupported schema and types")
  {
    auto json = acceptanceProjectToJson(makeProject());
    json.insert("schemaVersion", 99);
    CHECK(acceptanceProjectFromJson(json).is_error());

    json = acceptanceProjectToJson(makeProject());
    auto views = json.value("views").toArray();
    auto firstView = views.first().toObject();
    firstView.insert("renderMode", "future-mode");
    views.replace(0, firstView);
    json.insert("views", views);
    CHECK(acceptanceProjectFromJson(json).is_error());
  }

  SECTION("enforces references and portable document paths")
  {
    auto project = makeProject();
    project.comparisons.front().target.viewId = "missing";
    CHECK(validateAcceptanceProject(project).is_error());

    project = makeProject();
    project.comparisons.front().reference.path = "/absolute/source.map";
    CHECK(validateAcceptanceProject(project).is_error());

    project = makeContextProject();
    project.contexts.front().candidatePath = project.contexts.front().referencePath;
    CHECK(validateAcceptanceProject(project).is_error());

    project = makeContextProject();
    project.comparisons.front().target.path = "../maps/drifted.map";
    CHECK(validateAcceptanceProject(project).is_error());
  }

  SECTION("makes and resolves project-relative document paths")
  {
    const auto portable = makePortableAcceptancePath(
      "/projects/qa/acceptance.json", "/projects/maps/source.map");
    REQUIRE(portable.is_success());
    CHECK(portable.value() == std::filesystem::path{"../maps/source.map"});

    const auto resolved =
      resolveAcceptancePath("/projects/qa/acceptance.json", portable.value());
    REQUIRE(resolved.is_success());
    CHECK(resolved.value() == std::filesystem::path{"/projects/maps/source.map"});
  }
}

} // namespace tb::ui
