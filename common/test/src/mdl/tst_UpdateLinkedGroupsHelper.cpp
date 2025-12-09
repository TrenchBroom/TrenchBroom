/*
 Copyright (C) 2020 Kristian Duske

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

#include "TestFactory.h"
#include "TestUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/UpdateLinkedGroupsHelper.h"
#include "mdl/WorldNode.h"

#include "kd/overload.h"

#include <algorithm>

#include "catch/CatchConfig.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::mdl
{
using namespace Catch::Matchers;

namespace
{
class TestNode : public EntityNode
{
private:
  bool& m_deleted;

public:
  TestNode(Entity entity, bool& deleted)
    : EntityNode{std::move(entity)}
    , m_deleted{deleted}
  {
    m_deleted = false;
  }

  ~TestNode() override { m_deleted = true; };
};


void setGroupName(GroupNode& groupNode, const std::string& name)
{
  auto group = groupNode.group();
  group.setName(name);
  groupNode.setGroup(std::move(group));
}

GroupNode* findGroupByName(Node& node, const std::string& name)
{
  const auto visitChildren = [&](auto&& lambda, Node* n) -> GroupNode* {
    for (auto* child : n->children())
    {
      if (auto* result = child->accept(lambda))
      {
        return result;
      }
    }
    return nullptr;
  };
  return node.accept(kdl::overload(
    [&](auto&& thisLambda, WorldNode* worldNode) -> GroupNode* {
      return visitChildren(thisLambda, worldNode);
    },
    [&](auto&& thisLambda, LayerNode* layerNode) -> GroupNode* {
      return visitChildren(thisLambda, layerNode);
    },
    [&](auto&& thisLambda, GroupNode* groupNode) -> GroupNode* {
      if (groupNode->name() == name)
      {
        return groupNode;
      }
      return visitChildren(thisLambda, groupNode);
    },
    [](EntityNode*) -> GroupNode* { return nullptr; },
    [](BrushNode*) -> GroupNode* { return nullptr; },
    [](PatchNode*) -> GroupNode* { return nullptr; }));
}

} // namespace

TEST_CASE("checkLinkedGroupsToUpdate")
{
  auto groupNode1 = GroupNode{Group{"test"}};
  auto linkedGroupNode = GroupNode{Group{"test"}};
  setLinkId(groupNode1, "asdf");
  setLinkId(linkedGroupNode, "asdf");

  auto groupNode2 = GroupNode{Group{"test"}};
  setLinkId(groupNode2, "fdsa");

  CHECK(checkLinkedGroupsToUpdate({}));
  CHECK(checkLinkedGroupsToUpdate({&groupNode1}));
  CHECK(checkLinkedGroupsToUpdate({&groupNode1, &groupNode2}));
  CHECK(checkLinkedGroupsToUpdate({&linkedGroupNode, &groupNode2}));
  CHECK_FALSE(checkLinkedGroupsToUpdate({&groupNode1, &linkedGroupNode}));
}

TEST_CASE("UpdateLinkedGroupsHelper")
{
  bool deleted = false;

  auto fixture = MapFixture{};
  auto& map = fixture.create();

  SECTION("Ownership")
  {
    auto* groupNode = new GroupNode{Group{""}};
    setLinkId(*groupNode, "asdf");

    auto* entityNode = new TestNode{Entity{}, deleted};
    groupNode->addChild(entityNode);

    auto* linkedNode =
      static_cast<GroupNode*>(groupNode->cloneRecursively(map.worldBounds()));

    addNodes(map, {{parentForNodes(map), {groupNode, linkedNode}}});

    SECTION("Helper takes ownership of replaced child nodes")
    {
      {
        auto helper = UpdateLinkedGroupsHelper{{linkedNode}};
        REQUIRE(helper.applyLinkedGroupUpdates(map));
      }
      CHECK(deleted);
    }

    SECTION("Helper relinquishes ownership of replaced child nodes when undoing updates")
    {
      {
        auto helper = UpdateLinkedGroupsHelper{{linkedNode}};
        REQUIRE(helper.applyLinkedGroupUpdates(map));
        helper.undoLinkedGroupUpdates(map);
      }
      CHECK_FALSE(deleted);
    }
  }

  SECTION("applyLinkedGroupUpdates")
  {
    SECTION("Sibling linked groups")
    {
      auto* groupNode = new GroupNode{Group{"test"}};
      setLinkId(*groupNode, "asdf");

      auto* brushNode = createBrushNode(map);
      groupNode->addChild(brushNode);

      auto* linkedGroupNode =
        static_cast<GroupNode*>(groupNode->cloneRecursively(map.worldBounds()));

      REQUIRE(linkedGroupNode->children().size() == 1u);
      auto* linkedBrushNode =
        dynamic_cast<BrushNode*>(linkedGroupNode->children().front());
      REQUIRE(linkedBrushNode != nullptr);

      transformNode(
        *linkedGroupNode,
        vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)),
        map.worldBounds());
      REQUIRE(
        linkedBrushNode->physicalBounds()
        == brushNode->physicalBounds().translate(vm::vec3d(32.0, 0.0, 0.0)));

      addNodes(map, {{parentForNodes(map), {groupNode, linkedGroupNode}}});

      /*
      world
      +-defaultLayer
        +-groupNode
          +-brushNode
        +-linkedGroupNode (translated 32 0 0)
          +-linkedBrushNode (translated 32 0 0)
      */

      const auto originalBrushBounds = brushNode->physicalBounds();

      transformNode(
        *brushNode, vm::translation_matrix(vm::vec3d(0.0, 16.0, 0.0)), map.worldBounds());
      REQUIRE(
        brushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(0.0, 16.0, 0.0)));

      /*
      world
      +-defaultLayer
        +-groupNode
          +-brushNode (translated 0 16 0)
        +-linkedGroupNode (translated 32 0 0)
          +-linkedBrushNode (translated 32 0 0)
      */

      // propagate changes
      auto helper = UpdateLinkedGroupsHelper{{groupNode}};
      REQUIRE(helper.applyLinkedGroupUpdates(map));

      /*
      world
      +-defaultLayer
        +-groupNode
          +-brushNode (translated 0 16 0)
        +-linkedGroupNode (translated 32 0 0)
          +-newLinkedBrushNode (translated 32 16 0)
      */

      // changes were propagated
      REQUIRE(linkedGroupNode->childCount() == 1u);
      CHECK(linkedBrushNode->parent() == nullptr);
      auto* newLinkedBrushNode = linkedGroupNode->children().front();
      CHECK(
        newLinkedBrushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(32.0, 16.0, 0.0)));

      // undo change propagation
      helper.undoLinkedGroupUpdates(map);

      /*
      world
      +-defaultLayer
        +-groupNode
          +-brushNode (translated 0 16 0)
        +-linkedGroupNode (translated 32 0 0)
          +-linkedBrushNode (translated 32 0 0)
      */

      REQUIRE(linkedGroupNode->childCount() == 1u);
      CHECK_THAT(
        linkedGroupNode->children(), Equals(std::vector<Node*>{linkedBrushNode}));
      CHECK(linkedBrushNode->parent() == linkedGroupNode);
      CHECK(
        linkedBrushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(32.0, 0.0, 0.0)));
    }

    SECTION("Nested linked groups")
    {
      auto* outerGroupNode = new GroupNode{Group{"outerGroupNode"}};
      setLinkId(*outerGroupNode, "outerGroupNode");

      auto* innerGroupNode = new GroupNode{Group{"innerGroupNode"}};
      setLinkId(*innerGroupNode, "innerGroupNode");

      auto* brushNode = createBrushNode(map);
      innerGroupNode->addChild(brushNode);
      outerGroupNode->addChild(innerGroupNode);

      addNodes(map, {{parentForNodes(map), {outerGroupNode}}});

      // create a linked group of the inner group node so that cloning the outer group
      // node will create a linked clone of the inner group node
      auto* linkedInnerGroupNode =
        static_cast<GroupNode*>(innerGroupNode->cloneRecursively(map.worldBounds()));
      setGroupName(*linkedInnerGroupNode, "linkedInnerGroupNode");
      REQUIRE(linkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      auto* linkedOuterGroupNode =
        static_cast<GroupNode*>(outerGroupNode->cloneRecursively(map.worldBounds()));
      setGroupName(*linkedOuterGroupNode, "linkedOuterGroupNode");
      REQUIRE(linkedOuterGroupNode->linkId() == outerGroupNode->linkId());

      auto* nestedLinkedInnerGroupNode =
        static_cast<GroupNode*>(linkedOuterGroupNode->children().front());
      setGroupName(*nestedLinkedInnerGroupNode, "nestedLinkedInnerGroupNode");
      REQUIRE(nestedLinkedInnerGroupNode->linkId() == innerGroupNode->linkId());

      addNodes(
        map, {{parentForNodes(map), {linkedInnerGroupNode, linkedOuterGroupNode}}});

      /*
      world
      +-defaultLayer
        +-outerGroupNode--------+
          +-innerGroupNode------|-------+
            +-brushNode         |       |
        +-linkedInnerGroupNode--|-------+
          +-linkedBrushNode     |       |
        +-linkedOuterGroupNode--+       |
          +-nestedLinkedInnerGroupNode--+
            +-nestedLinkedBrushNode
      */

      const auto originalBrushBounds = brushNode->physicalBounds();

      transformNode(
        *linkedOuterGroupNode,
        vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)),
        map.worldBounds());

      REQUIRE(
        linkedOuterGroupNode->group().transformation()
        == vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)));
      REQUIRE(
        linkedOuterGroupNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(32.0, 0.0, 0.0)));

      REQUIRE(
        nestedLinkedInnerGroupNode->group().transformation()
        == vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)));
      REQUIRE(
        nestedLinkedInnerGroupNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(32.0, 0.0, 0.0)));

      auto* nestedLinkedBrushNode = nestedLinkedInnerGroupNode->children().front();
      REQUIRE(
        nestedLinkedBrushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(32.0, 0.0, 0.0)));

      /*
      world
      +-defaultLayer
        +-outerGroupNode
          +-innerGroupNode
            +-brushNode
        +-linkedInnerGroupNode
          +-linkedBrushNode
        +-linkedOuterGroupNode (translated 32 0 0)
          +-nestedLinkedInnerGroupNode (translated 32 0 0)
            +-nestedLinkedBrushNode (translated 32 0 0)
      */

      transformNode(
        *innerGroupNode,
        vm::translation_matrix(vm::vec3d(0.0, 16.0, 0.0)),
        map.worldBounds());

      REQUIRE(
        innerGroupNode->group().transformation()
        == vm::translation_matrix(vm::vec3d(0.0, 16.0, 0.0)));
      REQUIRE(
        innerGroupNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(0.0, 16.0, 0.0)));
      REQUIRE(
        brushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(0.0, 16.0, 0.0)));

      /*
      world
      +-defaultLayer
        +-outerGroupNode
          +-innerGroupNode (translated 0 16 0)
            +-brushNode (translated 0 16 0)
        +-linkedInnerGroupNode
          +-linkedBrushNode
        +-linkedOuterGroupNode (translated 32 0 0)
          +-nestedLinkedInnerGroupNode (translated 32 0 0)
            +-nestedLinkedBrushNode (translated 32 0 0)
      */

      transformNode(
        *brushNode, vm::translation_matrix(vm::vec3d(0.0, 0.0, 8.0)), map.worldBounds());

      REQUIRE(
        brushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(0.0, 16.0, 8.0)));

      /*
      world
      +-defaultLayer
        +-outerGroupNode
          +-innerGroupNode (translated 0 16 0)
            +-brushNode (translated 0 16 8)
        +-linkedInnerGroupNode
          +-linkedBrushNode
        +-linkedOuterGroupNode (translated 32 0 0)
          +-nestedLinkedInnerGroupNode (translated 32 0 0)
            +-nestedLinkedBrushNode (translated 32 0 0)
      */

      SECTION("First propagate changes to innerGroupNode, then outerGroupNode")
      {
        auto helper1 = UpdateLinkedGroupsHelper{{innerGroupNode}};
        CHECK(helper1.applyLinkedGroupUpdates(map));

        /*
        world
        +-defaultLayer
          +-outerGroupNode
            +-innerGroupNode (translated 0 16 0)
              +-brushNode (translated 0 16 8)
          +-linkedInnerGroupNode
            +-newLinkedBrushNode (translated 0 0 8)
          +-linkedOuterGroupNode (translated 32 0 0)
            +-nestedLinkedInnerGroupNode (translated 32 0 0)
              +-newNestedLinkedBrushNode (translated 32 0 8)
        */

        REQUIRE(linkedInnerGroupNode->childCount() == 1u);

        auto* newLinkedBrushNode = linkedInnerGroupNode->children().front();
        CHECK(
          newLinkedBrushNode->physicalBounds()
          == originalBrushBounds.translate(vm::vec3d(0.0, 0.0, 8.0)));

        REQUIRE(nestedLinkedInnerGroupNode != nullptr);
        CHECK(
          nestedLinkedInnerGroupNode->group().transformation()
          == vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)));
        CHECK(
          nestedLinkedInnerGroupNode->physicalBounds()
          == originalBrushBounds.translate(vm::vec3d(32.0, 0.0, 8.0)));
        REQUIRE(nestedLinkedInnerGroupNode->childCount() == 1u);

        auto* newNestedLinkedBrushNode = nestedLinkedInnerGroupNode->children().front();
        CHECK(
          newNestedLinkedBrushNode->physicalBounds()
          == originalBrushBounds.translate(vm::vec3d(32.0, 0.0, 8.0)));

        auto helper2 = UpdateLinkedGroupsHelper{{outerGroupNode}};
        CHECK(helper2.applyLinkedGroupUpdates(map));

        // see end of test for assertions of final state
      }

      SECTION("First propagate changes to outerGroupNode, then innerGroupNode")
      {
        auto helper1 = UpdateLinkedGroupsHelper{{outerGroupNode}};
        REQUIRE(helper1.applyLinkedGroupUpdates(map));

        /*
        world
        +-defaultLayer
          +-outerGroupNode
            +-innerGroupNode (translated 0 16 0)
              +-brushNode (translated 0 16 8)
          +-linkedInnerGroupNode
            +-linkedBrushNode
          +-linkedOuterGroupNode (translated 32 0 0)
            +-newNestedLinkedInnerGroupNode (translated 32 16 0)
              +-newNestedLinkedBrushNode (translated 32 16 8)
        */

        CHECK(
          linkedOuterGroupNode->group().transformation()
          == vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)));

        auto* newNestedLinkedInnerGroupNode =
          findGroupByName(map.world(), "nestedLinkedInnerGroupNode");
        CHECK(
          newNestedLinkedInnerGroupNode->group().transformation()
          == vm::translation_matrix(vm::vec3d(32.0, 16.0, 0.0)));
        CHECK(
          newNestedLinkedInnerGroupNode->physicalBounds()
          == originalBrushBounds.translate(vm::vec3d(32.0, 16.0, 8.0)));
        REQUIRE(newNestedLinkedInnerGroupNode->childCount() == 1u);

        auto* newNestedLinkedBrushNode =
          newNestedLinkedInnerGroupNode->children().front();
        CHECK(
          newNestedLinkedBrushNode->physicalBounds()
          == originalBrushBounds.translate(vm::vec3d(32.0, 16.0, 8.0)));

        auto helper2 = UpdateLinkedGroupsHelper{{innerGroupNode}};
        REQUIRE(helper2.applyLinkedGroupUpdates(map));

        // see end of test for assertions of final state
      }

      SECTION("Propagate both changes at once")
      {
        auto groupNodes = std::vector<GroupNode*>{outerGroupNode, innerGroupNode};
        std::ranges::sort(groupNodes);

        // The following code generates both permutations of the group nodes
        const auto permute = GENERATE(true, false);
        if (permute)
        {
          std::next_permutation(std::begin(groupNodes), std::end(groupNodes));
        }

        auto helper = UpdateLinkedGroupsHelper{groupNodes};
        REQUIRE(helper.applyLinkedGroupUpdates(map));
      }

      /*
      world
      +-defaultLayer
        +-outerGroupNode
          +-innerGroupNode (translated 0 16 0)
            +-brushNode (translated 0 16 8)
        +-linkedInnerGroupNode
          +-newLinkedBrushNode (translated 0 0 8)
        +-linkedOuterGroupNode (translated 32 0 0)
          +-newNestedLinkedInnerGroupNode (translated 32 16 8)
            +-newLinkedBrushNode (translated 32 16 8)
      */

      REQUIRE(linkedInnerGroupNode->childCount() == 1u);

      auto* newLinkedBrushNode = linkedInnerGroupNode->children().front();
      CHECK(
        newLinkedBrushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(0.0, 0.0, 8.0)));

      CHECK(
        linkedOuterGroupNode->group().transformation()
        == vm::translation_matrix(vm::vec3d(32.0, 0.0, 0.0)));

      auto* newNestedLinkedInnerGroupNode =
        findGroupByName(map.world(), "nestedLinkedInnerGroupNode");
      REQUIRE(newNestedLinkedInnerGroupNode != nullptr);
      CHECK(
        newNestedLinkedInnerGroupNode->group().transformation()
        == vm::translation_matrix(vm::vec3d(32.0, 16.0, 0.0)));
      CHECK(
        newNestedLinkedInnerGroupNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(32.0, 16.0, 8.0)));
      REQUIRE(newNestedLinkedInnerGroupNode->childCount() == 1u);

      auto* newNestedLinkedBrushNode = newNestedLinkedInnerGroupNode->children().front();
      CHECK(
        newNestedLinkedBrushNode->physicalBounds()
        == originalBrushBounds.translate(vm::vec3d(32.0, 16.0, 8.0)));
    }
  }
}

} // namespace tb::mdl
