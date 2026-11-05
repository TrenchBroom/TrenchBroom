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

    auto* layerNode = mdl::addNode(
      map, map.worldNode(), std::make_unique<mdl::LayerNode>(mdl::Layer{"Combat"}));
    CHECK(makeNodeVariableStore(map, *layerNode).value("type") == el::Value{"layer"});
    CHECK(makeNodeVariableStore(map, *layerNode).value("name") == el::Value{"Combat"});

    auto* groupNode = mdl::addNode(
      map,
      mdl::parentForNodes(map),
      std::make_unique<mdl::GroupNode>(mdl::Group{"Hallway"}));
    CHECK(makeNodeVariableStore(map, *groupNode).value("type") == el::Value{"group"});
    CHECK(makeNodeVariableStore(map, *groupNode).value("name") == el::Value{"Hallway"});

    auto* entityNode = mdl::addNode(
      map,
      mdl::parentForNodes(map),
      std::make_unique<mdl::EntityNode>(
        mdl::Entity{{{"classname", "info_player_start"}}}));
    CHECK(makeNodeVariableStore(map, *entityNode).value("type") == el::Value{"entity"});
    CHECK(
      makeNodeVariableStore(map, *entityNode).value("classname")
      == el::Value{"info_player_start"});

    auto* brushNode =
      mdl::addNode(map, mdl::parentForNodes(map), mdl::createBrushNode(map));
    CHECK(makeNodeVariableStore(map, *brushNode).value("type") == el::Value{"brush"});

    auto* patchNode = mdl::addNode(map, mdl::parentForNodes(map), mdl::createPatchNode());
    CHECK(makeNodeVariableStore(map, *patchNode).value("type") == el::Value{"patch"});
  }

  SECTION("behaves as a VariableStore")
  {
    auto* entityNode = mdl::addNode(
      map,
      mdl::parentForNodes(map),
      std::make_unique<mdl::EntityNode>(
        mdl::Entity{{{"classname", "info_player_start"}}}));

    auto store = makeNodeVariableStore(map, *entityNode);
    CHECK(store.size() == store.names().size());
    CHECK(store.value("missing") == el::Value::Undefined);

    // read-only, like every LazyMapVariableStore
    store.set("classname", el::Value{"other"});
    CHECK(store.value("classname") == el::Value{"info_player_start"});
  }
}

} // namespace tb::ql
