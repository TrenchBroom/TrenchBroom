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
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/ScaleTool.h"
#include "ui/ScaleToolController.h"
#include "ui/ToolController.h"

#include "kd/result.h"

#include <memory>

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("ScaleToolController")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto* brushNode = mdl::addNode(
    map,
    *map.editorContext().currentLayer(),
    std::make_unique<mdl::BrushNode>(
      builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()));
  mdl::selectNodes(map, {brushNode});

  auto tool = ScaleTool{document};
  auto scaleToolController = ScaleToolController3D{tool};
  auto& controller = static_cast<ToolController&>(scaleToolController);

  // looks at the brush from the front right and from above
  const auto camera = gl::PerspectiveCamera{
    90.0f,
    1.0f,
    8000.0f,
    gl::Camera::Viewport{0, 0, 1920, 1080},
    vm::vec3f{40, -120, 80},
    vm::normalize(vm::vec3f{-40, 120, -80}),
    vm::vec3f{0, 0, 1}};

  // the input state for the mouse pointing at the given point
  const auto inputStateAt = [&](
                              const vm::vec3d& point,
                              const ModifierKeyState modifierKeys = ModifierKeys::None) {
    auto inputState = InputState{0.0f, 0.0f};
    inputState.setPickRequest(
      PickRequest{vm::ray3d{camera.pickRay(vm::vec3f{point})}, camera});
    inputState.setModifierKeys(modifierKeys);
    inputState.mouseDown(MouseButtons::Left);

    auto pickResult = mdl::PickResult{};
    controller.pick(inputState, pickResult);
    inputState.setPickResult(std::move(pickResult));
    return inputState;
  };

  SECTION("acceptMouseDrag")
  {
    auto tracker = controller.acceptMouseDrag(inputStateAt({16, 0, 0}));
    REQUIRE(tracker != nullptr);

    CHECK(tracker->update(inputStateAt({32, 0, 0})));
    CHECK(brushNode->logicalBounds() == vm::bbox3d{{-16, -16, -16}, {32, 16, 16}});

    SECTION("pressing Alt during a drag scales about the center")
    {
      tracker->modifierKeyChange(inputStateAt({32, 0, 0}, ModifierKeys::Alt));
      CHECK(brushNode->logicalBounds() == vm::bbox3d{{-32, -16, -16}, {32, 16, 16}});

      SECTION("releasing Alt scales away from the opposite side again")
      {
        tracker->modifierKeyChange(inputStateAt({32, 0, 0}));
        CHECK(brushNode->logicalBounds() == vm::bbox3d{{-16, -16, -16}, {32, 16, 16}});
      }
    }

    SECTION("pressing Shift during a drag scales all axes proportionally")
    {
      tracker->modifierKeyChange(inputStateAt({32, 0, 0}, ModifierKeys::Shift));
      CHECK(brushNode->logicalBounds().size() == vm::vec3d{48, 48, 48});
    }

    tracker->cancel();
  }
}

} // namespace tb::ui
