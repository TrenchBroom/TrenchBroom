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
  using namespace EntityPropertyKeys;
  using LinkEndsForName = EntityLinkManager::LinkEndsForName;

  auto i = NodeIndex{};
  auto m = EntityLinkManager{i};

  SECTION("Adding and removing entities")
  {
    auto sourceNode = EntityNode{Entity{{
      {Target, "some_name"},
    }}};

    auto targetNode = EntityNode{Entity{{
      {Targetname, "some_name"},
    }}};

    i.addNode(targetNode);
    i.addNode(sourceNode);

    m.addEntityNode(sourceNode);
    CHECK(
      m.linksFrom(sourceNode)
      == LinkEndsForName{
        {Target, {&targetNode}},
      });
    CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
    CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
    CHECK(
      m.linksTo(targetNode)
      == LinkEndsForName{
        {Target, {&sourceNode}},
      });

    m.addEntityNode(targetNode);
    CHECK(
      m.linksFrom(sourceNode)
      == LinkEndsForName{
        {Target, {&targetNode}},
      });
    CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
    CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
    CHECK(
      m.linksTo(targetNode)
      == LinkEndsForName{
        {Target, {&sourceNode}},
      });

    CHECK(!m.hasMissingSource(sourceNode));
    CHECK(!m.hasMissingTarget(sourceNode, Target));
    CHECK(!m.hasMissingTarget(sourceNode, Killtarget));

    CHECK(!m.hasMissingTarget(targetNode, Target));
    CHECK(!m.hasMissingTarget(targetNode, Killtarget));
    CHECK(!m.hasMissingSource(targetNode));

    SECTION("Removing the source node, then the target node")
    {
      m.removeEntityNode(sourceNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(!m.hasMissingTarget(sourceNode, Target));
      CHECK(!m.hasMissingTarget(sourceNode, Killtarget));

      CHECK(!m.hasMissingTarget(targetNode, Target));
      CHECK(!m.hasMissingTarget(targetNode, Killtarget));
      CHECK(m.hasMissingSource(targetNode));

      m.removeEntityNode(targetNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(!m.hasMissingTarget(sourceNode, Target));
      CHECK(!m.hasMissingTarget(sourceNode, Killtarget));

      CHECK(!m.hasMissingTarget(targetNode, Target));
      CHECK(!m.hasMissingTarget(targetNode, Killtarget));
      CHECK(!m.hasMissingSource(targetNode));
    }

    SECTION("Removing the target node")
    {
      m.removeEntityNode(targetNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{{Target, {}}});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(m.hasMissingTarget(sourceNode, Target));
      CHECK(!m.hasMissingTarget(sourceNode, Killtarget));

      CHECK(!m.hasMissingTarget(targetNode, Target));
      CHECK(!m.hasMissingTarget(targetNode, Killtarget));
      CHECK(!m.hasMissingSource(targetNode));

      m.removeEntityNode(sourceNode);
      CHECK(m.linksFrom(sourceNode) == LinkEndsForName{});
      CHECK(m.linksTo(sourceNode) == LinkEndsForName{});
      CHECK(m.linksFrom(targetNode) == LinkEndsForName{});
      CHECK(m.linksTo(targetNode) == LinkEndsForName{});

      CHECK(!m.hasMissingSource(sourceNode));
      CHECK(!m.hasMissingTarget(sourceNode, Target));
      CHECK(!m.hasMissingTarget(sourceNode, Killtarget));

      CHECK(!m.hasMissingTarget(targetNode, Target));
      CHECK(!m.hasMissingTarget(targetNode, Killtarget));
      CHECK(!m.hasMissingSource(targetNode));
    }
  }

  SECTION("Killtarget")
  {
    auto n1 = EntityNode{Entity{{
      {Killtarget, "some_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {Targetname, "some_name"},
    }}};

    i.addNode(n1);
    i.addNode(n2);

    m.addEntityNode(n1);
    m.addEntityNode(n2);

    CHECK(!m.hasLink(n1, n2, Target));
    CHECK(m.hasLink(n1, n2, Killtarget));
  }

  SECTION("Numbered properties")
  {
    auto n1 = EntityNode{Entity{{
      {Target + "1", "some_name"},
      {Target + "2", "some_other_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {Targetname, "some_name"},
    }}};

    auto n3 = EntityNode{Entity{{
      {Targetname, "some_other_name"},
    }}};

    i.addNode(n1);
    i.addNode(n2);
    i.addNode(n3);

    m.addEntityNode(n1);
    m.addEntityNode(n2);
    m.addEntityNode(n3);

    CHECK(m.hasLink(n1, n2, Target));
    CHECK(m.hasLink(n1, n2, Target));

    CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {&n2, &n3}}});
    CHECK(m.linksTo(n1) == LinkEndsForName{});
    CHECK(m.linksFrom(n2) == LinkEndsForName{});
    CHECK(m.linksTo(n2) == LinkEndsForName{{Target, {&n1}}});
    CHECK(m.linksFrom(n3) == LinkEndsForName{});
    CHECK(m.linksTo(n3) == LinkEndsForName{{Target, {&n1}}});
  }

  SECTION("Loop")
  {
    auto n = EntityNode{Entity{{
      {Target, "n"},
      {Targetname, "n"},
    }}};

    i.addNode(n);

    m.addEntityNode(n);
    CHECK(m.linksFrom(n) == LinkEndsForName{{Target, {&n}}});
    CHECK(m.linksTo(n) == LinkEndsForName{{Target, {&n}}});
    CHECK(!m.hasMissingTarget(n, Target));
    CHECK(!m.hasMissingSource(n));

    m.removeEntityNode(n);
    CHECK(m.linksFrom(n) == LinkEndsForName{});
    CHECK(m.linksTo(n) == LinkEndsForName{});
    CHECK(!m.hasMissingTarget(n, Target));
    CHECK(!m.hasMissingSource(n));
  }

  SECTION("Cycle")
  {
    auto n1 = EntityNode{Entity{{
      {Targetname, "n1"},
      {Target, "n2"},
    }}};

    auto n2 = EntityNode{Entity{{
      {Targetname, "n2"},
      {Target, "n1"},
    }}};

    i.addNode(n1);
    i.addNode(n2);

    m.addEntityNode(n1);
    m.addEntityNode(n2);

    CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {&n2}}});
    CHECK(m.linksTo(n1) == LinkEndsForName{{Target, {&n2}}});
    CHECK(m.linksFrom(n2) == LinkEndsForName{{Target, {&n1}}});
    CHECK(m.linksTo(n2) == LinkEndsForName{{Target, {&n1}}});
    CHECK(!m.hasMissingTarget(n1, Target));
    CHECK(!m.hasMissingSource(n1));
    CHECK(!m.hasMissingTarget(n2, Target));
    CHECK(!m.hasMissingSource(n2));

    SECTION("Remove n1")
    {
      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{{Target, {}}});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(m.hasMissingTarget(n2, Target));
      CHECK(m.hasMissingSource(n2));
    }

    SECTION("Remove n2")
    {
      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, Target));
      CHECK(m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
    }
  }

  SECTION("Chain")
  {
    auto n1 = EntityNode{Entity{{
      {Target, "n2"},
    }}};

    auto n2 = EntityNode{Entity{{
      {Targetname, "n2"},
      {Target, "n3"},
    }}};

    auto n3 = EntityNode{Entity{{
      {Targetname, "n3"},
    }}};

    i.addNode(n1);
    i.addNode(n2);
    i.addNode(n3);

    m.addEntityNode(n1);
    m.addEntityNode(n2);
    m.addEntityNode(n3);

    CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {&n2}}});
    CHECK(m.linksTo(n1) == LinkEndsForName{});
    CHECK(m.linksFrom(n2) == LinkEndsForName{{Target, {&n3}}});
    CHECK(m.linksTo(n2) == LinkEndsForName{{Target, {&n1}}});
    CHECK(m.linksFrom(n3) == LinkEndsForName{});
    CHECK(m.linksTo(n3) == LinkEndsForName{{Target, {&n2}}});
    CHECK(!m.hasMissingTarget(n1, Target));
    CHECK(!m.hasMissingSource(n1));
    CHECK(!m.hasMissingTarget(n2, Target));
    CHECK(!m.hasMissingSource(n2));
    CHECK(!m.hasMissingTarget(n3, Target));
    CHECK(!m.hasMissingSource(n3));

    SECTION("Remove n1, n2, n3")
    {
      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{{Target, {&n3}}});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{{Target, {&n2}}});
      CHECK(!m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(m.hasMissingSource(n3));

      m.removeEntityNode(n3);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(!m.hasMissingSource(n3));
    }

    SECTION("Remove n2, n3, n1")
    {
      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(m.hasMissingSource(n3));

      m.removeEntityNode(n3);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(!m.hasMissingSource(n3));
    }

    SECTION("Remove n3, n2, n1")
    {
      m.removeEntityNode(n3);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {&n2}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{{Target, {}}});
      CHECK(m.linksTo(n2) == LinkEndsForName{{Target, {&n1}}});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n2);
      CHECK(m.linksFrom(n1) == LinkEndsForName{{Target, {}}});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(!m.hasMissingSource(n3));

      m.removeEntityNode(n1);
      CHECK(m.linksFrom(n1) == LinkEndsForName{});
      CHECK(m.linksTo(n1) == LinkEndsForName{});
      CHECK(m.linksFrom(n2) == LinkEndsForName{});
      CHECK(m.linksTo(n2) == LinkEndsForName{});
      CHECK(m.linksFrom(n3) == LinkEndsForName{});
      CHECK(m.linksTo(n3) == LinkEndsForName{});
      CHECK(!m.hasMissingTarget(n1, Target));
      CHECK(!m.hasMissingSource(n1));
      CHECK(!m.hasMissingTarget(n2, Target));
      CHECK(!m.hasMissingSource(n2));
      CHECK(!m.hasMissingTarget(n3, Target));
      CHECK(!m.hasMissingSource(n3));
    }
  }

  SECTION("hasLink")
  {
    auto sourceNode = EntityNode{Entity{{
      {Target, "some_name"},
    }}};

    auto targetNode = EntityNode{Entity{{
      {Targetname, "some_name"},
    }}};

    i.addNode(targetNode);
    i.addNode(sourceNode);
    REQUIRE(!m.hasLink(sourceNode, targetNode, Target));

    m.addEntityNode(sourceNode);
    CHECK(m.hasLink(sourceNode, targetNode, Target));

    m.addEntityNode(targetNode);
    CHECK(m.hasLink(sourceNode, targetNode, Target));

    m.removeEntityNode(targetNode);
    CHECK(!m.hasLink(sourceNode, targetNode, Target));
  }

  SECTION("Order of indexing and adding nodes")
  {
    auto n1 = EntityNode{Entity{{
      {Target, "some_name"},
      {Targetname, "some_other_name"},
    }}};

    auto n2 = EntityNode{Entity{{
      {Target, "some_other_name"},
      {Targetname, "some_name"},
    }}};

    SECTION("Index everything, then add")
    {
      i.addNode(n1);
      i.addNode(n2);

      m.addEntityNode(n1);
      CHECK(m.hasLink(n1, n2, Target));
      CHECK(m.hasLink(n2, n1, Target));

      m.addEntityNode(n2);
      CHECK(m.hasLink(n1, n2, Target));
      CHECK(m.hasLink(n2, n1, Target));
    }

    SECTION("Index and add nodes individually")
    {
      i.addNode(n1);
      m.addEntityNode(n1);

      CHECK(!m.hasLink(n1, n2, Target));
      CHECK(!m.hasLink(n2, n1, Target));

      i.addNode(n2);
      m.addEntityNode(n2);

      CHECK(m.hasLink(n1, n2, Target));
      CHECK(m.hasLink(n2, n1, Target));
    }
  }
}

} // namespace tb::mdl
