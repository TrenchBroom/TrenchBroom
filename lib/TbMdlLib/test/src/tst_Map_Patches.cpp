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

#include "mdl/BrushBuilder.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/CreatePatch.h"
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Patches.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kd/result.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{
TEST_CASE("Map_Patches")
{
  using namespace Catch::Matchers;

  auto fixture = MapFixture{};

  SECTION("createPatches")
  {
    auto& map = fixture.create();

    const auto builder = BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
    auto* brushNode = new BrushNode{builder.createCube(64.0, "material") | kdl::value()};
    addNodes(map, {{parentForNodes(map), {brushNode}}});

    const auto firstFaceIndex = brushNode->brush().findFace(vm::vec3d{0, 0, 1});
    const auto secondFaceIndex = brushNode->brush().findFace(vm::vec3d{1, 0, 0});
    REQUIRE(firstFaceIndex);
    REQUIRE(secondFaceIndex);

    const auto firstFaceHandle = BrushFaceHandle{brushNode, *firstFaceIndex};
    const auto secondFaceHandle = BrushFaceHandle{brushNode, *secondFaceIndex};
    selectBrushFaces(map, {firstFaceHandle, secondFaceHandle});

    const auto expectedPatches = kdl::vec_concat(
      createPatch(firstFaceHandle.face(), 3, 3),
      createPatch(secondFaceHandle.face(), 3, 3));

    createPatches(map, 3, 3);

    const auto& selection = map.selection();
    CHECK(selection.hasOnlyPatches());

    const auto actualPatches =
      selection.patches
      | std::views::transform([](const auto* patchNode) { return patchNode->patch(); });

    CHECK_THAT(actualPatches, UnorderedRangeEquals(expectedPatches));
  }
}

} // namespace tb::mdl
