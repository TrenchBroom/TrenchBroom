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
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/WorldNode.h"
#include "ql/WorldNodeBinding.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("WorldNodeBinding")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  const auto& worldNode = map.worldNode();
  const auto boundValue = makeWorldNodeBinding(map, worldNode);

  CHECK(
    boundValue.keys()
    == std::vector<std::string>{
      "classname",
      "locked",
      "properties",
      "selected",
      "tags",
      "type",
      "visible",
    });
  CHECK(worldNodeFieldNames() == boundValue.keys());

  CHECK(boundValue.at("type") == el::Value{"world"});
  CHECK(boundValue.at("classname") == el::Value{"worldspawn"});
  CHECK(boundValue.at("visible") == el::Value{true});
  CHECK(boundValue.at("locked") == el::Value{false});
  CHECK(boundValue.at("selected") == el::Value{false});

  // no smart tag is configured in this fixture, so tags is always empty
  CHECK(boundValue.at("tags") == el::Value{el::ArrayType{}});

  CHECK(boundValue.at("missing") == std::nullopt);

  el::withEvaluationContext([&](auto&) {
    const auto properties = boundValue.at("properties").value();
    CHECK(properties.boundValue().at("classname") == el::Value{"worldspawn"});
  }).ignore();
}

} // namespace tb::ql
