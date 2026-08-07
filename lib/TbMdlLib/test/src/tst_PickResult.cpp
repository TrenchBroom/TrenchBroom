/*
 Copyright (C) 2026 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mdl/BrushNode.h"
#include "mdl/Hit.h"
#include "mdl/HitFilter.h"
#include "mdl/PickResult.h"

#include "vm/util.h"
#include "vm/vec.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
namespace
{

const auto TestHitType = HitType::freeType();
const auto OtherHitType = HitType::freeType();

/**
 * Creates a hit that carries `id` as its target, so that the hits in a pick result can
 * be told apart in assertions.
 */
Hit makeHit(
  const HitType::Type type, const double distance, const int id, const double error = 0.0)
{
  return Hit{type, distance, vm::vec3d{0, 0, distance}, id, error};
}

std::vector<int> ids(const std::vector<Hit>& hits)
{
  auto result = std::vector<int>{};
  for (const auto& hit : hits)
  {
    result.push_back(hit.target<int>());
  }
  return result;
}

} // namespace

TEST_CASE("PickResult")
{
  using namespace Catch::Matchers;

  SECTION("empty")
  {
    auto pickResult = PickResult{};
    CHECK(pickResult.empty());

    pickResult.addHit(makeHit(TestHitType, 1.0, 1));
    CHECK(!pickResult.empty());
  }

  SECTION("size")
  {
    auto pickResult = PickResult{};
    CHECK(pickResult.size() == 0u);

    pickResult.addHit(makeHit(TestHitType, 1.0, 1));
    CHECK(pickResult.size() == 1u);

    pickResult.addHit(makeHit(TestHitType, 1.0, 2));
    CHECK(pickResult.size() == 2u);
  }

  SECTION("addHit")
  {
    SECTION("sorts hits by distance")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 2.0, 2));
      pickResult.addHit(makeHit(TestHitType, 3.0, 3));
      pickResult.addHit(makeHit(TestHitType, 1.0, 1));

      CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{1, 2, 3}));
    }

    SECTION("keeps hits at the same distance in insertion order")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 1.0, 1));
      pickResult.addHit(makeHit(TestHitType, 1.0, 2));
      pickResult.addHit(makeHit(TestHitType, 1.0, 3));

      CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{1, 2, 3}));
    }
  }

  SECTION("byDistance")
  {
    SECTION("sorts hits by distance")
    {
      auto pickResult = PickResult::byDistance();
      pickResult.addHit(makeHit(TestHitType, 2.0, 2));
      pickResult.addHit(makeHit(TestHitType, 1.0, 1));

      CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{1, 2}));
    }

    SECTION("sorts brush hits before other hits at the same distance")
    {
      const auto brushHitFirst = GENERATE(true, false);

      auto pickResult = PickResult::byDistance();
      if (brushHitFirst)
      {
        pickResult.addHit(makeHit(BrushNode::BrushHitType, 1.0, 1));
        pickResult.addHit(makeHit(TestHitType, 1.0, 2));
      }
      else
      {
        pickResult.addHit(makeHit(TestHitType, 1.0, 2));
        pickResult.addHit(makeHit(BrushNode::BrushHitType, 1.0, 1));
      }

      CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{1, 2}));
    }

    SECTION("sorts by distance before type")
    {
      auto pickResult = PickResult::byDistance();
      pickResult.addHit(makeHit(BrushNode::BrushHitType, 2.0, 2));
      pickResult.addHit(makeHit(TestHitType, 1.0, 1));

      CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{1, 2}));
    }
  }

  SECTION("bySize")
  {
    // hits whose targets are not nodes all have size 0, so they are sorted by distance
    auto pickResult = PickResult::bySize(vm::axis::z);
    pickResult.addHit(makeHit(TestHitType, 2.0, 2));
    pickResult.addHit(makeHit(TestHitType, 1.0, 1));

    CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{1, 2}));
  }

  SECTION("all")
  {
    SECTION("without a filter")
    {
      auto pickResult = PickResult{};
      CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{}));

      pickResult.addHit(makeHit(TestHitType, 2.0, 2));
      pickResult.addHit(makeHit(OtherHitType, 1.0, 1));

      CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{1, 2}));
    }

    SECTION("with a filter")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 1.0, 1));
      pickResult.addHit(makeHit(OtherHitType, 2.0, 2));
      pickResult.addHit(makeHit(TestHitType, 3.0, 3));

      CHECK_THAT(
        ids(pickResult.all(HitFilters::type(TestHitType))),
        RangeEquals(std::vector<int>{1, 3}));
      CHECK_THAT(
        ids(pickResult.all(HitFilters::type(OtherHitType))),
        RangeEquals(std::vector<int>{2}));
      CHECK_THAT(
        ids(pickResult.all(HitFilters::none())), RangeEquals(std::vector<int>{}));
      CHECK_THAT(
        ids(pickResult.all(HitFilters::any())), RangeEquals(std::vector<int>{1, 2, 3}));
    }
  }

  SECTION("first")
  {
    SECTION("returns no hit if the pick result is empty")
    {
      const auto pickResult = PickResult{};
      CHECK(!pickResult.first(HitFilters::any()).isMatch());
    }

    SECTION("returns no hit if no hit matches the filter")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 1.0, 1));

      CHECK(!pickResult.first(HitFilters::none()).isMatch());
      CHECK(!pickResult.first(HitFilters::type(OtherHitType)).isMatch());
    }

    SECTION("returns the closest matching hit")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 2.0, 2));
      pickResult.addHit(makeHit(TestHitType, 1.0, 1));
      pickResult.addHit(makeHit(TestHitType, 3.0, 3));

      CHECK(pickResult.first(HitFilters::any()).target<int>() == 1);
    }

    SECTION("skips hits that do not match the filter")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(OtherHitType, 1.0, 1));
      pickResult.addHit(makeHit(OtherHitType, 2.0, 2));
      pickResult.addHit(makeHit(TestHitType, 3.0, 3));

      CHECK(pickResult.first(HitFilters::type(TestHitType)).target<int>() == 3);
    }

    SECTION("prefers the hit with the smallest error")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 1.0, 1, 1.0));
      pickResult.addHit(makeHit(TestHitType, 1.0, 2, 0.5));
      pickResult.addHit(makeHit(TestHitType, 1.0, 3, 2.0));

      CHECK(pickResult.first(HitFilters::any()).target<int>() == 2);
    }

    SECTION("prefers the closest hit if the errors are equal")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 1.0, 1, 0.5));
      pickResult.addHit(makeHit(TestHitType, 2.0, 2, 0.5));

      CHECK(pickResult.first(HitFilters::any()).target<int>() == 1);
    }

    SECTION("prefers the hit with the smallest error over the closer hit")
    {
      auto pickResult = PickResult{};
      pickResult.addHit(makeHit(TestHitType, 1.0, 1, 1.0));
      pickResult.addHit(makeHit(TestHitType, 2.0, 2, 0.5));

      CHECK(pickResult.first(HitFilters::any()).target<int>() == 2);
    }
  }

  SECTION("clear")
  {
    auto pickResult = PickResult{};
    pickResult.addHit(makeHit(TestHitType, 1.0, 1));
    pickResult.clear();

    CHECK(pickResult.empty());
    CHECK(pickResult.size() == 0u);
    CHECK_THAT(ids(pickResult.all()), RangeEquals(std::vector<int>{}));
  }
}

} // namespace tb::mdl
