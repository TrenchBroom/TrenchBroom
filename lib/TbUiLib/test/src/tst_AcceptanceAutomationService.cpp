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

#include "ui/AcceptanceAutomationService.h"
#include "ui/CatchConfig.h"

#include <variant>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceProject makeProject()
{
  auto project = AcceptanceProject{};
  project.views = {
    {"source", "Source", {}, {800, 600}, "textured", {}},
    {"target", "Target", {}, {800, 600}, "textured", {}},
  };
  project.comparisons = {{
    "comparison",
    "Comparison",
    {"source.map", "source"},
    {"target.map", "target"},
    {},
    {},
    {},
    {},
  }};
  project.suites = {{AcceptanceSchemaVersion, "suite", "Suite", {"comparison"}}};
  return project;
}

class Capture : public AcceptanceVirtualCapture
{
public:
  std::vector<AcceptanceVirtualCaptureRequest> requests;
  std::vector<std::filesystem::path> colorPaths;

  Result<AcceptanceVirtualCaptureResult, AcceptanceVirtualCaptureError> capture(
    const AcceptanceVirtualCaptureRequest& request) override
  {
    requests.push_back(request);
    const auto colorPath = colorPaths.empty()
                             ? std::filesystem::path{"/tmp"}
                                 / ("capture-" + std::to_string(requests.size()) + ".png")
                             : colorPaths[(requests.size() - 1u) % colorPaths.size()];
    return AcceptanceVirtualCaptureResult{
      {request.documentPath, "document-" + std::to_string(requests.size()), 0u},
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
  Result<std::vector<AcceptanceGeometryHit>, AcceptanceGeometryError> cast(
    const AcceptanceStructuralRay&) const override
  {
    return std::vector<AcceptanceGeometryHit>{};
  }
};

class Geometry : public AcceptanceGeometryProvider
{
public:
  std::optional<AcceptanceCaptureDocumentIdentity> lastDocument;

  Result<std::shared_ptr<const AcceptanceGeometryQuery>, AcceptanceGeometryError>
  geometryFor(const AcceptanceCaptureDocumentIdentity& document) override
  {
    lastDocument = document;
    return std::make_shared<Query>();
  }
};

QJsonObject item(const AcceptanceProject& project, const char* kind, const auto index)
{
  return acceptanceProjectToJson(project).value(kind).toArray()[index].toObject();
}

} // namespace

TEST_CASE("AcceptanceAutomationService")
{
  auto directory = QTemporaryDir{};
  REQUIRE(directory.isValid());
  const auto projectPath =
    std::filesystem::path{directory.path().toStdString()} / "acceptance.json";
  auto store = AcceptanceViewStore{projectPath};
  auto capture = Capture{};
  auto geometry = Geometry{};
  auto service = AcceptanceAutomationService{store, capture, geometry};
  const auto project = makeProject();

  SECTION("routes optimistic CRUD and reports the explicit store path")
  {
    const auto source = service.handle(
      "acceptance.views.create",
      {{"expectedRevision", 0}, {"view", item(project, "views", 0)}});
    REQUIRE(source.is_success());
    CHECK(source.value().value("revision") == 1);
    CHECK(
      source.value().value("projectPath")
      == QString::fromStdString(projectPath.generic_string()));

    REQUIRE(service
              .handle(
                "acceptance.views.create",
                {{"expectedRevision", 1}, {"view", item(project, "views", 1)}})
              .is_success());
    REQUIRE(
      service
        .handle(
          "acceptance.comparisons.create",
          {{"expectedRevision", 2}, {"comparison", item(project, "comparisons", 0)}})
        .is_success());
    REQUIRE(service
              .handle(
                "acceptance.suites.create",
                {{"expectedRevision", 3}, {"suite", item(project, "suites", 0)}})
              .is_success());

    const auto views = service.handle("acceptance.views.list", {});
    REQUIRE(views.is_success());
    CHECK(views.value().value("items").toArray().size() == 2);

    auto updated = item(project, "views", 0);
    updated.insert("name", "Updated Source");
    const auto update = service.handle(
      "acceptance.views.update",
      {{"id", "source"}, {"expectedRevision", 4}, {"view", updated}});
    REQUIRE(update.is_success());
    CHECK(update.value().value("revision") == 5);

    const auto stale = service.handle(
      "acceptance.views.update",
      {{"id", "source"}, {"expectedRevision", 4}, {"view", updated}});
    REQUIRE(stale.is_error());
    CHECK(
      std::get<AcceptanceAutomationError>(stale.error()).code
      == AcceptanceAutomationErrorCode::StoreFailed);

    REQUIRE(
      service
        .handle("acceptance.suites.delete", {{"id", "suite"}, {"expectedRevision", 5}})
        .is_success());
    REQUIRE(service
              .handle(
                "acceptance.comparisons.delete",
                {{"id", "comparison"}, {"expectedRevision", 6}})
              .is_success());
  }

  SECTION("captures and runs a saved suite without GUI state")
  {
    auto metricProject = project;
    metricProject.comparisons.front().metrics = {{
      "pixels",
      AcceptanceMetricType::Color,
      std::nullopt,
      {{"absoluteError", 0.0}, {"relativeError", 0.0}, {"maxChangedFraction", 0.0}},
    }};
    const auto referencePath =
      std::filesystem::path{directory.path().toStdString()} / "reference.png";
    const auto targetPath =
      std::filesystem::path{directory.path().toStdString()} / "target.png";
    auto image = QImage{2, 2, QImage::Format_RGBA8888};
    image.fill(Qt::black);
    REQUIRE(image.save(QString::fromStdString(referencePath.string())));
    REQUIRE(image.save(QString::fromStdString(targetPath.string())));
    capture.colorPaths = {referencePath, targetPath};
    REQUIRE(store.replace(metricProject, 0u).is_success());

    const auto paired =
      service.handle("acceptance.capture", {{"comparisonId", "comparison"}});
    REQUIRE(paired.is_success());
    CHECK(capture.requests.size() == 2u);
    CHECK(
      paired.value().value("capture").toObject().value("comparisonId") == "comparison");
    CHECK(
      paired.value()
        .value("capture")
        .toObject()
        .value("imageComparison")
        .toObject()
        .value("metrics")
        .toArray()
        .size()
      == 1);

    const auto run =
      service.handle("acceptance.run", {{"suiteId", "suite"}, {"maxCpuConcurrency", 2}});
    REQUIRE(run.is_success());
    CHECK(run.value().value("report").toObject().value("status") == "passed");

    const auto unsupported = service.handle("acceptance.nope", {});
    REQUIRE(unsupported.is_error());
    CHECK(
      std::get<AcceptanceAutomationError>(unsupported.error()).code
      == AcceptanceAutomationErrorCode::MethodNotFound);
  }

  SECTION("evaluates a one-shot assertion against only its explicit document identity")
  {
    const auto evaluated = service.handle(
      "acceptance.assertions.evaluate",
      {{"document",
        QJsonObject{
          {"path", "/maps/explicit.map"},
          {"documentId", "captured-42"},
          {"revision", 7}}},
       {"assertion",
        QJsonObject{
          {"id", "clear"},
          {"type", "clearSightline"},
          {"configuration",
           QJsonObject{
             {"origin", QJsonArray{0.0, 0.0, 0.0}},
             {"target", QJsonArray{0.0, 64.0, 0.0}},
           }}}},
       {"context", QJsonObject{{"geometrySpace", "reference"}, {"tolerance", 0.01}}}});

    REQUIRE(evaluated.is_success());
    CHECK(evaluated.value().value("status") == "passed");
    CHECK(evaluated.value().value("assertionId") == "clear");
    CHECK(evaluated.value().value("report").toObject().value("totalRays") == 1);
    REQUIRE(geometry.lastDocument);
    CHECK(geometry.lastDocument->documentId == "captured-42");
    CHECK(geometry.lastDocument->path == "/maps/explicit.map");
    CHECK(geometry.lastDocument->revision == 7u);

    const auto missingIdentity = service.handle(
      "acceptance.assertions.evaluate",
      {{"document", QJsonObject{}},
       {"assertion",
        QJsonObject{
          {"id", "clear"},
          {"type", "clearSightline"},
          {"configuration",
           QJsonObject{
             {"origin", QJsonArray{0.0, 0.0, 0.0}},
             {"target", QJsonArray{0.0, 64.0, 0.0}},
           }}}}});
    REQUIRE(missingIdentity.is_error());
    CHECK(
      std::get<AcceptanceAutomationError>(missingIdentity.error()).code
      == AcceptanceAutomationErrorCode::InvalidParameters);
  }
}

} // namespace tb::ui
