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

#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/EntityNode.h"
#include "Model/Game.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/TestGame.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include "TestUtils.h"

#include <filesystem>

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(
  ValveMapDocumentTest, "ChangeBrushFaceAttributesTest.resetAttributesOfValve220Face")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const size_t faceIndex = 0u;
  const auto initialX = brushNode->brush().face(faceIndex).textureXAxis();
  const auto initialY = brushNode->brush().face(faceIndex).textureYAxis();

  document->selectBrushFaces({{brushNode, faceIndex}});

  auto rotate = Model::ChangeBrushFaceAttributesRequest{};
  rotate.addRotation(2.0);
  for (size_t i = 0; i < 5; ++i)
  {
    document->setFaceAttributes(rotate);
  }

  CHECK(brushNode->brush().face(faceIndex).attributes().rotation() == 10.0f);

  auto defaultFaceAttrs =
    Model::BrushFaceAttributes{Model::BrushFaceAttributes::NoTextureName};
  defaultFaceAttrs.setXScale(0.5f);
  defaultFaceAttrs.setYScale(2.0f);
  game->setDefaultFaceAttributes(defaultFaceAttrs);

  auto reset = Model::ChangeBrushFaceAttributesRequest{};
  reset.resetAll(defaultFaceAttrs);

  document->setFaceAttributes(reset);

  CHECK(brushNode->brush().face(faceIndex).attributes().xOffset() == 0.0f);
  CHECK(brushNode->brush().face(faceIndex).attributes().yOffset() == 0.0f);
  CHECK(brushNode->brush().face(faceIndex).attributes().rotation() == 0.0f);
  CHECK(
    brushNode->brush().face(faceIndex).attributes().xScale()
    == defaultFaceAttrs.xScale());
  CHECK(
    brushNode->brush().face(faceIndex).attributes().yScale()
    == defaultFaceAttrs.yScale());

  CHECK(brushNode->brush().face(faceIndex).textureXAxis() == initialX);
  CHECK(brushNode->brush().face(faceIndex).textureYAxis() == initialY);
}

TEST_CASE_METHOD(ValveMapDocumentTest, "ChangeBrushFaceAttributesTest.undoRedo")
{
  auto* brushNode = createBrushNode("original");
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  for (const auto& face : brushNode->brush().faces())
  {
    REQUIRE(face.attributes().textureName() == "original");
  }

  document->selectNodes({brushNode});

  auto setTexture1 = Model::ChangeBrushFaceAttributesRequest{};
  setTexture1.setTextureName("texture1");
  document->setFaceAttributes(setTexture1);
  for (const auto& face : brushNode->brush().faces())
  {
    REQUIRE(face.attributes().textureName() == "texture1");
  }

  auto setTexture2 = Model::ChangeBrushFaceAttributesRequest{};
  setTexture2.setTextureName("texture2");
  document->setFaceAttributes(setTexture2);
  for (const auto& face : brushNode->brush().faces())
  {
    REQUIRE(face.attributes().textureName() == "texture2");
  }

  document->undoCommand();
  for (const auto& face : brushNode->brush().faces())
  {
    CHECK(face.attributes().textureName() == "original");
  }

  document->redoCommand();
  for (const auto& face : brushNode->brush().faces())
  {
    CHECK(face.attributes().textureName() == "texture2");
  }
}

TEST_CASE_METHOD(ValveMapDocumentTest, "ChangeBrushFaceAttributesTest.setAll")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const size_t firstFaceIndex = 0u;
  const size_t secondFaceIndex = 1u;
  const size_t thirdFaceIndex = 2u;

  document->deselectAll();
  document->selectBrushFaces({{brushNode, firstFaceIndex}});

  auto setFirstFace = Model::ChangeBrushFaceAttributesRequest{};
  setFirstFace.setTextureName("first");
  setFirstFace.setXOffset(32.0f);
  setFirstFace.setYOffset(64.0f);
  setFirstFace.setRotation(90.0f);
  setFirstFace.setXScale(2.0f);
  setFirstFace.setYScale(4.0f);
  setFirstFace.replaceSurfaceFlags(63u);
  setFirstFace.replaceContentFlags(12u);
  setFirstFace.setSurfaceValue(3.14f);
  setFirstFace.setColor(Color{1.0f, 1.0f, 1.0f, 1.0f});
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
    CHECK(firstAttrs.color() == Color{1.0f, 1.0f, 1.0f, 1.0f});
  }

  document->deselectAll();
  document->selectBrushFaces({{brushNode, secondFaceIndex}});

  auto setSecondFace = Model::ChangeBrushFaceAttributesRequest{};
  setSecondFace.setTextureName("second");
  setSecondFace.setXOffset(16.0f);
  setSecondFace.setYOffset(48.0f);
  setSecondFace.setRotation(45.0f);
  setSecondFace.setXScale(1.0f);
  setSecondFace.setYScale(1.0f);
  setSecondFace.replaceSurfaceFlags(18u);
  setSecondFace.replaceContentFlags(2048u);
  setSecondFace.setSurfaceValue(1.0f);
  setSecondFace.setColor(Color{0.5f, 0.5f, 0.5f, 0.5f});
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
    CHECK(secondAttrs.color() == Color{0.5f, 0.5f, 0.5f, 0.5f});
  }

  document->deselectAll();
  document->selectBrushFaces({{brushNode, thirdFaceIndex}});

  auto copySecondToThirdFace = Model::ChangeBrushFaceAttributesRequest{};
  copySecondToThirdFace.setAll(brushNode->brush().face(secondFaceIndex));
  document->setFaceAttributes(copySecondToThirdFace);

  CHECK(
    brushNode->brush().face(thirdFaceIndex).attributes()
    == brushNode->brush().face(secondFaceIndex).attributes());

  auto thirdFaceContentsFlags =
    brushNode->brush().face(thirdFaceIndex).attributes().surfaceContents();

  document->deselectAll();
  document->selectBrushFaces({{brushNode, secondFaceIndex}});

  auto copyFirstToSecondFace = Model::ChangeBrushFaceAttributesRequest{};
  copyFirstToSecondFace.setAll(brushNode->brush().face(firstFaceIndex));
  document->setFaceAttributes(copyFirstToSecondFace);

  CHECK(
    brushNode->brush().face(secondFaceIndex).attributes()
    == brushNode->brush().face(firstFaceIndex).attributes());

  document->deselectAll();
  document->selectBrushFaces({{brushNode, thirdFaceIndex}});
  Model::ChangeBrushFaceAttributesRequest copyFirstToThirdFaceNoContents;
  copyFirstToThirdFaceNoContents.setAllExceptContentFlags(
    brushNode->brush().face(firstFaceIndex));
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

TEST_CASE_METHOD(
  ValveMapDocumentTest, "ChangeBrushFaceAttributesTest.setTextureKeepsSurfaceFlagsUnset")
{
  auto* brushNode = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  document->selectNodes({brushNode});
  CHECK(!brushNode->brush().face(0).attributes().hasSurfaceAttributes());

  auto request = Model::ChangeBrushFaceAttributesRequest{};
  request.setTextureName("something_else");
  document->setFaceAttributes(request);

  CHECK(brushNode->brush().face(0).attributes().textureName() == "something_else");
  CHECK(!brushNode->brush().face(0).attributes().hasSurfaceAttributes());
}

TEST_CASE("ChangeBrushFaceAttributesTest.Quake2IntegrationTest")
{
  const int WaterFlag = 32;
  const int LavaFlag = 8;

  auto [document, game, gameConfig] = View::loadMapDocument(
    "fixture/test/View/ChangeBrushFaceAttributesTest/lavaAndWater.map",
    "Quake2",
    Model::MapFormat::Unknown);
  REQUIRE(document->currentLayer() != nullptr);

  auto* lavabrush =
    dynamic_cast<Model::BrushNode*>(document->currentLayer()->children().at(0));
  REQUIRE(lavabrush);
  CHECK(!lavabrush->brush().face(0).attributes().hasSurfaceAttributes());
  CHECK(
    lavabrush->brush().face(0).resolvedSurfaceContents()
    == LavaFlag); // comes from the .wal texture

  auto* waterbrush =
    dynamic_cast<Model::BrushNode*>(document->currentLayer()->children().at(1));
  REQUIRE(waterbrush);
  CHECK(!waterbrush->brush().face(0).attributes().hasSurfaceAttributes());
  CHECK(
    waterbrush->brush().face(0).resolvedSurfaceContents()
    == WaterFlag); // comes from the .wal texture

  SECTION("transfer face attributes except content flags from waterbrush to lavabrush")
  {
    document->selectNodes({lavabrush});
    CHECK(document->setFaceAttributesExceptContentFlags(
      waterbrush->brush().face(0).attributes()));

    SECTION("check lavabrush is now inheriting the water content flags")
    {
      // Note: the contents flag wasn't transferred, but because lavabrushes's
      // content flag was "Inherit", it stays "Inherit" and now inherits the water
      // contents
      CHECK(!lavabrush->brush().face(0).attributes().hasSurfaceAttributes());
      CHECK(lavabrush->brush().face(0).resolvedSurfaceContents() == WaterFlag);
      CHECK(lavabrush->brush().face(0).attributes().textureName() == "watertest");
    }
  }

  SECTION(
    "setting a content flag when the existing one is inherited keeps the existing one")
  {
    document->selectNodes({lavabrush});

    auto request = Model::ChangeBrushFaceAttributesRequest{};
    request.setContentFlags(WaterFlag);
    CHECK(document->setFaceAttributes(request));

    CHECK(lavabrush->brush().face(0).attributes().hasSurfaceAttributes());
    CHECK(lavabrush->brush().face(0).resolvedSurfaceContents() == (WaterFlag | LavaFlag));
  }
}
} // namespace View
} // namespace TrenchBroom
