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

#include "mdl/CatchConfig.h"
#include "mdl/EntityDefinitionGroup.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityDefinitionUtils.h"

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
using Catch::Matchers::RangeEquals;
using Catch::Matchers::UnorderedRangeEquals;

namespace
{

const std::string& definitionName(const EntityDefinition* definition)
{
  return definition->name;
}

} // namespace

TEST_CASE("EntityDefinitionManager")
{
  auto manager = EntityDefinitionManager{};
  manager.setDefinitions({
    {"beta_two", {}, "", {}, PointEntityDefinition{}},
    {"alpha_one", {}, "", {}, PointEntityDefinition{}},
    {"alpha_three", {}, "", {}, PointEntityDefinition{}},
    {"brushdef", {}, "", {}},
    {"beta_one", {}, "", {}, PointEntityDefinition{}},
  });

  SECTION("setDefinitions")
  {
    SECTION("updates indices")
    {
      CHECK_THAT(
        manager.definitions() | std::views::transform([](const auto& definition) {
          return definition.index;
        }),
        RangeEquals({1, 2, 3, 4, 5}));
    }

    SECTION("groups contain expected names and definitions")
    {
      CHECK_THAT(
        manager.groups()
          | std::views::transform([](const auto& group) { return group.name; }),
        RangeEquals({"alpha", "beta", "brushdef"}));

      const auto findGroup = [&](const auto& name) {
        const auto iGroup = std::ranges::find_if(
          manager.groups(), [&](const auto& group) { return group.name == name; });

        REQUIRE(iGroup != manager.groups().end());
        return *iGroup;
      };

      const auto& alphaGroup = findGroup("alpha");
      const auto& betaGroup = findGroup("beta");
      const auto& brushGroup = findGroup("brushdef");

      CHECK_THAT(
        alphaGroup.definitions | std::views::transform(definitionName),
        UnorderedRangeEquals({"alpha_one", "alpha_three"}));

      CHECK_THAT(
        betaGroup.definitions | std::views::transform(definitionName),
        UnorderedRangeEquals({"beta_one", "beta_two"}));

      CHECK_THAT(
        brushGroup.definitions | std::views::transform(definitionName),
        UnorderedRangeEquals({"brushdef"}));
    }

    SECTION("sorts groups alphabetically by display name")
    {
      CHECK_THAT(
        manager.groups()
          | std::views::transform([](const auto& group) { return displayName(group); }),
        RangeEquals({"Alpha", "Beta", "Brushdef"}));
    }
  }

  SECTION("clear")
  {
    manager.clear();

    CHECK(manager.definitions().empty());
    CHECK(manager.groups().empty());
    CHECK(manager.definition("alpha_one") == nullptr);
  }

  SECTION("definition")
  {
    CHECK(manager.definition("alpha_one") != nullptr);
    CHECK(manager.definition("missing") == nullptr);
  }

  SECTION("definitions")
  {
    SECTION("filters by type and sorts by name")
    {
      const auto pointDefinitions =
        manager.definitions(EntityDefinitionType::Point, EntityDefinitionSortOrder::Name);
      CHECK_THAT(
        pointDefinitions | std::views ::transform(definitionName),
        RangeEquals({"alpha_one", "alpha_three", "beta_one", "beta_two"}));

      const auto brushDefinitions =
        manager.definitions(EntityDefinitionType::Brush, EntityDefinitionSortOrder::Name);
      CHECK_THAT(
        brushDefinitions | std::views ::transform(definitionName),
        RangeEquals({"brushdef"}));
    }

    SECTION("sorts by usage")
    {
      manager.definition("alpha_one")->incUsageCount();

      manager.definition("alpha_three")->incUsageCount();
      manager.definition("alpha_three")->incUsageCount();

      manager.definition("beta_two")->incUsageCount();
      manager.definition("beta_two")->incUsageCount();
      manager.definition("beta_two")->incUsageCount();

      const auto pointDefinitions = manager.definitions(
        EntityDefinitionType::Point, EntityDefinitionSortOrder::Usage);

      CHECK_THAT(
        pointDefinitions | std::views::transform(definitionName),
        RangeEquals({"beta_one", "alpha_one", "alpha_three", "beta_two"}));
    }
  }
}

} // namespace tb::mdl
