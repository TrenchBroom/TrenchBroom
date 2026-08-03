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

#include "mdl/Hit.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandles.h"
#include "mdl/PatchNode.h"
#include "mdl/TestFactory.h"
#include "ui/ControlPointTool.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"

#include "vm/vec.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("ControlPointTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  SECTION("move")
  {
    auto* patchNode = mdl::createPatchNode();
    mdl::addNodes(map, {{mdl::parentForNodes(map), {patchNode}}});
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
