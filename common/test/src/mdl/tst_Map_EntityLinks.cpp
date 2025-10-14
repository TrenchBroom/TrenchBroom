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
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityLinkManager.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
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

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>
#include <catch2/matchers/catch_matchers_quantifiers.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("Map_EntityLinks")
{
  using namespace EntityPropertyKeys;

  auto fixture = MapFixture{};
  auto& map = fixture.map();

  fixture.create({.game = MockGameFixture{}});

  constexpr auto sourceClassname = "source_definition";
  constexpr auto targetClassname = "target_definition";

  map.entityDefinitionManager().setDefinitions(
    {{sourceClassname,
      {},
      {},
      {
        {Target, PropertyValueTypes::TargetDestination{}, {}, {}},
      }},
     {targetClassname,
      {},
      {},
      {
        {Targetname, PropertyValueTypes::TargetSource{}, {}, {}},
      }}});

  SECTION("Adding nodes adds their links")
  {
    auto* sourceNode = new EntityNode{Entity{{
      {Classname, sourceClassname},
      {Target, "some_value"},
    }}};

    auto* targetNode = new EntityNode{Entity{{
      {Classname, targetClassname},
      {Targetname, "some_value"},
    }}};

    addNodes(map, {{parentForNodes(map), {sourceNode, targetNode}}});
    CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
  }

  SECTION("Removing nodes removes their links")
  {
    auto* sourceNode = new EntityNode{Entity{{
      {Classname, sourceClassname},
      {Target, "some_value"},
    }}};

    auto* targetNode = new EntityNode{Entity{{
      {Classname, targetClassname},
      {Targetname, "some_value"},
    }}};

    addNodes(map, {{parentForNodes(map), {sourceNode, targetNode}}});
    REQUIRE(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

    SECTION("Remove source node")
    {
      removeNodes(map, {sourceNode});
      CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }

    SECTION("Remove target node")
    {
      removeNodes(map, {targetNode});
      CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }
  }

  SECTION("Changing classname updates links")
  {
    SECTION("When the entities are already linked")
    {
      auto* sourceNode = new EntityNode{Entity{{
        {Classname, sourceClassname},
        {Target, "some_value"},
      }}};

      auto* targetNode = new EntityNode{Entity{{
        {Classname, targetClassname},
        {Targetname, "some_value"},
      }}};

      addNodes(map, {{parentForNodes(map), {sourceNode, targetNode}}});
      REQUIRE(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

      SECTION("Change source classname")
      {
        selectNodes(map, {sourceNode});
        setEntityProperty(map, Classname, "some_other_class");
        CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
      }

      SECTION("Remove source classname")
      {
        selectNodes(map, {sourceNode});
        removeEntityProperty(map, Classname);
        CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
      }

      SECTION("Change target classname")
      {
        selectNodes(map, {targetNode});
        setEntityProperty(map, Classname, "some_other_class");
        CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
      }

      SECTION("Remove target classname")
      {
        selectNodes(map, {targetNode});
        removeEntityProperty(map, Classname);
        CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
      }
    }

    SECTION("When the entities are not linked")
    {
      auto* sourceNode = new EntityNode{Entity{{
        {Classname, "some_other_class"},
        {Target, "some_value"},
      }}};

      auto* targetNode = new EntityNode{Entity{{
        {Classname, "yet_anotehr_class"},
        {Targetname, "some_value"},
      }}};

      addNodes(map, {{parentForNodes(map), {sourceNode, targetNode}}});
      REQUIRE(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

      SECTION("Change source classname, then target classname")
      {
        selectNodes(map, {sourceNode});
        setEntityProperty(map, Classname, sourceClassname);
        CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

        deselectAll(map);
        selectNodes(map, {targetNode});
        setEntityProperty(map, Classname, targetClassname);
        CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
      }

      SECTION("Change target classname, then source classname")
      {
        selectNodes(map, {targetNode});
        setEntityProperty(map, Classname, targetClassname);
        CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

        deselectAll(map);
        selectNodes(map, {sourceNode});
        setEntityProperty(map, Classname, sourceClassname);
        CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
      }
    }
  }

  SECTION("Setting properties updates links")
  {
    auto* sourceNode = new EntityNode{Entity{{
      {Classname, sourceClassname},
    }}};
    auto* targetNode = new EntityNode{Entity{{
      {Classname, targetClassname},
    }}};

    addNodes(map, {{parentForNodes(map), {sourceNode, targetNode}}});
    REQUIRE(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

    SECTION("First set target, then targetname")
    {
      selectNodes(map, {sourceNode});
      setEntityProperty(map, Target, "some_target");
      CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

      deselectAll(map);
      selectNodes(map, {targetNode});
      setEntityProperty(map, Targetname, "some_target");
      CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }

    SECTION("First set targetname, then target")
    {
      selectNodes(map, {targetNode});
      setEntityProperty(map, Targetname, "some_target");
      CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

      deselectAll(map);
      selectNodes(map, {sourceNode});
      setEntityProperty(map, Target, "some_target");
      CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }
  }

  SECTION("Unsetting properties removes links")
  {
    auto* sourceNode = new EntityNode{Entity{{
      {Classname, sourceClassname},
      {Target, "some_value"},
    }}};

    auto* targetNode = new EntityNode{Entity{{
      {Classname, targetClassname},
      {Targetname, "some_value"},
    }}};

    addNodes(map, {{parentForNodes(map), {sourceNode, targetNode}}});
    REQUIRE(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));

    SECTION("Unset target property")
    {
      selectNodes(map, {sourceNode});
      removeEntityProperty(map, Target);
      CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }

    SECTION("Unset targetname property")
    {
      selectNodes(map, {targetNode});
      removeEntityProperty(map, Targetname);
      CHECK(!map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }
  }

  SECTION("Grouped nodes")
  {
    auto* sourceNode = new EntityNode{Entity{{
      {Classname, sourceClassname},
      {Target, "some_value"},
    }}};

    auto* targetNode = new EntityNode{Entity{{
      {Classname, targetClassname},
      {Targetname, "some_value"},
    }}};

    auto* sourceGroupNode = new GroupNode{mdl::Group{"source"}};
    sourceGroupNode->addChild(sourceNode);

    addNodes(map, {{parentForNodes(map), {sourceGroupNode, targetNode}}});

    SECTION("Adding a grouped node adds links")
    {
      CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }

    SECTION("Grouping a linked node retains links")
    {
      selectNodes(map, {targetNode});
      groupSelectedNodes(map, "target");
      CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
    }

    SECTION("Creating a linked duplicate replicates links")
    {
      selectNodes(map, {sourceGroupNode});
      auto* linkedSourceGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedSourceGroupNode);
      REQUIRE(linkedSourceGroupNode->childCount() == 1);

      const auto* linkedSourceNode =
        dynamic_cast<EntityNode*>(linkedSourceGroupNode->children().front());
      REQUIRE(linkedSourceNode);

      CHECK(map.entityLinkManager().hasLink(*sourceNode, *targetNode, Target));
      CHECK(map.entityLinkManager().hasLink(*linkedSourceNode, *targetNode, Target));
    }
  }
}

} // namespace tb::mdl
