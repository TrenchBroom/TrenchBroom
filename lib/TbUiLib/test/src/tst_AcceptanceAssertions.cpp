/* Copyright (C) 2026 */
#include <QJsonArray>

#include "ui/AcceptanceAssertions.h"
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

class Query : public AcceptanceGeometryQuery
{
public:
  std::function<std::vector<AcceptanceGeometryHit>(const AcceptanceStructuralRay&)> hits;
  std::function<bool(const vm::bbox3d&)> volumeIntersects;
  mutable std::vector<AcceptanceStructuralRay> rays;
  mutable std::vector<vm::bbox3d> volumes;

  Result<std::vector<AcceptanceGeometryHit>, AcceptanceGeometryError> cast(
    const AcceptanceStructuralRay& ray) const override
  {
    rays.push_back(ray);
    return hits ? hits(ray) : std::vector<AcceptanceGeometryHit>{};
  }

  Result<bool, AcceptanceGeometryError> intersects(
    const vm::bbox3d& bounds) const override
  {
    volumes.push_back(bounds);
    return volumeIntersects ? volumeIntersects(bounds) : false;
  }
};

AcceptanceAssertion sightlineAssertion(const double minimumClearFraction = 1.0)
{
  auto assertion = AcceptanceAssertion{};
  assertion.type = AcceptanceAssertionType::ClearSightline;
  assertion.configuration = {
    {"origin", vector(0, 0, 0)},
    {"target", vector(10, 0, 0)},
    {"corridorWidth", 2.0},
    {"grid", 3},
    {"minimumClearFraction", minimumClearFraction}};
  return assertion;
}

AcceptanceAssertion playerClearanceAssertion(
  const vm::vec3d& start = {0.0, 0.0, 0.0},
  const std::optional<vm::vec3d>& end = std::nullopt)
{
  auto configuration = QJsonObject{
    {"start", vector(start.x(), start.y(), start.z())},
    {"radius", 16.0},
    {"height", 56.0},
    {"maxStep", 8.0}};
  if (end)
    configuration.insert("end", vector(end->x(), end->y(), end->z()));
  auto assertion = AcceptanceAssertion{};
  assertion.type = AcceptanceAssertionType::PlayerClearance;
  assertion.configuration = configuration;
  return assertion;
}

} // namespace

TEST_CASE("AcceptanceAssertions")
{
  SECTION("evaluates a sampled sightline corridor and reports partial occlusion")
  {
    auto query = Query{};
    query.hits = [](const auto& ray) {
      return ray.origin.y() > 0.5 ? std::vector<AcceptanceGeometryHit>{{2.0}}
                                  : std::vector<AcceptanceGeometryHit>{};
    };
    const auto result =
      AcceptanceAssertionEvaluator{query}.evaluate(sightlineAssertion());
    REQUIRE(result.is_success());
    CHECK_FALSE(result.value().passed);
    CHECK(result.value().totalRays == 9u);
    CHECK(result.value().clearRays == 6u);
  }

  SECTION("detects a start-inside-solid sightline deterministically")
  {
    auto query = Query{};
    query.hits = [](const auto&) { return std::vector<AcceptanceGeometryHit>{{0.0}}; };
    const auto result =
      AcceptanceAssertionEvaluator{query}.evaluate(sightlineAssertion());
    REQUIRE(result.is_error());
    CHECK(
      std::get<AcceptanceAssertionError>(result.error()).code
      == AcceptanceAssertionErrorCode::StartInsideSolid);
  }

  SECTION("transforms reference assertion coordinates for target-space geometry")
  {
    auto query = Query{};
    query.hits = [](const auto&) { return std::vector<AcceptanceGeometryHit>{}; };
    auto context = AcceptanceAssertionContext{};
    context.geometrySpace = AcceptanceAssertionSpace::Target;
    context.alignment.type = AcceptanceAlignmentType::Matrix;
    context.alignment.matrix = {1, 0, 0, 20, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    const auto result =
      AcceptanceAssertionEvaluator{query}.evaluate(sightlineAssertion(), context);
    REQUIRE(result.is_success());
    REQUIRE_FALSE(query.rays.empty());
    CHECK(query.rays.front().origin.x() == 20.0);
  }

  SECTION("approximates explicit-bounds visibility and measures opening clearance")
  {
    auto query = Query{};
    query.hits = [](const auto& ray) {
      return ray.origin.x() > 0.0 ? std::vector<AcceptanceGeometryHit>{{1.0}}
                                  : std::vector<AcceptanceGeometryHit>{};
    };
    auto visible = AcceptanceAssertion{};
    visible.type = AcceptanceAssertionType::BoundsVisible;
    visible.bounds = AcceptanceBounds{{10, -1, -1}, {12, 1, 1}};
    visible.configuration = {{"minimumCoverage", 1.0}};
    auto context = AcceptanceAssertionContext{};
    context.visibilityCamera = AcceptanceCamera{};
    const auto bounds = AcceptanceAssertionEvaluator{query}.evaluate(visible, context);
    REQUIRE(bounds.is_success());
    CHECK(bounds.value().passed);

    auto opening = AcceptanceAssertion{};
    opening.type = AcceptanceAssertionType::OpeningClearance;
    opening.configuration = {
      {"origin", vector(0, 0, 0)},
      {"direction", vector(1, 0, 0)},
      {"up", vector(0, 0, 1)},
      {"width", 6.0},
      {"height", 6.0},
      {"depth", 10.0},
      {"grid", 3},
      {"minimumWidth", 6.0},
      {"minimumHeight", 6.0}};
    query.hits = [](const auto&) { return std::vector<AcceptanceGeometryHit>{}; };
    const auto clearance = AcceptanceAssertionEvaluator{query}.evaluate(opening);
    REQUIRE(clearance.is_success());
    CHECK(clearance.value().passed);
    CHECK(clearance.value().measuredWidth == 6.0);
  }

  SECTION("tests a player-sized upright volume at a point and along a segment")
  {
    auto query = Query{};
    const auto clear =
      AcceptanceAssertionEvaluator{query}.evaluate(playerClearanceAssertion());
    REQUIRE(clear.is_success());
    CHECK(clear.value().passed);
    CHECK(clear.value().totalRays == 1u);
    CHECK(clear.value().measuredWidth == 32.0);
    CHECK(clear.value().measuredHeight == 56.0);

    query.volumeIntersects = [](const auto&) { return true; };
    const auto blocked =
      AcceptanceAssertionEvaluator{query}.evaluate(playerClearanceAssertion());
    REQUIRE(blocked.is_success());
    CHECK_FALSE(blocked.value().passed);
    CHECK(blocked.value().clearRays == 0u);

    query.volumeIntersects = [](const auto& bounds) { return bounds.max.z() > 48.0; };
    const auto headroom =
      AcceptanceAssertionEvaluator{query}.evaluate(playerClearanceAssertion());
    REQUIRE(headroom.is_success());
    CHECK_FALSE(headroom.value().passed);

    query.volumeIntersects = [](const auto& bounds) {
      return bounds.min.y() < -12.0 && bounds.max.y() > 12.0;
    };
    const auto narrowOpening =
      AcceptanceAssertionEvaluator{query}.evaluate(playerClearanceAssertion());
    REQUIRE(narrowOpening.is_success());
    CHECK_FALSE(narrowOpening.value().passed);

    query.volumeIntersects = [](const auto&) { return false; };
    const auto segment = AcceptanceAssertionEvaluator{query}.evaluate(
      playerClearanceAssertion({0.0, 0.0, 0.0}, vm::vec3d{20.0, 0.0, 0.0}));
    REQUIRE(segment.is_success());
    CHECK(segment.value().passed);
    CHECK(segment.value().totalRays == 4u);
  }

  SECTION("uses a tolerance skin without changing the reported player dimensions")
  {
    auto query = Query{};
    auto context = AcceptanceAssertionContext{};
    context.tolerance = 0.5;
    const auto result =
      AcceptanceAssertionEvaluator{query}.evaluate(playerClearanceAssertion(), context);
    REQUIRE(result.is_success());
    REQUIRE(query.volumes.size() == 1u);
    CHECK(query.volumes.front().min == vm::vec3d{-15.5, -15.5, 0.5});
    CHECK(query.volumes.front().max == vm::vec3d{15.5, 15.5, 55.5});
    CHECK(result.value().measuredWidth == 32.0);
    CHECK(result.value().measuredHeight == 56.0);
  }

  SECTION("caps player-clearance segment samples inclusively at 4096")
  {
    auto query = Query{};
    const auto evaluate = [&](const double length) {
      auto assertion =
        playerClearanceAssertion({0.0, 0.0, 0.0}, vm::vec3d{length, 0.0, 0.0});
      assertion.configuration.insert("maxStep", 1.0);
      return AcceptanceAssertionEvaluator{query}.evaluate(assertion);
    };

    const auto belowLimit = evaluate(4094.0);
    REQUIRE(belowLimit.is_success());
    CHECK(belowLimit.value().totalRays == 4095u);

    const auto atLimit = evaluate(4095.0);
    REQUIRE(atLimit.is_success());
    CHECK(atLimit.value().totalRays == 4096u);

    const auto overLimit = evaluate(4096.0);
    REQUIRE(overLimit.is_error());
    CHECK(
      std::get<AcceptanceAssertionError>(overLimit.error()).code
      == AcceptanceAssertionErrorCode::InvalidConfiguration);
  }

  SECTION("rejects scaled or sheared player-clearance target alignments")
  {
    auto query = Query{};
    auto context = AcceptanceAssertionContext{};
    context.geometrySpace = AcceptanceAssertionSpace::Target;
    context.alignment.type = AcceptanceAlignmentType::Matrix;
    context.alignment.matrix = {2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1};

    const auto scaled =
      AcceptanceAssertionEvaluator{query}.evaluate(playerClearanceAssertion(), context);
    REQUIRE(scaled.is_error());
    CHECK(
      std::get<AcceptanceAssertionError>(scaled.error()).code
      == AcceptanceAssertionErrorCode::UnsupportedTransformation);

    context.alignment.matrix = {1, 0.5, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    const auto sheared =
      AcceptanceAssertionEvaluator{query}.evaluate(playerClearanceAssertion(), context);
    REQUIRE(sheared.is_error());
    CHECK(
      std::get<AcceptanceAssertionError>(sheared.error()).code
      == AcceptanceAssertionErrorCode::UnsupportedTransformation);
  }
}

} // namespace tb::ui
