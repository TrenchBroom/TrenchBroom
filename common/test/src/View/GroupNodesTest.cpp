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
            Model::BrushNode* brush = createBrushNode();
            addNode(*document, document->parentForNodes(), brush);
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
            addNode(*document, document->parentForNodes(), brush1);

            Model::BrushNode* brush2 = createBrushNode();
            addNode(*document, document->parentForNodes(), brush2);

            Model::EntityNode* entity = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), entity);
            reparentNodes(*document, entity, { brush1, brush2 });

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
            addNode(*document, document->parentForNodes(), brush1);

            Model::BrushNode* brush2 = createBrushNode();
            addNode(*document, document->parentForNodes(), brush2);

            Model::EntityNode* entity = new Model::EntityNode();
            addNode(*document, document->parentForNodes(), entity);
            reparentNodes(*document, entity, { brush1, brush2 });

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
            REQUIRE(document->duplicateObjects());

            Model::BrushNode* brushCopy = document->selectedNodes().brushes().at(0u);
            CHECK(brushCopy->parent() == group);
        }
    }
}
