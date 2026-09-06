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
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeQueryValues.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("NodeQueryValues")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  SECTION("layerNameValue")
  {
    CHECK(layerNameValue(map.worldNode()) == el::Value::Undefined);

    auto* layerNode = new LayerNode{Layer{"Combat"}};
    addNodes(map, {{&map.worldNode(), {layerNode}}});

    // findContainingLayer treats a layer as containing itself
    CHECK(layerNameValue(*layerNode) == el::Value{"Combat"});

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{layerNode, {brushNode}}});
    CHECK(layerNameValue(*brushNode) == el::Value{"Combat"});
  }

  SECTION("groupNameValue")
  {
    auto* groupNode = new GroupNode{Group{"Hallway"}};
    addNodes(map, {{&parentForNodes(map), {groupNode}}});

    // findContainingGroup looks at ancestors, not the group node itself
    CHECK(groupNameValue(*groupNode) == el::Value::Undefined);
    CHECK(groupNameValue(map.worldNode()) == el::Value::Undefined);

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{groupNode, {brushNode}}});
    CHECK(groupNameValue(*brushNode) == el::Value{"Hallway"});
  }

  SECTION("isLinked")
  {
    auto* groupNode = new GroupNode{Group{"Hallway"}};
    addNodes(map, {{&parentForNodes(map), {groupNode}}});

    auto* brushNode = createBrushNode(map);
    addNodes(map, {{groupNode, {brushNode}}});

    CHECK_FALSE(isLinked(map, *groupNode));
    CHECK_FALSE(isLinked(map, *brushNode));
    CHECK_FALSE(isLinked(map, map.worldNode()));

    selectNodes(map, {groupNode});
    auto* linkedGroupNode = createLinkedDuplicate(map);
    deselectAll(map);

    // now that the group belongs to a link set of size 2, both copies -- and everything
    // nested inside them -- report linked == true
    CHECK(isLinked(map, *groupNode));
    CHECK(isLinked(map, *linkedGroupNode));
    CHECK(isLinked(map, *brushNode));
  }
}

} // namespace tb::mdl
