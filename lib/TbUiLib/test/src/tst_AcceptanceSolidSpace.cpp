/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/WorldNode.h"
#include "ui/AcceptanceSolidSpace.h"
#include "ui/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

void addCuboid(mdl::Map& map, const vm::bbox3d& bounds)
{
  const auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto brush = builder.createCuboid(bounds, "acceptance/test") | kdl::value();
  auto* node = new mdl::BrushNode{std::move(brush)};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {node}}});
}

class PredicateQuery : public AcceptanceSolidSpaceQuery
{
public:
  explicit PredicateQuery(std::function<bool(const vm::vec3d&)> predicate)
    : m_predicate{std::move(predicate)}
  {
  }

  Result<bool, AcceptanceSolidSpaceError> isSolid(const vm::vec3d& point) const override
  {
    return m_predicate(point);
  }

private:
  std::function<bool(const vm::vec3d&)> m_predicate;
};

AcceptanceSolidSpaceComparisonOptions options()
{
  return {{{0.0, 0.0, 0.0}, {12.0, 4.0, 4.0}}, 4.0, 100u, {}};
}

} // namespace

TEST_CASE("AcceptanceSolidSpace")
{
  SECTION("compares occupancy independently of brush decomposition")
  {
    auto referenceFixture = mdl::MapFixture{};
    auto& reference = referenceFixture.create();
    addCuboid(reference, {{0.0, 0.0, 0.0}, {8.0, 4.0, 4.0}});

    auto candidateFixture = mdl::MapFixture{};
    auto& candidate = candidateFixture.create();
    addCuboid(candidate, {{0.0, 0.0, 0.0}, {4.0, 4.0, 4.0}});
    addCuboid(candidate, {{4.0, 0.0, 0.0}, {8.0, 4.0, 4.0}});
    addCuboid(candidate, {{8.0, 0.0, 0.0}, {12.0, 4.0, 4.0}});

    const auto result = AcceptanceSolidSpaceComparison{}.compare(
      AcceptanceMapSolidSpaceQuery{reference},
      AcceptanceMapSolidSpaceQuery{candidate},
      options());

    REQUIRE(result.is_success());
    const auto& report = result.value();
    CHECK(report.status == AcceptanceSolidSpaceComparisonStatus::Complete);
    CHECK(report.totalCells == 3u);
    CHECK(report.sampledCells == 3u);
    CHECK(report.newlySolid.cellCount == 1u);
    REQUIRE(report.newlySolid.bounds);
    CHECK((report.newlySolid.bounds->min == vm::vec3d{8.0, 0.0, 0.0}));
    CHECK((report.newlySolid.bounds->max == vm::vec3d{12.0, 4.0, 4.0}));
    REQUIRE(report.newlySolid.cells.size() == 1u);
    CHECK((report.newlySolid.cells.front().min == vm::vec3d{8.0, 0.0, 0.0}));
    CHECK(report.newlyEmpty.cellCount == 0u);
    CHECK_FALSE(report.newlyEmpty.bounds);
  }

  SECTION("reports candidate removals and combines their spatial bounds")
  {
    const auto reference = PredicateQuery{
      [](const vm::vec3d& point) { return point.x() <= 2.0 || point.x() >= 10.0; }};
    const auto candidate = PredicateQuery{[](const vm::vec3d&) { return false; }};

    const auto result =
      AcceptanceSolidSpaceComparison{}.compare(reference, candidate, options());

    REQUIRE(result.is_success());
    CHECK(result.value().newlySolid.cellCount == 0u);
    CHECK(result.value().newlyEmpty.cellCount == 2u);
    REQUIRE(result.value().newlyEmpty.bounds);
    CHECK((result.value().newlyEmpty.bounds->min == vm::vec3d{0.0, 0.0, 0.0}));
    CHECK((result.value().newlyEmpty.bounds->max == vm::vec3d{12.0, 4.0, 4.0}));
  }

  SECTION("uses a partial high-edge cell and keeps its reported bounds within the region")
  {
    auto partial = options();
    partial.bounds = {{0.0, 0.0, 0.0}, {10.0, 4.0, 4.0}};
    const auto empty = PredicateQuery{[](const vm::vec3d&) { return false; }};
    const auto solid = PredicateQuery{[](const vm::vec3d&) { return true; }};

    const auto result = AcceptanceSolidSpaceComparison{}.compare(empty, solid, partial);

    REQUIRE(result.is_success());
    CHECK(result.value().totalCells == 3u);
    REQUIRE(result.value().newlySolid.bounds);
    CHECK((result.value().newlySolid.bounds->max == vm::vec3d{10.0, 4.0, 4.0}));
  }

  SECTION("validates the configured sample cap before querying geometry")
  {
    auto constrained = options();
    constrained.maxSamples = 2u;
    auto queried = false;
    const auto query = PredicateQuery{[&](const vm::vec3d&) {
      queried = true;
      return false;
    }};

    const auto result =
      AcceptanceSolidSpaceComparison{}.compare(query, query, constrained);

    CHECK(result.is_error());
    CHECK_FALSE(queried);
  }

  SECTION("returns a partial, explicitly cancelled report")
  {
    auto calls = 0u;
    auto cancelled = options();
    cancelled.cancelled = [&] { return calls++ >= 1u; };
    const auto empty = PredicateQuery{[](const vm::vec3d&) { return false; }};

    const auto result = AcceptanceSolidSpaceComparison{}.compare(empty, empty, cancelled);

    REQUIRE(result.is_success());
    CHECK(result.value().status == AcceptanceSolidSpaceComparisonStatus::Cancelled);
    CHECK(result.value().sampledCells == 1u);
    CHECK(result.value().totalCells == 3u);
  }
}

} // namespace tb::ui
