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

#include "Observer.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"

#include "kd/vector_utils.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{

void setLayerSortIndex(LayerNode& layerNode, int sortIndex)
{
  auto layer = layerNode.layer();
  layer.setSortIndex(sortIndex);
  layerNode.setLayer(layer);
}

} // namespace

TEST_CASE("Map_Layers")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  SECTION("setCurrentLayer")
  {
    SECTION("Switching layers notifies map observers")
    {
      auto currentLayerDidChange = Observer<>{map.currentLayerDidChangeNotifier};

      auto* defaultLayerNode = map.worldNode().defaultLayer();
      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));

      REQUIRE(map.editorContext().currentLayer() == defaultLayerNode);

      setCurrentLayer(map, layerNode);
      CHECK(map.editorContext().currentLayer() == layerNode);
      CHECK(currentLayerDidChange.notifications == std::vector<std::tuple<>>{{}});
      currentLayerDidChange.reset();

      map.undoCommand();
      CHECK(map.editorContext().currentLayer() == defaultLayerNode);
      CHECK(currentLayerDidChange.notifications == std::vector<std::tuple<>>{{}});
      currentLayerDidChange.reset();

      map.redoCommand();
      CHECK(map.editorContext().currentLayer() == layerNode);
      CHECK(currentLayerDidChange.notifications == std::vector<std::tuple<>>{{}});
    }

    SECTION("Switching layers is collated into a single undo step")
    {
      auto* defaultLayerNode = map.worldNode().defaultLayer();
      auto* layerNode1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));
      auto* layerNode2 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test2"}));
      CHECK(map.editorContext().currentLayer() == defaultLayerNode);

      setCurrentLayer(map, layerNode1);
      setCurrentLayer(map, layerNode2);
      CHECK(map.editorContext().currentLayer() == layerNode2);

      // No collation currently because of the transactions in setCurrentLayer()
      map.undoCommand();
      CHECK(map.editorContext().currentLayer() == layerNode1);
      map.undoCommand();
      CHECK(map.editorContext().currentLayer() == defaultLayerNode);

      map.redoCommand();
      CHECK(map.editorContext().currentLayer() == layerNode1);
      map.redoCommand();
      CHECK(map.editorContext().currentLayer() == layerNode2);
    }

    SECTION("Switching away from a hidden layer with visible nodes hides them")
    {
      auto* layerNode1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));
      auto* layerNode2 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test2"}));

      setCurrentLayer(map, layerNode1);

      // Create an entity in layer1
      auto* entityNode1 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      // Hide layer1. The entity now inherits its visibility state and is hidden
      hideLayers(map, {layerNode1});

      REQUIRE(entityNode1->visibilityState() == VisibilityState::Inherited);
      REQUIRE(!entityNode1->visible());

      // Create another entity in layer1. It will be visible, while entity1 will still be
      // hidden.
      auto* entityNode2 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      REQUIRE(entityNode2->parent() == layerNode1);

      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(!entityNode1->visible());
      CHECK(entityNode2->visibilityState() == VisibilityState::Shown);
      CHECK(entityNode2->visible());

      // Change to layer2. This hides all objects in layer1
      setCurrentLayer(map, layerNode2);

      CHECK(map.editorContext().currentLayer() == layerNode2);
      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(!entityNode1->visible());
      CHECK(entityNode2->visibilityState() == VisibilityState::Inherited);
      CHECK(!entityNode2->visible());

      // Undo (Switch current layer back to layer1)
      map.undoCommand();

      CHECK(map.editorContext().currentLayer() == layerNode1);
      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(!entityNode1->visible());
      CHECK(entityNode2->visibilityState() == VisibilityState::Shown);
      CHECK(entityNode2->visible());
    }

    SECTION("Switching away from a locked layer with unlocked nodes locks them")
    {
      auto* layerNode1 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));
      auto* layerNode2 =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test2"}));

      setCurrentLayer(map, layerNode1);

      // Create an entity in layer1
      auto* entityNode1 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      lockNodes(map, {layerNode1});

      REQUIRE(entityNode1->lockState() == LockState::Inherited);
      REQUIRE(entityNode1->locked());

      // Create another entity in layer1. It will be unlocked, while entity1 will still be
      // locked.
      auto* entityNode2 =
        addNode(map, parentForNodes(map), std::make_unique<EntityNode>(Entity{}));

      REQUIRE(entityNode2->parent() == layerNode1);

      CHECK(entityNode1->lockState() == LockState::Inherited);
      CHECK(entityNode1->locked());
      CHECK(entityNode2->lockState() == LockState::Unlocked);
      CHECK(!entityNode2->locked());

      // Change to layer2. This locks all objects in layer1
      setCurrentLayer(map, layerNode2);

      CHECK(map.editorContext().currentLayer() == layerNode2);
      CHECK(entityNode1->lockState() == LockState::Inherited);
      CHECK(entityNode1->locked());
      CHECK(entityNode2->lockState() == LockState::Inherited);
      CHECK(entityNode2->locked());

      // Undo (Switch current layer back to layer1)
      map.undoCommand();

      CHECK(map.editorContext().currentLayer() == layerNode1);
      CHECK(entityNode1->lockState() == LockState::Inherited);
      CHECK(entityNode1->locked());
      CHECK(entityNode2->lockState() == LockState::Unlocked);
      CHECK(!entityNode2->locked());
    }
  }

  SECTION("renameLayer")
  {
    auto* layerNode =
      addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"test1"}));
    CHECK(layerNode->name() == "test1");

    renameLayer(map, layerNode, "test2");
    CHECK(layerNode->name() == "test2");

    map.undoCommand();
    CHECK(layerNode->name() == "test1");
  }

  SECTION("moveLayer")
  {
    auto layerNode0Ptr = std::make_unique<LayerNode>(Layer{"layer0"});
    auto* layerNode0 = layerNode0Ptr.get();
    auto layerNode1Ptr = std::make_unique<LayerNode>(Layer{"layer1"});
    auto* layerNode1 = layerNode1Ptr.get();
    auto layerNode2Ptr = std::make_unique<LayerNode>(Layer{"layer2"});
    auto* layerNode2 = layerNode2Ptr.get();

    setLayerSortIndex(*layerNode0, 0);
    setLayerSortIndex(*layerNode1, 1);
    setLayerSortIndex(*layerNode2, 2);

    addNodes(
      map,
      map.worldNode(),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(layerNode0Ptr), std::move(layerNode1Ptr), std::move(layerNode2Ptr)));

    SECTION("canMoveLayer")
    {
      // defaultLayer() can never be moved
      CHECK(!canMoveLayer(map, map.worldNode().defaultLayer(), 1));
      CHECK(canMoveLayer(map, layerNode0, 0));
      CHECK(!canMoveLayer(map, layerNode0, -1));
      CHECK(canMoveLayer(map, layerNode0, 1));
      CHECK(canMoveLayer(map, layerNode0, 2));
      CHECK(!canMoveLayer(map, layerNode0, 3));
    }

    SECTION("moveLayer by 0 has no effect")
    {
      moveLayer(map, layerNode0, 0);
      CHECK(layerNode0->layer().sortIndex() == 0);
    }
    SECTION("moveLayer by invalid negative amount is clamped")
    {
      moveLayer(map, layerNode0, -1000);
      CHECK(layerNode0->layer().sortIndex() == 0);
    }
    SECTION("moveLayer by 1")
    {
      moveLayer(map, layerNode0, 1);
      CHECK(layerNode1->layer().sortIndex() == 0);
      CHECK(layerNode0->layer().sortIndex() == 1);
      CHECK(layerNode2->layer().sortIndex() == 2);
    }
    SECTION("moveLayer by 2")
    {
      moveLayer(map, layerNode0, 2);
      CHECK(layerNode1->layer().sortIndex() == 0);
      CHECK(layerNode2->layer().sortIndex() == 1);
      CHECK(layerNode0->layer().sortIndex() == 2);
    }
    SECTION("moveLayer by invalid positive amount is clamped")
    {
      moveLayer(map, layerNode0, 1000);
      CHECK(layerNode1->layer().sortIndex() == 0);
      CHECK(layerNode2->layer().sortIndex() == 1);
      CHECK(layerNode0->layer().sortIndex() == 2);
    }
  }

  SECTION("moveSelectedNodesToLayer")
  {
    auto* customLayer =
      addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"layer"}));

    auto* defaultLayer = map.worldNode().defaultLayer();

    GIVEN("A top level node")
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

      auto* node = addNode(map, parentForNodes(map), createNode(map));

      REQUIRE(findContainingLayer(*node) == defaultLayer);

      WHEN("The node is moved to another layer")
      {
        selectNodes(map, {node});
        moveSelectedNodesToLayer(map, customLayer);

        THEN("The group node is in the target layer")
        {
          CHECK(findContainingLayer(*node) == customLayer);

          AND_THEN("The node is selected")
          {
            CHECK(map.selection().nodes == std::vector<Node*>{node});
          }
        }

        AND_WHEN("The operation is undone")
        {
          map.undoCommand();

          THEN("The node is back in the original layer")
          {
            CHECK(findContainingLayer(*node) == defaultLayer);

            AND_THEN("The node is selected")
            {
              CHECK(map.selection().nodes == std::vector<Node*>{node});
            }
          }
        }
      }
    }

    GIVEN("A brush entity node")
    {
      auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
      auto* entityNode = entityNodePtr.get();
      auto* childNode1 = &entityNode->addChild(createBrushNode(map));
      auto* childNode2 = &entityNode->addChild(createPatchNode());

      addNode(map, parentForNodes(map), std::move(entityNodePtr));

      REQUIRE(findContainingLayer(*entityNode) == defaultLayer);

      WHEN("Any child node is selected and moved to another layer")
      {
        // clang-format off
        const auto [selectChild1, selectChild2] = GENERATE(
          std::tuple{true, true},
          std::tuple{true, false},
          std::tuple{false, true}
        );
        // clang-format on

        if (selectChild1)
        {
          selectNodes(map, {childNode1});
        }
        if (selectChild2)
        {
          selectNodes(map, {childNode2});
        }

        const auto selectedNodes = map.selection().nodes;
        moveSelectedNodesToLayer(map, customLayer);

        THEN("The brush entity node is moved to the target layer")
        {
          CHECK(findContainingLayer(*entityNode) == customLayer);
          CHECK(childNode1->parent() == entityNode);
          CHECK(childNode2->parent() == entityNode);

          AND_THEN("The child nodes are selected")
          {
            CHECK_THAT(map.selection().nodes, RangeEquals(entityNode->children()));
          }
        }

        AND_WHEN("The operation is undone")
        {
          map.undoCommand();

          THEN("The brush entity node is back in the original layer")
          {
            CHECK(findContainingLayer(*entityNode) == defaultLayer);
            CHECK(childNode1->parent() == entityNode);
            CHECK(childNode2->parent() == entityNode);

            AND_THEN("The originally selected nodes are selected")
            {
              CHECK_THAT(map.selection().nodes, UnorderedEquals(selectedNodes));
            }
          }
        }
      }
    }
  }

  SECTION("hideLayers")
  {
    auto entityNodePtr = std::make_unique<EntityNode>(Entity{});
    auto* entityNode = entityNodePtr.get();

    SECTION("Hide default layer")
    {
      auto& layerNode = *map.worldNode().defaultLayer();
      addNode(map, layerNode, std::move(entityNodePtr));
      REQUIRE(layerNode.visible());
      REQUIRE(entityNode->visible());

      hideLayers(map, {&layerNode});
      CHECK(!layerNode.visible());
      CHECK(!entityNode->visible());

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(layerNode.visible());
        CHECK(entityNode->visible());

        map.redoCommand();
        CHECK(!layerNode.visible());
        CHECK(!entityNode->visible());
      }
    }

    SECTION("Hide custom layer")
    {
      auto* layerNode =
        addNode(map, map.worldNode(), std::make_unique<LayerNode>(Layer{"custom layer"}));
      addNode(map, *layerNode, std::move(entityNodePtr));
      REQUIRE(layerNode->visible());
      REQUIRE(entityNode->visible());

      hideLayers(map, {layerNode});
      CHECK(!layerNode->visible());
      CHECK(!entityNode->visible());
    }
  }

  SECTION("isolateLayers")
  {
    auto& defaultLayerNode = *map.worldNode().defaultLayer();
    auto customLayerNodePtr = std::make_unique<LayerNode>(Layer{"custom layer"});
    auto* customLayerNode = customLayerNodePtr.get();
    auto otherLayerNodePtr = std::make_unique<LayerNode>(Layer{"other layer"});
    auto* otherLayerNode = otherLayerNodePtr.get();

    auto* defaultLayerEntityNode =
      addNode(map, defaultLayerNode, std::make_unique<EntityNode>(Entity{}));
    addNodes(
      map,
      map.worldNode(),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(customLayerNodePtr), std::move(otherLayerNodePtr)));
    auto* customLayerEntityNode =
      addNode(map, *customLayerNode, std::make_unique<EntityNode>(Entity{}));

    REQUIRE(defaultLayerNode.visible());
    REQUIRE(defaultLayerEntityNode->visible());
    REQUIRE(customLayerNode->visible());
    REQUIRE(customLayerEntityNode->visible());
    REQUIRE(otherLayerNode->visible());

    SECTION("Isolate default layer")
    {
      isolateLayers(map, {&defaultLayerNode});
      CHECK(defaultLayerNode.visible());
      CHECK(defaultLayerEntityNode->visible());
      CHECK(!customLayerNode->visible());
      CHECK(!customLayerEntityNode->visible());
      CHECK(!otherLayerNode->visible());

      SECTION("Undo and redo")
      {
        map.undoCommand();
        CHECK(defaultLayerNode.visible());
        CHECK(defaultLayerEntityNode->visible());
        CHECK(customLayerNode->visible());
        CHECK(customLayerEntityNode->visible());
        CHECK(otherLayerNode->visible());

        map.redoCommand();
        CHECK(defaultLayerNode.visible());
        CHECK(defaultLayerEntityNode->visible());
        CHECK(!customLayerNode->visible());
        CHECK(!customLayerEntityNode->visible());
        CHECK(!otherLayerNode->visible());
      }
    }

    SECTION("Isolate custom layer")
    {
      isolateLayers(map, {customLayerNode});
      CHECK(!defaultLayerNode.visible());
      CHECK(!defaultLayerEntityNode->visible());
      CHECK(customLayerNode->visible());
      CHECK(customLayerEntityNode->visible());
      CHECK(!otherLayerNode->visible());
    }

    SECTION("Isolate two layers")
    {
      isolateLayers(map, {&defaultLayerNode, customLayerNode});
      CHECK(defaultLayerNode.visible());
      CHECK(defaultLayerEntityNode->visible());
      CHECK(customLayerNode->visible());
      CHECK(customLayerEntityNode->visible());
      CHECK(!otherLayerNode->visible());
    }
  }

  SECTION("setOmitLayersFromExport")
  {
    auto& defaultLayerNode = *map.worldNode().defaultLayer();
    REQUIRE(!defaultLayerNode.layer().omitFromExport());

    setOmitLayerFromExport(map, &defaultLayerNode, true);
    CHECK(defaultLayerNode.layer().omitFromExport());

    setOmitLayerFromExport(map, &defaultLayerNode, false);
    CHECK(!defaultLayerNode.layer().omitFromExport());

    SECTION("Undo and redo")
    {
      map.undoCommand();
      CHECK(defaultLayerNode.layer().omitFromExport());

      map.redoCommand();
      CHECK(!defaultLayerNode.layer().omitFromExport());
    }
  }
}

} // namespace tb::mdl
