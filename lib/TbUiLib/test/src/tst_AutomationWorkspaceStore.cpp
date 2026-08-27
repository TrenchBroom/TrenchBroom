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

#include "fs/TestEnvironment.h"
#include "ui/AutomationWorkspaceManifest.h"
#include "ui/AutomationWorkspaceStore.h"
#include "ui/CatchConfig.h"

#include <filesystem>
#include <ranges>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

const auto fingerprint = QString{"sha256:"} + QString{64, 'a'};

AutomationWorkspaceManifest makeManifest(
  const QString& workspaceId, const std::filesystem::path& sourcePath)
{
  auto manifest = AutomationWorkspaceManifest{};
  manifest.workspaceId = workspaceId;
  manifest.name = "north vault";
  manifest.createdAt = QDateTime::fromString("2026-08-27T19:42:00Z", Qt::ISODate);
  manifest.source = {sourcePath, fingerprint, 50u};
  manifest.base = {"base.map", fingerprint};
  manifest.branch = {"snapshots/0/branch.map", fingerprint};
  manifest.nodeIdentities = {
    {1u, "world", {}, 0u, AutomationWorkspaceNodePath{}},
    {2u, "group", {0u, 4u}, 1u, AutomationWorkspaceNodePath{0u, 5u}},
  };
  return manifest;
}

bool hasDiagnostic(
  const AutomationWorkspaceStoreRecord& record,
  const AutomationWorkspaceStoreDiagnosticKind kind)
{
  return std::ranges::any_of(
    record.diagnostics, [&](const auto& diagnostic) { return diagnostic.kind == kind; });
}

} // namespace

TEST_CASE("AutomationWorkspaceManifest")
{
  auto environment = fs::TestEnvironment{};
  const auto sourcePath = environment.dir() / "source.map";

  SECTION("round trips deterministically and preserves unknown top-level fields")
  {
    auto json = makeManifest("11111111-1111-4111-8111-111111111111", sourcePath).toJson();
    json.insert("extension", QJsonObject{{"kept", true}});

    QString error;
    const auto parsed = AutomationWorkspaceManifest::fromJson(json, &error);
    REQUIRE(parsed);
    CHECK(parsed->toJson() == json);
    CHECK(parsed->createdAt.timeSpec() == Qt::UTC);
  }

  SECTION("round trips optional map loading metadata")
  {
    auto manifest = makeManifest("11111111-1111-4111-8111-111111111111", sourcePath);
    manifest.mapMetadata = {
      "Quake",
      mdl::MapFormat::Valve,
      vm::bbox3d{vm::vec3d{-8192.0, -4096.0, -1024.0}, vm::vec3d{8192.0, 4096.0, 2048.0}},
    };

    QString error;
    const auto parsed = AutomationWorkspaceManifest::fromJson(manifest.toJson(), &error);
    REQUIRE(parsed);
    REQUIRE(parsed->mapMetadata);
    CHECK(parsed->mapMetadata->gameName == "Quake");
    CHECK(parsed->mapMetadata->mapFormat == mdl::MapFormat::Valve);
    CHECK(parsed->mapMetadata->worldBounds == manifest.mapMetadata->worldBounds);
  }

  SECTION("rejects unsupported versions, malformed fingerprints, and escaping paths")
  {
    auto json = makeManifest("11111111-1111-4111-8111-111111111111", sourcePath).toJson();
    json.insert("schemaVersion", 2);
    CHECK_FALSE(AutomationWorkspaceManifest::fromJson(json));

    json = makeManifest("11111111-1111-4111-8111-111111111111", sourcePath).toJson();
    auto source = json.value("source").toObject();
    source.insert("fingerprintAtFork", "sha256:short");
    json.insert("source", source);
    CHECK_FALSE(AutomationWorkspaceManifest::fromJson(json));

    json = makeManifest("11111111-1111-4111-8111-111111111111", sourcePath).toJson();
    auto base = json.value("base").toObject();
    base.insert("path", "../base.map");
    json.insert("base", base);
    CHECK_FALSE(AutomationWorkspaceManifest::fromJson(json));

    json = makeManifest("11111111-1111-4111-8111-111111111111", sourcePath).toJson();
    json.insert(
      "mapMetadata",
      QJsonObject{
        {"gameName", "Quake"},
        {"mapFormat", "not-a-format"},
        {"worldBounds",
         QJsonObject{
           {"min", QJsonArray{0.0, 0.0, 0.0}}, {"max", QJsonArray{1.0, 1.0, 1.0}}}},
      });
    CHECK_FALSE(AutomationWorkspaceManifest::fromJson(json));
  }

  SECTION("rejects artifact symlinks that escape the workspace")
  {
    environment.createDirectory("workspace");
    environment.createDirectory("outside");
    environment.createSymLink("outside", "workspace/escape");

    QString error;
    CHECK_FALSE(
      AutomationWorkspaceManifest::resolveArtifactPath(
        environment.dir() / "workspace", "escape/branch.map", &error));
    CHECK(error.contains("escapes"));
  }
}

TEST_CASE("AutomationWorkspaceStore")
{
  auto environment = fs::TestEnvironment{};
  environment.createFile("source.map", "source");
  environment.createFile("base-input.map", "base");
  environment.createFile("branch-input.map", "branch zero");

  const auto root = environment.dir() / "workspaces";
  auto store = AutomationWorkspaceStore{root};
  const auto firstId = QString{"11111111-1111-4111-8111-111111111111"};
  const auto secondId = QString{"22222222-2222-4222-8222-222222222222"};

  SECTION("creates, checkpoints, and reads a complete generation")
  {
    const auto created = store.create(
      makeManifest(firstId, environment.dir() / "source.map"),
      environment.dir() / "base-input.map",
      environment.dir() / "branch-input.map");
    REQUIRE(created);
    CHECK(created.manifest->checkpointGeneration == 0u);

    environment.createFile("branch-one.map", "branch one");
    const auto checkpoint =
      store.publishCheckpoint(*created.manifest, environment.dir() / "branch-one.map");
    REQUIRE(checkpoint);
    CHECK(checkpoint.manifest->checkpointGeneration == 1u);
    CHECK(
      std::filesystem::is_regular_file(
        root / firstId.toStdString() / "snapshots" / "1" / "branch.map"));

    const auto record = store.read(store.workspaceDirectory(firstId));
    REQUIRE(record.valid());
    CHECK(record.manifest->checkpointGeneration == 1u);
    CHECK(
      record.manifest->branch.path == std::filesystem::path{"snapshots/1/branch.map"});
  }

  SECTION("updates lifecycle metadata without creating a new snapshot")
  {
    const auto created = store.create(
      makeManifest(firstId, environment.dir() / "source.map"),
      environment.dir() / "base-input.map",
      environment.dir() / "branch-input.map");
    REQUIRE(created);
    const auto manifestPath = root / firstId.toStdString() / "workspace.json";

    auto renamed = *created.manifest;
    renamed.name = "north vault repaired";
    const auto renamedResult = store.updateMetadata(renamed);
    REQUIRE(renamedResult);
    CHECK(renamedResult.manifest->checkpointGeneration == 0u);
    CHECK(renamedResult.manifest->name == "north vault repaired");
    CHECK_FALSE(
      std::filesystem::exists(root / firstId.toStdString() / "snapshots" / "1"));

    auto abandoned = *renamedResult.manifest;
    abandoned.state = AutomationWorkspaceLifecycleState::Abandoned;
    const auto abandonedResult = store.updateMetadata(abandoned);
    REQUIRE(abandonedResult);
    CHECK(abandonedResult.manifest->checkpointGeneration == 0u);
    CHECK(
      abandonedResult.manifest->state == AutomationWorkspaceLifecycleState::Abandoned);
    const auto record = store.read(store.workspaceDirectory(firstId));
    REQUIRE(record.valid());
    CHECK(record.manifest->name == "north vault repaired");
    CHECK(record.manifest->state == AutomationWorkspaceLifecycleState::Abandoned);

    const auto beforeRejectedUpdate = environment.loadFile(manifestPath);
    auto invalid = *abandonedResult.manifest;
    invalid.name.clear();
    CHECK_FALSE(store.updateMetadata(invalid));
    auto stale = *abandonedResult.manifest;
    ++stale.checkpointGeneration;
    CHECK_FALSE(store.updateMetadata(stale));
    auto reactivated = *abandonedResult.manifest;
    reactivated.state = AutomationWorkspaceLifecycleState::Active;
    CHECK_FALSE(store.updateMetadata(reactivated));
    CHECK(environment.loadFile(manifestPath) == beforeRejectedUpdate);
  }

  SECTION("does not permit metadata updates to change branch loading metadata")
  {
    auto manifest = makeManifest(firstId, environment.dir() / "source.map");
    manifest.mapMetadata = {
      "Quake",
      mdl::MapFormat::Valve,
      vm::bbox3d{vm::vec3d{-8192.0, -8192.0, -8192.0}, vm::vec3d{8192.0, 8192.0, 8192.0}},
    };
    const auto created = store.create(
      manifest,
      environment.dir() / "base-input.map",
      environment.dir() / "branch-input.map");
    REQUIRE(created);
    const auto manifestPath = root / firstId.toStdString() / "workspace.json";
    const auto before = environment.loadFile(manifestPath);

    auto changed = *created.manifest;
    changed.mapMetadata->gameName = "Quake 2";
    CHECK_FALSE(store.updateMetadata(changed));
    CHECK(environment.loadFile(manifestPath) == before);
  }

  SECTION("ignores temporary workspace directories and does not mutate while reading")
  {
    REQUIRE(store.create(
      makeManifest(firstId, environment.dir() / "source.map"),
      environment.dir() / "base-input.map",
      environment.dir() / "branch-input.map"));
    environment.createDirectory("workspaces/.interrupted.tmp");
    environment.createFile("workspaces/.interrupted.tmp/workspace.json", "{");

    const auto manifestPath = root / firstId.toStdString() / "workspace.json";
    const auto before = environment.loadFile(manifestPath);
    const auto records = store.scan();
    const auto after = environment.loadFile(manifestPath);

    REQUIRE(records.size() == 1u);
    CHECK(records.front().valid());
    CHECK(before == after);
  }

  SECTION(
    "uses the latest complete snapshot when root-manifest publication was interrupted")
  {
    const auto created = store.create(
      makeManifest(firstId, environment.dir() / "source.map"),
      environment.dir() / "base-input.map",
      environment.dir() / "branch-input.map");
    REQUIRE(created);
    const auto rootManifest = root / firstId.toStdString() / "workspace.json";
    const auto oldManifest = environment.loadFile(rootManifest);

    environment.createFile("branch-one.map", "branch one");
    REQUIRE(
      store.publishCheckpoint(*created.manifest, environment.dir() / "branch-one.map"));
    environment.createFile(rootManifest, oldManifest);

    const auto record = store.read(store.workspaceDirectory(firstId));
    REQUIRE(record.valid());
    CHECK(record.manifest->checkpointGeneration == 1u);
    CHECK(hasDiagnostic(record, AutomationWorkspaceStoreDiagnosticKind::SnapshotIgnored));
  }

  SECTION("reports corrupt records beside valid ones in deterministic directory order")
  {
    REQUIRE(store.create(
      makeManifest(secondId, environment.dir() / "source.map"),
      environment.dir() / "base-input.map",
      environment.dir() / "branch-input.map"));
    environment.createDirectory(root / firstId.toStdString());
    environment.createFile(root / firstId.toStdString() / "workspace.json", "not json");

    const auto records = store.scan();
    REQUIRE(records.size() == 2u);
    CHECK(records[0].directory.filename() == firstId.toStdString());
    CHECK_FALSE(records[0].valid());
    CHECK(
      hasDiagnostic(records[0], AutomationWorkspaceStoreDiagnosticKind::ManifestInvalid));
    CHECK(records[1].valid());
  }

  SECTION("reports a missing branch without changing files")
  {
    REQUIRE(store.create(
      makeManifest(firstId, environment.dir() / "source.map"),
      environment.dir() / "base-input.map",
      environment.dir() / "branch-input.map"));
    const auto branch = root / firstId.toStdString() / "snapshots" / "0" / "branch.map";
    REQUIRE(std::filesystem::remove(branch));

    const auto record = store.read(store.workspaceDirectory(firstId));
    CHECK_FALSE(record.valid());
    CHECK(hasDiagnostic(record, AutomationWorkspaceStoreDiagnosticKind::BranchMissing));
    CHECK_FALSE(std::filesystem::exists(branch));
  }
}

} // namespace tb::ui
