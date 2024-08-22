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

#include "Assets/Material.h"
#include "Assets/MaterialCollection.h"
#include "Assets/MaterialManager.h"
#include "Model/BrushFace.h"
#include "Model/BrushNode.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "TestUtils.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <cassert>

#include "Catch2.h"

namespace TrenchBroom::View
{

TEST_CASE_METHOD(MapDocumentTest, "UndoTest.setMaterialsAfterRestore")
{
  document->deselectAll();
  document->setProperty(
    Model::EntityPropertyKeys::Wad, "fixture/test/IO/Wad/cr8_czg.wad");

  auto* brushNode = createBrushNode("coffin1");
  document->addNodes({{document->parentForNodes(), {brushNode}}});

  const auto* material = document->materialManager().material("coffin1");
  CHECK(material != nullptr);
  CHECK(material->usageCount() == 6u);

  for (const auto& face : brushNode->brush().faces())
  {
    CHECK(face.material() == material);
  }

  SECTION("translate brush")
  {
    document->selectNodes({brushNode});
    document->translateObjects(vm::vec3(1, 1, 1));
    CHECK(material->usageCount() == 6u);

    document->undoCommand();
    CHECK(material->usageCount() == 6u);
  }

  SECTION("delete brush")
  {
    document->selectNodes({brushNode});
    document->deleteObjects();
    CHECK(material->usageCount() == 0u);

    document->undoCommand();
    CHECK(material->usageCount() == 6u);
  }

  SECTION("select top face, translate UV")
  {
    auto topFaceIndex = brushNode->brush().findFace(vm::vec3::pos_z());
    REQUIRE(topFaceIndex.has_value());

    document->selectBrushFaces({{brushNode, *topFaceIndex}});

    auto request = Model::ChangeBrushFaceAttributesRequest{};
    request.setXOffset(12.34f);
    REQUIRE(document->setFaceAttributes(request));

    document->undoCommand(); // undo move
    CHECK(material->usageCount() == 6u);
    REQUIRE(document->hasSelectedBrushFaces());

    document->undoCommand(); // undo select
    CHECK(material->usageCount() == 6u);
    REQUIRE(!document->hasSelectedBrushFaces());
  }

  for (const auto& face : brushNode->brush().faces())
  {
    CHECK(face.material() == material);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "UndoTest.undoRotation")
{
  auto* entityNode = new Model::EntityNode{Model::Entity{{
    {Model::EntityPropertyKeys::Classname, "test"},
  }}};

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  CHECK(!entityNode->entity().hasProperty("angle"));

  document->selectNodes({entityNode});
  document->rotateObjects(vm::vec3::zero(), vm::vec3::pos_z(), vm::to_radians(15.0));
  CHECK(entityNode->entity().hasProperty("angle"));
  CHECK(*entityNode->entity().property("angle") == "15");

  document->undoCommand();
  CHECK(!entityNode->entity().hasProperty("angle"));
}

} // namespace TrenchBroom::View
