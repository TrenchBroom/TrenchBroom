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
#include "Model/BrushBuilder.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/NodeCollection.h"
#include "Model/World.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class SelectionTest : public MapDocumentTest {};

        TEST_CASE_METHOD(SelectionTest, "SelectionTest.selectTouchingWithGroup") {
            document->selectAllNodes();
            document->deleteObjects();
            assert(document->selectedNodes().nodeCount() == 0);

            Model::LayerNode* layer = new Model::LayerNode("Layer 1");
            document->addNode(layer, document->world());

            Model::GroupNode* group = new Model::GroupNode("Unnamed");
            document->addNode(group, layer);

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            const vm::bbox3 brushBounds(vm::vec3(-32.0, -32.0, -32.0),
                                    vm::vec3(+32.0, +32.0, +32.0));

            Model::BrushNode* brush = builder.createCuboid(brushBounds, "texture");
            document->addNode(brush, group);

            const vm::bbox3 selectionBounds(vm::vec3(-16.0, -16.0, -48.0),
                                        vm::vec3(+16.0, +16.0, +48.0));

            Model::BrushNode* selectionBrush = builder.createCuboid(selectionBounds, "texture");
            document->addNode(selectionBrush, layer);

            document->select(selectionBrush);
            document->selectTouching(true);

            ASSERT_EQ(1u, document->selectedNodes().nodeCount());
        }

        TEST_CASE_METHOD(SelectionTest, "SelectionTest.selectInsideWithGroup") {
            document->selectAllNodes();
            document->deleteObjects();
            assert(document->selectedNodes().nodeCount() == 0);

            Model::LayerNode* layer = new Model::LayerNode("Layer 1");
            document->addNode(layer, document->world());

            Model::GroupNode* group = new Model::GroupNode("Unnamed");
            document->addNode(group, layer);

            Model::BrushBuilder builder(document->world(), document->worldBounds());
            const vm::bbox3 brushBounds(vm::vec3(-32.0, -32.0, -32.0),
                                    vm::vec3(+32.0, +32.0, +32.0));

            Model::BrushNode* brush = builder.createCuboid(brushBounds, "texture");
            document->addNode(brush, group);

            const vm::bbox3 selectionBounds(vm::vec3(-48.0, -48.0, -48.0),
                                        vm::vec3(+48.0, +48.0, +48.0));

            Model::BrushNode* selectionBrush = builder.createCuboid(selectionBounds, "texture");
            document->addNode(selectionBrush, layer);

            document->select(selectionBrush);
            document->selectInside(true);

            ASSERT_EQ(1u, document->selectedNodes().nodeCount());
        }
    }
}
