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

#include "el/BoundValue.h"
#include "el/Value.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/GroupNodeBoundValue.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("GroupNodeBoundValue")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  auto* groupNode = new GroupNode{Group{"Hallway"}};
  addNodes(map, {{&parentForNodes(map), {groupNode}}});

  const auto boundValue = makeGroupNodeBoundValue(map, *groupNode);

  CHECK(
    boundValue.keys()
    == std::vector<std::string>{
      "bounds",
      "center",
      "groupName",
      "layerName",
      "linked",
      "locked",
      "name",
      "selected",
      "type",
      "visible",
    });

  CHECK(boundValue.at("type") == el::Value{"group"});
  CHECK(boundValue.at("name") == el::Value{"Hallway"});
  CHECK(boundValue.at("bounds") == el::Value{groupNode->logicalBounds()});
  CHECK(boundValue.at("center") == el::Value{groupNode->logicalBounds().center()});
  CHECK(boundValue.at("visible") == el::Value{true});
  CHECK(boundValue.at("locked") == el::Value{false});
  CHECK(boundValue.at("selected") == el::Value{false});

  // findContainingGroup looks at ancestors, not the group node itself
  CHECK(boundValue.at("groupName") == el::Value::Undefined);
  CHECK(boundValue.at("linked") == el::Value{false});

  CHECK(boundValue.at("missing") == std::nullopt);

  SECTION("nested inside another group")
  {
    selectNodes(map, {groupNode});
    groupSelectedNodes(map, "Level");
    deselectAll(map);

    CHECK(makeGroupNodeBoundValue(map, *groupNode).at("groupName") == el::Value{"Level"});
  }

  SECTION("linked")
  {
    selectNodes(map, {groupNode});
    auto* linkedGroupNode = createLinkedDuplicate(map);
    deselectAll(map);

    CHECK(makeGroupNodeBoundValue(map, *groupNode).at("linked") == el::Value{true});
    CHECK(makeGroupNodeBoundValue(map, *linkedGroupNode).at("linked") == el::Value{true});
  }
}

} // namespace tb::mdl
