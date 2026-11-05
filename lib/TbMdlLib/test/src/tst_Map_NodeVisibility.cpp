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

#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
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
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"

#include "kd/vector_utils.h"

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
      auto* nodeToHide =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      REQUIRE(!nodeToHide->hidden());

      AND_GIVEN("Another top level node that should be isolated")
      {
        using CreateNode = std::function<std::unique_ptr<Node>(const Map&)>;
        const auto createNode = GENERATE_COPY(
          CreateNode{[](const auto& m) {
            auto groupNode = std::make_unique<GroupNode>(Group{"group"});
            groupNode->addChild(createBrushNode(m));
            return groupNode;
          }},
          CreateNode{[](const auto&) { return std::make_unique<EntityNode>(Entity{}); }},
          CreateNode{[](const auto& m) { return createBrushNode(m); }},
          CreateNode{[](const auto&) { return createPatchNode(); }});

        auto* nodeToIsolate = addNode(map, parentForNodes(map), createNode(map));

        REQUIRE(!nodeToIsolate->hidden());

        WHEN("The node is isolated")
        {
          selectNodes(map, {nodeToIsolate});

          const auto selectedNodes = map.selection().nodes;
          isolateSelectedNodes(map);

          THEN("The node is isolated and selected")
          {
            CHECK(!nodeToIsolate->hidden());
            CHECK(nodeToHide->hidden());
            CHECK(nodeToIsolate->selected());
          }

          AND_WHEN("The operation is undone")
          {
            map.undoCommand();

            THEN("All nodes are visible again and selection is restored")
            {
              CHECK(!nodeToIsolate->hidden());
              CHECK(!nodeToHide->hidden());

              CHECK_THAT(map.selection().nodes, UnorderedEquals(selectedNodes));
            }
          }
        }
      }

      AND_GIVEN("A top level brush entity")
      {
        auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
        auto* entityNode = entityNodePtr.get();
        auto* childNode1 = &entityNode->addChild(createBrushNode(map));
        auto* childNode2 = &entityNode->addChild(createPatchNode());

        addNode(map, parentForNodes(map), std::move(entityNodePtr));

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
              CHECK(!nodeToHide->hidden());
              CHECK(!entityNode->hidden());
              CHECK(!childNode1->hidden());
              CHECK(!childNode2->hidden());

              CHECK_THAT(map.selection().nodes, UnorderedEquals(selectedNodes));
            }
          }
        }
      }
    }
  }

  SECTION("hideSelectedNodes")
  {
    auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* entityNode = entityNodePtr.get();
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(entityNodePtr), std::move(groupNodePtr)));
    auto* groupedEntityNode =
      addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));

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
    auto pointEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* pointEntityNode = pointEntityNodePtr.get();
    auto selectedEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* selectedEntityNode = selectedEntityNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto selectedBrushNodePtr = createBrushNode(map);
    auto* selectedBrushNode = selectedBrushNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(pointEntityNodePtr),
        std::move(selectedEntityNodePtr),
        std::move(brushEntityNodePtr)));
    addNodes(
      map,
      *brushEntityNode,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodePtr), std::move(selectedBrushNodePtr)));

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
    auto shownEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* shownEntityNode = shownEntityNodePtr.get();
    auto hiddenEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* hiddenEntityNode = hiddenEntityNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto hiddenBrushNodePtr = createBrushNode(map);
    auto* hiddenBrushNode = hiddenBrushNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(shownEntityNodePtr),
        std::move(hiddenEntityNodePtr),
        std::move(brushEntityNodePtr)));
    addNodes(
      map,
      *brushEntityNode,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodePtr), std::move(hiddenBrushNodePtr)));

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
    auto shownEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* shownEntityNode = shownEntityNodePtr.get();
    auto hiddenEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* hiddenEntityNode = hiddenEntityNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto hiddenBrushNodePtr = createBrushNode(map);
    auto* hiddenBrushNode = hiddenBrushNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(shownEntityNodePtr),
        std::move(hiddenEntityNodePtr),
        std::move(brushEntityNodePtr)));
    addNodes(
      map,
      *brushEntityNode,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodePtr), std::move(hiddenBrushNodePtr)));

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
    auto shownEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* shownEntityNode = shownEntityNodePtr.get();
    auto hiddenEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* hiddenEntityNode = hiddenEntityNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto hiddenBrushNodePtr = createBrushNode(map);
    auto* hiddenBrushNode = hiddenBrushNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(shownEntityNodePtr),
        std::move(hiddenEntityNodePtr),
        std::move(brushEntityNodePtr)));
    addNodes(
      map,
      *brushEntityNode,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodePtr), std::move(hiddenBrushNodePtr)));

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
    auto shownEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* shownEntityNode = shownEntityNodePtr.get();
    auto hiddenEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* hiddenEntityNode = hiddenEntityNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto hiddenBrushNodePtr = createBrushNode(map);
    auto* hiddenBrushNode = hiddenBrushNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(shownEntityNodePtr),
        std::move(hiddenEntityNodePtr),
        std::move(brushEntityNodePtr)));
    addNodes(
      map,
      *brushEntityNode,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodePtr), std::move(hiddenBrushNodePtr)));

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
    auto shownEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* shownEntityNode = shownEntityNodePtr.get();
    auto hiddenEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* hiddenEntityNode = hiddenEntityNodePtr.get();
    auto brushEntityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* brushEntityNode = brushEntityNodePtr.get();
    auto brushNodePtr = createBrushNode(map);
    auto* brushNode = brushNodePtr.get();
    auto hiddenBrushNodePtr = createBrushNode(map);
    auto* hiddenBrushNode = hiddenBrushNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(shownEntityNodePtr),
        std::move(hiddenEntityNodePtr),
        std::move(brushEntityNodePtr)));
    addNodes(
      map,
      *brushEntityNode,
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(brushNodePtr), std::move(hiddenBrushNodePtr)));

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
