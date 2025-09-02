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
#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentTest.h"
#include "ui/TransactionScope.h"

#include "vm/approx.h"
#include "vm/mat.h"
#include "vm/mat_ext.h"
#include "vm/scalar.h"
#include "vm/vec.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.canRepeat")
{
  CHECK_FALSE(document->canRepeatCommands());

  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  CHECK_FALSE(document->canRepeatCommands());

  document->selectNodes({entityNode});
  CHECK_FALSE(document->canRepeatCommands());

  document->duplicate();
  CHECK(document->canRepeatCommands());

  document->clearRepeatableCommands();
  CHECK_FALSE(document->canRepeatCommands());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatTranslate")
{
  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->selectNodes({entityNode});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->translate(vm::vec3d(1, 2, 3));
  CHECK(document->canRepeatCommands());

  REQUIRE(entityNode->entity().origin() == vm::vec3d(1, 2, 3));
  document->repeatCommands();
  CHECK(entityNode->entity().origin() == vm::vec3d(2, 4, 6));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatRotate")
{
  auto entity = mdl::Entity();
  entity.transform(vm::translation_matrix(vm::vec3d(1, 2, 3)), true);

  auto* entityNode = new mdl::EntityNode(std::move(entity));

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->selectNodes({entityNode});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->rotate(vm::vec3d{0, 0, 0}, vm::vec3d{0, 0, 1}, vm::to_radians(90.0));
  CHECK(document->canRepeatCommands());

  REQUIRE(
    entityNode->entity().origin()
    == vm::approx(
      vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(90.0))
      * vm::vec3d(1, 2, 3)));
  document->repeatCommands();
  CHECK(
    entityNode->entity().origin()
    == vm::approx(
      vm::rotation_matrix(vm::vec3d{0, 0, 1}, vm::to_radians(180.0))
      * vm::vec3d(1, 2, 3)));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatScaleWithBBox")
{
  auto* brushNode1 = createBrushNode();

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  const auto oldBounds = brushNode1->logicalBounds();
  const auto newBounds = vm::bbox3d(oldBounds.min, 2.0 * oldBounds.max);
  document->scale(oldBounds, newBounds);
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == newBounds);
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatScaleWithFactors")
{
  auto* brushNode1 = createBrushNode();

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->scale(brushNode1->logicalBounds().center(), vm::vec3d(2, 2, 2));
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  document->deselectAll();
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.shear")
{
  auto* brushNode1 = createBrushNode();
  const auto originalBounds = brushNode1->logicalBounds();

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->shear(originalBounds, vm::vec3d{0, 0, 1}, vm::vec3d(32, 0, 0));
  REQUIRE(brushNode1->logicalBounds() != originalBounds);
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  document->deselectAll();
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.flip")
{
  auto* brushNode1 = createBrushNode();
  const auto originalBounds = brushNode1->logicalBounds();

  document->addNodes({{document->parentForNodes(), {brushNode1}}});
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->flip(originalBounds.max, vm::axis::z);
  REQUIRE(brushNode1->logicalBounds() != originalBounds);
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  document->addNodes({{document->parentForNodes(), {brushNode2}}});
  document->deselectAll();
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.selectionClears")
{
  auto* entityNode1 = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode1}}});

  auto* entityNode2 = new mdl::EntityNode{mdl::Entity{}};
  document->addNodes({{document->parentForNodes(), {entityNode2}}});

  document->selectNodes({entityNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->translate(vm::vec3d(1, 2, 3));
  REQUIRE(document->canRepeatCommands());

  document->deselectAll();
  document->selectNodes({entityNode2});
  CHECK(document->canRepeatCommands());

  // this command will not clear the repeat stack
  document->setProperty("this", "that");
  CHECK(document->canRepeatCommands());

  // this command will replace the command on the repeat stack
  document->translate(vm::vec3d(-1, -2, -3));
  CHECK(document->canRepeatCommands());

  document->deselectAll();
  document->selectNodes({entityNode1});

  document->repeatCommands();
  CHECK(entityNode1->entity().origin() == vm::vec3d{0, 0, 0});

  document->deselectAll();
  document->selectNodes({entityNode1});
  CHECK(document->canRepeatCommands());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatTransaction")
{
  auto* entityNode1 = new mdl::EntityNode({});
  document->addNodes({{document->parentForNodes(), {entityNode1}}});

  document->selectNodes({entityNode1});
  CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

  document->startTransaction("", TransactionScope::Oneshot);
  document->translate(vm::vec3d(0, 0, 10));
  document->rollbackTransaction();
  document->translate(vm::vec3d(10, 0, 0));
  document->commitTransaction();
  // overall result: x += 10

  CHECK(entityNode1->entity().origin() == vm::vec3d(10, 0, 0));

  // now repeat the transaction on a second entity

  auto* entityNode2 = new mdl::EntityNode({});
  document->addNodes({{document->parentForNodes(), {entityNode2}}});

  document->deselectAll();
  document->selectNodes({entityNode2});
  CHECK(entityNode2->entity().origin() == vm::vec3d(0, 0, 0));

  CHECK(document->canRepeatCommands());
  document->repeatCommands();
  CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));

  document->repeatCommands();
  CHECK(entityNode2->entity().origin() == vm::vec3d(20, 0, 0));

  // ensure entityNode1 was unmodified

  CHECK(entityNode1->entity().origin() == vm::vec3d(10, 0, 0));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatDuplicateAndTranslate")
{
  auto* entityNode1 = new mdl::EntityNode({});
  document->addNodes({{document->parentForNodes(), {entityNode1}}});

  document->selectNodes({entityNode1});
  CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

  SECTION("transaction containing a rollback")
  {
    document->duplicate();

    document->startTransaction("", TransactionScope::Oneshot);
    document->translate(vm::vec3d(0, 0, 10));
    document->rollbackTransaction();
    document->translate(vm::vec3d(10, 0, 0));
    document->commitTransaction();
  }
  SECTION("translations that get coalesced")
  {
    document->duplicate();

    document->translate(vm::vec3d(5, 0, 0));
    document->translate(vm::vec3d(5, 0, 0));
  }
  SECTION("duplicate inside transaction, then standalone movements")
  {
    document->startTransaction("", TransactionScope::Oneshot);
    document->duplicate();
    document->translate(vm::vec3d(2, 0, 0));
    document->translate(vm::vec3d(2, 0, 0));
    document->commitTransaction();

    document->translate(vm::vec3d(2, 0, 0));
    document->translate(vm::vec3d(2, 0, 0));
    document->translate(vm::vec3d(2, 0, 0));
  }

  // repeatable actions:
  //  - duplicate
  //  - translate by x = +10

  REQUIRE(document->selection().allEntities().size() == 1);

  auto* entityNode2 = document->selection().allEntities().at(0);
  CHECK(entityNode2 != entityNode1);

  CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));
  CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));

  document->repeatCommands();

  REQUIRE(document->selection().allEntities().size() == 1);

  auto* entityNode3 = document->selection().allEntities().at(0);
  CHECK(entityNode3 != entityNode2);

  CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));
  CHECK(entityNode2->entity().origin() == vm::vec3d(10, 0, 0));
  CHECK(entityNode3->entity().origin() == vm::vec3d(20, 0, 0));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatUndo")
{
  auto* entityNode1 = new mdl::EntityNode({});
  document->addNodes({{document->parentForNodes(), {entityNode1}}});

  document->selectNodes({entityNode1});
  CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

  document->translate(vm::vec3d(0, 0, 10));
  CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 10));
  CHECK(document->canRepeatCommands());

  document->undoCommand();
  CHECK(entityNode1->entity().origin() == vm::vec3d(0, 0, 0));

  // For now, we won't support repeating a sequence of commands
  // containing undo/redo (it just clears the repeat stack)
  CHECK(!document->canRepeatCommands());
}

} // namespace tb::ui
