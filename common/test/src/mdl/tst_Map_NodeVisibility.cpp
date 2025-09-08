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

#include "MapFixture.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{

TEST_CASE("Map_NodeVisibility")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

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

              CHECK_THAT(
                map.selection().nodes, Catch::Matchers::UnorderedEquals(selectedNodes));
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

              CHECK_THAT(
                map.selection().nodes, Catch::Matchers::UnorderedEquals(selectedNodes));
            }
          }
        }
      }
    }
  }
}

} // namespace tb::mdl
