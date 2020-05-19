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
#include "Model/Entity.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/World.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class ReparentNodesTest : public MapDocumentTest {};

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentLayerToLayer") {
            Model::LayerNode* layer1 = new Model::LayerNode("Layer 1");
            document->addNode(layer1, document->world());

            Model::LayerNode* layer2 = new Model::LayerNode("Layer 2");
            document->addNode(layer2, document->world());

            ASSERT_FALSE(document->reparentNodes(layer2, { layer1 }));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentBetweenLayers") {
            Model::LayerNode* oldParent = new Model::LayerNode("Layer 1");
            document->addNode(oldParent, document->world());

            Model::LayerNode* newParent = new Model::LayerNode("Layer 2");
            document->addNode(newParent, document->world());

            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, oldParent);

            assert(entity->parent() == oldParent);
            ASSERT_TRUE(document->reparentNodes(newParent, { entity }));
            ASSERT_EQ(newParent, entity->parent());

            document->undoCommand();
            ASSERT_EQ(oldParent, entity->parent());
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentGroupToItself") {
            Model::GroupNode* group = new Model::GroupNode("Group");
            document->addNode(group, document->currentParent());

            ASSERT_FALSE(document->reparentNodes(group, { group }));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentGroupToChild") {
            Model::GroupNode* outer = new Model::GroupNode("Outer");
            document->addNode(outer, document->currentParent());

            Model::GroupNode* inner = new Model::GroupNode("Inner");
            document->addNode(inner, outer);

            ASSERT_FALSE(document->reparentNodes(inner, { outer }));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyGroup") {
            Model::GroupNode* group = new Model::GroupNode("Group");
            document->addNode(group, document->currentParent());

            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, group);

            ASSERT_TRUE(document->reparentNodes(document->currentParent(), { entity }));
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_TRUE(group->parent() == nullptr);

            document->undoCommand();
            ASSERT_EQ(document->currentParent(), group->parent());
            ASSERT_EQ(group, entity->parent());
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.recursivelyRemoveEmptyGroups") {
            Model::GroupNode* outer = new Model::GroupNode("Outer");
            document->addNode(outer, document->currentParent());

            Model::GroupNode* inner = new Model::GroupNode("Inner");
            document->addNode(inner, outer);

            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, inner);

            ASSERT_TRUE(document->reparentNodes(document->currentParent(), { entity }));
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_TRUE(inner->parent() == nullptr);
            ASSERT_TRUE(outer->parent() == nullptr);

            document->undoCommand();
            ASSERT_EQ(document->currentParent(), outer->parent());
            ASSERT_EQ(outer, inner->parent());
            ASSERT_EQ(inner, entity->parent());
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyEntity") {
            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, document->currentParent());

            Model::BrushNode* brush = createBrush();
            document->addNode(brush, entity);

            ASSERT_TRUE(document->reparentNodes(document->currentParent(), { brush }));
            ASSERT_EQ(document->currentParent(), brush->parent());
            ASSERT_TRUE(entity->parent() == nullptr);

            document->undoCommand();
            ASSERT_EQ(document->currentParent(), entity->parent());
            ASSERT_EQ(entity, brush->parent());
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyGroupAndEntity") {
            Model::GroupNode* group = new Model::GroupNode("Group");
            document->addNode(group, document->currentParent());

            Model::Entity* entity = new Model::Entity();
            document->addNode(entity, group);

            Model::BrushNode* brush = createBrush();
            document->addNode(brush, entity);

            ASSERT_TRUE(document->reparentNodes(document->currentParent(), { brush }));
            ASSERT_EQ(document->currentParent(), brush->parent());
            ASSERT_TRUE(group->parent() == nullptr);
            ASSERT_TRUE(entity->parent() == nullptr);

            document->undoCommand();
            ASSERT_EQ(document->currentParent(), group->parent());
            ASSERT_EQ(group, entity->parent());
            ASSERT_EQ(entity, brush->parent());
        }
    }
}
