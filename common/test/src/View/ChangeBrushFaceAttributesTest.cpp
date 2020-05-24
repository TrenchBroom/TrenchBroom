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

#include "TestUtils.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocumentTest.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        class ChangeBrushFaceAttributesTest : public MapDocumentTest {
        public:
            ChangeBrushFaceAttributesTest() :
            MapDocumentTest(Model::MapFormat::Valve) {}
        };

        TEST_CASE_METHOD(ChangeBrushFaceAttributesTest, "ChangeBrushFaceAttributesTest.resetAttributesOfValve220Face") {
            Model::BrushNode* brushNode = createBrushNode();
            document->addNode(brushNode, document->currentParent());

            const size_t faceIndex = 0u;
            const vm::vec3 initialX = brushNode->brush().face(faceIndex).textureXAxis();
            const vm::vec3 initialY = brushNode->brush().face(faceIndex).textureYAxis();

            document->select(Model::BrushFaceHandle(brushNode, faceIndex));

            Model::ChangeBrushFaceAttributesRequest rotate;
            rotate.addRotation(2.0);
            for (size_t i = 0; i < 5; ++i) {
                document->setFaceAttributes(rotate);
            }

            CHECK(brushNode->brush().face(faceIndex).attributes().rotation() == 10.0f);

            Model::ChangeBrushFaceAttributesRequest reset;
            reset.resetAll();

            document->setFaceAttributes(reset);

            CHECK(brushNode->brush().face(faceIndex).attributes().xOffset() == 0.0f);
            CHECK(brushNode->brush().face(faceIndex).attributes().yOffset() == 0.0f);
            CHECK(brushNode->brush().face(faceIndex).attributes().rotation() == 0.0f);
            CHECK(brushNode->brush().face(faceIndex).attributes().xScale() == 1.0f);
            CHECK(brushNode->brush().face(faceIndex).attributes().yScale() == 1.0f);

            CHECK(brushNode->brush().face(faceIndex).textureXAxis() == initialX);
            CHECK(brushNode->brush().face(faceIndex).textureYAxis() == initialY);
        }
        
        TEST_CASE_METHOD(ChangeBrushFaceAttributesTest, "ChangeBrushFaceAttributesTest.undoRedo") {
            Model::BrushNode* brushNode = createBrushNode("original");
            document->addNode(brushNode, document->currentParent());
            
            const auto requireTexture = [&](const std::string& textureName) {
                for (const auto& face : brushNode->brush().faces()) {
                    REQUIRE(face.attributes().textureName() == textureName);
                }
            };
            
            const auto checkTexture = [&](const std::string& textureName) {
                for (const auto& face : brushNode->brush().faces()) {
                    CHECK(face.attributes().textureName() == textureName);
                }
            };
            
            requireTexture("original");
            
            document->select(brushNode);
            
            Model::ChangeBrushFaceAttributesRequest setTexture1;
            setTexture1.setTextureName("texture1");
            document->setFaceAttributes(setTexture1);
            requireTexture("texture1");
            
            Model::ChangeBrushFaceAttributesRequest setTexture2;
            setTexture2.setTextureName("texture2");
            document->setFaceAttributes(setTexture2);
            requireTexture("texture2");
            
            document->undoCommand();
            checkTexture("original");
            
            document->redoCommand();
            checkTexture("texture2");
        }
    }
}
