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

#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Brushes.h"
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Matchers.h"
#include "mdl/NodeIndex.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestFactory.h"
#include "mdl/TestUtils.h"

#include "kd/vector_utils.h"

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
  auto& map = fixture.create();

  SECTION("creating the world indexes the world node")
  {
    CHECK(map.findNodes("classname") == std::vector<Node*>{&map.worldNode()});
  }

  SECTION("adding nodes updates the index")
  {
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto* entityNode = &groupNode->addChild(std::make_unique<EntityNode>(Entity{{
      {"some_key", "some_value"},
    }}));

    addNode(map, parentForNodes(map), std::move(groupNodePtr));

    REQUIRE(map.findNodes("classname") == std::vector<Node*>{&map.worldNode()});

    CHECK(map.findNodes("some_key") == std::vector<Node*>{entityNode});
    CHECK(map.findNodes("group") == std::vector<Node*>{groupNode});
  }

  SECTION("special characters in property keys can be queried with escaped patterns")
  {
    auto literalEntityNodePtr = std::make_unique<EntityNode>(Entity{{
      {"name*?%\\", "literal_value"},
    }});
    auto* literalEntityNode = literalEntityNodePtr.get();
    auto wildcardEntityNodePtr = std::make_unique<EntityNode>(Entity{{
      {"name1234", "wildcard_value"},
    }});
    auto* wildcardEntityNode = wildcardEntityNodePtr.get();

    addNodes(
      map,
      parentForNodes(map),
      kdl::vec_from<std::unique_ptr<Node>>(
        std::move(literalEntityNodePtr), std::move(wildcardEntityNodePtr)));

    CHECK_THAT(
      map.findNodes("name*"),
      UnorderedEquals(std::vector<Node*>{literalEntityNode, wildcardEntityNode}));
    CHECK_THAT(
      map.findNodes(NodeIndex::escapePattern("name*?%\\")),
      UnorderedEquals(std::vector<Node*>{literalEntityNode}));
  }

  SECTION("Removing nodes updates the index")
  {
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto* entityNode = &groupNode->addChild(std::make_unique<EntityNode>(Entity{{
      {"some_key", "some_value"},
    }}));

    addNode(map, parentForNodes(map), std::move(groupNodePtr));

    REQUIRE(map.findNodes("classname") == std::vector<Node*>{&map.worldNode()});
    REQUIRE(map.findNodes("some_key") == std::vector<Node*>{entityNode});
    REQUIRE(map.findNodes("group") == std::vector<Node*>{groupNode});

    SECTION("Recursively removing nodes")
    {
      removeNodes(map, {groupNode});

      REQUIRE(map.findNodes("classname") == std::vector<Node*>{&map.worldNode()});

      CHECK(map.findNodes("some_key") == std::vector<Node*>{});
      CHECK(map.findNodes("group") == std::vector<Node*>{});
    }

    SECTION("Removing leaf nodes")
    {
      // prevent the group from being removed when it's empty
      addNode(map, *groupNode, std::make_unique<EntityNode>(Entity{}));

      removeNodes(map, {entityNode});

      REQUIRE(map.findNodes("classname") == std::vector<Node*>{&map.worldNode()});

      CHECK(map.findNodes("some_key") == std::vector<Node*>{});
      CHECK(map.findNodes("group") == std::vector<Node*>{groupNode});
    }
  }

  SECTION("Updating nodes updates the index")
  {
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto* entityNode = &groupNode->addChild(std::make_unique<EntityNode>(Entity{{
      {"some_key", "some_value"},
    }}));

    addNode(map, parentForNodes(map), std::move(groupNodePtr));
    selectNodes(map, {entityNode});

    REQUIRE(map.findNodes("classname") == std::vector<Node*>{&map.worldNode()});
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
    auto groupNodePtr = std::make_unique<GroupNode>(Group{"group"});
    auto* groupNode = groupNodePtr.get();
    auto* entityNode = &groupNode->addChild(std::make_unique<EntityNode>(Entity{{
      {"some_key", "some_value"},
    }}));

    addNode(map, parentForNodes(map), std::move(groupNodePtr));
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
