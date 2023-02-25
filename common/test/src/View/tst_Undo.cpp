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

#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <cassert>

#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom
{
namespace View
{
TEST_CASE_METHOD(MapDocumentTest, "UndoTest.setTexturesAfterRestore")
{
  document->deselectAll();
  document->setProperty(
    Model::EntityPropertyKeys::Wad, "fixture/test/IO/Wad/cr8_czg.wad");

  Model::BrushNode* brushNode = createBrushNode("coffin1");
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const Assets::Texture* texture = document->textureManager().texture("coffin1");
  CHECK(texture != nullptr);
  CHECK(texture->usageCount() == 6u);

  for (const Model::BrushFace& face : brushNode->brush().faces())
  {
    CHECK(face.texture() == texture);
  }

  SECTION("translate brush")
  {
    document->selectNodes({brushNode});
    document->translateObjects(vm::vec3(1, 1, 1));
    CHECK(texture->usageCount() == 6u);

    document->undoCommand();
    CHECK(texture->usageCount() == 6u);
  }

  SECTION("delete brush")
  {
    document->selectNodes({brushNode});
    document->deleteObjects();
    CHECK(texture->usageCount() == 0u);

    document->undoCommand();
    CHECK(texture->usageCount() == 6u);
  }

  SECTION("select top face, move texture")
  {
    auto topFaceIndex = brushNode->brush().findFace(vm::vec3::pos_z());
    REQUIRE(topFaceIndex.has_value());

    document->selectBrushFaces({{brushNode, *topFaceIndex}});

    Model::ChangeBrushFaceAttributesRequest request;
    request.setXOffset(static_cast<float>(12.34f));
    REQUIRE(document->setFaceAttributes(request));

    document->undoCommand(); // undo move
    CHECK(texture->usageCount() == 6u);
    REQUIRE(document->hasSelectedBrushFaces());

    document->undoCommand(); // undo select
    CHECK(texture->usageCount() == 6u);
    REQUIRE(!document->hasSelectedBrushFaces());
  }

  for (const Model::BrushFace& face : brushNode->brush().faces())
  {
    CHECK(face.texture() == texture);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "UndoTest.undoRotation")
{
  auto* entityNode =
    new Model::EntityNode{{}, {{Model::EntityPropertyKeys::Classname, "test"}}};

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  CHECK(!entityNode->entity().hasProperty("angle"));

  document->selectNodes({entityNode});
  document->rotateObjects(vm::vec3::zero(), vm::vec3::pos_z(), vm::to_radians(15.0));
  CHECK(entityNode->entity().hasProperty("angle"));
  CHECK(*entityNode->entity().property("angle") == "15");

  document->undoCommand();
  CHECK(!entityNode->entity().hasProperty("angle"));
}
} // namespace View
} // namespace TrenchBroom
