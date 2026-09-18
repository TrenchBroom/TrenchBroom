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
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "ql/WorldNodeLazyMap.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("WorldNodeLazyMap")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  const auto& worldNode = map.worldNode();
  const auto lazyMap = makeWorldNodeLazyMap(map, worldNode);

  CHECK(
    lazyMap.keys()
    == std::vector<std::string>{
      "classname",
      "locked",
      "properties",
      "selected",
      "tags",
      "type",
      "visible",
    });
  CHECK(worldNodeFieldNames() == lazyMap.keys());

  CHECK(lazyMap.at("type") == el::Value{"world"});
  CHECK(lazyMap.at("classname") == el::Value{"worldspawn"});
  CHECK(lazyMap.at("visible") == el::Value{true});
  CHECK(lazyMap.at("locked") == el::Value{false});
  CHECK(lazyMap.at("selected") == el::Value{false});

  // no smart tag is configured in this fixture, so tags is always empty
  CHECK(lazyMap.at("tags") == el::Value{el::ArrayType{}});

  CHECK(lazyMap.at("missing") == std::nullopt);

  const auto properties = lazyMap.at("properties").value();
  CHECK(properties.lazyMapValue().at("classname") == el::Value{"worldspawn"});
}

} // namespace tb::ql
