/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "ui/AcceptanceComparisonContextResolver.h"
#include "ui/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

AcceptanceProject makeProject()
{
  auto project = AcceptanceProject{};
  auto alignment = AcceptanceAlignment{};
  alignment.type = AcceptanceAlignmentType::Independent;
  project.contexts = {{
    "unrest-rebuild",
    "Unrest rebuild",
    "maps/reference.map",
    "maps/candidate.map",
    alignment,
  }};
  return project;
}

} // namespace

TEST_CASE("AcceptanceComparisonContextResolver")
{
  SECTION("resolves a context to canonical paths and preserves its alignment")
  {
    const auto resolved = resolveAcceptanceComparisonContext(
      "/projects/qa/acceptance/acceptance.json", makeProject(), "unrest-rebuild");

    REQUIRE(resolved.is_success());
    CHECK(resolved.value().id == "unrest-rebuild");
    CHECK(
      resolved.value().referencePath
      == std::filesystem::path{"/projects/qa/acceptance/maps/reference.map"});
    CHECK(
      resolved.value().candidatePath
      == std::filesystem::path{"/projects/qa/acceptance/maps/candidate.map"});
    CHECK(resolved.value().alignment.type == AcceptanceAlignmentType::Independent);
  }

  SECTION("rejects an absent context")
  {
    const auto resolved = resolveAcceptanceComparisonContext(
      "/projects/qa/acceptance/acceptance.json", makeProject(), "missing");

    REQUIRE(resolved.is_error());
    CHECK(
      std::get<AcceptanceError>(resolved.error()).code
      == AcceptanceErrorCode::BrokenReference);
  }

  SECTION("rejects an invalid project context")
  {
    auto project = makeProject();
    project.contexts.front().candidatePath = "/absolute/candidate.map";

    const auto resolved = resolveAcceptanceComparisonContext(
      "/projects/qa/acceptance/acceptance.json", project, "unrest-rebuild");

    REQUIRE(resolved.is_error());
    CHECK(
      std::get<AcceptanceError>(resolved.error()).code
      == AcceptanceErrorCode::InvalidValue);
  }

  SECTION("rejects distinct portable paths that resolve to the same document")
  {
    auto project = makeProject();
    project.contexts.front().candidatePath = "maps/./reference.map";

    const auto resolved = resolveAcceptanceComparisonContext(
      "/projects/qa/acceptance/acceptance.json", project, "unrest-rebuild");

    REQUIRE(resolved.is_error());
    CHECK(
      std::get<AcceptanceError>(resolved.error()).code
      == AcceptanceErrorCode::InvalidValue);
  }
}

} // namespace tb::ui
