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
#include "Model/BrushBuilder.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GameError.h"
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

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createEmptyGroup")
{
  CHECK(document->groupSelection("test") == nullptr);
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.createGroupWithOneNode", "[GroupNodesTest]")
{
  using CreateNode = std::function<Model::Node*(const MapDocumentTest&)>;
  CreateNode createNode = GENERATE_COPY(
    CreateNode{[](const auto& test) { return test.createBrushNode(); }},
    CreateNode{[](const auto& test) { return test.createPatchNode(); }});

  auto* node = createNode(*this);
  document->addNodes({{document->parentForNodes(), {node}}});
  document->selectNodes({node});

  Model::GroupNode* group = document->groupSelection("test");
  CHECK(group != nullptr);

  CHECK(node->parent() == group);
  CHECK(group->selected());
  CHECK_FALSE(node->selected());

  document->undoCommand();
  CHECK(group->parent() == nullptr);
  CHECK(node->parent() == document->parentForNodes());
  CHECK(node->selected());
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.createGroupWithPartialBrushEntity", "[GroupNodesTest]")
{
  Model::BrushNode* child1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {child1}}});

  Model::PatchNode* child2 = createPatchNode();
  document->addNodes({{document->parentForNodes(), {child2}}});

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entity}}});
  document->reparentNodes({{entity, {child1, child2}}});

  document->selectNodes({child1});

  Model::GroupNode* group = document->groupSelection("test");
  CHECK(group != nullptr);

  CHECK(child1->parent() == entity);
  CHECK(child2->parent() == entity);
  CHECK(entity->parent() == group);
  CHECK(group->selected());
  CHECK_FALSE(child1->selected());

  document->undoCommand();
  CHECK(group->parent() == nullptr);
  CHECK(child1->parent() == entity);
  CHECK(child2->parent() == entity);
  CHECK(entity->parent() == document->parentForNodes());
  CHECK_FALSE(group->selected());
  CHECK(child1->selected());
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.createGroupWithFullBrushEntity", "[GroupNodesTest]")
{
  Model::BrushNode* child1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {child1}}});

  Model::PatchNode* child2 = createPatchNode();
  document->addNodes({{document->parentForNodes(), {child2}}});

  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entity}}});
  document->reparentNodes({{entity, {child1, child2}}});

  document->selectNodes({child1, child2});

  Model::GroupNode* group = document->groupSelection("test");
  CHECK(group != nullptr);

  CHECK(child1->parent() == entity);
  CHECK(child2->parent() == entity);
  CHECK(entity->parent() == group);
  CHECK(group->selected());
  CHECK_FALSE(child1->selected());
  CHECK_FALSE(child2->selected());

  document->undoCommand();
  CHECK(group->parent() == nullptr);
  CHECK(child1->parent() == entity);
  CHECK(child2->parent() == entity);
  CHECK(entity->parent() == document->parentForNodes());
  CHECK_FALSE(group->selected());
  CHECK(child1->selected());
  CHECK(child2->selected());
}

static bool hasEmptyName(const std::vector<std::string>& names)
{
  for (const auto& name : names)
  {
    if (name.empty())
    {
      return true;
    }
  }
  return false;
}

TEST_CASE_METHOD(
  MapDocumentTest,
  "GroupNodesTest.undoMoveGroupContainingBrushEntity",
  "[GroupNodesTest]")
{
  // Test for issue #1715

  Model::BrushNode* brush1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brush1}}});

  Model::EntityNode* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->reparentNodes({{entityNode, {brush1}}});

  document->selectNodes({brush1});

  Model::GroupNode* group = document->groupSelection("test");
  CHECK(group->selected());

  CHECK(document->translateObjects(vm::vec3(16, 0, 0)));

  CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));

  document->undoCommand();

  CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.rotateGroupContainingBrushEntity", "[GroupNodesTest]")
{
  // Test for issue #1754

  Model::BrushNode* brush1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brush1}}});

  Model::EntityNode* entityNode = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->reparentNodes({{entityNode, {brush1}}});

  document->selectNodes({brush1});

  Model::GroupNode* group = document->groupSelection("test");
  CHECK(group->selected());

  CHECK_FALSE(entityNode->entity().hasProperty("origin"));
  CHECK(document->rotateObjects(
    vm::vec3::zero(), vm::vec3::pos_z(), static_cast<FloatType>(10.0)));
  CHECK_FALSE(entityNode->entity().hasProperty("origin"));

  document->undoCommand();

  CHECK_FALSE(entityNode->entity().hasProperty("origin"));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.renameGroup")
{
  Model::BrushNode* brush1 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brush1}}});
  document->selectNodes({brush1});

  Model::GroupNode* group = document->groupSelection("test");

  document->renameGroups("abc");
  CHECK(group->name() == "abc");

  document->undoCommand();
  CHECK(group->name() == "test");

  document->redoCommand();
  CHECK(group->name() == "abc");
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.duplicateNodeInGroup", "[GroupNodesTest]")
{
  Model::BrushNode* brush = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brush}}});
  document->selectNodes({brush});

  Model::GroupNode* group = document->groupSelection("test");
  REQUIRE(group != nullptr);

  document->openGroup(group);

  document->selectNodes({brush});
  document->duplicateObjects();

  Model::BrushNode* brushCopy = document->selectedNodes().brushes().at(0u);
  CHECK(brushCopy->parent() == group);
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupInnerGroup")
{
  // see https://github.com/TrenchBroom/TrenchBroom/issues/2050
  Model::EntityNode* outerEnt1 = new Model::EntityNode{Model::Entity{}};
  Model::EntityNode* outerEnt2 = new Model::EntityNode{Model::Entity{}};
  Model::EntityNode* innerEnt1 = new Model::EntityNode{Model::Entity{}};
  Model::EntityNode* innerEnt2 = new Model::EntityNode{Model::Entity{}};

  document->addNodes({{document->parentForNodes(), {innerEnt1}}});
  document->addNodes({{document->parentForNodes(), {innerEnt2}}});
  document->selectNodes({innerEnt1, innerEnt2});

  Model::GroupNode* inner = document->groupSelection("Inner");

  document->deselectAll();
  document->addNodes({{document->parentForNodes(), {outerEnt1}}});
  document->addNodes({{document->parentForNodes(), {outerEnt2}}});
  document->selectNodes({inner, outerEnt1, outerEnt2});

  Model::GroupNode* outer = document->groupSelection("Outer");
  document->deselectAll();

  // check our assumptions
  CHECK(outer->childCount() == 3u);
  CHECK(inner->childCount() == 2u);

  CHECK(outer->parent() == document->currentLayer());

  CHECK(outerEnt1->parent() == outer);
  CHECK(outerEnt2->parent() == outer);
  CHECK(inner->parent() == outer);

  CHECK(innerEnt1->parent() == inner);
  CHECK(innerEnt2->parent() == inner);

  CHECK(document->currentGroup() == nullptr);
  CHECK(!outer->opened());
  CHECK(!inner->opened());

  CHECK(Model::findOutermostClosedGroup(innerEnt1) == outer);
  CHECK(Model::findOutermostClosedGroup(outerEnt1) == outer);

  CHECK(Model::findContainingGroup(innerEnt1) == inner);
  CHECK(Model::findContainingGroup(outerEnt1) == outer);

  // open the outer group and ungroup the inner group
  document->openGroup(outer);
  document->selectNodes({inner});
  document->ungroupSelection();
  document->deselectAll();

  CHECK(innerEnt1->parent() == outer);
  CHECK(innerEnt2->parent() == outer);
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLeavesPointEntitySelected")
{
  Model::EntityNode* ent1 = new Model::EntityNode{Model::Entity{}};

  document->addNodes({{document->parentForNodes(), {ent1}}});
  document->selectNodes({ent1});

  Model::GroupNode* group = document->groupSelection("Group");
  CHECK_THAT(
    document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*>{group}));

  document->ungroupSelection();
  CHECK_THAT(
    document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*>{ent1}));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLeavesBrushEntitySelected")
{
  const Model::BrushBuilder builder(
    document->world()->mapFormat(), document->worldBounds());

  Model::EntityNode* ent1 = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {ent1}}});

  Model::BrushNode* brushNode1 = new Model::BrushNode(
    builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture")
      .value());
  document->addNodes({{ent1, {brushNode1}}});
  document->selectNodes({ent1});
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{brushNode1}));
  CHECK_FALSE(ent1->selected());
  CHECK(brushNode1->selected());

  Model::GroupNode* group = document->groupSelection("Group");
  CHECK_THAT(group->children(), Catch::Equals(std::vector<Model::Node*>{ent1}));
  CHECK_THAT(ent1->children(), Catch::Equals(std::vector<Model::Node*>{brushNode1}));
  CHECK_THAT(
    document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*>{group}));
  CHECK(document->allSelectedBrushNodes() == std::vector<Model::BrushNode*>{brushNode1});
  CHECK(document->hasAnySelectedBrushNodes());
  CHECK(!document->selectedNodes().hasBrushes());

  document->ungroupSelection();
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::Equals(std::vector<Model::Node*>{brushNode1}));
  CHECK_FALSE(ent1->selected());
  CHECK(brushNode1->selected());
}

// https://github.com/TrenchBroom/TrenchBroom/issues/3824
TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupGroupAndPointEntity")
{
  auto* ent1 = new Model::EntityNode{Model::Entity{}};
  auto* ent2 = new Model::EntityNode{Model::Entity{}};

  document->addNodes({{document->parentForNodes(), {ent1}}});
  document->addNodes({{document->parentForNodes(), {ent2}}});
  document->selectNodes({ent1});

  auto* group = document->groupSelection("Group");
  document->selectNodes({ent2});
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{group, ent2}));

  document->ungroupSelection();
  CHECK_THAT(
    document->selectedNodes().nodes(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{ent1, ent2}));
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.mergeGroups")
{
  document->selectAllNodes();
  document->deleteObjects();

  Model::EntityNode* ent1 = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {ent1}}});
  document->deselectAll();
  document->selectNodes({ent1});
  Model::GroupNode* group1 = document->groupSelection("group1");

  Model::EntityNode* ent2 = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {ent2}}});
  document->deselectAll();
  document->selectNodes({ent2});
  Model::GroupNode* group2 = document->groupSelection("group2");

  CHECK_THAT(
    document->currentLayer()->children(),
    Catch::UnorderedEquals(std::vector<Model::Node*>{group1, group2}));

  document->selectNodes({group1, group2});
  document->mergeSelectedGroupsWithGroup(group2);

  CHECK_THAT(
    document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*>{group2}));
  CHECK_THAT(
    document->currentLayer()->children(),
    Catch::Equals(std::vector<Model::Node*>{group2}));

  CHECK_THAT(group1->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{}));
  CHECK_THAT(
    group2->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{ent1, ent2}));
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.ungroupLinkedGroups", "[GroupNodesTest]")
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
  document->selectNodes({linkedGroupNode});

  auto* linkedGroupNode2 = document->createLinkedDuplicate();

  document->deselectAll();
  REQUIRE_THAT(
    document->world()->defaultLayer()->children(),
    Catch::UnorderedEquals(
      std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));

  SECTION(
    "Given three linked groups, we ungroup one of them, the other two remain linked")
  {
    document->selectNodes({linkedGroupNode2});

    auto* linkedBrushNode2 = linkedGroupNode2->children().front();

    document->ungroupSelection();
    CHECK_THAT(
      document->world()->defaultLayer()->children(),
      Catch::UnorderedEquals(
        std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedBrushNode2}));
    CHECK(groupNode->group().linkedGroupId().has_value());
    CHECK(linkedGroupNode->group().linkedGroupId().has_value());
    CHECK(groupNode->group().linkedGroupId() == linkedGroupNode->group().linkedGroupId());
  }

  SECTION(
    "Given three linked groups, we ungroup two of them, and the remaining one becomes a "
    "regular group")
  {
    document->selectNodes({linkedGroupNode});
    document->selectNodes({linkedGroupNode2});

    auto* linkedBrushNode = linkedGroupNode->children().front();
    auto* linkedBrushNode2 = linkedGroupNode2->children().front();

    document->ungroupSelection();
    CHECK_THAT(
      document->world()->defaultLayer()->children(),
      Catch::UnorderedEquals(
        std::vector<Model::Node*>{groupNode, linkedBrushNode, linkedBrushNode2}));
    CHECK_FALSE(groupNode->group().linkedGroupId().has_value());
  }

  SECTION("Given three linked groups, we ungroup all of them")
  {
    document->selectNodes({groupNode});
    document->selectNodes({linkedGroupNode});
    document->selectNodes({linkedGroupNode2});

    auto* linkedBrushNode = linkedGroupNode->children().front();
    auto* linkedBrushNode2 = linkedGroupNode2->children().front();

    document->ungroupSelection();
    CHECK_THAT(
      document->world()->defaultLayer()->children(),
      Catch::UnorderedEquals(
        std::vector<Model::Node*>{brushNode, linkedBrushNode, linkedBrushNode2}));
  }

  document->undoCommand();
  CHECK_THAT(
    document->world()->defaultLayer()->children(),
    Catch::UnorderedEquals(
      std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));
  CHECK(groupNode->group().linkedGroupId().has_value());
  CHECK(linkedGroupNode->group().linkedGroupId().has_value());
  CHECK(linkedGroupNode2->group().linkedGroupId().has_value());
  CHECK(groupNode->group().linkedGroupId() == linkedGroupNode->group().linkedGroupId());
  CHECK(groupNode->group().linkedGroupId() == linkedGroupNode2->group().linkedGroupId());
}

TEST_CASE_METHOD(
  MapDocumentTest, "GroupNodesTest.createLinkedDuplicate", "[GroupNodesTest]")
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
  CHECK(linkedGroupNode != nullptr);

  CHECK(groupNode->group().linkedGroupId() != std::nullopt);
  CHECK(linkedGroupNode->group().linkedGroupId() == groupNode->group().linkedGroupId());
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

  REQUIRE(linkedGroupNode != nullptr);
  REQUIRE(groupNode->group().linkedGroupId() != std::nullopt);
  REQUIRE(linkedGroupNode->group().linkedGroupId() == groupNode->group().linkedGroupId());

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

  SECTION("Separating a group that isn't linked")
  {
    CHECK_FALSE(document->canSeparateLinkedGroups());
  }

  SECTION("Separating all members of a link set")
  {
    auto* linkedGroupNode = document->createLinkedDuplicate();
    REQUIRE(linkedGroupNode != nullptr);
    REQUIRE(groupNode->group().linkedGroupId() != std::nullopt);
    REQUIRE(
      linkedGroupNode->group().linkedGroupId() == groupNode->group().linkedGroupId());

    document->selectNodes({groupNode, linkedGroupNode});
    CHECK_FALSE(document->canSeparateLinkedGroups());
  }

  SECTION("Separating one group from a link set with two members")
  {
    auto* linkedGroupNode = document->createLinkedDuplicate();
    REQUIRE(linkedGroupNode != nullptr);

    const auto originalLinkedGroupId = groupNode->group().linkedGroupId();
    REQUIRE(originalLinkedGroupId != std::nullopt);
    REQUIRE(linkedGroupNode->group().linkedGroupId() == originalLinkedGroupId);

    document->deselectAll();
    document->selectNodes({linkedGroupNode});

    CHECK(document->canSeparateLinkedGroups());
    document->separateLinkedGroups();
    CHECK(groupNode->group().linkedGroupId() == std::nullopt);
    CHECK(linkedGroupNode->group().linkedGroupId() == std::nullopt);

    document->undoCommand();
    CHECK(groupNode->group().linkedGroupId() == originalLinkedGroupId);
    CHECK(linkedGroupNode->group().linkedGroupId() == originalLinkedGroupId);
  }

  SECTION("Separating multiple groups from a link set with several members")
  {
    auto* linkedGroupNode1 = document->createLinkedDuplicate();
    auto* linkedGroupNode2 = document->createLinkedDuplicate();
    auto* linkedGroupNode3 = document->createLinkedDuplicate();

    REQUIRE(linkedGroupNode1 != nullptr);
    REQUIRE(linkedGroupNode2 != nullptr);
    REQUIRE(linkedGroupNode3 != nullptr);

    const auto originalLinkedGroupId = groupNode->group().linkedGroupId();
    REQUIRE(originalLinkedGroupId != std::nullopt);
    REQUIRE(linkedGroupNode1->group().linkedGroupId() == originalLinkedGroupId);
    REQUIRE(linkedGroupNode2->group().linkedGroupId() == originalLinkedGroupId);
    REQUIRE(linkedGroupNode3->group().linkedGroupId() == originalLinkedGroupId);

    document->deselectAll();
    document->selectNodes({linkedGroupNode2, linkedGroupNode3});
    CHECK(document->canSeparateLinkedGroups());

    document->separateLinkedGroups();
    CHECK(groupNode->group().linkedGroupId() == originalLinkedGroupId);
    CHECK(linkedGroupNode1->group().linkedGroupId() == originalLinkedGroupId);

    CHECK(linkedGroupNode2->group().linkedGroupId() != std::nullopt);
    CHECK(linkedGroupNode2->group().linkedGroupId() != originalLinkedGroupId);
    CHECK(
      linkedGroupNode3->group().linkedGroupId()
      == linkedGroupNode2->group().linkedGroupId());

    CHECK(document->selectedNodes().groupCount() == 2u);

    document->undoCommand();

    CHECK(groupNode->group().linkedGroupId() == originalLinkedGroupId);
    CHECK(linkedGroupNode1->group().linkedGroupId() == originalLinkedGroupId);
    CHECK(linkedGroupNode2->group().linkedGroupId() == originalLinkedGroupId);
    CHECK(linkedGroupNode3->group().linkedGroupId() == originalLinkedGroupId);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.newWithGroupOpen")
{
  Model::EntityNode* entity = new Model::EntityNode{Model::Entity{}};
  document->addNodes({{document->parentForNodes(), {entity}}});
  document->selectNodes({entity});
  Model::GroupNode* group = document->groupSelection("my group");
  document->openGroup(group);

  CHECK(document->currentGroup() == group);

  REQUIRE(document
            ->newDocument(
              Model::MapFormat::Valve, MapDocument::DefaultWorldBounds, document->game())
            .is_success());

  CHECK(document->currentGroup() == nullptr);
}

// https://github.com/TrenchBroom/TrenchBroom/issues/3768
TEST_CASE_METHOD(
  MapDocumentTest,
  "GroupNodesTest.operationsOnSeveralGroupsInLinkSet",
  "[GroupNodesTest]")
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
  MapDocumentTest,
  "GroupNodesTest.operationsOnSeveralGroupsInLinkSetWithPointEntities",
  "[GroupNodesTest]")
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
} // namespace View
} // namespace TrenchBroom
