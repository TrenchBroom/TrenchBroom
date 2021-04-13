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
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/PasteType.h"

#include <functional>
#include <set>

#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class GroupNodesTest : public MapDocumentTest {};

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createEmptyGroup", "[GroupNodesTest]") {
            CHECK(document->groupSelection("test") == nullptr);
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithOneNode", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithPartialBrushEntity", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithFullBrushEntity", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.pasteInGroup", "[GroupNodesTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1734

            const std::string data("{"
                              "\"classname\" \"light\""
                              "\"origin\" \"0 0 0\""
                              "}");

            Model::BrushNode* brush = createBrushNode();
            addNode(*document, document->parentForNodes(), brush);
            document->select(brush);

            Model::GroupNode* group = document->groupSelection("test");
            document->openGroup(group);

            CHECK(document->paste(data) == PasteType::Node);
            CHECK(document->selectedNodes().hasOnlyEntities());
            CHECK(document->selectedNodes().entityCount() == 1u);

            Model::EntityNode* light = document->selectedNodes().entities().front();
            CHECK(light->parent() == group);
        }

        static bool hasEmptyName(const std::vector<std::string>& names) {
            for (const auto& name : names) {
                if (name.empty()) {
                    return true;
                }
            }
            return false;
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.copyPasteGroupResetsDuplicateGroupId", "[GroupNodesTest]") {
            auto* entityNode = new Model::EntityNode{};
            document->addNodes({{document->parentForNodes(), {entityNode}}});

            document->select(entityNode);
            auto* groupNode = document->groupSelection("test");

            const auto persistentGroupId = groupNode->persistentId();
            REQUIRE(persistentGroupId.has_value());

            document->deselectAll();
            document->select(groupNode);

            const auto str = document->serializeSelectedNodes();

            SECTION("Copy and paste resets persistent group ID") {
                document->deselectAll();
                REQUIRE(document->paste(str) == PasteType::Node);

                auto* pastedGroupNode = dynamic_cast<Model::GroupNode*>(document->world()->defaultLayer()->children().back());
                REQUIRE(pastedGroupNode != nullptr);
                REQUIRE(pastedGroupNode != groupNode);

                CHECK(pastedGroupNode->persistentId() != persistentGroupId);
            }

            SECTION("Cut and paste retains persistent group ID") {
                document->deleteObjects();
                document->deselectAll();
                REQUIRE(document->paste(str) == PasteType::Node);

                auto* pastedGroupNode = dynamic_cast<Model::GroupNode*>(document->world()->defaultLayer()->children().back());
                REQUIRE(pastedGroupNode != nullptr);
                REQUIRE(pastedGroupNode != groupNode);

                CHECK(pastedGroupNode->persistentId() == persistentGroupId);
            }
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.undoMoveGroupContainingBrushEntity", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.rotateGroupContainingBrushEntity", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.renameGroup", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.duplicateNodeInGroup", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.ungroupLinkedGroups", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createLinkedDuplicate", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.selectLinkedGroups", "[GroupNodesTest]") {
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodestTest.separateGroups", "[GroupNodesTest]") {
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
    }
}
