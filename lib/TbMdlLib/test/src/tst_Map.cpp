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

#include "TestEnvironment.h"
#include "TestLogger.h"
#include "base/Logger.h"
#include "fs/TestEnvironment.h"
#include "gl/ResourceManager.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityModelManager.h"
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
#include "mdl/Matchers.h"
#include "mdl/PasteType.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"

#include "kd/vector_utils.h"

#include "vm/vec_io.h" // IWYU pragma: keep

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

} // namespace

TEST_CASE("Map")
{
  auto environmentConfig = EnvironmentConfig{};

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

  SECTION("setGamePath reloads model shaders")
  {
    auto taskManager = createTestTaskManager();
    auto resourceManager = gl::ResourceManager{};
    auto logger = TestLogger{};

    auto gameInfo = DefaultGameInfo;
    gameInfo.gameConfig.materialConfig.shaderSearchPath = "scripts";
    gameInfo.gamePathPreference.defaultValue =
      getFixtureRoot() / "test/mdl/LoadMd3Model/armor";

    Map::createMap(
      environmentConfig,
      gameInfo,
      gameInfo.gamePathPreference.defaultValue,
      MapFormat::Standard,
      vm::bbox3d{8192.0},
      *taskManager,
      resourceManager,
      logger)
      | kdl::transform([&](auto map) {
          const auto warnCountBefore = logger.countMessages(LogLevel::Warn);

          map->setGamePath(
            getFixtureRoot()
            / "test/mdl/LoadMaterialCollections/shaders/malformed_shader");

          CHECK(logger.countMessages(LogLevel::Warn) > warnCountBefore);
        })
      | kdl::transform_error([](const auto& e) { FAIL(e.msg); });
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
