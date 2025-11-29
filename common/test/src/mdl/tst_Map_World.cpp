/*
 Copyright (C) 2025 Kristian Duske

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

#include "MapFixture.h"
#include "fs/TestEnvironment.h"
#include "io/SystemPaths.h"
#include "mdl/Map.h"
#include "mdl/Map_World.h"
#include "mdl/WorldNode.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace tb::mdl
{
TEST_CASE("Map_World")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  SECTION("softMapBounds")
  {
    SECTION("No world node")
    {
      CHECK(softMapBounds(map) == SoftMapBounds{SoftMapBoundsType::Game, std::nullopt});
    }

    SECTION("World node without soft map bounds key")
    {
      fixture.create(QuakeFixtureConfig);

      CHECK(
        softMapBounds(map) == SoftMapBounds{SoftMapBoundsType::Game, vm::bbox3d{4096.0}});
    }

    SECTION("World node with soft map bounds key")
    {
      fixture.create(QuakeFixtureConfig);

      {
        auto* worldNode = map.world();
        auto world = worldNode->entity();
        world.addOrUpdateProperty(
          EntityPropertyKeys::SoftMapBounds, "-2048 -2048 -2048 2048 2048 2048");
        worldNode->setEntity(std::move(world));
      }

      CHECK(
        softMapBounds(map) == SoftMapBounds{SoftMapBoundsType::Map, vm::bbox3d{2048.0}});
    }
  }

  SECTION("setSoftMapBounds")
  {
    using T = std::tuple<SoftMapBounds, std::optional<std::string>>;

    const auto [softBounds, expectedPropertyValue] = GENERATE(values<T>({
      {SoftMapBounds{SoftMapBoundsType::Game, std::nullopt}, std::nullopt},
      {SoftMapBounds{SoftMapBoundsType::Game, vm::bbox3d{4096.0}}, std::nullopt},
      {SoftMapBounds{SoftMapBoundsType::Map, vm::bbox3d{2048.0}},
       "-2048 -2048 -2048 2048 2048 2048"},
      {SoftMapBounds{SoftMapBoundsType::Map, vm::bbox3d{1024.0}},
       "-1024 -1024 -1024 1024 1024 1024"},
    }));

    fixture.create(QuakeFixtureConfig);

    setSoftMapBounds(map, softBounds);

    auto* worldNode = map.world();
    const auto& world = worldNode->entity();

    REQUIRE(
      world.hasProperty(EntityPropertyKeys::SoftMapBounds)
      == expectedPropertyValue.has_value());

    if (expectedPropertyValue)
    {
      CHECK(*world.property(EntityPropertyKeys::SoftMapBounds) == *expectedPropertyValue);
    }
  }

  SECTION("externalSearchPaths")
  {
    SECTION("No world node")
    {
      CHECK(externalSearchPaths(map) == std::vector{io::SystemPaths::appDirectory()});
    }


    SECTION("With node")
    {

      SECTION("Map is transient")
      {
        auto fixtureConfig = MapFixtureConfig{};
        fixtureConfig.gamePath = ".";
        fixture.create(fixtureConfig);

        REQUIRE(!map.persistent());

        CHECK(
          externalSearchPaths(map)
          == std::vector{
            std::filesystem::path{"."}, // game path
            io::SystemPaths::appDirectory(),
          });
      }

      SECTION("Map is persistent")
      {
        auto env = fs::TestEnvironment{};

        const auto filename = "test.map";
        env.createFile(filename, R"(// Game: Test
// Format: Valve
// entity 0
{
"classname" "worldspawn"
}
)");

        const auto path = env.dir() / filename;

        auto fixtureConfig = MapFixtureConfig{};
        fixtureConfig.gameConfig.fileFormats = {{"Valve", ""}};
        fixtureConfig.gamePath = ".";
        fixture.load(path, fixtureConfig);

        CHECK(
          externalSearchPaths(map)
          == std::vector{
            path.parent_path(),         // map path
            std::filesystem::path{"."}, // game path
            io::SystemPaths::appDirectory(),
          });
      }
    }
  }

  SECTION("enabledMods")
  {
    SECTION("When passing an entity")
    {
      auto entity = Entity{};

      CHECK(enabledMods(entity).empty());

      entity.addOrUpdateProperty(EntityPropertyKeys::Mods, "mod1;mod2;mod3");
      CHECK(enabledMods(entity) == std::vector<std::string>{"mod1", "mod2", "mod3"});
    }

    SECTION("When passing a map")
    {
      SECTION("No world node")
      {
        CHECK(enabledMods(map).empty());
      }

      SECTION("With world node")
      {
        fixture.create();

        CHECK(enabledMods(map).empty());

        {
          auto* worldNode = map.world();
          auto world = worldNode->entity();
          world.addOrUpdateProperty(EntityPropertyKeys::Mods, "mod1;mod2;mod3");
          worldNode->setEntity(std::move(world));
        }

        CHECK(enabledMods(map) == std::vector<std::string>{"mod1", "mod2", "mod3"});
      }
    }
  }

  SECTION("setEnabledMods")
  {
    fixture.create();

    const auto* worldNode = map.world();
    const auto& world = worldNode->entity();

    REQUIRE(!world.hasProperty(EntityPropertyKeys::Mods));
    REQUIRE(enabledMods(map).empty());

    SECTION("Setting mods on a map without mods property")
    {
      SECTION("Setting empty mods")
      {
        setEnabledMods(map, {});

        CHECK(!world.hasProperty(EntityPropertyKeys::Mods));
        CHECK(enabledMods(world).empty());
      }

      SECTION("Setting non-empty mods")
      {
        setEnabledMods(map, {"mod1", "mod2", "mod3"});

        REQUIRE(world.hasProperty(EntityPropertyKeys::Mods));
        CHECK(*world.property(EntityPropertyKeys::Mods) == "mod1;mod2;mod3");
      }
    }

    SECTION("Setting mods on a map with mods property")
    {
      setEnabledMods(map, {"mod1", "mod2", "mod3"});

      REQUIRE(world.hasProperty(EntityPropertyKeys::Mods));
      REQUIRE(*world.property(EntityPropertyKeys::Mods) == "mod1;mod2;mod3");

      SECTION("Setting empty mods")
      {
        setEnabledMods(map, {});

        CHECK(!world.hasProperty(EntityPropertyKeys::Mods));
        CHECK(enabledMods(world).empty());
      }

      SECTION("Setting non-empty mods")
      {
        setEnabledMods(map, {"mod1", "mod3", "mod4"});

        REQUIRE(world.hasProperty(EntityPropertyKeys::Mods));
        CHECK(*world.property(EntityPropertyKeys::Mods) == "mod1;mod3;mod4");
      }
    }
  }

  SECTION("defaultMod")
  {
    fixture.create(QuakeFixtureConfig);

    CHECK(defaultMod(map) == "id1");
  }
}

} // namespace tb::mdl
