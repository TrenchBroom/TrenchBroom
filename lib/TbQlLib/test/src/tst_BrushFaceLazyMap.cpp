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
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/Tag.h"
#include "mdl/TagMatcher.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ql/BrushFaceLazyMap.h"

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("BrushFaceLazyMap")
{
  // MaterialNameTagMatcher tags at face granularity, unlike EntityClassNameTagMatcher
  auto fixtureConfig = mdl::MapFixtureConfig{};
  fixtureConfig.gameInfo.gameConfig.smartTags = {
    mdl::SmartTag{
      "Detail",
      {},
      std::make_unique<mdl::MaterialNameTagMatcher>("tagged_material"),
    },
  };

  auto fixture = mdl::MapFixture{};
  auto& map = fixture.create(fixtureConfig);

  auto* brushNode = mdl::createBrushNode(map, "tagged_material");
  mdl::addNodes(map, {{&mdl::parentForNodes(map), {brushNode}}});

  const auto handle = mdl::toHandles(*brushNode).front();
  const auto lazyMap = makeBrushFaceLazyMap(map, handle);

  CHECK(
    lazyMap.keys()
    == std::vector<std::string>{
      "bounds",
      "center",
      "entity",
      "groupName",
      "layerName",
      "locked",
      "material",
      "normal",
      "selected",
      "tags",
      "type",
      "visible",
    });
  CHECK(brushFaceFieldNames() == lazyMap.keys());

  CHECK(lazyMap.at("type") == el::Value{"face"});
  CHECK(lazyMap.at("material") == el::Value{"tagged_material"});
  CHECK(lazyMap.at("normal") == el::Value{handle.face().normal()});
  CHECK(lazyMap.at("bounds") == el::Value{handle.face().bounds()});
  CHECK(lazyMap.at("center") == el::Value{handle.face().center()});
  CHECK(lazyMap.at("visible") == el::Value{true});
  CHECK(lazyMap.at("locked") == el::Value{false});
  CHECK(lazyMap.at("selected") == el::Value{false});
  CHECK(lazyMap.at("layerName") == el::Value{"Default Layer"});
  CHECK(lazyMap.at("groupName") == el::Value::Undefined);
  CHECK(lazyMap.at("tags") == el::Value{el::ArrayType{el::Value{"Detail"}}});

  const auto entityValue = lazyMap.at("entity").value();
  CHECK(
    entityValue.lazyMapValue().at("classname")
    == el::Value{map.worldNode().entity().classname()});

  SECTION("selected")
  {
    mdl::selectBrushFaces(map, {handle});
    CHECK(makeBrushFaceLazyMap(map, handle).at("selected") == el::Value{true});
  }
}

} // namespace tb::ql
