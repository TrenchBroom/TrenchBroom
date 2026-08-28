/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>

#include "ui/AcceptanceAutomationService.h"
#include "ui/AcceptanceDivergencePolicy.h"
#include "ui/CatchConfig.h"

#include <fstream>
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
    "unrest-rebuild",
  }};
  project.suites = {{AcceptanceSchemaVersion, "suite", "Suite", {"comparison"}}};
  project.contexts = {
    {"unrest-rebuild", "Unrest rebuild", "source.map", "target.map", {}}};
  return project;
}

class Capture : public AcceptanceVirtualCapture, public AcceptanceDocumentSnapshotProvider
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

  Result<void, AcceptanceEvidenceError> snapshot(
    const AcceptanceCaptureDocumentIdentity& document,
    const std::filesystem::path& outputPath) override
  {
    auto stream = std::ofstream{outputPath, std::ios::binary};
    stream << document.documentId << ':' << document.revision;
    if (!stream)
      return AcceptanceEvidenceError{"Could not write fake snapshot"};
    return {};
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

class SolidSpaceQuery : public AcceptanceSolidSpaceQuery
{
public:
  explicit SolidSpaceQuery(const bool reference)
    : m_reference{reference}
  {
  }

  Result<bool, AcceptanceSolidSpaceError> isSolid(const vm::vec3d& point) const override
  {
    return m_reference ? point.x() < 1.0 : point.x() >= 1.0;
  }

private:
  bool m_reference;
};

class SolidSpace : public AcceptanceSolidSpaceProvider
{
public:
  std::vector<std::filesystem::path> paths;

  Result<AcceptanceSolidSpaceDocument, AcceptanceSolidSpaceError> queryFor(
    const std::filesystem::path& path) override
  {
    paths.push_back(path);
    const auto reference = path.filename() == "source.map";
    return AcceptanceSolidSpaceDocument{
      std::make_shared<SolidSpaceQuery>(reference),
      reference ? "source-document" : "target-document",
      reference ? 3u : 7u};
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
  auto solidSpace = SolidSpace{};
  auto service =
    AcceptanceAutomationService{store, capture, geometry, solidSpace, capture};
  const auto project = makeProject();

  SECTION("routes optimistic CRUD and reports the explicit store path")
  {
    const auto context = service.handle(
      "acceptance.contexts.create",
      {{"expectedRevision", 0}, {"context", item(project, "contexts", 0)}});
    REQUIRE(context.is_success());
    CHECK(context.value().value("revision") == 1);
    CHECK(
      context.value().value("projectPath")
      == QString::fromStdString(projectPath.generic_string()));

    const auto source = service.handle(
      "acceptance.views.create",
      {{"expectedRevision", 1}, {"view", item(project, "views", 0)}});
    REQUIRE(source.is_success());
    CHECK(source.value().value("revision") == 2);

    REQUIRE(service
              .handle(
                "acceptance.views.create",
                {{"expectedRevision", 2}, {"view", item(project, "views", 1)}})
              .is_success());
    REQUIRE(
      service
        .handle(
          "acceptance.comparisons.create",
          {{"expectedRevision", 3}, {"comparison", item(project, "comparisons", 0)}})
        .is_success());
    REQUIRE(service
              .handle(
                "acceptance.suites.create",
                {{"expectedRevision", 4}, {"suite", item(project, "suites", 0)}})
              .is_success());

    const auto views = service.handle("acceptance.views.list", {});
    REQUIRE(views.is_success());
    CHECK(views.value().value("items").toArray().size() == 2);

    const auto contexts = service.handle("acceptance.contexts.list", {});
    REQUIRE(contexts.is_success());
    CHECK(contexts.value().value("items").toArray().size() == 1);

    auto updated = item(project, "contexts", 0);
    updated.insert("candidate", QJsonObject{{"documentPath", "updated-target.map"}});
    const auto update = service.handle(
      "acceptance.contexts.update",
      {{"id", "unrest-rebuild"}, {"expectedRevision", 5}, {"context", updated}});
    REQUIRE(update.is_success());
    CHECK(update.value().value("revision") == 6);
    const auto loaded = store.load();
    REQUIRE(loaded.is_success());
    CHECK(loaded.value().comparisons.front().target.path == "updated-target.map");

    const auto stale = service.handle(
      "acceptance.contexts.update",
      {{"id", "unrest-rebuild"}, {"expectedRevision", 5}, {"context", updated}});
    REQUIRE(stale.is_error());
    CHECK(
      std::get<AcceptanceAutomationError>(stale.error()).code
      == AcceptanceAutomationErrorCode::StoreFailed);

    const auto referencedContext = service.handle(
      "acceptance.contexts.delete", {{"id", "unrest-rebuild"}, {"expectedRevision", 6}});
    REQUIRE(referencedContext.is_error());

    REQUIRE(
      service
        .handle("acceptance.suites.delete", {{"id", "suite"}, {"expectedRevision", 6}})
        .is_success());
    REQUIRE(service
              .handle(
                "acceptance.comparisons.delete",
                {{"id", "comparison"}, {"expectedRevision", 7}})
              .is_success());
    REQUIRE(service
              .handle(
                "acceptance.contexts.delete",
                {{"id", "unrest-rebuild"}, {"expectedRevision", 8}})
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

    const auto evidenceDirectory =
      std::filesystem::path{directory.path().toStdString()} / "evidence";
    const auto evidence = service.handle(
      "acceptance.evidence.run",
      {{"suiteId", "suite"},
       {"outputDirectory", QString::fromStdString(evidenceDirectory.string())}});
    REQUIRE(evidence.is_success());
    CHECK(evidence.value().value("report").toObject().value("status") == "passed");
    CHECK(std::filesystem::is_regular_file(evidenceDirectory / "manifest.json"));
    CHECK(
      std::filesystem::is_regular_file(
        evidenceDirectory / "comparisons" / "comparison" / "reference.map"));
    CHECK(
      std::filesystem::is_regular_file(
        evidenceDirectory / "comparisons" / "comparison" / "candidate.png"));
    CHECK(evidence.value().value("manifestSha256").toString().size() == 64);
    auto manifestFile =
      QFile{QString::fromStdString((evidenceDirectory / "manifest.json").string())};
    REQUIRE(manifestFile.open(QIODevice::ReadOnly));
    const auto manifest = QJsonDocument::fromJson(manifestFile.readAll()).object();
    const auto captures = manifest.value("captures").toArray();
    REQUIRE(captures.size() == 1);
    const auto reference = captures[0].toObject().value("reference").toObject();
    CHECK(reference.value("camera").toObject().value("projection") == "perspective");
    CHECK(reference.value("size").toObject().value("width") == 800);
    CHECK(reference.value("renderMode") == "textured");
    CHECK(reference.value("overlays").toObject().value("brushEdges") == false);
    CHECK(reference.value("map").toObject().value("sha256").toString().size() == 64);
    CHECK(reference.value("color").toObject().value("sha256").toString().size() == 64);
    const auto reportComparison =
      manifest.value("report").toObject().value("comparisons").toArray()[0].toObject();
    CHECK(
      reportComparison.value("imageComparison").toObject().value("referencePath")
      == "comparisons/comparison/reference.png");
    CHECK(
      reportComparison.value("imageComparison").toObject().value("targetPath")
      == "comparisons/comparison/candidate.png");

    const auto cannotOverwrite = service.handle(
      "acceptance.evidence.run",
      {{"suiteId", "suite"},
       {"outputDirectory", QString::fromStdString(evidenceDirectory.string())}});
    REQUIRE(cannotOverwrite.is_error());
    CHECK(
      std::get<AcceptanceAutomationError>(cannotOverwrite.error()).code
      == AcceptanceAutomationErrorCode::EvidenceFailed);

    const auto unsupported = service.handle("acceptance.nope", {});
    REQUIRE(unsupported.is_error());
    CHECK(
      std::get<AcceptanceAutomationError>(unsupported.error()).code
      == AcceptanceAutomationErrorCode::MethodNotFound);
  }

  SECTION("compares context solid space and classifies an accepted reference repair")
  {
    REQUIRE(store.replace(project, 0u).is_success());
    auto policy = AcceptanceDivergencePolicy{};
    policy.rules = {{
      "export-repair",
      "Known exporter defect",
      AcceptanceDivergenceDomain::SolidSpace,
      AcceptanceDivergenceDirection::CandidateOnly,
      vm::bbox3d{{1.0, 0.0, 0.0}, {2.0, 1.0, 1.0}},
      AcceptanceDivergenceDisposition::AcceptedRepair,
      AcceptanceDivergenceProvenance::ExportDefect,
      "unrest-export-audit.md#repair",
      "The candidate restores solid geometry omitted by the exporter.",
    }};

    const auto compared = service.handle(
      "acceptance.geometry.compare",
      {{"contextId", "unrest-rebuild"},
       {"bounds",
        QJsonObject{
          {"min", QJsonArray{0.0, 0.0, 0.0}},
          {"max", QJsonArray{2.0, 1.0, 1.0}},
        }},
       {"cellSize", 1.0},
       {"includeCells", true},
       {"policy", acceptanceDivergencePolicyToJson(policy)}});

    REQUIRE(compared.is_success());
    REQUIRE(solidSpace.paths.size() == 2u);
    const auto comparison = compared.value().value("comparison").toObject();
    CHECK(comparison.value("coordinateSpace") == "reference");
    CHECK(comparison.value("occupancyModel") == "brushVolumesV1");
    CHECK(
      comparison.value("referenceDocument").toObject().value("id") == "source-document");
    CHECK(comparison.value("referenceDocument").toObject().value("revision") == 3);
    CHECK(
      comparison.value("candidateDocument").toObject().value("id") == "target-document");
    CHECK(comparison.value("candidateDocument").toObject().value("revision") == 7);
    CHECK(comparison.value("referenceOnly").toObject().value("cellCount") == 1);
    CHECK(comparison.value("candidateOnly").toObject().value("cellCount") == 1);
    CHECK(comparison.value("referenceOnly").toObject().value("regionCount") == 1);
    CHECK(
      comparison.value("candidateOnly")
        .toObject()
        .value("regions")
        .toArray()[0]
        .toObject()
        .value("cellCount")
      == 1);
    CHECK(
      comparison.value("referenceOnly").toObject().value("cells").toArray().size() == 1);
    const auto policyReport = comparison.value("policy").toObject();
    CHECK(
      policyReport.value("definition").toObject().value("rules").toArray().size() == 1);
    CHECK(policyReport.value("total") == 2);
    CHECK(policyReport.value("dispositions").toObject().value("acceptedRepair") == 1);
    CHECK(policyReport.value("dispositions").toObject().value("review") == 1);
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
