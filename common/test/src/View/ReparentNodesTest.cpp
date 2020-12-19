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
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class ReparentNodesTest : public MapDocumentTest {};

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentLayerToLayer") {
            Model::LayerNode* layer1 = new Model::LayerNode("Layer 1");
            document->addNode(layer1, document->world());

            Model::LayerNode* layer2 = new Model::LayerNode("Layer 2");
            document->addNode(layer2, document->world());

            CHECK_FALSE(document->reparentNodes(layer2, { layer1 }));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentBetweenLayers") {
            Model::LayerNode* oldParent = new Model::LayerNode("Layer 1");
            document->addNode(oldParent, document->world());

            Model::LayerNode* newParent = new Model::LayerNode("Layer 2");
            document->addNode(newParent, document->world());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, oldParent);

            assert(entity->parent() == oldParent);
            CHECK(document->reparentNodes(newParent, { entity }));
            CHECK(entity->parent() == newParent);

            document->undoCommand();
            CHECK(entity->parent() == oldParent);
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentGroupToItself") {
            Model::GroupNode* group = new Model::GroupNode("Group");
            document->addNode(group, document->parentForNodes());

            CHECK_FALSE(document->reparentNodes(group, { group }));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentGroupToChild") {
            Model::GroupNode* outer = new Model::GroupNode("Outer");
            document->addNode(outer, document->parentForNodes());

            Model::GroupNode* inner = new Model::GroupNode("Inner");
            document->addNode(inner, outer);

            CHECK_FALSE(document->reparentNodes(inner, { outer }));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyGroup") {
            Model::GroupNode* group = new Model::GroupNode("Group");
            document->addNode(group, document->parentForNodes());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, group);

            CHECK(document->reparentNodes(document->parentForNodes(), { entity }));
            CHECK(entity->parent() == document->parentForNodes());
            CHECK(group->parent() == nullptr);

            document->undoCommand();
            CHECK(group->parent() == document->parentForNodes());
            CHECK(entity->parent() == group);
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.recursivelyRemoveEmptyGroups") {
            Model::GroupNode* outer = new Model::GroupNode("Outer");
            document->addNode(outer, document->parentForNodes());

            Model::GroupNode* inner = new Model::GroupNode("Inner");
            document->addNode(inner, outer);

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, inner);

            CHECK(document->reparentNodes(document->parentForNodes(), { entity }));
            CHECK(entity->parent() == document->parentForNodes());
            CHECK(inner->parent() == nullptr);
            CHECK(outer->parent() == nullptr);

            document->undoCommand();
            CHECK(outer->parent() == document->parentForNodes());
            CHECK(inner->parent() == outer);
            CHECK(entity->parent() == inner);
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyEntity") {
            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, document->parentForNodes());

            Model::BrushNode* brush = createBrushNode();
            document->addNode(brush, entity);

            CHECK(document->reparentNodes(document->parentForNodes(), { brush }));
            CHECK(brush->parent() == document->parentForNodes());
            CHECK(entity->parent() == nullptr);

            document->undoCommand();
            CHECK(entity->parent() == document->parentForNodes());
            CHECK(brush->parent() == entity);
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyGroupAndEntity") {
            Model::GroupNode* group = new Model::GroupNode("Group");
            document->addNode(group, document->parentForNodes());

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNode(entity, group);

            Model::BrushNode* brush = createBrushNode();
            document->addNode(brush, entity);

            CHECK(document->reparentNodes(document->parentForNodes(), { brush }));
            CHECK(brush->parent() == document->parentForNodes());
            CHECK(group->parent() == nullptr);
            CHECK(entity->parent() == nullptr);

            document->undoCommand();
            CHECK(group->parent() == document->parentForNodes());
            CHECK(entity->parent() == group);
            CHECK(brush->parent() == entity);
        }
    }
}
