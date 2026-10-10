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
#include "el/VariableStore.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/PatchNode.h" // IWYU pragma: keep
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep
#include "ql/NodeVariableStore.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("NodeVariableStore")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  SECTION("dispatches to the right per-kind LazyMap")
  {
    CHECK(
      makeNodeVariableStore(map, map.worldNode()).value("type") == el::Value{"world"});

    auto* layerNode = new mdl::LayerNode{mdl::Layer{"Combat"}};
    mdl::addNodes(map, {{&map.worldNode(), {layerNode}}});
    CHECK(makeNodeVariableStore(map, *layerNode).value("type") == el::Value{"layer"});
    CHECK(makeNodeVariableStore(map, *layerNode).value("name") == el::Value{"Combat"});

    auto* groupNode = new mdl::GroupNode{mdl::Group{"Hallway"}};
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {groupNode}}});
    CHECK(makeNodeVariableStore(map, *groupNode).value("type") == el::Value{"group"});
    CHECK(makeNodeVariableStore(map, *groupNode).value("name") == el::Value{"Hallway"});

    auto* entityNode =
      new mdl::EntityNode{mdl::Entity{{{"classname", "info_player_start"}}}};
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {entityNode}}});
    CHECK(makeNodeVariableStore(map, *entityNode).value("type") == el::Value{"entity"});
    CHECK(
      makeNodeVariableStore(map, *entityNode).value("classname")
      == el::Value{"info_player_start"});

    auto* brushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode}}});
    CHECK(makeNodeVariableStore(map, *brushNode).value("type") == el::Value{"brush"});

    auto* patchNode = mdl::createPatchNode();
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode}}});
    CHECK(makeNodeVariableStore(map, *patchNode).value("type") == el::Value{"patch"});
  }

  SECTION("behaves as a VariableStore")
  {
    auto* entityNode =
      new mdl::EntityNode{mdl::Entity{{{"classname", "info_player_start"}}}};
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {entityNode}}});

    auto store = makeNodeVariableStore(map, *entityNode);
    CHECK(store.size() == store.names().size());
    CHECK(store.value("missing") == el::Value::Undefined);

    const auto clone = std::unique_ptr<el::VariableStore>{store.clone()};
    CHECK(clone->value("classname") == el::Value{"info_player_start"});

    // read-only, like every LazyMapVariableStore
    store.set("classname", el::Value{"other"});
    CHECK(store.value("classname") == el::Value{"info_player_start"});
  }
}

} // namespace tb::ql
