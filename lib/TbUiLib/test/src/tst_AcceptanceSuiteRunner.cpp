/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QImage>
#include <QJsonArray>
#include <QTemporaryDir>

#include "ui/AcceptanceSuiteRunner.h"
#include "ui/CatchConfig.h"

#include <functional>
#include <variant>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

QJsonArray vector(const double x, const double y, const double z)
{
  return {x, y, z};
}

AcceptanceProject makeProject()
{
  auto project = AcceptanceProject{};
  project.views = {
    {"source", "Source", {}, {800, 600}, "textured", {}},
    {"target", "Target", {}, {800, 600}, "textured", {}},
  };

  auto sightline = AcceptanceAssertion{};
  sightline.id = "clear";
  sightline.type = AcceptanceAssertionType::ClearSightline;
  sightline.configuration = {
    {"origin", vector(0.0, 0.0, 0.0)},
    {"target", vector(10.0, 0.0, 0.0)},
    {"corridorWidth", 0.0},
    {"grid", 1},
    {"minimumClearFraction", 1.0}};

  project.comparisons = {
    {"b",
     "B",
     {"reference-b.map", "source"},
     {"target-b.map", "target"},
     {},
     {},
     {},
     {sightline}},
    {"a",
     "A",
     {"reference-a.map", "source"},
     {"target-a.map", "target"},
     {},
     {},
     {},
     {sightline}},
  };
  project.suites = {{AcceptanceSchemaVersion, "suite", "Suite", {"b", "a"}}};
  return project;
}

class Capture : public AcceptanceVirtualCapture
{
public:
  std::vector<AcceptanceVirtualCaptureRequest> requests;
  std::vector<std::filesystem::path> colorPaths;
  std::function<void()> onSecondCapture;

  Result<AcceptanceVirtualCaptureResult, AcceptanceVirtualCaptureError> capture(
    const AcceptanceVirtualCaptureRequest& request) override
  {
    requests.push_back(request);
    if (requests.size() == 2u && onSecondCapture)
      onSecondCapture();
    const auto colorPath = colorPaths.empty()
                             ? std::filesystem::path{"/tmp"}
                                 / ("capture-" + std::to_string(requests.size()) + ".png")
                             : colorPaths.at(requests.size() - 1u);
    return AcceptanceVirtualCaptureResult{
      {request.documentPath,
       "document-" + std::to_string(requests.size()),
       requests.size() - 1u},
      request.camera,
      request.size,
      colorPath,
      std::nullopt,
      "renderer-v1",
    };
  }
};

class Query : public AcceptanceGeometryQuery
{
public:
  explicit Query(const bool blocked)
    : m_blocked{blocked}
  {
  }

  Result<std::vector<AcceptanceGeometryHit>, AcceptanceGeometryError> cast(
    const AcceptanceStructuralRay&) const override
  {
    return m_blocked ? std::vector<AcceptanceGeometryHit>{{1.0}}
                     : std::vector<AcceptanceGeometryHit>{};
  }

private:
  bool m_blocked;
};

class Geometry : public AcceptanceGeometryProvider
{
public:
  size_t requests = 0u;

  Result<std::shared_ptr<const AcceptanceGeometryQuery>, AcceptanceGeometryError>
  geometryFor(const AcceptanceCaptureDocumentIdentity& document) override
  {
    ++requests;
    const auto blocked = document.path.filename() == "target-b.map";
    return std::make_shared<Query>(blocked);
  }
};

} // namespace

TEST_CASE("AcceptanceSuiteRunner")
{
  auto directory = QTemporaryDir{};
  REQUIRE(directory.isValid());
  const auto projectPath =
    std::filesystem::path{directory.path().toStdString()} / "acceptance.json";
  auto store = AcceptanceViewStore{projectPath};
  REQUIRE(store.replace(makeProject(), 0u).is_success());

  SECTION("executes sorted filtered comparisons with mixed assertion outcomes")
  {
    auto capture = Capture{};
    auto geometry = Geometry{};
    auto comparisons = AcceptanceComparisonRunner{projectPath, capture};
    const auto report =
      AcceptanceSuiteRunner{store, comparisons, geometry}.run("suite", {{}, 2u, {}});

    CHECK(report.status == AcceptanceRunStatus::Failed);
    REQUIRE(report.comparisons.size() == 2u);
    CHECK(report.comparisons[0].comparisonId == "a");
    CHECK(report.comparisons[0].status == AcceptanceRunStatus::Passed);
    CHECK(report.comparisons[1].comparisonId == "b");
    CHECK(report.comparisons[1].status == AcceptanceRunStatus::Failed);
    CHECK(capture.requests.size() == 4u);
    CHECK(geometry.requests == 2u);
    CHECK(acceptanceSuiteRunReportToJson(report).value("status") == "failed");

    auto filter = AcceptanceSuiteRunOptions{};
    filter.comparisonFilter = {"a"};
    const auto filtered =
      AcceptanceSuiteRunner{store, comparisons, geometry}.run("suite", filter);
    REQUIRE(filtered.comparisons.size() == 1u);
    CHECK(filtered.comparisons.front().comparisonId == "a");
    CHECK(filtered.status == AcceptanceRunStatus::Passed);
  }

  SECTION("cancels before capture without issuing requests")
  {
    auto capture = Capture{};
    auto geometry = Geometry{};
    auto comparisons = AcceptanceComparisonRunner{projectPath, capture};
    const auto report = AcceptanceSuiteRunner{store, comparisons, geometry}.run(
      "suite", {{}, 1u, []() { return true; }});

    CHECK(report.status == AcceptanceRunStatus::Cancelled);
    CHECK(capture.requests.empty());
    REQUIRE(report.comparisons.size() == 2u);
    CHECK(report.comparisons[0].status == AcceptanceRunStatus::Cancelled);
    CHECK(report.comparisons[1].status == AcceptanceRunStatus::Cancelled);
  }

  SECTION("fails a comparison when an image metric fails and still runs assertions")
  {
    auto project = makeProject();
    const auto comparison =
      std::ranges::find(project.comparisons, "a", &AcceptanceComparison::id);
    REQUIRE(comparison != project.comparisons.end());
    comparison->metrics = {
      {"color",
       AcceptanceMetricType::Color,
       std::nullopt,
       {{"absoluteError", 0.0}, {"relativeError", 0.0}, {"maxChangedFraction", 0.0}}}};
    REQUIRE(store.replace(project, 1u).is_success());

    const auto referencePath =
      std::filesystem::path{directory.path().toStdString()} / "reference.png";
    const auto targetPath =
      std::filesystem::path{directory.path().toStdString()} / "target.png";
    auto image = QImage{2, 2, QImage::Format_RGBA8888};
    image.fill(Qt::black);
    REQUIRE(image.save(QString::fromStdString(referencePath.string())));
    image.fill(Qt::white);
    REQUIRE(image.save(QString::fromStdString(targetPath.string())));

    auto capture = Capture{};
    capture.colorPaths = {referencePath, targetPath};
    auto geometry = Geometry{};
    auto comparisons = AcceptanceComparisonRunner{projectPath, capture};
    auto options = AcceptanceSuiteRunOptions{};
    options.comparisonFilter = {"a"};
    const auto report =
      AcceptanceSuiteRunner{store, comparisons, geometry}.run("suite", options);

    CHECK(report.status == AcceptanceRunStatus::Failed);
    REQUIRE(report.comparisons.size() == 1u);
    CHECK(report.comparisons.front().status == AcceptanceRunStatus::Failed);
    REQUIRE(report.comparisons.front().assertions.size() == 1u);
    CHECK(
      report.comparisons.front().assertions.front().status
      == AcceptanceRunStatus::Passed);
    const auto json = acceptanceSuiteRunReportToJson(report);
    CHECK(
      json.value("comparisons")
        .toArray()
        .at(0)
        .toObject()
        .value("imageComparison")
        .toObject()
        .value("passed")
      == false);
  }

  SECTION("propagates a store revision change after serialized capture")
  {
    auto capture = Capture{};
    capture.onSecondCapture = [&]() {
      const auto update =
        store.update(1u, [](AcceptanceProject&) { return AcceptanceValidationResult{}; });
      REQUIRE(update.is_success());
    };
    auto geometry = Geometry{};
    auto comparisons = AcceptanceComparisonRunner{projectPath, capture};
    const auto report = AcceptanceSuiteRunner{store, comparisons, geometry}.run("suite");

    CHECK(report.status == AcceptanceRunStatus::Error);
    REQUIRE(report.comparisons.size() == 2u);
    CHECK(report.comparisons[0].comparisonId == "a");
    CHECK(report.comparisons[0].status == AcceptanceRunStatus::Error);
    CHECK(report.comparisons[0].error == "Acceptance store revision changed during run");
    CHECK(report.comparisons[1].comparisonId == "b");
    CHECK(report.comparisons[1].status == AcceptanceRunStatus::Error);
  }
}

} // namespace tb::ui
