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

#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "View/Command.h"
#include "View/MapDocument.h"
#include "View/MapDocumentTest.h"

#include <kdl/result.h>

#include <vecmath/approx.h>
#include <vecmath/bbox_io.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>
#include <vecmath/mat_io.h>
#include <vecmath/scalar.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include "TestUtils.h"

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.canRepeat") {
  CHECK_FALSE(document->canRepeatCommands());

  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entityNode);
  CHECK_FALSE(document->canRepeatCommands());

  document->selectNodes({entityNode});
  CHECK_FALSE(document->canRepeatCommands());

  document->duplicateObjects();
  CHECK(document->canRepeatCommands());

  document->clearRepeatableCommands();
  CHECK_FALSE(document->canRepeatCommands());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatTranslate") {
  auto* entityNode = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entityNode);
  document->selectNodes({entityNode});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->translateObjects(vm::vec3(1, 2, 3));
  CHECK(document->canRepeatCommands());

  REQUIRE(entityNode->entity().origin() == vm::vec3(1, 2, 3));
  document->repeatCommands();
  CHECK(entityNode->entity().origin() == vm::vec3(2, 4, 6));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatRotate") {
  auto entity = Model::Entity();
  entity.transform({}, vm::translation_matrix(vm::vec3(1, 2, 3)));

  auto* entityNode = new Model::EntityNode(std::move(entity));

  addNode(*document, document->parentForNodes(), entityNode);
  document->selectNodes({entityNode});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->rotateObjects(vm::vec3::zero(), vm::vec3::pos_z(), vm::to_radians(90.0));
  CHECK(document->canRepeatCommands());

  REQUIRE(
    entityNode->entity().origin() ==
    vm::approx(vm::rotation_matrix(vm::vec3::pos_z(), vm::to_radians(90.0)) * vm::vec3(1, 2, 3)));
  document->repeatCommands();
  CHECK(
    entityNode->entity().origin() ==
    vm::approx(vm::rotation_matrix(vm::vec3::pos_z(), vm::to_radians(180.0)) * vm::vec3(1, 2, 3)));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatScaleWithBBox") {
  auto* brushNode1 = createBrushNode();

  addNode(*document, document->parentForNodes(), brushNode1);
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  const auto oldBounds = brushNode1->logicalBounds();
  const auto newBounds = vm::bbox3(oldBounds.min, 2.0 * oldBounds.max);
  document->scaleObjects(oldBounds, newBounds);
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  addNode(*document, document->parentForNodes(), brushNode2);
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == newBounds);
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatScaleWithFactors") {
  auto* brushNode1 = createBrushNode();

  addNode(*document, document->parentForNodes(), brushNode1);
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->scaleObjects(brushNode1->logicalBounds().center(), vm::vec3(2, 2, 2));
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  addNode(*document, document->parentForNodes(), brushNode2);
  document->deselectAll();
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.shearObjects") {
  auto* brushNode1 = createBrushNode();
  const auto originalBounds = brushNode1->logicalBounds();

  addNode(*document, document->parentForNodes(), brushNode1);
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->shearObjects(originalBounds, vm::vec3::pos_z(), vm::vec3(32, 0, 0));
  REQUIRE(brushNode1->logicalBounds() != originalBounds);
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  addNode(*document, document->parentForNodes(), brushNode2);
  document->deselectAll();
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.flipObjects") {
  auto* brushNode1 = createBrushNode();
  const auto originalBounds = brushNode1->logicalBounds();

  addNode(*document, document->parentForNodes(), brushNode1);
  document->selectNodes({brushNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->flipObjects(originalBounds.max, vm::axis::z);
  REQUIRE(brushNode1->logicalBounds() != originalBounds);
  CHECK(document->canRepeatCommands());

  auto* brushNode2 = createBrushNode();
  addNode(*document, document->parentForNodes(), brushNode2);
  document->deselectAll();
  document->selectNodes({brushNode2});

  document->repeatCommands();
  CHECK(brushNode2->logicalBounds() == brushNode1->logicalBounds());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.selectionClears") {
  auto* entityNode1 = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entityNode1);

  auto* entityNode2 = new Model::EntityNode{Model::Entity{}};
  addNode(*document, document->parentForNodes(), entityNode2);

  document->selectNodes({entityNode1});

  REQUIRE_FALSE(document->canRepeatCommands());
  document->translateObjects(vm::vec3(1, 2, 3));
  REQUIRE(document->canRepeatCommands());

  document->deselectAll();
  document->selectNodes({entityNode2});
  CHECK(document->canRepeatCommands());

  // this command will not clear the repeat stack
  document->setProperty("this", "that");
  CHECK(document->canRepeatCommands());

  // this command will replace the command on the repeat stack
  document->translateObjects(vm::vec3(-1, -2, -3));
  CHECK(document->canRepeatCommands());

  document->deselectAll();
  document->selectNodes({entityNode1});

  document->repeatCommands();
  CHECK(entityNode1->entity().origin() == vm::vec3::zero());

  document->deselectAll();
  document->selectNodes({entityNode1});
  CHECK(document->canRepeatCommands());
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatTransaction") {
  auto* entityNode1 = new Model::EntityNode({});
  addNode(*document, document->parentForNodes(), entityNode1);

  document->selectNodes({entityNode1});
  CHECK(entityNode1->entity().origin() == vm::vec3(0, 0, 0));

  document->startTransaction("");
  document->translateObjects(vm::vec3(0, 0, 10));
  document->rollbackTransaction();
  document->translateObjects(vm::vec3(10, 0, 0));
  document->commitTransaction();
  // overall result: x += 10

  CHECK(entityNode1->entity().origin() == vm::vec3(10, 0, 0));

  // now repeat the transaction on a second entity

  auto* entityNode2 = new Model::EntityNode({});
  addNode(*document, document->parentForNodes(), entityNode2);

  document->deselectAll();
  document->selectNodes({entityNode2});
  CHECK(entityNode2->entity().origin() == vm::vec3(0, 0, 0));

  CHECK(document->canRepeatCommands());
  document->repeatCommands();
  CHECK(entityNode2->entity().origin() == vm::vec3(10, 0, 0));

  document->repeatCommands();
  CHECK(entityNode2->entity().origin() == vm::vec3(20, 0, 0));

  // ensure entityNode1 was unmodified

  CHECK(entityNode1->entity().origin() == vm::vec3(10, 0, 0));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatDuplicateAndTranslate") {
  auto* entityNode1 = new Model::EntityNode({});
  addNode(*document, document->parentForNodes(), entityNode1);

  document->selectNodes({entityNode1});
  CHECK(entityNode1->entity().origin() == vm::vec3(0, 0, 0));

  SECTION("transaction containing a rollback") {
    document->duplicateObjects();

    document->startTransaction("");
    document->translateObjects(vm::vec3(0, 0, 10));
    document->rollbackTransaction();
    document->translateObjects(vm::vec3(10, 0, 0));
    document->commitTransaction();
  }
  SECTION("translations that get coalesced") {
    document->duplicateObjects();

    document->translateObjects(vm::vec3(5, 0, 0));
    document->translateObjects(vm::vec3(5, 0, 0));
  }
  SECTION("duplicate inside transaction, then standalone movements") {
    document->startTransaction("");
    document->duplicateObjects();
    document->translateObjects(vm::vec3(2, 0, 0));
    document->translateObjects(vm::vec3(2, 0, 0));
    document->commitTransaction();

    document->translateObjects(vm::vec3(2, 0, 0));
    document->translateObjects(vm::vec3(2, 0, 0));
    document->translateObjects(vm::vec3(2, 0, 0));
  }

  // repeatable actions:
  //  - duplicate
  //  - translate by x = +10

  REQUIRE(document->allSelectedEntityNodes().size() == 1);

  auto* entityNode2 = document->allSelectedEntityNodes().at(0);
  CHECK(entityNode2 != entityNode1);

  CHECK(entityNode1->entity().origin() == vm::vec3(0, 0, 0));
  CHECK(entityNode2->entity().origin() == vm::vec3(10, 0, 0));

  document->repeatCommands();

  REQUIRE(document->allSelectedEntityNodes().size() == 1);

  auto* entityNode3 = document->allSelectedEntityNodes().at(0);
  CHECK(entityNode3 != entityNode2);

  CHECK(entityNode1->entity().origin() == vm::vec3(0, 0, 0));
  CHECK(entityNode2->entity().origin() == vm::vec3(10, 0, 0));
  CHECK(entityNode3->entity().origin() == vm::vec3(20, 0, 0));
}

TEST_CASE_METHOD(MapDocumentTest, "RepeatableActionsTest.repeatUndo") {
  auto* entityNode1 = new Model::EntityNode({});
  addNode(*document, document->parentForNodes(), entityNode1);

  document->selectNodes({entityNode1});
  CHECK(entityNode1->entity().origin() == vm::vec3(0, 0, 0));

  document->translateObjects(vm::vec3(0, 0, 10));
  CHECK(entityNode1->entity().origin() == vm::vec3(0, 0, 10));
  CHECK(document->canRepeatCommands());

  document->undoCommand();
  CHECK(entityNode1->entity().origin() == vm::vec3(0, 0, 0));

  // For now, we won't support repeating a sequence of commands
  // containing undo/redo (it just clears the repeat stack)
  CHECK(!document->canRepeatCommands());
}
} // namespace View
} // namespace TrenchBroom
