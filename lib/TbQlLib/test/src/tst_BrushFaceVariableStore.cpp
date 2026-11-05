/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Value.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TestFactory.h"
#include "ql/BrushFaceVariableStore.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("BrushFaceVariableStore")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto* brushNode =
    mdl::addNode(map, mdl::parentForNodes(map), mdl::createBrushNode(map));

  const auto handle = mdl::toHandles(*brushNode).front();

  auto store = makeBrushFaceVariableStore(map, handle);
  CHECK(store.value("type") == el::Value{"face"});
  CHECK(store.value("material") == el::Value{"material"});
  CHECK(store.size() == store.names().size());
  CHECK(store.value("missing") == el::Value::Undefined);

  // read-only, like every LazyMapVariableStore
  store.set("material", el::Value{"other"});
  CHECK(store.value("material") == el::Value{"material"});
}

} // namespace tb::ql
