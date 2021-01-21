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
            Model::LayerNode* layer1 = new Model::LayerNode(Model::Layer("Layer 1"));
            document->addNodes({{document->world(), {layer1}}});

            Model::LayerNode* layer2 = new Model::LayerNode(Model::Layer("Layer 2"));
            document->addNodes({{document->world(), {layer2}}});

            CHECK_FALSE(document->reparentNodes({{layer2, { layer1 }}}));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentBetweenLayers") {
            Model::LayerNode* oldParent = new Model::LayerNode(Model::Layer("Layer 1"));
            document->addNodes({{document->world(), {oldParent}}});

            Model::LayerNode* newParent = new Model::LayerNode(Model::Layer("Layer 2"));
            document->addNodes({{document->world(), {newParent}}});

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{oldParent, {entity}}});

            assert(entity->parent() == oldParent);
            CHECK(document->reparentNodes({{newParent, { entity }}}));
            CHECK(entity->parent() == newParent);

            document->undoCommand();
            CHECK(entity->parent() == oldParent);
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentGroupToItself") {
            Model::GroupNode* group = new Model::GroupNode(Model::Group("Group"));
            document->addNodes({{document->parentForNodes(), {group}}});

            CHECK_FALSE(document->reparentNodes({{group, { group }}}));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.reparentGroupToChild") {
            Model::GroupNode* outer = new Model::GroupNode(Model::Group("Outer"));
            document->addNodes({{document->parentForNodes(), {outer}}});

            Model::GroupNode* inner = new Model::GroupNode(Model::Group("Inner"));
            document->addNodes({{outer, {inner}}});

            CHECK_FALSE(document->reparentNodes({{inner, { outer }}}));
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyGroup") {
            Model::GroupNode* group = new Model::GroupNode(Model::Group("Group"));
            document->addNodes({{document->parentForNodes(), {group}}});

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{group, {entity}}});

            CHECK(document->reparentNodes({{document->parentForNodes(), { entity }}}));
            CHECK(entity->parent() == document->parentForNodes());
            CHECK(group->parent() == nullptr);

            document->undoCommand();
            CHECK(group->parent() == document->parentForNodes());
            CHECK(entity->parent() == group);
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.recursivelyRemoveEmptyGroups") {
            Model::GroupNode* outer = new Model::GroupNode(Model::Group("Outer"));
            document->addNodes({{document->parentForNodes(), {outer}}});

            Model::GroupNode* inner = new Model::GroupNode(Model::Group("Inner"));
            document->addNodes({{outer, {inner}}});

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{inner, {entity}}});

            CHECK(document->reparentNodes({{document->parentForNodes(), { entity }}}));
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
            document->addNodes({{document->parentForNodes(), {entity}}});

            Model::BrushNode* brush = createBrushNode();
            document->addNodes({{entity, {brush}}});

            CHECK(document->reparentNodes({{document->parentForNodes(), { brush }}}));
            CHECK(brush->parent() == document->parentForNodes());
            CHECK(entity->parent() == nullptr);

            document->undoCommand();
            CHECK(entity->parent() == document->parentForNodes());
            CHECK(brush->parent() == entity);
        }

        TEST_CASE_METHOD(ReparentNodesTest, "ReparentNodesTest.removeEmptyGroupAndEntity") {
            Model::GroupNode* group = new Model::GroupNode(Model::Group("Group"));
            document->addNodes({{document->parentForNodes(), {group}}});

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{group, {entity}}});

            Model::BrushNode* brush = createBrushNode();
            document->addNodes({{entity, {brush}}});

            CHECK(document->reparentNodes({{document->parentForNodes(), { brush }}}));
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
