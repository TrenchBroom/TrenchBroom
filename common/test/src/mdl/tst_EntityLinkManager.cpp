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

#include "mdl/Entity.h"
#include "mdl/EntityDefinition.h"
#include "mdl/EntityLinkManager.h"
#include "mdl/EntityNode.h"
#include "mdl/NodeIndex.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

TEST_CASE("EntityLinkManager")
{
  using namespace std::string_literals;
  using namespace EntityPropertyKeys;
  using LinkEndsForName = EntityLinkManager::LinkEndsForName;

  const auto SourceProp = "target"s;
  const auto AltSourceProp = "alt_target"s;
  const auto TargetProp = "targetname"s;
  const auto AltTargetProp = "alt_targetname"s;

  auto i = NodeIndex{};
  auto m = EntityLinkManager{i};

  const auto sourceDefinition = EntityDefinition{
    "source_definition",
    {},
    {},
    {
      {SourceProp, PropertyValueTypes::TargetDestination{}, {}, {}},
      {AltSourceProp, PropertyValueTypes::TargetDestination{}, {}, {}},
    }};

  const auto targetDefinition = EntityDefinition{
    "target_definition",
    {},
    {},
    {
      {TargetProp, PropertyValueTypes::TargetSource{}, {}, {}},
      {AltTargetProp, PropertyValueTypes::TargetSource{}, {}, {}},
    }};

  const auto sourceTargetDefinition = EntityDefinition{
    "source_target_definition",
    {},
    {},
    {
      {SourceProp, PropertyValueTypes::TargetDestination{}, {}, {}},
      {TargetProp, PropertyValueTypes::TargetSource{}, {}, {}},
    }};

  SECTION("Adding and removing entities")
  {
    auto sourceNode = EntityNode{Entity{{
      {SourceProp, "some_name"},
    }}};
    sourceNode.setDefinition(&sourceDefinition);

    auto targetNode = EntityNode{Entity{{
      {TargetProp, "some_name"},
    }}};
    targetNode.setDefinition(&targetDefinition);

    i.addNode(targetNode);
    i.addNode(sourceNode);

    m.addEntityNode(sourceNode);
    CHECK(
      m.linksFrom(sourceNode)
      == LinkEndsForName{
        {SourceProp, {&targetNode}},
      });
    CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
    CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
    CHECK(
      m.linksTo(targetNode)
      == LinkEndsForName{
        {SourceProp, {&sourceNode}},
      });

    m.addEntityNode(targetNode);
    CHECK(
      m.linksFrom(sourceNode)
      == LinkEndsForName{
        {SourceProp, {&targetNode}},
      });
    CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
    CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
    CHECK(
      m.linksTo(targetNode)
      == LinkEndsForName{
        {SourceProp, {&sourceNode}},
      });

    CHECK(!m.hasMissingSource(sourceNode));
    CHECK(!m.hasMissingTarget(sourceNode, SourceProp));
    CHECK(!m.hasMissingTarget(sourceNode, AltSourceProp));

    CHECK(!m.hasMissingTarget(targetNode, SourceProp));
    CHECK(!m.hasMissingTarget(targetNode, AltSourceProp));
    CHECK(!m.hasMissingSource(targetNode));

    SECTION("Removing the source node, then the target node")
    {
      m.removeEntityNode(sourceNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(!m.hasMissingTarget(sourceNode, SourceProp));
      CHECK(!m.hasMissingTarget(sourceNode, AltSourceProp));

      CHECK(!m.hasMissingTarget(targetNode, SourceProp));
      CHECK(!m.hasMissingTarget(targetNode, AltSourceProp));
      CHECK(m.hasMissingSource(targetNode));

      m.removeEntityNode(targetNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(!m.hasMissingTarget(sourceNode, SourceProp));
      CHECK(!m.hasMissingTarget(sourceNode, AltSourceProp));

      CHECK(!m.hasMissingTarget(targetNode, SourceProp));
      CHECK(!m.hasMissingTarget(targetNode, AltSourceProp));
      CHECK(!m.hasMissingSource(targetNode));
    }

    SECTION("Removing the target node")
    {
      m.removeEntityNode(targetNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{{SourceProp, {}}});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(m.hasMissingTarget(sourceNode, SourceProp));
      CHECK(!m.hasMissingTarget(sourceNode, AltSourceProp));

      CHECK(!m.hasMissingTarget(targetNode, SourceProp));
      CHECK(!m.hasMissingTarget(targetNode, AltSourceProp));
      CHECK(!m.hasMissingSource(targetNode));

      m.removeEntityNode(sourceNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(!m.hasMissingTarget(sourceNode, SourceProp));
      CHECK(!m.hasMissingTarget(sourceNode, Killtarget));

      CHECK(!m.hasMissingTarget(targetNode, SourceProp));
      CHECK(!m.hasMissingTarget(targetNode, AltSourceProp));
      CHECK(!m.hasMissingSource(targetNode));
    }
  }

  SECTION("No source or prop definitions")
  {
    auto n1 = EntityNode{Entity{{
      {SourceProp, "some_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {TargetProp, "some_name"},
    }}};

    i.addNode(n1);
    i.addNode(n2);

    m.addEntityNode(n1);
    m.addEntityNode(n2);

    CHECK(!m.hasLink(n1, n2, SourceProp));
    CHECK(!m.hasMissingTarget(n1, SourceProp));
    CHECK(!m.hasMissingSource(n2));
  }

  SECTION("No source prop definition")
  {
    auto n1 = EntityNode{Entity{{
      {SourceProp, "some_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {TargetProp, "some_name"},
    }}};

    n2.setDefinition(&targetDefinition);

    i.addNode(n1);
    i.addNode(n2);

    m.addEntityNode(n1);
    m.addEntityNode(n2);

    CHECK(!m.hasLink(n1, n2, SourceProp));
    CHECK(!m.hasMissingTarget(n1, SourceProp));
    CHECK(m.hasMissingSource(n2));
  }

  SECTION("No target prop definition")
  {
    auto n1 = EntityNode{Entity{{
      {SourceProp, "some_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {TargetProp, "some_name"},
    }}};

    n1.setDefinition(&sourceDefinition);

    i.addNode(n1);
    i.addNode(n2);

    m.addEntityNode(n1);
    m.addEntityNode(n2);

    CHECK(!m.hasLink(n1, n2, SourceProp));
    CHECK(m.hasMissingTarget(n1, SourceProp));
    CHECK(!m.hasMissingSource(n2));
  }

  SECTION("Mixed properties, same link name")
  {
    auto n1 = EntityNode{Entity{{
      {SourceProp, "some_name"},
      {AltSourceProp, "some_alt_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {TargetProp, "some_name"},
    }}};

    auto n3 = EntityNode{Entity{{
      {AltTargetProp, "some_alt_name"},
    }}};

    n1.setDefinition(&sourceDefinition);
    n2.setDefinition(&targetDefinition);
    n3.setDefinition(&targetDefinition);

    i.addNode(n1);
    i.addNode(n2);
    i.addNode(n3);

    m.addEntityNode(n1);
    m.addEntityNode(n2);
    m.addEntityNode(n3);

    CHECK(m.hasLink(n1, n2, SourceProp));
    CHECK(!m.hasLink(n1, n2, AltSourceProp));
    CHECK(!m.hasLink(n1, n3, SourceProp));
    CHECK(m.hasLink(n1, n3, AltSourceProp));
  }

  SECTION("Numbered properties")
  {
    auto n1 = EntityNode{Entity{{
      {SourceProp + "1", "some_name"},
      {SourceProp + "2", "some_other_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {TargetProp, "some_name"},
    }}};

    auto n3 = EntityNode{Entity{{
      {TargetProp, "some_other_name"},
    }}};

    n1.setDefinition(&sourceDefinition);
    n2.setDefinition(&targetDefinition);
    n3.setDefinition(&targetDefinition);

    i.addNode(n1);
    i.addNode(n2);
    i.addNode(n3);

    m.addEntityNode(n1);
    m.addEntityNode(n2);
    m.addEntityNode(n3);

    CHECK(m.hasLink(n1, n2, SourceProp));
    CHECK(m.hasLink(n1, n2, SourceProp));

    CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {&n2, &n3}}});
    CHECK(m.linksTo(n1) == LinkEndsForName{});
    CHECK(m.linksFrom(n2) == LinkEndsForName{});
    CHECK(m.linksTo(n2) == LinkEndsForName{{SourceProp, {&n1}}});
    CHECK(m.linksFrom(n3) == LinkEndsForName{});
    CHECK(m.linksTo(n3) == LinkEndsForName{{SourceProp, {&n1}}});
  }

  SECTION("Loop")
  {
    auto n = EntityNode{Entity{{
      {SourceProp, "n"},
      {TargetProp, "n"},
    }}};

    n.setDefinition(&sourceTargetDefinition);

    i.addNode(n);

    m.addEntityNode(n);
    CHECK(m.linksFrom(n) == LinkEndsForName{{SourceProp, {&n}}});
    CHECK(m.linksTo(n) == LinkEndsForName{{SourceProp, {&n}}});
    CHECK(!m.hasMissingTarget(n, SourceProp));
    CHECK(!m.hasMissingSource(n));

    m.removeEntityNode(n);
    CHECK(m.linksFrom(n) == LinkEndsForName{});
    CHECK(m.linksTo(n) == LinkEndsForName{});
    CHECK(!m.hasMissingTarget(n, SourceProp));
    CHECK(!m.hasMissingSource(n));
  }

  SECTION("Cycle")
  {
    auto n1 = EntityNode{Entity{{
      {TargetProp, "n1"},
      {SourceProp, "n2"},
    }}};

    auto n2 = EntityNode{Entity{{
      {TargetProp, "n2"},
      {SourceProp, "n1"},
    }}};

    n1.setDefinition(&sourceTargetDefinition);
    n2.setDefinition(&sourceTargetDefinition);

    i.addNode(n1);
    i.addNode(n2);

    m.addEntityNode(n1);
    m.addEntityNode(n2);

    CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {&n2}}});
    CHECK(m.linksTo(n1) == LinkEndsForName{{SourceProp, {&n2}}});
    CHECK(m.linksFrom(n2) == LinkEndsForName{{SourceProp, {&n1}}});
    CHECK(m.linksTo(n2) == LinkEndsForName{{SourceProp, {&n1}}});
    CHECK(!m.hasMissingTarget(n1, SourceProp));
    CHECK(!m.hasMissingSource(n1));
    CHECK(!m.hasMissingTarget(n2, SourceProp));
    CHECK(!m.hasMissingSource(n2));

    SECTION("Remove n1")
    {
      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{{SourceProp, {}}});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(m.hasMissingTarget(n2, SourceProp));
      CHECK(m.hasMissingSource(n2));
    }

    SECTION("Remove n2")
    {
      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, SourceProp));
      CHECK(m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
    }
  }

  SECTION("Chain")
  {
    auto n1 = EntityNode{Entity{{
      {SourceProp, "n2"},
    }}};

    auto n2 = EntityNode{Entity{{
      {TargetProp, "n2"},
      {SourceProp, "n3"},
    }}};

    auto n3 = EntityNode{Entity{{
      {TargetProp, "n3"},
    }}};

    n1.setDefinition(&sourceDefinition);
    n2.setDefinition(&sourceTargetDefinition);
    n3.setDefinition(&targetDefinition);

    i.addNode(n1);
    i.addNode(n2);
    i.addNode(n3);

    m.addEntityNode(n1);
    m.addEntityNode(n2);
    m.addEntityNode(n3);

    CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {&n2}}});
    CHECK(m.linksTo(n1) == LinkEndsForName{});
    CHECK(m.linksFrom(n2) == LinkEndsForName{{SourceProp, {&n3}}});
    CHECK(m.linksTo(n2) == LinkEndsForName{{SourceProp, {&n1}}});
    CHECK(m.linksFrom(n3) == LinkEndsForName{});
    CHECK(m.linksTo(n3) == LinkEndsForName{{SourceProp, {&n2}}});
    CHECK(!m.hasMissingTarget(n1, SourceProp));
    CHECK(!m.hasMissingSource(n1));
    CHECK(!m.hasMissingTarget(n2, SourceProp));
    CHECK(!m.hasMissingSource(n2));
    CHECK(!m.hasMissingTarget(n3, SourceProp));
    CHECK(!m.hasMissingSource(n3));

    SECTION("Remove n1, n2, n3")
    {
      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{{SourceProp, {&n3}}});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{{SourceProp, {&n2}}});
      CHECK(!m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(m.hasMissingSource(n3));

      m.removeEntityNode(n3);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(!m.hasMissingSource(n3));
    }

    SECTION("Remove n2, n3, n1")
    {
      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(m.hasMissingSource(n3));

      m.removeEntityNode(n3);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(!m.hasMissingSource(n3));
    }

    SECTION("Remove n3, n2, n1")
    {
      m.removeEntityNode(n3);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {&n2}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{{SourceProp, {}}});
      CHECK(m.linksTo(n2) == LinkEndsForName{{SourceProp, {&n1}}});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{SourceProp, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, SourceProp));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, SourceProp));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, SourceProp));
      CHECK(!m.hasMissingSource(n3));
    }
  }

  SECTION("hasLink")
  {
    auto sourceNode = EntityNode{Entity{{
      {SourceProp, "some_name"},
    }}};

    auto targetNode = EntityNode{Entity{{
      {TargetProp, "some_name"},
    }}};

    sourceNode.setDefinition(&sourceDefinition);
    targetNode.setDefinition(&targetDefinition);

    i.addNode(targetNode);
    i.addNode(sourceNode);
    REQUIRE(!m.hasLink(sourceNode, targetNode, SourceProp));

    m.addEntityNode(sourceNode);
    CHECK(m.hasLink(sourceNode, targetNode, SourceProp));

    m.addEntityNode(targetNode);
    CHECK(m.hasLink(sourceNode, targetNode, SourceProp));

    m.removeEntityNode(targetNode);
    CHECK(!m.hasLink(sourceNode, targetNode, SourceProp));
  }

  SECTION("Order of indexing and adding nodes")
  {
    auto n1 = EntityNode{Entity{{
      {SourceProp, "some_name"},
      {TargetProp, "some_other_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {SourceProp, "some_other_name"},
      {TargetProp, "some_name"},
    }}};

    n1.setDefinition(&sourceTargetDefinition);
    n2.setDefinition(&sourceTargetDefinition);

    SECTION("Index everything, then add")
    {
      i.addNode(n1);
      i.addNode(n2);

      m.addEntityNode(n1);
      CHECK(m.hasLink(n1, n2, SourceProp));
      CHECK(m.hasLink(n2, n1, SourceProp));

      m.addEntityNode(n2);
      CHECK(m.hasLink(n1, n2, SourceProp));
      CHECK(m.hasLink(n2, n1, SourceProp));
    }

    SECTION("Index and add nodes individually")
    {
      i.addNode(n1);
      m.addEntityNode(n1);

      CHECK(!m.hasLink(n1, n2, SourceProp));
      CHECK(!m.hasLink(n2, n1, SourceProp));

      i.addNode(n2);
      m.addEntityNode(n2);

      CHECK(m.hasLink(n1, n2, SourceProp));
      CHECK(m.hasLink(n2, n1, SourceProp));
    }
  }
}

} // namespace tb::mdl
