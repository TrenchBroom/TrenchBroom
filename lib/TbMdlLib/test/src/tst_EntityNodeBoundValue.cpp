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
#include "mdl/EntityNodeBoundValue.h"
#include "mdl/EntityProperties.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("EntityNodeBoundValue")
{
  const auto entity = Entity{{
    {"classname", "info_player_start"},
    {"targetname", "start1"},
  }};

  SECTION("entityPropertyValue")
  {
    CHECK(entityPropertyValue(entity, "classname") == el::Value{"info_player_start"});
    CHECK(entityPropertyValue(entity, "targetname") == el::Value{"start1"});
    CHECK(entityPropertyValue(entity, "missing") == std::nullopt);
  }

  SECTION("entityPropertyNames")
  {
    CHECK(
      entityPropertyNames(entity) == std::vector<std::string>{"classname", "targetname"});
  }

  SECTION("makeEntityPropertiesBoundValue")
  {
    const auto value = makeEntityPropertiesBoundValue(entity);

    CHECK(value.typeName() == "Map");

    el::withEvaluationContext([&](auto& context) {
      CHECK(value.boundValue(context).at("classname") == el::Value{"info_player_start"});
      CHECK(value.boundValue(context).at("missing") == std::nullopt);
      CHECK(
        value.boundValue(context).keys()
        == std::vector<std::string>{"classname", "targetname"});
    }).ignore();
  }

  SECTION("makeEntityBoundValue")
  {
    const auto value = makeEntityBoundValue(entity);

    CHECK(value.typeName() == "Map");

    el::withEvaluationContext([&](auto& context) {
      CHECK(value.boundValue(context).at("classname") == el::Value{"info_player_start"});
      CHECK(value.boundValue(context).at("missing") == std::nullopt);
      CHECK(
        value.boundValue(context).keys()
        == std::vector<std::string>{"classname", "properties"});

      const auto properties = value.boundValue(context).at("properties").value();
      CHECK(properties.boundValue(context).at("targetname") == el::Value{"start1"});
      CHECK(properties.boundValue(context).at("missing") == std::nullopt);
      CHECK(
        properties.boundValue(context).keys()
        == std::vector<std::string>{"classname", "targetname"});
    }).ignore();
  }

  SECTION("makeEntityNodeBoundValue")
  {
    auto fixture = MapFixture{};
    auto& map = fixture.create();

    auto* entityNode = new EntityNode{entity};
    addNodes(map, {{&parentForNodes(map), {entityNode}}});

    const auto boundValue = makeEntityNodeBoundValue(map, *entityNode);

    CHECK(
      boundValue.keys()
      == std::vector<std::string>{
        "bounds",
        "center",
        "classname",
        "entity",
        "groupName",
        "layerName",
        "linked",
        "locked",
        "name",
        "properties",
        "selected",
        "tags",
        "type",
        "visible",
      });

    CHECK(boundValue.at("type") == el::Value{"entity"});
    CHECK(boundValue.at("name") == el::Value{"info_player_start"});
    CHECK(boundValue.at("classname") == el::Value{"info_player_start"});
    CHECK(boundValue.at("bounds") == el::Value{entityNode->logicalBounds()});
    CHECK(boundValue.at("center") == el::Value{entityNode->logicalBounds().center()});
    CHECK(boundValue.at("visible") == el::Value{true});
    CHECK(boundValue.at("locked") == el::Value{false});
    CHECK(boundValue.at("selected") == el::Value{false});

    // added directly under the default layer, not inside any group
    CHECK(boundValue.at("layerName") == el::Value{"Default Layer"});
    CHECK(boundValue.at("groupName") == el::Value::Undefined);
    CHECK(boundValue.at("linked") == el::Value{false});

    // no smart tag is configured in this fixture, so tags is always empty
    CHECK(boundValue.at("tags") == el::Value{el::ArrayType{}});

    CHECK(boundValue.at("missing") == std::nullopt);

    el::withEvaluationContext([&](auto& context) {
      const auto properties = boundValue.at("properties").value();
      CHECK(properties.boundValue(context).at("targetname") == el::Value{"start1"});

      const auto entityValue = boundValue.at("entity").value();
      CHECK(
        entityValue.boundValue(context).at("classname")
        == el::Value{"info_player_start"});
    }).ignore();
  }
}

} // namespace tb::mdl
