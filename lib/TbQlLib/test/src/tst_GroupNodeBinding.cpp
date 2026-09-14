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
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "ql/GroupNodeBinding.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("GroupNodeBinding")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto* groupNode = new mdl::GroupNode{mdl::Group{"Hallway"}};
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {groupNode}}});

  const auto boundValue = makeGroupNodeBinding(map, *groupNode);

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
  CHECK(groupNodeFieldNames() == boundValue.keys());

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
    mdl::selectNodes(map, {groupNode});
    mdl::groupSelectedNodes(map, "Level");
    mdl::deselectAll(map);

    CHECK(makeGroupNodeBinding(map, *groupNode).at("groupName") == el::Value{"Level"});
  }

  SECTION("linked")
  {
    mdl::selectNodes(map, {groupNode});
    auto* linkedGroupNode = mdl::createLinkedDuplicate(map);
    mdl::deselectAll(map);

    CHECK(makeGroupNodeBinding(map, *groupNode).at("linked") == el::Value{true});
    CHECK(makeGroupNodeBinding(map, *linkedGroupNode).at("linked") == el::Value{true});
  }
}

} // namespace tb::ql
