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

#include <catch2/catch.hpp>

#include "GTestCompat.h"

#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/PasteType.h"

#include <set>

namespace TrenchBroom {
    namespace View {
        class GroupNodesTest : public MapDocumentTest {};

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createEmptyGroup") {
            ASSERT_EQ(nullptr, document->groupSelection("test"));
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithOneNode") {
            Model::BrushNode* brush = createBrushNode();
            document->addNode(brush, document->currentParent());
            document->select(brush);

            Model::GroupNode* group = document->groupSelection("test");
            ASSERT_TRUE(group != nullptr);

            ASSERT_EQ(group, brush->parent());
            ASSERT_TRUE(group->selected());
            ASSERT_FALSE(brush->selected());

            document->undoCommand();
            ASSERT_EQ(nullptr, group->parent());
            ASSERT_EQ(document->currentParent(), brush->parent());
            ASSERT_TRUE(brush->selected());
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithPartialBrushEntity") {
            Model::BrushNode* brush1 = createBrushNode();
            document->addNode(brush1, document->currentParent());

            Model::BrushNode* brush2 = createBrushNode();
            document->addNode(brush2, document->currentParent());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, { brush1, brush2 });

            document->select(brush1);

            Model::GroupNode* group = document->groupSelection("test");
            ASSERT_TRUE(group != nullptr);

            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(group, entity->parent());
            ASSERT_TRUE(group->selected());
            ASSERT_FALSE(brush1->selected());

            document->undoCommand();
            ASSERT_EQ(nullptr, group->parent());
            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_FALSE(group->selected());
            ASSERT_TRUE(brush1->selected());
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.createGroupWithFullBrushEntity") {
            Model::BrushNode* brush1 = createBrushNode();
            document->addNode(brush1, document->currentParent());

            Model::BrushNode* brush2 = createBrushNode();
            document->addNode(brush2, document->currentParent());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, { brush1, brush2 });

            document->select(std::vector<Model::Node*>({ brush1, brush2 }));

            Model::GroupNode* group = document->groupSelection("test");
            ASSERT_TRUE(group != nullptr);

            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(group, entity->parent());
            ASSERT_TRUE(group->selected());
            ASSERT_FALSE(brush1->selected());
            ASSERT_FALSE(brush2->selected());

            document->undoCommand();
            ASSERT_EQ(nullptr, group->parent());
            ASSERT_EQ(entity, brush1->parent());
            ASSERT_EQ(entity, brush2->parent());
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_FALSE(group->selected());
            ASSERT_TRUE(brush1->selected());
            ASSERT_TRUE(brush2->selected());
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.pasteInGroup") {
            // https://github.com/kduske/TrenchBroom/issues/1734

            const std::string data("{"
                              "\"classname\" \"light\""
                              "\"origin\" \"0 0 0\""
                              "}");

            Model::BrushNode* brush = createBrushNode();
            document->addNode(brush, document->currentParent());
            document->select(brush);

            Model::GroupNode* group = document->groupSelection("test");
            document->openGroup(group);

            ASSERT_EQ(PasteType::Node, document->paste(data));
            ASSERT_TRUE(document->selectedNodes().hasOnlyEntities());
            ASSERT_EQ(1u, document->selectedNodes().entityCount());

            Model::EntityNode* light = document->selectedNodes().entities().front();
            ASSERT_EQ(group, light->parent());
        }

        static bool hasEmptyName(const std::vector<std::string>& names) {
            for (const auto& name : names) {
                if (name.empty()) {
                    return true;
                }
            }
            return false;
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.undoMoveGroupContainingBrushEntity") {
            // Test for issue #1715

            Model::BrushNode* brush1 = createBrushNode();
            document->addNode(brush1, document->currentParent());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, { brush1 });

            document->select(brush1);

            Model::GroupNode* group = document->groupSelection("test");
            ASSERT_TRUE(group->selected());

            ASSERT_TRUE(document->translateObjects(vm::vec3(16,0,0)));

            ASSERT_FALSE(hasEmptyName(entity->attributeNames()));

            document->undoCommand();

            ASSERT_FALSE(hasEmptyName(entity->attributeNames()));
        }

        TEST_CASE_METHOD(GroupNodesTest, "GroupNodesTest.rotateGroupContainingBrushEntity") {
            // Test for issue #1754

            Model::BrushNode* brush1 = createBrushNode();
            document->addNode(brush1, document->currentParent());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->currentParent());
            document->reparentNodes(entity, { brush1 });

            document->select(brush1);

            Model::GroupNode* group = document->groupSelection("test");
            ASSERT_TRUE(group->selected());

            EXPECT_FALSE(entity->hasAttribute("origin"));
            ASSERT_TRUE(document->rotateObjects(vm::vec3::zero(), vm::vec3::pos_z(), static_cast<FloatType>(10.0)));
            EXPECT_FALSE(entity->hasAttribute("origin"));

            document->undoCommand();

            EXPECT_FALSE(entity->hasAttribute("origin"));
        }
    }
}
