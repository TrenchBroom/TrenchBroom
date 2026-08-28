/*
 Copyright (C) 2026

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 */

#include <QJsonArray>

#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/WorldNode.h"
#include "ui/AcceptanceMapGeometry.h"
#include "ui/AutomationDocumentRegistry.h"
#include "ui/CatchConfig.h"

#include <variant>

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

class Resolver : public AcceptanceMapResolver
{
public:
  mdl::Map* map = nullptr;
  std::vector<std::string> documentIds;

  Result<mdl::Map*, AcceptanceGeometryError> resolve(
    const AcceptanceCaptureDocumentIdentity& document) override
  {
    documentIds.push_back(document.documentId);
    if (document.documentId != "exact" || map == nullptr)
      return AcceptanceGeometryError{"Unknown acceptance document"};
    return map;
  }
};

} // namespace

TEST_CASE("AcceptanceMapGeometry")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();
  addCuboid(map, {{10.0, -2.0, -2.0}, {20.0, 2.0, 2.0}});
  auto query = AcceptanceMapGeometryQuery{map};

  SECTION("returns structural brush hits bounded by the requested ray range")
  {
    const auto hits = query.cast({{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, 30.0});
    REQUIRE(hits.is_success());
    REQUIRE_FALSE(hits.value().empty());
    CHECK(hits.value().front().distance == 10.0);

    const auto shortRay = query.cast({{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, 9.0});
    REQUIRE(shortRay.is_success());
    CHECK(shortRay.value().empty());
  }

  SECTION("reports a start-inside-solid hit for structural assertions")
  {
    const auto hits = query.cast({{15.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, 30.0});
    REQUIRE(hits.is_success());
    REQUIRE_FALSE(hits.value().empty());
    CHECK(hits.value().front().distance == 0.0);
  }

  SECTION("uses exact brush-volume intersection for player clearance probes")
  {
    const auto clear = query.intersects({{-5.0, -5.0, -5.0}, {5.0, 5.0, 5.0}});
    REQUIRE(clear.is_success());
    CHECK_FALSE(clear.value());

    const auto blocked = query.intersects({{15.0, -1.0, -1.0}, {16.0, 1.0, 1.0}});
    REQUIRE(blocked.is_success());
    CHECK(blocked.value());
  }

  SECTION("detects player body, headroom, and a narrow opening")
  {
    const auto player = [](const vm::vec3d& start) {
      auto assertion = AcceptanceAssertion{};
      assertion.type = AcceptanceAssertionType::PlayerClearance;
      assertion.configuration = {
        {"start", QJsonArray{start.x(), start.y(), start.z()}},
        {"radius", 4.0},
        {"height", 10.0}};
      return assertion;
    };
    const auto evaluator = AcceptanceAssertionEvaluator{query};

    const auto clear = evaluator.evaluate(player({0.0, 0.0, 0.0}));
    REQUIRE(clear.is_success());
    CHECK(clear.value().passed);

    const auto body = evaluator.evaluate(player({15.0, 0.0, 0.0}));
    REQUIRE(body.is_success());
    CHECK_FALSE(body.value().passed);

    addCuboid(map, {{-5.0, -5.0, 8.0}, {5.0, 5.0, 12.0}});
    const auto headroom = evaluator.evaluate(player({0.0, 0.0, 0.0}));
    REQUIRE(headroom.is_success());
    CHECK_FALSE(headroom.value().passed);

    addCuboid(map, {{-5.0, -5.0, -30.0}, {5.0, -3.0, -10.0}});
    addCuboid(map, {{-5.0, 3.0, -30.0}, {5.0, 5.0, -10.0}});
    const auto narrowOpening = evaluator.evaluate(player({0.0, 0.0, -20.0}));
    REQUIRE(narrowOpening.is_success());
    CHECK_FALSE(narrowOpening.value().passed);
  }

  SECTION("requires an exact document identity through the injected resolver")
  {
    auto resolver = Resolver{};
    resolver.map = &map;
    auto provider = AcceptanceMapGeometryProvider{resolver};

    const auto exact = provider.geometryFor({"map.map", "exact", 7u});
    REQUIRE(exact.is_success());
    CHECK(resolver.documentIds == std::vector<std::string>{"exact"});
    const auto hits = exact.value()->cast({{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, 30.0});
    REQUIRE(hits.is_success());
    CHECK(hits.value().front().distance == 10.0);

    const auto missing = provider.geometryFor({"map.map", "", 7u});
    REQUIRE(missing.is_error());
    CHECK(resolver.documentIds == std::vector<std::string>{"exact"});
  }

  SECTION("consults an injected hidden-document map only after live lookup")
  {
    auto documents = AutomationDocumentRegistry{[] { return QString{"token"}; }};
    auto hiddenIds = std::vector<std::string>{};
    auto resolver = AcceptanceAutomationMapResolver{
      documents, [&](const std::string_view id) -> mdl::Map* {
        hiddenIds.emplace_back(id);
        return id == "hidden" ? &map : nullptr;
      }};

    const auto hidden = resolver.resolve({"hidden.map", "hidden", 1u});
    REQUIRE(hidden.is_success());
    CHECK(hidden.value() == &map);
    CHECK(hiddenIds == std::vector<std::string>{"hidden"});

    const auto missing = resolver.resolve({"hidden.map", "", 1u});
    REQUIRE(missing.is_error());
    CHECK(hiddenIds == std::vector<std::string>{"hidden"});
  }
}

} // namespace tb::ui
