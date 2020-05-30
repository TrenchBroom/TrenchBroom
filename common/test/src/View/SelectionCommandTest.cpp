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

#include "Model/BrushBuilder.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"

namespace TrenchBroom {
    namespace View {
        class SelectionCommandTest : public MapDocumentTest {};

        TEST_CASE_METHOD(SelectionCommandTest, "SelectionCommandTest.faceSelectionUndoAfterTranslationUndo") {
            Model::BrushNode* brushNode = createBrushNode();
            ASSERT_EQ(vm::vec3::zero(), brushNode->logicalBounds().center());

            document->addNode(brushNode, document->parentForNodes());

            const auto topFaceIndex = brushNode->brush().findFace(vm::vec3::pos_z());
            REQUIRE(topFaceIndex);

            // select the top face
            document->select({ brushNode, *topFaceIndex });
            ASSERT_EQ(std::vector<Model::BrushFaceHandle>({{ brushNode, *topFaceIndex }}), document->selectedBrushFaces());

            // deselect it
            document->deselect({ brushNode, *topFaceIndex });
            ASSERT_EQ(std::vector<Model::BrushFaceHandle>({}), document->selectedBrushFaces());

            // select the brush
            document->select(brushNode);
            ASSERT_EQ(std::vector<Model::BrushNode*>({ brushNode }), document->selectedNodes().brushes());

            // translate the brush
            document->translateObjects(vm::vec3(10.0, 0.0, 0.0));
            ASSERT_EQ(vm::vec3(10.0, 0.0, 0.0), brushNode->logicalBounds().center());

            // Start undoing changes

            document->undoCommand();
            ASSERT_EQ(vm::vec3::zero(), brushNode->logicalBounds().center());
            ASSERT_EQ(std::vector<Model::BrushNode*>({ brushNode }), document->selectedNodes().brushes());
            ASSERT_EQ(std::vector<Model::BrushFaceHandle>({}), document->selectedBrushFaces());

            document->undoCommand();
            ASSERT_EQ(std::vector<Model::BrushNode*>({}), document->selectedNodes().brushes());
            ASSERT_EQ(std::vector<Model::BrushFaceHandle>({}), document->selectedBrushFaces());

            document->undoCommand();
            ASSERT_EQ(std::vector<Model::BrushFaceHandle>({{ brushNode, *topFaceIndex}}), document->selectedBrushFaces());
        }
    }
}
