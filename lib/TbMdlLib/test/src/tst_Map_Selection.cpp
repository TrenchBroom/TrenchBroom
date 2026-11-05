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

#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"

#include "kd/map_utils.h"
#include "kd/ranges/concat_view.h"
#include "kd/ranges/to.h"
#include "kd/vector_utils.h"

#include <map>
#include <memory>
#include <ranges>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_Selection")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  auto builder = BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};

  map.entityDefinitionManager().setDefinitions({
    {"brush_entity", Color{}, "this is a brush entity", {}},
  });

  const auto& brushEntityDefinition = map.entityDefinitionManager().definitions().back();

  SECTION("selectAllNodes")
  {
    auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* entityNode = entityNodePtr.get();
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto patchNodePtr = createPatchNode();
    auto* patchNode = patchNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(entityNodePtr),
        std::move(brushNodePtr),
        std::move(groupNodePtr),
        std::move(patchNodePtr),
        std::move(brushEntityNodePtr)));
    auto* groupedEntityNode =
      addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));
    auto* entityBrushNode = addNode(map, *brushEntityNode, createBrushNode(map));

    SECTION("No group is opened")
    {
      selectAllNodes(map);

      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{
          entityNode, brushNode, groupNode, patchNode, entityBrushNode}));

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(map.selection().nodes == std::vector<Node*>{});

        map.redoCommand();
        CHECK_THAT(
          map.selection().nodes,
          UnorderedEquals(std::vector<Node*>{
            entityNode, brushNode, groupNode, patchNode, entityBrushNode}));
      }
    }

    SECTION("With open group")
    {
      openGroup(map, *groupNode);

      selectAllNodes(map);

      CHECK(map.selection().nodes == std::vector<Node*>{groupedEntityNode});
    }
  }

  SECTION("selectNodes")
  {
    SECTION("Linked groups")
    {
      auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* entityNode = entityNodePtr.get();
      auto brushNodePtr = createBrushNode(map);
      auto* brushNode = brushNodePtr.get();
      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNodePtr), std::move(entityNodePtr)));
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      SECTION("Cannot select linked groups if selection is empty")
      {
        deselectAll(map);
        CHECK(!canSelectLinkedGroups(map));
      }

      SECTION("Cannot select linked groups if selection contains non-groups")
      {
        deselectAll(map);
        selectNodes(map, {entityNode});
        CHECK(!canSelectLinkedGroups(map));
        selectNodes(map, {groupNode});
        CHECK(!canSelectLinkedGroups(map));
      }

      SECTION("Cannot select linked groups if selection contains unlinked groups")
      {
        deselectAll(map);
        selectNodes(map, {entityNode});

        auto* unlinkedGroupNode = groupSelectedNodes(map, "other");
        REQUIRE(unlinkedGroupNode != nullptr);

        CHECK(!canSelectLinkedGroups(map));

        selectNodes(map, {groupNode});
        CHECK(!canSelectLinkedGroups(map));
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
          UnorderedEquals(std::vector<Node*>{groupNode, linkedGroupNode}));
      }
    }

    SECTION("A non-selectable node kind is not recorded as selected")
    {
      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer"}));

      auto* entityNode =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      selectNodes(map, {layerNode, entityNode});

      CHECK(!layerNode->selected());
      CHECK(map.selection().nodes == std::vector<Node*>{entityNode});

      deselectAll(map);
      CHECK(map.selection().nodes == std::vector<Node*>{});
    }
  }

  SECTION("selectSiblingNodes")
  {
    const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};
    auto brushNode1Ptr =
      std::make_unique<BrushNode>(builder.createCuboid(box, "material") | kdl::value());
    auto* brushNode1 = brushNode1Ptr.get();
    auto brushNode2Ptr = std::make_unique<BrushNode>(
      builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value());
    auto* brushNode2 = brushNode2Ptr.get();
    auto brushNode3Ptr = std::make_unique<BrushNode>(
      builder.createCuboid(box.translate({2, 2, 2}), "material") | kdl::value());
    auto* brushNode3 = brushNode3Ptr.get();
    auto patchNodePtr = createPatchNode();
    auto* patchNode = patchNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNode1Ptr),
        std::move(brushNode2Ptr),
        std::move(brushNode3Ptr),
        std::move(patchNodePtr)));

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
        map.selection().nodes, UnorderedEquals(std::vector<Node*>{brushNode3}));

      selectSiblingNodes(map);
      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(
          std::vector<Node*>{brushNode1, brushNode2, brushNode3, patchNode}));

      map.undoCommand();
      CHECK_THAT(map.selection().nodes, UnorderedEquals(std::vector<Node*>{brushNode3}));
    }

    SECTION("Brush in brush entity")
    {
      selectNodes(map, {brushNode1});
      REQUIRE_THAT(
        map.selection().nodes, UnorderedEquals(std::vector<Node*>{brushNode1}));

      selectSiblingNodes(map);
      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{brushNode1, brushNode2}));

      map.undoCommand();
      CHECK_THAT(map.selection().nodes, UnorderedEquals(std::vector<Node*>{brushNode1}));
    }
  }

  SECTION("selectTouchingNodes")
  {
    SECTION("Select touching brushes")
    {
      auto brushNode1Ptr =
        std::make_unique<BrushNode>(builder.createCube(64.0, "none") | kdl::value());
      auto* brushNode1 = brushNode1Ptr.get();
      auto brushNode2Ptr =
        std::make_unique<BrushNode>(builder.createCube(64.0, "none") | kdl::value());
      auto* brushNode2 = brushNode2Ptr.get();
      auto brushNode3Ptr =
        std::make_unique<BrushNode>(builder.createCube(64.0, "none") | kdl::value());
      auto* brushNode3 = brushNode3Ptr.get();

      transformNode(
        *brushNode2,
        vm::translation_matrix(vm::vec3d{10.0, 0.0, 0.0}),
        map.worldBounds());
      transformNode(
        *brushNode3,
        vm::translation_matrix(vm::vec3d{100.0, 0.0, 0.0}),
        map.worldBounds());

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNode1Ptr), std::move(brushNode2Ptr), std::move(brushNode3Ptr)));

      REQUIRE(brushNode1->intersects(*brushNode2));
      REQUIRE(brushNode2->intersects(*brushNode1));

      REQUIRE(!brushNode1->intersects(*brushNode3));
      REQUIRE(!brushNode3->intersects(*brushNode1));

      selectNodes(map, {brushNode1});
      selectTouchingNodes(map, false);

      CHECK_THAT(
        map.selection().brushes, UnorderedEquals(std::vector<BrushNode*>{brushNode2}));
    }

    SECTION("Select touching group")
    {
      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 1"}));

      auto* groupNode =
        addNode(map, *layerNode, std::make_unique<GroupNode>(Group{"Unnamed"}));

      const auto brushBounds = vm::bbox3d{{-32.0, -32.0, -32.0}, {+32.0, +32.0, +32.0}};
      addNode(
        map,
        *groupNode,
        std::make_unique<BrushNode>(
          builder.createCuboid(brushBounds, "material") | kdl::value()));

      const auto selectionBounds =
        vm::bbox3d{{-16.0, -16.0, -48.0}, {+16.0, +16.0, +48.0}};

      auto* selectionBrush = addNode(
        map,
        *layerNode,
        std::make_unique<BrushNode>(
          builder.createCuboid(selectionBounds, "material") | kdl::value()));

      selectNodes(map, {selectionBrush});
      selectTouchingNodes(map, true);

      CHECK(map.selection().nodes == std::vector<Node*>{groupNode});
    }

    SECTION("Don't crash when input brushes overlap")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/2476
      const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

      auto* brushNode1 = addNode(
        map,
        parentForNodes(map),
        std::make_unique<BrushNode>(
          builder.createCuboid(box, "material") | kdl::value()));

      auto* brushNode2 = addNode(
        map,
        parentForNodes(map),
        std::make_unique<BrushNode>(
          builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()));

      selectAllNodes(map);

      CHECK_THAT(
        map.selection().brushes,
        UnorderedEquals(std::vector<BrushNode*>{brushNode1, brushNode2}));
      CHECK_THAT(
        map.editorContext().currentLayer()->children(),
        RangeEquals(std::vector<Node*>{brushNode1, brushNode2}));

      selectTouchingNodes(map, true);

      // only this next line was failing
      CHECK_THAT(map.selection().brushes, UnorderedEquals(std::vector<BrushNode*>{}));
      CHECK_THAT(
        map.editorContext().currentLayer()->children(),
        RangeEquals(std::vector<Node*>{}));

      // brush1 and brush2 are deleted
      CHECK(brushNode1->parent() == nullptr);
      CHECK(brushNode2->parent() == nullptr);
    }

    SECTION("Select touching nodes inside nested group")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3826

      auto brushNode1Ptr = createBrushNode(map);
      auto* brushNode1 = brushNode1Ptr.get();
      auto brushNode2Ptr = createBrushNode(map);
      auto* brushNode2 = brushNode2Ptr.get();


      auto* outerGroup = addNode(
        map, parentForNodes(map), std::make_unique<GroupNode>(Group{"outerGroup"}));
      auto* innerGroup =
        addNode(map, *outerGroup, std::make_unique<GroupNode>(Group{"innerGroup"}));
      addNodes(
        map,
        *innerGroup,
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNode1Ptr), std::move(brushNode2Ptr)));

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
        map.selection().brushes, UnorderedEquals(std::vector<BrushNode*>{brushNode2}));
    }

    SECTION("Select touching nodes along axis")
    {
      auto brushNode1Ptr =
        std::make_unique<BrushNode>(builder.createCube(64.0, "none") | kdl::value());
      auto* brushNode1 = brushNode1Ptr.get();
      auto brushNode2Ptr =
        std::make_unique<BrushNode>(builder.createCube(64.0, "none") | kdl::value());
      auto* brushNode2 = brushNode2Ptr.get();
      auto brushNode3Ptr =
        std::make_unique<BrushNode>(builder.createCube(64.0, "none") | kdl::value());
      auto* brushNode3 = brushNode3Ptr.get();

      transformNode(
        *brushNode2,
        vm::translation_matrix(vm::vec3d{0.0, 0.0, -500.0}),
        map.worldBounds());
      transformNode(
        *brushNode3,
        vm::translation_matrix(vm::vec3d{100.0, 0.0, 0.0}),
        map.worldBounds());

      REQUIRE(!brushNode1->intersects(*brushNode2));
      REQUIRE(!brushNode1->intersects(*brushNode3));

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNode1Ptr), std::move(brushNode2Ptr), std::move(brushNode3Ptr)));
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
      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"Layer 1"}));

      auto* groupNode =
        addNode(map, *layerNode, std::make_unique<GroupNode>(Group{"Unnamed"}));

      const auto brushBounds = vm::bbox3d{{-32.0, -32.0, -32.0}, {+32.0, +32.0, +32.0}};
      addNode(
        map,
        *groupNode,
        std::make_unique<BrushNode>(
          builder.createCuboid(brushBounds, "material") | kdl::value()));

      const auto selectionBounds =
        vm::bbox3d{{-48.0, -48.0, -48.0}, {+48.0, +48.0, +48.0}};

      auto* selectionBrush = addNode(
        map,
        *layerNode,
        std::make_unique<BrushNode>(
          builder.createCuboid(selectionBounds, "material") | kdl::value()));

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

    auto brushPtr = createBrushNode(map, "brush");
    auto* brush = brushPtr.get();
    auto pointEntityPtr = std::make_unique<EntityNode>(Entity{});
    auto* pointEntity = pointEntityPtr.get();
    auto patchPtr = createPatchNode("patch");
    auto* patch = patchPtr.get();

    auto brushEntityPtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntity = brushEntityPtr.get();
    auto brushInEntity1Ptr = createBrushNode(map, "brushInEntity1");
    auto* brushInEntity1 = brushInEntity1Ptr.get();
    auto brushInEntity2Ptr = createBrushNode(map, "brushInEntity2");
    auto* brushInEntity2 = brushInEntity2Ptr.get();

    auto outerGroupPtr = std::make_unique<GroupNode>(Group{"outerGroup"});
    auto* outerGroup = outerGroupPtr.get();
    auto brushInOuterGroupPtr = createBrushNode(map, "brushInOuterGroup");
    auto* brushInOuterGroup = brushInOuterGroupPtr.get();
    auto innerGroupPtr = std::make_unique<GroupNode>(Group{"innerGroup"});
    auto* innerGroup = innerGroupPtr.get();
    auto brushInInnerGroupPtr = createBrushNode(map, "brushInInnerGroup");
    auto* brushInInnerGroup = brushInInnerGroupPtr.get();

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
      return nodes | std::views::transform([&](const Node* node) {
               return kdl::map_find_or_default(
                 nodeToName, node, std::string{"<unknown>"});
             });
    };

    addNodes(
      map,
      *map.worldNode().defaultLayer(),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushPtr),
        std::move(pointEntityPtr),
        std::move(patchPtr),
        std::move(brushEntityPtr),
        std::move(outerGroupPtr)));

    addNodes(
      map,
      *brushEntity,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushInEntity1Ptr), std::move(brushInEntity2Ptr)));
    addNodes(
      map,
      *outerGroup,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushInOuterGroupPtr), std::move(innerGroupPtr)));

    addNode(map, *innerGroup, std::move(brushInInnerGroupPtr));

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
        mapNodeNames(map.selection().nodes), UnorderedRangeEquals(expectedNodeNames));
    }

    SECTION("outer group is open")
    {
      openGroup(map, *outerGroup);

      const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
        {{31}, {}},
        {{32}, {"brushInOuterGroup"}},
        {{39}, {"innerGroup"}},
        {{43}, {"innerGroup"}},
      }));

      CAPTURE(lineNumbers);

      selectNodesWithFilePosition(map, lineNumbers);
      CHECK_THAT(
        mapNodeNames(map.selection().nodes), UnorderedRangeEquals(expectedNodeNames));
    }

    SECTION("inner group is open")
    {
      openGroup(map, *outerGroup);
      openGroup(map, *innerGroup);

      const auto [lineNumbers, expectedNodeNames] = GENERATE(values<T>({
        {{31}, {}},
        {{32}, {}},
        {{39}, {}},
        {{43}, {"brushInInnerGroup"}},
      }));

      CAPTURE(lineNumbers);

      selectNodesWithFilePosition(map, lineNumbers);
      CHECK_THAT(
        mapNodeNames(map.selection().nodes), UnorderedRangeEquals(expectedNodeNames));
    }
  }

  SECTION("selectBrushesWithMaterial")
  {
    auto brushNodeM1Ptr = createBrushNode(map, "material1");
    auto* brushNodeM1 = brushNodeM1Ptr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::make_unique<EntityNode>(Entity{}),
        std::move(brushNodeM1Ptr),
        createBrushNode(map, "material2"),
        std::move(groupNodePtr),
        createPatchNode(),
        std::move(brushEntityNodePtr)));
    auto* groupedBrushNodeM1 =
      addNode(map, *groupNode, createBrushNode(map, "material1"));
    auto* entityBrushNodeM1 =
      addNode(map, *brushEntityNode, createBrushNode(map, "material1"));

    SECTION("Case insensitivity")
    {
      const auto name = GENERATE("material1", "MATERIAL1");
      CAPTURE(name);

      selectBrushesWithMaterial(map, name);

      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{brushNodeM1, entityBrushNodeM1, groupNode}));
    }

    SECTION("No group open")
    {
      selectBrushesWithMaterial(map, "material1");

      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{brushNodeM1, entityBrushNodeM1, groupNode}));
    }

    SECTION("With open group")
    {
      openGroup(map, *groupNode);

      selectBrushesWithMaterial(map, "material1");

      CHECK(map.selection().nodes == std::vector<Node*>{groupedBrushNodeM1});
    }
  }

  SECTION("selectEntitiesWithClassname")
  {
    auto pointEntityNodePtr =
      std::make_unique<EntityNode>(Entity{{{"classname", "trigger_secret"}}});
    auto* pointEntityNode = pointEntityNodePtr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto groupedEntityNodePtr =
      std::make_unique<EntityNode>(Entity{{{"classname", "trigger_secret"}}});
    auto* groupedEntityNode = groupedEntityNodePtr.get();
    auto otherGroupedEntityNodePtr =
      std::make_unique<EntityNode>(Entity{{{"classname", "info_player_start"}}});
    auto brushEntityNodePtr =
      std::make_unique<EntityNode>(Entity{{{"classname", "trigger_secret"}}});
    auto* brushEntityNode = brushEntityNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(pointEntityNodePtr),
        std::make_unique<EntityNode>(Entity{{{"classname", "info_player_start"}}}),
        createBrushNode(map),
        std::move(groupNodePtr),
        std::move(brushEntityNodePtr)));
    addNodes(
      map,
      *groupNode,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(groupedEntityNodePtr), std::move(otherGroupedEntityNodePtr)));
    auto* entityBrushNode = addNode(map, *brushEntityNode, createBrushNode(map));

    SECTION("Case insensitivity")
    {
      const auto classname = GENERATE("trigger_secret", "TRIGGER_SECRET");
      CAPTURE(classname);

      selectEntitiesWithClassname(map, classname);

      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(
          std::vector<Node*>{pointEntityNode, entityBrushNode, groupedEntityNode}));
    }

    SECTION("Entities in a closed group are selected individually")
    {
      selectEntitiesWithClassname(map, "trigger_secret");

      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(
          std::vector<Node*>{pointEntityNode, entityBrushNode, groupedEntityNode}));

      // Selecting the group instead would act on otherGroupedEntityNode too.
      CHECK_THAT(
        map.selection().allEntities(),
        UnorderedEquals(std::vector<EntityNodeBase*>{
          pointEntityNode, brushEntityNode, groupedEntityNode}));
    }

    SECTION("With open group")
    {
      openGroup(map, *groupNode);

      selectEntitiesWithClassname(map, "trigger_secret");

      CHECK(map.selection().nodes == std::vector<Node*>{groupedEntityNode});
    }

    SECTION("Worldspawn is not selected")
    {
      selectEntitiesWithClassname(map, "worldspawn");

      CHECK(map.selection().nodes == std::vector<Node*>{});
    }

    SECTION("Hidden entities are not selected")
    {
      hideNodes(map, {pointEntityNode, brushEntityNode});

      selectEntitiesWithClassname(map, "trigger_secret");

      CHECK(map.selection().nodes == std::vector<Node*>{groupedEntityNode});
    }

    SECTION("Entities in a hidden group are not selected")
    {
      hideNodes(map, {groupNode});

      selectEntitiesWithClassname(map, "trigger_secret");

      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{pointEntityNode, entityBrushNode}));
    }

    SECTION("Locked entities are not selected")
    {
      lockNodes(map, {pointEntityNode, brushEntityNode});

      selectEntitiesWithClassname(map, "trigger_secret");

      CHECK(map.selection().nodes == std::vector<Node*>{groupedEntityNode});
    }

    SECTION("Entities in a locked group are not selected")
    {
      lockNodes(map, {groupNode});

      selectEntitiesWithClassname(map, "trigger_secret");

      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{pointEntityNode, entityBrushNode}));
    }
  }

  SECTION("invertNodeSelection")
  {
    const auto box = vm::bbox3d{{0, 0, 0}, {64, 64, 64}};

    auto* brushNode1 = addNode(
      map,
      parentForNodes(map),
      std::make_unique<BrushNode>(builder.createCuboid(box, "material") | kdl::value()));

    auto* brushNode2 = addNode(
      map,
      parentForNodes(map),
      std::make_unique<BrushNode>(
        builder.createCuboid(box.translate({1, 1, 1}), "material") | kdl::value()));

    auto* brushNode3 = addNode(
      map,
      parentForNodes(map),
      std::make_unique<BrushNode>(
        builder.createCuboid(box.translate({2, 2, 2}), "material") | kdl::value()));

    auto* patchNode = addNode(map, parentForNodes(map), createPatchNode());

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
      UnorderedEquals(std::vector<Node*>{brushNode2, brushNode3, patchNode}));
    CHECK(!brushNode1->selected());
    CHECK(brushNode2->selected());
    CHECK(brushNode3->selected());
    CHECK(!brushEnt->selected());
    CHECK(patchNode->selected());
  }

  SECTION("selectAllInLayers")
  {
    auto* defaultLayer = map.worldNode().defaultLayer();
    auto customLayer1Ptr = std::make_unique<LayerNode>(Layer{"1"});
    auto* customLayer1 = customLayer1Ptr.get();
    auto customLayer2Ptr = std::make_unique<LayerNode>(Layer{"2"});
    auto* customLayer2 = customLayer2Ptr.get();


    addNodes(
      map,
      map.worldNode(),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(customLayer1Ptr), std::move(customLayer2Ptr)));
    auto* entityNodeInDefaultLayer =
      addNode(map, *defaultLayer, std::make_unique<EntityNode>(Entity{}));
    auto* entityNodeInCustomLayer1 =
      addNode(map, *customLayer1, std::make_unique<EntityNode>(Entity{}));
    addNode(map, *customLayer2, std::make_unique<EntityNode>(Entity{}));

    selectAllInLayers(map, {defaultLayer});
    CHECK(map.selection().nodes == std::vector<Node*>{entityNodeInDefaultLayer});

    selectAllInLayers(map, {defaultLayer, customLayer1});
    CHECK_THAT(
      map.selection().nodes,
      UnorderedEquals(
        std::vector<Node*>{entityNodeInDefaultLayer, entityNodeInCustomLayer1}));
  }

  SECTION("selectLinkedGroups")
  {
    auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* entityNode = entityNodePtr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto linkedGroupNode1Ptr = std::make_unique<GroupNode>(Group{"linked group"});
    auto* linkedGroupNode1 = linkedGroupNode1Ptr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(entityNodePtr),
        std::move(groupNodePtr),
        std::move(linkedGroupNode1Ptr)));
    addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));
    addNode(map, *linkedGroupNode1, std::make_unique<EntityNode>(Entity{}));

    selectNodes(map, {linkedGroupNode1});
    auto* linkedGroupNode2 = createLinkedDuplicate(map);
    auto* linkedGroupNode3 = createLinkedDuplicate(map);

    deselectAll(map);

    SECTION("When the original is selected")
    {
      selectNodes(map, {linkedGroupNode1});

      selectLinkedGroups(map);
      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(
          std::vector<Node*>{linkedGroupNode1, linkedGroupNode2, linkedGroupNode3}));
    }

    SECTION("When a duplicate is selected")
    {
      selectNodes(map, {linkedGroupNode2});

      selectLinkedGroups(map);
      CHECK_THAT(
        map.selection().nodes,
        UnorderedEquals(
          std::vector<Node*>{linkedGroupNode1, linkedGroupNode2, linkedGroupNode3}));
    }

    SECTION("When nothing is selected")
    {
      selectLinkedGroups(map);
      CHECK(map.selection().nodes == std::vector<Node*>{});
    }

    SECTION("When an unlinked group is selected")
    {
      selectNodes(map, {groupNode});

      selectLinkedGroups(map);
      CHECK(map.selection().nodes == std::vector<Node*>{groupNode});
    }

    SECTION("When some node is selected")
    {
      selectNodes(map, {entityNode});

      selectLinkedGroups(map);
      CHECK(map.selection().nodes == std::vector<Node*>{entityNode});
    }
  }

  SECTION("selectBrushFaces")
  {
    SECTION("Basic selection")
    {
      auto brushNode1Ptr = createBrushNode(map);
      auto* brushNode1 = brushNode1Ptr.get();
      auto brushNode2Ptr = createBrushNode(map);
      auto* brushNode2 = brushNode2Ptr.get();

      addNodes(
        map,
        parentForNodes(map),
        kdl::vec_from<std::unique_ptr<Node>>(
          std::move(brushNode1Ptr), std::move(brushNode2Ptr)));

      selectBrushFaces(map, {{*brushNode1, 0}, {*brushNode1, 2}, {*brushNode2, 5}});

      CHECK_THAT(
        map.selection().brushFaces,
        UnorderedEquals(std::vector<BrushFaceHandle>{
          {*brushNode1, 0},
          {*brushNode1, 2},
          {*brushNode2, 5},
        }));

      selectBrushFaces(map, {{*brushNode1, 1}});

      CHECK_THAT(
        map.selection().brushFaces,
        UnorderedEquals(std::vector<BrushFaceHandle>{
          {*brushNode1, 0},
          {*brushNode1, 1},
          {*brushNode1, 2},
          {*brushNode2, 5},
        }));
    }

    SECTION("Linked groups")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode != nullptr);

      deselectAll(map);

      SECTION("Face selection locks other groups in link set")
      {
        CHECK(!linkedGroupNode->locked());

        selectBrushFaces(map, {{*brushNode, 0}});
        CHECK(linkedGroupNode->locked());

        deselectAll(map);
        CHECK(!linkedGroupNode->locked());
      }
    }
  }

  SECTION("selectBrushFacesWithMaterial")
  {
    auto brushNodeM1Ptr = createBrushNode(map, "material1");
    auto* brushNodeM1 = brushNodeM1Ptr.get();
    auto brushNodeM13Ptr = createBrushNode(map, "material3");
    auto* brushNodeM13 = brushNodeM13Ptr.get();

    {
      auto brushM13 = brushNodeM13->brush();
      for (auto& brushFace : (brushM13.faces() | std::views::take(3)))
      {
        brushFace.setMaterialName("material1");
      }
      brushNodeM13->setBrush(std::move(brushM13));
    }

    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();

    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodeM1Ptr),
        createBrushNode(map, "material2"),
        std::move(brushNodeM13Ptr),
        std::move(groupNodePtr),
        std::move(brushEntityNodePtr)));
    auto* groupedBrushNodeM1 =
      addNode(map, *groupNode, createBrushNode(map, "material1"));
    auto* entityBrushNodeM1 =
      addNode(map, *brushEntityNode, createBrushNode(map, "material1"));

    SECTION("Case insensitivity")
    {
      const auto name = GENERATE("material1", "MATERIAL1");
      CAPTURE(name);

      selectBrushFacesWithMaterial(map, name);

      const auto expectedBrushFaces =
        kdl::views::concat(
          toHandles(*brushNodeM1),
          toHandles(*entityBrushNodeM1),
          toHandles(*groupedBrushNodeM1),
          toHandles(*brushNodeM13) | std::views::filter([](const auto& handle) {
            return handle.face().materialName() == "material1";
          }) | kdl::ranges::to<std::vector>())
        | kdl::ranges::to<std::vector>();

      CHECK_THAT(map.selection().brushFaces, UnorderedEquals(expectedBrushFaces));
    }

    SECTION("No group open")
    {
      selectBrushFacesWithMaterial(map, "material1");

      const auto expectedBrushFaces =
        kdl::views::concat(
          toHandles(*brushNodeM1),
          toHandles(*entityBrushNodeM1),
          toHandles(*groupedBrushNodeM1),
          toHandles(*brushNodeM13) | std::views::filter([](const auto& handle) {
            return handle.face().materialName() == "material1";
          }) | kdl::ranges::to<std::vector>())
        | kdl::ranges::to<std::vector>();

      CHECK_THAT(map.selection().brushFaces, UnorderedEquals(expectedBrushFaces));
    }

    SECTION("With open group")
    {
      openGroup(map, *groupNode);

      selectBrushFacesWithMaterial(map, "material1");

      CHECK_THAT(
        map.selection().brushFaces, UnorderedEquals(toHandles(*groupedBrushNodeM1)));
    }
  }

  SECTION("deselectAll")
  {
    auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* entityNode = entityNodePtr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(entityNodePtr),
        std::move(groupNodePtr),
        std::move(brushEntityNodePtr)));
    addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));
    auto* entityBrushNode = addNode(map, *brushEntityNode, createBrushNode(map));

    SECTION("Node selection")
    {
      selectNodes(map, {entityNode, groupNode, entityBrushNode});

      REQUIRE_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{entityNode, groupNode, entityBrushNode}));

      deselectAll(map);
      CHECK(map.selection().nodes == std::vector<Node*>{});
    }

    SECTION("Brush face selection")
    {
      selectBrushFaces(map, toHandles(*entityBrushNode));

      REQUIRE_THAT(
        map.selection().brushFaces, UnorderedEquals(toHandles(*entityBrushNode)));
      deselectAll(map);
      CHECK(map.selection().brushFaces == std::vector<BrushFaceHandle>{});
    }
  }

  SECTION("deselectNodes")
  {
    auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* entityNode = entityNodePtr.get();
    auto unselectedEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* unselectedEntityNode = unselectedEntityNodePtr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(entityNodePtr),
        std::move(unselectedEntityNodePtr),
        std::move(groupNodePtr),
        std::move(brushEntityNodePtr)));
    addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));
    auto* entityBrushNode = addNode(map, *brushEntityNode, createBrushNode(map));

    SECTION("Node selection")
    {
      selectNodes(map, {entityNode, groupNode, entityBrushNode});

      REQUIRE_THAT(
        map.selection().nodes,
        UnorderedEquals(std::vector<Node*>{entityNode, groupNode, entityBrushNode}));

      SECTION("Deselect nothing does nothing")
      {
        deselectNodes(map, {});
        CHECK_THAT(
          map.selection().nodes,
          UnorderedEquals(std::vector<Node*>{entityNode, groupNode, entityBrushNode}));
      }

      SECTION("Deselect an unselected node has does nothing")
      {
        deselectNodes(map, {unselectedEntityNode});
        CHECK_THAT(
          map.selection().nodes,
          UnorderedEquals(std::vector<Node*>{entityNode, groupNode, entityBrushNode}));
      }

      SECTION("Deselect a single node")
      {
        deselectNodes(map, {entityNode});
        CHECK_THAT(
          map.selection().nodes,
          UnorderedEquals(std::vector<Node*>{groupNode, entityBrushNode}));
      }

      SECTION("Deselect multiple nodes")
      {
        deselectNodes(map, {entityNode, groupNode});
        CHECK_THAT(
          map.selection().nodes, UnorderedEquals(std::vector<Node*>{entityBrushNode}));
      }

      SECTION("Deselect all nodes")
      {
        deselectNodes(map, {entityNode, groupNode, entityBrushNode});
        CHECK(map.selection().nodes == std::vector<Node*>{});
      }
    }

    SECTION("Brush face selection")
    {
      selectBrushFaces(map, toHandles(*entityBrushNode));

      REQUIRE_THAT(
        map.selection().brushFaces, UnorderedEquals(toHandles(*entityBrushNode)));

      deselectNodes(map, {entityNode});
      CHECK_THAT(
        map.selection().brushFaces, UnorderedEquals(toHandles(*entityBrushNode)));
    }
  }

  SECTION("deselectBrushFaces")
  {
    auto brushNode1Ptr = createBrushNode(map);
    auto* brushNode1 = brushNode1Ptr.get();
    auto brushNode2Ptr = createBrushNode(map);
    auto* brushNode2 = brushNode2Ptr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNode1Ptr), std::move(brushNode2Ptr)));

    selectBrushFaces(map, {{*brushNode1, 0}, {*brushNode1, 2}, {*brushNode2, 5}});

    REQUIRE_THAT(
      map.selection().brushFaces,
      UnorderedEquals(std::vector<BrushFaceHandle>{
        {*brushNode1, 0},
        {*brushNode1, 2},
        {*brushNode2, 5},
      }));

    SECTION("Deselect nothing does nothing")
    {
      deselectBrushFaces(map, {});
      CHECK_THAT(
        map.selection().brushFaces,
        UnorderedEquals(std::vector<BrushFaceHandle>{
          {*brushNode1, 0},
          {*brushNode1, 2},
          {*brushNode2, 5},
        }));
    }

    SECTION("Deselect an unselected face does nothing")
    {
      deselectBrushFaces(map, {{*brushNode1, 1}});
      CHECK_THAT(
        map.selection().brushFaces,
        UnorderedEquals(std::vector<BrushFaceHandle>{
          {*brushNode1, 0},
          {*brushNode1, 2},
          {*brushNode2, 5},
        }));
    }

    SECTION("Deselect a single face")
    {
      deselectBrushFaces(map, {{*brushNode1, 0}});
      CHECK_THAT(
        map.selection().brushFaces,
        UnorderedEquals(std::vector<BrushFaceHandle>{
          {*brushNode1, 2},
          {*brushNode2, 5},
        }));
    }

    SECTION("Deselect multiple faces")
    {
      deselectBrushFaces(map, {{*brushNode1, 0}, {*brushNode2, 5}});
      CHECK_THAT(
        map.selection().brushFaces,
        UnorderedEquals(std::vector<BrushFaceHandle>{
          {*brushNode1, 2},
        }));
    }

    SECTION("Deselect all faces")
    {
      deselectBrushFaces(map, {{*brushNode1, 0}, {*brushNode1, 2}, {*brushNode2, 5}});
      CHECK(map.selection().brushFaces == std::vector<BrushFaceHandle>{});
    }
  }

  SECTION("Selection clears repeat stack")
  {
    auto* entityNode1 =
      addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

    auto* entityNode2 =
      addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

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
    auto* entityNode = addNode(
      map,
      parentForNodes(map),
      std::make_unique<EntityNode>(Entity{{{"classname", "point_entity"}}}));
    REQUIRE(!entityNode->logicalBounds().is_empty());

    selectAllNodes(map);

    auto bounds = map.selectionBounds();
    deselectAll(map);
    CHECK(map.lastSelectionBounds() == bounds);

    deselectAll(map);
    CHECK(map.lastSelectionBounds() == bounds);

    auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));

    selectNodes(map, {brushNode});
    CHECK(map.lastSelectionBounds() == bounds);

    bounds = brushNode->logicalBounds();

    deselectAll(map);
    CHECK(map.lastSelectionBounds() == bounds);
  }

  SECTION("referenceBounds")
  {
    SECTION("without a selection or a previous selection")
    {
      CHECK(map.referenceBounds() == vm::bbox3d{16.0});
    }

    SECTION("with a selection")
    {
      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));
      selectNodes(map, {brushNode});

      CHECK(map.referenceBounds() == brushNode->logicalBounds());
    }

    SECTION("with a previous but no current selection")
    {
      auto* brushNode = addNode(map, parentForNodes(map), createBrushNode(map));
      selectNodes(map, {brushNode});
      const auto bounds = brushNode->logicalBounds();

      deselectAll(map);

      CHECK(map.referenceBounds() == bounds);
    }
  }
}

} // namespace tb::mdl
