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

#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"

#include "kdl/overload.h"

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Nodes")
{
  auto taskManager = createTestTaskManager();
  auto logger = NullLogger{};

  auto map = Map{*taskManager, logger};

  SECTION("addNodes")
  {
    SECTION("Nodes added to a hidden layer are visible")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      auto* layerNode2 = new LayerNode{Layer{"test2"}};
      map.addNodes({{map.world(), {layerNode1}}});
      map.addNodes({{map.world(), {layerNode2}}});

      map.setCurrentLayer(layerNode1);

      // Create an entity in layer1
      auto* entityNode1 = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode1}}});

      REQUIRE(entityNode1->parent() == layerNode1);

      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(entityNode1->visible());

      // Hide layer1. If any nodes in the layer were Visibility_Shown they would be reset
      // to Visibility_Inherited
      map.hideLayers({layerNode1});

      REQUIRE(entityNode1->visibilityState() == VisibilityState::Inherited);
      REQUIRE(!entityNode1->visible());

      // Create another entity in layer1. It will be visible, while entity1 will still be
      // hidden.
      auto* entityNode2 = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode1}}});

      REQUIRE(entityNode2->parent() == layerNode1);

      CHECK(entityNode1->visibilityState() == VisibilityState::Inherited);
      CHECK(!entityNode1->visible());
      CHECK(entityNode2->visibilityState() == VisibilityState::Shown);
      CHECK(entityNode2->visible());
    }

    SECTION("Nodes added to a locked layer are unlocked")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      auto* layerNode2 = new LayerNode{Layer{"test2"}};
      map.addNodes({{map.world(), {layerNode1}}});
      map.addNodes({{map.world(), {layerNode2}}});

      map.setCurrentLayer(layerNode1);

      // Create an entity in layer1
      auto* entityNode1 = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode1}}});

      REQUIRE(entityNode1->parent() == layerNode1);

      CHECK(entityNode1->lockState() == mdl::LockState::Inherited);
      CHECK(!entityNode1->locked());

      map.lockNodes({layerNode1});

      REQUIRE(entityNode1->lockState() == LockState::Inherited);
      REQUIRE(entityNode1->locked());

      auto* entityNode2 = new EntityNode{Entity{}};
      map.addNodes({{map.parentForNodes(), {entityNode1}}});

      REQUIRE(entityNode2->parent() == layerNode1);

      CHECK(entityNode1->lockState() == LockState::Inherited);
      CHECK(entityNode1->locked());
      CHECK(entityNode2->lockState() == LockState::Unlocked);
      CHECK(!entityNode2->locked());
    }

    SECTION("Linked groups")
    {
      SECTION("Child nodes are added to linked groups")
      {
        auto* groupNode = new GroupNode{Group{"test"}};
        auto* brushNode = createBrushNode(map);
        groupNode->addChild(brushNode);
        map.addNodes({{map.parentForNodes(), {groupNode}}});

        map.selectNodes({groupNode});
        auto* linkedGroupNode = map.createLinkedDuplicate();
        map.deselectAll();

        using CreateNode = std::function<Node*(const Map&)>;
        CreateNode createNode = GENERATE_COPY(
          CreateNode{[](const auto&) -> Node* { return new EntityNode{Entity{}}; }},
          CreateNode{[](const auto& m) -> Node* { return createBrushNode(m); }},
          CreateNode{[](const auto&) -> Node* { return createPatchNode(); }});

        auto* nodeToAdd = createNode(map);
        map.addNodes({{groupNode, {nodeToAdd}}});

        CHECK(linkedGroupNode->childCount() == 2u);

        auto* linkedNode = linkedGroupNode->children().back();
        linkedNode->accept(kdl::overload(
          [](const WorldNode*) {},
          [](const LayerNode*) {},
          [](const GroupNode*) {},
          [&](const EntityNode* linkedEntityNode) {
            const auto* originalEntityNode = dynamic_cast<EntityNode*>(nodeToAdd);
            REQUIRE(originalEntityNode);
            CHECK(originalEntityNode->entity() == linkedEntityNode->entity());
          },
          [&](const BrushNode* linkedBrushNode) {
            const auto* originalBrushNode = dynamic_cast<BrushNode*>(nodeToAdd);
            REQUIRE(originalBrushNode);
            CHECK(originalBrushNode->brush() == linkedBrushNode->brush());
          },
          [&](const PatchNode* linkedPatchNode) {
            const auto* originalPatchNode = dynamic_cast<PatchNode*>(nodeToAdd);
            REQUIRE(originalPatchNode);
            CHECK(originalPatchNode->patch() == linkedPatchNode->patch());
          }));

        map.undoCommand();
        REQUIRE(groupNode->childCount() == 1u);
        CHECK(linkedGroupNode->childCount() == 1u);

        map.redoCommand();

        REQUIRE(groupNode->childCount() == 2u);
        CHECK(linkedGroupNode->childCount() == 2u);
      }

      SECTION("Linked nodes inherit the group's transformation when they are added")
      {
        auto* groupNode = new GroupNode{Group{"group"}};
        map.addNodes({{map.parentForNodes(), {groupNode}}});

        map.selectNodes({groupNode});
        auto* linkedGroupNode = map.createLinkedDuplicate();
        map.deselectAll();

        map.selectNodes({linkedGroupNode});
        map.translateSelection(vm::vec3d{32, 0, 0});
        map.deselectAll();

        auto* brushNode = createBrushNode(map);
        map.addNodes({{groupNode, {brushNode}}});

        REQUIRE(groupNode->childCount() == 1u);
        REQUIRE(linkedGroupNode->childCount() == 1u);

        auto* linkedBrushNode =
          dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
        REQUIRE(linkedBrushNode != nullptr);

        CHECK(
          linkedBrushNode->physicalBounds()
          == brushNode->physicalBounds().transform(
            linkedGroupNode->group().transformation()));

        map.undoCommand();
        REQUIRE(groupNode->childCount() == 0u);
        REQUIRE(linkedGroupNode->childCount() == 0u);

        map.redoCommand();
        REQUIRE(groupNode->childCount() == 1u);
        REQUIRE(linkedGroupNode->childCount() == 1u);
        CHECK(
          linkedBrushNode->physicalBounds()
          == brushNode->physicalBounds().transform(
            linkedGroupNode->group().transformation()));
      }

      SECTION("Child cannot be added because adding it to a linked group fails")
      {
        auto* groupNode = new GroupNode{Group{"group"}};
        map.addNodes({{map.parentForNodes(), {groupNode}}});

        map.selectNodes({groupNode});
        auto* linkedGroupNode = map.createLinkedDuplicate();
        map.deselectAll();

        // adding a brush to the linked group node will fail because it will go out of
        // world bounds
        map.selectNodes({linkedGroupNode});
        map.translateSelection(map.worldBounds().max);
        map.deselectAll();

        auto* brushNode = createBrushNode(map);
        CHECK(map.addNodes({{groupNode, {brushNode}}}).empty());

        CHECK(groupNode->childCount() == 0u);
        CHECK(linkedGroupNode->childCount() == 0u);
      }
    }
  }

  SECTION("duplicateSelectedNodes")
  {
    SECTION("Duplicated nodes are added to the source layer")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      auto* layerNode2 = new LayerNode{Layer{"test2"}};
      map.addNodes({{map.world(), {layerNode1}}});
      map.addNodes({{map.world(), {layerNode2}}});

      map.setCurrentLayer(layerNode1);
      auto* entityNode = new EntityNode{Entity{}};
      CHECK(entityNode->parent() == layerNode1);
      CHECK(layerNode1->childCount() == 1);

      map.setCurrentLayer(layerNode2);
      map.selectNodes({entityNode});
      map.duplicateSelectedNodes();

      REQUIRE(map.selection().entities.size() == 1);

      auto* entityClone = map.selection().entities.at(0);
      CHECK(entityClone->parent() == layerNode1);
      CHECK(layerNode1->childCount() == 2);
      CHECK(map.currentLayer() == layerNode2);
    }

    SECTION("Nodes duplicated in a hidden layer become visible")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      map.addNodes({{map.world(), {layerNode1}}});

      map.setCurrentLayer(layerNode1);
      map.hideLayers({layerNode1});

      // Create entity1 and brush1 in the hidden layer1
      auto* entityNode1 = new EntityNode{Entity{}};
      auto* brushNode1 = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {entityNode1, brushNode1}}});

      REQUIRE(entityNode1->parent() == layerNode1);
      REQUIRE(brushNode1->parent() == layerNode1);
      REQUIRE(layerNode1->childCount() == 2u);

      REQUIRE(entityNode1->visibilityState() == VisibilityState::Shown);
      REQUIRE(brushNode1->visibilityState() == VisibilityState::Shown);
      REQUIRE(entityNode1->visible());
      REQUIRE(brushNode1->visible());

      map.selectNodes({entityNode1, brushNode1});

      // Duplicate entity1 and brush1
      map.duplicateSelectedNodes();
      REQUIRE(map.selection().entities.size() == 1u);
      REQUIRE(map.selection().brushes.size() == 1u);
      auto* entityNode2 = map.selection().entities.front();
      auto* brushNode2 = map.selection().brushes.front();

      REQUIRE(entityNode2 != entityNode1);
      REQUIRE(brushNode2 != brushNode1);

      CHECK(entityNode2->visibilityState() == VisibilityState::Shown);
      CHECK(entityNode2->visible());

      CHECK(brushNode2->visibilityState() == VisibilityState::Shown);
      CHECK(brushNode2->visible());
    }
  }

  SECTION("reparentNodes")
  {
    SECTION("Linked groups")
    {
      auto* brushNode = createBrushNode(map);
      auto* entityNode = new EntityNode{Entity{}};

      map.addNodes({{map.parentForNodes(), {brushNode, entityNode}}});
      map.selectNodes({brushNode, entityNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      map.deselectAll();
      map.selectNodes({groupNode});

      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

      map.deselectAll();
      map.openGroup(groupNode);

      map.reparentNodes({{map.world()->defaultLayer(), {brushNode}}});
      REQUIRE(groupNode->children() == std::vector<Node*>{entityNode});
      REQUIRE(brushNode->parent() == map.world()->defaultLayer());
      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));
    }

    SECTION("Nested linked groups")
    {
      auto* brushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      map.deselectAll();
      map.selectNodes({groupNode});
      auto* linkedGroupNode = map.createLinkedDuplicate();
      map.deselectAll();

      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

      SECTION("Adding a linked group to its linked sibling does nothing")
      {
        CHECK_FALSE(map.reparentNodes({{groupNode, {linkedGroupNode}}}));
      }

      SECTION(
        "Adding a group containing a nested linked sibling to a linked group does "
        "nothing")
      {
        map.selectNodes({linkedGroupNode});

        auto* outerGroupNode = map.groupSelectedNodes("outer");
        REQUIRE(outerGroupNode != nullptr);

        map.deselectAll();
        CHECK_FALSE(map.reparentNodes({{groupNode, {outerGroupNode}}}));
      }
    }
  }
}

} // namespace tb::mdl
