/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentLayerToLayer")
{
  Model::LayerNode* layer1 = new Model::LayerNode(Model::Layer("Layer 1"));
  document->addNodes({{document->world(), {layer1}}});

  Model::LayerNode* layer2 = new Model::LayerNode(Model::Layer("Layer 2"));
  document->addNodes({{document->world(), {layer2}}});

  CHECK_FALSE(document->reparentNodes({{layer2, {layer1}}}));
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentBetweenLayers")
{
  Model::LayerNode* oldParent = new Model::LayerNode(Model::Layer("Layer 1"));
  document->addNodes({{document->world(), {oldParent}}});

  Model::LayerNode* newParent = new Model::LayerNode(Model::Layer("Layer 2"));
  document->addNodes({{document->world(), {newParent}}});

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{oldParent, {entity}}});

  assert(entity->parent() == oldParent);
  CHECK(document->reparentNodes({{newParent, {entity}}}));
  CHECK(entity->parent() == newParent);

  document->undoCommand();
  CHECK(entity->parent() == oldParent);
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentGroupToItself")
{
  Model::GroupNode* group = new Model::GroupNode(Model::Group("Group"));
  document->addNodes({{document->parentForNodes(), {group}}});

  CHECK_FALSE(document->reparentNodes({{group, {group}}}));
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentGroupToChild")
{
  Model::GroupNode* outer = new Model::GroupNode(Model::Group("Outer"));
  document->addNodes({{document->parentForNodes(), {outer}}});

  Model::GroupNode* inner = new Model::GroupNode(Model::Group("Inner"));
  document->addNodes({{outer, {inner}}});

  CHECK_FALSE(document->reparentNodes({{inner, {outer}}}));
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.removeEmptyGroup")
{
  Model::GroupNode* group = new Model::GroupNode(Model::Group("Group"));
  document->addNodes({{document->parentForNodes(), {group}}});

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{group, {entity}}});

  CHECK(document->reparentNodes({{document->parentForNodes(), {entity}}}));
  CHECK(entity->parent() == document->parentForNodes());
  CHECK(group->parent() == nullptr);

  document->undoCommand();
  CHECK(group->parent() == document->parentForNodes());
  CHECK(entity->parent() == group);
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.recursivelyRemoveEmptyGroups")
{
  Model::GroupNode* outer = new Model::GroupNode(Model::Group("Outer"));
  document->addNodes({{document->parentForNodes(), {outer}}});

  Model::GroupNode* inner = new Model::GroupNode(Model::Group("Inner"));
  document->addNodes({{outer, {inner}}});

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{inner, {entity}}});

  CHECK(document->reparentNodes({{document->parentForNodes(), {entity}}}));
  CHECK(entity->parent() == document->parentForNodes());
  CHECK(inner->parent() == nullptr);
  CHECK(outer->parent() == nullptr);

  document->undoCommand();
  CHECK(outer->parent() == document->parentForNodes());
  CHECK(inner->parent() == outer);
  CHECK(entity->parent() == inner);
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.removeEmptyEntity")
{
  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entity}}});

  Model::BrushNode* brush = createBrushNode();
  document->addNodes({{entity, {brush}}});

  CHECK(document->reparentNodes({{document->parentForNodes(), {brush}}}));
  CHECK(brush->parent() == document->parentForNodes());
  CHECK(entity->parent() == nullptr);

  document->undoCommand();
  CHECK(entity->parent() == document->parentForNodes());
  CHECK(brush->parent() == entity);
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.removeEmptyGroupAndEntity")
{
  Model::GroupNode* group = new Model::GroupNode(Model::Group("Group"));
  document->addNodes({{document->parentForNodes(), {group}}});

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{group, {entity}}});

  Model::BrushNode* brush = createBrushNode();
  document->addNodes({{entity, {brush}}});

  CHECK(document->reparentNodes({{document->parentForNodes(), {brush}}}));
  CHECK(brush->parent() == document->parentForNodes());
  CHECK(group->parent() == nullptr);
  CHECK(entity->parent() == nullptr);

  document->undoCommand();
  CHECK(group->parent() == document->parentForNodes());
  CHECK(entity->parent() == group);
  CHECK(brush->parent() == entity);
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.updateLinkedGroups")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  document->selectNodes({linkedGroupNode});
  document->translateObjects(vm::vec3(32.0, 0.0, 0.0));
  document->deselectAll();

  SECTION("Move node into group node")
  {
    auto* entityNode = new Model::EntityNode{Model::Entity{}};
    document->addNodes({{document->parentForNodes(), {entityNode}}});

    REQUIRE(groupNode->childCount() == 1u);
    REQUIRE(linkedGroupNode->childCount() == 1u);

    document->reparentNodes({{groupNode, {entityNode}}});

    CHECK(groupNode->childCount() == 2u);
    CHECK(linkedGroupNode->childCount() == 2u);

    auto* linkedEntityNode =
      dynamic_cast<Model::EntityNode*>(linkedGroupNode->children().back());
    CHECK(linkedEntityNode != nullptr);

    CHECK(
      linkedEntityNode->physicalBounds()
      == entityNode->physicalBounds().transform(
        linkedGroupNode->group().transformation()));

    document->undoCommand();

    CHECK(entityNode->parent() == document->parentForNodes());
    CHECK(groupNode->childCount() == 1u);
    CHECK(linkedGroupNode->childCount() == 1u);
  }

  SECTION("Move node out of group node")
  {
    auto* entityNode = new Model::EntityNode{Model::Entity{}};
    document->addNodes({{groupNode, {entityNode}}});

    REQUIRE(groupNode->childCount() == 2u);
    REQUIRE(linkedGroupNode->childCount() == 2u);

    document->reparentNodes({{document->parentForNodes(), {entityNode}}});

    CHECK(entityNode->parent() == document->parentForNodes());
    CHECK(groupNode->childCount() == 1u);
    CHECK(linkedGroupNode->childCount() == 1u);

    document->undoCommand();

    CHECK(entityNode->parent() == groupNode);
    CHECK(groupNode->childCount() == 2u);
    CHECK(linkedGroupNode->childCount() == 2u);
  }
}

TEST_CASE_METHOD(
  MapDocumentTest, "RemoveNodesTest.updateLinkedGroupsAfterRecursiveDelete")
{
  auto* outerGroupNode = new Model::GroupNode(Model::Group("outer"));
  document->addNodes({{document->parentForNodes(), {outerGroupNode}}});

  document->openGroup(outerGroupNode);

  auto* outerEntityNode = new Model::EntityNode{Model::Entity{}};
  auto* innerGroupNode = new Model::GroupNode{Model::Group{"inner"}};
  document->addNodes({{document->parentForNodes(), {outerEntityNode, innerGroupNode}}});

  document->openGroup(innerGroupNode);

  auto* innerEntityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {innerEntityNode}}});

  document->closeGroup();
  document->closeGroup();

  document->selectNodes({outerGroupNode});

  auto* linkedOuterGroupNode = document->createLinkedDuplicate();

  document->deselectAll();

  document->reparentNodes({{document->parentForNodes(), {innerEntityNode}}});
  REQUIRE(outerGroupNode->children() == std::vector<Model::Node*>{outerEntityNode});
  CHECK(linkedOuterGroupNode->childCount() == outerGroupNode->childCount());

  document->undoCommand();
  CHECK(linkedOuterGroupNode->childCount() == outerGroupNode->childCount());

  document->redoCommand();
  CHECK(linkedOuterGroupNode->childCount() == outerGroupNode->childCount());
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.updateLinkedGroupsFails")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  // adding a brush to the linked group node will fail because it will go out of world
  // bounds
  document->selectNodes({linkedGroupNode});
  document->translateObjects(document->worldBounds().max);
  document->deselectAll();

  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  CHECK_FALSE(document->reparentNodes({{groupNode, {brushNode}}}));

  CHECK(groupNode->childCount() == 0u);
  CHECK(linkedGroupNode->childCount() == 0u);
}

TEST_CASE_METHOD(
  MapDocumentTest,
  "ReparentNodesTest.updateLinkedGroupsFailsAfterMovingNodeBetweenLinkedGroups")
{
  auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);

  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  CHECK_FALSE(document->reparentNodes({{linkedGroupNode, {brushNode}}}));

  CHECK(groupNode->childCount() == 1u);
  CHECK(linkedGroupNode->childCount() == 1u);
}
} // namespace View
} // namespace TrenchBroom
