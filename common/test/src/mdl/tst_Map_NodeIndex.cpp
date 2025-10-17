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
 along with TrenchBroom. If not, see <http:www.gnu.org/licenses/>.
 */

#include "MapFixture.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/TagMatcher.h"

#include "catch/CatchConfig.h"
#include "catch/Matchers.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>
#include <catch2/matchers/catch_matchers_quantifiers.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_NodeIndex")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  fixture.create();

  SECTION("creating the world indexes the world node")
  {
    CHECK(map.findNodes("classname") == std::vector<Node*>{map.world()});
  }

  SECTION("adding nodes updates the index")
  {
    auto* groupNode = new GroupNode{Group{"group"}};
    auto* entityNode = new EntityNode{Entity{{
      {"some_key", "some_value"},
    }}};

    groupNode->addChild(entityNode);

    addNodes(map, {{parentForNodes(map), {groupNode}}});

    REQUIRE(map.findNodes("classname") == std::vector<Node*>{map.world()});

    CHECK(map.findNodes("some_key") == std::vector<Node*>{entityNode});
    CHECK(map.findNodes("group") == std::vector<Node*>{groupNode});
  }

  SECTION("Removing nodes updates the index")
  {
    auto* groupNode = new GroupNode{Group{"group"}};
    auto* entityNode = new EntityNode{Entity{{
      {"some_key", "some_value"},
    }}};

    groupNode->addChild(entityNode);

    addNodes(map, {{parentForNodes(map), {groupNode}}});

    REQUIRE(map.findNodes("classname") == std::vector<Node*>{map.world()});
    REQUIRE(map.findNodes("some_key") == std::vector<Node*>{entityNode});
    REQUIRE(map.findNodes("group") == std::vector<Node*>{groupNode});

    SECTION("Recursively removing nodes")
    {
      removeNodes(map, {groupNode});

      REQUIRE(map.findNodes("classname") == std::vector<Node*>{map.world()});

      CHECK(map.findNodes("some_key") == std::vector<Node*>{});
      CHECK(map.findNodes("group") == std::vector<Node*>{});
    }

    SECTION("Removing leaf nodes")
    {
      // prevent the group from being removed when it's empty
      auto* otherEntityNode = new EntityNode{Entity{}};
      addNodes(map, {{groupNode, {otherEntityNode}}});

      removeNodes(map, {entityNode});

      REQUIRE(map.findNodes("classname") == std::vector<Node*>{map.world()});

      CHECK(map.findNodes("some_key") == std::vector<Node*>{});
      CHECK(map.findNodes("group") == std::vector<Node*>{groupNode});
    }
  }

  SECTION("Updating nodes updates the index")
  {
    auto* groupNode = new GroupNode{Group{"group"}};
    auto* entityNode = new EntityNode{Entity{{
      {"some_key", "some_value"},
    }}};

    groupNode->addChild(entityNode);

    addNodes(map, {{parentForNodes(map), {groupNode}}});
    selectNodes(map, {entityNode});

    REQUIRE(map.findNodes("classname") == std::vector<Node*>{map.world()});
    REQUIRE(map.findNodes("some_key") == std::vector<Node*>{entityNode});
    REQUIRE(map.findNodes("group") == std::vector<Node*>{groupNode});

    setEntityProperty(map, "some_other_key", "some_other_value");
    CHECK(map.findNodes("some_key") == std::vector<Node*>{entityNode});
    CHECK(map.findNodes("some_other_key") == std::vector<Node*>{entityNode});

    removeEntityProperty(map, "some_key");
    CHECK(map.findNodes("some_key") == std::vector<Node*>{});
    CHECK(map.findNodes("some_other_key") == std::vector<Node*>{entityNode});
  }

  SECTION("Linked Groups")
  {
    auto* groupNode = new GroupNode{Group{"group"}};
    auto* entityNode = new EntityNode{Entity{{
      {"some_key", "some_value"},
    }}};

    groupNode->addChild(entityNode);

    addNodes(map, {{parentForNodes(map), {groupNode}}});
    selectNodes(map, {groupNode});

    auto* linkedGroupNode = createLinkedDuplicate(map);
    REQUIRE(linkedGroupNode);
    REQUIRE(linkedGroupNode->childCount() == 1u);
    auto* linkedEntityNode = linkedGroupNode->children().front();

    deselectAll(map);

    SECTION("Creating a linked group updates the index")
    {
      CHECK_THAT(
        map.findNodes("some_key"),
        UnorderedEquals(std::vector<Node*>{entityNode, linkedEntityNode}));
    }

    SECTION("Upading a linked group updates the index")
    {
      openGroup(map, *linkedGroupNode);
      selectNodes(map, {linkedEntityNode});

      setEntityProperty(map, "some_other_key", "some_other_value");

      CHECK_THAT(
        map.findNodes("some_other_key"),
        UnorderedEquals(std::vector<Node*>{
          groupNode->children().front(),
          linkedGroupNode->children().front(),
        }));
    }
  }
}

} // namespace tb::mdl
