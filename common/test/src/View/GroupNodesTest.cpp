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
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/PasteType.h"

#include <kdl/vector_utils.h>

#include <set>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class GroupNodesTest : public MapDocumentTest {};

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createEmptyGroup", "[GroupNodesTest]") {
            CHECK(document->groupSelection("test") == nullptr);
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithOneNode", "[GroupNodesTest]") {
            Model::BrushNode* brush = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush}}});
            document->select(brush);

            Model::GroupNode* group = document->groupSelection("test");
            CHECK(group != nullptr);

            CHECK(brush->parent() == group);
            CHECK(group->selected());
            CHECK_FALSE(brush->selected());

            document->undoCommand();
            CHECK(group->parent() == nullptr);
            CHECK(brush->parent() == document->parentForNodes());
            CHECK(brush->selected());
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithPartialBrushEntity", "[GroupNodesTest]") {
            Model::BrushNode* brush1 = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush1}}});

            Model::BrushNode* brush2 = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush2}}});

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});
            document->reparentNodes({{entity, { brush1, brush2 }}});

            document->select(brush1);

            Model::GroupNode* group = document->groupSelection("test");
            CHECK(group != nullptr);

            CHECK(brush1->parent() == entity);
            CHECK(brush2->parent() == entity);
            CHECK(entity->parent() == group);
            CHECK(group->selected());
            CHECK_FALSE(brush1->selected());

            document->undoCommand();
            CHECK(group->parent() == nullptr);
            CHECK(brush1->parent() == entity);
            CHECK(brush2->parent() == entity);
            CHECK(entity->parent() == document->parentForNodes());
            CHECK_FALSE(group->selected());
            CHECK(brush1->selected());
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithFullBrushEntity", "[GroupNodesTest]") {
            Model::BrushNode* brush1 = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush1}}});

            Model::BrushNode* brush2 = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush2}}});

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entity}}});
            document->reparentNodes({{entity, { brush1, brush2 }}});

            document->select(std::vector<Model::Node*>({ brush1, brush2 }));

            Model::GroupNode* group = document->groupSelection("test");
            CHECK(group != nullptr);

            CHECK(brush1->parent() == entity);
            CHECK(brush2->parent() == entity);
            CHECK(entity->parent() == group);
            CHECK(group->selected());
            CHECK_FALSE(brush1->selected());
            CHECK_FALSE(brush2->selected());

            document->undoCommand();
            CHECK(group->parent() == nullptr);
            CHECK(brush1->parent() == entity);
            CHECK(brush2->parent() == entity);
            CHECK(entity->parent() == document->parentForNodes());
            CHECK_FALSE(group->selected());
            CHECK(brush1->selected());
            CHECK(brush2->selected());
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.pasteInGroup", "[GroupNodesTest]") {
            // https://github.com/TrenchBroom/TrenchBroom/issues/1734

            const std::string data("{"
                              "\"classname\" \"light\""
                              "\"origin\" \"0 0 0\""
                              "}");

            Model::BrushNode* brush = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush}}});
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

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.undoMoveGroupContainingBrushEntity", "[GroupNodesTest]") {
            // Test for issue #1715

            Model::BrushNode* brush1 = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush1}}});

            Model::EntityNode* entityNode = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entityNode}}});
            document->reparentNodes({{entityNode, { brush1 }}});

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
            document->addNodes({{document->parentForNodes(), {brush1}}});

            Model::EntityNode* entityNode = new Model::EntityNode();
            document->addNodes({{document->parentForNodes(), {entityNode}}});
            document->reparentNodes({{entityNode, { brush1 }}});

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
            document->addNodes({{document->parentForNodes(), {brush1}}});
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
            document->addNodes({{document->parentForNodes(), {brush}}});
            document->select(brush);

            Model::GroupNode* group = document->groupSelection("test");
            REQUIRE(group != nullptr);

            document->openGroup(group);

            document->select(brush);
            document->duplicateObjects();

            Model::BrushNode* brushCopy = document->selectedNodes().brushes().at(0u);
            CHECK(brushCopy->parent() == group);
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createLinkedGroup", "[GroupNodesTest]") {
            auto* brushNode = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(brushNode);

            auto* groupNode = document->groupSelection("test");
            REQUIRE(groupNode != nullptr);

            document->deselectAll();
            document->select(groupNode);

            auto* linkedGroupNode = document->createLinkedGroup();
            CHECK(linkedGroupNode != nullptr);

            CHECK(groupNode->connectedToLinkSet());
            CHECK_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode, linkedGroupNode}));
            
            CHECK(linkedGroupNode->connectedToLinkSet());
            CHECK_THAT(linkedGroupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode, linkedGroupNode}));
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodestTest.unlinkGroups", "[GroupNodesTest]") {
            auto* brushNode = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brushNode}}});
            document->select(brushNode);

            auto* groupNode = document->groupSelection("test");
            REQUIRE(groupNode != nullptr);

            document->deselectAll();
            document->select(groupNode);

            SECTION("Unlinking a group from a singleton link set") {
                CHECK_FALSE(document->canUnlinkGroups());
                document->unlinkGroups();

                CHECK(groupNode->parent() != nullptr);
                CHECK_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode}));
            }

            SECTION("Unlinking a group from a link set with two members") {
                auto* linkedGroupNode = document->createLinkedGroup();
                REQUIRE(linkedGroupNode != nullptr);
                REQUIRE_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode, linkedGroupNode}));

                document->select(std::vector<Model::Node*>{groupNode, linkedGroupNode});
                CHECK_FALSE(document->canUnlinkGroups());

                document->deselectAll();
                document->select(linkedGroupNode);

                CHECK(document->canUnlinkGroups());
                document->unlinkGroups();

                CHECK_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode}));
                CHECK(document->selectedNodes().groupCount() == 1u);
                
                auto* newLinkedGroupNode = document->selectedNodes().groups().front();
                CHECK_THAT(newLinkedGroupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{newLinkedGroupNode}));

                document->undoCommand();
                REQUIRE(linkedGroupNode->parent() != nullptr);
                REQUIRE_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode, linkedGroupNode}));
            }

            SECTION("Unlinking multiple groups from a link set with several members") {
                auto* linkedGroupNode1 = document->createLinkedGroup();
                auto* linkedGroupNode2 = document->createLinkedGroup();
                auto* linkedGroupNode3 = document->createLinkedGroup();

                REQUIRE(linkedGroupNode1 != nullptr);
                REQUIRE(linkedGroupNode2 != nullptr);
                REQUIRE(linkedGroupNode3 != nullptr);
                REQUIRE_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode, linkedGroupNode1, linkedGroupNode2, linkedGroupNode3}));

                document->deselectAll();
                document->select(std::vector<Model::Node*>{linkedGroupNode2, linkedGroupNode3});
                CHECK(document->canUnlinkGroups());

                document->unlinkGroups();

                CHECK_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode, linkedGroupNode1}));
                CHECK(document->selectedNodes().groupCount() == 2u);
                
                const auto& newLinkedGroupNodes = document->selectedNodes().groups();
                for (const auto* newLinkedGroupNode : newLinkedGroupNodes) {
                    CHECK_THAT(newLinkedGroupNode->linkedGroups(), Catch::UnorderedEquals(newLinkedGroupNodes));
                }

                document->undoCommand();

                CHECK(linkedGroupNode2->parent() != nullptr);
                CHECK(linkedGroupNode3->parent() != nullptr);
                CHECK_THAT(groupNode->linkedGroups(), Catch::UnorderedEquals(std::vector<Model::GroupNode*>{groupNode, linkedGroupNode1, linkedGroupNode2, linkedGroupNode3}));
            }
        }
    }
}
