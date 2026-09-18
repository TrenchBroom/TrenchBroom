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
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "ql/EntityNodeLazyMap.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("EntityNodeLazyMap")
{
  const auto entity = mdl::Entity{{
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

  SECTION("makeEntityPropertiesLazyMap")
  {
    const auto value = makeEntityPropertiesLazyMap(entity);

    CHECK(value.typeName() == "Map");

    CHECK(value.lazyMapValue().at("classname") == el::Value{"info_player_start"});
    CHECK(value.lazyMapValue().at("missing") == std::nullopt);
    CHECK(
      value.lazyMapValue().keys() == std::vector<std::string>{"classname", "targetname"});
  }

  SECTION("makeEntityLazyMap")
  {
    const auto value = makeEntityLazyMap(entity);

    CHECK(value.typeName() == "Map");

    CHECK(value.lazyMapValue().at("classname") == el::Value{"info_player_start"});
    CHECK(value.lazyMapValue().at("missing") == std::nullopt);
    CHECK(
      value.lazyMapValue().keys() == std::vector<std::string>{"classname", "properties"});

    const auto properties = value.lazyMapValue().at("properties").value();
    CHECK(properties.lazyMapValue().at("targetname") == el::Value{"start1"});
    CHECK(properties.lazyMapValue().at("missing") == std::nullopt);
    CHECK(
      properties.lazyMapValue().keys()
      == std::vector<std::string>{"classname", "targetname"});
  }

  SECTION("makeEntityNodeLazyMap")
  {
    auto fixture = mdl::MapFixture{};
    auto& map = fixture.create();

    auto* entityNode = new mdl::EntityNode{entity};
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {entityNode}}});

    const auto lazyMap = makeEntityNodeLazyMap(map, *entityNode);

    CHECK(
      lazyMap.keys()
      == std::vector<std::string>{
        "bounds",
        "center",
        "classname",
        "groupName",
        "layerName",
        "locked",
        "properties",
        "selected",
        "tags",
        "type",
        "visible",
      });
    CHECK(entityNodeFieldNames() == lazyMap.keys());

    CHECK(lazyMap.at("type") == el::Value{"entity"});
    CHECK(lazyMap.at("classname") == el::Value{"info_player_start"});
    CHECK(lazyMap.at("bounds") == el::Value{entityNode->logicalBounds()});
    CHECK(lazyMap.at("center") == el::Value{entityNode->logicalBounds().center()});
    CHECK(lazyMap.at("visible") == el::Value{true});
    CHECK(lazyMap.at("locked") == el::Value{false});
    CHECK(lazyMap.at("selected") == el::Value{false});

    // added directly under the default layer, not inside any group
    CHECK(lazyMap.at("layerName") == el::Value{"Default Layer"});
    CHECK(lazyMap.at("groupName") == el::Value::Undefined);

    // no smart tag is configured in this fixture, so tags is always empty
    CHECK(lazyMap.at("tags") == el::Value{el::ArrayType{}});

    CHECK(lazyMap.at("missing") == std::nullopt);

    const auto properties = lazyMap.at("properties").value();
    CHECK(properties.lazyMapValue().at("targetname") == el::Value{"start1"});
  }
}

} // namespace tb::ql
