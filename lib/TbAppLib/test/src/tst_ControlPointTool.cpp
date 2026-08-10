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

#include "gl/Camera.h"
#include "gl/PerspectiveCamera.h"
#include "mdl/BezierPatch.h"
#include "mdl/Hit.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandles.h"
#include "mdl/PatchNode.h"
#include "mdl/PickResult.h"
#include "mdl/TestFactory.h"
#include "ui/ControlPointTool.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "vm/ray.h"
#include "vm/vec.h"

#include <ranges>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

namespace
{

// createPatchNode() always returns the same fixed geometry, so a second call would sit at
// the exact same control point positions as the first and its handles would collapse into
// the same clumps. Offset it so its handles are spatially distinct.
mdl::PatchNode* createOffsetPatchNode()
{
  // clang-format off
  return new mdl::PatchNode{mdl::BezierPatch{3, 3, {
    {10, 0, 0}, {11, 0, 1}, {12, 0, 0},
    {10, 1, 1}, {11, 1, 2}, {12, 1, 1},
    {10, 2, 0}, {11, 2, 1}, {12, 2, 0},
  }, "material"}};
  // clang-format on
}

} // namespace

TEST_CASE("ControlPointTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  SECTION("pick")
  {
    auto* patchNode = mdl::createPatchNode();
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode}}});
    mdl::selectNodes(map, {patchNode});

    auto tool = ControlPointTool{document};
    REQUIRE(tool.activate());

    // Camera close to the patch, looking in +Z; a pick ray along +Z passes directly
    // through the patch's corner control point at (0, 0, 0). The patch's control points
    // are only 1-2 units apart, so the camera distance and handle radius are kept small
    // enough that the (perspective-scaled) pick sphere does not engulf its neighbors.
    const auto camera = gl::PerspectiveCamera{
      90.0f,
      1.0f,
      8192.0f,
      gl::Camera::Viewport{0, 0, 800, 600},
      vm::vec3f{0.0f, 0.0f, -10.0f},
      vm::vec3f{0.0f, 0.0f, 1.0f},
      vm::vec3f{0.0f, 1.0f, 0.0f}};
    const auto handleRadius = 0.1;

    WHEN("A pick ray hits a control point")
    {
      const auto pickRay = vm::ray3d{{0.0, 0.0, -10.0}, {0.0, 0.0, 1.0}};

      auto pickResult = mdl::PickResult{};
      tool.pick(pickRay, camera, handleRadius, pickResult);

      THEN("A hit for that control point is produced")
      {
        CHECK_THAT(
          pickResult.all() | std::views::transform([](const auto& hit) {
            return hit.template target<mdl::ControlPointHandle>();
          }),
          RangeEquals(std::vector<mdl::ControlPointHandle>{{{0, 0, 0}}}));
      }
    }

    WHEN("A pick ray misses all control points")
    {
      const auto pickRay = vm::ray3d{{100.0, 100.0, -10.0}, {0.0, 0.0, 1.0}};

      auto pickResult = mdl::PickResult{};
      tool.pick(pickRay, camera, handleRadius, pickResult);

      THEN("No hit is produced")
      {
        CHECK(pickResult.empty());
      }
    }
  }

  SECTION("handlePositionAndHitPoint")
  {
    auto* patchNode = mdl::createPatchNode();
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode}}});
    mdl::selectNodes(map, {patchNode});

    auto tool = ControlPointTool{document};
    REQUIRE(tool.activate());

    const auto handlePosition = vm::vec3d{0, 0, 0};
    const auto hitPoint = vm::vec3d{1, 2, 3};
    const auto hit = mdl::Hit{
      mdl::ControlPointHandle::HandleHitType,
      0.0,
      hitPoint,
      mdl::ControlPointHandle{handlePosition}};

    const auto [actualHandlePosition, actualHitPoint] =
      tool.handlePositionAndHitPoint({hit});

    CHECK(actualHandlePosition == handlePosition);
    CHECK(actualHitPoint == hitPoint);
  }

  SECTION("addHandles")
  {
    auto* patchNode1 = mdl::createPatchNode();
    auto* patchNode2 = createOffsetPatchNode();
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode1, patchNode2}}});
    mdl::selectNodes(map, {patchNode1});

    auto tool = ControlPointTool{document};
    REQUIRE(tool.activate());
    REQUIRE(map.nodeHandles().handleCount<mdl::ControlPointHandle>() == 9u);

    WHEN("Another node is selected while the tool is active")
    {
      mdl::selectNodes(map, {patchNode2});

      THEN("Its control point handles are added to the handle manager")
      {
        CHECK(map.nodeHandles().handleCount<mdl::ControlPointHandle>() == 18u);
      }
    }
  }

  SECTION("removeHandles")
  {
    auto* patchNode1 = mdl::createPatchNode();
    auto* patchNode2 = createOffsetPatchNode();
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode1, patchNode2}}});
    mdl::selectNodes(map, {patchNode1, patchNode2});

    auto tool = ControlPointTool{document};
    REQUIRE(tool.activate());
    REQUIRE(map.nodeHandles().handleCount<mdl::ControlPointHandle>() == 18u);

    WHEN("A node is deselected while the tool is active")
    {
      mdl::deselectNodes(map, {patchNode1});

      THEN("Its control point handles are removed from the handle manager")
      {
        CHECK(map.nodeHandles().handleCount<mdl::ControlPointHandle>() == 9u);
      }
    }
  }

  SECTION("move")
  {
    auto* patchNode = mdl::createPatchNode();
    mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode}}});
    mdl::selectNodes(map, {patchNode});

    auto tool = ControlPointTool{document};
    REQUIRE(tool.activate());

    GIVEN(
      "A control point is dragged onto another control point's position over several "
      "mouse-move events")
    {
      // Regression test for https://github.com/TrenchBroom/TrenchBroom/issues/5379: a
      // single drag generates one command per mouse-move event, and all but the first are
      // collated away when the drag ends. Redoing the resulting undo entry must not
      // crash.
      const auto originalPosition = vm::vec3d{0, 0, 0};
      // coincides with the patch's other corner control point at (0, 2)
      const auto finalPosition = vm::vec3d{2, 0, 0};

      const auto hit = mdl::Hit{
        mdl::ControlPointHandle::HandleHitType,
        0.0,
        originalPosition,
        mdl::ControlPointHandle{originalPosition}};

      WHEN("The control point is dragged there in two steps")
      {
        REQUIRE(tool.startMove({hit}));
        CHECK(tool.move(vm::vec3d{1, 0, 0}) == ControlPointTool::MoveResult::Continue);
        CHECK(tool.move(vm::vec3d{1, 0, 0}) == ControlPointTool::MoveResult::Continue);
        tool.endMove();

        THEN("The patch reflects the final position")
        {
          CHECK(patchNode->patch().controlPoint(0, 0).xyz() == finalPosition);
        }

        AND_WHEN("The move is undone")
        {
          map.undoCommand();

          THEN("The patch is restored to its original position")
          {
            CHECK(patchNode->patch().controlPoint(0, 0).xyz() == originalPosition);
          }

          AND_WHEN("The move is redone")
          {
            map.redoCommand();

            THEN("The patch reflects the final position and its handle is selected")
            {
              CHECK(patchNode->patch().controlPoint(0, 0).xyz() == finalPosition);

              const auto finalHit = mdl::Hit{
                mdl::ControlPointHandle::HandleHitType,
                0.0,
                finalPosition,
                mdl::ControlPointHandle{finalPosition}};
              CHECK(tool.selected(finalHit));
            }
          }
        }
      }
    }
  }
}

} // namespace tb::ui
