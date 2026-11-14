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
#include "mdl/Brush.h"
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
#include "ui/ShearTool.h"
#include "ui/ShearToolController.h"
#include "ui/ToolController.h"

#include "kd/result.h"

#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

TEST_CASE("ShearToolController")
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

  auto tool = ShearTool{document};
  auto shearToolController = ShearToolController3D{tool};
  auto& controller = static_cast<ToolController&>(shearToolController);

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
    SECTION("shears a side sideways")
    {
      auto tracker = controller.acceptMouseDrag(inputStateAt({16, 0, 0}));
      REQUIRE(tracker != nullptr);

      CHECK(tracker->update(inputStateAt({16, 16, 0})));
      tracker->end(inputStateAt({16, 16, 0}));

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        UnorderedEquals(std::vector<vm::vec3d>{
          {-16, -16, -16},
          {-16, -16, 16},
          {-16, 16, -16},
          {-16, 16, 16},
          {16, 0, -16},
          {16, 0, 16},
          {16, 32, -16},
          {16, 32, 16},
        }));
    }

    SECTION("pressing Alt during a drag shears a side vertically")
    {
      auto tracker = controller.acceptMouseDrag(inputStateAt({16, 0, 0}));
      REQUIRE(tracker != nullptr);

      tracker->modifierKeyChange(inputStateAt({16, 0, 0}, ModifierKeys::Alt));
      CHECK(tracker->update(inputStateAt({16, 0, 16}, ModifierKeys::Alt)));
      tracker->end(inputStateAt({16, 0, 16}, ModifierKeys::Alt));

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        UnorderedEquals(std::vector<vm::vec3d>{
          {-16, -16, -16},
          {-16, -16, 16},
          {-16, 16, -16},
          {-16, 16, 16},
          {16, -16, 0},
          {16, -16, 32},
          {16, 16, 0},
          {16, 16, 32},
        }));
    }

    SECTION("releasing Alt during a drag shears a side sideways")
    {
      auto tracker =
        controller.acceptMouseDrag(inputStateAt({16, 0, 0}, ModifierKeys::Alt));
      REQUIRE(tracker != nullptr);

      tracker->modifierKeyChange(inputStateAt({16, 0, 0}));
      CHECK(tracker->update(inputStateAt({16, 16, 0})));
      tracker->end(inputStateAt({16, 16, 0}));

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        UnorderedEquals(std::vector<vm::vec3d>{
          {-16, -16, -16},
          {-16, -16, 16},
          {-16, 16, -16},
          {-16, 16, 16},
          {16, 0, -16},
          {16, 0, 16},
          {16, 32, -16},
          {16, 32, 16},
        }));
    }

    SECTION("pressing Alt during a drag doesn't constrain the top side")
    {
      auto tracker = controller.acceptMouseDrag(inputStateAt({0, 0, 16}));
      REQUIRE(tracker != nullptr);

      tracker->modifierKeyChange(inputStateAt({0, 0, 16}, ModifierKeys::Alt));
      CHECK(tracker->update(inputStateAt({16, 0, 16}, ModifierKeys::Alt)));
      tracker->end(inputStateAt({16, 0, 16}, ModifierKeys::Alt));

      CHECK_THAT(
        brushNode->brush().vertexPositions(),
        UnorderedEquals(std::vector<vm::vec3d>{
          {-16, -16, -16},
          {-16, 16, -16},
          {16, -16, -16},
          {16, 16, -16},
          {0, -16, 16},
          {0, 16, 16},
          {32, -16, 16},
          {32, 16, 16},
        }));
    }
  }
}

} // namespace tb::ui
