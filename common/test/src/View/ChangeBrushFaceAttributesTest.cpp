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
            document->addNode(brushNode, document->parentForNodes());

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
            document->addNode(brushNode, document->parentForNodes());
            
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

        TEST_CASE_METHOD(ChangeBrushFaceAttributesTest, "ChangeBrushFaceAttributesTest.setAll") {
            Model::BrushNode* brushNode = createBrushNode();
            document->addNode(brushNode, document->parentForNodes());

            const size_t firstFaceIndex = 0u;
            const size_t secondFaceIndex = 1u;
            const size_t thirdFaceIndex = 2u;

            document->deselectAll();
            document->select(Model::BrushFaceHandle(brushNode, firstFaceIndex));
            Model::ChangeBrushFaceAttributesRequest setFirstFace;
            setFirstFace.setTextureName("first");
            setFirstFace.setXOffset(32.0f);
            setFirstFace.setYOffset(64.0f);
            setFirstFace.setRotation(90.0f);
            setFirstFace.setXScale(2.0f);
            setFirstFace.setYScale(4.0f);
            setFirstFace.replaceSurfaceFlags(63u);
            setFirstFace.replaceContentFlags(12u);
            setFirstFace.setSurfaceValue(3.14f);
            const Color firstColor(1.0f, 1.0f, 1.0f, 1.0f);
            setFirstFace.setColor(firstColor);
            document->setFaceAttributes(setFirstFace);

            {
                const auto& firstAttrs = brushNode->brush().face(firstFaceIndex).attributes();
                CHECK(firstAttrs.textureName() == "first");
                CHECK(firstAttrs.xOffset() == 32.0f);
                CHECK(firstAttrs.yOffset() == 64.0f);
                CHECK(firstAttrs.rotation() == 90.0f);
                CHECK(firstAttrs.xScale() == 2.0f);
                CHECK(firstAttrs.yScale() == 4.0f);
                CHECK(firstAttrs.surfaceFlags() == 63u);
                CHECK(firstAttrs.surfaceContents() == 12u);
                CHECK(firstAttrs.surfaceValue() == 3.14f);
                CHECK(firstAttrs.color() == firstColor);
            }

            document->deselectAll();
            document->select(Model::BrushFaceHandle(brushNode, secondFaceIndex));
            Model::ChangeBrushFaceAttributesRequest setSecondFace;
            setSecondFace.setTextureName("second");
            setSecondFace.setXOffset(16.0f);
            setSecondFace.setYOffset(48.0f);
            setSecondFace.setRotation(45.0f);
            setSecondFace.setXScale(1.0f);
            setSecondFace.setYScale(1.0f);
            setSecondFace.replaceSurfaceFlags(18u);
            setSecondFace.replaceContentFlags(2048u);
            setSecondFace.setSurfaceValue(1.0f);
            const Color secondColor(0.5f, 0.5f, 0.5f, 0.5f);
            setSecondFace.setColor(secondColor);
            document->setFaceAttributes(setSecondFace);

            {
                const auto& secondAttrs = brushNode->brush().face(secondFaceIndex).attributes();
                CHECK(secondAttrs.textureName() == "second");
                CHECK(secondAttrs.xOffset() == 16.0f);
                CHECK(secondAttrs.yOffset() == 48.0f);
                CHECK(secondAttrs.rotation() == 45.0f);
                CHECK(secondAttrs.xScale() == 1.0f);
                CHECK(secondAttrs.yScale() == 1.0f);
                CHECK(secondAttrs.surfaceFlags() == 18u);
                CHECK(secondAttrs.surfaceContents() == 2048u);
                CHECK(secondAttrs.surfaceValue() == 1.0f);
                CHECK(secondAttrs.color() == secondColor);
            }

            document->deselectAll();
            document->select(Model::BrushFaceHandle(brushNode, thirdFaceIndex));
            Model::ChangeBrushFaceAttributesRequest copySecondToThirdFace;
            copySecondToThirdFace.setAll(brushNode->brush().face(secondFaceIndex));
            document->setFaceAttributes(copySecondToThirdFace);

            {
                const auto& secondAttrs = brushNode->brush().face(secondFaceIndex).attributes();
                const auto& thirdAttrs = brushNode->brush().face(thirdFaceIndex).attributes();
                CHECK(thirdAttrs == secondAttrs);
            }

            auto thirdFaceContentsFlags = brushNode->brush().face(thirdFaceIndex).attributes().surfaceContents();

            document->deselectAll();
            document->select(Model::BrushFaceHandle(brushNode, secondFaceIndex));
            Model::ChangeBrushFaceAttributesRequest copyFirstToSecondFace;
            copyFirstToSecondFace.setAll(brushNode->brush().face(firstFaceIndex));
            document->setFaceAttributes(copyFirstToSecondFace);

            {
                const auto& firstAttrs = brushNode->brush().face(firstFaceIndex).attributes();
                const auto& newSecondAttrs = brushNode->brush().face(secondFaceIndex).attributes();
                CHECK(newSecondAttrs == firstAttrs);
            }

            document->deselectAll();
            document->select(Model::BrushFaceHandle(brushNode, thirdFaceIndex));
            Model::ChangeBrushFaceAttributesRequest copyFirstToThirdFaceNoContents;
            copyFirstToThirdFaceNoContents.setAllExceptContentFlags(brushNode->brush().face(firstFaceIndex));
            document->setFaceAttributes(copyFirstToThirdFaceNoContents);

            {
                const auto& firstAttrs = brushNode->brush().face(firstFaceIndex).attributes();
                const auto& newThirdAttrs = brushNode->brush().face(thirdFaceIndex).attributes();
                CHECK(newThirdAttrs.textureName() == firstAttrs.textureName());
                CHECK(newThirdAttrs.xOffset() == firstAttrs.xOffset());
                CHECK(newThirdAttrs.yOffset() == firstAttrs.yOffset());
                CHECK(newThirdAttrs.rotation() == firstAttrs.rotation());
                CHECK(newThirdAttrs.xScale() == firstAttrs.xScale());
                CHECK(newThirdAttrs.yScale() == firstAttrs.yScale());
                CHECK(newThirdAttrs.surfaceFlags() == firstAttrs.surfaceFlags());
                CHECK(newThirdAttrs.surfaceContents() == thirdFaceContentsFlags);
                CHECK(newThirdAttrs.surfaceValue() == firstAttrs.surfaceValue());
                CHECK(newThirdAttrs.color() == firstAttrs.color());
            }
        }
    }
}
