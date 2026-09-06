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
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceBoundValue.h"
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

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::mdl
{

TEST_CASE("BrushFaceBoundValue")
{
  // MaterialNameTagMatcher tags at face granularity, unlike EntityClassNameTagMatcher
  auto fixtureConfig = MapFixtureConfig{};
  fixtureConfig.gameInfo.gameConfig.smartTags = {
    SmartTag{
      "Detail",
      {},
      std::make_unique<MaterialNameTagMatcher>("tagged_material"),
    },
  };

  auto fixture = MapFixture{};
  auto& map = fixture.create(fixtureConfig);

  auto* brushNode = createBrushNode(map, "tagged_material");
  addNodes(map, {{&parentForNodes(map), {brushNode}}});

  const auto handle = toHandles(brushNode).front();
  const auto boundValue = makeBrushFaceBoundValue(map, handle);

  CHECK(
    boundValue.keys()
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

  CHECK(boundValue.at("type") == el::Value{"face"});
  CHECK(boundValue.at("material") == el::Value{"tagged_material"});
  CHECK(boundValue.at("normal") == el::Value{handle.face().normal()});
  CHECK(boundValue.at("bounds") == el::Value{handle.face().bounds()});
  CHECK(boundValue.at("center") == el::Value{handle.face().center()});
  CHECK(boundValue.at("visible") == el::Value{true});
  CHECK(boundValue.at("locked") == el::Value{false});
  CHECK(boundValue.at("selected") == el::Value{false});
  CHECK(boundValue.at("layerName") == el::Value{"Default Layer"});
  CHECK(boundValue.at("groupName") == el::Value::Undefined);
  CHECK(boundValue.at("tags") == el::Value{el::ArrayType{el::Value{"Detail"}}});

  el::withEvaluationContext([&](auto& context) {
    const auto entityValue = boundValue.at("entity").value();
    CHECK(
      entityValue.boundValue(context).at("classname")
      == el::Value{map.worldNode().entity().classname()});
  }).ignore();

  SECTION("selected")
  {
    selectBrushFaces(map, {handle});
    CHECK(makeBrushFaceBoundValue(map, handle).at("selected") == el::Value{true});
  }
}

} // namespace tb::mdl
