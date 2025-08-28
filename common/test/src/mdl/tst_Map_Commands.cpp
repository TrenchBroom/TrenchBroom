/*
 Copyright (C) 2025 Kristian Duske

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

#include "Exceptions.h"
#include "MapFixture.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushNode.h"
#include "mdl/ChangeBrushFaceAttributesRequest.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/MaterialManager.h"
#include "mdl/TransactionScope.h"

#include "vm/approx.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Commands")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  SECTION("undoCommand")
  {
    SECTION("Undoing a rotation removes angle key")
    {
      auto* entityNode = new EntityNode{Entity{{
        {EntityPropertyKeys::Classname, "test"},
      }}};

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      CHECK(!entityNode->entity().hasProperty("angle"));

      map.selectNodes({entityNode});
      map.rotateSelection(vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::to_radians(15.0));
      CHECK(entityNode->entity().hasProperty("angle"));
      CHECK(*entityNode->entity().property("angle") == "15");

      map.undoCommand();
      CHECK(!entityNode->entity().hasProperty("angle"));
    }

    SECTION("Update materials")
    {
      map.deselectAll();
      map.setEntityProperty(EntityPropertyKeys::Wad, "fixture/test/io/Wad/cr8_czg.wad");

      auto* brushNode = createBrushNode(map, "coffin1");
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      const auto* material = map.materialManager().material("coffin1");
      CHECK(material != nullptr);
      CHECK(material->usageCount() == 6u);

      for (const auto& face : brushNode->brush().faces())
      {
        CHECK(face.material() == material);
      }

      SECTION("translateSelection")
      {
        map.selectNodes({brushNode});
        map.translateSelection(vm::vec3d{1, 1, 1});
        CHECK(material->usageCount() == 6u);

        map.undoCommand();
        CHECK(material->usageCount() == 6u);
      }

      SECTION("removeSelectedNodes")
      {
        map.selectNodes({brushNode});
        removeSelectedNodes(map);
        CHECK(material->usageCount() == 0u);

        map.undoCommand();
        CHECK(material->usageCount() == 6u);
      }

      SECTION("translateUV")
      {
        auto topFaceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
        REQUIRE(topFaceIndex.has_value());

        map.selectBrushFaces({{brushNode, *topFaceIndex}});

        auto request = ChangeBrushFaceAttributesRequest{};
        request.setXOffset(12.34f);
        REQUIRE(map.setFaceAttributes(request));

        map.undoCommand(); // undo move
        CHECK(material->usageCount() == 6u);
        REQUIRE(map.selection().hasBrushFaces());

        map.undoCommand(); // undo select
        CHECK(material->usageCount() == 6u);
        REQUIRE(!map.selection().hasBrushFaces());
      }

      for (const auto& face : brushNode->brush().faces())
      {
        CHECK(face.material() == material);
      }
    }
  }

  SECTION("canRepeatCommands")
  {
    CHECK_FALSE(map.canRepeatCommands());

    auto* entityNode = new EntityNode{Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode}}});
    CHECK_FALSE(map.canRepeatCommands());

    map.selectNodes({entityNode});
    CHECK_FALSE(map.canRepeatCommands());

    duplicateSelectedNodes(map);
    CHECK(map.canRepeatCommands());

    map.clearRepeatableCommands();
    CHECK_FALSE(map.canRepeatCommands());
  }

  SECTION("repeatCommands")
  {
    SECTION("Repeat translation")
    {
      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      map.selectNodes({entityNode});

      REQUIRE_FALSE(map.canRepeatCommands());
      map.translateSelection({1, 2, 3});
      CHECK(map.canRepeatCommands());

      REQUIRE(entityNode->entity().origin() == vm::vec3d(1, 2, 3));
      map.repeatCommands();
      CHECK(entityNode->entity().origin() == vm::vec3d(2, 4, 6));
    }

    SECTION("Repeat rotation")
    {
      auto entity = Entity();
      entity.transform(vm::translation_matrix(vm::vec3d(1, 2, 3)), true);

      auto* entityNode = new EntityNode(std::move(entity));

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      map.selectNodes({entityNode});

      REQUIRE_FALSE(map.canRepeatCommands());
      map.rotateSelection(vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::to_radians(90.0));
      CHECK(map.canRepeatCommands());

      REQUIRE(
        entityNode->entity().origin()
        == vm::approx(
          vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(90.0))
          * vm::vec3d(1, 2, 3)));
      map.repeatCommands();
      CHECK(
        entityNode->entity().origin()
        == vm::approx(
          vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(180.0))
          * vm::vec3d(1, 2, 3)));
    }

    SECTION("Scale with bounding box")
    {
      auto* brushNode1 = createBrushNode(map);

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      map.selectNodes({brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      const auto oldBounds = brushNode1->logicalBounds();
      const auto newBounds = vm::bbox3d(oldBounds.min, 2.0 * oldBounds.max);
      map.scaleSelection(oldBounds, newBounds);
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      map.selectNodes({brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == newBounds);
    }

    SECTION("Scale with factors")
    {
      auto* brushNode1 = createBrushNode(map);

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      map.selectNodes({brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      map.scaleSelection(brushNode1->logicalBounds().center(), vm::vec3d(2, 2, 2));
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      map.deselectAll();
      map.selectNodes({brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
    }

    SECTION("Shear")
    {
      auto* brushNode1 = createBrushNode(map);
      const auto originalBounds = brushNode1->logicalBounds();

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      map.selectNodes({brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      map.shearSelection(originalBounds, vm::vec3d{0, 0, 1}, vm::vec3d(32, 0, 0));
      REQUIRE(brushNode1->logicalBounds() != originalBounds);
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      map.deselectAll();
      map.selectNodes({brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
    }

    SECTION("Flip")
    {
      auto* brushNode1 = createBrushNode(map);
      const auto originalBounds = brushNode1->logicalBounds();

      addNodes(map, {{parentForNodes(map), {brushNode1}}});
      map.selectNodes({brushNode1});

      REQUIRE_FALSE(map.canRepeatCommands());
      map.flipSelection(originalBounds.max, vm::axis::z);
      REQUIRE(brushNode1->logicalBounds() != originalBounds);
      CHECK(map.canRepeatCommands());

      auto* brushNode2 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode2}}});
      map.deselectAll();
      map.selectNodes({brushNode2});

      map.repeatCommands();
      CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
    }

    SECTION("Duplicate and translate")
    {
      auto* entityNode1 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      map.selectNodes({entityNode1});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      SECTION("transaction containing a rollback")
      {
        duplicateSelectedNodes(map);

        map.startTransaction("", TransactionScope::Oneshot);
        map.translateSelection({0, 0, 10});
        map.rollbackTransaction();
        map.translateSelection({10, 0, 0});
        map.commitTransaction();
      }
      SECTION("translations that get coalesced")
      {
        duplicateSelectedNodes(map);

        map.translateSelection({5, 0, 0});
        map.translateSelection({5, 0, 0});
      }
      SECTION("duplicate inside transaction, then standalone movements")
      {
        map.startTransaction("", TransactionScope::Oneshot);
        duplicateSelectedNodes(map);
        map.translateSelection({2, 0, 0});
        map.translateSelection({2, 0, 0});
        map.commitTransaction();

        map.translateSelection({2, 0, 0});
        map.translateSelection({2, 0, 0});
        map.translateSelection({2, 0, 0});
      }

      // repeatable actions:
      //  - duplicate
      //  - translate by x = +10

      REQUIRE(map.selection().allEntities().size() == 1);

      auto* entityNode2 = map.selection().allEntities().at(0);
      CHECK(entityNode2 != entityNode1);

      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));
      CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));

      map.repeatCommands();

      REQUIRE(map.selection().allEntities().size() == 1);

      auto* entityNode3 = map.selection().allEntities().at(0);
      CHECK(entityNode3 != entityNode2);

      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));
      CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));
      CHECK(entityNode3->entity().origin() == vm::vec3d(20, 0, 0));
    }

    SECTION("Repeat applies to transactions")
    {
      auto* entityNode1 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      map.selectNodes({entityNode1});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      map.startTransaction("", TransactionScope::Oneshot);
      map.translateSelection({0, 0, 10});
      map.rollbackTransaction();
      map.translateSelection({10, 0, 0});
      map.commitTransaction();
      // overall result: x += 10

      CHECK(entityNode1->entity().origin() == vm::vec3d(10, 0, 0));

      // now repeat the transaction on a second entity

      auto* entityNode2 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode2}}});

      map.deselectAll();
      map.selectNodes({entityNode2});
      CHECK(entityNode2->entity().origin() == vm::vec3d(0, 0, 0));

      CHECK(map.canRepeatCommands());
      map.repeatCommands();
      CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));

      map.repeatCommands();
      CHECK(entityNode2->entity().origin() == vm::vec3d(20, 0, 0));

      // ensure entityNode1 was unmodified

      CHECK(entityNode1->entity().origin() == vm::vec3d(10, 0, 0));
    }

    SECTION("Undo")
    {
      auto* entityNode1 = new EntityNode({});
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      map.selectNodes({entityNode1});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      map.translateSelection({0, 0, 10});
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 10));
      CHECK(map.canRepeatCommands());

      map.undoCommand();
      CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

      // For now, we won't support repeating a sequence of commands
      // containing undo/redo (it just clears the repeat stack)
      CHECK(!map.canRepeatCommands());
    }
  }

  SECTION("throwExceptionDuringCommand")
  {
    CHECK_THROWS_AS(map.throwExceptionDuringCommand(), CommandProcessorException);
  }
}

} // namespace tb::mdl
