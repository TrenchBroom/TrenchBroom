/*
 Copyright (C) 2022 Kristian Duske

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
#include "TestFactory.h"
#include "mdl/BrushNode.h" // IWYU pragma: keep
#include "mdl/CurrentGroupCommand.h"
#include "mdl/GroupNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/UpdateLinkedGroupsCommand.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("UpdateLinkedGroupsCommand")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  const auto createLinkedGroup = [&]() {
    auto brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});
    map.selectNodes({brushNode});

    auto* groupNode = map.groupSelectedNodes("group");
    map.selectNodes({groupNode});

    auto* linkedGroupNode = map.createLinkedDuplicate();
    map.deselectAll();

    return std::tuple{groupNode, linkedGroupNode};
  };

  auto [groupNode1, linkedGroupNode1] = createLinkedGroup();
  auto [groupNode2, linkedGroupNode2] = createLinkedGroup();

  SECTION("Collate two UpdateLinkedGroupCommand instances")
  {
    auto firstCommand = UpdateLinkedGroupsCommand{{groupNode1}};
    auto secondCommand = UpdateLinkedGroupsCommand{{groupNode1, groupNode2}};

    firstCommand.performDo(map);
    secondCommand.performDo(map);

    CHECK(firstCommand.collateWith(secondCommand));
  }

  SECTION("Collate UpdateLinkedGroupCommand with another command")
  {
    auto firstCommand = UpdateLinkedGroupsCommand{{groupNode1}};
    auto secondCommand = CurrentGroupCommand{groupNode2};

    firstCommand.performDo(map);
    secondCommand.performDo(map);

    CHECK_FALSE(firstCommand.collateWith(secondCommand));
  }
}

} // namespace tb::mdl
