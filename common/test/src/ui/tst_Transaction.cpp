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
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "ui/Transaction.h"

#include "vm/mat_ext.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "Transaction")
{
  document->selectAllNodes();
  document->remove();
  document->selectAllNodes();

  REQUIRE(document->selection().empty());

  auto* entityNode = new mdl::EntityNode{mdl::Entity{}};

  auto transaction = Transaction{document};
  CHECK(transaction.state() == Transaction::State::Running);

  document->addNodes({{document->parentForNodes(), {entityNode}}});
  document->selectNodes({entityNode});
  document->transform("translate", vm::translation_matrix(vm::vec3d{1, 0, 0}));

  REQUIRE(transaction.state() == Transaction::State::Running);
  REQUIRE(entityNode->entity().origin() == vm::vec3d{1, 0, 0});

  SECTION("commit")
  {
    CHECK(transaction.commit());

    CHECK(transaction.state() == Transaction::State::Committed);
    CHECK(entityNode->entity().origin() == vm::vec3d{1, 0, 0});

    document->undoCommand();
    document->selectAllNodes();

    CHECK(document->selection().empty());
  }

  SECTION("rollback")
  {
    transaction.rollback();

    CHECK(transaction.state() == Transaction::State::Running);

    document->selectAllNodes();
    CHECK(document->selection().empty());

    // must commit the transaction in order to destroy it
    transaction.commit();
  }

  SECTION("cancel")
  {
    transaction.cancel();

    CHECK(transaction.state() == Transaction::State::Cancelled);

    document->selectAllNodes();
    CHECK(document->selection().empty());
  }
}

} // namespace tb::ui
