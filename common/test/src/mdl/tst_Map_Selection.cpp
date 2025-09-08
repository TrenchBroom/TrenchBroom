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
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/map_utils.h"

#include <map>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{

TEST_CASE("Map_Selection")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

  map.entityDefinitionManager().setDefinitions({
    {"brush_entity", Color{}, "this is a brush entity", {}},
  });

  const auto& brushEntityDefinition = map.entityDefinitionManager().definitions().back();

  SECTION("selectNodes")
  {
    SECTION("Linked groups")
    {
      auto* entityNode = new EntityNode{Entity{}};
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode, entityNode}}});
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      SECTION("Cannot select linked groups if selection is empty")
      {
        deselectAll(map);
        CHECK_FALSE(canSelectLinkedGroups(map));
      }

      SECTION("Cannot select linked groups if selection contains non-groups")
      {
        deselectAll(map);
        selectNodes(map, {entityNode});
        CHECK_FALSE(canSelectLinkedGroups(map));
        selectNodes(map, {groupNode});
        CHECK_FALSE(canSelectLinkedGroups(map));
      }

      SECTION("Cannot select linked groups if selection contains unlinked groups")
      {
        deselectAll(map);
        selectNodes(map, {entityNode});

        auto* unlinkedGroupNode = groupSelectedNodes(map, "other");
        REQUIRE(unlinkedGroupNode != nullptr);

        CHECK_FALSE(canSelectLinkedGroups(map));

        selectNodes(map, {groupNode});
        CHECK_FALSE(canSelectLinkedGroups(map));
      }

      SECTION("Select linked groups")
      {
        auto* linkedGroupNode = createLinkedDuplicate(map);
        REQUIRE(linkedGroupNode != nullptr);

        deselectAll(map);
        selectNodes(map, {groupNode});

        REQUIRE(canSelectLinkedGroups(map));
        selectLinkedGroups(map);
        CHECK_THAT(
          map.selection().nodes,
          Catch::Matchers::UnorderedEquals(
            std::vector<Node*>{groupNode, linkedGroupNode}));
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

    selectNodes(map, {brushNode1, brushNode2});
    createBrushEntity(map, brushEntityDefinition);

    deselectAll(map);

    // worldspawn {
    //   brushEnt { brush1, brush2 },
    //   brush3
    //   patch
    // }

    SECTION("Brush in default layer")
    {
      selectNodes(map, {brushNode3});
      REQUIRE_THAT(
        map.selection().nodes,
        Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode3}));

      selectSiblingNodes(map);
      CHECK_THAT(
        map.selection().nodes,
        Catch::Matchers::UnorderedEquals(
          std::vector<Node*>{brushNode1, brushNode2, brushNode3, patchNode}));

      map.undoCommand();
      CHECK_THAT(
        map.selection().nodes,
        Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode3}));
    }

    SECTION("Brush in brush entity")
    {
      selectNodes(map, {brushNode1});
      REQUIRE_THAT(
        map.selection().nodes,
        Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode1}));

      selectSiblingNodes(map);
      CHECK_THAT(
        map.selection().nodes,
        Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode1, brushNode2}));

      map.undoCommand();
      CHECK_THAT(
        map.selection().nodes,
        Catch::Matchers::UnorderedEquals(std::vector<Node*>{brushNode1}));
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

      selectNodes(map, {brushNode1});
      selectTouchingNodes(map, false);

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

      selectNodes(map, {selectionBrush});
      selectTouchingNodes(map, true);

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

      selectAllNodes(map);

      CHECK_THAT(
        map.selection().brushes,
        Catch::Matchers::UnorderedEquals(
          std::vector<BrushNode*>{brushNode1, brushNode2}));
      CHECK_THAT(
        map.editorContext().currentLayer()->children(),
        Catch::Matchers::Equals(std::vector<Node*>{brushNode1, brushNode2}));

      selectTouchingNodes(map, true);

      // only this next line was failing
      CHECK_THAT(
        map.selection().brushes,
        Catch::Matchers::UnorderedEquals(std::vector<BrushNode*>{}));
      CHECK_THAT(
        map.editorContext().currentLayer()->children(),
        Catch::Matchers::Equals(std::vector<Node*>{}));

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
      selectNodes(map, {brushNode1});

      selectTouchingNodes(map, false);

      CHECK_THAT(
        map.selection().brushes,
        Catch::Matchers::UnorderedEquals(std::vector<BrushNode*>{brushNode2}));
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
      selectNodes(map, {brushNode1});

      SECTION("z camera")
      {
        selectTouchingNodes(map, vm::axis::z, true);

        CHECK_THAT(
          map.selection().brushes, UnorderedEquals(std::vector<BrushNode*>{brushNode2}));
      }
      SECTION("x camera")
      {
        selectTouchingNodes(map, vm::axis::x, true);

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

      selectNodes(map, {selectionBrush});
      selectContainedNodes(map, true);

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

    deselectAll(map);

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

      selectNodesWithFilePosition(map, lineNumbers);
      CHECK_THAT(
        mapNodeNames(map.selection().nodes),
        Catch::Matchers::UnorderedEquals(expectedNodeNames));
    }

    SECTION("outer group is open")
    {
      openGroup(map, outerGroup);

      const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
        {{31}, {}},
        {{32}, {"brushInOuterGroup"}},
        {{39}, {"innerGroup"}},
        {{43}, {"innerGroup"}},
      }));

      CAPTURE(lineNumbers);

      selectNodesWithFilePosition(map, lineNumbers);
      CHECK_THAT(
        mapNodeNames(map.selection().nodes),
        Catch::Matchers::UnorderedEquals(expectedNodeNames));
    }

    SECTION("inner group is open")
    {
      openGroup(map, outerGroup);
      openGroup(map, innerGroup);

      const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
        {{31}, {}},
        {{32}, {}},
        {{39}, {}},
        {{43}, {"brushInInnerGroup"}},
      }));

      CAPTURE(lineNumbers);

      selectNodesWithFilePosition(map, lineNumbers);
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

    selectNodes(map, {brushNode1, brushNode2});
    auto* brushEnt = createBrushEntity(map, brushEntityDefinition);

    deselectAll(map);

    // worldspawn {
    //   brushEnt { brush1, brush2 },
    //   brush3
    //   patch
    // }

    selectNodes(map, {brushNode1});
    REQUIRE(brushNode1->selected());
    REQUIRE(!brushNode2->selected());
    REQUIRE(!brushNode3->selected());
    REQUIRE(!brushEnt->selected());
    REQUIRE(!patchNode->selected());

    invertNodeSelection(map);

    CHECK_THAT(
      map.selection().nodes,
      Catch::Matchers::UnorderedEquals(
        std::vector<Node*>{brushNode2, brushNode3, patchNode}));
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
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode != nullptr);

      deselectAll(map);

      SECTION("Face selection locks other groups in link set")
      {
        CHECK(!linkedGroupNode->locked());

        selectBrushFaces(map, {{brushNode, 0}});
        CHECK(linkedGroupNode->locked());

        deselectAll(map);
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

    selectNodes(map, {entityNode1});

    REQUIRE_FALSE(map.canRepeatCommands());
    translateSelection(map, {1, 2, 3});
    REQUIRE(map.canRepeatCommands());

    deselectAll(map);
    selectNodes(map, {entityNode2});
    CHECK(map.canRepeatCommands());

    // this command will not clear the repeat stack
    setEntityProperty(map, "this", "that");
    CHECK(map.canRepeatCommands());

    // this command will replace the command on the repeat stack
    translateSelection(map, {-1, -2, -3});
    CHECK(map.canRepeatCommands());

    deselectAll(map);
    selectNodes(map, {entityNode1});

    map.repeatCommands();
    CHECK(entityNode1->entity().origin() == vm::vec3d{0, 0, 0});

    deselectAll(map);
    selectNodes(map, {entityNode1});
    CHECK(map.canRepeatCommands());
  }

  SECTION("lastSelectionBounds")
  {
    auto* entityNode = new EntityNode{Entity{{{"classname", "point_entity"}}}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    REQUIRE(!entityNode->logicalBounds().is_empty());

    selectAllNodes(map);

    auto bounds = map.selectionBounds();
    deselectAll(map);
    CHECK(map.lastSelectionBounds() == bounds);

    deselectAll(map);
    CHECK(map.lastSelectionBounds() == bounds);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    selectNodes(map, {brushNode});
    CHECK(map.lastSelectionBounds() == bounds);

    bounds = brushNode->logicalBounds();

    deselectAll(map);
    CHECK(map.lastSelectionBounds() == bounds);
  }
}

} // namespace tb::mdl
