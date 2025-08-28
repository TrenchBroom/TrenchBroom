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
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/map_utils.h"

#include <map>
#include <vector>

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Selection")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

  map.setEntityDefinitions({
    {"brush_entity", Color{}, "this is a brush entity", {}},
  });

  const auto& brushEntityDefinition = map.entityDefinitionManager().definitions().back();

  SECTION("selection")
  {
    SECTION("brushFaces")
    {
      auto* brushNode = createBrushNode(map);
      CHECK(brushNode->logicalBounds().center() == vm::vec3d{0, 0, 0});

      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto topFaceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
      REQUIRE(topFaceIndex);

      // select the top face
      map.selectBrushFaces({{brushNode, *topFaceIndex}});
      CHECK_THAT(
        map.selection().brushFaces,
        Catch::Equals(std::vector<mdl::BrushFaceHandle>{{brushNode, *topFaceIndex}}));

      // deselect it
      map.deselectBrushFaces({{brushNode, *topFaceIndex}});
      CHECK_THAT(
        map.selection().brushFaces, Catch::Equals(std::vector<mdl::BrushFaceHandle>{}));

      // select the brush
      map.selectNodes({brushNode});
      CHECK_THAT(
        map.selection().brushes, Catch::Equals(std::vector<mdl::BrushNode*>{brushNode}));

      // translate the brush
      map.translateSelection(vm::vec3d{10.0, 0.0, 0.0});
      CHECK(brushNode->logicalBounds().center() == vm::vec3d{10.0, 0.0, 0.0});

      // Start undoing changes

      map.undoCommand();
      CHECK(brushNode->logicalBounds().center() == vm::vec3d{0, 0, 0});
      CHECK_THAT(
        map.selection().brushes, Catch::Equals(std::vector<mdl::BrushNode*>{brushNode}));
      CHECK_THAT(
        map.selection().brushFaces, Catch::Equals(std::vector<mdl::BrushFaceHandle>{}));

      map.undoCommand();
      CHECK_THAT(map.selection().brushes, Catch::Equals(std::vector<mdl::BrushNode*>{}));
      CHECK_THAT(
        map.selection().brushFaces, Catch::Equals(std::vector<mdl::BrushFaceHandle>{}));

      map.undoCommand();
      CHECK_THAT(
        map.selection().brushFaces,
        Catch::Equals(std::vector<mdl::BrushFaceHandle>{{brushNode, *topFaceIndex}}));
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

        map.deselectAll();

        WHEN("Nothing is selected")
        {
          THEN("The world node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              Catch::Matchers::UnorderedEquals(
                std::vector<EntityNodeBase*>{map.world()}));
          }
        }

        WHEN("A top level brush node is selected")
        {
          map.selectNodes({topLevelBrushNode});

          THEN("The world node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              Catch::Matchers::UnorderedEquals(
                std::vector<EntityNodeBase*>{map.world()}));
          }
        }

        WHEN("A top level patch node is selected")
        {
          map.selectNodes({topLevelPatchNode});

          THEN("The world node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              Catch::Matchers::UnorderedEquals(
                std::vector<EntityNodeBase*>{map.world()}));
          }
        }

        WHEN("An empty group node is selected")
        {
          map.selectNodes({emptyGroupNode});

          THEN("Worldspawn is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              Catch::Matchers::UnorderedEquals(
                std::vector<EntityNodeBase*>{map.world()}));
          }
        }

        WHEN("A group node containing an entity node is selected")
        {
          map.selectNodes({groupNodeWithEntity});

          THEN("The grouped entity node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              Catch::Matchers::UnorderedEquals(
                std::vector<EntityNodeBase*>{groupedEntityNode}));
          }

          AND_WHEN("A top level entity node is selected")
          {
            map.selectNodes({topLevelEntityNode});

            THEN("The top level entity node and the grouped entity node are returned")
            {
              CHECK_THAT(
                map.selection().allEntities(),
                Catch::Matchers::UnorderedEquals(
                  std::vector<EntityNodeBase*>{groupedEntityNode, topLevelEntityNode}));
            }
          }
        }

        WHEN("An empty top level entity node is selected")
        {
          map.selectNodes({topLevelEntityNode});

          THEN("That entity node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              Catch::Matchers::UnorderedEquals(
                std::vector<EntityNodeBase*>{topLevelEntityNode}));
          }
        }

        WHEN("A node in a brush entity node is selected")
        {
          const auto selectBrushNode =
            [](auto* brushNode, auto* patchNode) -> std::tuple<Node*, Node*> {
            return {brushNode, patchNode};
          };
          const auto selectPatchNode =
            [](auto* brushNode, auto* patchNode) -> std::tuple<Node*, Node*> {
            return {patchNode, brushNode};
          };
          const auto selectNodes = GENERATE_COPY(selectBrushNode, selectPatchNode);

          const auto [nodeToSelect, otherNode] =
            selectNodes(brushEntityBrushNode, brushEntityPatchNode);

          CAPTURE(nodeToSelect->name(), otherNode->name());

          map.selectNodes({nodeToSelect});

          THEN("The containing entity node is returned")
          {
            CHECK_THAT(
              map.selection().allEntities(),
              Catch::Matchers::UnorderedEquals(
                std::vector<EntityNodeBase*>{topLevelBrushEntityNode}));
          }

          AND_WHEN("Another node in the same entity node is selected")
          {
            map.selectNodes({otherNode});

            THEN("The containing entity node is returned only once")
            {
              CHECK_THAT(
                map.selection().allEntities(),
                Catch::Matchers::UnorderedEquals(
                  std::vector<EntityNodeBase*>{topLevelBrushEntityNode}));
            }
          }

          AND_WHEN("A top level entity node is selected")
          {
            map.selectNodes({topLevelEntityNode});

            THEN("The top level entity node and the brush entity node are returned")
            {
              CHECK_THAT(
                map.selection().allEntities(),
                Catch::Matchers::UnorderedEquals(std::vector<EntityNodeBase*>{
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
        {{map.world()->defaultLayer(),
          {brushNodeInDefaultLayer, brushEntityNode, pointEntityNode, outerGroupNode}},
         {map.world(), {customLayerNode}}});

      addNodes(
        map,
        {
          {customLayerNode, {brushNodeInCustomLayer}},
          {outerGroupNode, {innerGroupNode, brushNodeInGroup}},
          {brushEntityNode, {brushNodeInEntity}},
        });

      addNodes(map, {{innerGroupNode, {brushNodeInNestedGroup}}});

      const auto getPath = [&](const Node* node) { return node->pathFrom(*map.world()); };
      const auto resolvePaths = [&](const auto& paths) {
        return paths | std::views::transform([&](const auto& path) {
                 return map.world()->resolvePath(path);
               })
               | kdl::to_vector;
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

      map.selectNodes(nodes);

      CHECK_THAT(
        map.selection().allBrushes(), Catch::Matchers::UnorderedEquals(brushNodes));
    }
  }

  SECTION("selectNodes")
  {
    SECTION("Linked groups")
    {
      auto* entityNode = new EntityNode{Entity{}};
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode, entityNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      SECTION("Cannot select linked groups if selection is empty")
      {
        map.deselectAll();
        CHECK_FALSE(map.canSelectLinkedGroups());
      }

      SECTION("Cannot select linked groups if selection contains non-groups")
      {
        map.deselectAll();
        map.selectNodes({entityNode});
        CHECK_FALSE(map.canSelectLinkedGroups());
        map.selectNodes({groupNode});
        CHECK_FALSE(map.canSelectLinkedGroups());
      }

      SECTION("Cannot select linked groups if selection contains unlinked groups")
      {
        map.deselectAll();
        map.selectNodes({entityNode});

        auto* unlinkedGroupNode = map.groupSelectedNodes("other");
        REQUIRE(unlinkedGroupNode != nullptr);

        CHECK_FALSE(map.canSelectLinkedGroups());

        map.selectNodes({groupNode});
        CHECK_FALSE(map.canSelectLinkedGroups());
      }

      SECTION("Select linked groups")
      {
        auto* linkedGroupNode = map.createLinkedDuplicate();
        REQUIRE(linkedGroupNode != nullptr);

        map.deselectAll();
        map.selectNodes({groupNode});

        REQUIRE(map.canSelectLinkedGroups());
        map.selectLinkedGroups();
        CHECK_THAT(
          map.selection().nodes,
          Catch::UnorderedEquals(std::vector<Node*>{groupNode, linkedGroupNode}));
      }
    }
  }

  SECTION("selectSiblingNodes")
  {
    const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};
    auto* brushNode1 =
      new BrushNode{builder.createCuboid(box, "material") | kdl::value()};
    auto* brushNode2 = new BrushNode{
      builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()};
    auto* brushNode3 = new BrushNode{
      builder.createCuboid(box.translate({2, 2, 2}), "material") | kdl::value()};
    auto* patchNode = createPatchNode();

    addNodes(
      map, {{parentForNodes(map), {brushNode1, brushNode2, brushNode3, patchNode}}});

    map.selectNodes({brushNode1, brushNode2});
    map.createBrushEntity(brushEntityDefinition);

    map.deselectAll();

    // worldspawn {
    //   brushEnt { brush1, brush2 },
    //   brush3
    //   patch
    // }

    SECTION("Brush in default layer")
    {
      map.selectNodes({brushNode3});
      REQUIRE_THAT(
        map.selection().nodes, Catch::UnorderedEquals(std::vector<Node*>{brushNode3}));

      map.selectSiblingNodes();
      CHECK_THAT(
        map.selection().nodes,
        Catch::UnorderedEquals(
          std::vector<Node*>{brushNode1, brushNode2, brushNode3, patchNode}));

      map.undoCommand();
      CHECK_THAT(
        map.selection().nodes, Catch::UnorderedEquals(std::vector<Node*>{brushNode3}));
    }

    SECTION("Brush in brush entity")
    {
      map.selectNodes({brushNode1});
      REQUIRE_THAT(
        map.selection().nodes, Catch::UnorderedEquals(std::vector<Node*>{brushNode1}));

      map.selectSiblingNodes();
      CHECK_THAT(
        map.selection().nodes,
        Catch::UnorderedEquals(std::vector<Node*>{brushNode1, brushNode2}));

      map.undoCommand();
      CHECK_THAT(
        map.selection().nodes, Catch::UnorderedEquals(std::vector<Node*>{brushNode1}));
    }
  }

  SECTION("selectTouchingNodes")
  {
    SECTION("Select touching brushes")
    {
      auto* brushNode1 = new BrushNode{builder.createCube(64.0, "none") | kdl::value()};
      auto* brushNode2 = new BrushNode{builder.createCube(64.0, "none") | kdl::value()};
      auto* brushNode3 = new BrushNode{builder.createCube(64.0, "none") | kdl::value()};

      transformNode(
        *brushNode2,
        vm::translation_matrix(vm::vec3d{10.0, 0.0, 0.0}),
        map.worldBounds());
      transformNode(
        *brushNode3,
        vm::translation_matrix(vm::vec3d{100.0, 0.0, 0.0}),
        map.worldBounds());

      addNodes(map, {{parentForNodes(map), {brushNode1, brushNode2, brushNode3}}});

      REQUIRE(brushNode1->intersects(brushNode2));
      REQUIRE(brushNode2->intersects(brushNode1));

      REQUIRE(!brushNode1->intersects(brushNode3));
      REQUIRE(!brushNode3->intersects(brushNode1));

      map.selectNodes({brushNode1});
      map.selectTouchingNodes(false);

      using Catch::Matchers::UnorderedEquals;
      CHECK_THAT(
        map.selection().brushes, UnorderedEquals(std::vector<BrushNode*>{brushNode2}));
    }

    SECTION("Select touching group")
    {
      auto* layerNode = new LayerNode{Layer{"Layer 1"}};
      addNodes(map, {{map.world(), {layerNode}}});

      auto* groupNode = new GroupNode{Group{"Unnamed"}};
      addNodes(map, {{layerNode, {groupNode}}});

      const auto brushBounds = vm::bbox3d{{-32.0, -32.0, -32.0}, {+32.0, +32.0, +32.0}};
      auto* brushNode =
        new BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
      addNodes(map, {{groupNode, {brushNode}}});

      const auto selectionBounds =
        vm::bbox3d{{-16.0, -16.0, -48.0}, {+16.0, +16.0, +48.0}};

      auto* selectionBrush =
        new BrushNode{builder.createCuboid(selectionBounds, "material") | kdl::value()};
      addNodes(map, {{layerNode, {selectionBrush}}});

      map.selectNodes({selectionBrush});
      map.selectTouchingNodes(true);

      CHECK(map.selection().nodes == std::vector<Node*>{groupNode});
    }

    SECTION("Don't crash when input brushes overlap")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/2476
      const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

      auto* brushNode1 =
        new BrushNode{builder.createCuboid(box, "material") | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode1}}});

      auto* brushNode2 = new BrushNode{
        builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()};
      addNodes(map, {{parentForNodes(map), {brushNode2}}});

      map.selectAllNodes();

      CHECK_THAT(
        map.selection().brushes,
        Catch::UnorderedEquals(std::vector<BrushNode*>{brushNode1, brushNode2}));
      CHECK_THAT(
        map.currentLayer()->children(),
        Catch::Equals(std::vector<Node*>{brushNode1, brushNode2}));

      map.selectTouchingNodes(true);

      // only this next line was failing
      CHECK_THAT(
        map.selection().brushes, Catch::UnorderedEquals(std::vector<BrushNode*>{}));
      CHECK_THAT(map.currentLayer()->children(), Catch::Equals(std::vector<Node*>{}));

      // brush1 and brush2 are deleted
      CHECK(brushNode1->parent() == nullptr);
      CHECK(brushNode2->parent() == nullptr);
    }

    SECTION("Select touching nodes inside nested group")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3826

      auto* brushNode1 = createBrushNode(map);
      auto* brushNode2 = createBrushNode(map);

      auto* outerGroup = new GroupNode{Group{"outerGroup"}};
      auto* innerGroup = new GroupNode{Group{"innerGroup"}};

      addNodes(map, {{parentForNodes(map), {outerGroup}}});
      addNodes(map, {{outerGroup, {innerGroup}}});
      addNodes(map, {{innerGroup, {brushNode1, brushNode2}}});

      // worldspawn {
      //   outerGroup {
      //     innerGroup { brush1, brush2 }
      //   }
      // }

      outerGroup->open();
      innerGroup->open();
      map.selectNodes({brushNode1});

      map.selectTouchingNodes(false);

      CHECK_THAT(
        map.selection().brushes,
        Catch::UnorderedEquals(std::vector<BrushNode*>{brushNode2}));
    }

    SECTION("Select touching nodes along axis")
    {
      using Catch::Matchers::UnorderedEquals;

      auto* brushNode1 = new BrushNode{builder.createCube(64.0, "none") | kdl::value()};
      auto* brushNode2 = new BrushNode{builder.createCube(64.0, "none") | kdl::value()};
      auto* brushNode3 = new BrushNode{builder.createCube(64.0, "none") | kdl::value()};

      transformNode(
        *brushNode2,
        vm::translation_matrix(vm::vec3d{0.0, 0.0, -500.0}),
        map.worldBounds());
      transformNode(
        *brushNode3,
        vm::translation_matrix(vm::vec3d{100.0, 0.0, 0.0}),
        map.worldBounds());

      REQUIRE(!brushNode1->intersects(brushNode2));
      REQUIRE(!brushNode1->intersects(brushNode3));

      addNodes(map, {{parentForNodes(map), {brushNode1, brushNode2, brushNode3}}});
      map.selectNodes({brushNode1});

      SECTION("z camera")
      {
        map.selectTouchingNodes(vm::axis::z, true);

        CHECK_THAT(
          map.selection().brushes, UnorderedEquals(std::vector<BrushNode*>{brushNode2}));
      }
      SECTION("x camera")
      {
        map.selectTouchingNodes(vm::axis::x, true);

        CHECK_THAT(
          map.selection().brushes, UnorderedEquals(std::vector<BrushNode*>{brushNode3}));
      }
    }
  }

  SECTION("selectContainedNodes")
  {
    SECTION("Select contained group")
    {
      auto* layerNode = new LayerNode{Layer{"Layer 1"}};
      addNodes(map, {{map.world(), {layerNode}}});

      auto* groupNode = new GroupNode{Group{"Unnamed"}};
      addNodes(map, {{layerNode, {groupNode}}});

      const auto brushBounds = vm::bbox3d{{-32.0, -32.0, -32.0}, {+32.0, +32.0, +32.0}};
      auto* brushNode =
        new BrushNode{builder.createCuboid(brushBounds, "material") | kdl::value()};
      addNodes(map, {{groupNode, {brushNode}}});

      const auto selectionBounds =
        vm::bbox3d{{-48.0, -48.0, -48.0}, {+48.0, +48.0, +48.0}};

      auto* selectionBrush =
        new BrushNode{builder.createCuboid(selectionBounds, "material") | kdl::value()};
      addNodes(map, {{layerNode, {selectionBrush}}});

      map.selectNodes({selectionBrush});
      map.selectContainedNodes(true);

      CHECK(map.selection().nodes == std::vector<Node*>{groupNode});
    }
  }

  SECTION("selectNodesWithFilePosition")
  {
    /*
    - defaultLayer
      - brush                    4,  5
      - pointEntity             10, 15
      - patch                   16, 20
      - brushEntity             20, 30
        - brushInEntity1        23, 25
        - brushInEntity2        26, 29
      - outerGroup              31, 50
        - brushInOuterGroup     32, 38
        - innerGroup            39, 49
          - brushInInnerGroup   43, 48
    */

    auto* brush = createBrushNode(map, "brush");
    auto* pointEntity = new EntityNode{Entity{}};
    auto* patch = createPatchNode("patch");

    auto* brushEntity = new EntityNode{Entity{}};
    auto* brushInEntity1 = createBrushNode(map, "brushInEntity1");
    auto* brushInEntity2 = createBrushNode(map, "brushInEntity2");

    auto* outerGroup = new GroupNode{Group{"outerGroup"}};
    auto* brushInOuterGroup = createBrushNode(map, "brushInOuterGroup");
    auto* innerGroup = new GroupNode{Group{"innerGroup"}};
    auto* brushInInnerGroup = createBrushNode(map, "brushInInnerGroup");

    brush->setFilePosition(4, 2);
    pointEntity->setFilePosition(10, 5);
    patch->setFilePosition(16, 4);
    brushEntity->setFilePosition(20, 10);
    brushInEntity1->setFilePosition(23, 2);
    brushInEntity2->setFilePosition(26, 3);
    outerGroup->setFilePosition(31, 19);
    brushInOuterGroup->setFilePosition(32, 6);
    innerGroup->setFilePosition(39, 10);
    brushInInnerGroup->setFilePosition(43, 5);

    const auto nodeToName = std::map<const Node*, std::string>{
      {brush, "brush"},
      {pointEntity, "pointEntity"},
      {patch, "patch"},
      {brushEntity, "brushEntity"},
      {brushInEntity1, "brushInEntity1"},
      {brushInEntity2, "brushInEntity2"},
      {outerGroup, "outerGroup"},
      {brushInOuterGroup, "brushInOuterGroup"},
      {innerGroup, "innerGroup"},
      {brushInInnerGroup, "brushInInnerGroup"},
    };

    const auto mapNodeNames = [&](const auto& nodes) {
      return kdl::vec_transform(nodes, [&](const Node* node) {
        return kdl::map_find_or_default(nodeToName, node, std::string{"<unknown>"});
      });
    };

    addNodes(
      map,
      {
        {map.world()->defaultLayer(),
         {brush, pointEntity, patch, brushEntity, outerGroup}},
      });

    addNodes(
      map,
      {
        {brushEntity, {brushInEntity1, brushInEntity2}},
        {outerGroup, {brushInOuterGroup, innerGroup}},
      });

    addNodes(map, {{innerGroup, {brushInInnerGroup}}});

    map.deselectAll();

    using T = std::tuple<std::vector<size_t>, std::vector<std::string>>;

    SECTION("outer group is closed")
    {
      const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
        {{0}, {}},
        {{4}, {"brush"}},
        {{5}, {"brush"}},
        {{4, 5}, {"brush"}},
        {{6}, {}},
        {{7}, {}},
        {{12}, {"pointEntity"}},
        {{16}, {"patch"}},
        {{20}, {"brushInEntity1", "brushInEntity2"}},
        {{24}, {"brushInEntity1"}},
        {{26}, {"brushInEntity2"}},
        {{31}, {"outerGroup"}},
        {{32}, {"outerGroup"}},
        {{39}, {"outerGroup"}},
        {{43}, {"outerGroup"}},
        {{0, 4, 12, 24, 32}, {"brush", "pointEntity", "brushInEntity1", "outerGroup"}},
      }));

      CAPTURE(lineNumbers);

      map.selectNodesWithFilePosition(lineNumbers);
      CHECK_THAT(
        mapNodeNames(map.selection().nodes),
        Catch::Matchers::UnorderedEquals(expectedNodeNames));
    }

    SECTION("outer group is open")
    {
      map.openGroup(outerGroup);

      const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
        {{31}, {}},
        {{32}, {"brushInOuterGroup"}},
        {{39}, {"innerGroup"}},
        {{43}, {"innerGroup"}},
      }));

      CAPTURE(lineNumbers);

      map.selectNodesWithFilePosition(lineNumbers);
      CHECK_THAT(
        mapNodeNames(map.selection().nodes),
        Catch::Matchers::UnorderedEquals(expectedNodeNames));
    }

    SECTION("inner group is open")
    {
      map.openGroup(outerGroup);
      map.openGroup(innerGroup);

      const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
        {{31}, {}},
        {{32}, {}},
        {{39}, {}},
        {{43}, {"brushInInnerGroup"}},
      }));

      CAPTURE(lineNumbers);

      map.selectNodesWithFilePosition(lineNumbers);
      CHECK_THAT(
        mapNodeNames(map.selection().nodes),
        Catch::Matchers::UnorderedEquals(expectedNodeNames));
    }
  }

  SECTION("invertNodeSelection")
  {
    const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

    auto* brushNode1 =
      new BrushNode{builder.createCuboid(box, "material") | kdl::value()};
    addNodes(map, {{parentForNodes(map), {brushNode1}}});

    auto* brushNode2 = new BrushNode{
      builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()};
    addNodes(map, {{parentForNodes(map), {brushNode2}}});

    auto* brushNode3 = new BrushNode{
      builder.createCuboid(box.translate({2, 2, 2}), "material") | kdl::value()};
    addNodes(map, {{parentForNodes(map), {brushNode3}}});

    auto* patchNode = createPatchNode();
    addNodes(map, {{parentForNodes(map), {patchNode}}});

    map.selectNodes({brushNode1, brushNode2});
    auto* brushEnt = map.createBrushEntity(brushEntityDefinition);

    map.deselectAll();

    // worldspawn {
    //   brushEnt { brush1, brush2 },
    //   brush3
    //   patch
    // }

    map.selectNodes({brushNode1});
    REQUIRE(brushNode1->selected());
    REQUIRE(!brushNode2->selected());
    REQUIRE(!brushNode3->selected());
    REQUIRE(!brushEnt->selected());
    REQUIRE(!patchNode->selected());

    map.invertNodeSelection();

    CHECK_THAT(
      map.selection().nodes,
      Catch::UnorderedEquals(std::vector<Node*>{brushNode2, brushNode3, patchNode}));
    CHECK(!brushNode1->selected());
    CHECK(brushNode2->selected());
    CHECK(brushNode3->selected());
    CHECK(!brushEnt->selected());
    CHECK(patchNode->selected());
  }

  SECTION("selectBrushFaces")
  {
    SECTION("Linked groups")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedGroupNode != nullptr);

      map.deselectAll();

      SECTION("Face selection locks other groups in link set")
      {
        CHECK(!linkedGroupNode->locked());

        map.selectBrushFaces({{brushNode, 0}});
        CHECK(linkedGroupNode->locked());

        map.deselectAll();
        CHECK(!linkedGroupNode->locked());
      }
    }
  }

  SECTION("Selection clears repeat stack")
  {
    auto* entityNode1 = new EntityNode{Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode1}}});

    auto* entityNode2 = new EntityNode{Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode2}}});

    map.selectNodes({entityNode1});

    REQUIRE_FALSE(map.canRepeatCommands());
    map.translateSelection({1, 2, 3});
    REQUIRE(map.canRepeatCommands());

    map.deselectAll();
    map.selectNodes({entityNode2});
    CHECK(map.canRepeatCommands());

    // this command will not clear the repeat stack
    map.setEntityProperty("this", "that");
    CHECK(map.canRepeatCommands());

    // this command will replace the command on the repeat stack
    map.translateSelection({-1, -2, -3});
    CHECK(map.canRepeatCommands());

    map.deselectAll();
    map.selectNodes({entityNode1});

    map.repeatCommands();
    CHECK(entityNode1->entity().origin() == vm::vec3d{0, 0, 0});

    map.deselectAll();
    map.selectNodes({entityNode1});
    CHECK(map.canRepeatCommands());
  }

  SECTION("lastSelectionBounds")
  {
    auto* entityNode = new EntityNode{Entity{{{"classname", "point_entity"}}}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    REQUIRE(!entityNode->logicalBounds().is_empty());

    map.selectAllNodes();

    auto bounds = map.selectionBounds();
    map.deselectAll();
    CHECK(map.lastSelectionBounds() == bounds);

    map.deselectAll();
    CHECK(map.lastSelectionBounds() == bounds);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    map.selectNodes({brushNode});
    CHECK(map.lastSelectionBounds() == bounds);

    bounds = brushNode->logicalBounds();

    map.deselectAll();
    CHECK(map.lastSelectionBounds() == bounds);
  }
}

} // namespace tb::mdl
