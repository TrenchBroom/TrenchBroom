/*
 Copyright (C) 2025 Kristian Duske
 Copyright (C) 2025 Eric Wasylishen

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

#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_NodeVisibility")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  SECTION("isolateSelectedNodes")
  {
    GIVEN("An unrelated top level node")
    {
      auto* nodeToHide = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {nodeToHide}}});

      REQUIRE(!nodeToHide->hidden());

      AND_GIVEN("Another top level node that should be isolated")
      {
        using CreateNode = std::function<Node*(const Map&)>;
        const auto createNode = GENERATE_COPY(
          CreateNode{[](const auto& m) {
            auto* groupNode = new GroupNode{Group{"group"}};
            groupNode->addChild(createBrushNode(m));
            return groupNode;
          }},
          CreateNode{[](const auto&) { return new EntityNode{Entity{}}; }},
          CreateNode{[](const auto& m) { return createBrushNode(m); }},
          CreateNode{[](const auto&) { return createPatchNode(); }});

        auto* nodeToIsolate = createNode(map);
        addNodes(map, {{parentForNodes(map), {nodeToIsolate}}});

        REQUIRE(!nodeToIsolate->hidden());

        WHEN("The node is isolated")
        {
          selectNodes(map, {nodeToIsolate});

          const auto selectedNodes = map.selection().nodes;
          isolateSelectedNodes(map);

          THEN("The node is isolated and selected")
          {
            CHECK_FALSE(nodeToIsolate->hidden());
            CHECK(nodeToHide->hidden());
            CHECK(nodeToIsolate->selected());
          }

          AND_WHEN("The operation is undone")
          {
            map.undoCommand();

            THEN("All nodes are visible again and selection is restored")
            {
              CHECK_FALSE(nodeToIsolate->hidden());
              CHECK_FALSE(nodeToHide->hidden());

              CHECK_THAT(map.selection().nodes, UnorderedEquals(selectedNodes));
            }
          }
        }
      }

      AND_GIVEN("A top level brush entity")
      {
        auto* childNode1 = createBrushNode(map);
        auto* childNode2 = createPatchNode();

        auto* entityNode = new EntityNode{Entity{}};
        entityNode->addChildren({childNode1, childNode2});

        addNodes(map, {{parentForNodes(map), {entityNode}}});

        // Check initial state
        REQUIRE_FALSE(nodeToHide->hidden());
        REQUIRE_FALSE(entityNode->hidden());
        REQUIRE_FALSE(childNode1->hidden());
        REQUIRE_FALSE(childNode2->hidden());

        WHEN("Any child node is isolated")
        {
          const auto [selectChild1, selectChild2] = GENERATE(
            std::tuple{true, true}, std::tuple{true, false}, std::tuple{false, true});

          if (selectChild1)
          {
            selectNodes(map, {childNode1});
          }
          if (selectChild2)
          {
            selectNodes(map, {childNode2});
          }
          REQUIRE_FALSE(entityNode->selected());

          const auto selectedNodes = map.selection().nodes;
          isolateSelectedNodes(map);

          // https://github.com/TrenchBroom/TrenchBroom/issues/3117
          THEN("The containining entity node is visible")
          {
            CHECK(!entityNode->hidden());

            AND_THEN("The top level node is hidden")
            {
              CHECK(nodeToHide->hidden());
            }

            AND_THEN("Any selected child node is visible and selected")
            {
              CHECK(childNode1->hidden() != selectChild1);
              CHECK(childNode2->hidden() != selectChild2);
              CHECK(childNode1->selected() == selectChild1);
              CHECK(childNode2->selected() == selectChild2);
            }
          }

          AND_WHEN("The operation is undone")
          {
            map.undoCommand();

            THEN("All nodes are visible and selection is restored")
            {
              CHECK_FALSE(nodeToHide->hidden());
              CHECK_FALSE(entityNode->hidden());
              CHECK_FALSE(childNode1->hidden());
              CHECK_FALSE(childNode2->hidden());

              CHECK_THAT(map.selection().nodes, UnorderedEquals(selectedNodes));
            }
          }
        }
      }
    }
  }

  SECTION("hideSelectedNodes")
  {
    auto* entityNode = new EntityNode{Entity{}};
    auto* groupNode = new GroupNode{Group{"group"}};
    auto* groupedEntityNode = new EntityNode{Entity{}};

    addNodes(map, {{parentForNodes(map), {entityNode, groupNode}}});
    addNodes(map, {{groupNode, {groupedEntityNode}}});

    showNodes(map, {groupedEntityNode});
    REQUIRE(groupedEntityNode->visibilityState() == VisibilityState::Shown);

    selectNodes(map, {entityNode, groupNode});
    hideSelectedNodes(map);
    CHECK(map.selection().nodes == std::vector<Node*>{});
    CHECK(entityNode->visibilityState() == VisibilityState::Hidden);
    CHECK(groupNode->visibilityState() == VisibilityState::Hidden);
    CHECK(groupedEntityNode->visibilityState() == VisibilityState::Inherited);
  }

  SECTION("hideNodes")
  {
    auto* pointEntityNode = new EntityNode{Entity{}};
    auto* selectedEntityNode = new EntityNode{Entity{}};
    auto* brushEntityNode = new EntityNode{Entity{}};
    auto* brushNode = createBrushNode(map);
    auto* selectedBrushNode = createBrushNode(map);

    addNodes(
      map,
      {{parentForNodes(map), {pointEntityNode, selectedEntityNode, brushEntityNode}}});
    addNodes(map, {{brushEntityNode, {brushNode, selectedBrushNode}}});

    showNodes(map, {selectedBrushNode});
    REQUIRE(selectedBrushNode->visibilityState() == VisibilityState::Shown);

    selectNodes(map, {selectedEntityNode, selectedBrushNode});
    hideNodes(map, {pointEntityNode, brushEntityNode});
    CHECK(map.selection().nodes == std::vector<Node*>{selectedEntityNode});
    CHECK(pointEntityNode->visibilityState() == VisibilityState::Hidden);
    CHECK(brushEntityNode->visibilityState() == VisibilityState::Hidden);
    CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
    CHECK(selectedBrushNode->visibilityState() == VisibilityState::Inherited);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(
        map.selection().nodes
        == std::vector<Node*>{selectedEntityNode, selectedBrushNode});
      CHECK(pointEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(selectedBrushNode->visibilityState() == VisibilityState::Shown);

      map.redoCommand();
      CHECK(map.selection().nodes == std::vector<Node*>{selectedEntityNode});
      CHECK(pointEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(selectedBrushNode->visibilityState() == VisibilityState::Inherited);
    }
  }

  SECTION("showAllNodes")
  {
    auto* shownEntityNode = new EntityNode{Entity{}};
    auto* hiddenEntityNode = new EntityNode{Entity{}};
    auto* brushEntityNode = new EntityNode{Entity{}};
    auto* brushNode = createBrushNode(map);
    auto* hiddenBrushNode = createBrushNode(map);

    addNodes(
      map, {{parentForNodes(map), {shownEntityNode, hiddenEntityNode, brushEntityNode}}});
    addNodes(map, {{brushEntityNode, {brushNode, hiddenBrushNode}}});

    shownEntityNode->setVisibilityState(VisibilityState::Shown);
    hiddenEntityNode->setVisibilityState(VisibilityState::Hidden);
    hiddenBrushNode->setVisibilityState(VisibilityState::Hidden);

    REQUIRE(shownEntityNode->visibilityState() == VisibilityState::Shown);
    REQUIRE(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
    REQUIRE(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(brushNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    showAllNodes(map);
    CHECK(shownEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
    CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Inherited);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

      map.redoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Inherited);
    }
  }


  SECTION("showNodes")
  {
    auto* shownEntityNode = new EntityNode{Entity{}};
    auto* hiddenEntityNode = new EntityNode{Entity{}};
    auto* brushEntityNode = new EntityNode{Entity{}};
    auto* brushNode = createBrushNode(map);
    auto* hiddenBrushNode = createBrushNode(map);

    addNodes(
      map, {{parentForNodes(map), {shownEntityNode, hiddenEntityNode, brushEntityNode}}});
    addNodes(map, {{brushEntityNode, {brushNode, hiddenBrushNode}}});

    shownEntityNode->setVisibilityState(VisibilityState::Shown);
    hiddenEntityNode->setVisibilityState(VisibilityState::Hidden);
    hiddenBrushNode->setVisibilityState(VisibilityState::Hidden);

    REQUIRE(shownEntityNode->visibilityState() == VisibilityState::Shown);
    REQUIRE(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
    REQUIRE(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(brushNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    showNodes(map, {shownEntityNode, hiddenEntityNode, brushNode});
    CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
    CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Shown);
    CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(brushNode->visibilityState() == VisibilityState::Shown);
    CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

      map.redoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);
    }
  }

  SECTION("ensureNodesVisible")
  {
    auto* shownEntityNode = new EntityNode{Entity{}};
    auto* hiddenEntityNode = new EntityNode{Entity{}};
    auto* brushEntityNode = new EntityNode{Entity{}};
    auto* brushNode = createBrushNode(map);
    auto* hiddenBrushNode = createBrushNode(map);

    addNodes(
      map, {{parentForNodes(map), {shownEntityNode, hiddenEntityNode, brushEntityNode}}});
    addNodes(map, {{brushEntityNode, {brushNode, hiddenBrushNode}}});

    shownEntityNode->setVisibilityState(VisibilityState::Shown);
    hiddenEntityNode->setVisibilityState(VisibilityState::Hidden);
    hiddenBrushNode->setVisibilityState(VisibilityState::Hidden);

    REQUIRE(shownEntityNode->visibilityState() == VisibilityState::Shown);
    REQUIRE(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
    REQUIRE(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(brushNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    ensureNodesVisible(map, {shownEntityNode, hiddenEntityNode, brushEntityNode});
    CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
    CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Shown);
    CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
    CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

      map.redoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);
    }
  }

  SECTION("resetNodeVisibility")
  {
    auto* shownEntityNode = new EntityNode{Entity{}};
    auto* hiddenEntityNode = new EntityNode{Entity{}};
    auto* brushEntityNode = new EntityNode{Entity{}};
    auto* brushNode = createBrushNode(map);
    auto* hiddenBrushNode = createBrushNode(map);

    addNodes(
      map, {{parentForNodes(map), {shownEntityNode, hiddenEntityNode, brushEntityNode}}});
    addNodes(map, {{brushEntityNode, {brushNode, hiddenBrushNode}}});

    shownEntityNode->setVisibilityState(VisibilityState::Shown);
    hiddenEntityNode->setVisibilityState(VisibilityState::Hidden);
    hiddenBrushNode->setVisibilityState(VisibilityState::Hidden);

    REQUIRE(shownEntityNode->visibilityState() == VisibilityState::Shown);
    REQUIRE(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
    REQUIRE(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(brushNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    resetNodeVisibility(map, {shownEntityNode, hiddenEntityNode, brushEntityNode});
    CHECK(shownEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
    CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

      map.redoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);
    }
  }

  SECTION("downgradeShownToInherit")
  {
    auto* shownEntityNode = new EntityNode{Entity{}};
    auto* hiddenEntityNode = new EntityNode{Entity{}};
    auto* brushEntityNode = new EntityNode{Entity{}};
    auto* brushNode = createBrushNode(map);
    auto* hiddenBrushNode = createBrushNode(map);

    addNodes(
      map, {{parentForNodes(map), {shownEntityNode, hiddenEntityNode, brushEntityNode}}});
    addNodes(map, {{brushEntityNode, {brushNode, hiddenBrushNode}}});

    shownEntityNode->setVisibilityState(VisibilityState::Shown);
    hiddenEntityNode->setVisibilityState(VisibilityState::Hidden);
    hiddenBrushNode->setVisibilityState(VisibilityState::Hidden);

    REQUIRE(shownEntityNode->visibilityState() == VisibilityState::Shown);
    REQUIRE(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
    REQUIRE(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(brushNode->visibilityState() == VisibilityState::Inherited);
    REQUIRE(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    downgradeShownToInherit(map, {shownEntityNode, hiddenEntityNode, brushEntityNode});
    CHECK(shownEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
    CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
    CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
    CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Shown);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);

      map.redoCommand();
      CHECK(shownEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenEntityNode->visibilityState() == VisibilityState::Hidden);
      CHECK(brushEntityNode->visibilityState() == VisibilityState::Inherited);
      CHECK(brushNode->visibilityState() == VisibilityState::Inherited);
      CHECK(hiddenBrushNode->visibilityState() == VisibilityState::Hidden);
    }
  }
}

} // namespace tb::mdl
