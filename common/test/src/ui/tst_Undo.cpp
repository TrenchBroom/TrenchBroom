/*
 Copyright (C) 2010 Kristian Duske

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

#include "TestUtils.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Material.h"
#include "mdl/MaterialManager.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"

#include <cassert>

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "UndoTest.setMaterialsAfterRestore")
{
  document->deselectAll();
  document->setProperty(mdl::EntityPropertyKeys::Wad, "fixture/test/io/Wad/cr8_czg.wad");

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
    document->translate(vm::vec3d(1, 1, 1));
    CHECK(material->usageCount() == 6u);

    document->undoCommand();
    CHECK(material->usageCount() == 6u);
  }

  SECTION("delete brush")
  {
    document->selectNodes({brushNode});
    document->remove();
    CHECK(material->usageCount() == 0u);

    document->undoCommand();
    CHECK(material->usageCount() == 6u);
  }

  SECTION("select top face, translate UV")
  {
    auto topFaceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    REQUIRE(topFaceIndex.has_value());

    document->selectBrushFaces({{brushNode, *topFaceIndex}});

    auto request = mdl::ChangeBrushFaceAttributesRequest{};
    request.setXOffset(12.34f);
    REQUIRE(document->setFaceAttributes(request));

    document->undoCommand(); // undo move
    CHECK(material->usageCount() == 6u);
    REQUIRE(document->selection().hasBrushFaces());

    document->undoCommand(); // undo select
    CHECK(material->usageCount() == 6u);
    REQUIRE(!document->selection().hasBrushFaces());
  }

  for (const auto& face : brushNode->brush().faces())
  {
    CHECK(face.material() == material);
  }
}

TEST_CASE_METHOD(MapDocumentTest, "UndoTest.undoRotation")
{
  auto* entityNode = new mdl::EntityNode{mdl::Entity{{
    {mdl::EntityPropertyKeys::Classname, "test"},
  }}};

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  CHECK(!entityNode->entity().hasProperty("angle"));

  document->selectNodes({entityNode});
  document->rotate(vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::to_radians(15.0));
  CHECK(entityNode->entity().hasProperty("angle"));
  CHECK(*entityNode->entity().property("angle") == "15");

  document->undoCommand();
  CHECK(!entityNode->entity().hasProperty("angle"));
}

} // namespace tb::ui
