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
#include "el/Value.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/WorldNode.h"
#include "mdl/WorldNodeBoundValue.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("WorldNodeBoundValue")
{
  auto fixture = MapFixture{};
  auto& map = fixture.create();

  const auto& worldNode = map.worldNode();
  const auto boundValue = makeWorldNodeBoundValue(map, worldNode);

  CHECK(
    boundValue.keys()
    == std::vector<std::string>{
      "bounds",
      "center",
      "groupName",
      "layerName",
      "linked",
      "locked",
      "selected",
      "type",
      "visible",
    });

  CHECK(boundValue.at("type") == el::Value{"world"});
  CHECK(boundValue.at("bounds") == el::Value{worldNode.logicalBounds()});
  CHECK(boundValue.at("center") == el::Value{worldNode.logicalBounds().center()});
  CHECK(boundValue.at("visible") == el::Value{true});
  CHECK(boundValue.at("locked") == el::Value{false});
  CHECK(boundValue.at("selected") == el::Value{false});

  // a WorldNode is never itself inside a layer or group, and never part of a link set
  CHECK(boundValue.at("layerName") == el::Value::Undefined);
  CHECK(boundValue.at("groupName") == el::Value::Undefined);
  CHECK(boundValue.at("linked") == el::Value{false});

  CHECK(boundValue.at("missing") == std::nullopt);
}

} // namespace tb::mdl
