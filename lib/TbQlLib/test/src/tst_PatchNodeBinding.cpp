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

#include "el/BoundValue.h"
#include "el/EvaluationContext.h"
#include "el/Value.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/EntityProperties.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/PatchNode.h"
#include "mdl/Tag.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ql/PatchNodeBinding.h"

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("PatchNodeBinding")
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
    auto* patchNode = mdl::createPatchNode();
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode}}});

    const auto boundValue = makePatchNodeBinding(map, *patchNode);

    CHECK(
      boundValue.keys()
      == std::vector<std::string>{
        "bounds",
        "center",
        "entity",
        "groupName",
        "layerName",
        "linked",
        "locked",
        "materials",
        "selected",
        "tags",
        "type",
        "visible",
      });
    CHECK(patchNodeFieldNames() == boundValue.keys());

    CHECK(boundValue.at("type") == el::Value{"patch"});
    CHECK(boundValue.at("bounds") == el::Value{patchNode->logicalBounds()});
    CHECK(boundValue.at("center") == el::Value{patchNode->logicalBounds().center()});
    CHECK(boundValue.at("visible") == el::Value{true});
    CHECK(boundValue.at("locked") == el::Value{false});
    CHECK(boundValue.at("selected") == el::Value{false});
    CHECK(boundValue.at("layerName") == el::Value{"Default Layer"});
    CHECK(boundValue.at("groupName") == el::Value::Undefined);
    CHECK(boundValue.at("linked") == el::Value{false});
    CHECK(boundValue.at("materials") == el::Value{el::ArrayType{el::Value{"material"}}});
    CHECK(boundValue.at("tags") == el::Value{el::ArrayType{}});

    el::withEvaluationContext([&](auto&) {
      const auto entityValue = boundValue.at("entity").value();
      CHECK(
        entityValue.boundValue().at("classname")
        == el::Value{map.worldNode().entity().classname()});
    }).ignore();
  }

  SECTION("Owned by a placed entity")
  {
    auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"classname", "func_detail"}}}};
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {entityNode}}});

    auto* patchNode = mdl::createPatchNode("tagged_material");
    mdl::addNodes(map, {{entityNode, {patchNode}}});

    const auto boundValue = makePatchNodeBinding(map, *patchNode);

    CHECK(
      boundValue.at("materials")
      == el::Value{el::ArrayType{el::Value{"tagged_material"}}});
    CHECK(boundValue.at("tags") == el::Value{el::ArrayType{el::Value{"Detail"}}});

    el::withEvaluationContext([&](auto&) {
      const auto entityValue = boundValue.at("entity").value();
      CHECK(entityValue.boundValue().at("classname") == el::Value{"func_detail"});
    }).ignore();
  }
}

} // namespace tb::ql
