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

#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Patches.h"
#include "mdl/Map_Selection.h"
#include "mdl/PatchNode.h"
#include "mdl/TestFactory.h"

#include "vm/mat_ext.h"

#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::mdl
{

TEST_CASE("Map_Patches")
{
  using namespace Catch::Matchers;

  auto fixture = MapFixture{};

  SECTION("transformControlPoints")
  {
    auto& map = fixture.create();

    GIVEN("Two selected patches and no patch has any of the given control points")
    {
      auto* patchNode1 = createPatchNode();
      auto* patchNode2 = createPatchNode();
      addNodes(map, {{parentForNodes(map), {patchNode1, patchNode2}}});
      selectNodes(map, {patchNode1, patchNode2});

      const auto patch1OriginalControlPoints = patchNode1->patch().controlPoints();
      const auto patch2OriginalControlPoints = patchNode2->patch().controlPoints();
      const auto patch1OriginalBounds = patchNode1->patch().bounds();
      const auto patch2OriginalBounds = patchNode2->patch().bounds();

      WHEN("Control points are transformed")
      {
        CHECK(transformControlPoints(
          map, {{100, 100, 100}}, vm::translation_matrix(vm::vec3d{2, 0, 0})));

        THEN("Neither patch changes and bounds stay the same")
        {
          CHECK(patchNode1->patch().controlPoints() == patch1OriginalControlPoints);
          CHECK(patchNode2->patch().controlPoints() == patch2OriginalControlPoints);
          CHECK(patchNode1->patch().bounds() == patch1OriginalBounds);
          CHECK(patchNode2->patch().bounds() == patch2OriginalBounds);
        }
      }
    }

    GIVEN("Two selected patches and only one patch has one of the given control points")
    {
      auto* patchNode1 = createPatchNode();

      // clang-format off
      auto* patchNode2 = new PatchNode{BezierPatch{3, 3, {
        {10, 0, 0}, {11, 0, 1}, {12, 0, 0},
        {10, 1, 1}, {11, 1, 2}, {12, 1, 1},
        {10, 2, 0}, {11, 2, 1}, {12, 2, 0},
      }, "material"}};
      // clang-format on

      addNodes(map, {{parentForNodes(map), {patchNode1, patchNode2}}});
      selectNodes(map, {patchNode1, patchNode2});

      const auto patch2OriginalControlPoints = patchNode2->patch().controlPoints();
      const auto patch2OriginalBounds = patchNode2->patch().bounds();

      WHEN("Control points are transformed")
      {
        CHECK(transformControlPoints(
          map, {{1, 1, 2}}, vm::translation_matrix(vm::vec3d{2, 0, 0})));

        THEN("Only the matching patch is updated and its bounds are updated")
        {
          CHECK(patchNode1->patch().controlPoint(1, 1).xyz() == vm::vec3d{3, 1, 2});
          CHECK(patchNode1->patch().bounds() == vm::bbox3d{{0, 0, 0}, {3, 2, 2}});

          CHECK(patchNode2->patch().controlPoints() == patch2OriginalControlPoints);
          CHECK(patchNode2->patch().bounds() == patch2OriginalBounds);
        }
      }
    }

    GIVEN("Two selected patches sharing the same control point")
    {
      auto* patchNode1 = createPatchNode();
      auto* patchNode2 = createPatchNode();
      addNodes(map, {{parentForNodes(map), {patchNode1, patchNode2}}});
      selectNodes(map, {patchNode1, patchNode2});

      WHEN("Control points are transformed")
      {
        CHECK(transformControlPoints(
          map, {{1, 1, 2}}, vm::translation_matrix(vm::vec3d{2, 0, 0})));

        THEN("Both patches are updated and bounds are updated")
        {
          CHECK(patchNode1->patch().controlPoint(1, 1).xyz() == vm::vec3d{3, 1, 2});
          CHECK(patchNode2->patch().controlPoint(1, 1).xyz() == vm::vec3d{3, 1, 2});
          CHECK(patchNode1->patch().bounds() == vm::bbox3d{{0, 0, 0}, {3, 2, 2}});
          CHECK(patchNode2->patch().bounds() == vm::bbox3d{{0, 0, 0}, {3, 2, 2}});
        }

        AND_WHEN("The transformation is undone")
        {
          map.undoCommand();

          THEN("Both patches are restored")
          {
            CHECK(patchNode1->patch().controlPoint(1, 1).xyz() == vm::vec3d{1, 1, 2});
            CHECK(patchNode2->patch().controlPoint(1, 1).xyz() == vm::vec3d{1, 1, 2});
            CHECK(patchNode1->patch().bounds() == vm::bbox3d{{0, 0, 0}, {2, 2, 2}});
            CHECK(patchNode2->patch().bounds() == vm::bbox3d{{0, 0, 0}, {2, 2, 2}});
          }

          AND_WHEN("The transformation is redone")
          {
            map.redoCommand();

            THEN("Both patches are transformed again")
            {
              CHECK(patchNode1->patch().controlPoint(1, 1).xyz() == vm::vec3d{3, 1, 2});
              CHECK(patchNode2->patch().controlPoint(1, 1).xyz() == vm::vec3d{3, 1, 2});
              CHECK(patchNode1->patch().bounds() == vm::bbox3d{{0, 0, 0}, {3, 2, 2}});
              CHECK(patchNode2->patch().bounds() == vm::bbox3d{{0, 0, 0}, {3, 2, 2}});
            }
          }
        }
      }
    }
  }
}

} // namespace tb::mdl
