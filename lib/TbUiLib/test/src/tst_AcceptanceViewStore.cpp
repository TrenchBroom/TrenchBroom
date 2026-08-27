/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QFile>
#include <QTemporaryDir>

#include "ui/AcceptanceViewStore.h"
#include "ui/CatchConfig.h"

#include <variant>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceProject projectWithViews()
{
  auto project = AcceptanceProject{};
  project.views = {
    {"source", "Source", {}, {1600, 900}, "textured", {}},
    {"target", "Target", {}, {1600, 900}, "textured", {}},
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

} // namespace

TEST_CASE("AcceptanceViewStore")
{
  SECTION("creates atomically and rejects a stale optimistic revision")
  {
    auto directory = QTemporaryDir{};
    REQUIRE(directory.isValid());
    auto store = AcceptanceViewStore{
      std::filesystem::path{directory.path().toStdString()} / "qa" / "acceptance.json"};

    const auto initial = store.load();
    REQUIRE(initial.is_success());
    CHECK(initial.value().revision == 0u);

    const auto written = store.replace(projectWithViews(), initial.value().revision);
    REQUIRE(written.is_success());
    CHECK(written.value().revision == 1u);

    const auto stale =
      store.update(0u, [](AcceptanceProject&) { return AcceptanceValidationResult{}; });
    REQUIRE(stale.is_error());
    CHECK(
      std::get<AcceptanceError>(stale.error()).code
      == AcceptanceErrorCode::RevisionConflict);

    const auto loaded = store.load();
    REQUIRE(loaded.is_success());
    CHECK(loaded.value().revision == 1u);
    CHECK(loaded.value().suites.front().suiteId == "suite");
  }

  SECTION("does not commit an invalid update")
  {
    auto directory = QTemporaryDir{};
    REQUIRE(directory.isValid());
    auto store = AcceptanceViewStore{
      std::filesystem::path{directory.path().toStdString()} / "acceptance.json"};
    REQUIRE(store.replace(projectWithViews(), 0u).is_success());

    const auto invalid = store.update(1u, [](AcceptanceProject& project) {
      project.suites.front().comparisonIds = {"missing"};
      return AcceptanceValidationResult{};
    });
    CHECK(invalid.is_error());
    const auto loaded = store.load();
    REQUIRE(loaded.is_success());
    CHECK(loaded.value().revision == 1u);
    CHECK(
      loaded.value().suites.front().comparisonIds
      == std::vector<std::string>{"comparison"});
  }

  SECTION("reports corrupt files and failed atomic writes")
  {
    auto directory = QTemporaryDir{};
    REQUIRE(directory.isValid());
    const auto base = std::filesystem::path{directory.path().toStdString()};
    const auto corruptPath = base / "corrupt.json";
    auto corrupt = QFile{QString::fromStdString(corruptPath.string())};
    REQUIRE(corrupt.open(QIODevice::WriteOnly));
    REQUIRE(corrupt.write("not json") == 8);
    corrupt.close();
    CHECK(AcceptanceViewStore{corruptPath}.load().is_error());

    const auto blockerPath = base / "blocker";
    auto blocker = QFile{QString::fromStdString(blockerPath.string())};
    REQUIRE(blocker.open(QIODevice::WriteOnly));
    blocker.close();
    const auto failed = AcceptanceViewStore{blockerPath / "acceptance.json"}.replace(
      projectWithViews(), 0u);
    CHECK(failed.is_error());
  }
}

} // namespace tb::ui
