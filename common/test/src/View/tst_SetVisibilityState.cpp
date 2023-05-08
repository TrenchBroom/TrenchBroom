/*
 Copyright (C) 2021 Kristian Duske
 Copyright (C) 2021 Eric Wasylishen

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

#include "Catch2.h"
#include "MapDocumentTest.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/PatchNode.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(MapDocumentTest, "SetVisibilityState.isolate")
{
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  GIVEN("An unrelated top level node")
  {
    auto* nodeToHide = new Model::EntityNode{Model::Entity{}};
    document->addNodes({{document->parentForNodes(), {nodeToHide}}});

    REQUIRE(!nodeToHide->hidden());

    AND_GIVEN("Another top level node that should be isolated")
    {
      using CreateNode = std::function<Model::Node*(const MapDocumentTest&)>;
      const auto createNode = GENERATE_COPY(
        CreateNode{[](const auto& test) {
          auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
          groupNode->addChild(test.createBrushNode());
          return groupNode;
        }},
        CreateNode{[](const auto&) { return new Model::EntityNode{Model::Entity{}}; }},
        CreateNode{[](const auto& test) { return test.createBrushNode(); }},
        CreateNode{[](const auto& test) { return test.createPatchNode(); }});

      auto* nodeToIsolate = createNode(*this);
      document->addNodes({{document->parentForNodes(), {nodeToIsolate}}});

      REQUIRE(!nodeToIsolate->hidden());

      WHEN("The node is isolated")
      {
        document->selectNodes({nodeToIsolate});

        const auto selectedNodes = document->selectedNodes().nodes();
        document->isolate();

        THEN("The node is isolated and selected")
        {
          CHECK_FALSE(nodeToIsolate->hidden());
          CHECK(nodeToHide->hidden());
          CHECK(nodeToIsolate->selected());
        }

        AND_WHEN("The operation is undone")
        {
          document->undoCommand();

          THEN("All nodes are visible again and selection is restored")
          {
            CHECK_FALSE(nodeToIsolate->hidden());
            CHECK_FALSE(nodeToHide->hidden());

            CHECK_THAT(
              document->selectedNodes().nodes(),
              Catch::Matchers::UnorderedEquals(selectedNodes));
          }
        }
      }
    }

    AND_GIVEN("A top level brush entity")
    {
      auto* childNode1 = createBrushNode();
      auto* childNode2 = createPatchNode();

      auto* entityNode = new Model::EntityNode{Model::Entity{}};
      entityNode->addChildren({childNode1, childNode2});

      document->addNodes({{document->parentForNodes(), {entityNode}}});

      // Check initial state
      REQUIRE_FALSE(nodeToHide->hidden());
      REQUIRE_FALSE(entityNode->hidden());
      REQUIRE_FALSE(childNode1->hidden());
      REQUIRE_FALSE(childNode2->hidden());

      WHEN("Any child node is isolated")
      {
        const auto [selectChild1, selectChild2] = GENERATE(
          std::make_tuple(true, true),
          std::make_tuple(true, false),
          std::make_tuple(false, true));

        if (selectChild1)
        {
          document->selectNodes({childNode1});
        }
        if (selectChild2)
        {
          document->selectNodes({childNode2});
        }
        REQUIRE_FALSE(entityNode->selected());

        const auto selectedNodes = document->selectedNodes().nodes();
        document->isolate();

        // https://github.com/TrenchBroom/TrenchBroom/issues/3117
        THEN("The containining entity node is visible")
        {
          CHECK(!entityNode->hidden());

          AND_THEN("The top level node is hidden") { CHECK(nodeToHide->hidden()); }

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
          document->undoCommand();

          THEN("All nodes are visible and selection is restored")
          {
            CHECK_FALSE(nodeToHide->hidden());
            CHECK_FALSE(entityNode->hidden());
            CHECK_FALSE(childNode1->hidden());
            CHECK_FALSE(childNode2->hidden());

            CHECK_THAT(
              document->selectedNodes().nodes(),
              Catch::Matchers::UnorderedEquals(selectedNodes));
          }
        }
      }
    }
  }
}
} // namespace View
} // namespace TrenchBroom
