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

#include "Exceptions.h"
#include "Model/BrushNode.h"
#include "Model/BrushBuilder.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/NodeCollection.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

#include <kdl/result.h>

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class SelectionTest : public MapDocumentTest {};

        TEST_CASE_METHOD(SelectionTest, "SelectionTest.selectTouchingWithGroup") {
            document->selectAllNodes();
            document->deleteObjects();
            assert(document->selectedNodes().nodeCount() == 0);

            Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
            document->addNodes({{document->world(), {layer}}});

            Model::GroupNode* group = new Model::GroupNode(Model::Group("Unnamed"));
            document->addNodes({{layer, {group}}});

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const vm::bbox3 brushBounds(vm::vec3(-32.0, -32.0, -32.0),
                                    vm::vec3(+32.0, +32.0, +32.0));

            Model::BrushNode* brush = new Model::BrushNode(builder.createCuboid(brushBounds, "texture").value());
            document->addNodes({{group, {brush}}});

            const vm::bbox3 selectionBounds(vm::vec3(-16.0, -16.0, -48.0),
                                        vm::vec3(+16.0, +16.0, +48.0));

            Model::BrushNode* selectionBrush = new Model::BrushNode(builder.createCuboid(selectionBounds, "texture").value());
            document->addNodes({{layer, {selectionBrush}}});

            document->select(selectionBrush);
            document->selectTouching(true);

            CHECK(document->selectedNodes().nodeCount() == 1u);
        }

        TEST_CASE_METHOD(SelectionTest, "SelectionTest.selectInsideWithGroup") {
            document->selectAllNodes();
            document->deleteObjects();
            assert(document->selectedNodes().nodeCount() == 0);

            Model::LayerNode* layer = new Model::LayerNode(Model::Layer("Layer 1"));
            document->addNodes({{document->world(), {layer}}});

            Model::GroupNode* group = new Model::GroupNode(Model::Group("Unnamed"));
            document->addNodes({{layer, {group}}});

            Model::BrushBuilder builder(document->world()->mapFormat(), document->worldBounds());
            const vm::bbox3 brushBounds(vm::vec3(-32.0, -32.0, -32.0),
                                    vm::vec3(+32.0, +32.0, +32.0));

            Model::BrushNode* brush = new Model::BrushNode(builder.createCuboid(brushBounds, "texture").value());
            document->addNodes({{group, {brush}}});

            const vm::bbox3 selectionBounds(vm::vec3(-48.0, -48.0, -48.0),
                                        vm::vec3(+48.0, +48.0, +48.0));

            Model::BrushNode* selectionBrush = new Model::BrushNode(builder.createCuboid(selectionBounds, "texture").value());
            document->addNodes({{layer, {selectionBrush}}});

            document->select(selectionBrush);
            document->selectInside(true);

            CHECK(document->selectedNodes().nodeCount() == 1u);
        }
        
        TEST_CASE_METHOD(SelectionTest, "SelectionTest.updateLastSelectionBounds") {
            auto* entityNode = new Model::EntityNode({
                {"classname", "point_entity"}
            });
            document->addNodes({{document->parentForNodes(), {entityNode}}});
            REQUIRE(!entityNode->logicalBounds().is_empty());
            
            document->selectAllNodes();
            
            auto bounds = document->selectionBounds();
            document->deselectAll();
            CHECK(document->lastSelectionBounds() == bounds);
            
            document->deselectAll();
            CHECK(document->lastSelectionBounds() == bounds);
            
            auto* brushNode = createBrushNode();
            document->addNodes({{document->parentForNodes(), {brushNode}}});
            
            document->select(brushNode);
            CHECK(document->lastSelectionBounds() == bounds);
            
            bounds = brushNode->logicalBounds();
            
            document->deselectAll();
            CHECK(document->lastSelectionBounds() == bounds);
        }
    }
}
