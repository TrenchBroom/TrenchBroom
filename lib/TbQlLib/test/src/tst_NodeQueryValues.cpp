/*
 Copyright (C) 2026 Kristian Duske

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

#include "el/Value.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Tag.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep
#include "ql/NodeQueryValues.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("NodeQueryValues")
{
  // EntityClassNameTagMatcher tags geometry *owned by* a matching entity
  auto fixtureConfig = mdl::MapFixtureConfig{};
  fixtureConfig.gameInfo.gameConfig.smartTags = {
    mdl::SmartTag{
      "Detail",
      {},
      std::make_unique<mdl::EntityClassNameTagMatcher>("func_detail", ""),
    },
    mdl::SmartTag{
      "Trigger",
      {},
      std::make_unique<mdl::EntityClassNameTagMatcher>("trigger*", ""),
    },
  };

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create(fixtureConfig);

  SECTION("layerNameValue")
  {
    CHECK(layerNameValue(map.worldNode()) == el::Value::Undefined);

    auto* layerNode = new mdl::LayerNode{mdl::Layer{"Combat"}};
    mdl::addNodes(map, {{&map.worldNode(), {layerNode}}});

    // findContainingLayer treats a layer as containing itself
    CHECK(layerNameValue(*layerNode) == el::Value{"Combat"});

    auto* brushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{layerNode, {brushNode}}});
    CHECK(layerNameValue(*brushNode) == el::Value{"Combat"});
  }

  SECTION("groupNameValue")
  {
    auto* groupNode = new mdl::GroupNode{mdl::Group{"Hallway"}};
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {groupNode}}});

    // findContainingGroup looks at ancestors, not the group node itself
    CHECK(groupNameValue(*groupNode) == el::Value::Undefined);
    CHECK(groupNameValue(map.worldNode()) == el::Value::Undefined);

    auto* brushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{groupNode, {brushNode}}});
    CHECK(groupNameValue(*brushNode) == el::Value{"Hallway"});
  }

  SECTION("tagsValue")
  {
    auto* detailEntityNode =
      new mdl::EntityNode{mdl::Entity{{{"classname", "func_detail"}}}};
    auto* triggerEntityNode =
      new mdl::EntityNode{mdl::Entity{{{"classname", "trigger_once"}}}};
    mdl::addNodes(
      map, {{&mdl::parentForNodes(map), {detailEntityNode, triggerEntityNode}}});

    auto* detailBrushNode = mdl::createBrushNode(map);
    auto* triggerBrushNode = mdl::createBrushNode(map);
    auto* worldBrushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{detailEntityNode, {detailBrushNode}}});
    mdl::addNodes(map, {{triggerEntityNode, {triggerBrushNode}}});
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {worldBrushNode}}});

    // only the smart tags a node actually has are listed
    CHECK(
      tagsValue(map, *detailBrushNode) == el::Value{el::ArrayType{el::Value{"Detail"}}});
    CHECK(
      tagsValue(map, *triggerBrushNode)
      == el::Value{el::ArrayType{el::Value{"Trigger"}}});
    CHECK(tagsValue(map, *worldBrushNode) == el::Value{el::ArrayType{}});
  }
}

} // namespace tb::ql
