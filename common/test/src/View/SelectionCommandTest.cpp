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

#include "Model/BrushBuilder.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"

#include "Catch2.h"

namespace TrenchBroom {
    namespace View {
        class SelectionCommandTest : public MapDocumentTest {};

        TEST_CASE_METHOD(SelectionCommandTest, "SelectionCommandTest.faceSelectionUndoAfterTranslationUndo") {
            Model::BrushNode* brushNode = createBrushNode();
            CHECK(brushNode->logicalBounds().center() == vm::vec3::zero());

            document->addNodes({{document->parentForNodes(), {brushNode}}});

            const auto topFaceIndex = brushNode->brush().findFace(vm::vec3::pos_z());
            REQUIRE(topFaceIndex);

            // select the top face
            document->select({ brushNode, *topFaceIndex });
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{{ brushNode, *topFaceIndex }}));

            // deselect it
            document->deselect({ brushNode, *topFaceIndex });
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

            // select the brush
            document->select(brushNode);
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{ brushNode }));

            // translate the brush
            document->translateObjects(vm::vec3(10.0, 0.0, 0.0));
            CHECK(brushNode->logicalBounds().center() == vm::vec3(10.0, 0.0, 0.0));

            // Start undoing changes

            document->undoCommand();
            CHECK(brushNode->logicalBounds().center() == vm::vec3::zero());
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{ brushNode }));
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

            document->undoCommand();
            CHECK_THAT(document->selectedNodes().brushes(), Catch::Equals(std::vector<Model::BrushNode*>{}));
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{}));

            document->undoCommand();
            CHECK_THAT(document->selectedBrushFaces(), Catch::Equals(std::vector<Model::BrushFaceHandle>{{ brushNode, *topFaceIndex }}));
        }
    }
}
