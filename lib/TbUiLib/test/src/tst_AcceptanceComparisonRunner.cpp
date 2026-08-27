/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceComparisonRunner.h"
#include "ui/CatchConfig.h"

#include <variant>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceProject makeProject()
{
  auto source =
    AcceptanceNamedView{"source-view", "Source", {}, {1600, 900}, "textured", {}};
  source.camera.position = {1.0, 2.0, 3.0};
  source.camera.direction = {0.0, 1.0, 0.0};
  source.camera.up = {0.0, 0.0, 1.0};
  source.size = {800, 600};
  source.overlays = {true, false, false};

  auto target =
    AcceptanceNamedView{"target-view", "Target", {}, {1600, 900}, "textured", {}};
  target.camera.position = {100.0, 200.0, 300.0};
  target.camera.direction = {1.0, 0.0, 0.0};
  target.camera.up = {0.0, 0.0, 1.0};
  target.size = {320, 200};
  target.renderMode = "flat";

  auto project = AcceptanceProject{};
  project.views = {source, target};
  project.comparisons = {{
    "comparison",
    "Comparison",
    {"../reference/source.map", "source-view"},
    {"../target/rebuilt.map", "target-view"},
    {},
    {},
    {},
    {},
  }};
  project.suites = {{AcceptanceSchemaVersion, "suite", "Suite", {"comparison"}}};
  return project;
}

class FakeCapture : public AcceptanceVirtualCapture
{
public:
  std::vector<AcceptanceVirtualCaptureRequest> requests;
  bool wrongDocument = false;

  Result<AcceptanceVirtualCaptureResult, AcceptanceVirtualCaptureError> capture(
    const AcceptanceVirtualCaptureRequest& request) override
  {
    requests.push_back(request);
    const auto resultPath = wrongDocument && requests.size() == 2u
                              ? std::filesystem::path{"wrong.map"}
                              : request.documentPath;
    return AcceptanceVirtualCaptureResult{
      {resultPath, "document-" + std::to_string(requests.size()), requests.size() - 1u},
      request.camera,
      request.size,
      std::filesystem::path{"/tmp"}
        / ("capture-" + std::to_string(requests.size()) + ".png"),
      std::nullopt,
      "renderer-v1",
    };
  }
};

} // namespace

TEST_CASE("AcceptanceComparisonRunner")
{
  const auto projectPath = std::filesystem::path{"/work/qa/acceptance.json"};

  SECTION("normalizes identity capture requests and captures reference then target")
  {
    const auto project = makeProject();
    auto capture = FakeCapture{};
    const auto report =
      AcceptanceComparisonRunner{projectPath, capture}.run(project, "comparison");
    REQUIRE(report.is_success());
    REQUIRE(capture.requests.size() == 2u);
    CHECK(capture.requests[0].documentPath == "/work/reference/source.map");
    CHECK(capture.requests[1].documentPath == "/work/target/rebuilt.map");
    CHECK(capture.requests[1].camera.position == capture.requests[0].camera.position);
    CHECK(capture.requests[1].size.width == 800);
    CHECK(capture.requests[1].renderMode == "textured");
    CHECK(report.value().reference.document.revision == 0u);
    CHECK(report.value().target.document.revision == 1u);
  }

  SECTION("applies a row-major affine matrix to the reference camera")
  {
    auto project = makeProject();
    auto& alignment = project.comparisons.front().alignment;
    alignment.type = AcceptanceAlignmentType::Matrix;
    alignment.matrix = {
      0.0,
      -1.0,
      0.0,
      10.0,
      1.0,
      0.0,
      0.0,
      20.0,
      0.0,
      0.0,
      1.0,
      30.0,
      0.0,
      0.0,
      0.0,
      1.0,
    };
    auto capture = FakeCapture{};
    const auto report =
      AcceptanceComparisonRunner{projectPath, capture}.run(project, "comparison");
    REQUIRE(report.is_success());
    const auto& target = report.value().requests.target.camera;
    CHECK(target.position == vm::vec3d{8.0, 21.0, 33.0});
    CHECK(target.direction == vm::vec3d{-1.0, 0.0, 0.0});
    CHECK(target.up == vm::vec3d{0.0, 0.0, 1.0});
  }

  SECTION("uses the authored target camera for independent comparisons")
  {
    auto project = makeProject();
    project.comparisons.front().alignment.type = AcceptanceAlignmentType::Independent;
    auto capture = FakeCapture{};
    const auto report =
      AcceptanceComparisonRunner{projectPath, capture}.run(project, "comparison");
    REQUIRE(report.is_success());
    CHECK(
      report.value().requests.target.camera.position == vm::vec3d{100.0, 200.0, 300.0});
    CHECK(report.value().requests.target.size.width == 800);
  }

  SECTION("requests EV6 depth only for depth-backed semantic metrics")
  {
    auto project = makeProject();
    project.comparisons.front().metrics = {{
      "silhouette",
      AcceptanceMetricType::Silhouette,
      std::nullopt,
      {{"maxChangedFraction", 0.0}},
    }};
    const auto semantic = makeAcceptancePairedCaptureRequests(
      projectPath, project, project.comparisons.front());
    REQUIRE(semantic.is_success());
    CHECK(semantic.value().reference.depth);
    CHECK(semantic.value().target.depth);

    project.comparisons.front().metrics.front().type = AcceptanceMetricType::Color;
    const auto color = makeAcceptancePairedCaptureRequests(
      projectPath, project, project.comparisons.front());
    REQUIRE(color.is_success());
    CHECK_FALSE(color.value().reference.depth);
    CHECK_FALSE(color.value().target.depth);
  }

  SECTION("reports deterministic alignment and capture echo failures")
  {
    auto project = makeProject();
    project.comparisons.front().alignment.type = AcceptanceAlignmentType::Landmarks;
    project.comparisons.front().alignment.landmarks = {
      {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}},
      {{1.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},
      {{0.0, 1.0, 0.0}, {0.0, 1.0, 0.0}},
    };
    auto capture = FakeCapture{};
    const auto deferred =
      AcceptanceComparisonRunner{projectPath, capture}.run(project, "comparison");
    REQUIRE(deferred.is_error());
    CHECK(
      std::get<AcceptanceComparisonError>(deferred.error()).code
      == AcceptanceComparisonErrorCode::AlignmentFailed);

    project = makeProject();
    auto& alignment = project.comparisons.front().alignment;
    alignment.type = AcceptanceAlignmentType::Matrix;
    alignment.matrix = {
      0.0,
      0.0,
      0.0,
      0.0,
      0.0,
      1.0,
      0.0,
      0.0,
      0.0,
      0.0,
      1.0,
      0.0,
      0.0,
      0.0,
      0.0,
      1.0,
    };
    const auto nonInvertible =
      AcceptanceComparisonRunner{projectPath, capture}.run(project, "comparison");
    REQUIRE(nonInvertible.is_error());
    CHECK(
      std::get<AcceptanceComparisonError>(nonInvertible.error()).code
      == AcceptanceComparisonErrorCode::AlignmentFailed);

    project = makeProject();
    capture.wrongDocument = true;
    const auto mismatch =
      AcceptanceComparisonRunner{projectPath, capture}.run(project, "comparison");
    REQUIRE(mismatch.is_error());
    CHECK(
      std::get<AcceptanceComparisonError>(mismatch.error()).code
      == AcceptanceComparisonErrorCode::CaptureEchoMismatch);
  }
}

} // namespace tb::ui
