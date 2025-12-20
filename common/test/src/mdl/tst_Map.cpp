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
 along with TrenchBroom. If not, see <http:www.gnu.org/licenses/>.
 */

#include "Logger.h"
#include "Observer.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "fs/TestEnvironment.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GameInfo.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Material.h"
#include "mdl/MaterialManager.h"
#include "mdl/PasteType.h"
#include "mdl/TagMatcher.h"
#include "mdl/TextureResource.h"
#include "mdl/TransactionScope.h"
#include "mdl/UpdateBrushFaceAttributes.h"
#include "mdl/WorldNode.h"

#include "kd/vector_utils.h"

#include "vm/approx.h"
#include "vm/vec_io.h" // IWYU pragma: keep

#include "catch/CatchConfig.h"
#include "catch/Matchers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>
#include <catch2/matchers/catch_matchers_quantifiers.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{

class TestCallback : public TagMatcherCallback
{
private:
  size_t m_option;

public:
  explicit TestCallback(const size_t option)
    : m_option{option}
  {
  }

  size_t selectOption(const std::vector<std::string>&) override { return m_option; }
};

auto makeAbsolute(const auto& path)
{
  return std::filesystem::current_path() / path;
}

} // namespace

TEST_CASE("Map")
{
  SECTION("create")
  {
    auto taskManager = createTestTaskManager();
    auto logger = NullLogger{};

    SECTION("Calling create sets worldspawn and notifies observers")
    {
      Map::createMap(
        MapFormat::Standard,
        DefaultGameInfo,
        DefaultGameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
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
        MapFormat::Valve,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
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
        MapFormat::Valve,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
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
        MapFormat::Valve,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
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
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Standard", {}},
      };

      Map::createMap(
        MapFormat::Valve,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
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
        MapFormat::Standard,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
        logger)
        | kdl::transform([](auto map) {
            REQUIRE(map->entityDefinitionManager().definitions().size() == 1);
            CHECK(
              map->entityDefinitionManager().definitions().front().name == "some_entity");
          })
        | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
    }
  }

  SECTION("loadMap")
  {
    auto taskManager = createTestTaskManager();
    auto logger = NullLogger{};

    SECTION("Sets world bounds, game and file path")
    {
      const auto worldBounds = vm::bbox3d{8192.0};
      const auto path = makeAbsolute("fixture/test/mdl/Map/emptyValveMap.map");

      auto gameInfo = DefaultGameInfo;
      gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
        {"Valve", {}},
      };

      Map::loadMap(
        path,
        MapFormat::Unknown,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        worldBounds,
        *taskManager,
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
          makeAbsolute("fixture/test/mdl/Map/valveFormatMapWithoutFormatTag.map"),
          MapFormat::Unknown,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          vm::bbox3d{8192.0},
          *taskManager,
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
          makeAbsolute("fixture/test/mdl/Map/standardFormatMapWithoutFormatTag.map"),
          MapFormat::Unknown,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          vm::bbox3d{8192.0},
          *taskManager,
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
          makeAbsolute("fixture/test/mdl/Map/emptyMapWithoutFormatTag.map"),
          MapFormat::Unknown,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          vm::bbox3d{8192.0},
          *taskManager,
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
          makeAbsolute("fixture/test/mdl/Map/mixedFormats.map"),
          MapFormat::Unknown,
          gameInfo,
          gameInfo.gamePathPreference.defaultValue,
          vm::bbox3d{8192.0},
          *taskManager,
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
        makeAbsolute("fixture/test/mdl/Map/valveFormatMapWithoutFormatTag.map"),
        MapFormat::Unknown,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
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
    auto taskManager = createTestTaskManager();
    auto logger = NullLogger{};

    auto gameInfo = DefaultGameInfo;
    gameInfo.gameConfig.fileFormats = std::vector<MapFormatConfig>{
      {"Valve", {}},
    };

    const auto worldBounds = vm::bbox3d{8192.0};
    const auto path = makeAbsolute("fixture/test/mdl/Map/emptyValveMap.map");

    Map::loadMap(
      path,
      MapFormat::Unknown,
      gameInfo,
      gameInfo.gamePathPreference.defaultValue,
      vm::bbox3d{8192.0},
      *taskManager,
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

    auto mapWasSaved = Observer<void>{map.mapWasSavedNotifier};
    auto modificationStateDidChange =
      Observer<void>{map.modificationStateDidChangeNotifier};

    REQUIRE(map.save());

    CHECK(mapWasSaved.called);
    CHECK(modificationStateDidChange.called);
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

    auto mapWasSaved = Observer<void>{map.mapWasSavedNotifier};
    auto modificationStateDidChange =
      Observer<void>{map.modificationStateDidChangeNotifier};

    auto env = fs::TestEnvironment{};

    const auto path = env.dir() / "test.map";
    REQUIRE(map.saveAs(path));

    CHECK(mapWasSaved.called);
    CHECK(modificationStateDidChange.called);
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

      REQUIRE(map.exportAs(io::ObjExportOptions{
        env.dir() / objFilename,
        io::ObjMtlPathMode::RelativeToExportPath,
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
      REQUIRE(map.exportAs(io::MapExportOptions{env.dir() / filename}));
      CHECK(env.fileExists(filename));
      CHECK(!map.persistent());
      CHECK(map.path() == "unnamed.map");
    }

    SECTION("Omit layers from export")
    {
      const auto newDocumentPath = std::filesystem::path{"test.map"};

      {
        auto& map = fixture.create(QuakeFixtureConfig);

        auto layer = mdl::Layer{"Layer"};
        layer.setOmitFromExport(true);

        auto* layerNode = new mdl::LayerNode{std::move(layer)};
        addNodes(map, {{&map.worldNode(), {layerNode}}});

        REQUIRE(map.exportAs(io::MapExportOptions{env.dir() / newDocumentPath}));
        REQUIRE(env.fileExists(newDocumentPath));
      }

      auto& map = fixture.load(env.dir() / newDocumentPath, QuakeFixtureConfig);
      CHECK(map.worldNode().customLayers().empty());
    }
  }

  SECTION("persistent")
  {
    auto fixture = MapFixture{};

    SECTION("A newly created map is transient")
    {
      auto& map = fixture.create();

      CHECK(!map.persistent());
    }

    SECTION("A loaded map is persistent")
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
      auto& map = fixture.load(path, QuakeFixtureConfig);

      CHECK(map.persistent());

      SECTION("If the backing file is deleted, the map is transient again")
      {
        REQUIRE(env.remove(filename));

        CHECK(!map.persistent());
      }
    }
  }

  SECTION("modified")
  {
    auto fixture = MapFixture{};

    auto& map = fixture.create();
    CHECK(!map.modified());

    auto* entityNode = new EntityNode{Entity{{{"key", "value"}}}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});

    CHECK(map.modified());

    SECTION("Saving resets the modified flag")
    {
      auto env = fs::TestEnvironment{};
      REQUIRE(map.saveAs(env.dir() / "test.map"));

      CHECK(!map.modified());
    }
  }

  SECTION("selection")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();

    SECTION("brushFaces")
    {
      auto* brushNode = createBrushNode(map);
      CHECK(brushNode->logicalBounds().center() == vm::vec3d{0, 0, 0});

      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto topFaceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
      REQUIRE(topFaceIndex);

      // select the top face
      selectBrushFaces(map, {{brushNode, *topFaceIndex}});
      CHECK_THAT(
        map.selection().brushFaces,
        Equals(std::vector<mdl::BrushFaceHandle>{{brushNode, *topFaceIndex}}));

      // deselect it
      deselectBrushFaces(map, {{brushNode, *topFaceIndex}});
      CHECK_THAT(map.selection().brushFaces, Equals(std::vector<mdl::BrushFaceHandle>{}));

      // select the brush
      selectNodes(map, {brushNode});
      CHECK_THAT(
        map.selection().brushes, Equals(std::vector<mdl::BrushNode*>{brushNode}));

      // translate the brush
      translateSelection(map, vm::vec3d{10.0, 0.0, 0.0});
      CHECK(brushNode->logicalBounds().center() == vm::vec3d{10.0, 0.0, 0.0});

      // Start undoing changes

      map.undoCommand();
      CHECK(brushNode->logicalBounds().center() == vm::vec3d{0, 0, 0});
      CHECK_THAT(
        map.selection().brushes, Equals(std::vector<mdl::BrushNode*>{brushNode}));
      CHECK_THAT(map.selection().brushFaces, Equals(std::vector<mdl::BrushFaceHandle>{}));

      map.undoCommand();
      CHECK_THAT(map.selection().brushes, Equals(std::vector<mdl::BrushNode*>{}));
      CHECK_THAT(map.selection().brushFaces, Equals(std::vector<mdl::BrushFaceHandle>{}));

      map.undoCommand();
      CHECK_THAT(
        map.selection().brushFaces,
        Equals(std::vector<mdl::BrushFaceHandle>{{brushNode, *topFaceIndex}}));
    }

    SECTION("allEntities")
    {
      GIVEN("A document with multiple entity nodes in various configurations")
      {
        auto* topLevelEntityNode = new EntityNode{Entity{}};

        auto* emptyGroupNode = new GroupNode{Group{"empty"}};
        auto* groupNodeWithEntity = new GroupNode{Group{"group"}};
        auto* groupedEntityNode = new EntityNode{Entity{}};
        groupNodeWithEntity->addChild(groupedEntityNode);

        auto* topLevelBrushNode = createBrushNode(map);
        auto* topLevelPatchNode = createPatchNode();

        auto* topLevelBrushEntityNode = new EntityNode{Entity{}};
        auto* brushEntityBrushNode = createBrushNode(map);
        auto* brushEntityPatchNode = createPatchNode();
        topLevelBrushEntityNode->addChildren(
          {brushEntityBrushNode, brushEntityPatchNode});

        addNodes(
          map,
          {{parentForNodes(map),
            {topLevelEntityNode,
             topLevelBrushEntityNode,
             topLevelBrushNode,
             topLevelPatchNode,
             emptyGroupNode,
             groupNodeWithEntity}}});

        deselectAll(map);

        WHEN("Nothing is selected")
        {
          THEN("The world node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              UnorderedEquals(std::vector<EntityNodeBase*>{&map.worldNode()}));
          }
        }

        WHEN("A top level brush node is selected")
        {
          selectNodes(map, {topLevelBrushNode});

          THEN("The world node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              UnorderedEquals(std::vector<EntityNodeBase*>{&map.worldNode()}));
          }
        }

        WHEN("A top level patch node is selected")
        {
          selectNodes(map, {topLevelPatchNode});

          THEN("The world node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              UnorderedEquals(std::vector<EntityNodeBase*>{&map.worldNode()}));
          }
        }

        WHEN("An empty group node is selected")
        {
          selectNodes(map, {emptyGroupNode});

          THEN("Worldspawn is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              UnorderedEquals(std::vector<EntityNodeBase*>{&map.worldNode()}));
          }
        }

        WHEN("A group node containing an entity node is selected")
        {
          selectNodes(map, {groupNodeWithEntity});

          THEN("The grouped entity node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              UnorderedEquals(std::vector<EntityNodeBase*>{groupedEntityNode}));
          }

          AND_WHEN("A top level entity node is selected")
          {
            selectNodes(map, {topLevelEntityNode});

            THEN("The top level entity node and the grouped entity node are returned")
            {
              CHECK_THAT(
                map.selection().allEntities(),
                UnorderedEquals(
                  std::vector<EntityNodeBase*>{groupedEntityNode, topLevelEntityNode}));
            }
          }
        }

        WHEN("An empty top level entity node is selected")
        {
          selectNodes(map, {topLevelEntityNode});

          THEN("That entity node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              UnorderedEquals(std::vector<EntityNodeBase*>{topLevelEntityNode}));
          }
        }

        WHEN("A node in a brush entity node is selected")
        {
          using SelectNodes =
            std::function<std::tuple<Node*, Node*>(BrushNode*, PatchNode*)>;

          const auto selectBrushNode =
            SelectNodes{[](auto* brushNode, auto* patchNode) -> std::tuple<Node*, Node*> {
              return {brushNode, patchNode};
            }};
          const auto selectPatchNode =
            SelectNodes{[](auto* brushNode, auto* patchNode) -> std::tuple<Node*, Node*> {
              return {patchNode, brushNode};
            }};
          const auto selectNodes = GENERATE_COPY(selectBrushNode, selectPatchNode);

          const auto [nodeToSelect, otherNode] =
            selectNodes(brushEntityBrushNode, brushEntityPatchNode);

          CAPTURE(nodeToSelect->name(), otherNode->name());

          mdl::selectNodes(map, {nodeToSelect});

          THEN("The containing entity node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              UnorderedEquals(std::vector<EntityNodeBase*>{topLevelBrushEntityNode}));
          }

          AND_WHEN("Another node in the same entity node is selected")
          {
            mdl::selectNodes(map, {otherNode});

            THEN("The containing entity node is returned only once")
            {
              CHECK_THAT(
                map.selection().allEntities(),
                UnorderedEquals(std::vector<EntityNodeBase*>{topLevelBrushEntityNode}));
            }
          }

          AND_WHEN("A top level entity node is selected")
          {
            mdl::selectNodes(map, {topLevelEntityNode});

            THEN("The top level entity node and the brush entity node are returned")
            {
              CHECK_THAT(
                map.selection().allEntities(),
                UnorderedEquals(std::vector<EntityNodeBase*>{
                  topLevelBrushEntityNode, topLevelEntityNode}));
            }
          }
        }
      }
    }

    SECTION("allBrushes")
    {
      auto* brushNodeInDefaultLayer = createBrushNode(map, "brushNodeInDefaultLayer");
      auto* brushNodeInCustomLayer = createBrushNode(map, "brushNodeInCustomLayer");
      auto* brushNodeInEntity = createBrushNode(map, "brushNodeInEntity");
      auto* brushNodeInGroup = createBrushNode(map, "brushNodeInGroup");
      auto* brushNodeInNestedGroup = createBrushNode(map, "brushNodeInNestedGroup");

      auto* customLayerNode = new LayerNode{Layer{"customLayerNode"}};
      auto* brushEntityNode = new EntityNode{Entity{}};
      auto* pointEntityNode = new EntityNode{Entity{}};
      auto* outerGroupNode = new GroupNode{Group{"outerGroupNode"}};
      auto* innerGroupNode = new GroupNode{Group{"outerGroupNode"}};

      addNodes(
        map,
        {{map.worldNode().defaultLayer(),
          {brushNodeInDefaultLayer, brushEntityNode, pointEntityNode, outerGroupNode}},
         {&map.worldNode(), {customLayerNode}}});

      addNodes(
        map,
        {
          {customLayerNode, {brushNodeInCustomLayer}},
          {outerGroupNode, {innerGroupNode, brushNodeInGroup}},
          {brushEntityNode, {brushNodeInEntity}},
        });

      addNodes(map, {{innerGroupNode, {brushNodeInNestedGroup}}});

      const auto getPath = [&](const Node* node) {
        return node->pathFrom(map.worldNode());
      };
      const auto resolvePaths = [&](const auto& paths) {
        return paths | std::views::transform([&](const auto& path) {
                 return map.worldNode().resolvePath(path);
               })
               | kdl::ranges::to<std::vector>();
      };

      using T = std::vector<NodePath>;

      // clang-format off
      const auto 
      paths = GENERATE_COPY(values<T>({
      {},
      {getPath(brushNodeInDefaultLayer)},
      {getPath(brushNodeInDefaultLayer), getPath(brushNodeInCustomLayer)},
      {getPath(brushNodeInDefaultLayer), getPath(brushNodeInCustomLayer), getPath(brushNodeInEntity)},
      {getPath(brushNodeInGroup)},
      {getPath(brushNodeInGroup), getPath(brushNodeInNestedGroup)},
      }));
      // clang-format on

      const auto nodes = resolvePaths(paths);
      const auto brushNodes = kdl::vec_static_cast<BrushNode*>(nodes);

      selectNodes(map, nodes);

      CHECK_THAT(map.selection().allBrushes(), UnorderedEquals(brushNodes));
    }
  }

  SECTION("Tag management")
  {
    const auto materialMatch = std::string{"some_material"};
    const auto materialPatternMatch = std::string{"*er_material"};
    const auto singleParamMatch = std::string{"parm2"};
    const auto multiParamsMatch =
      kdl::vector_set<std::string>{"some_parm", "parm1", "parm3"};

    auto fixtureConfig = MapFixtureConfig{};
    fixtureConfig.gameInfo.gameConfig.smartTags = {
      SmartTag{
        "material",
        {},
        std::make_unique<MaterialNameTagMatcher>(materialMatch),
      },
      SmartTag{
        "materialPattern",
        {},
        std::make_unique<MaterialNameTagMatcher>(materialPatternMatch),
      },
      SmartTag{
        "surfaceparm_single",
        {},
        std::make_unique<SurfaceParmTagMatcher>(singleParamMatch),
      },
      SmartTag{
        "surfaceparm_multi",
        {},
        std::make_unique<SurfaceParmTagMatcher>(multiParamsMatch),
      },
      SmartTag{
        "contentflags",
        {},
        std::make_unique<ContentFlagsTagMatcher>(1),
      },
      SmartTag{
        "surfaceflags",
        {},
        std::make_unique<SurfaceFlagsTagMatcher>(1),
      },
      SmartTag{
        "entity",
        {},
        std::make_unique<EntityClassNameTagMatcher>("brush_entity", ""),
      }};

    auto fixture = MapFixture{};
    auto& map = fixture.create(fixtureConfig);

    map.entityDefinitionManager().setDefinitions({
      {"brush_entity", Color{}, "this is a brush entity", {}},
    });

    const auto* brushEntityDefinition =
      map.entityDefinitionManager().definition("brush_entity");

    auto& materialManager = map.materialManager();
    {
      auto materialA = Material{"some_material", createTextureResource(Texture{16, 16})};
      auto materialB = Material{"other_material", createTextureResource(Texture{32, 32})};
      auto materialC =
        Material{"yet_another_material", createTextureResource(Texture{64, 64})};

      const auto singleParam = std::string{"some_parm"};
      const auto multiParams = std::set<std::string>{"parm1", "parm2"};

      materialA.setSurfaceParms({singleParam});
      materialB.setSurfaceParms(multiParams);

      auto materials =
        kdl::vec_from(std::move(materialA), std::move(materialB), std::move(materialC));

      auto collections = kdl::vec_from(MaterialCollection{std::move(materials)});

      materialManager.setMaterialCollections(std::move(collections));
    }

    auto* materialA = materialManager.material("some_material");
    auto* materialB = materialManager.material("other_material");
    auto* materialC = materialManager.material("yet_another_material");

    SECTION("registerSmartTags")
    {
      CHECK(map.isRegisteredSmartTag("material"));
      CHECK(map.smartTag("material").index() == 0u);
      CHECK(map.smartTag("material").type() == 1u);

      CHECK(map.isRegisteredSmartTag("materialPattern"));
      CHECK(map.smartTag("materialPattern").index() == 1u);
      CHECK(map.smartTag("materialPattern").type() == 2u);

      CHECK(map.isRegisteredSmartTag("surfaceparm_single"));
      CHECK(map.smartTag("surfaceparm_single").index() == 2u);
      CHECK(map.smartTag("surfaceparm_single").type() == 4u);

      CHECK(map.isRegisteredSmartTag("surfaceparm_multi"));
      CHECK(map.smartTag("surfaceparm_multi").index() == 3u);
      CHECK(map.smartTag("surfaceparm_multi").type() == 8u);

      CHECK(map.isRegisteredSmartTag("contentflags"));
      CHECK(map.smartTag("contentflags").index() == 4u);
      CHECK(map.smartTag("contentflags").type() == 16u);

      CHECK(map.isRegisteredSmartTag("surfaceflags"));
      CHECK(map.smartTag("surfaceflags").index() == 5u);
      CHECK(map.smartTag("surfaceflags").type() == 32u);

      CHECK(map.isRegisteredSmartTag("entity"));
      CHECK(map.smartTag("entity").index() == 6u);
      CHECK(map.smartTag("entity").type() == 64u);

      CHECK_FALSE(map.isRegisteredSmartTag(""));
      CHECK_FALSE(map.isRegisteredSmartTag("asdf"));
    }

    SECTION("registerSmartTags checks duplicate tags")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/2905

      auto fixtureConfigWithDuplicateTags = MapFixtureConfig{};
      fixtureConfigWithDuplicateTags.gameInfo.gameConfig.smartTags = {
        SmartTag{
          "material",
          {},
          std::make_unique<MaterialNameTagMatcher>("some_material"),
        },
        SmartTag{
          "material",
          {},
          std::make_unique<SurfaceParmTagMatcher>("some_other_material"),
        },
      };
      CHECK_THROWS_AS(fixture.create(fixtureConfigWithDuplicateTags), std::logic_error);
    }

    SECTION("addNodes initializes brush tags")
    {
      auto* entityNode = new EntityNode{Entity{{
        {"classname", "brush_entity"},
      }}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      REQUIRE(entityNode->entity().definition() == brushEntityDefinition);

      auto* brush = createBrushNode(map, "some_material");
      addNodes(map, {{entityNode, {brush}}});

      const auto& tag = map.smartTag("entity");
      CHECK(brush->hasTag(tag));
    }

    SECTION("removeNodes removes tags")
    {
      SECTION("Brush tags")
      {
        auto* entityNode = new EntityNode{Entity{{
          {"classname", "brush_entity"},
        }}};
        addNodes(map, {{parentForNodes(map), {entityNode}}});
        REQUIRE(entityNode->entity().definition() == brushEntityDefinition);

        auto* brush = createBrushNode(map, "some_material");
        addNodes(map, {{entityNode, {brush}}});

        removeNodes(map, {brush});

        const auto& tag = map.smartTag("entity");
        CHECK_FALSE(brush->hasTag(tag));
      }

      SECTION("Brush face tags")
      {
        auto* brushNodeWithTags = createBrushNode(map, "some_material");
        addNodes(map, {{parentForNodes(map), {brushNodeWithTags}}});
        removeNodes(map, {brushNodeWithTags});

        const auto& tag = map.smartTag("material");
        for (const auto& face : brushNodeWithTags->brush().faces())
        {
          CHECK_FALSE(face.hasTag(tag));
        }
      }
    }

    SECTION("reparentNodes updates brush tags")
    {
      SECTION("Reparent from world to entity")
      {
        auto* brushNode = createBrushNode(map, "some_material");
        addNodes(map, {{parentForNodes(map), {brushNode}}});

        auto* entityNode = new EntityNode{Entity{{
          {"classname", "brush_entity"},
        }}};
        addNodes(map, {{parentForNodes(map), {entityNode}}});
        REQUIRE(entityNode->entity().definition() == brushEntityDefinition);

        const auto& tag = map.smartTag("entity");
        CHECK_FALSE(brushNode->hasTag(tag));

        reparentNodes(map, {{entityNode, {brushNode}}});
        CHECK(brushNode->hasTag(tag));
      }

      SECTION("Reparent between entities")
      {
        auto* lightEntityNode = new EntityNode{Entity{{
          {"classname", "brush_entity"},
        }}};
        auto* otherEntityNode = new EntityNode{Entity{{
          {"classname", "other"},
        }}};
        addNodes(map, {{parentForNodes(map), {lightEntityNode, otherEntityNode}}});
        REQUIRE(lightEntityNode->entity().definition() == brushEntityDefinition);

        auto* brushNode = createBrushNode(map, "some_material");
        addNodes(map, {{otherEntityNode, {brushNode}}});

        const auto& tag = map.smartTag("entity");
        CHECK_FALSE(brushNode->hasTag(tag));

        reparentNodes(map, {{lightEntityNode, {brushNode}}});
        CHECK(brushNode->hasTag(tag));
      }
    }

    SECTION("setEntityProperty updates tags")
    {
      auto* lightEntityNode = new EntityNode{Entity{{
        {"classname", "asdf"},
      }}};
      addNodes(map, {{parentForNodes(map), {lightEntityNode}}});

      auto* brushNode = createBrushNode(map, "some_material");
      addNodes(map, {{lightEntityNode, {brushNode}}});

      const auto& tag = map.smartTag("entity");
      CHECK_FALSE(brushNode->hasTag(tag));

      selectNodes(map, {lightEntityNode});
      setEntityProperty(map, "classname", "brush_entity");
      deselectAll(map);

      CHECK(brushNode->hasTag(tag));
    }

    SECTION("setBrushFaceAttributes updates tags")
    {
      auto* brushNode = createBrushNode(map, "asdf");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto& tag = map.smartTag("contentflags");

      const auto faceHandle = BrushFaceHandle{brushNode, 0u};
      CHECK_FALSE(faceHandle.face().hasTag(tag));

      selectBrushFaces(map, {faceHandle});
      setBrushFaceAttributes(map, {.surfaceContents = SetFlagBits{1}});
      deselectAll(map);

      const auto& faces = brushNode->brush().faces();
      CHECK(faces[0].hasTag(tag));
      for (size_t i = 1u; i < faces.size(); ++i)
      {
        CHECK(!faces[i].hasTag(tag));
      }
    }

    SECTION("Material name tag")
    {
      SECTION("matches")
      {
        auto nodeA = std::unique_ptr<BrushNode>{createBrushNode(map, materialA->name())};
        auto nodeB = std::unique_ptr<BrushNode>{createBrushNode(map, materialB->name())};
        auto nodeC = std::unique_ptr<BrushNode>{createBrushNode(map, materialC->name())};
        const auto& tag = map.smartTag("material");
        const auto& patternTag = map.smartTag("materialPattern");
        for (const auto& face : nodeA->brush().faces())
        {
          CHECK(tag.matches(face));
          CHECK_FALSE(patternTag.matches(face));
        }
        for (const auto& face : nodeB->brush().faces())
        {
          CHECK_FALSE(tag.matches(face));
          CHECK(patternTag.matches(face));
        }
        for (const auto& face : nodeC->brush().faces())
        {
          CHECK_FALSE(tag.matches(face));
          CHECK(patternTag.matches(face));
        }
      }

      SECTION("enable")
      {
        auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
        addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

        const auto& tag = map.smartTag("material");
        CHECK(tag.canEnable());

        const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
        CHECK_FALSE(tag.matches(faceHandle.face()));

        selectBrushFaces(map, {faceHandle});

        auto callback = TestCallback{0};
        tag.enable(callback, map);

        CHECK(tag.matches(faceHandle.face()));
      }

      SECTION("disable")
      {
        const auto& tag = map.smartTag("material");
        CHECK_FALSE(tag.canDisable());
      }
    }

    SECTION("Surface parameter tag")
    {
      SECTION("matches")
      {
        auto nodeA = std::unique_ptr<BrushNode>{
          createBrushNode(map, materialA->name(), [&](auto& b) {
            for (auto& face : b.faces())
            {
              face.setMaterial(materialA);
            }
          })};
        auto nodeB = std::unique_ptr<BrushNode>{
          createBrushNode(map, materialB->name(), [&](auto& b) {
            for (auto& face : b.faces())
            {
              face.setMaterial(materialB);
            }
          })};
        auto nodeC = std::unique_ptr<BrushNode>{
          createBrushNode(map, materialC->name(), [&](auto& b) {
            for (auto& face : b.faces())
            {
              face.setMaterial(materialC);
            }
          })};
        const auto& singleTag = map.smartTag("surfaceparm_single");
        const auto& multiTag = map.smartTag("surfaceparm_multi");
        for (const auto& face : nodeA->brush().faces())
        {
          CHECK_FALSE(singleTag.matches(face));
          CHECK(multiTag.matches(face));
        }
        for (const auto& face : nodeB->brush().faces())
        {
          CHECK(singleTag.matches(face));
          CHECK(multiTag.matches(face));
        }
        for (const auto& face : nodeC->brush().faces())
        {
          CHECK_FALSE(singleTag.matches(face));
          CHECK_FALSE(multiTag.matches(face));
        }
      }

      SECTION("enable")
      {
        auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
        addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

        const auto& tag = map.smartTag("surfaceparm_single");
        CHECK(tag.canEnable());

        const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
        CHECK_FALSE(tag.matches(faceHandle.face()));

        selectBrushFaces(map, {faceHandle});

        auto callback = TestCallback{0};
        tag.enable(callback, map);

        CHECK(tag.matches(faceHandle.face()));
      }

      SECTION("disable")
      {
        const auto& tag = map.smartTag("surfaceparm_single");
        CHECK_FALSE(tag.canDisable());
      }
    }

    SECTION("Content flags tag")
    {
      SECTION("matches")
      {
        auto matchingBrushNode =
          std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
            for (auto& face : b.faces())
            {
              auto attributes = face.attributes();
              attributes.setSurfaceContents(1);
              face.setAttributes(attributes);
            }
          })};
        auto nonMatchingBrushNode =
          std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
            for (auto& face : b.faces())
            {
              auto attributes = face.attributes();
              attributes.setSurfaceContents(2);
              face.setAttributes(attributes);
            }
          })};

        const auto& tag = map.smartTag("contentflags");
        for (const auto& face : matchingBrushNode->brush().faces())
        {
          CHECK(tag.matches(face));
        }
        for (const auto& face : nonMatchingBrushNode->brush().faces())
        {
          CHECK_FALSE(tag.matches(face));
        }
      }

      SECTION("enable")
      {
        auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
        addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

        const auto& tag = map.smartTag("contentflags");
        CHECK(tag.canEnable());

        const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
        CHECK_FALSE(tag.matches(faceHandle.face()));

        selectBrushFaces(map, {faceHandle});

        auto callback = TestCallback{0};
        tag.enable(callback, map);

        CHECK(tag.matches(faceHandle.face()));
      }

      SECTION("disable")
      {
        auto* matchingBrushNode = createBrushNode(map, "asdf", [](auto& b) {
          for (auto& face : b.faces())
          {
            auto attributes = face.attributes();
            attributes.setSurfaceContents(1);
            face.setAttributes(attributes);
          }
        });

        addNodes(map, {{parentForNodes(map), {matchingBrushNode}}});

        const auto& tag = map.smartTag("contentflags");
        CHECK(tag.canDisable());

        const auto faceHandle = BrushFaceHandle{matchingBrushNode, 0u};
        CHECK(tag.matches(faceHandle.face()));

        selectBrushFaces(map, {faceHandle});

        auto callback = TestCallback{0};
        tag.disable(callback, map);

        CHECK_FALSE(tag.matches(faceHandle.face()));
      }
    }

    SECTION("Surface flags tag")
    {
      SECTION("matches")
      {
        auto matchingBrushNode =
          std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
            for (auto& face : b.faces())
            {
              auto attributes = face.attributes();
              attributes.setSurfaceFlags(1);
              face.setAttributes(attributes);
            }
          })};
        auto nonMatchingBrushNode =
          std::unique_ptr<BrushNode>{createBrushNode(map, "asdf", [](auto& b) {
            for (auto& face : b.faces())
            {
              auto attributes = face.attributes();
              attributes.setSurfaceFlags(2);
              face.setAttributes(attributes);
            }
          })};

        const auto& tag = map.smartTag("surfaceflags");
        for (const auto& face : matchingBrushNode->brush().faces())
        {
          CHECK(tag.matches(face));
        }
        for (const auto& face : nonMatchingBrushNode->brush().faces())
        {
          CHECK_FALSE(tag.matches(face));
        }
      }

      SECTION("enable")
      {
        auto* nonMatchingBrushNode = createBrushNode(map, "asdf");
        addNodes(map, {{parentForNodes(map), {nonMatchingBrushNode}}});

        const auto& tag = map.smartTag("surfaceflags");
        CHECK(tag.canEnable());

        const auto faceHandle = BrushFaceHandle{nonMatchingBrushNode, 0u};
        CHECK_FALSE(tag.matches(faceHandle.face()));

        selectBrushFaces(map, {faceHandle});

        auto callback = TestCallback{0};
        tag.enable(callback, map);

        CHECK(tag.matches(faceHandle.face()));
      }

      SECTION("disable")
      {
        auto* matchingBrushNode = createBrushNode(map, "asdf", [](auto& b) {
          for (auto& face : b.faces())
          {
            auto attributes = face.attributes();
            attributes.setSurfaceFlags(1);
            face.setAttributes(attributes);
          }
        });

        addNodes(map, {{parentForNodes(map), {matchingBrushNode}}});

        const auto& tag = map.smartTag("surfaceflags");
        CHECK(tag.canDisable());

        const auto faceHandle = BrushFaceHandle{matchingBrushNode, 0u};
        CHECK(tag.matches(faceHandle.face()));

        selectBrushFaces(map, {faceHandle});

        auto callback = TestCallback{0};
        tag.disable(callback, map);

        CHECK_FALSE(tag.matches(faceHandle.face()));
      }
    }

    SECTION("Entity classname tag")
    {
      SECTION("matches")
      {
        auto* matchingBrushNode = createBrushNode(map, "asdf");
        auto* nonMatchingBrushNode = createBrushNode(map, "asdf");

        auto matchingEntity =
          std::make_unique<EntityNode>(Entity{{{"classname", "brush_entity"}}});
        matchingEntity->addChild(matchingBrushNode);

        auto nonMatchingEntity =
          std::make_unique<EntityNode>(Entity{{{"classname", "something"}}});
        nonMatchingEntity->addChild(nonMatchingBrushNode);

        const auto& tag = map.smartTag("entity");
        CHECK(tag.matches(*matchingBrushNode));
        CHECK_FALSE(tag.matches(*nonMatchingBrushNode));
      }

      SECTION("enable")
      {
        auto* brushNode = createBrushNode(map, "asdf");
        addNodes(map, {{parentForNodes(map), {brushNode}}});

        const auto& tag = map.smartTag("entity");
        CHECK_FALSE(tag.matches(*brushNode));

        CHECK(tag.canEnable());

        selectNodes(map, {brushNode});

        auto callback = TestCallback{0};
        tag.enable(callback, map);
        CHECK(tag.matches(*brushNode));
      }

      SECTION("enable retains entity properties")
      {
        auto* brushNode = createBrushNode(map, "asdf");

        auto* oldEntity = new EntityNode{Entity{{
          {"classname", "something"},
          {"some_attr", "some_value"},
        }}};

        addNodes(map, {{parentForNodes(map), {oldEntity}}});
        addNodes(map, {{oldEntity, {brushNode}}});

        const auto& tag = map.smartTag("entity");
        selectNodes(map, {brushNode});

        auto callback = TestCallback{0};
        tag.enable(callback, map);
        CHECK(tag.matches(*brushNode));

        auto* newEntityNode = brushNode->entity();
        CHECK(newEntityNode != oldEntity);

        CHECK(newEntityNode != nullptr);
        CHECK(newEntityNode->entity().hasProperty("some_attr"));
        CHECK(*newEntityNode->entity().property("some_attr") == "some_value");
      }

      SECTION("disable")
      {
        auto* brushNode = createBrushNode(map, "asdf");

        auto* oldEntityNode = new EntityNode{Entity{{
          {"classname", "brush_entity"},
        }}};

        addNodes(map, {{parentForNodes(map), {oldEntityNode}}});
        addNodes(map, {{oldEntityNode, {brushNode}}});
        REQUIRE(oldEntityNode->entity().definition() == brushEntityDefinition);

        const auto& tag = map.smartTag("entity");
        CHECK(tag.matches(*brushNode));

        CHECK(tag.canDisable());

        selectNodes(map, {brushNode});

        auto callback = TestCallback{0};
        tag.disable(callback, map);
        CHECK_FALSE(tag.matches(*brushNode));
      }
    }
  }

  SECTION("undoCommand")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();


    SECTION("Update materials")
    {
      deselectAll(map);
      setEntityProperty(map, EntityPropertyKeys::Wad, "fixture/test/io/Wad/cr8_czg.wad");

      auto* brushNode = createBrushNode(map, "coffin1");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto* material = map.materialManager().material("coffin1");
      REQUIRE(material != nullptr);
      REQUIRE(material->usageCount() == 6u);

      REQUIRE_THAT(
        brushNode->brush().faces(),
        AllMatch(Predicate<const BrushFace&>(
          [&](const auto& face) { return face.material() == material; })));

      SECTION("translateSelection")
      {
        selectNodes(map, {brushNode});
        translateSelection(map, vm::vec3d{1, 1, 1});
        CHECK(material->usageCount() == 6u);

        map.undoCommand();
        CHECK(material->usageCount() == 6u);
      }

      SECTION("removeSelectedNodes")
      {
        selectNodes(map, {brushNode});
        removeSelectedNodes(map);
        CHECK(material->usageCount() == 0u);

        map.undoCommand();
        CHECK(material->usageCount() == 6u);
      }

      SECTION("translateUV")
      {
        auto topFaceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
        REQUIRE(topFaceIndex.has_value());

        selectBrushFaces(map, {{brushNode, *topFaceIndex}});

        REQUIRE(setBrushFaceAttributes(map, {.xOffset = SetValue{12.34f}}));

        map.undoCommand(); // undo move
        CHECK(material->usageCount() == 6u);
        REQUIRE(map.selection().hasBrushFaces());

        map.undoCommand(); // undo select
        CHECK(material->usageCount() == 6u);
        REQUIRE(!map.selection().hasBrushFaces());
      }

      CHECK_THAT(
        brushNode->brush().faces(),
        AllMatch(Predicate<const BrushFace&>(
          [&](const auto& face) { return face.material() == material; })));
    }
  }

  SECTION("canRepeatCommands")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();

    CHECK_FALSE(map.canRepeatCommands());

    auto* entityNode = new EntityNode{Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    CHECK_FALSE(map.canRepeatCommands());

    selectNodes(map, {entityNode});
    CHECK_FALSE(map.canRepeatCommands());

    duplicateSelectedNodes(map);
    CHECK(map.canRepeatCommands());

    map.clearRepeatableCommands();
    CHECK_FALSE(map.canRepeatCommands());
  }

  SECTION("repeatCommands")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();

    SECTION("Repeat translation")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      selectNodes(map, {entityNode});

      REQUIRE_FALSE(map.canRepeatCommands());
      translateSelection(map, {1, 2, 3});
      CHECK(map.canRepeatCommands());

      REQUIRE(entityNode->entity().origin() == vm::vec3d(1, 2, 3));
      map.repeatCommands();
      CHECK(entityNode->entity().origin() == vm::vec3d(2, 4, 6));
    }

    SECTION("Repeat rotation")
    {
      auto entity = Entity();
      entity.transform(vm::translation_matrix(vm::vec3d(1, 2, 3)), true);

      auto* entityNode = new EntityNode(std::move(entity));

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      selectNodes(map, {entityNode});

      REQUIRE_FALSE(map.canRepeatCommands());
      rotateSelection(map, vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::to_radians(90.0));
      CHECK(map.canRepeatCommands());

      REQUIRE(
        entityNode->entity().origin()
        == vm::approx(
          vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(90.0))
          * vm::vec3d(1, 2, 3)));
      map.repeatCommands();
      CHECK(
        entityNode->entity().origin()
        == vm::approx(
          vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(180.0))
          * vm::vec3d(1, 2, 3)));
    }

    SECTION("Scale with bounding box")
    {
      auto* brushNode1 = createBrushNode(map);

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      selectNodes(map, {brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      const auto oldBounds = brushNode1->logicalBounds();
      const auto newBounds = vm::bbox3d(oldBounds.min, 2.0 * oldBounds.max);
      scaleSelection(map, oldBounds, newBounds);
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      selectNodes(map, {brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == newBounds);
    }

    SECTION("Scale with factors")
    {
      auto* brushNode1 = createBrushNode(map);

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      selectNodes(map, {brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      scaleSelection(map, brushNode1->logicalBounds().center(), vm::vec3d(2, 2, 2));
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      deselectAll(map);
      selectNodes(map, {brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
    }

    SECTION("Shear")
    {
      auto* brushNode1 = createBrushNode(map);
      const auto originalBounds = brushNode1->logicalBounds();

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      selectNodes(map, {brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      shearSelection(map, originalBounds, vm::vec3d{0, 0, 1}, vm::vec3d(32, 0, 0));
      REQUIRE(brushNode1->logicalBounds() != originalBounds);
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      deselectAll(map);
      selectNodes(map, {brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
    }

    SECTION("Flip")
    {
      auto* brushNode1 = createBrushNode(map);
      const auto originalBounds = brushNode1->logicalBounds();

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      selectNodes(map, {brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      flipSelection(map, originalBounds.max, vm::axis::z);
      REQUIRE(brushNode1->logicalBounds() != originalBounds);
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      deselectAll(map);
      selectNodes(map, {brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
    }

    SECTION("Duplicate and translate")
    {
      auto* entityNode1 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      selectNodes(map, {entityNode1});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      SECTION("transaction containing a rollback")
      {
        duplicateSelectedNodes(map);

        map.startTransaction("", TransactionScope::Oneshot);
        translateSelection(map, {0, 0, 10});
        map.rollbackTransaction();
        translateSelection(map, {10, 0, 0});
        map.commitTransaction();
      }
      SECTION("translations that get coalesced")
      {
        duplicateSelectedNodes(map);

        translateSelection(map, {5, 0, 0});
        translateSelection(map, {5, 0, 0});
      }
      SECTION("duplicate inside transaction, then standalone movements")
      {
        map.startTransaction("", TransactionScope::Oneshot);
        duplicateSelectedNodes(map);
        translateSelection(map, {2, 0, 0});
        translateSelection(map, {2, 0, 0});
        map.commitTransaction();

        translateSelection(map, {2, 0, 0});
        translateSelection(map, {2, 0, 0});
        translateSelection(map, {2, 0, 0});
      }

      // repeatable actions:
      //  - duplicate
      //  - translate by x = +10

      REQUIRE(map.selection().allEntities().size() == 1);

      auto* entityNode2 = map.selection().allEntities().at(0);
      CHECK(entityNode2 != entityNode1);

      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));
      CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));

      map.repeatCommands();

      REQUIRE(map.selection().allEntities().size() == 1);

      auto* entityNode3 = map.selection().allEntities().at(0);
      CHECK(entityNode3 != entityNode2);

      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));
      CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));
      CHECK(entityNode3->entity().origin() == vm::vec3d(20, 0, 0));
    }

    SECTION("Repeat applies to transactions")
    {
      auto* entityNode1 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      selectNodes(map, {entityNode1});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      map.startTransaction("", TransactionScope::Oneshot);
      translateSelection(map, {0, 0, 10});
      map.rollbackTransaction();
      translateSelection(map, {10, 0, 0});
      map.commitTransaction();
      // overall result: x += 10

      CHECK(entityNode1->entity().origin() == vm::vec3d(10, 0, 0));

      // now repeat the transaction on a second entity

      auto* entityNode2 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode2}}});

      deselectAll(map);
      selectNodes(map, {entityNode2});
      CHECK(entityNode2->entity().origin() == vm::vec3d(0, 0, 0));

      CHECK(map.canRepeatCommands());
      map.repeatCommands();
      CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));

      map.repeatCommands();
      CHECK(entityNode2->entity().origin() == vm::vec3d(20, 0, 0));

      // ensure entityNode1 was unmodified

      CHECK(entityNode1->entity().origin() == vm::vec3d(10, 0, 0));
    }

    SECTION("Undo")
    {
      auto* entityNode1 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      selectNodes(map, {entityNode1});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      translateSelection(map, {0, 0, 10});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 10));
      CHECK(map.canRepeatCommands());

      map.undoCommand();
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      // For now, we won't support repeating a sequence of commands
      // containing undo/redo (it just clears the repeat stack)
      CHECK(!map.canRepeatCommands());
    }
  }

  SECTION("throwExceptionDuringCommand")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();

    CHECK_THROWS_AS(map.throwExceptionDuringCommand(), std::exception);
  }

  SECTION("Entity definition file handling")
  {
    auto taskManager = createTestTaskManager();
    auto logger = NullLogger{};

    SECTION("Add and convert properties")
    {
      using namespace EntityPropertyKeys;

      auto env = fs::TestEnvironment{};
      env.createFile("Quake.fgd", R"(@PointClass = some_entity : "Some Entity" [])");

      auto gameInfo = QuakeGameInfo;
      gameInfo.gameConfig.path = env.dir() / "GameConfig.cfg";
      gameInfo.gameConfig.entityConfig.defFilePaths.emplace_back("Quake.fgd");


      Map::createMap(
        MapFormat::Standard,
        gameInfo,
        gameInfo.gamePathPreference.defaultValue,
        vm::bbox3d{8192.0},
        *taskManager,
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

  SECTION("Duplicate and Copy / Paste behave identically")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();

    enum class Mode
    {
      CopyPaste,
      Duplicate,
    };

    const auto mode = GENERATE(Mode::CopyPaste, Mode::Duplicate);

    const auto duplicateOrCopyPaste = [&]() {
      switch (mode)
      {
      case Mode::CopyPaste:
        REQUIRE(paste(map, serializeSelectedNodes(map)) == PasteType::Node);
        break;
      case Mode::Duplicate:
        duplicateSelectedNodes(map);
        break;
        switchDefault();
      }
    };

    CAPTURE(mode);

    SECTION("Grouped nodes")
    {
      auto* entityNode = new EntityNode{Entity{}};
      auto* brushNode = createBrushNode(map);
      entityNode->addChild(brushNode);

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      selectNodes(map, {entityNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      SECTION("If the group is not linked")
      {
        openGroup(map, *groupNode);

        selectNodes(map, {brushNode});
        duplicateOrCopyPaste();

        const auto* brushNodeCopy = map.selection().brushes.at(0u);
        CHECK(brushNodeCopy->linkId() != brushNode->linkId());

        const auto* entityNodeCopy =
          dynamic_cast<const EntityNode*>(brushNodeCopy->entity());
        REQUIRE(entityNodeCopy != nullptr);
        CHECK(entityNodeCopy->linkId() != entityNode->linkId());
      }

      SECTION("If the group is linked")
      {
        const auto* linkedGroupNode = createLinkedDuplicate(map);
        REQUIRE(linkedGroupNode != nullptr);
        REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

        deselectAll(map);
        selectNodes(map, {groupNode});
        openGroup(map, *groupNode);

        selectNodes(map, {entityNode});
        duplicateOrCopyPaste();

        const auto* brushNodeCopy = map.selection().brushes.at(0u);
        CHECK(brushNodeCopy->linkId() != brushNode->linkId());

        const auto* entityNodeCopy =
          dynamic_cast<const EntityNode*>(brushNodeCopy->entity());
        REQUIRE(entityNodeCopy != nullptr);
        CHECK(entityNodeCopy->linkId() != entityNode->linkId());
      }
    }

    SECTION("Linked group")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

      duplicateOrCopyPaste();

      auto* groupNodeCopy = map.selection().groups.at(0u);
      CHECK(groupNodeCopy->linkId() == groupNode->linkId());
    }

    SECTION("Nodes in a linked group")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

      openGroup(map, *groupNode);

      selectNodes(map, {brushNode});
      duplicateOrCopyPaste();

      auto* brushNodeCopy = map.selection().brushes.at(0u);
      CHECK(brushNodeCopy->linkId() != brushNode->linkId());
    }

    SECTION("Groups in a linked group")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* innerGroupNode = groupSelectedNodes(map, "inner");
      REQUIRE(innerGroupNode != nullptr);

      auto* outerGroupNode = groupSelectedNodes(map, "outer");
      REQUIRE(outerGroupNode != nullptr);

      auto* linkedOuterGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedOuterGroupNode->linkId() == outerGroupNode->linkId());

      const auto linkedInnerGroupNode = getChildAs<GroupNode>(*linkedOuterGroupNode);
      REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      openGroup(map, *outerGroupNode);

      selectNodes(map, {innerGroupNode});
      duplicateOrCopyPaste();

      auto* innerGroupNodeCopy = map.selection().groups.at(0u);
      CHECK(innerGroupNodeCopy->linkId() == innerGroupNode->linkId());
    }

    SECTION("Nested groups")
    {
      auto* innerBrushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {innerBrushNode}}});
      selectNodes(map, {innerBrushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* outerBrushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {outerBrushNode}}});

      deselectAll(map);
      selectNodes(map, {groupNode, outerBrushNode});
      auto* outerGroupNode = groupSelectedNodes(map, "outer");

      deselectAll(map);
      selectNodes(map, {outerGroupNode});

      duplicateOrCopyPaste();

      const auto* outerGroupNodeCopy = map.selection().groups.at(0u);
      const auto [groupNodeCopy, outerBrushNodeCopy] =
        getChildrenAs<GroupNode, BrushNode>(*outerGroupNodeCopy);

      CHECK(groupNodeCopy->linkId() != groupNode->linkId());
      CHECK(outerBrushNodeCopy->linkId() != outerBrushNode->linkId());
    }

    SECTION("Nested linked groups")
    {
      /*
      outerGroupNode  this node is duplicated
        innerGroupNode
          innerBrushNode
        linkedInnerGroupNode
          linkedInnerBrushNode
        outerBrushNode
      */

      auto* innerBrushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {innerBrushNode}}});
      selectNodes(map, {innerBrushNode});

      auto* innerGroupNode = groupSelectedNodes(map, "inner");
      REQUIRE(innerGroupNode != nullptr);

      deselectAll(map);
      selectNodes(map, {innerGroupNode});

      auto* linkedInnerGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      const auto linkedInnerBrushNode = getChildAs<BrushNode>(*linkedInnerGroupNode);

      auto* outerBrushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {outerBrushNode}}});

      deselectAll(map);
      selectNodes(map, {innerGroupNode, linkedInnerGroupNode, outerBrushNode});
      auto* outerGroupNode = groupSelectedNodes(map, "outer");

      deselectAll(map);
      selectNodes(map, {outerGroupNode});

      duplicateOrCopyPaste();

      const auto* outerGroupNodeCopy = map.selection().groups.at(0);
      REQUIRE(outerGroupNodeCopy != nullptr);
      REQUIRE(outerGroupNodeCopy->childCount() == 3u);

      const auto [innerGroupNodeCopy, linkedInnerGroupNodeCopy, outerBrushNodeCopy] =
        getChildrenAs<GroupNode, GroupNode, BrushNode>(*outerGroupNodeCopy);

      const auto innerBrushNodeCopy = getChildAs<BrushNode>(*innerGroupNodeCopy);

      const auto linkedInnerBrushNodeCopy =
        getChildAs<BrushNode>(*linkedInnerGroupNodeCopy);

      CHECK(innerGroupNodeCopy->linkId() == innerGroupNode->linkId());
      CHECK(linkedInnerGroupNodeCopy->linkId() == linkedInnerGroupNode->linkId());
      CHECK(innerBrushNodeCopy->linkId() == innerBrushNode->linkId());
      CHECK(linkedInnerBrushNodeCopy->linkId() == linkedInnerBrushNode->linkId());
      CHECK(outerBrushNodeCopy->linkId() != outerBrushNode->linkId());
    }
  }
}

} // namespace tb::mdl
