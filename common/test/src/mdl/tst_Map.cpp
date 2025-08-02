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
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Map.h"
#include "mdl/PasteType.h"

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();
  fixture.create();

  SECTION("Duplicate and Copy / Paste behave identically")
  {
    enum class Mode
    {
      CopyPaste,
      Duplicate,
    };

    const auto mode = GENERATE(Mode::CopyPaste, Mode::Duplicate);

    const auto duplicateOrCopyPaste = [&]() {
      switch (mode)
      {
      case Mode::CopyPaste:
        REQUIRE(map.paste(map.serializeSelectedNodes()) == PasteType::Node);
        break;
      case Mode::Duplicate:
        map.duplicateSelectedNodes();
        break;
        switchDefault();
      }
    };

    CAPTURE(mode);

    SECTION("Grouped nodes")
    {
      auto* entityNode = new EntityNode{Entity{}};
      auto* brushNode = createBrushNode(map);
      entityNode->addChild(brushNode);

      map.addNodes({{map.parentForNodes(), {entityNode}}});
      map.selectNodes({entityNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      SECTION("If the group is not linked")
      {
        map.openGroup(groupNode);

        map.selectNodes({brushNode});
        duplicateOrCopyPaste();

        const auto* brushNodeCopy = map.selection().brushes.at(0u);
        CHECK(brushNodeCopy->linkId() != brushNode->linkId());

        const auto* entityNodeCopy =
          dynamic_cast<const EntityNode*>(brushNodeCopy->entity());
        REQUIRE(entityNodeCopy != nullptr);
        CHECK(entityNodeCopy->linkId() != entityNode->linkId());
      }

      SECTION("If the group is linked")
      {
        const auto* linkedGroupNode = map.createLinkedDuplicate();
        REQUIRE(linkedGroupNode != nullptr);
        REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

        map.deselectAll();
        map.selectNodes({groupNode});
        map.openGroup(groupNode);

        map.selectNodes({entityNode});
        duplicateOrCopyPaste();

        const auto* brushNodeCopy = map.selection().brushes.at(0u);
        CHECK(brushNodeCopy->linkId() != brushNode->linkId());

        const auto* entityNodeCopy =
          dynamic_cast<const EntityNode*>(brushNodeCopy->entity());
        REQUIRE(entityNodeCopy != nullptr);
        CHECK(entityNodeCopy->linkId() != entityNode->linkId());
      }
    }

    SECTION("Linked group")
    {
      auto* brushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

      duplicateOrCopyPaste();

      auto* groupNodeCopy = map.selection().groups.at(0u);
      CHECK(groupNodeCopy->linkId() == groupNode->linkId());
    }

    SECTION("Nodes in a linked group")
    {
      auto* brushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {brushNode}}});
      map.selectNodes({brushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

      map.openGroup(groupNode);

      map.selectNodes({brushNode});
      duplicateOrCopyPaste();

      auto* brushNodeCopy = map.selection().brushes.at(0u);
      CHECK(brushNodeCopy->linkId() != brushNode->linkId());
    }

    SECTION("Groups in a linked group")
    {
      auto* brushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {brushNode}}});
      map.selectNodes({brushNode});

      auto* innerGroupNode = map.groupSelectedNodes("inner");
      REQUIRE(innerGroupNode != nullptr);

      auto* outerGroupNode = map.groupSelectedNodes("outer");
      REQUIRE(outerGroupNode != nullptr);

      auto* linkedOuterGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedOuterGroupNode->linkId() == outerGroupNode->linkId());

      const auto linkedInnerGroupNode = getChildAs<GroupNode>(*linkedOuterGroupNode);
      REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      map.openGroup(outerGroupNode);

      map.selectNodes({innerGroupNode});
      duplicateOrCopyPaste();

      auto* innerGroupNodeCopy = map.selection().groups.at(0u);
      CHECK(innerGroupNodeCopy->linkId() == innerGroupNode->linkId());
    }

    SECTION("Nested groups")
    {
      auto* innerBrushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {innerBrushNode}}});
      map.selectNodes({innerBrushNode});

      auto* groupNode = map.groupSelectedNodes("test");
      REQUIRE(groupNode != nullptr);

      auto* outerBrushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {outerBrushNode}}});

      map.deselectAll();
      map.selectNodes({groupNode, outerBrushNode});
      auto* outerGroupNode = map.groupSelectedNodes("outer");

      map.deselectAll();
      map.selectNodes({outerGroupNode});

      duplicateOrCopyPaste();

      const auto* outerGroupNodeCopy = map.selection().groups.at(0u);
      const auto [groupNodeCopy, outerBrushNodeCopy] =
        getChildrenAs<GroupNode, BrushNode>(*outerGroupNodeCopy);

      CHECK(groupNodeCopy->linkId() != groupNode->linkId());
      CHECK(outerBrushNodeCopy->linkId() != outerBrushNode->linkId());
    }

    SECTION("Nested linked groups")
    {
      /*
      outerGroupNode  this node is duplicated
        innerGroupNode
          innerBrushNode
        linkedInnerGroupNode
          linkedInnerBrushNode
        outerBrushNode
      */

      auto* innerBrushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {innerBrushNode}}});
      map.selectNodes({innerBrushNode});

      auto* innerGroupNode = map.groupSelectedNodes("inner");
      REQUIRE(innerGroupNode != nullptr);

      map.deselectAll();
      map.selectNodes({innerGroupNode});

      auto* linkedInnerGroupNode = map.createLinkedDuplicate();
      REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      const auto linkedInnerBrushNode = getChildAs<BrushNode>(*linkedInnerGroupNode);

      auto* outerBrushNode = createBrushNode(map);
      map.addNodes({{map.parentForNodes(), {outerBrushNode}}});

      map.deselectAll();
      map.selectNodes({innerGroupNode, linkedInnerGroupNode, outerBrushNode});
      auto* outerGroupNode = map.groupSelectedNodes("outer");

      map.deselectAll();
      map.selectNodes({outerGroupNode});

      duplicateOrCopyPaste();

      const auto* outerGroupNodeCopy = map.selection().groups.at(0);
      REQUIRE(outerGroupNodeCopy != nullptr);
      REQUIRE(outerGroupNodeCopy->childCount() == 3u);

      const auto [innerGroupNodeCopy, linkedInnerGroupNodeCopy, outerBrushNodeCopy] =
        getChildrenAs<GroupNode, GroupNode, BrushNode>(*outerGroupNodeCopy);

      const auto innerBrushNodeCopy = getChildAs<BrushNode>(*innerGroupNodeCopy);

      const auto linkedInnerBrushNodeCopy =
        getChildAs<BrushNode>(*linkedInnerGroupNodeCopy);

      CHECK(innerGroupNodeCopy->linkId() == innerGroupNode->linkId());
      CHECK(linkedInnerGroupNodeCopy->linkId() == linkedInnerGroupNode->linkId());
      CHECK(innerBrushNodeCopy->linkId() == innerBrushNode->linkId());
      CHECK(linkedInnerBrushNodeCopy->linkId() == linkedInnerBrushNode->linkId());
      CHECK(outerBrushNodeCopy->linkId() != outerBrushNode->linkId());
    }
  }
}

} // namespace tb::mdl
