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
  mutable std::vector<AcceptanceStructuralRay> rays;

  Result<std::vector<AcceptanceGeometryHit>, AcceptanceGeometryError> cast(
    const AcceptanceStructuralRay& ray) const override
  {
    rays.push_back(ray);
    return hits ? hits(ray) : std::vector<AcceptanceGeometryHit>{};
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
}

} // namespace tb::ui
