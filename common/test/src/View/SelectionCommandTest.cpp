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
#include "Model/Entity.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"

namespace TrenchBroom {
    namespace View {
        class SelectionCommandTest : public MapDocumentTest {};

        TEST_CASE_METHOD(SelectionCommandTest, "SelectionCommandTest.faceSelectionUndoAfterTranslationUndo") {
            Model::BrushNode* brush = createBrush();
            ASSERT_EQ(vm::vec3::zero(), brush->logicalBounds().center());

            document->addNode(brush, document->currentParent());

            // select the top face
            document->select(brush->findFace(vm::vec3::pos_z()));
            ASSERT_EQ(std::vector<Model::BrushFace*>({ brush->findFace(vm::vec3::pos_z()) }), document->selectedBrushFaces());

            // deselect it
            document->deselect(brush->findFace(vm::vec3::pos_z()));
            ASSERT_EQ(std::vector<Model::BrushFace*>({}), document->selectedBrushFaces());

            // select the brush
            document->select(brush);
            ASSERT_EQ(std::vector<Model::BrushNode*>({ brush }), document->selectedNodes().brushes());

            // translate the brush
            document->translateObjects(vm::vec3(10.0, 0.0, 0.0));
            ASSERT_EQ(vm::vec3(10.0, 0.0, 0.0), brush->logicalBounds().center());

            // Start undoing changes

            document->undoCommand();
            ASSERT_EQ(vm::vec3::zero(), brush->logicalBounds().center());
            ASSERT_EQ(std::vector<Model::BrushNode*>({ brush }), document->selectedNodes().brushes());
            ASSERT_EQ(std::vector<Model::BrushFace*>({}), document->selectedBrushFaces());

            document->undoCommand();
            ASSERT_EQ(std::vector<Model::BrushNode*>({}), document->selectedNodes().brushes());
            ASSERT_EQ(std::vector<Model::BrushFace*>({}), document->selectedBrushFaces());

            document->undoCommand();
            ASSERT_EQ(std::vector<Model::BrushFace*>({ brush->findFace(vm::vec3::pos_z()) }), document->selectedBrushFaces());
        }
    }
}
