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
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Tag.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ql/BrushNodeLazyMap.h"

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("BrushNodeLazyMap")
{
  // EntityClassNameTagMatcher tags geometry *owned by* a matching entity
  auto fixtureConfig = mdl::MapFixtureConfig{};
  fixtureConfig.gameInfo.gameConfig.smartTags = {
    mdl::SmartTag{
      "Detail",
      {},
      std::make_unique<mdl::EntityClassNameTagMatcher>("func_detail", ""),
    },
  };

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create(fixtureConfig);

  SECTION("Owned by worldspawn")
  {
    auto* brushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode}}});

    const auto lazyMap = makeBrushNodeLazyMap(map, *brushNode);

    CHECK(
      lazyMap.keys()
      == std::vector<std::string>{
        "bounds",
        "center",
        "entity",
        "groupName",
        "layerName",
        "locked",
        "materials",
        "selected",
        "tags",
        "type",
        "visible",
      });
    CHECK(brushNodeFieldNames() == lazyMap.keys());

    CHECK(lazyMap.at("type") == el::Value{"brush"});
    CHECK(lazyMap.at("bounds") == el::Value{brushNode->logicalBounds()});
    CHECK(lazyMap.at("center") == el::Value{brushNode->logicalBounds().center()});
    CHECK(lazyMap.at("visible") == el::Value{true});
    CHECK(lazyMap.at("locked") == el::Value{false});
    CHECK(lazyMap.at("selected") == el::Value{false});
    CHECK(lazyMap.at("layerName") == el::Value{"Default Layer"});
    CHECK(lazyMap.at("groupName") == el::Value::Undefined);
    CHECK(lazyMap.at("materials") == el::Value{el::ArrayType{el::Value{"material"}}});
    CHECK(lazyMap.at("tags") == el::Value{el::ArrayType{}});

    const auto entityValue = lazyMap.at("entity").value();
    CHECK(
      entityValue.lazyMapValue().at("classname")
      == el::Value{map.worldNode().entity().classname()});
  }

  SECTION("Owned by a placed entity")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"classname", "func_detail"}}}};
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {entityNode}}});

    auto* brushNode = mdl::createBrushNode(map, "tagged_material");
    mdl::addNodes(map, {{entityNode, {brushNode}}});

    const auto lazyMap = makeBrushNodeLazyMap(map, *brushNode);

    CHECK(
      lazyMap.at("materials") == el::Value{el::ArrayType{el::Value{"tagged_material"}}});
    CHECK(lazyMap.at("tags") == el::Value{el::ArrayType{el::Value{"Detail"}}});

    const auto entityValue = lazyMap.at("entity").value();
    CHECK(entityValue.lazyMapValue().at("classname") == el::Value{"func_detail"});
  }
}

} // namespace tb::ql
