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

#include "mdl/BezierPatch.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/TestUtils.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("GroupNode")
{
  SECTION("open and close")
  {
    auto grandParentGroupNode = GroupNode{Group{"grandparent"}};
    auto* parentGroupNode = new GroupNode{Group{"parent"}};
    auto* groupNode = new GroupNode{Group{"group"}};
    auto* childGroupNode = new GroupNode{Group{"child"}};

    grandParentGroupNode.addChild(parentGroupNode);
    parentGroupNode->addChild(groupNode);
    groupNode->addChild(childGroupNode);

    REQUIRE_FALSE(grandParentGroupNode.opened());
    REQUIRE(grandParentGroupNode.closed());
    REQUIRE_FALSE(parentGroupNode->opened());
    REQUIRE(parentGroupNode->closed());
    REQUIRE_FALSE(groupNode->opened());
    REQUIRE(groupNode->closed());
    REQUIRE_FALSE(childGroupNode->opened());
    REQUIRE(childGroupNode->closed());

    REQUIRE_FALSE(grandParentGroupNode.hasOpenedDescendant());
    REQUIRE_FALSE(parentGroupNode->hasOpenedDescendant());
    REQUIRE_FALSE(groupNode->hasOpenedDescendant());
    REQUIRE_FALSE(childGroupNode->hasOpenedDescendant());

    groupNode->open();
    CHECK(!grandParentGroupNode.opened());
    CHECK(!grandParentGroupNode.closed());
    CHECK(!parentGroupNode->opened());
    CHECK(!parentGroupNode->closed());
    CHECK(groupNode->opened());
    CHECK(!groupNode->closed());
    CHECK(!childGroupNode->opened());
    CHECK(childGroupNode->closed());

    CHECK(grandParentGroupNode.hasOpenedDescendant());
    CHECK(parentGroupNode->hasOpenedDescendant());
    CHECK(!groupNode->hasOpenedDescendant());
    CHECK(!childGroupNode->hasOpenedDescendant());

    groupNode->close();
    CHECK(!grandParentGroupNode.opened());
    CHECK(grandParentGroupNode.closed());
    CHECK(!parentGroupNode->opened());
    CHECK(parentGroupNode->closed());
    CHECK(!groupNode->opened());
    CHECK(groupNode->closed());
    CHECK(!childGroupNode->opened());
    CHECK(childGroupNode->closed());

    CHECK(!grandParentGroupNode.hasOpenedDescendant());
    CHECK(!parentGroupNode->hasOpenedDescendant());
    CHECK(!groupNode->hasOpenedDescendant());
    CHECK(!childGroupNode->hasOpenedDescendant());
  }

  SECTION("canAddChild")
  {
    constexpr auto worldBounds = vm::bbox3d{8192.0};
    constexpr auto mapFormat = MapFormat::Quake3;

    auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK(!groupNode.canAddChild(worldNode));
    CHECK(!groupNode.canAddChild(layerNode));
    CHECK(!groupNode.canAddChild(groupNode));
    CHECK(groupNode.canAddChild(entityNode));
    CHECK(groupNode.canAddChild(brushNode));
    CHECK(groupNode.canAddChild(patchNode));

    SECTION("Recursive linked groups")
    {
      auto linkedGroupNode = std::make_unique<GroupNode>(Group{"group"});
      setLinkId(groupNode, "linked_group_id");
      setLinkId(*linkedGroupNode, groupNode.linkId());
      CHECK(!groupNode.canAddChild(*linkedGroupNode));

      auto outerGroupNode = GroupNode{Group{"outer_group"}};
      outerGroupNode.addChild(linkedGroupNode.release());
      CHECK(!groupNode.canAddChild(outerGroupNode));
    }
  }

  SECTION("canRemoveChild")
  {
    constexpr auto worldBounds = vm::bbox3d{8192.0};
    constexpr auto mapFormat = MapFormat::Quake3;

    const auto worldNode = WorldNode{{}, {}, mapFormat};
    auto layerNode = LayerNode{Layer{"layer"}};
    auto groupNode = GroupNode{Group{"group"}};
    auto entityNode = EntityNode{Entity{}};
    auto brushNode = BrushNode{
      BrushBuilder{mapFormat, worldBounds}.createCube(64.0, "material") | kdl::value()};

    // clang-format off
    auto patchNode = PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "material"}};
    // clang-format on

    CHECK(groupNode.canRemoveChild(worldNode));
    CHECK(groupNode.canRemoveChild(layerNode));
    CHECK(groupNode.canRemoveChild(groupNode));
    CHECK(groupNode.canRemoveChild(entityNode));
    CHECK(groupNode.canRemoveChild(brushNode));
    CHECK(groupNode.canRemoveChild(patchNode));
  }
}

} // namespace tb::mdl
