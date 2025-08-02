/*
 Copyright (C) 2025 Kristian Duske

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

#include "Logger.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Selection")
{
  auto taskManager = createTestTaskManager();
  auto logger = NullLogger{};
  auto map = Map{*taskManager, logger};

  SECTION("selectNodes")
  {
    SECTION("Linked groups")
    {
      auto* entityNode = new EntityNode{Entity{}};
      auto* brushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {brushNode, entityNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      SECTION("Cannot select linked groups if selection is empty")
      {
        map.deselectAll();
        CHECK_FALSE(map.canSelectLinkedGroups());
      }

      SECTION("Cannot select linked groups if selection contains non-groups")
      {
        map.deselectAll();
        map.selectNodes({entityNode});
        CHECK_FALSE(map.canSelectLinkedGroups());
        map.selectNodes({groupNode});
        CHECK_FALSE(map.canSelectLinkedGroups());
      }

      SECTION("Cannot select linked groups if selection contains unlinked groups")
      {
        map.deselectAll();
        map.selectNodes({entityNode});

        auto* unlinkedGroupNode = map.groupSelectedNodes("other");
        REQUIRE(unlinkedGroupNode != nullptr);

        CHECK_FALSE(map.canSelectLinkedGroups());

        map.selectNodes({groupNode});
        CHECK_FALSE(map.canSelectLinkedGroups());
      }

      SECTION("Select linked groups")
      {
        auto* linkedGroupNode = map.createLinkedDuplicate();
        REQUIRE(linkedGroupNode != nullptr);

        map.deselectAll();
        map.selectNodes({groupNode});

        REQUIRE(map.canSelectLinkedGroups());
        map.selectLinkedGroups();
        CHECK_THAT(
          map.selection().nodes,
          Catch::UnorderedEquals(std::vector<Node*>{groupNode, linkedGroupNode}));
      }
    }
  }

  SECTION("selectBrushFaces")
  {
    SECTION("Linked groups")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3768

      auto* brushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedGroupNode != nullptr);

      map.deselectAll();

      SECTION("Face selection locks other groups in link set")
      {
        CHECK(!linkedGroupNode->locked());

        map.selectBrushFaces({{brushNode, 0}});
        CHECK(linkedGroupNode->locked());

        map.deselectAll();
        CHECK(!linkedGroupNode->locked());
      }
    }
  }
}

} // namespace tb::mdl
