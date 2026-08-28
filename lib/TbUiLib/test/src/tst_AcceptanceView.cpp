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
    {{"visible", AcceptanceAssertionType::BoundsVisible, "portal", std::nullopt, {}}},
  }};
  project.suites = {
    {AcceptanceSchemaVersion, "front-gardens", "Front gardens", {"entry"}}};
  return project;
}

} // namespace

TEST_CASE("AcceptanceView")
{
  SECTION("round trips a complete schema-v1 project deterministically")
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
