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

#include "MapFixture.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Transaction.h"

#include "vm/mat_ext.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Transaction")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  REQUIRE_FALSE(map.selection().hasNodes());

  auto* entityNode = new EntityNode{Entity{}};

  auto transaction = Transaction{map};
  CHECK(transaction.state() == Transaction::State::Running);

  addNodes(map, {{parentForNodes(map), {entityNode}}});
  map.selectNodes({entityNode});
  transformSelection(map, "translate", vm::translation_matrix(vm::vec3d{1, 0, 0}));

  REQUIRE(transaction.state() == Transaction::State::Running);
  REQUIRE(entityNode->entity().origin() == vm::vec3d{1, 0, 0});

  SECTION("commit")
  {
    CHECK(transaction.commit());

    CHECK(transaction.state() == Transaction::State::Committed);
    CHECK(entityNode->entity().origin() == vm::vec3d{1, 0, 0});

    map.undoCommand();
    map.selectAllNodes();

    CHECK_FALSE(map.selection().hasNodes());
  }

  SECTION("rollback")
  {
    transaction.rollback();

    CHECK(transaction.state() == Transaction::State::Running);

    map.selectAllNodes();
    CHECK_FALSE(map.selection().hasNodes());

    // must commit the transaction in order to destroy it
    transaction.commit();
  }

  SECTION("cancel")
  {
    transaction.cancel();

    CHECK(transaction.state() == Transaction::State::Cancelled);

    map.selectAllNodes();
    CHECK_FALSE(map.selection().hasNodes());
  }
}

} // namespace tb::mdl
