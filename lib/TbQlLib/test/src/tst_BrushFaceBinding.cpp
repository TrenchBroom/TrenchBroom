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
#include "ql/BrushFaceBinding.h"

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace tb::ql
{

TEST_CASE("BrushFaceBinding")
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
  const auto boundValue = makeBrushFaceBinding(map, handle);

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
      "material",
      "normal",
      "selected",
      "tags",
      "type",
      "visible",
    });
  CHECK(brushFaceFieldNames() == boundValue.keys());

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
  CHECK(boundValue.at("linked") == el::Value{false});
  CHECK(boundValue.at("tags") == el::Value{el::ArrayType{el::Value{"Detail"}}});

  el::withEvaluationContext([&](auto&) {
    const auto entityValue = boundValue.at("entity").value();
    CHECK(
      entityValue.boundValue().at("classname")
      == el::Value{map.worldNode().entity().classname()});
  }).ignore();

  SECTION("selected")
  {
    mdl::selectBrushFaces(map, {handle});
    CHECK(makeBrushFaceBinding(map, handle).at("selected") == el::Value{true});
  }
}

} // namespace tb::ql
