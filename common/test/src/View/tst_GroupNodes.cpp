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

#include "Assets/EntityDefinition.h"
#include "Error.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"
#include "View/MapDocumentTest.h"
#include "View/PasteType.h"

#include <kdl/result.h>

#include <vecmath/mat_ext.h>

#include <functional>
#include <set>

#include "CatchUtils/Matchers.h"

#include "Catch2.h"

namespace TrenchBroom::View
{

namespace
{
bool hasEmptyName(const std::vector<std::string>& names)
{
  return kdl::any_of(names, [](const auto& s) { return s.empty(); });
}

} // namespace

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createEmptyGroup")
{
  CHECK(document->groupSelection("test") == nullptr);
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createGroupWithOneNode")
{
  using CreateNode = std::function<Model::Node*(const MapDocumentTest&)>;
  auto createNode = GENERATE_COPY(
    CreateNode{[](const auto& test) { return test.createBrushNode(); }},
    CreateNode{[](const auto& test) { return test.createPatchNode(); }});

  auto* node = createNode(*this);
  document->addNodes({{document->parentForNodes(), {node}}});
  document->selectNodes({node});

  auto* groupNode = document->groupSelection("test");
  CHECK(groupNode != nullptr);

  CHECK(node->parent() == groupNode);
  CHECK(groupNode->selected());
  CHECK_FALSE(node->selected());

  document->undoCommand();
  CHECK(groupNode->parent() == nullptr);
  CHECK(node->parent() == document->parentForNodes());
  CHECK(node->selected());
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createGroupWithPartialBrushEntity")
{
  auto* childNode1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {childNode1}}});

  auto* childNode2 = createPatchNode();
  document->addNodes({{document->parentForNodes(), {childNode2}}});

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->reparentNodes({{entityNode, {childNode1, childNode2}}});

  document->selectNodes({childNode1});

  Model::GroupNode* groupNode = document->groupSelection("test");
  CHECK(groupNode != nullptr);

  CHECK(childNode1->parent() == entityNode);
  CHECK(childNode2->parent() == entityNode);
  CHECK(entityNode->parent() == groupNode);
  CHECK(groupNode->selected());
  CHECK_FALSE(childNode1->selected());

  document->undoCommand();
  CHECK(groupNode->parent() == nullptr);
  CHECK(childNode1->parent() == entityNode);
  CHECK(childNode2->parent() == entityNode);
  CHECK(entityNode->parent() == document->parentForNodes());
  CHECK_FALSE(groupNode->selected());
  CHECK(childNode1->selected());
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createGroupWithFullBrushEntity")
{
  auto* childNode1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {childNode1}}});

  auto* childNode2 = createPatchNode();
  document->addNodes({{document->parentForNodes(), {childNode2}}});

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->reparentNodes({{entityNode, {childNode1, childNode2}}});

  document->selectNodes({childNode1, childNode2});

  auto* groupNode = document->groupSelection("test");
  CHECK(groupNode != nullptr);

  CHECK(childNode1->parent() == entityNode);
  CHECK(childNode2->parent() == entityNode);
  CHECK(entityNode->parent() == groupNode);
  CHECK(groupNode->selected());
  CHECK_FALSE(childNode1->selected());
  CHECK_FALSE(childNode2->selected());

  document->undoCommand();
  CHECK(groupNode->parent() == nullptr);
  CHECK(childNode1->parent() == entityNode);
  CHECK(childNode2->parent() == entityNode);
  CHECK(entityNode->parent() == document->parentForNodes());
  CHECK_FALSE(groupNode->selected());
  CHECK(childNode1->selected());
  CHECK(childNode2->selected());
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.undoMoveGroupContainingBrushEntity")
{
  // Test for issue #1715

  auto* brushNode1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->reparentNodes({{entityNode, {brushNode1}}});

  document->selectNodes({brushNode1});

  auto* groupNode = document->groupSelection("test");
  CHECK(groupNode->selected());

  CHECK(document->translateObjects(vm::vec3(16, 0, 0)));

  CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));

  document->undoCommand();

  CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.rotateGroupContainingBrushEntity")
{
  // Test for issue #1754

  auto* brushNode1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode1}}});

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->reparentNodes({{entityNode, {brushNode1}}});

  document->selectNodes({brushNode1});

  auto* groupNode = document->groupSelection("test");
  CHECK(groupNode->selected());

  CHECK_FALSE(entityNode->entity().hasProperty("origin"));
  CHECK(document->rotateObjects(
    vm::vec3::zero(), vm::vec3::pos_z(), static_cast<FloatType>(10.0)));
  CHECK_FALSE(entityNode->entity().hasProperty("origin"));

  document->undoCommand();

  CHECK_FALSE(entityNode->entity().hasProperty("origin"));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.renameGroup")
{
  auto* brushNode1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->selectNodes({brushNode1});

  auto* groupNode = document->groupSelection("test");

  document->renameGroups("abc");
  CHECK(groupNode->name() == "abc");

  document->undoCommand();
  CHECK(groupNode->name() == "test");

  document->redoCommand();
  CHECK(groupNode->name() == "abc");
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.duplicateNodeInGroup")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  document->openGroup(groupNode);

  document->selectNodes({brushNode});
  document->duplicateObjects();

  auto* brushNodeCopy = document->selectedNodes().brushes().at(0u);
  CHECK(brushNodeCopy->parent() == groupNode);
  CHECK(brushNodeCopy->linkId() != brushNode->linkId());
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.duplicateLinkedGroup")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

  document->duplicateObjects();

  auto* groupNodeCopy = document->selectedNodes().groups().at(0u);
  CHECK(groupNodeCopy->linkId() == groupNode->linkId());
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.duplicateNodeInLinkedGroup")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

  document->openGroup(groupNode);

  document->selectNodes({brushNode});
  document->duplicateObjects();

  auto* brushNodeCopy = document->selectedNodes().brushes().at(0u);
  CHECK(brushNodeCopy->linkId() != brushNode->linkId());
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.duplicateGroupInLinkedGroup")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* innerGroupNode = document->groupSelection("inner");
  REQUIRE(innerGroupNode != nullptr);

  auto* outerGroupNode = document->groupSelection("outer");
  REQUIRE(outerGroupNode != nullptr);

  auto* linkedOuterGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedOuterGroupNode->linkId() == outerGroupNode->linkId());

  auto* linkedInnerGroupNode =
    dynamic_cast<Model::GroupNode*>(linkedOuterGroupNode->children().front());
  REQUIRE(linkedInnerGroupNode != nullptr);
  REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

  document->openGroup(outerGroupNode);

  document->selectNodes({innerGroupNode});
  document->duplicateObjects();

  auto* innerGroupNodeCopy = document->selectedNodes().groups().at(0u);
  CHECK(innerGroupNodeCopy->linkId() == innerGroupNode->linkId());
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupInnerGroup")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2050
  auto* outerEntityNode1 = new Model::EntityNode{Model::Entity{}};
  auto* outerEntityNode2 = new Model::EntityNode{Model::Entity{}};
  auto* innerEntityNode1 = new Model::EntityNode{Model::Entity{}};
  auto* innerEntityNode2 = new Model::EntityNode{Model::Entity{}};

  document->addNodes({{document->parentForNodes(), {innerEntityNode1}}});
  document->addNodes({{document->parentForNodes(), {innerEntityNode2}}});
  document->selectNodes({innerEntityNode1, innerEntityNode2});

  auto* innerGroupNode = document->groupSelection("Inner");

  document->deselectAll();
  document->addNodes({{document->parentForNodes(), {outerEntityNode1}}});
  document->addNodes({{document->parentForNodes(), {outerEntityNode2}}});
  document->selectNodes({innerGroupNode, outerEntityNode1, outerEntityNode2});

  auto* outerGroupNode = document->groupSelection("Outer");
  document->deselectAll();

  // check our assumptions
  CHECK(outerGroupNode->childCount() == 3u);
  CHECK(innerGroupNode->childCount() == 2u);

  CHECK(outerGroupNode->parent() == document->currentLayer());

  CHECK(outerEntityNode1->parent() == outerGroupNode);
  CHECK(outerEntityNode2->parent() == outerGroupNode);
  CHECK(innerGroupNode->parent() == outerGroupNode);

  CHECK(innerEntityNode1->parent() == innerGroupNode);
  CHECK(innerEntityNode2->parent() == innerGroupNode);

  CHECK(document->currentGroup() == nullptr);
  CHECK(!outerGroupNode->opened());
  CHECK(!innerGroupNode->opened());

  CHECK(Model::findOutermostClosedGroup(innerEntityNode1) == outerGroupNode);
  CHECK(Model::findOutermostClosedGroup(outerEntityNode1) == outerGroupNode);

  CHECK(Model::findContainingGroup(innerEntityNode1) == innerGroupNode);
  CHECK(Model::findContainingGroup(outerEntityNode1) == outerGroupNode);

  // open the outer group and ungroup the inner group
  document->openGroup(outerGroupNode);
  document->selectNodes({innerGroupNode});
  document->ungroupSelection();
  document->deselectAll();

  CHECK(innerEntityNode1->parent() == outerGroupNode);
  CHECK(innerEntityNode2->parent() == outerGroupNode);
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLeavesPointEntitySelected")
{
  auto* entityNode1 = new Model::EntityNode{Model::Entity{}};

  document->addNodes({{document->parentForNodes(), {entityNode1}}});
  document->selectNodes({entityNode1});

  auto* groupNode = document->groupSelection("Group");
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{groupNode}));

  document->ungroupSelection();
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{entityNode1}));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLeavesBrushEntitySelected")
{
  const auto builder =
    Model::BrushBuilder{document->world()->mapFormat(), document->worldBounds()};

  auto* entityNode1 = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode1}}});

  auto* brushNode1 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture")
      .value());
  document->addNodes({{entityNode1, {brushNode1}}});
  document->selectNodes({entityNode1});
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{brushNode1}));
  CHECK_FALSE(entityNode1->selected());
  CHECK(brushNode1->selected());

  auto* groupNode = document->groupSelection("Group");
  CHECK_THAT(
    groupNode->children(), Catch::Equals(std::vector<Model::Node*>{entityNode1}));
  CHECK_THAT(
    entityNode1->children(), Catch::Equals(std::vector<Model::Node*>{brushNode1}));
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{groupNode}));
  CHECK(document->allSelectedBrushNodes() == std::vector<Model::BrushNode*>{brushNode1});
  CHECK(document->hasAnySelectedBrushNodes());
  CHECK(!document->selectedNodes().hasBrushes());

  document->ungroupSelection();
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{brushNode1}));
  CHECK_FALSE(entityNode1->selected());
  CHECK(brushNode1->selected());
}

// https://github.com/TrenchBroom/TrenchBroom/issues/3824
TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupGroupAndPointEntity")
{
  auto* entityNode1 = new Model::EntityNode{Model::Entity{}};
  auto* entityNode2 = new Model::EntityNode{Model::Entity{}};

  document->addNodes({{document->parentForNodes(), {entityNode1}}});
  document->addNodes({{document->parentForNodes(), {entityNode2}}});
  document->selectNodes({entityNode1});

  auto* groupNode = document->groupSelection("Group");
  document->selectNodes({entityNode2});
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode, entityNode2}));

  document->ungroupSelection();
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{entityNode1, entityNode2}));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.mergeGroups")
{
  document->selectAllNodes();
  document->deleteObjects();

  auto* entityNode1 = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode1}}});
  document->deselectAll();
  document->selectNodes({entityNode1});
  auto* groupNode1 = document->groupSelection("group1");

  auto* entityNode2 = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode2}}});
  document->deselectAll();
  document->selectNodes({entityNode2});
  auto* groupNode2 = document->groupSelection("group2");

  CHECK_THAT(
    document->currentLayer()->children(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode1, groupNode2}));

  document->selectNodes({groupNode1, groupNode2});
  document->mergeSelectedGroupsWithGroup(groupNode2);

  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{groupNode2}));
  CHECK_THAT(
    document->currentLayer()->children(),
    Catch::Equals(std::vector<Model::Node*>{groupNode2}));

  CHECK_THAT(groupNode1->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{}));
  CHECK_THAT(
    groupNode2->children(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{entityNode1, entityNode2}));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLinkedGroups")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  const auto originalGroupLinkId = groupNode->linkId();
  const auto originalBrushLinkId = brushNode->linkId();

  document->deselectAll();
  document->selectNodes({groupNode});

  auto* linkedGroupNode = document->createLinkedDuplicate();

  document->deselectAll();
  document->selectNodes({linkedGroupNode});

  auto* linkedGroupNode2 = document->createLinkedDuplicate();
  document->deselectAll();

  auto* linkedBrushNode =
    dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
  auto* linkedBrushNode2 =
    dynamic_cast<Model::BrushNode*>(linkedGroupNode2->children().front());


  REQUIRE_THAT(
    document->world()->defaultLayer()->children(),
    Catch::UnorderedEquals(
      std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));

  SECTION(
    "Given three linked groups, we ungroup one of them, the other two remain linked")
  {
    document->selectNodes({linkedGroupNode2});

    document->ungroupSelection();
    CHECK_THAT(
      document->world()->defaultLayer()->children(),
      Catch::UnorderedEquals(
        std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedBrushNode2}));
    CHECK(groupNode->linkId() == linkedGroupNode->linkId());
    CHECK(linkedGroupNode2->linkId() != groupNode->linkId());
    CHECK(linkedBrushNode2->linkId() != brushNode->linkId());
  }

  SECTION(
    "Given three linked groups, we ungroup two of them, and the remaining one keeps its "
    "ID")
  {
    document->selectNodes({linkedGroupNode, linkedGroupNode2});

    document->ungroupSelection();
    CHECK_THAT(
      document->world()->defaultLayer()->children(),
      Catch::UnorderedEquals(
        std::vector<Model::Node*>{groupNode, linkedBrushNode, linkedBrushNode2}));

    CHECK(groupNode->linkId() == originalGroupLinkId);
    CHECK(linkedGroupNode->linkId() != originalGroupLinkId);
    CHECK(linkedGroupNode2->linkId() != originalGroupLinkId);
    CHECK(linkedGroupNode2->linkId() != linkedGroupNode->linkId());

    CHECK(linkedBrushNode->linkId() != brushNode->linkId());
    CHECK(linkedBrushNode2->linkId() != brushNode->linkId());
    CHECK(linkedBrushNode2->linkId() != linkedBrushNode->linkId());
  }

  SECTION("Given three linked groups, we ungroup all of them")
  {
    document->selectNodes({groupNode});
    document->selectNodes({linkedGroupNode});
    document->selectNodes({linkedGroupNode2});

    document->ungroupSelection();
    CHECK_THAT(
      document->world()->defaultLayer()->children(),
      Catch::UnorderedEquals(
        std::vector<Model::Node*>{brushNode, linkedBrushNode, linkedBrushNode2}));

    CHECK(groupNode->linkId() != originalGroupLinkId);
    CHECK(linkedGroupNode->linkId() != originalGroupLinkId);
    CHECK(linkedGroupNode2->linkId() != originalGroupLinkId);

    CHECK(linkedGroupNode->linkId() != groupNode->linkId());
    CHECK(linkedGroupNode2->linkId() != groupNode->linkId());
    CHECK(linkedGroupNode2->linkId() != linkedGroupNode->linkId());
  }

  document->undoCommand();
  CHECK_THAT(
    document->world()->defaultLayer()->children(),
    Catch::UnorderedEquals(
      std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));
  CHECK(groupNode->linkId() == originalGroupLinkId);
  CHECK(linkedGroupNode->linkId() == originalGroupLinkId);
  CHECK(linkedGroupNode2->linkId() == originalGroupLinkId);

  CHECK(brushNode->linkId() == originalBrushLinkId);
  CHECK(linkedBrushNode->linkId() == originalBrushLinkId);
  CHECK(linkedBrushNode2->linkId() == originalBrushLinkId);
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createLinkedDuplicate")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  document->deselectAll();

  CHECK_FALSE(document->canCreateLinkedDuplicate());
  CHECK(document->createLinkedDuplicate() == nullptr);

  document->selectNodes({groupNode});
  CHECK(document->canCreateLinkedDuplicate());

  auto* linkedGroupNode = document->createLinkedDuplicate();
  CHECK_THAT(*linkedGroupNode, Model::MatchesNode(*groupNode));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.recursiveLinkedGroups")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  document->deselectAll();
  document->selectNodes({groupNode});
  auto* linkedGroupNode = document->createLinkedDuplicate();
  document->deselectAll();

  REQUIRE_THAT(*linkedGroupNode, Model::MatchesNode(*groupNode));

  SECTION("Adding a linked group to its linked sibling does nothing")
  {
    CHECK_FALSE(document->reparentNodes({{groupNode, {linkedGroupNode}}}));
  }

  SECTION(
    "Adding a group containing a nested linked sibling to a linked group does nothing")
  {
    document->selectNodes({linkedGroupNode});

    auto* outerGroupNode = document->groupSelection("outer");
    REQUIRE(outerGroupNode != nullptr);

    document->deselectAll();
    CHECK_FALSE(document->reparentNodes({{groupNode, {outerGroupNode}}}));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.selectLinkedGroups")
{
  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode, entityNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  SECTION("Cannot select linked groups if selection is empty")
  {
    document->deselectAll();
    CHECK_FALSE(document->canSelectLinkedGroups());
  }

  SECTION("Cannot select linked groups if selection contains non-groups")
  {
    document->deselectAll();
    document->selectNodes({entityNode});
    CHECK_FALSE(document->canSelectLinkedGroups());
    document->selectNodes({groupNode});
    CHECK_FALSE(document->canSelectLinkedGroups());
  }

  SECTION("Cannot select linked groups if selection contains unlinked groups")
  {
    document->deselectAll();
    document->selectNodes({entityNode});

    auto* unlinkedGroupNode = document->groupSelection("other");
    REQUIRE(unlinkedGroupNode != nullptr);

    CHECK_FALSE(document->canSelectLinkedGroups());

    document->selectNodes({groupNode});
    CHECK_FALSE(document->canSelectLinkedGroups());
  }

  SECTION("Select linked groups")
  {
    auto* linkedGroupNode = document->createLinkedDuplicate();
    REQUIRE(linkedGroupNode != nullptr);

    document->deselectAll();
    document->selectNodes({groupNode});

    REQUIRE(document->canSelectLinkedGroups());
    document->selectLinkedGroups();
    CHECK_THAT(
      document->selectedNodes().nodes(),
      Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode, linkedGroupNode}));
  }
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodestTest.separateGroups")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  document->deselectAll();
  document->selectNodes({groupNode});

  const auto originalGroupLinkId = groupNode->linkId();
  const auto originalBrushLinkId = brushNode->linkId();

  SECTION("Separating a group that isn't linked")
  {
    CHECK_FALSE(document->canSeparateLinkedGroups());
  }

  SECTION("Separating all members of a link set")
  {
    auto* linkedGroupNode = document->createLinkedDuplicate();
    REQUIRE_THAT(*linkedGroupNode, Model::MatchesNode(*groupNode));

    document->selectNodes({groupNode, linkedGroupNode});
    CHECK_FALSE(document->canSeparateLinkedGroups());
  }

  SECTION("Separating one group from a link set with two members")
  {
    auto* linkedGroupNode = document->createLinkedDuplicate();
    REQUIRE_THAT(*linkedGroupNode, Model::MatchesNode(*groupNode));

    auto* linkedBrushNode =
      dynamic_cast<Model::BrushNode*>(linkedGroupNode->children().front());
    REQUIRE(linkedBrushNode != nullptr);

    document->deselectAll();
    document->selectNodes({linkedGroupNode});

    CHECK(document->canSeparateLinkedGroups());
    document->separateLinkedGroups();
    CHECK(groupNode->linkId() == originalGroupLinkId);
    CHECK(brushNode->linkId() == originalBrushLinkId);
    CHECK(linkedGroupNode->linkId() != originalGroupLinkId);
    CHECK(linkedBrushNode->linkId() != originalBrushLinkId);

    document->undoCommand();
    CHECK(groupNode->linkId() == originalGroupLinkId);
    CHECK(linkedGroupNode->linkId() == originalGroupLinkId);
    CHECK(brushNode->linkId() == originalBrushLinkId);
    CHECK(linkedBrushNode->linkId() == originalBrushLinkId);
  }

  SECTION("Separating multiple groups from a link set with several members")
  {
    auto* linkedGroupNode1 = document->createLinkedDuplicate();
    auto* linkedGroupNode2 = document->createLinkedDuplicate();
    auto* linkedGroupNode3 = document->createLinkedDuplicate();

    REQUIRE_THAT(*linkedGroupNode1, Model::MatchesNode(*groupNode));
    REQUIRE_THAT(*linkedGroupNode2, Model::MatchesNode(*groupNode));
    REQUIRE_THAT(*linkedGroupNode3, Model::MatchesNode(*groupNode));

    auto* linkedBrushNode1 =
      dynamic_cast<Model::BrushNode*>(linkedGroupNode1->children().front());
    auto* linkedBrushNode2 =
      dynamic_cast<Model::BrushNode*>(linkedGroupNode2->children().front());
    auto* linkedBrushNode3 =
      dynamic_cast<Model::BrushNode*>(linkedGroupNode3->children().front());

    document->deselectAll();
    document->selectNodes({linkedGroupNode2, linkedGroupNode3});
    CHECK(document->canSeparateLinkedGroups());

    document->separateLinkedGroups();
    CHECK(groupNode->linkId() == originalGroupLinkId);
    CHECK(linkedGroupNode1->linkId() == originalGroupLinkId);

    CHECK(linkedGroupNode2->linkId() != originalGroupLinkId);
    CHECK(linkedGroupNode3->linkId() == linkedGroupNode2->linkId());

    CHECK(linkedBrushNode2->linkId() != originalBrushLinkId);
    CHECK(linkedBrushNode3->linkId() == linkedBrushNode2->linkId());

    CHECK(document->selectedNodes().groupCount() == 2u);

    document->undoCommand();

    CHECK(groupNode->linkId() == originalGroupLinkId);
    CHECK(linkedGroupNode1->linkId() == originalGroupLinkId);
    CHECK(linkedGroupNode2->linkId() == originalGroupLinkId);
    CHECK(linkedGroupNode3->linkId() == originalGroupLinkId);

    CHECK(brushNode->linkId() == originalBrushLinkId);
    CHECK(linkedBrushNode1->linkId() == originalBrushLinkId);
    CHECK(linkedBrushNode2->linkId() == originalBrushLinkId);
    CHECK(linkedBrushNode3->linkId() == originalBrushLinkId);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.newWithGroupOpen")
{
  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->selectNodes({entityNode});
  auto* groupNode = document->groupSelection("my group");
  document->openGroup(groupNode);

  CHECK(document->currentGroup() == groupNode);

  REQUIRE(document
            ->newDocument(
              Model::MapFormat::Valve, MapDocument::DefaultWorldBounds, document->game())
            .is_success());

  CHECK(document->currentGroup() == nullptr);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/3768
TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.operationsOnSeveralGroupsInLinkSet")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});
  document->selectNodes({brushNode});

  auto* groupNode = document->groupSelection("test");
  REQUIRE(groupNode != nullptr);

  auto* linkedGroupNode = document->createLinkedDuplicate();
  REQUIRE(linkedGroupNode != nullptr);

  document->deselectAll();

  SECTION("Face selection locks other groups in link set")
  {
    CHECK(!linkedGroupNode->locked());

    document->selectBrushFaces({{brushNode, 0}});
    CHECK(linkedGroupNode->locked());

    document->deselectAll();
    CHECK(!linkedGroupNode->locked());
  }

  SECTION("Can select two linked groups and apply a texture")
  {
    document->selectNodes({groupNode, linkedGroupNode});

    auto setTexture = Model::ChangeBrushFaceAttributesRequest{};
    setTexture.setTextureName("abc");
    CHECK(document->setFaceAttributes(setTexture));

    // check that the brushes in both linked groups were textured
    for (auto* g : std::vector<Model::GroupNode*>{groupNode, linkedGroupNode})
    {
      auto* brush = dynamic_cast<Model::BrushNode*>(g->children().at(0));
      REQUIRE(brush != nullptr);

      auto attrs = brush->brush().face(0).attributes();
      CHECK(attrs.textureName() == "abc");
    }
  }

  SECTION("Can't snap to grid with both groups selected")
  {
    document->selectNodes({groupNode, linkedGroupNode});

    CHECK(
      document->transformObjects("", vm::translation_matrix(vm::vec3{0.5, 0.5, 0.0})));

    // This could generate conflicts, because what snaps one group could misalign another
    // group in the link set. So, just reject the change.
    CHECK(!document->snapVertices(16.0));
  }
}

// https://github.com/TrenchBroom/TrenchBroom/issues/3768
TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.operationsOnSeveralGroupsInLinkSetWithPointEntities")
{
  {
    auto* entityNode = new Model::EntityNode{Model::Entity{}};
    document->addNodes({{document->parentForNodes(), {entityNode}}});
    document->selectNodes({entityNode});
  }

  auto* groupNode = document->groupSelection("test");
  auto* linkedGroupNode1 = document->createLinkedDuplicate();
  auto* linkedGroupNode2 = document->createLinkedDuplicate();

  REQUIRE(groupNode != nullptr);
  REQUIRE(linkedGroupNode1 != nullptr);
  REQUIRE(linkedGroupNode2 != nullptr);

  document->deselectAll();

  SECTION("Attempt to set a property with 2 out of 3 groups selected")
  {
    document->selectNodes({groupNode, linkedGroupNode1});

    // Current design is to reject this because it's modifying entities from multiple
    // groups in a link set. While in this case the change isn't conflicting, some entity
    // changes are, e.g. unprotecting a property with 2 linked groups selected, where
    // entities have different values for that protected property.
    //
    // Additionally, the use case for editing entity properties with the entire map
    // selected seems unlikely.
    CHECK(!document->setProperty("key", "value"));

    auto* groupNodeEntity = dynamic_cast<Model::EntityNode*>(groupNode->children().at(0));
    auto* linkedEntityNode1 =
      dynamic_cast<Model::EntityNode*>(linkedGroupNode1->children().at(0));
    auto* linkedEntityNode2 =
      dynamic_cast<Model::EntityNode*>(linkedGroupNode2->children().at(0));
    REQUIRE(groupNodeEntity != nullptr);
    REQUIRE(linkedEntityNode1 != nullptr);
    REQUIRE(linkedEntityNode2 != nullptr);

    CHECK(!groupNodeEntity->entity().hasProperty("key"));
    CHECK(!linkedEntityNode1->entity().hasProperty("key"));
    CHECK(!linkedEntityNode2->entity().hasProperty("key"));
  }
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.dontCrashWhenLinkedGroupUpdateFailsDuringEntityCreate")
{

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->selectNodes({entityNode});

  // move the entity down
  REQUIRE(document->translateObjects({0, 0, -256}));
  REQUIRE(
    entityNode->physicalBounds() == vm::bbox3{{-8, -8, -256 - 8}, {8, 8, -256 + 8}});

  auto* groupNode = document->groupSelection("test");
  auto* linkedGroupNode = document->createLinkedDuplicate();

  // move the linked group up by half the world bounds
  const auto zOffset = document->worldBounds().max.z();
  document->deselectAll();
  document->selectNodes({linkedGroupNode});
  document->translateObjects({0, 0, document->worldBounds().max.z()});
  REQUIRE(
    linkedGroupNode->physicalBounds()
    == vm::bbox3{{-8, -8, -256 - 8 + zOffset}, {8, 8, -256 + 8 + zOffset}});

  // create a brush entity inside the original group
  document->openGroup(groupNode);
  document->deselectAll();

  SECTION("create point entity")
  {
    REQUIRE(m_pointEntityDef->bounds() == vm::bbox3{{-16, -16, -16}, {16, 16, 16}});

    // create a new point entity below the origin -- this entity is temporarily created at
    // the origin and then moved to its eventual position, but the entity at the origin is
    // propagated into the linked group, where it ends up out of  world bounds
    CHECK(document->createPointEntity(m_pointEntityDef, {0, 0, -32}) != nullptr);
  }

  SECTION("create brush entity")
  {
    auto* brushNode = createBrushNode();
    Model::transformNode(
      *brushNode, vm::translation_matrix(vm::vec3{0, 0, -32}), document->worldBounds());
    REQUIRE(brushNode->physicalBounds() == vm::bbox3{{-16, -16, -48}, {16, 16, -16}});

    document->addNodes({{document->parentForNodes(), {brushNode}}});
    document->deselectAll();
    document->selectNodes({brushNode});

    // create a brush entity - a temporarily empty entity will be created at the origin
    // and propagated into the linked group, where it ends up out of world bounds and thus
    // failing
    CHECK(document->createBrushEntity(m_brushEntityDef) != nullptr);
  }
}

} // namespace TrenchBroom::View
