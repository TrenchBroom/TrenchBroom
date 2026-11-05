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

#include "kd/vector_utils.h"

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

    auto* layerNode = mdl::addNode(
      map, map.worldNode(), std::make_unique<mdl::LayerNode>(mdl::Layer{"Combat"}));

    // findContainingLayer treats a layer as containing itself
    CHECK(layerNameValue(*layerNode) == el::Value{"Combat"});

    auto* brushNode = mdl::addNode(map, *layerNode, mdl::createBrushNode(map));
    CHECK(layerNameValue(*brushNode) == el::Value{"Combat"});
  }

  SECTION("groupNameValue")
  {
    auto* groupNode = mdl::addNode(
      map,
      mdl::parentForNodes(map),
      std::make_unique<mdl::GroupNode>(mdl::Group{"Hallway"}));

    // findContainingGroup looks at ancestors, not the group node itself
    CHECK(groupNameValue(*groupNode) == el::Value::Undefined);
    CHECK(groupNameValue(map.worldNode()) == el::Value::Undefined);

    auto* brushNode = mdl::addNode(map, *groupNode, mdl::createBrushNode(map));
    CHECK(groupNameValue(*brushNode) == el::Value{"Hallway"});
  }

  SECTION("tagsValue")
  {
    auto detailEntityNodePtr =
      std::make_unique<mdl::EntityNode>(mdl::Entity{{{"classname", "func_detail"}}});
    auto* detailEntityNode = detailEntityNodePtr.get();
    auto triggerEntityNodePtr =
      std::make_unique<mdl::EntityNode>(mdl::Entity{{{"classname", "trigger_once"}}});
    auto* triggerEntityNode = triggerEntityNodePtr.get();
    mdl::addNodes(
      map,
      mdl::parentForNodes(map),
      kdl::vec_from<std::unique_ptr<mdl::Node>>(
        std::move(detailEntityNodePtr), std::move(triggerEntityNodePtr)));

    auto* detailBrushNode =
      mdl::addNode(map, *detailEntityNode, mdl::createBrushNode(map));
    auto* triggerBrushNode =
      mdl::addNode(map, *triggerEntityNode, mdl::createBrushNode(map));
    auto* worldBrushNode =
      mdl::addNode(map, mdl::parentForNodes(map), mdl::createBrushNode(map));

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
