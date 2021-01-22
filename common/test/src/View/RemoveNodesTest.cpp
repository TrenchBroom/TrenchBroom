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
#include "Model/BrushBuilder.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include <cstdio>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class RemoveNodesTest : public MapDocumentTest {};

        TEST_CASE_METHOD(RemoveNodesTest, "RemoveNodesTest.removeLayer") {
            Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
            document->addNodes({{document->world(), {layer}}});

            document->removeNodes({layer});
            CHECK(layer->parent() == nullptr);

            document->undoCommand();
            CHECK(layer->parent() == document->world());
        }

        TEST_CASE_METHOD(RemoveNodesTest, "RemoveNodesTest.removeEmptyBrushEntity") {
            Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
            document->addNodes({{document->world(), {layer}}});

            Model::EntityNode* entity = new Model::EntityNode();
            document->addNodes({{layer, {entity}}});

            Model::BrushNode* brush = createBrushNode();
            document->addNodes({{entity, {brush}}});

            document->removeNodes({brush});
            CHECK(brush->parent() == nullptr);
            CHECK(entity->parent() == nullptr);

            document->undoCommand();
            CHECK(brush->parent() == entity);
            CHECK(entity->parent() == layer);
        }

        TEST_CASE_METHOD(RemoveNodesTest, "RemoveNodesTest.removeEmptyGroup") {
            Model::GroupNode* group = new Model::GroupNode(Model::Group("group"));
            document->addNodes({{document->parentForNodes(), {group}}});

            document->openGroup(group);

            Model::BrushNode* brush = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush}}});

            document->removeNodes({brush});
            CHECK(document->currentGroup() == nullptr);
            CHECK(brush->parent() == nullptr);
            CHECK(group->parent() == nullptr);

            document->undoCommand();
            CHECK(document->currentGroup() == group);
            CHECK(brush->parent() == group);
            CHECK(group->parent() == document->world()->defaultLayer());
        }

        TEST_CASE_METHOD(RemoveNodesTest, "RemoveNodesTest.recursivelyRemoveEmptyGroups") {
            Model::GroupNode* outer = new Model::GroupNode(Model::Group("outer"));
            document->addNodes({{document->parentForNodes(), {outer}}});

            document->openGroup(outer);

            Model::GroupNode* inner = new Model::GroupNode(Model::Group("inner"));
            document->addNodes({{document->parentForNodes(), {inner}}});

            document->openGroup(inner);

            Model::BrushNode* brush = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brush}}});

            document->removeNodes({brush});
            CHECK(document->currentGroup() == nullptr);
            CHECK(brush->parent() == nullptr);
            CHECK(inner->parent() == nullptr);
            CHECK(outer->parent() == nullptr);

            document->undoCommand();
            CHECK(document->currentGroup() == inner);
            CHECK(brush->parent() == inner);
            CHECK(inner->parent() == outer);
            CHECK(outer->parent() == document->world()->defaultLayer());
        }

        TEST_CASE_METHOD(RemoveNodesTest, "RemoveNodesTest.disconnectRemovedSingletonGroups") {
            auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
            
            document->addNodes({{document->parentForNodes(), {groupNode}}});
            REQUIRE(groupNode->connectedToLinkSet());

            document->removeNodes({groupNode});
            CHECK_FALSE(groupNode->connectedToLinkSet());

            document->undoCommand();
            CHECK(groupNode->connectedToLinkSet());
        }

        TEST_CASE_METHOD(RemoveNodesTest, "RemoveNodesTest.disconnectRemovedLinkedGroups") {
            auto* groupNode = new Model::GroupNode{Model::Group{"group"}};
            auto* linkedGroupNode = static_cast<Model::GroupNode*>(groupNode->cloneRecursively(document->worldBounds()));
            document->addNodes({{document->parentForNodes(), {groupNode, linkedGroupNode}}});

            REQUIRE(groupNode->connectedToLinkSet());
            REQUIRE(linkedGroupNode->connectedToLinkSet());

            document->removeNodes({groupNode});
            CHECK_FALSE(groupNode->connectedToLinkSet());
            CHECK(linkedGroupNode->connectedToLinkSet());

            document->undoCommand();
            CHECK(groupNode->connectedToLinkSet());
        }

        TEST_CASE_METHOD(RemoveNodesTest, "RemoveNodesTest.recursivelyDisconnectRemovedSingletonGroups") {
            auto* outer = new Model::GroupNode{Model::Group{"outer"}};
            auto* inner = new Model::GroupNode{Model::Group{"inner"}};
            outer->addChild(inner);

            document->addNodes({{document->parentForNodes(), {outer}}});
            REQUIRE(outer->connectedToLinkSet());
            REQUIRE(inner->connectedToLinkSet());

            document->removeNodes({outer});
            CHECK_FALSE(outer->connectedToLinkSet());
            CHECK_FALSE(inner->connectedToLinkSet());

            document->undoCommand();
            CHECK(outer->connectedToLinkSet());
            CHECK(inner->connectedToLinkSet());
        }
    }
}
