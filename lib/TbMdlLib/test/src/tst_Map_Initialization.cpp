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

#include "Logger.h"
#include "fs/TestEnvironment.h"
#include "gl/ResourceManager.h"
#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityDefinitionManager.h" // IWYU pragma: keep
#include "mdl/EnvironmentConfig.h"
#include "mdl/GameConfigFixture.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/PropertyDefinition.h"
#include "mdl/TestUtils.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{
namespace
{

auto makeAbsolute(const auto& path)
{
  return std::filesystem::current_path() / path;
}

} // namespace

TEST_CASE("Map_Initialization")
{
  auto environmentConfig = EnvironmentConfig{};

  auto taskManager = createTestTaskManager();
  auto resourceManager = gl::ResourceManager{};
  auto logger = NullLogger{};

  SECTION("createMap")
  {
    SECTION("Calling create sets worldspawn and notifies observers")
    {
      Map::createMap(
        environmentConfig,
        DefaultGameInfo,
        DefaultGameInfo.gamePathPreference.defaultValue,
        MapFormat::Standard,
        vm::bbox3d{8192.0},
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([](auto map) {
            CHECK(
              map->worldNode().entity()
              == Entity{{
                {EntityPropertyKeys::Classname,
                 EntityPropertyValues::WorldspawnClassname},
              }});
            CHECK(!map->worldBounds().is_empty());
            CHECK(!map->persistent());
            CHECK(map->path() == "unnamed.map");
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }

    SECTION("Loads the initial map if configured")
    {
      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.forceEmptyNewMap = false;
      gameInfo.gameConfig.path = "fixture/test/mdl/Map/GameConfig.cfg";
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Valve", {"initialMap.map"}},
      };

      Map::createMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Valve,
        vm::bbox3d{8192.0},
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([](auto map) {
            const auto* defaultLayerNode = map->worldNode().defaultLayer();
            REQUIRE(defaultLayerNode->children().size() == 1);

            const auto* brushNode =
              dynamic_cast<const BrushNode*>(defaultLayerNode->children().front());
            REQUIRE(brushNode);

            CHECK(
              brushNode->brush().bounds() == vm::bbox3d{{-32, -32, -64}, {32, 32, 64}});

            const auto* valveVersionProperty =
              map->worldNode().entity().property(EntityPropertyKeys::ValveVersion);
            REQUIRE(valveVersionProperty);
            CHECK(*valveVersionProperty == "220");
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }

    SECTION("Sets Valve version property")
    {
      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.forceEmptyNewMap = false;
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Valve", {"initialMap.map"}},
      };

      Map::createMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Valve,
        vm::bbox3d{8192.0},
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([](auto map) {
            const auto* valveVersionProperty =
              map->worldNode().entity().property(EntityPropertyKeys::ValveVersion);
            REQUIRE(valveVersionProperty);
            CHECK(*valveVersionProperty == "220");
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }

    SECTION("Sets material config property")
    {
      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.forceEmptyNewMap = false;
      gameInfo.gameConfig.materialConfig.property = "wad";
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Valve", {"initialMap.map"}},
      };

      Map::createMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Valve,
        vm::bbox3d{8192.0},
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([](auto map) {
            const auto* materialConfigProperty =
              map->worldNode().entity().property("wad");
            REQUIRE(materialConfigProperty);
            CHECK(*materialConfigProperty == "");
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }

    SECTION("Creates an initial brush if configured")
    {
      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.forceEmptyNewMap = false;
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{{"Standard", {}}};

      Map::createMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Valve,
        vm::bbox3d{8192.0},
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([](auto map) {
            const auto* defaultLayerNode = map->worldNode().defaultLayer();
            REQUIRE(defaultLayerNode->children().size() == 1);

            const auto* brushNode =
              dynamic_cast<const BrushNode*>(defaultLayerNode->children().front());
            REQUIRE(brushNode);

            CHECK(
              brushNode->brush().bounds() == vm::bbox3d{{-64, -64, -16}, {64, 64, 16}});
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }

    SECTION("Loads default entity definition file")
    {
      auto env = fs::TestEnvironment{};
      env.createFile("Quake.fgd", R"(@SolidClass = some_entity : "Some Entity" [])");

      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.path = env.dir() / "GameConfig.cfg";
      gameInfo.gameConfig.entityConfig.defFilePaths.emplace_back("Quake.fgd");

      Map::createMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Standard,
        vm::bbox3d{8192.0},
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([](auto map) {
            REQUIRE(map->entityDefinitionManager().definitions().size() == 1);
            CHECK(
              map->entityDefinitionManager().definitions().front().name == "some_entity");
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }

    SECTION("Add and convert properties")
    {
      using namespace EntityPropertyKeys;

      auto env = fs::TestEnvironment{};
      env.createFile("Quake.fgd", R"(@PointClass = some_entity : "Some Entity" [])");

      auto gameInfo = QuakeGameInfo;
      gameInfo.gameConfig.path = env.dir() / "GameConfig.cfg";
      gameInfo.gameConfig.entityConfig.defFilePaths.emplace_back("Quake.fgd");

      Map::createMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Standard,
        vm::bbox3d{8192.0},
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([](auto map) {
            CHECK(
              map->entityDefinitionManager().definitions()
              == std::vector{EntityDefinition{
                "some_entity",
                RgbaF{0.6f, 0.6f, 0.6f, 1.0f},
                "Some Entity",
                {
                  PropertyDefinition{
                    Target,
                    PropertyValueTypes::LinkSource{},
                    "name of entity to trigger",
                    "generated by TrenchBroom"},
                  PropertyDefinition{
                    Killtarget,
                    PropertyValueTypes::LinkSource{},
                    "name of entity to kill",
                    "generated by TrenchBroom"},
                  PropertyDefinition{
                    Targetname,
                    PropertyValueTypes::LinkTarget{},
                    "target name for linking",
                    "generated by TrenchBroom"},
                  PropertyDefinition{
                    Origin,
                    PropertyValueTypes::Origin{},
                    "point entity origin",
                    "generated by TrenchBroom"},

                },
                PointEntityDefinition{vm::bbox3d{8.0}, {}, {}},
                1}});
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }
  }

  SECTION("loadMap")
  {
    SECTION("Sets world bounds, game and file path")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      const auto path = makeAbsolute("fixture/test/mdl/Map/emptyValveMap.map");

      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Valve", {}},
      };

      Map::loadMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Unknown,
        worldBounds,
        path,
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([&](auto map) {
            CHECK(map->worldBounds() == worldBounds);
            CHECK(map->path() == path);
            CHECK(map->persistent());
          })
        | kdl::transform_error([](auto e) { FAIL(e.msg); });
    }

    SECTION("Format detection")
    {
      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Valve", {}},
        {"Standard", {}},
        {"Quake3", {}},
      };

      SECTION("Detect Valve Format Map")
      {
        Map::loadMap(
          environmentConfig,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          MapFormat::Unknown,
          vm::bbox3d{8192.0},
          makeAbsolute("fixture/test/mdl/Map/valveFormatMapWithoutFormatTag.map"),
          *taskManager,
          resourceManager,
          logger)
          | kdl::transform([&](auto map) {
              CHECK(map->worldNode().mapFormat() == mdl::MapFormat::Valve);
              CHECK(map->worldNode().defaultLayer()->childCount() == 1);
            })
          | kdl::transform_error([](auto e) { FAIL(e.msg); });
      }

      SECTION("Detect Standard Format Map")
      {
        Map::loadMap(
          environmentConfig,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          MapFormat::Unknown,
          vm::bbox3d{8192.0},
          makeAbsolute("fixture/test/mdl/Map/standardFormatMapWithoutFormatTag.map"),
          *taskManager,
          resourceManager,
          logger)
          | kdl::transform([&](auto map) {
              CHECK(map->worldNode().mapFormat() == mdl::MapFormat::Standard);
              CHECK(map->worldNode().defaultLayer()->childCount() == 1);
            })
          | kdl::transform_error([](auto e) { FAIL(e.msg); });
      }

      SECTION("detectEmptyMap")
      {
        Map::loadMap(
          environmentConfig,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          MapFormat::Unknown,
          vm::bbox3d{8192.0},
          makeAbsolute("fixture/test/mdl/Map/emptyMapWithoutFormatTag.map"),
          *taskManager,
          resourceManager,
          logger)
          | kdl::transform([&](auto map) {
              // an empty map detects as Valve because Valve is listed first in the game
              // config
              CHECK(map->worldNode().mapFormat() == mdl::MapFormat::Valve);
              CHECK(map->worldNode().defaultLayer()->childCount() == 0);
            })
          | kdl::transform_error([](auto e) { FAIL(e.msg); });
      }

      SECTION("mixedFormats")
      {
        // map has both Standard and Valve brushes
        CHECK(!Map::loadMap(
          environmentConfig,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          MapFormat::Unknown,
          vm::bbox3d{8192.0},
          makeAbsolute("fixture/test/mdl/Map/mixedFormats.map"),
          *taskManager,
          resourceManager,
          logger));
      }
    }

    SECTION("Loads default entity definition file")
    {
      auto env = fs::TestEnvironment{};
      env.createFile("Quake.fgd", R"(@SolidClass = some_entity : "Some Entity" [])");

      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.path = env.dir() / "GameConfig.cfg";
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Valve", {}},
      };
      gameInfo.gameConfig.entityConfig.defFilePaths.emplace_back("Quake.fgd");


      Map::loadMap(
        environmentConfig,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        MapFormat::Unknown,
        vm::bbox3d{8192.0},
        makeAbsolute("fixture/test/mdl/Map/valveFormatMapWithoutFormatTag.map"),
        *taskManager,
        resourceManager,
        logger)
        | kdl::transform([&](auto map) {
            REQUIRE(map->entityDefinitionManager().definitions().size() == 1);
            CHECK(
              map->entityDefinitionManager().definitions().front().name == "some_entity");
          })
        | kdl::transform_error([](auto e) { FAIL(e.msg); });
    }
  }

  SECTION("reload")
  {
    auto gameInfo = DefaultGameInfo;
    gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
      {"Valve", {}},
    };

    const auto worldBounds = vm::bbox3d{8192.0};
    const auto path = makeAbsolute("fixture/test/mdl/Map/emptyValveMap.map");

    Map::loadMap(
      environmentConfig,
      gameInfo,
      gameInfo.gamePathPreference.defaultValue,
      MapFormat::Unknown,
      vm::bbox3d{8192.0},
      path,
      *taskManager,
      resourceManager,
      logger)
      | kdl::and_then([&](auto map) {
          REQUIRE(map->worldBounds() == worldBounds);
          REQUIRE(&map->gameInfo() == &gameInfo);
          REQUIRE(map->path() == path);
          REQUIRE(map->persistent());

          auto* transientEntityNode = new EntityNode{{}};
          addNodes(*map, {{parentForNodes(*map), {transientEntityNode}}});
          REQUIRE(
            map->worldNode().defaultLayer()->children()
            == std::vector<Node*>{transientEntityNode});
          REQUIRE(map->modified());

          return map->reload() | kdl::transform([&](auto reloadedMap) {
                   CHECK(reloadedMap->worldBounds() == worldBounds);
                   CHECK(&reloadedMap->gameInfo() == &gameInfo);
                   CHECK(reloadedMap->path() == path);
                   CHECK(reloadedMap->persistent());
                   CHECK(!reloadedMap->modified());
                   CHECK(
                     reloadedMap->worldNode().defaultLayer()->children()
                     == std::vector<Node*>{});
                 });
        })
      | kdl::transform_error([](auto e) { FAIL(e.msg); });
  }
}

} // namespace tb::mdl
