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
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/WorldNode.h" // IWYU pragma: keep
#include "ql/LayerNodeLazyMap.h"

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("LayerNodeLazyMap")
{
  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create();

  auto* layerNode = new mdl::LayerNode{mdl::Layer{"Combat"}};
  mdl::addNodes(map, {{&map.worldNode(), {layerNode}}});

  const auto lazyMap = makeLayerNodeLazyMap(*layerNode);

  CHECK(
    lazyMap.keys()
    == std::vector<std::string>{
      "bounds",
      "center",
      "groupName",
      "layerName",
      "locked",
      "name",
      "selected",
      "type",
      "visible",
    });
  CHECK(layerNodeFieldNames() == lazyMap.keys());

  CHECK(lazyMap.at("type") == el::Value{"layer"});
  CHECK(lazyMap.at("name") == el::Value{"Combat"});
  CHECK(lazyMap.at("bounds") == el::Value{layerNode->logicalBounds()});
  CHECK(lazyMap.at("center") == el::Value{layerNode->logicalBounds().center()});
  CHECK(lazyMap.at("visible") == el::Value{true});
  CHECK(lazyMap.at("locked") == el::Value{false});
  CHECK(lazyMap.at("selected") == el::Value{false});

  // findContainingLayer treats a layer as containing itself
  CHECK(lazyMap.at("layerName") == el::Value{"Combat"});
  CHECK(lazyMap.at("groupName") == el::Value::Undefined);

  CHECK(lazyMap.at("missing") == std::nullopt);
}

} // namespace tb::ql
