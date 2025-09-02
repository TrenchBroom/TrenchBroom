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

#include "MapFixture.h"
#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/Entity.h"
#include "mdl/EntityDefinitionManager.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Entities.h"
#include "mdl/Map_Groups.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/ModelUtils.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include <functional>
#include <vector>

#include "catch/Matchers.h"

#include "Catch2.h"

namespace tb::mdl
{

TEST_CASE("Map_Groups")
{
  auto fixture = MapFixture{};
  auto& map = fixture.map();

  fixture.create();

  map.entityDefinitionManager().setDefinitions({
    {"point_entity",
     Color{},
     "this is a point entity",
     {},
     PointEntityDefinition{vm::bbox3d{16.0}, {}, {}}},
  });

  const auto& pointEntityDefinition = map.entityDefinitionManager().definitions().front();

  SECTION("groupSelectedNodes")
  {
    SECTION("Create empty group")
    {
      CHECK(groupSelectedNodes(map, "test") == nullptr);
    }

    SECTION("Create group with one node")
    {
      using CreateNode = std::function<Node*(const Map&)>;
      auto createNode = GENERATE_COPY(
        CreateNode{[](const auto& m) { return createBrushNode(m); }},
        CreateNode{[](const auto&) { return createPatchNode(); }});

      auto* node = createNode(map);
      addNodes(map, {{parentForNodes(map), {node}}});
      selectNodes(map, {node});

      auto* groupNode = groupSelectedNodes(map, "test");
      CHECK(groupNode != nullptr);

      CHECK(node->parent() == groupNode);
      CHECK(groupNode->selected());
      CHECK_FALSE(node->selected());

      map.undoCommand();
      CHECK(groupNode->parent() == nullptr);
      CHECK(node->parent() == parentForNodes(map));
      CHECK(node->selected());
    }

    SECTION("Create group with partial brush entity")
    {
      auto* childNode1 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {childNode1}}});

      auto* childNode2 = createPatchNode();
      addNodes(map, {{parentForNodes(map), {childNode2}}});

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      reparentNodes(map, {{entityNode, {childNode1, childNode2}}});

      selectNodes(map, {childNode1});

      GroupNode* groupNode = groupSelectedNodes(map, "test");
      CHECK(groupNode != nullptr);

      CHECK(childNode1->parent() == entityNode);
      CHECK(childNode2->parent() == entityNode);
      CHECK(entityNode->parent() == groupNode);
      CHECK(groupNode->selected());
      CHECK_FALSE(childNode1->selected());

      map.undoCommand();
      CHECK(groupNode->parent() == nullptr);
      CHECK(childNode1->parent() == entityNode);
      CHECK(childNode2->parent() == entityNode);
      CHECK(entityNode->parent() == parentForNodes(map));
      CHECK_FALSE(groupNode->selected());
      CHECK(childNode1->selected());
    }

    SECTION("Create group with full brush entity")
    {
      auto* childNode1 = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {childNode1}}});

      auto* childNode2 = createPatchNode();
      addNodes(map, {{parentForNodes(map), {childNode2}}});

      auto* entityNode = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode}}});
      reparentNodes(map, {{entityNode, {childNode1, childNode2}}});

      selectNodes(map, {childNode1, childNode2});

      auto* groupNode = groupSelectedNodes(map, "test");
      CHECK(groupNode != nullptr);

      CHECK(childNode1->parent() == entityNode);
      CHECK(childNode2->parent() == entityNode);
      CHECK(entityNode->parent() == groupNode);
      CHECK(groupNode->selected());
      CHECK_FALSE(childNode1->selected());
      CHECK_FALSE(childNode2->selected());

      map.undoCommand();
      CHECK(groupNode->parent() == nullptr);
      CHECK(childNode1->parent() == entityNode);
      CHECK(childNode2->parent() == entityNode);
      CHECK(entityNode->parent() == parentForNodes(map));
      CHECK_FALSE(groupNode->selected());
      CHECK(childNode1->selected());
      CHECK(childNode2->selected());
    }

    SECTION("New groups are added to the source layer")
    {
      auto* layerNode1 = new LayerNode{Layer{"test1"}};
      auto* layerNode2 = new LayerNode{Layer{"test2"}};
      addNodes(map, {{map.world(), {layerNode1}}});
      addNodes(map, {{map.world(), {layerNode2}}});

      setCurrentLayer(map, layerNode1);
      auto* entityNode = createPointEntity(map, pointEntityDefinition, {0, 0, 0});
      CHECK(entityNode->parent() == layerNode1);
      CHECK(layerNode1->childCount() == 1);

      setCurrentLayer(map, layerNode2);
      selectNodes(map, {entityNode});
      auto* newGroupNode = groupSelectedNodes(map, "Group in Layer 1");

      CHECK(entityNode->parent() == newGroupNode);
      CHECK(findContainingLayer(entityNode) == layerNode1);
      CHECK(findContainingLayer(newGroupNode) == layerNode1);
      CHECK(map.editorContext().currentLayer() == layerNode2);
    }

    SECTION("Grouping objects within a linked group keeps their link IDs")
    {
      auto* nestedBrushNode = createBrushNode(map);
      auto* nestedEntityNode = new EntityNode{Entity{}};

      addNodes(map, {{parentForNodes(map), {nestedBrushNode, nestedEntityNode}}});
      selectNodes(map, {nestedBrushNode, nestedEntityNode});

      auto* nestedGroupNode = groupSelectedNodes(map, "nested");

      deselectAll(map);
      selectNodes(map, {nestedGroupNode});

      auto* linkedNestedGroupNode = createLinkedDuplicate(map);

      auto* brushNode = createBrushNode(map);
      auto* entityNode = new EntityNode{Entity{}};
      auto* entityBrushNode = createBrushNode(map);
      entityNode->addChild(entityBrushNode);

      addNodes(map, {{parentForNodes(map), {brushNode, entityNode}}});

      selectNodes(map, {brushNode, entityNode, nestedGroupNode});
      auto* groupNode = groupSelectedNodes(map, "group");

      deselectAll(map);
      selectNodes(map, {groupNode});

      auto* linkedGroupNode = createLinkedDuplicate(map);
      auto* linkedGroupNode2 = createLinkedDuplicate(map);

      deselectAll(map);

      const auto originalNestedBrushLinkId = nestedBrushNode->linkId();
      const auto originalBrushLinkId = brushNode->linkId();
      const auto originalEntityLinkId = entityNode->linkId();
      const auto originalEntityBrushLinkId = entityBrushNode->linkId();

      REQUIRE_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));
      REQUIRE_THAT(*linkedGroupNode2, MatchesNode(*groupNode));

      selectNodes(map, {entityNode});
      groupSelectedNodes(map, "new group");
      CHECK(entityNode->linkId() == originalEntityLinkId);
      CHECK(entityBrushNode->linkId() == originalEntityBrushLinkId);

      CHECK_THAT(*linkedNestedGroupNode, MatchesNode(*nestedGroupNode));
      CHECK_THAT(*linkedGroupNode, MatchesNode(*groupNode));
      CHECK_THAT(*linkedGroupNode2, MatchesNode(*groupNode));
    }
  }

  SECTION("ungroupSelectedNodes")
  {
    SECTION("Ungroup inner group")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/2050

      auto* outerEntityNode1 = new EntityNode{Entity{}};
      auto* outerEntityNode2 = new EntityNode{Entity{}};
      auto* innerEntityNode1 = new EntityNode{Entity{}};
      auto* innerEntityNode2 = new EntityNode{Entity{}};

      addNodes(map, {{parentForNodes(map), {innerEntityNode1}}});
      addNodes(map, {{parentForNodes(map), {innerEntityNode2}}});
      selectNodes(map, {innerEntityNode1, innerEntityNode2});

      auto* innerGroupNode = groupSelectedNodes(map, "Inner");

      deselectAll(map);
      addNodes(map, {{parentForNodes(map), {outerEntityNode1}}});
      addNodes(map, {{parentForNodes(map), {outerEntityNode2}}});
      selectNodes(map, {innerGroupNode, outerEntityNode1, outerEntityNode2});

      auto* outerGroupNode = groupSelectedNodes(map, "Outer");
      deselectAll(map);

      // check our assumptions
      CHECK(outerGroupNode->childCount() == 3u);
      CHECK(innerGroupNode->childCount() == 2u);

      CHECK(outerGroupNode->parent() == map.editorContext().currentLayer());

      CHECK(outerEntityNode1->parent() == outerGroupNode);
      CHECK(outerEntityNode2->parent() == outerGroupNode);
      CHECK(innerGroupNode->parent() == outerGroupNode);

      CHECK(innerEntityNode1->parent() == innerGroupNode);
      CHECK(innerEntityNode2->parent() == innerGroupNode);

      CHECK(map.editorContext().currentGroup() == nullptr);
      CHECK(!outerGroupNode->opened());
      CHECK(!innerGroupNode->opened());

      CHECK(findOutermostClosedGroup(innerEntityNode1) == outerGroupNode);
      CHECK(findOutermostClosedGroup(outerEntityNode1) == outerGroupNode);

      CHECK(findContainingGroup(innerEntityNode1) == innerGroupNode);
      CHECK(findContainingGroup(outerEntityNode1) == outerGroupNode);

      // open the outer group and ungroup the inner group
      openGroup(map, outerGroupNode);
      selectNodes(map, {innerGroupNode});
      ungroupSelectedNodes(map);
      deselectAll(map);

      CHECK(innerEntityNode1->parent() == outerGroupNode);
      CHECK(innerEntityNode2->parent() == outerGroupNode);
    }

    SECTION("Ungrouping leaves a point entity selected")
    {
      auto* entityNode1 = new EntityNode{Entity{}};

      addNodes(map, {{parentForNodes(map), {entityNode1}}});
      selectNodes(map, {entityNode1});

      auto* groupNode = groupSelectedNodes(map, "Group");
      CHECK_THAT(map.selection().nodes, Catch::Equals(std::vector<Node*>{groupNode}));

      ungroupSelectedNodes(map);
      CHECK_THAT(map.selection().nodes, Catch::Equals(std::vector<Node*>{entityNode1}));
    }

    SECTION("Ungrouping leaves a brush entity selected")
    {
      const auto builder = BrushBuilder{map.world()->mapFormat(), map.worldBounds()};

      auto* entityNode1 = new EntityNode{Entity{}};
      addNodes(map, {{parentForNodes(map), {entityNode1}}});

      auto* brushNode1 = new BrushNode{
        builder.createCuboid(vm::bbox3d{{0, 0, 0}, {64, 64, 64}}, "material")
        | kdl::value()};
      addNodes(map, {{entityNode1, {brushNode1}}});
      selectNodes(map, {entityNode1});
      CHECK_THAT(map.selection().nodes, Catch::Equals(std::vector<Node*>{brushNode1}));
      CHECK_FALSE(entityNode1->selected());
      CHECK(brushNode1->selected());

      auto* groupNode = groupSelectedNodes(map, "Group");
      CHECK_THAT(groupNode->children(), Catch::Equals(std::vector<Node*>{entityNode1}));
      CHECK_THAT(entityNode1->children(), Catch::Equals(std::vector<Node*>{brushNode1}));
      CHECK_THAT(map.selection().nodes, Catch::Equals(std::vector<Node*>{groupNode}));
      CHECK(map.selection().allBrushes() == std::vector<BrushNode*>{brushNode1});
      CHECK(!map.selection().hasBrushes());

      ungroupSelectedNodes(map);
      CHECK_THAT(map.selection().nodes, Catch::Equals(std::vector<Node*>{brushNode1}));
      CHECK_FALSE(entityNode1->selected());
      CHECK(brushNode1->selected());
    }

    SECTION("Ungrouping works in a mixed selection")
    {
      // https://github.com/TrenchBroom/TrenchBroom/issues/3824
      auto* entityNode1 = new EntityNode{Entity{}};
      auto* entityNode2 = new EntityNode{Entity{}};

      addNodes(map, {{parentForNodes(map), {entityNode1}}});
      addNodes(map, {{parentForNodes(map), {entityNode2}}});
      selectNodes(map, {entityNode1});

      auto* groupNode = groupSelectedNodes(map, "Group");
      selectNodes(map, {entityNode2});
      CHECK_THAT(
        map.selection().nodes,
        Catch::UnorderedEquals(std::vector<Node*>{groupNode, entityNode2}));

      ungroupSelectedNodes(map);
      CHECK_THAT(
        map.selection().nodes,
        Catch::UnorderedEquals(std::vector<Node*>{entityNode1, entityNode2}));
    }

    SECTION("Ungrouping linked groups")
    {
      auto* brushNode = createBrushNode(map);
      addNodes(map, {{parentForNodes(map), {brushNode}}});

      selectNodes(map, {brushNode});

      auto* groupNode = groupSelectedNodes(map, "test");
      REQUIRE(groupNode != nullptr);

      const auto originalGroupLinkId = groupNode->linkId();
      const auto originalBrushLinkId = brushNode->linkId();

      deselectAll(map);
      selectNodes(map, {groupNode});

      auto* linkedGroupNode = createLinkedDuplicate(map);

      deselectAll(map);
      selectNodes(map, {linkedGroupNode});

      auto* linkedGroupNode2 = createLinkedDuplicate(map);
      deselectAll(map);

      auto* linkedBrushNode =
        dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
      auto* linkedBrushNode2 =
        dynamic_cast<BrushNode*>(linkedGroupNode2->children().front());


      REQUIRE_THAT(
        map.world()->defaultLayer()->children(),
        Catch::UnorderedEquals(
          std::vector<Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));

      SECTION(
        "Given three linked groups, we ungroup one of them, the other two remain linked")
      {
        selectNodes(map, {linkedGroupNode2});

        ungroupSelectedNodes(map);
        CHECK_THAT(
          map.world()->defaultLayer()->children(),
          Catch::UnorderedEquals(
            std::vector<Node*>{groupNode, linkedGroupNode, linkedBrushNode2}));
        CHECK(groupNode->linkId() == linkedGroupNode->linkId());
        CHECK(linkedGroupNode2->linkId() != groupNode->linkId());
        CHECK(linkedBrushNode2->linkId() != brushNode->linkId());
      }

      SECTION(
        "Given three linked groups, we ungroup two of them, and the remaining one keeps "
        "its "
        "ID")
      {
        selectNodes(map, {linkedGroupNode, linkedGroupNode2});

        ungroupSelectedNodes(map);
        CHECK_THAT(
          map.world()->defaultLayer()->children(),
          Catch::UnorderedEquals(
            std::vector<Node*>{groupNode, linkedBrushNode, linkedBrushNode2}));

        CHECK(groupNode->linkId() == originalGroupLinkId);
        CHECK(linkedGroupNode->linkId() != originalGroupLinkId);
        CHECK(linkedGroupNode2->linkId() != originalGroupLinkId);
        CHECK(linkedGroupNode2->linkId() != linkedGroupNode->linkId());

        CHECK(linkedBrushNode->linkId() != brushNode->linkId());
        CHECK(linkedBrushNode2->linkId() != brushNode->linkId());
        CHECK(linkedBrushNode2->linkId() != linkedBrushNode->linkId());
      }

      SECTION("Given three linked groups, we ungroup all of them")
      {
        selectNodes(map, {groupNode});
        selectNodes(map, {linkedGroupNode});
        selectNodes(map, {linkedGroupNode2});

        ungroupSelectedNodes(map);
        CHECK_THAT(
          map.world()->defaultLayer()->children(),
          Catch::UnorderedEquals(
            std::vector<Node*>{brushNode, linkedBrushNode, linkedBrushNode2}));

        CHECK(groupNode->linkId() != originalGroupLinkId);
        CHECK(linkedGroupNode->linkId() != originalGroupLinkId);
        CHECK(linkedGroupNode2->linkId() != originalGroupLinkId);

        CHECK(linkedGroupNode->linkId() != groupNode->linkId());
        CHECK(linkedGroupNode2->linkId() != groupNode->linkId());
        CHECK(linkedGroupNode2->linkId() != linkedGroupNode->linkId());
      }

      map.undoCommand();
      CHECK_THAT(
        map.world()->defaultLayer()->children(),
        Catch::UnorderedEquals(
          std::vector<Node*>{groupNode, linkedGroupNode, linkedGroupNode2}));
      CHECK(groupNode->linkId() == originalGroupLinkId);
      CHECK(linkedGroupNode->linkId() == originalGroupLinkId);
      CHECK(linkedGroupNode2->linkId() == originalGroupLinkId);

      CHECK(brushNode->linkId() == originalBrushLinkId);
      CHECK(linkedBrushNode->linkId() == originalBrushLinkId);
      CHECK(linkedBrushNode2->linkId() == originalBrushLinkId);
    }
  }

  SECTION("mergeSelectedGroupsWithGroup")
  {
    auto* entityNode1 = new EntityNode{Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode1}}});
    deselectAll(map);
    selectNodes(map, {entityNode1});
    auto* groupNode1 = groupSelectedNodes(map, "group1");

    auto* entityNode2 = new EntityNode{Entity{}};
    addNodes(map, {{parentForNodes(map), {entityNode2}}});
    deselectAll(map);
    selectNodes(map, {entityNode2});
    auto* groupNode2 = groupSelectedNodes(map, "group2");

    CHECK_THAT(
      map.editorContext().currentLayer()->children(),
      Catch::UnorderedEquals(std::vector<Node*>{groupNode1, groupNode2}));

    selectNodes(map, {groupNode1, groupNode2});
    mergeSelectedGroupsWithGroup(map, groupNode2);

    CHECK_THAT(map.selection().nodes, Catch::Equals(std::vector<Node*>{groupNode2}));
    CHECK_THAT(
      map.editorContext().currentLayer()->children(),
      Catch::Equals(std::vector<Node*>{groupNode2}));

    CHECK_THAT(groupNode1->children(), Catch::UnorderedEquals(std::vector<Node*>{}));
    CHECK_THAT(
      groupNode2->children(),
      Catch::UnorderedEquals(std::vector<Node*>{entityNode1, entityNode2}));
  }

  SECTION("renameSelectedGroups")
  {
    auto* brushNode1 = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode1}}});
    selectNodes(map, {brushNode1});

    auto* groupNode = groupSelectedNodes(map, "test");

    renameSelectedGroups(map, "abc");
    CHECK(groupNode->name() == "abc");

    map.undoCommand();
    CHECK(groupNode->name() == "test");

    map.redoCommand();
    CHECK(groupNode->name() == "abc");
  }

  SECTION("createdLinkedDuplicate")
  {
    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});
    selectNodes(map, {brushNode});

    auto* groupNode = groupSelectedNodes(map, "test");
    REQUIRE(groupNode != nullptr);

    deselectAll(map);

    CHECK_FALSE(canCreateLinkedDuplicate(map));
    CHECK(createLinkedDuplicate(map) == nullptr);

    selectNodes(map, {groupNode});
    CHECK(canCreateLinkedDuplicate(map));

    auto* linkedGroupNode = createLinkedDuplicate(map);
    CHECK_THAT(*linkedGroupNode, MatchesNode(*groupNode));
  }

  SECTION("separateSelectedLinkedGroups")
  {
    auto* brushNode = createBrushNode(map);
    addNodes(map, {{parentForNodes(map), {brushNode}}});
    selectNodes(map, {brushNode});

    auto* groupNode = groupSelectedNodes(map, "test");
    REQUIRE(groupNode != nullptr);

    deselectAll(map);
    selectNodes(map, {groupNode});

    const auto originalGroupLinkId = groupNode->linkId();
    const auto originalBrushLinkId = brushNode->linkId();

    SECTION("Separating a group that isn't linked")
    {
      CHECK_FALSE(canSeparateSelectedLinkedGroups(map));
    }

    SECTION("Separating all members of a link set")
    {
      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

      selectNodes(map, {groupNode, linkedGroupNode});
      CHECK_FALSE(canSeparateSelectedLinkedGroups(map));
    }

    SECTION("Separating one group from a link set with two members")
    {
      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

      auto* linkedBrushNode =
        dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
      REQUIRE(linkedBrushNode != nullptr);

      deselectAll(map);
      selectNodes(map, {linkedGroupNode});

      CHECK(canSeparateSelectedLinkedGroups(map));
      separateSelectedLinkedGroups(map);
      CHECK(groupNode->linkId() == originalGroupLinkId);
      CHECK(brushNode->linkId() == originalBrushLinkId);
      CHECK(linkedGroupNode->linkId() != originalGroupLinkId);
      CHECK(linkedBrushNode->linkId() != originalBrushLinkId);

      map.undoCommand();
      CHECK(groupNode->linkId() == originalGroupLinkId);
      CHECK(linkedGroupNode->linkId() == originalGroupLinkId);
      CHECK(brushNode->linkId() == originalBrushLinkId);
      CHECK(linkedBrushNode->linkId() == originalBrushLinkId);
    }

    SECTION("Separating multiple groups from a link set with several members")
    {
      auto* linkedGroupNode1 = createLinkedDuplicate(map);
      auto* linkedGroupNode2 = createLinkedDuplicate(map);
      auto* linkedGroupNode3 = createLinkedDuplicate(map);

      REQUIRE_THAT(*linkedGroupNode1, MatchesNode(*groupNode));
      REQUIRE_THAT(*linkedGroupNode2, MatchesNode(*groupNode));
      REQUIRE_THAT(*linkedGroupNode3, MatchesNode(*groupNode));

      auto* linkedBrushNode1 =
        dynamic_cast<BrushNode*>(linkedGroupNode1->children().front());
      auto* linkedBrushNode2 =
        dynamic_cast<BrushNode*>(linkedGroupNode2->children().front());
      auto* linkedBrushNode3 =
        dynamic_cast<BrushNode*>(linkedGroupNode3->children().front());

      deselectAll(map);
      selectNodes(map, {linkedGroupNode2, linkedGroupNode3});
      CHECK(canSeparateSelectedLinkedGroups(map));

      separateSelectedLinkedGroups(map);
      CHECK(groupNode->linkId() == originalGroupLinkId);
      CHECK(linkedGroupNode1->linkId() == originalGroupLinkId);

      CHECK(linkedGroupNode2->linkId() != originalGroupLinkId);
      CHECK(linkedGroupNode3->linkId() == linkedGroupNode2->linkId());

      CHECK(linkedBrushNode2->linkId() != originalBrushLinkId);
      CHECK(linkedBrushNode3->linkId() == linkedBrushNode2->linkId());

      CHECK(map.selection().groups.size() == 2u);

      map.undoCommand();

      CHECK(groupNode->linkId() == originalGroupLinkId);
      CHECK(linkedGroupNode1->linkId() == originalGroupLinkId);
      CHECK(linkedGroupNode2->linkId() == originalGroupLinkId);
      CHECK(linkedGroupNode3->linkId() == originalGroupLinkId);

      CHECK(brushNode->linkId() == originalBrushLinkId);
      CHECK(linkedBrushNode1->linkId() == originalBrushLinkId);
      CHECK(linkedBrushNode2->linkId() == originalBrushLinkId);
      CHECK(linkedBrushNode3->linkId() == originalBrushLinkId);
    }

    SECTION("Nested linked groups")
    {
      /*
       * groupNode
       *   brushNode
       *   nestedGroupNode
       *     nestedEntityNode
       *   nestedLinkedGroupNode
       *     nestedLinkedEntityNode
       * linkedOuterGroupNode
       *   linkedBrushNode
       *   linkedNestedGroupNode
       *     linkedNestedEntityNode
       *   linkedNestedLinkedGroupNode
       *     linkedNestedLinkedEntityNode
       */

      auto* nestedGroupNode = new GroupNode{Group{"nestedGroupNode"}};
      auto* nestedEntityNode = new EntityNode{Entity{}};
      nestedGroupNode->addChild(nestedEntityNode);
      addNodes(map, {{groupNode, {nestedGroupNode}}});

      openGroup(map, groupNode);
      deselectAll(map);
      selectNodes(map, {nestedGroupNode});

      auto* nestedLinkedGroupNode = createLinkedDuplicate(map);
      REQUIRE_THAT(*nestedLinkedGroupNode, MatchesNode(*nestedGroupNode));

      deselectAll(map);
      closeGroup(map);

      selectNodes(map, {groupNode});
      auto* linkedGroupNode = createLinkedDuplicate(map);
      REQUIRE_THAT(*linkedGroupNode, MatchesNode(*groupNode));

      const auto [linkedBrushNode, linkedNestedGroupNode, linkedNestedLinkedGroupNode] =
        getChildrenAs<BrushNode, GroupNode, GroupNode>(*linkedGroupNode);

      deselectAll(map);

      SECTION("Separating linked groups with nested linked groups inside")
      {
        selectNodes(map, {groupNode});
        separateSelectedLinkedGroups(map);

        // The outer groups where separated
        CHECK(groupNode->linkId() != linkedGroupNode->linkId());
        CHECK(brushNode->linkId() != linkedBrushNode->linkId());

        // But the nested group nodes are still all linked to each other
        CHECK(linkedNestedGroupNode->linkId() == nestedGroupNode->linkId());
        CHECK(nestedGroupNode->linkId() == nestedLinkedGroupNode->linkId());
        CHECK(linkedNestedGroupNode->linkId() == linkedNestedLinkedGroupNode->linkId());
      }

      SECTION("Separating linked groups nested inside a linked group")
      {
        openGroup(map, groupNode);
        selectNodes(map, {nestedLinkedGroupNode});
        separateSelectedLinkedGroups(map);

        REQUIRE(nestedGroupNode->linkId() != nestedLinkedGroupNode->linkId());

        deselectAll(map);
        closeGroup(map);

        // the change was propagated to linkedGroupNode:
        CHECK_THAT(*linkedGroupNode, MatchesNode(*groupNode));
      }
    }
  }

  SECTION("canUpdateLinkedGroups")
  {
    auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"inner"}};
    auto* entityNode = new mdl::EntityNode{mdl::Entity{}};
    innerGroupNode->addChild(entityNode);

    auto* linkedInnerGroupNode =
      static_cast<mdl::GroupNode*>(innerGroupNode->cloneRecursively(map.worldBounds()));

    auto* linkedEntityNode =
      dynamic_cast<mdl::EntityNode*>(linkedInnerGroupNode->children().front());
    REQUIRE(linkedEntityNode != nullptr);

    auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"outer"}};
    outerGroupNode->addChildren({innerGroupNode, linkedInnerGroupNode});

    addNodes(map, {{parentForNodes(map), {outerGroupNode}}});
    selectNodes(map, {outerGroupNode});

    const auto entityNodes = map.selection().allEntities();
    REQUIRE_THAT(
      entityNodes,
      Catch::UnorderedEquals(
        std::vector<mdl::EntityNodeBase*>{entityNode, linkedEntityNode}));

    CHECK(canUpdateLinkedGroups({entityNode}));
    CHECK(canUpdateLinkedGroups({linkedEntityNode}));
    CHECK_FALSE(canUpdateLinkedGroups(kdl::vec_static_cast<mdl::Node*>(entityNodes)));
  }
}

} // namespace tb::mdl
