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
#include "mdl/Map_CopyPaste.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
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
        REQUIRE(paste(map, serializeSelectedNodes(map)) == PasteType::Node);
        break;
      case Mode::Duplicate:
        duplicateSelectedNodes(map);
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

      addNodes(map, {{parentForNodes(map), {entityNode}}});
      selectNodes(map, {entityNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      SECTION("If the group is not linked")
      {
        openGroup(map, groupNode);

        selectNodes(map, {brushNode});
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
        const auto* linkedGroupNode = createLinkedDuplicate(map);
        REQUIRE(linkedGroupNode != nullptr);
        REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

        deselectAll(map);
        selectNodes(map, {groupNode});
        openGroup(map, groupNode);

        selectNodes(map, {entityNode});
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
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

      duplicateOrCopyPaste();

      auto* groupNodeCopy = map.selection().groups.at(0u);
      CHECK(groupNodeCopy->linkId() == groupNode->linkId());
    }

    SECTION("Nodes in a linked group")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedGroupNode->linkId() == groupNode->linkId());

      openGroup(map, groupNode);

      selectNodes(map, {brushNode});
      duplicateOrCopyPaste();

      auto* brushNodeCopy = map.selection().brushes.at(0u);
      CHECK(brushNodeCopy->linkId() != brushNode->linkId());
    }

    SECTION("Groups in a linked group")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});
      selectNodes(map, {brushNode});

      auto* innerGroupNode = groupSelectedNodes(map, "inner");
      REQUIRE(innerGroupNode != nullptr);

      auto* outerGroupNode = groupSelectedNodes(map, "outer");
      REQUIRE(outerGroupNode != nullptr);

      auto* linkedOuterGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedOuterGroupNode->linkId() == outerGroupNode->linkId());

      const auto linkedInnerGroupNode = getChildAs<GroupNode>(*linkedOuterGroupNode);
      REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      openGroup(map, outerGroupNode);

      selectNodes(map, {innerGroupNode});
      duplicateOrCopyPaste();

      auto* innerGroupNodeCopy = map.selection().groups.at(0u);
      CHECK(innerGroupNodeCopy->linkId() == innerGroupNode->linkId());
    }

    SECTION("Nested groups")
    {
      auto* innerBrushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {innerBrushNode}}});
      selectNodes(map, {innerBrushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      auto* outerBrushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {outerBrushNode}}});

      deselectAll(map);
      selectNodes(map, {groupNode, outerBrushNode});
      auto* outerGroupNode = groupSelectedNodes(map, "outer");

      deselectAll(map);
      selectNodes(map, {outerGroupNode});

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
      addNodes(map, {{parentForNodes(map), {innerBrushNode}}});
      selectNodes(map, {innerBrushNode});

      auto* innerGroupNode = groupSelectedNodes(map, "inner");
      REQUIRE(innerGroupNode != nullptr);

      deselectAll(map);
      selectNodes(map, {innerGroupNode});

      auto* linkedInnerGroupNode = createLinkedDuplicate(map);
      REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      const auto linkedInnerBrushNode = getChildAs<BrushNode>(*linkedInnerGroupNode);

      auto* outerBrushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {outerBrushNode}}});

      deselectAll(map);
      selectNodes(map, {innerGroupNode, linkedInnerGroupNode, outerBrushNode});
      auto* outerGroupNode = groupSelectedNodes(map, "outer");

      deselectAll(map);
      selectNodes(map, {outerGroupNode});

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
