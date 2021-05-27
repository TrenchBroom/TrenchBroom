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

#include "View/MapDocumentTest.h"
#include "TestUtils.h"

#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/ModelUtils.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "View/PasteType.h"

#include <kdl/result.h>

#include <functional>
#include <set>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createEmptyGroup", "[GroupNodesTest]") {
            CHECK(document->groupSelection("test") == nullptr);
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createGroupWithOneNode", "[GroupNodesTest]") {
            using CreateNode = std::function<Model::Node*(const MapDocumentTest&)>;
            CreateNode createNode = GENERATE_COPY(
                CreateNode{[](const auto& test) { return test.createBrushNode(); }},
                CreateNode{[](const auto& test) { return test.createPatchNode(); }});

            auto* node = createNode(*this);
            addNode(*document, document->parentForNodes(), node);
            document->select(node);

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

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createGroupWithPartialBrushEntity", "[GroupNodesTest]") {
            Model::BrushNode* child1 = createBrushNode();
            addNode(*document, document->parentForNodes(), child1);

            Model::PatchNode* child2 = createPatchNode();
            addNode(*document, document->parentForNodes(), child2);

            Model::EntityNode* entity = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), entity);
            reparentNodes(*document, entity, { child1, child2 });

            document->select(child1);

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

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createGroupWithFullBrushEntity", "[GroupNodesTest]") {
            Model::BrushNode* child1 = createBrushNode();
            addNode(*document, document->parentForNodes(), child1);

            Model::PatchNode* child2 = createPatchNode();
            addNode(*document, document->parentForNodes(), child2);

            Model::EntityNode* entity = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), entity);
            reparentNodes(*document, entity, { child1, child2 });

            document->select(std::vector<Model::Node*>({ child1, child2 }));

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

        static bool hasEmptyName(const std::vector<std::string>& names) {
            for (const auto& name : names) {
                if (name.empty()) {
                    return true;
                }
            }
            return false;
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.undoMoveGroupContainingBrushEntity", "[GroupNodesTest]") {
            // Test for issue #1715

            Model::BrushNode* brush1 = createBrushNode();
            addNode(*document, document->parentForNodes(), brush1);

            Model::EntityNode* entityNode = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), entityNode);
            reparentNodes(*document, entityNode, { brush1 });

            document->select(brush1);

            Model::GroupNode* group = document->groupSelection("test");
            CHECK(group->selected());

            CHECK(document->translateObjects(vm::vec3(16,0,0)));

            CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));

            document->undoCommand();

            CHECK_FALSE(hasEmptyName(entityNode->entity().propertyKeys()));
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.rotateGroupContainingBrushEntity", "[GroupNodesTest]") {
            // Test for issue #1754

            Model::BrushNode* brush1 = createBrushNode();
            addNode(*document, document->parentForNodes(), brush1);

            Model::EntityNode* entityNode = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), entityNode);
            reparentNodes(*document, entityNode, { brush1 });

            document->select(brush1);

            Model::GroupNode* group = document->groupSelection("test");
            CHECK(group->selected());

            CHECK_FALSE(entityNode->entity().hasProperty("origin"));
            CHECK(document->rotateObjects(vm::vec3::zero(), vm::vec3::pos_z(), static_cast<FloatType>(10.0)));
            CHECK_FALSE(entityNode->entity().hasProperty("origin"));

            document->undoCommand();

            CHECK_FALSE(entityNode->entity().hasProperty("origin"));
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.renameGroup", "[GroupNodesTest]") {
            Model::BrushNode* brush1 = createBrushNode();
            addNode(*document, document->parentForNodes(), brush1);
            document->select(brush1);

            Model::GroupNode* group = document->groupSelection("test");
            
            document->renameGroups("abc");
            CHECK(group->name() == "abc");
            
            document->undoCommand();
            CHECK(group->name() == "test");

            document->redoCommand();
            CHECK(group->name() == "abc");
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.duplicateNodeInGroup", "[GroupNodesTest]") {
            Model::BrushNode* brush = createBrushNode();
            addNode(*document, document->parentForNodes(), brush);
            document->select(brush);

            Model::GroupNode* group = document->groupSelection("test");
            REQUIRE(group != nullptr);

            document->openGroup(group);

            document->select(brush);
            document->duplicateObjects();

            Model::BrushNode* brushCopy = document->selectedNodes().brushes().at(0u);
            CHECK(brushCopy->parent() == group);
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupInnerGroup") {
            // see https://github.com/TrenchBroom/TrenchBroom/issues/2050
            Model::EntityNode* outerEnt1 = new Model::EntityNode();
            Model::EntityNode* outerEnt2 = new Model::EntityNode();
            Model::EntityNode* innerEnt1 = new Model::EntityNode();
            Model::EntityNode* innerEnt2 = new Model::EntityNode();

            addNode(*document, document->parentForNodes(), innerEnt1);
            addNode(*document, document->parentForNodes(), innerEnt2);
            document->select(std::vector<Model::Node*> {innerEnt1, innerEnt2});

            Model::GroupNode* inner = document->groupSelection("Inner");

            document->deselectAll();
            addNode(*document, document->parentForNodes(), outerEnt1);
            addNode(*document, document->parentForNodes(), outerEnt2);
            document->select(std::vector<Model::Node*> {inner, outerEnt1, outerEnt2});

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
            document->select(inner);
            document->ungroupSelection();
            document->deselectAll();

            CHECK(innerEnt1->parent() == outer);
            CHECK(innerEnt2->parent() == outer);
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLeavesPointEntitySelected") {
            Model::EntityNode* ent1 = new Model::EntityNode();

            addNode(*document, document->parentForNodes(), ent1);
            document->select(std::vector<Model::Node*> {ent1});

            Model::GroupNode* group = document->groupSelection("Group");
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {group}));

            document->ungroupSelection();
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {ent1}));
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLeavesBrushEntitySelected") {
            const Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());

            Model::EntityNode* ent1 = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), ent1);

            Model::BrushNode* brushNode1 = new Model::BrushNode(builder.createCuboid(vm::bbox3(vm::vec3(0, 0, 0), vm::vec3(64, 64, 64)), "texture").value());
            addNode(*document, ent1, brushNode1);
            document->select(std::vector<Model::Node*>{ent1});
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> { brushNode1}));
            CHECK_FALSE(ent1->selected());
            CHECK(brushNode1->selected());

            Model::GroupNode* group = document->groupSelection("Group");
            CHECK_THAT(group->children(), Catch::Equals(std::vector<Model::Node*> {ent1}));
            CHECK_THAT(ent1->children(), Catch::Equals(std::vector<Model::Node*> { brushNode1}));
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {group}));
            CHECK(document->selectedNodes().brushesRecursively() == std::vector<Model::BrushNode*>{ brushNode1});
            CHECK(document->selectedNodes().hasBrushesRecursively());
            CHECK(!document->selectedNodes().hasBrushes());

            document->ungroupSelection();
            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> { brushNode1}));
            CHECK_FALSE(ent1->selected());
            CHECK(brushNode1->selected());
        }

        // https://github.com/TrenchBroom/TrenchBroom/issues/3824
        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupGroupAndPointEntity") {
            auto* ent1 = new Model::EntityNode{};
            auto* ent2 = new Model::EntityNode{};

            addNode(*document, document->parentForNodes(), ent1);
            addNode(*document, document->parentForNodes(), ent2);
            document->select(std::vector<Model::Node*>{ ent1 });

            auto* group = document->groupSelection("Group");
            document->select(std::vector<Model::Node*>{ent2});
            CHECK_THAT(document->selectedNodes().nodes(), Catch::UnorderedEquals(std::vector<Model::Node*>{group, ent2}));
            
            document->ungroupSelection();
            CHECK_THAT(document->selectedNodes().nodes(), Catch::UnorderedEquals(std::vector<Model::Node*>{ent1, ent2}));
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.mergeGroups") {
            document->selectAllNodes();
            document->deleteObjects();

            Model::EntityNode* ent1 = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), ent1);
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent1});
            Model::GroupNode* group1 = document->groupSelection("group1");

            Model::EntityNode* ent2 = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), ent2);
            document->deselectAll();
            document->select(std::vector<Model::Node*> {ent2});
            Model::GroupNode* group2 = document->groupSelection("group2");

            CHECK_THAT(document->currentLayer()->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{ group1, group2 }));

            document->select(std::vector<Model::Node*> {group1, group2});
            document->mergeSelectedGroupsWithGroup(group2);

            CHECK_THAT(document->selectedNodes().nodes(), Catch::Equals(std::vector<Model::Node*> {group2}));
            CHECK_THAT(document->currentLayer()->children(), Catch::Equals(std::vector<Model::Node*> {group2}));

            CHECK_THAT(group1->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{}));
            CHECK_THAT(group2->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{ ent1, ent2 }));
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.ungroupLinkedGroups", "[GroupNodesTest]") {
            auto* brushNode = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brushNode}}});

            document->select(brushNode);

            auto* groupNode = document->groupSelection("test");
            REQUIRE(groupNode != nullptr);

            document->deselectAll();
            document->select(groupNode);

            auto* linkedGroupNode = document->createLinkedDuplicate();

            document->deselectAll();
            document->select(linkedGroupNode);

            auto* linkedGroupNode2 = document->createLinkedDuplicate();

            document->deselectAll();
            REQUIRE_THAT(document->world()->defaultLayer()->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));

            SECTION("Given three linked groups, we ungroup one of them, the other two remain linked") {
                document->select(linkedGroupNode2);

                auto* linkedBrushNode2 = linkedGroupNode2->children().front();

                document->ungroupSelection();
                CHECK_THAT(document->world()->defaultLayer()->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedBrushNode2}));
                CHECK(groupNode->group().linkedGroupId().has_value());
                CHECK(linkedGroupNode->group().linkedGroupId().has_value());
                CHECK(groupNode->group().linkedGroupId() == linkedGroupNode->group().linkedGroupId());
            }

            SECTION("Given three linked groups, we ungroup two of them, and the remaining one becomes a regular group") {
                document->select(linkedGroupNode);
                document->select(linkedGroupNode2);

                auto* linkedBrushNode = linkedGroupNode->children().front();
                auto* linkedBrushNode2 = linkedGroupNode2->children().front();

                document->ungroupSelection();
                CHECK_THAT(document->world()->defaultLayer()->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode, linkedBrushNode, linkedBrushNode2}));
                CHECK_FALSE(groupNode->group().linkedGroupId().has_value());
            }

            SECTION("Given three linked groups, we ungroup all of them") {
                document->select(groupNode);
                document->select(linkedGroupNode);
                document->select(linkedGroupNode2);

                auto* linkedBrushNode = linkedGroupNode->children().front();
                auto* linkedBrushNode2 = linkedGroupNode2->children().front();

                document->ungroupSelection();
                CHECK_THAT(document->world()->defaultLayer()->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{brushNode, linkedBrushNode, linkedBrushNode2}));
            }

            document->undoCommand();
            CHECK_THAT(document->world()->defaultLayer()->children(), Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));
            CHECK(groupNode->group().linkedGroupId().has_value());
            CHECK(linkedGroupNode->group().linkedGroupId().has_value());
            CHECK(linkedGroupNode2->group().linkedGroupId().has_value());
            CHECK(groupNode->group().linkedGroupId() == linkedGroupNode->group().linkedGroupId());
            CHECK(groupNode->group().linkedGroupId() == linkedGroupNode2->group().linkedGroupId());
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.createLinkedDuplicate", "[GroupNodesTest]") {
            auto* brushNode = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(brushNode);

            auto* groupNode = document->groupSelection("test");
            REQUIRE(groupNode != nullptr);

            document->deselectAll();

            CHECK_FALSE(document->canCreateLinkedDuplicate());
            CHECK(document->createLinkedDuplicate() == nullptr);

            document->select(groupNode);
            CHECK(document->canCreateLinkedDuplicate());

            auto* linkedGroupNode = document->createLinkedDuplicate();
            CHECK(linkedGroupNode != nullptr);

            CHECK(groupNode->group().linkedGroupId() != std::nullopt);
            CHECK(linkedGroupNode->group().linkedGroupId() == groupNode->group().linkedGroupId());
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.selectLinkedGroups", "[GroupNodesTest]") {
            auto* entityNode = new Model::EntityNode{};
            auto* brushNode = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brushNode, entityNode}}});
            document->select(brushNode);

            auto* groupNode = document->groupSelection("test");
            REQUIRE(groupNode != nullptr);

            SECTION("Cannot select linked groups if selection is empty") {
                document->deselectAll();
                CHECK_FALSE(document->canSelectLinkedGroups());
            }

            SECTION("Cannot select linked groups if selection contains non-groups") {
                document->deselectAll();
                document->select(entityNode);
                CHECK_FALSE(document->canSelectLinkedGroups());
                document->select(groupNode);
                CHECK_FALSE(document->canSelectLinkedGroups());
            }

            SECTION("Cannot select linked groups if selection contains unlinked groups") {
                document->deselectAll();
                document->select(entityNode);

                auto* unlinkedGroupNode = document->groupSelection("other");
                REQUIRE(unlinkedGroupNode != nullptr);

                CHECK_FALSE(document->canSelectLinkedGroups());

                document->select(groupNode);
                CHECK_FALSE(document->canSelectLinkedGroups());
            }

            SECTION("Select linked groups") {
                auto* linkedGroupNode = document->createLinkedDuplicate();
                REQUIRE(linkedGroupNode != nullptr);

                document->deselectAll();
                document->select(groupNode);
                
                REQUIRE(document->canSelectLinkedGroups());
                document->selectLinkedGroups();
                CHECK_THAT(document->selectedNodes().nodes(), Catch::UnorderedEquals(std::vector<Model::Node*>{groupNode, linkedGroupNode}));
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodestTest.separateGroups", "[GroupNodesTest]") {
            auto* brushNode = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(brushNode);

            auto* groupNode = document->groupSelection("test");
            REQUIRE(groupNode != nullptr);

            document->deselectAll();
            document->select(groupNode);

            SECTION("Separating a group that isn't linked") {
                CHECK_FALSE(document->canSeparateLinkedGroups());
            }

            SECTION("Separating all members of a link set") {
                auto* linkedGroupNode = document->createLinkedDuplicate();
                REQUIRE(linkedGroupNode != nullptr);
                REQUIRE(groupNode->group().linkedGroupId() != std::nullopt);
                REQUIRE(linkedGroupNode->group().linkedGroupId() == groupNode->group().linkedGroupId());

                document->select(std::vector<Model::Node*>{groupNode, linkedGroupNode});
                CHECK_FALSE(document->canSeparateLinkedGroups());
            }

            SECTION("Separating one group from a link set with two members") {
                auto* linkedGroupNode = document->createLinkedDuplicate();
                REQUIRE(linkedGroupNode != nullptr);

                const auto originalLinkedGroupId = groupNode->group().linkedGroupId();
                REQUIRE(originalLinkedGroupId != std::nullopt);
                REQUIRE(linkedGroupNode->group().linkedGroupId() == originalLinkedGroupId);

                document->deselectAll();
                document->select(linkedGroupNode);

                CHECK(document->canSeparateLinkedGroups());
                document->separateLinkedGroups();
                CHECK(groupNode->group().linkedGroupId() == std::nullopt);
                CHECK(linkedGroupNode->group().linkedGroupId() == std::nullopt);

                document->undoCommand();
                CHECK(groupNode->group().linkedGroupId() == originalLinkedGroupId);
                CHECK(linkedGroupNode->group().linkedGroupId() == originalLinkedGroupId);
            }

            SECTION("Separating multiple groups from a link set with several members") {
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
                document->select(std::vector<Model::Node*>{linkedGroupNode2, linkedGroupNode3});
                CHECK(document->canSeparateLinkedGroups());

                document->separateLinkedGroups();
                CHECK(groupNode->group().linkedGroupId() == originalLinkedGroupId);
                CHECK(linkedGroupNode1->group().linkedGroupId() == originalLinkedGroupId);

                CHECK(linkedGroupNode2->group().linkedGroupId() != std::nullopt);
                CHECK(linkedGroupNode2->group().linkedGroupId() != originalLinkedGroupId);
                CHECK(linkedGroupNode3->group().linkedGroupId() == linkedGroupNode2->group().linkedGroupId());

                CHECK(document->selectedNodes().groupCount() == 2u);
                
                document->undoCommand();

                CHECK(groupNode->group().linkedGroupId() == originalLinkedGroupId);
                CHECK(linkedGroupNode1->group().linkedGroupId() == originalLinkedGroupId);
                CHECK(linkedGroupNode2->group().linkedGroupId() == originalLinkedGroupId);
                CHECK(linkedGroupNode3->group().linkedGroupId() == originalLinkedGroupId);
            }
        }

        TEST_CASE_METHOD(MapDocumentTest, "GroupNodesTest.newWithGroupOpen") {
            Model::EntityNode* entity = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), entity);
            document->select(entity);
            Model::GroupNode* group = document->groupSelection("my group");
            document->openGroup(group);

            CHECK(document->currentGroup() == group);

            document->newDocument(Model::MapFormat::Valve, MapDocument::DefaultWorldBounds, document->game());

            CHECK(document->currentGroup() == nullptr);
        }
    }
}
