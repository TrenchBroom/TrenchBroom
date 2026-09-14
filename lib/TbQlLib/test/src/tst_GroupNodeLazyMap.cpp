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
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "ql/GroupNodeLazyMap.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("GroupNodeLazyMap")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto* groupNode = new mdl::GroupNode{mdl::Group{"Hallway"}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {groupNode}}});

  const auto lazyMap = makeGroupNodeLazyMap(*groupNode);

  CHECK(
    lazyMap.keys()
    == std::vector<std::string>{
      "bounds",
      "center",
      "groupName",
      "layerName",
      "locked",
      "name",
      "selected",
      "type",
      "visible",
    });
  CHECK(groupNodeFieldNames() == lazyMap.keys());

  CHECK(lazyMap.at("type") == el::Value{"group"});
  CHECK(lazyMap.at("name") == el::Value{"Hallway"});
  CHECK(lazyMap.at("bounds") == el::Value{groupNode->logicalBounds()});
  CHECK(lazyMap.at("center") == el::Value{groupNode->logicalBounds().center()});
  CHECK(lazyMap.at("visible") == el::Value{true});
  CHECK(lazyMap.at("locked") == el::Value{false});
  CHECK(lazyMap.at("selected") == el::Value{false});

  // findContainingGroup looks at ancestors, not the group node itself
  CHECK(lazyMap.at("groupName") == el::Value::Undefined);

  CHECK(lazyMap.at("missing") == std::nullopt);

  SECTION("nested inside another group")
  {
    mdl::selectNodes(map, {groupNode});
    mdl::groupSelectedNodes(map, "Level");
    mdl::deselectAll(map);

    CHECK(makeGroupNodeLazyMap(*groupNode).at("groupName") == el::Value{"Level"});
  }
}

} // namespace tb::ql
