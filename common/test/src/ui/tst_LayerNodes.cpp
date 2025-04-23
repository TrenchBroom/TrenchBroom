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

#include "MapDocumentTest.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/LockState.h"
#include "mdl/ModelUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/VisibilityState.h"
#include "mdl/WorldNode.h"

#include "Catch2.h"

namespace tb::ui
{
namespace
{

void setLayerSortIndex(mdl::LayerNode& layerNode, int sortIndex)
{
  auto layer = layerNode.layer();
  layer.setSortIndex(sortIndex);
  layerNode.setLayer(layer);
}

} // namespace

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.defaultLayerSortIndexImmutable")
{
  auto* defaultLayerNode = document->world()->defaultLayer();
  setLayerSortIndex(*defaultLayerNode, 555);

  CHECK(defaultLayerNode->layer().sortIndex() == mdl::Layer::defaultLayerSortIndex());
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.renameLayer")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* layerNode = new mdl::LayerNode{mdl::Layer{"test1"}};
  document->addNodes({{document->world(), {layerNode}}});
  CHECK(layerNode->name() == "test1");

  document->renameLayer(layerNode, "test2");
  CHECK(layerNode->name() == "test2");

  document->undoCommand();
  CHECK(layerNode->name() == "test1");
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.duplicateObjectGoesIntoSourceLayer")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"test1"}};
  auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"test2"}};
  document->addNodes({{document->world(), {layerNode1}}});
  document->addNodes({{document->world(), {layerNode2}}});

  document->setCurrentLayer(layerNode1);
  auto* entity = document->createPointEntity(*m_pointEntityDef, vm::vec3d{0, 0, 0});
  CHECK(entity->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1);

  document->setCurrentLayer(layerNode2);
  document->selectNodes({entity});
  document->duplicate(); // the duplicate should stay in layer1

  REQUIRE(document->selectedNodes().entityCount() == 1);
  auto* entityClone = document->selectedNodes().entities().at(0);
  CHECK(entityClone->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2);
  CHECK(document->currentLayer() == layerNode2);
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.newGroupGoesIntoSourceLayer")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"test1"}};
  auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"test2"}};
  document->addNodes({{document->world(), {layerNode1}}});
  document->addNodes({{document->world(), {layerNode2}}});

  document->setCurrentLayer(layerNode1);
  auto* entity = document->createPointEntity(*m_pointEntityDef, vm::vec3d{0, 0, 0});
  CHECK(entity->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1);

  document->setCurrentLayer(layerNode2);
  document->selectNodes({entity});
  mdl::GroupNode* newGroup =
    document->groupSelection("Group in Layer 1"); // the new group should stay in layer1

  CHECK(entity->parent() == newGroup);
  CHECK(mdl::findContainingLayer(entity) == layerNode1);
  CHECK(mdl::findContainingLayer(newGroup) == layerNode1);
  CHECK(document->currentLayer() == layerNode2);
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.newObjectsInHiddenLayerAreVisible")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"test1"}};
  auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"test2"}};
  document->addNodes({{document->world(), {layerNode1}}});
  document->addNodes({{document->world(), {layerNode2}}});

  document->setCurrentLayer(layerNode1);

  // Create an entity in layer1
  auto* entity1 = document->createPointEntity(*m_pointEntityDef, vm::vec3d{0, 0, 0});
  CHECK(entity1->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1u);

  CHECK(entity1->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(entity1->visible());

  // Hide layer1. If any nodes in the layer were Visibility_Shown they would be reset to
  // Visibility_Inherited
  document->hideLayers({layerNode1});

  CHECK(entity1->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(!entity1->visible());

  // Create another entity in layer1. It will be visible, while entity1 will still be
  // hidden.
  auto* entity2 = document->createPointEntity(*m_pointEntityDef, vm::vec3d{0, 0, 0});
  CHECK(entity2->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2u);

  CHECK(entity1->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(!entity1->visible());
  CHECK(entity2->visibilityState() == mdl::VisibilityState::Shown);
  CHECK(entity2->visible());

  // Change to layer2. This hides all objects in layer1
  document->setCurrentLayer(layerNode2);

  CHECK(document->currentLayer() == layerNode2);
  CHECK(entity1->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(!entity1->visible());
  CHECK(entity2->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(!entity2->visible());

  // Undo (Switch current layer back to layer1)
  document->undoCommand();

  CHECK(document->currentLayer() == layerNode1);
  CHECK(entity1->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(!entity1->visible());
  CHECK(entity2->visibilityState() == mdl::VisibilityState::Shown);
  CHECK(entity2->visible());

  // Undo (entity2 creation)
  document->undoCommand();

  CHECK(layerNode1->childCount() == 1u);
  CHECK(entity1->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(!entity1->visible());

  // Undo (hiding layer1)
  document->undoCommand();

  CHECK(entity1->visibilityState() == mdl::VisibilityState::Inherited);
  CHECK(entity1->visible());
}

TEST_CASE_METHOD(
  MapDocumentTest,
  "LayerNodeTest.duplicatedObjectInHiddenLayerIsVisible",
  "[LayerNodesTest]")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"test1"}};
  document->addNodes({{document->world(), {layerNode1}}});

  document->setCurrentLayer(layerNode1);
  document->hideLayers({layerNode1});

  // Create entity1 and brush1 in the hidden layer1
  auto* entity1 = document->createPointEntity(*m_pointEntityDef, vm::vec3d{0, 0, 0});
  auto* brush1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brush1}}});

  CHECK(entity1->parent() == layerNode1);
  CHECK(brush1->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2u);

  CHECK(entity1->visibilityState() == mdl::VisibilityState::Shown);
  CHECK(brush1->visibilityState() == mdl::VisibilityState::Shown);
  CHECK(entity1->visible());
  CHECK(brush1->visible());

  document->selectNodes({entity1, brush1});

  // Duplicate entity1 and brush1
  document->duplicate();
  REQUIRE(document->selectedNodes().entityCount() == 1u);
  REQUIRE(document->selectedNodes().brushCount() == 1u);
  auto* entity2 = document->selectedNodes().entities().front();
  auto* brush2 = document->selectedNodes().brushes().front();

  CHECK(entity2 != entity1);
  CHECK(brush2 != brush1);

  CHECK(entity2->visibilityState() == mdl::VisibilityState::Shown);
  CHECK(entity2->visible());

  CHECK(brush2->visibilityState() == mdl::VisibilityState::Shown);
  CHECK(brush2->visible());
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.newObjectsInLockedLayerAreUnlocked")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"test1"}};
  auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"test2"}};
  document->addNodes({{document->world(), {layerNode1}}});
  document->addNodes({{document->world(), {layerNode2}}});

  document->setCurrentLayer(layerNode1);

  // Create an entity in layer1
  auto* entity1 = document->createPointEntity(*m_pointEntityDef, vm::vec3d{0, 0, 0});
  CHECK(entity1->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1u);

  CHECK(entity1->lockState() == mdl::LockState::Inherited);
  CHECK(!entity1->locked());

  // Lock layer1
  document->lock({layerNode1});

  CHECK(entity1->lockState() == mdl::LockState::Inherited);
  CHECK(entity1->locked());

  // Create another entity in layer1. It will be unlocked, while entity1 will still be
  // locked (inherited).
  auto* entity2 = document->createPointEntity(*m_pointEntityDef, vm::vec3d{0, 0, 0});
  CHECK(entity2->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2u);

  CHECK(entity1->lockState() == mdl::LockState::Inherited);
  CHECK(entity1->locked());
  CHECK(entity2->lockState() == mdl::LockState::Unlocked);
  CHECK(!entity2->locked());

  // Change to layer2. This causes the Lock_Unlocked objects in layer1 to be degraded to
  // Lock_Inherited (i.e. everything in layer1 becomes locked)
  document->setCurrentLayer(layerNode2);

  CHECK(document->currentLayer() == layerNode2);
  CHECK(entity1->lockState() == mdl::LockState::Inherited);
  CHECK(entity1->locked());
  CHECK(entity2->lockState() == mdl::LockState::Inherited);
  CHECK(entity2->locked());

  // Undo (Switch current layer back to layer1)
  document->undoCommand();

  CHECK(document->currentLayer() == layerNode1);
  CHECK(entity1->lockState() == mdl::LockState::Inherited);
  CHECK(entity1->locked());
  CHECK(entity2->lockState() == mdl::LockState::Unlocked);
  CHECK(!entity2->locked());

  // Undo entity2 creation
  document->undoCommand();

  CHECK(layerNode1->childCount() == 1u);
  CHECK(entity1->lockState() == mdl::LockState::Inherited);
  CHECK(entity1->locked());

  // Undo locking layer1
  document->undoCommand();

  CHECK(entity1->lockState() == mdl::LockState::Inherited);
  CHECK(!entity1->locked());
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.moveLayer")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* layerNode0 = new mdl::LayerNode{mdl::Layer{"layer0"}};
  auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"layer1"}};
  auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"layer2"}};

  setLayerSortIndex(*layerNode0, 0);
  setLayerSortIndex(*layerNode1, 1);
  setLayerSortIndex(*layerNode2, 2);

  document->addNodes({{document->world(), {layerNode0}}});
  document->addNodes({{document->world(), {layerNode1}}});
  document->addNodes({{document->world(), {layerNode2}}});

  SECTION("check canMoveLayer")
  {
    // defaultLayer() can never be moved
    CHECK(!document->canMoveLayer(document->world()->defaultLayer(), 1));
    CHECK(document->canMoveLayer(layerNode0, 0));
    CHECK(!document->canMoveLayer(layerNode0, -1));
    CHECK(document->canMoveLayer(layerNode0, 1));
    CHECK(document->canMoveLayer(layerNode0, 2));
    CHECK(!document->canMoveLayer(layerNode0, 3));
  }

  SECTION("moveLayer by 0 has no effect")
  {
    document->moveLayer(layerNode0, 0);
    CHECK(layerNode0->layer().sortIndex() == 0);
  }
  SECTION("moveLayer by invalid negative amount is clamped")
  {
    document->moveLayer(layerNode0, -1000);
    CHECK(layerNode0->layer().sortIndex() == 0);
  }
  SECTION("moveLayer by 1")
  {
    document->moveLayer(layerNode0, 1);
    CHECK(layerNode1->layer().sortIndex() == 0);
    CHECK(layerNode0->layer().sortIndex() == 1);
    CHECK(layerNode2->layer().sortIndex() == 2);
  }
  SECTION("moveLayer by 2")
  {
    document->moveLayer(layerNode0, 2);
    CHECK(layerNode1->layer().sortIndex() == 0);
    CHECK(layerNode2->layer().sortIndex() == 1);
    CHECK(layerNode0->layer().sortIndex() == 2);
  }
  SECTION("moveLayer by invalid positive amount is clamped")
  {
    document->moveLayer(layerNode0, 1000);
    CHECK(layerNode1->layer().sortIndex() == 0);
    CHECK(layerNode2->layer().sortIndex() == 1);
    CHECK(layerNode0->layer().sortIndex() == 2);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.moveSelectionToLayer")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* customLayer = new mdl::LayerNode{mdl::Layer{"layer"}};
  document->addNodes({{document->world(), {customLayer}}});

  auto* defaultLayer = document->world()->defaultLayer();

  GIVEN("A top level node")
  {
    using CreateNode = std::function<mdl::Node*(const MapDocumentTest&)>;
    const auto createNode = GENERATE_COPY(
      CreateNode{[](const auto& test) {
        auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
        groupNode->addChild(test.createBrushNode());
        return groupNode;
      }},
      CreateNode{[](const auto&) { return new mdl::EntityNode{mdl::Entity{}}; }},
      CreateNode{[](const auto& test) { return test.createBrushNode(); }},
      CreateNode{[](const auto& test) { return test.createPatchNode(); }});

    auto* node = createNode(*this);
    document->addNodes({{document->parentForNodes(), {node}}});

    REQUIRE(mdl::findContainingLayer(node) == defaultLayer);

    WHEN("The node is moved to another layer")
    {
      document->selectNodes({node});
      document->moveSelectionToLayer(customLayer);

      THEN("The group node is in the target layer")
      {
        CHECK(mdl::findContainingLayer(node) == customLayer);

        AND_THEN("The node is selected")
        {
          CHECK(document->selectedNodes().nodes() == std::vector<mdl::Node*>{node});
        }
      }

      AND_WHEN("The operation is undone")
      {
        document->undoCommand();

        THEN("The node is back in the original layer")
        {
          CHECK(mdl::findContainingLayer(node) == defaultLayer);

          AND_THEN("The node is selected")
          {
            CHECK(document->selectedNodes().nodes() == std::vector<mdl::Node*>{node});
          }
        }
      }
    }
  }

  GIVEN("A brush entity node")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
    auto* childNode1 = createBrushNode();
    auto* childNode2 = createPatchNode();

    entityNode->addChildren({childNode1, childNode2});
    document->addNodes({{document->parentForNodes(), {entityNode}}});

    REQUIRE(mdl::findContainingLayer(entityNode) == defaultLayer);

    WHEN("Any child node is selected and moved to another layer")
    {
      // clang-format off
      const auto [selectChild1, selectChild2] = GENERATE(
        std::make_tuple(true, true),
        std::make_tuple(true, false),
        std::make_tuple(false, true)
      );
      // clang-format on

      if (selectChild1)
      {
        document->selectNodes({childNode1});
      }
      if (selectChild2)
      {
        document->selectNodes({childNode2});
      }

      const auto selectedNodes = document->selectedNodes().nodes();
      document->moveSelectionToLayer(customLayer);

      THEN("The brush entity node is moved to the target layer")
      {
        CHECK(mdl::findContainingLayer(entityNode) == customLayer);
        CHECK(childNode1->parent() == entityNode);
        CHECK(childNode2->parent() == entityNode);

        AND_THEN("The child nodes are selected")
        {
          CHECK(document->selectedNodes().nodes() == entityNode->children());
        }
      }

      AND_WHEN("The operation is undone")
      {
        document->undoCommand();

        THEN("The brush entity node is back in the original layer")
        {
          CHECK(mdl::findContainingLayer(entityNode) == defaultLayer);
          CHECK(childNode1->parent() == entityNode);
          CHECK(childNode2->parent() == entityNode);

          AND_THEN("The originally selected nodes are selected")
          {
            CHECK_THAT(
              document->selectedNodes().nodes(),
              Catch::Matchers::UnorderedEquals(selectedNodes));
          }
        }
      }
    }
  }
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.setCurrentLayerCollation")
{
  // delete default brush
  document->selectAllNodes();
  document->remove();

  auto* defaultLayerNode = document->world()->defaultLayer();
  auto* layerNode1 = new mdl::LayerNode{mdl::Layer{"test1"}};
  auto* layerNode2 = new mdl::LayerNode{mdl::Layer{"test2"}};
  document->addNodes({{document->world(), {layerNode1}}});
  document->addNodes({{document->world(), {layerNode2}}});
  CHECK(document->currentLayer() == defaultLayerNode);

  document->setCurrentLayer(layerNode1);
  document->setCurrentLayer(layerNode2);
  CHECK(document->currentLayer() == layerNode2);

  // No collation currently because of the transactions in setCurrentLayer()
  document->undoCommand();
  CHECK(document->currentLayer() == layerNode1);
  document->undoCommand();
  CHECK(document->currentLayer() == defaultLayerNode);

  document->redoCommand();
  CHECK(document->currentLayer() == layerNode1);
  document->redoCommand();
  CHECK(document->currentLayer() == layerNode2);
}

} // namespace tb::ui
