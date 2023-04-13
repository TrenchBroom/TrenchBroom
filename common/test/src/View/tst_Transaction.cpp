/*
 Copyright (C) 2023 Kristian Duske

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

#include "MapDocumentTest.h"
#include "TestUtils.h"

#include "Model/Entity.h"
#include "Model/EntityNode.h"

#include <vecmath/mat_ext.h>

#include "Catch2.h"

namespace TrenchBroom::View
{

TEST_CASE_METHOD(MapDocumentTest, "Transaction")
{
  document->selectAllNodes();
  document->deleteObjects();
  document->selectAllNodes();

  REQUIRE(document->selectedNodes().empty());

  auto* entityNode = new Model::EntityNode{Model::Entity{}};

  auto transaction = Transaction{document};
  CHECK(transaction.state() == Transaction::State::Running);

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->selectNodes({entityNode});
  document->transformObjects("translate", vm::translation_matrix(vm::vec3{1, 0, 0}));

  REQUIRE(transaction.state() == Transaction::State::Running);
  REQUIRE(entityNode->entity().origin() == vm::vec3{1, 0, 0});

  SECTION("commit")
  {
    CHECK(transaction.commit());

    CHECK(transaction.state() == Transaction::State::Committed);
    CHECK(entityNode->entity().origin() == vm::vec3{1, 0, 0});

    document->undoCommand();
    document->selectAllNodes();

    CHECK(document->selectedNodes().empty());
  }

  SECTION("rollback")
  {
    transaction.rollback();

    CHECK(transaction.state() == Transaction::State::Running);

    document->selectAllNodes();
    CHECK(document->selectedNodes().empty());

    // must commit the transaction in order to destroy it
    transaction.commit();
  }

  SECTION("cancel")
  {
    transaction.cancel();

    CHECK(transaction.state() == Transaction::State::Cancelled);

    document->selectAllNodes();
    /* EXPECTED:
    CHECK(document->selectedNodes().empty());
    ACTUAL: */
    CHECK_FALSE(document->selectedNodes().empty());
  }
}

} // namespace TrenchBroom::View
