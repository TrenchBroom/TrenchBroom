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

#include "Observer.h"
#include "fs/TestEnvironment.h"
#include "mdl/BrushBuilder.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TestFactory.h"

#include "kd/k.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("Map_Persistence")
{
  SECTION("save")
  {
    auto env = fs::TestEnvironment{};

    const auto filename = "test.map";
    env.createFile(filename, R"(// Game: Test
// Format: Valve
// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"name" "entity1"
}
)");

    const auto path = env.dir() / filename;

    auto fixtureConfig = MapFixtureConfig{};
    fixtureConfig.gameInfo.gameConfig.fileFormats = {{"Valve", ""}};

    auto fixture = MapFixture{};
    auto& map = fixture.load(path, fixtureConfig);

    REQUIRE(map.persistent());
    REQUIRE(map.path() == path);

    auto* entityNode = new EntityNode{Entity{{{"name", "entity2"}}}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});

    auto mapWasSaved = Observer<>{map.mapWasSavedNotifier};
    auto modificationStateDidChange = Observer<>{map.modificationStateDidChangeNotifier};

    REQUIRE(map.save());

    CHECK(mapWasSaved.notifications == std::vector<std::tuple<>>{{}});
    CHECK(modificationStateDidChange.notifications == std::vector<std::tuple<>>{{}});
    CHECK(map.persistent());
    CHECK(map.path() == path);

    REQUIRE(env.fileExists(path));
    CHECK(env.loadFile(path) == R"(// Game: Test
// Format: Valve
// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"name" "entity1"
}
// entity 2
{
"name" "entity2"
}
)");
  }

  SECTION("saveAs")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();

    auto* entityNode = new EntityNode{Entity{{{"key", "value"}}}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    REQUIRE(map.worldNode().defaultLayer()->children() == std::vector<Node*>{entityNode});

    auto mapWasSaved = Observer<>{map.mapWasSavedNotifier};
    auto modificationStateDidChange = Observer<>{map.modificationStateDidChangeNotifier};

    auto env = fs::TestEnvironment{};

    const auto path = env.dir() / "test.map";
    REQUIRE(map.saveAs(path));

    CHECK(mapWasSaved.notifications == std::vector<std::tuple<>>{{}});
    CHECK(modificationStateDidChange.notifications == std::vector<std::tuple<>>{{}});
    CHECK(map.persistent());
    CHECK(map.path() == path);

    REQUIRE(env.fileExists(path));
    CHECK(env.loadFile(path) == R"(// Game: Test
// Format: Standard
// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"key" "value"
}
)");
  }

  SECTION("exportAs")
  {
    auto fixture = MapFixture{};
    auto env = fs::TestEnvironment{};

    SECTION("Export as obj")
    {
      auto& map = fixture.create();

      const auto builder = BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};

      auto* brushNode = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto objFilename = "test.obj";
      const auto mtlFilename = "test.mtl";

      REQUIRE(map.exportAs(ObjExportOptions{
        env.dir() / objFilename,
        ObjMtlPathMode::RelativeToExportPath,
      }));

      CHECK(env.fileExists(objFilename));
      CHECK(env.fileExists(mtlFilename));
      CHECK(!map.persistent());
      CHECK(map.path() == "unnamed.map");
    }

    SECTION("Export as map")
    {
      auto& map = fixture.create();

      auto* entityNode = new EntityNode{Entity{{{"key", "value"}}}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      const auto filename = "test.map";
      REQUIRE(map.exportAs(MapExportOptions{
        env.dir() / filename,
        !K(stripTbProperties),
        std::nullopt,
        std::nullopt,
      }));
      REQUIRE(env.fileExists(filename));
      CHECK(env.loadFile(filename) == R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"key" "value"
}
)");
      CHECK(!map.persistent());
      CHECK(map.path() == "unnamed.map");
    }

    SECTION("Omit layers from export")
    {
      const auto newDocumentPath = std::filesystem::path{"test.map"};
      auto& map = fixture.create(QuakeFixtureConfig);

      auto layer = mdl::Layer{"Layer"};
      layer.setOmitFromExport(true);

      auto* layerNode = new mdl::LayerNode{std::move(layer)};
      addNodes(map, {{&map.worldNode(), {layerNode}}});

      REQUIRE(map.exportAs(MapExportOptions{
        env.dir() / newDocumentPath,
        !K(stripTbProperties),
        std::nullopt,
        std::nullopt,
      }));
      REQUIRE(env.fileExists(newDocumentPath));
      CHECK(env.loadFile(newDocumentPath) == R"(// entity 0
{
"classname" "worldspawn"
}
)");
    }

    SECTION("Strip TB properties")
    {
      const auto newDocumentPath = std::filesystem::path{"test.map"};

      auto& map = fixture.create(QuakeFixtureConfig);

      auto* layerNode = new mdl::LayerNode{mdl::Layer{"Layer"}};
      addNodes(map, {{&map.worldNode(), {layerNode}}});

      REQUIRE(map.exportAs(MapExportOptions{
        env.dir() / newDocumentPath,
        K(stripTbProperties),
        std::nullopt,
        std::nullopt,
      }));
      REQUIRE(env.fileExists(newDocumentPath));
      CHECK(env.loadFile(newDocumentPath) == R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"classname" "func_group"
}
)");
    }

    SECTION("Strip entities")
    {
      const auto newDocumentPath = std::filesystem::path{"test.map"};

      auto& map = fixture.create(QuakeFixtureConfig);

      auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"classname", "light"}}}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      REQUIRE(map.exportAs(MapExportOptions{
        env.dir() / newDocumentPath,
        !K(stripTbProperties),
        "light",
        std::nullopt,
      }));
      REQUIRE(env.fileExists(newDocumentPath));
      CHECK(env.loadFile(newDocumentPath) == R"(// entity 0
{
"classname" "worldspawn"
}
)");
    }

    SECTION("Add entity")
    {
      const auto newDocumentPath = std::filesystem::path{"test.map"};

      auto& map = fixture.create(QuakeFixtureConfig);

      auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"classname", "light"}}}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});

      REQUIRE(map.exportAs(MapExportOptions{
        env.dir() / newDocumentPath,
        !K(stripTbProperties),
        std::nullopt,
        mdl::Entity{{{"classname", "info_player_start"}}},
      }));
      REQUIRE(env.fileExists(newDocumentPath));
      CHECK(env.loadFile(newDocumentPath) == R"(// entity 0
{
"classname" "worldspawn"
}
// entity 1
{
"classname" "light"
}
// entity 2
{
"classname" "info_player_start"
}
)");
    }
  }
}

} // namespace tb::mdl
