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

#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/LockState.h"
#include "Model/ModelUtils.h"
#include "Model/PatchNode.h"
#include "Model/VisibilityState.h"
#include "Model/WorldNode.h"

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
static void setLayerSortIndex(Model::LayerNode& layerNode, int sortIndex) {
  auto layer = layerNode.layer();
  layer.setSortIndex(sortIndex);
  layerNode.setLayer(layer);
}

TEST_CASE_METHOD(
  MapDocumentTest, "LayerNodeTest.defaultLayerSortIndexImmutable", "[LayerNodesTest]") {
  Model::LayerNode* defaultLayerNode = document->world()->defaultLayer();
  setLayerSortIndex(*defaultLayerNode, 555);

  CHECK(defaultLayerNode->layer().sortIndex() == Model::Layer::defaultLayerSortIndex());
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.renameLayer", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  Model::LayerNode* layerNode = new Model::LayerNode(Model::Layer("test1"));
  addNode(*document, document->world(), layerNode);
  CHECK(layerNode->name() == "test1");

  document->renameLayer(layerNode, "test2");
  CHECK(layerNode->name() == "test2");

  document->undoCommand();
  CHECK(layerNode->name() == "test1");
}

TEST_CASE_METHOD(
  MapDocumentTest, "LayerNodeTest.duplicateObjectGoesIntoSourceLayer", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
  Model::LayerNode* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
  addNode(*document, document->world(), layerNode1);
  addNode(*document, document->world(), layerNode2);

  document->setCurrentLayer(layerNode1);
  Model::EntityNode* entity = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
  CHECK(entity->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1);

  document->setCurrentLayer(layerNode2);
  document->selectNode(entity);
  document->duplicateObjects(); // the duplicate should stay in layer1

  REQUIRE(document->selectedNodes().entityCount() == 1);
  Model::EntityNode* entityClone = document->selectedNodes().entities().at(0);
  CHECK(entityClone->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2);
  CHECK(document->currentLayer() == layerNode2);
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.newGroupGoesIntoSourceLayer", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
  Model::LayerNode* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
  addNode(*document, document->world(), layerNode1);
  addNode(*document, document->world(), layerNode2);

  document->setCurrentLayer(layerNode1);
  Model::EntityNode* entity = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
  CHECK(entity->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1);

  document->setCurrentLayer(layerNode2);
  document->selectNode(entity);
  Model::GroupNode* newGroup =
    document->groupSelection("Group in Layer 1"); // the new group should stay in layer1

  CHECK(entity->parent() == newGroup);
  CHECK(Model::findContainingLayer(entity) == layerNode1);
  CHECK(Model::findContainingLayer(newGroup) == layerNode1);
  CHECK(document->currentLayer() == layerNode2);
}

TEST_CASE_METHOD(
  MapDocumentTest, "LayerNodeTest.newObjectsInHiddenLayerAreVisible", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
  Model::LayerNode* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
  addNode(*document, document->world(), layerNode1);
  addNode(*document, document->world(), layerNode2);

  document->setCurrentLayer(layerNode1);

  // Create an entity in layer1
  Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
  CHECK(entity1->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1u);

  CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(entity1->visible());

  // Hide layer1. If any nodes in the layer were Visibility_Shown they would be reset to
  // Visibility_Inherited
  document->hideLayers({layerNode1});

  CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(!entity1->visible());

  // Create another entity in layer1. It will be visible, while entity1 will still be hidden.
  Model::EntityNode* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
  CHECK(entity2->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2u);

  CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(!entity1->visible());
  CHECK(entity2->visibilityState() == Model::VisibilityState::Shown);
  CHECK(entity2->visible());

  // Change to layer2. This hides all objects in layer1
  document->setCurrentLayer(layerNode2);

  CHECK(document->currentLayer() == layerNode2);
  CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(!entity1->visible());
  CHECK(entity2->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(!entity2->visible());

  // Undo (Switch current layer back to layer1)
  document->undoCommand();

  CHECK(document->currentLayer() == layerNode1);
  CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(!entity1->visible());
  CHECK(entity2->visibilityState() == Model::VisibilityState::Shown);
  CHECK(entity2->visible());

  // Undo (entity2 creation)
  document->undoCommand();

  CHECK(layerNode1->childCount() == 1u);
  CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(!entity1->visible());

  // Undo (hiding layer1)
  document->undoCommand();

  CHECK(entity1->visibilityState() == Model::VisibilityState::Inherited);
  CHECK(entity1->visible());
}

TEST_CASE_METHOD(
  MapDocumentTest, "LayerNodeTest.duplicatedObjectInHiddenLayerIsVisible", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  Model::LayerNode* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
  addNode(*document, document->world(), layerNode1);

  document->setCurrentLayer(layerNode1);
  document->hideLayers({layerNode1});

  // Create entity1 and brush1 in the hidden layer1
  Model::EntityNode* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
  Model::BrushNode* brush1 = createBrushNode();
  addNode(*document, document->parentForNodes(), brush1);

  CHECK(entity1->parent() == layerNode1);
  CHECK(brush1->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2u);

  CHECK(entity1->visibilityState() == Model::VisibilityState::Shown);
  CHECK(brush1->visibilityState() == Model::VisibilityState::Shown);
  CHECK(entity1->visible());
  CHECK(brush1->visible());

  document->selectNodes({entity1, brush1});

  // Duplicate entity1 and brush1
  document->duplicateObjects();
  REQUIRE(document->selectedNodes().entityCount() == 1u);
  REQUIRE(document->selectedNodes().brushCount() == 1u);
  Model::EntityNode* entity2 = document->selectedNodes().entities().front();
  Model::BrushNode* brush2 = document->selectedNodes().brushes().front();

  CHECK(entity2 != entity1);
  CHECK(brush2 != brush1);

  CHECK(entity2->visibilityState() == Model::VisibilityState::Shown);
  CHECK(entity2->visible());

  CHECK(brush2->visibilityState() == Model::VisibilityState::Shown);
  CHECK(brush2->visible());
}

TEST_CASE_METHOD(
  MapDocumentTest, "LayerNodeTest.newObjectsInLockedLayerAreUnlocked", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  auto* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
  auto* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
  addNode(*document, document->world(), layerNode1);
  addNode(*document, document->world(), layerNode2);

  document->setCurrentLayer(layerNode1);

  // Create an entity in layer1
  auto* entity1 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
  CHECK(entity1->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 1u);

  CHECK(entity1->lockState() == Model::LockState::Inherited);
  CHECK(!entity1->locked());

  // Lock layer1
  document->lock({layerNode1});

  CHECK(entity1->lockState() == Model::LockState::Inherited);
  CHECK(entity1->locked());

  // Create another entity in layer1. It will be unlocked, while entity1 will still be locked
  // (inherited).
  auto* entity2 = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());
  CHECK(entity2->parent() == layerNode1);
  CHECK(layerNode1->childCount() == 2u);

  CHECK(entity1->lockState() == Model::LockState::Inherited);
  CHECK(entity1->locked());
  CHECK(entity2->lockState() == Model::LockState::Unlocked);
  CHECK(!entity2->locked());

  // Change to layer2. This causes the Lock_Unlocked objects in layer1 to be degraded to
  // Lock_Inherited (i.e. everything in layer1 becomes locked)
  document->setCurrentLayer(layerNode2);

  CHECK(document->currentLayer() == layerNode2);
  CHECK(entity1->lockState() == Model::LockState::Inherited);
  CHECK(entity1->locked());
  CHECK(entity2->lockState() == Model::LockState::Inherited);
  CHECK(entity2->locked());

  // Undo (Switch current layer back to layer1)
  document->undoCommand();

  CHECK(document->currentLayer() == layerNode1);
  CHECK(entity1->lockState() == Model::LockState::Inherited);
  CHECK(entity1->locked());
  CHECK(entity2->lockState() == Model::LockState::Unlocked);
  CHECK(!entity2->locked());

  // Undo entity2 creation
  document->undoCommand();

  CHECK(layerNode1->childCount() == 1u);
  CHECK(entity1->lockState() == Model::LockState::Inherited);
  CHECK(entity1->locked());

  // Undo locking layer1
  document->undoCommand();

  CHECK(entity1->lockState() == Model::LockState::Inherited);
  CHECK(!entity1->locked());
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.moveLayer", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  auto* layerNode0 = new Model::LayerNode(Model::Layer("layer0"));
  auto* layerNode1 = new Model::LayerNode(Model::Layer("layer1"));
  auto* layerNode2 = new Model::LayerNode(Model::Layer("layer2"));

  setLayerSortIndex(*layerNode0, 0);
  setLayerSortIndex(*layerNode1, 1);
  setLayerSortIndex(*layerNode2, 2);

  addNode(*document, document->world(), layerNode0);
  addNode(*document, document->world(), layerNode1);
  addNode(*document, document->world(), layerNode2);

  SECTION("check canMoveLayer") {
    // defaultLayer() can never be moved
    CHECK(!document->canMoveLayer(document->world()->defaultLayer(), 1));
    CHECK(document->canMoveLayer(layerNode0, 0));
    CHECK(!document->canMoveLayer(layerNode0, -1));
    CHECK(document->canMoveLayer(layerNode0, 1));
    CHECK(document->canMoveLayer(layerNode0, 2));
    CHECK(!document->canMoveLayer(layerNode0, 3));
  }

  SECTION("moveLayer by 0 has no effect") {
    document->moveLayer(layerNode0, 0);
    CHECK(layerNode0->layer().sortIndex() == 0);
  }
  SECTION("moveLayer by invalid negative amount is clamped") {
    document->moveLayer(layerNode0, -1000);
    CHECK(layerNode0->layer().sortIndex() == 0);
  }
  SECTION("moveLayer by 1") {
    document->moveLayer(layerNode0, 1);
    CHECK(layerNode1->layer().sortIndex() == 0);
    CHECK(layerNode0->layer().sortIndex() == 1);
    CHECK(layerNode2->layer().sortIndex() == 2);
  }
  SECTION("moveLayer by 2") {
    document->moveLayer(layerNode0, 2);
    CHECK(layerNode1->layer().sortIndex() == 0);
    CHECK(layerNode2->layer().sortIndex() == 1);
    CHECK(layerNode0->layer().sortIndex() == 2);
  }
  SECTION("moveLayer by invalid positive amount is clamped") {
    document->moveLayer(layerNode0, 1000);
    CHECK(layerNode1->layer().sortIndex() == 0);
    CHECK(layerNode2->layer().sortIndex() == 1);
    CHECK(layerNode0->layer().sortIndex() == 2);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.moveSelectionToLayer", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  auto* customLayer = new Model::LayerNode(Model::Layer("layer"));
  addNode(*document, document->world(), customLayer);

  auto* defaultLayer = document->world()->defaultLayer();

  GIVEN("A top level node") {
    using CreateNode = std::function<Model::Node*(const MapDocumentTest&)>;
    const auto createNode = GENERATE_COPY(
      CreateNode{[](const auto& test) {
        auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
        groupNode->addChild(test.createBrushNode());
        return groupNode;
      }},
      CreateNode{[](const auto&) {
        return new Model::EntityNode{Model::Entity{}};
      }},
      CreateNode{[](const auto& test) {
        return test.createBrushNode();
      }},
      CreateNode{[](const auto& test) {
        return test.createPatchNode();
      }});

    auto* node = createNode(*this);
    document->addNodes({{document->parentForNodes(), {node}}});

    REQUIRE(Model::findContainingLayer(node) == defaultLayer);

    WHEN("The node is moved to another layer") {
      document->selectNode(node);
      document->moveSelectionToLayer(customLayer);

      THEN("The group node is in the target layer") {
        CHECK(Model::findContainingLayer(node) == customLayer);

        AND_THEN("The node is selected") {
          CHECK(document->selectedNodes().nodes() == std::vector<Model::Node*>{node});
        }
      }

      AND_WHEN("The operation is undone") {
        document->undoCommand();

        THEN("The node is back in the original layer") {
          CHECK(Model::findContainingLayer(node) == defaultLayer);

          AND_THEN("The node is selected") {
            CHECK(document->selectedNodes().nodes() == std::vector<Model::Node*>{node});
          }
        }
      }
    }
  }

  GIVEN("A brush entity node") {
    auto* entityNode = new Model::EntityNode{Model::Entity{}};
    auto* childNode1 = createBrushNode();
    auto* childNode2 = createPatchNode();

    entityNode->addChildren({childNode1, childNode2});
    document->addNodes({{document->parentForNodes(), {entityNode}}});

    REQUIRE(Model::findContainingLayer(entityNode) == defaultLayer);

    WHEN("Any child node is selected and moved to another layer") {
      // clang-format off
      const auto [selectChild1, selectChild2] = GENERATE(
        std::make_tuple(true, true),
        std::make_tuple(true, false),
        std::make_tuple(false, true)
      );
      // clang-format on

      if (selectChild1) {
        document->selectNode(childNode1);
      }
      if (selectChild2) {
        document->selectNode(childNode2);
      }

      const auto selectedNodes = document->selectedNodes().nodes();
      document->moveSelectionToLayer(customLayer);

      THEN("The brush entity node is moved to the target layer") {
        CHECK(Model::findContainingLayer(entityNode) == customLayer);
        CHECK(childNode1->parent() == entityNode);
        CHECK(childNode2->parent() == entityNode);

        AND_THEN("The child nodes are selected") {
          CHECK(document->selectedNodes().nodes() == entityNode->children());
        }
      }

      AND_WHEN("The operation is undone") {
        document->undoCommand();

        THEN("The brush entity node is back in the original layer") {
          CHECK(Model::findContainingLayer(entityNode) == defaultLayer);
          CHECK(childNode1->parent() == entityNode);
          CHECK(childNode2->parent() == entityNode);

          AND_THEN("The originally selected nodes are selected") {
            CHECK_THAT(
              document->selectedNodes().nodes(), Catch::Matchers::UnorderedEquals(selectedNodes));
          }
        }
      }
    }
  }
}

TEST_CASE_METHOD(MapDocumentTest, "LayerNodeTest.setCurrentLayerCollation", "[LayerNodesTest]") {
  // delete default brush
  document->selectAllNodes();
  document->deleteObjects();

  auto* defaultLayerNode = document->world()->defaultLayer();
  auto* layerNode1 = new Model::LayerNode(Model::Layer("test1"));
  auto* layerNode2 = new Model::LayerNode(Model::Layer("test2"));
  addNode(*document, document->world(), layerNode1);
  addNode(*document, document->world(), layerNode2);
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
} // namespace View
} // namespace TrenchBroom
