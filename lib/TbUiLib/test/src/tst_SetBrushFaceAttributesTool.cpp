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

#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Picking.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "ui/CatchConfig.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/SetBrushFaceAttributesTool.h"
#include "ui/ToolController.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{
namespace
{

mdl::PickResult pickAlongRay(mdl::Map& map, const vm::ray3d& pickRay)
{
  auto pickResult = mdl::PickResult{};
  mdl::pick(map, pickRay, pickResult);
  return pickResult;
}

mdl::PickResult pickFaceCenter(mdl::Map& map, const vm::vec3d& faceCenter)
{
  return pickAlongRay(map, vm::ray3d{faceCenter + vm::vec3d{0, 0, 500}, {0, 0, -1}});
}

} // namespace

TEST_CASE("SetBrushFaceAttributesTool")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};

  auto* brushA = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{16.0}, "material_a") | kdl::value()};
  auto* brushB = new mdl::BrushNode{
    builder.createCuboid(vm::bbox3d{{100, -16, -16}, {132, 16, 16}}, "material_b")
    | kdl::value()};

  mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushA, brushB}}});
  mdl::selectBrushFaces(
    map, {mdl::BrushFaceHandle{brushA, *brushA->brush().findFace(vm::vec3d{0, 0, 1})}});

  const auto brushBTopFaceIndex = *brushB->brush().findFace(vm::vec3d{0, 0, 1});
  const auto brushBSideFaceIndex = *brushB->brush().findFace(vm::vec3d{1, 0, 0});

  auto tool = SetBrushFaceAttributesTool{document};
  // mouseClick, mouseDoubleClick, acceptMouseDrag and cancel are private overrides of
  // the public ToolController interface, so they must be called through the base
  // class, just like the real dispatcher (ToolBoxConnector/ToolChain) does.
  auto& controller = static_cast<ToolController&>(tool);

  SECTION("cancel always returns false")
  {
    CHECK(!controller.cancel());
  }

  SECTION("tool returns itself")
  {
    CHECK(&controller.tool() == static_cast<Tool*>(&tool));
  }

  SECTION("mouseClick requires the copy modifier, a selected face and a brush hit")
  {
    SECTION("false without the copy modifier")
    {
      auto inputState = InputState{};
      inputState.mouseDown(MouseButtons::Left);
      inputState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));
      CHECK(!controller.mouseClick(inputState));
    }

    SECTION("false without exactly one selected face")
    {
      mdl::deselectAll(map);

      auto inputState = InputState{};
      inputState.setModifierKeys(ModifierKeys::Alt);
      inputState.mouseDown(MouseButtons::Left);
      inputState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));
      CHECK(!controller.mouseClick(inputState));
    }

    SECTION("false without a brush hit")
    {
      auto inputState = InputState{};
      inputState.setModifierKeys(ModifierKeys::Alt);
      inputState.mouseDown(MouseButtons::Left);
      CHECK(!controller.mouseClick(inputState));
    }
  }

  SECTION("mouseClick copies attributes to only the clicked face")
  {
    auto inputState = InputState{};
    inputState.setModifierKeys(ModifierKeys::Alt);
    inputState.mouseDown(MouseButtons::Left);
    inputState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));

    CHECK(controller.mouseClick(inputState));

    CHECK(brushB->brush().face(brushBTopFaceIndex).materialName() == "material_a");
    CHECK(brushB->brush().face(brushBSideFaceIndex).materialName() == "material_b");
  }

  SECTION(
    "mouseDoubleClick undoes the single-face transfer and applies to the whole brush")
  {
    auto inputState = InputState{};
    inputState.setModifierKeys(ModifierKeys::Alt);
    inputState.mouseDown(MouseButtons::Left);
    inputState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));

    REQUIRE(controller.mouseClick(inputState));
    REQUIRE(brushB->brush().face(brushBSideFaceIndex).materialName() == "material_b");

    inputState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));
    CHECK(controller.mouseDoubleClick(inputState));

    for (size_t i = 0; i < brushB->brush().faceCount(); ++i)
    {
      CHECK(brushB->brush().face(i).materialName() == "material_a");
    }
  }

  SECTION("mouseDoubleClick does nothing if the last command was not a face transfer")
  {
    auto inputState = InputState{};
    inputState.setModifierKeys(ModifierKeys::Alt);
    inputState.mouseDown(MouseButtons::Left);
    inputState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));

    CHECK(!controller.mouseDoubleClick(inputState));
    CHECK(brushB->brush().face(brushBTopFaceIndex).materialName() == "material_b");
  }

  SECTION("acceptMouseDrag")
  {
    auto inputState = InputState{};
    inputState.setModifierKeys(ModifierKeys::Alt);
    inputState.mouseDown(MouseButtons::Left);

    SECTION("returns nullptr if the drag modifiers don't apply")
    {
      auto plainInputState = InputState{};
      plainInputState.mouseDown(MouseButtons::Left);
      CHECK(controller.acceptMouseDrag(plainInputState) == nullptr);
    }

    SECTION("returns nullptr without exactly one selected face")
    {
      mdl::deselectAll(map);
      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("dragging across faces transfers attributes along the drag path")
    {
      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      auto topHitState = InputState{};
      topHitState.setModifierKeys(ModifierKeys::Alt);
      topHitState.mouseDown(MouseButtons::Left);
      topHitState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));
      CHECK(tracker->update(topHitState));
      CHECK(brushB->brush().face(brushBTopFaceIndex).materialName() == "material_a");
      CHECK(brushB->brush().face(brushBSideFaceIndex).materialName() == "material_b");

      auto sideHitState = InputState{};
      sideHitState.setModifierKeys(ModifierKeys::Alt);
      sideHitState.mouseDown(MouseButtons::Left);
      sideHitState.setPickResult(pickAlongRay(map, vm::ray3d{{200, 0, 0}, {-1, 0, 0}}));
      CHECK(tracker->update(sideHitState));
      CHECK(brushB->brush().face(brushBSideFaceIndex).materialName() == "material_a");

      tracker->end(sideHitState);
      CHECK(brushB->brush().face(brushBSideFaceIndex).materialName() == "material_a");
    }

    SECTION("cancelling the drag reverts the changes")
    {
      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      auto topHitState = InputState{};
      topHitState.setModifierKeys(ModifierKeys::Alt);
      topHitState.mouseDown(MouseButtons::Left);
      topHitState.setPickResult(pickFaceCenter(map, vm::vec3d{116, 0, 16}));
      CHECK(tracker->update(topHitState));
      REQUIRE(brushB->brush().face(brushBTopFaceIndex).materialName() == "material_a");

      tracker->cancel();
      CHECK(brushB->brush().face(brushBTopFaceIndex).materialName() == "material_b");
    }
  }
}

} // namespace tb::ui
