/*
 Copyright (C) 2010 Kristian Duske

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

#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentLayerToLayer")
{
  auto* layer1 = new mdl::LayerNode{mdl::Layer{"Layer 1"}};
  document->addNodes({{document->world(), {layer1}}});

  auto* layer2 = new mdl::LayerNode{mdl::Layer{"Layer 2"}};
  document->addNodes({{document->world(), {layer2}}});

  CHECK_FALSE(document->reparentNodes({{layer2, {layer1}}}));
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentBetweenLayers")
{
  auto* oldParent = new mdl::LayerNode{mdl::Layer{"Layer 1"}};
  document->addNodes({{document->world(), {oldParent}}});

  auto* newParent = new mdl::LayerNode{mdl::Layer{"Layer 2"}};
  document->addNodes({{document->world(), {newParent}}});

  auto* entity = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{oldParent, {entity}}});

  assert(entity->parent() == oldParent);
  CHECK(document->reparentNodes({{newParent, {entity}}}));
  CHECK(entity->parent() == newParent);

  document->undoCommand();
  CHECK(entity->parent() == oldParent);
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentGroupToItself")
{
  auto* group = new mdl::GroupNode{mdl::Group{"Group"}};
  document->addNodes({{document->parentForNodes(), {group}}});

  CHECK_FALSE(document->reparentNodes({{group, {group}}}));
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.reparentGroupToChild")
{
  auto* outer = new mdl::GroupNode{mdl::Group{"Outer"}};
  document->addNodes({{document->parentForNodes(), {outer}}});

  auto* inner = new mdl::GroupNode{mdl::Group{"Inner"}};
  document->addNodes({{outer, {inner}}});

  CHECK_FALSE(document->reparentNodes({{inner, {outer}}}));
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.removeEmptyGroup")
{
  auto* group = new mdl::GroupNode{mdl::Group{"Group"}};
  document->addNodes({{document->parentForNodes(), {group}}});

  auto* entity = new mdl::EntityNode{mdl::Entity{}};
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
  auto* outer = new mdl::GroupNode{mdl::Group{"Outer"}};
  document->addNodes({{document->parentForNodes(), {outer}}});

  auto* inner = new mdl::GroupNode{mdl::Group{"Inner"}};
  document->addNodes({{outer, {inner}}});

  auto* entity = new mdl::EntityNode{mdl::Entity{}};
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
  auto* entity = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entity}}});

  auto* brush = createBrushNode();
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
  auto* group = new mdl::GroupNode{mdl::Group{"Group"}};
  document->addNodes({{document->parentForNodes(), {group}}});

  auto* entity = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{group, {entity}}});

  auto* brush = createBrushNode();
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

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.resetLinkIds")
{
  auto* nestedBrushNode = createBrushNode();
  auto* nestedEntityNode = new mdl::EntityNode{mdl::Entity{}};

  document->addNodes({{document->parentForNodes(), {nestedBrushNode, nestedEntityNode}}});
  document->selectNodes({nestedBrushNode, nestedEntityNode});

  auto* nestedGroupNode = document->groupSelection("nested");

  document->deselectAll();
  document->selectNodes({nestedGroupNode});

  auto* linkedNestedGroupNode = document->createLinkedDuplicate();

  auto* brushNode = createBrushNode();
  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  auto* entityBrushNode = createBrushNode();
  entityNode->addChild(entityBrushNode);

  document->addNodes({{document->parentForNodes(), {brushNode, entityNode}}});

  document->selectNodes({brushNode, entityNode, nestedGroupNode});
  auto* groupNode = document->groupSelection("group");

  document->deselectAll();
  document->selectNodes({groupNode});

  auto* linkedGroupNode = document->createLinkedDuplicate();
  auto* linkedGroupNode2 = document->createLinkedDuplicate();

  document->deselectAll();

  const auto originalNestedBrushLinkId = nestedBrushNode->linkId();
  const auto originalBrushLinkId = brushNode->linkId();
  const auto originalEntityLinkId = entityNode->linkId();
  const auto originalEntityBrushLinkId = entityBrushNode->linkId();

  REQUIRE_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
  REQUIRE_THAT(*linkedGroupNode, mdl::MatchesNode(*groupNode));
  REQUIRE_THAT(*linkedGroupNode2, mdl::MatchesNode(*groupNode));

  SECTION("Moving a brush entity to the world resets its link IDs")
  {
    REQUIRE(document->reparentNodes({{document->parentForNodes(), {entityNode}}}));

    CHECK(entityNode->linkId() != originalEntityLinkId);
    CHECK(entityBrushNode->linkId() != originalEntityBrushLinkId);

    CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
    CHECK_THAT(*linkedGroupNode, mdl::MatchesNode(*groupNode));
    CHECK_THAT(*linkedGroupNode2, mdl::MatchesNode(*groupNode));
  }

  SECTION("Moving objects out of a nested group into the container resets their link IDs")
  {
    REQUIRE(document->reparentNodes({{groupNode, {nestedBrushNode}}}));
    CHECK(nestedBrushNode->linkId() != originalNestedBrushLinkId);

    CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
    CHECK_THAT(*linkedGroupNode, mdl::MatchesNode(*groupNode));
    CHECK_THAT(*linkedGroupNode2, mdl::MatchesNode(*groupNode));
  }

  SECTION("Moving objects into a nested linked group keeps their link IDs")
  {
    REQUIRE(document->reparentNodes({{nestedGroupNode, {brushNode}}}));
    CHECK(brushNode->linkId() == originalBrushLinkId);

    CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
    CHECK_THAT(*linkedGroupNode, mdl::MatchesNode(*groupNode));
    CHECK_THAT(*linkedGroupNode2, mdl::MatchesNode(*groupNode));
  }

  SECTION("Grouping objects within a linked group keeps their link IDs")
  {
    document->selectNodes({entityNode});
    document->groupSelection("new group");
    CHECK(entityNode->linkId() == originalEntityLinkId);
    CHECK(entityBrushNode->linkId() == originalEntityBrushLinkId);

    CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
    CHECK_THAT(*linkedGroupNode, mdl::MatchesNode(*groupNode));
    CHECK_THAT(*linkedGroupNode2, mdl::MatchesNode(*groupNode));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.updateLinkedGroups")
{
  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
  auto* brushNode = createBrushNode();
  groupNode->addChild(brushNode);
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  document->selectNodes({linkedGroupNode});
  document->translate(vm::vec3d{32, 0, 0});
  document->deselectAll();

  SECTION("Move node into group node")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
    document->addNodes({{document->parentForNodes(), {entityNode}}});

    REQUIRE(groupNode->childCount() == 1u);
    REQUIRE(linkedGroupNode->childCount() == 1u);

    document->reparentNodes({{groupNode, {entityNode}}});

    CHECK(groupNode->childCount() == 2u);
    CHECK(linkedGroupNode->childCount() == 2u);

    auto* linkedEntityNode =
      dynamic_cast<mdl::EntityNode*>(linkedGroupNode->children().back());
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
    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
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
  auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"outer"}};
  document->addNodes({{document->parentForNodes(), {outerGroupNode}}});

  document->openGroup(outerGroupNode);

  auto* outerEntityNode = new mdl::EntityNode{mdl::Entity{}};
  auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"inner"}};
  document->addNodes({{document->parentForNodes(), {outerEntityNode, innerGroupNode}}});

  document->openGroup(innerGroupNode);

  auto* innerEntityNode = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {innerEntityNode}}});

  document->closeGroup();
  document->closeGroup();

  document->selectNodes({outerGroupNode});

  auto* linkedOuterGroupNode = document->createLinkedDuplicate();
  REQUIRE(
    outerGroupNode->children()
    == std::vector<mdl::Node*>{outerEntityNode, innerGroupNode});
  REQUIRE_THAT(*linkedOuterGroupNode, mdl::MatchesNode(*outerGroupNode));

  document->deselectAll();

  document->reparentNodes({{document->parentForNodes(), {innerEntityNode}}});
  CHECK(outerGroupNode->children() == std::vector<mdl::Node*>{outerEntityNode});
  CHECK_THAT(*linkedOuterGroupNode, mdl::MatchesNode(*outerGroupNode));

  document->undoCommand();
  CHECK(
    outerGroupNode->children()
    == std::vector<mdl::Node*>{outerEntityNode, innerGroupNode});
  REQUIRE_THAT(*linkedOuterGroupNode, mdl::MatchesNode(*outerGroupNode));

  document->redoCommand();
  CHECK(outerGroupNode->children() == std::vector<mdl::Node*>{outerEntityNode});
  CHECK_THAT(*linkedOuterGroupNode, mdl::MatchesNode(*outerGroupNode));
}

TEST_CASE_METHOD(MapDocumentTest, "ReparentNodesTest.updateLinkedGroupsFails")
{
  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
  document->addNodes({{document->parentForNodes(), {groupNode}}});

  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  // adding a brush to the linked group node will fail because it will go out of world
  // bounds
  document->selectNodes({linkedGroupNode});
  document->translate(document->worldBounds().max);
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
  auto* groupNode = new mdl::GroupNode{mdl::Group{"group"}};
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

} // namespace tb::ui
