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
#include "gl/FontManager.h"
#include "gl/PerspectiveCamera.h"
#include "gl/ShaderManager.h"
#include "gl/TestGl.h"
#include "mdl/BezierPatch.h"
#include "mdl/CatchConfig.h"
#include "mdl/HitFilter.h"
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "mdl/PatchNode.h"
#include "mdl/PickResult.h"
#include "render/RenderContext.h"
#include "ui/ControlPointTool.h"
#include "ui/ControlPointToolController.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/ToolController.h"

#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

TEST_CASE("ControlPointToolController")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  // clang-format off
  auto* patchNode = new mdl::PatchNode{mdl::BezierPatch{3, 3, {
    {-16, -16, -16}, {0, -16,  0}, {16, -16, -16},
    {-16,   0,   0}, {0,   0, 16}, {16,   0,   0},
    {-16,  16, -16}, {0,  16,  0}, {16,  16, -16},
  }, "material"}};
  // clang-format on

  mdl::addNodes(map, {{&mdl::parentForNodes(map), {patchNode}}});
  mdl::selectNodes(map, {patchNode});

  auto tool = ControlPointTool{document};
  auto controller = ControlPointToolController{tool};

  REQUIRE(tool.activate());

  // looks at the patch from the front right and from above, so that no two control
  // points line up
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
                              const ModifierKeyState modifierKeys = ModifierKeys::None,
                              const MouseButtonState mouseButtons = MouseButtons::Left) {
    auto inputState = InputState{0.0f, 0.0f};
    inputState.setPickRequest(
      PickRequest{vm::ray3d{camera.pickRay(vm::vec3f{point})}, camera});
    inputState.setModifierKeys(modifierKeys);
    inputState.mouseDown(mouseButtons);

    auto pickResult = mdl::PickResult{};
    controller.pick(inputState, pickResult);
    inputState.setPickResult(std::move(pickResult));
    return inputState;
  };

  const auto selectControlPoint = [&](const vm::vec3d& position) {
    map.nodeHandles().selectHandle(mdl::ControlPointHandle{position});
  };

  const auto selectedControlPoints = [&]() {
    return mdl::ControlPointHandle::getPositions(
      map.nodeHandles().selectedHandles<mdl::ControlPointHandle>());
  };

  const auto controlPointHits = [](const InputState& inputState) {
    return inputState.pickResult().all(
      mdl::HitFilters::type(mdl::ControlPointHandle::HandleHitType));
  };

  // the control points at the front left and front right corners of the patch
  const auto frontLeft = [&]() { return patchNode->patch().controlPoint(0, 0).xyz(); };
  const auto frontRight = [&]() { return patchNode->patch().controlPoint(0, 2).xyz(); };

  // above the patch, away from any handle
  const auto emptySpace = vm::vec3d{0, 0, 64};

  SECTION("tool")
  {
    CHECK(&controller.tool() == static_cast<Tool*>(&tool));
    CHECK(&std::as_const(controller).tool() == static_cast<Tool*>(&tool));
  }

  SECTION("pick")
  {
    SECTION("adds a hit for the control point under the mouse")
    {
      const auto hits = controlPointHits(inputStateAt({16, -16, -16}));
      REQUIRE(hits.size() == 1u);
      CHECK(
        hits.front().target<mdl::ControlPointHandle>().position
        == vm::vec3d{16, -16, -16});
    }

    SECTION("adds nothing in empty space")
    {
      CHECK(inputStateAt(emptySpace).pickResult().empty());
    }
  }

  SECTION("mouseClick")
  {
    SECTION("selects the control point under the mouse")
    {
      CHECK(controller.mouseClick(inputStateAt({16, -16, -16})));
      CHECK(selectedControlPoints() == std::vector<vm::vec3d>{{16, -16, -16}});
    }

    SECTION("replaces the selection")
    {
      selectControlPoint({-16, -16, -16});

      CHECK(controller.mouseClick(inputStateAt({16, -16, -16})));
      CHECK(selectedControlPoints() == std::vector<vm::vec3d>{{16, -16, -16}});
    }

    SECTION("with CtrlCmd")
    {
      selectControlPoint({-16, -16, -16});

      SECTION("adds an unselected control point to the selection")
      {
        CHECK(controller.mouseClick(inputStateAt({16, -16, -16}, ModifierKeys::CtrlCmd)));
        CHECK_THAT(
          selectedControlPoints(),
          UnorderedRangeEquals(std::vector<vm::vec3d>{{-16, -16, -16}, {16, -16, -16}}));
      }

      SECTION("removes a selected control point from the selection")
      {
        CHECK(
          controller.mouseClick(inputStateAt({-16, -16, -16}, ModifierKeys::CtrlCmd)));
        CHECK(selectedControlPoints().empty());
      }
    }

    SECTION("in empty space")
    {
      SECTION("deselects all control points")
      {
        selectControlPoint({16, -16, -16});

        CHECK(controller.mouseClick(inputStateAt(emptySpace)));
        CHECK(selectedControlPoints().empty());
      }

      SECTION("deselects all control points with CtrlCmd")
      {
        selectControlPoint({16, -16, -16});

        CHECK(controller.mouseClick(inputStateAt(emptySpace, ModifierKeys::CtrlCmd)));
        CHECK(selectedControlPoints().empty());
      }

      SECTION("does nothing if no control point is selected")
      {
        CHECK(!controller.mouseClick(inputStateAt(emptySpace)));
      }
    }

    SECTION("does nothing with another mouse button")
    {
      CHECK(!controller.mouseClick(
        inputStateAt({16, -16, -16}, ModifierKeys::None, MouseButtons::Right)));
      CHECK(selectedControlPoints().empty());
    }

    SECTION("does nothing with Alt or Shift")
    {
      const auto modifierKeys = GENERATE(
        ModifierKeys::Alt,
        ModifierKeys::Shift,
        ModifierKeys::Alt | ModifierKeys::CtrlCmd,
        ModifierKeys::Shift | ModifierKeys::CtrlCmd);
      CAPTURE(modifierKeys);

      CHECK(!controller.mouseClick(inputStateAt({16, -16, -16}, modifierKeys)));
      CHECK(selectedControlPoints().empty());
    }
  }

  SECTION("acceptMouseDrag")
  {
    SECTION("on a control point")
    {
      SECTION("starts a move with CtrlCmd, Alt or no modifier keys")
      {
        const auto modifierKeys = GENERATE(
          ModifierKeys::None,
          ModifierKeys::Alt,
          ModifierKeys::CtrlCmd,
          ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        CAPTURE(modifierKeys);

        auto tracker =
          controller.acceptMouseDrag(inputStateAt({16, -16, -16}, modifierKeys));
        REQUIRE(tracker != nullptr);
        CHECK(selectedControlPoints() == std::vector<vm::vec3d>{{16, -16, -16}});

        tracker->cancel();
      }

      SECTION("does nothing with Shift")
      {
        const auto modifierKeys = GENERATE(
          ModifierKeys::Shift,
          ModifierKeys::Shift | ModifierKeys::Alt,
          ModifierKeys::Shift | ModifierKeys::CtrlCmd,
          ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        CAPTURE(modifierKeys);

        CHECK(
          controller.acceptMouseDrag(inputStateAt({16, -16, -16}, modifierKeys))
          == nullptr);
        CHECK(selectedControlPoints().empty());
      }

      SECTION("moves the control point horizontally")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt({16, -16, -16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({32, -16, -16})));
        tracker->end(inputStateAt({32, -16, -16}));

        CHECK(frontRight() == vm::vec3d{32, -16, -16});
        CHECK(selectedControlPoints() == std::vector<vm::vec3d>{{32, -16, -16}});
      }

      SECTION("moves the control point vertically with Alt")
      {
        auto tracker =
          controller.acceptMouseDrag(inputStateAt({16, -16, -16}, ModifierKeys::Alt));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({16, -16, 0}, ModifierKeys::Alt)));
        tracker->end(inputStateAt({16, -16, 0}, ModifierKeys::Alt));

        CHECK(frontRight() == vm::vec3d{16, -16, 0});
      }

      SECTION("snapping")
      {
        // move the control point at 16 -16 -16 off the grid
        REQUIRE(mdl::translateSelection(map, vm::vec3d{4, 0, 0}));
        REQUIRE(frontRight() == vm::vec3d{20, -16, -16});

        SECTION("snaps the distance moved to the grid")
        {
          auto tracker = controller.acceptMouseDrag(inputStateAt({20, -16, -16}));
          REQUIRE(tracker != nullptr);

          CHECK(tracker->update(inputStateAt({42, -16, -16})));
          tracker->end(inputStateAt({42, -16, -16}));

          CHECK(frontRight() == vm::vec3d{36, -16, -16});
        }

        SECTION("does not snap the control point to the grid with CtrlCmd")
        {
          auto tracker = controller.acceptMouseDrag(
            inputStateAt({20, -16, -16}, ModifierKeys::CtrlCmd));
          REQUIRE(tracker != nullptr);

          CHECK(tracker->update(inputStateAt({42, -16, -16}, ModifierKeys::CtrlCmd)));
          tracker->end(inputStateAt({42, -16, -16}, ModifierKeys::CtrlCmd));

          CHECK(frontRight() == vm::vec3d{36, -16, -16});
        }
      }

      SECTION("moves all selected control points if the control point is selected")
      {
        selectControlPoint({16, -16, -16});
        selectControlPoint({-16, -16, -16});

        auto tracker = controller.acceptMouseDrag(inputStateAt({16, -16, -16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({16, -32, -16})));
        tracker->end(inputStateAt({16, -32, -16}));

        CHECK(frontRight() == vm::vec3d{16, -32, -16});
        CHECK(frontLeft() == vm::vec3d{-16, -32, -16});
        CHECK_THAT(
          selectedControlPoints(),
          UnorderedRangeEquals(std::vector<vm::vec3d>{{16, -32, -16}, {-16, -32, -16}}));
      }

      SECTION("moves only the control point if it is not selected")
      {
        selectControlPoint({-16, -16, -16});

        auto tracker = controller.acceptMouseDrag(inputStateAt({16, -16, -16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({16, -32, -16})));
        tracker->end(inputStateAt({16, -32, -16}));

        CHECK(frontRight() == vm::vec3d{16, -32, -16});
        CHECK(frontLeft() == vm::vec3d{-16, -16, -16});
        CHECK(selectedControlPoints() == std::vector<vm::vec3d>{{16, -32, -16}});
      }

      SECTION("restores the patch when the move is cancelled")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt({16, -16, -16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({32, -16, -16})));
        REQUIRE(frontRight() == vm::vec3d{32, -16, -16});

        tracker->cancel();

        CHECK(frontRight() == vm::vec3d{16, -16, -16});
      }
    }

    SECTION("in empty space")
    {
      // opposite corners of a box around the front right control point of the patch
      const auto controlPoint = vm::vec3d{16, -16, -16};
      const auto diagonal = vm::vec3d{4.0f * (camera.right() + camera.up())};
      const auto lassoStart = controlPoint - diagonal;
      const auto lassoEnd = controlPoint + diagonal;

      SECTION("starts a lasso that selects only the control points inside of it")
      {
        selectControlPoint({-16, -16, -16});

        auto tracker = controller.acceptMouseDrag(inputStateAt(lassoStart));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt(lassoEnd)));
        tracker->end(inputStateAt(lassoEnd));

        CHECK(selectedControlPoints() == std::vector<vm::vec3d>{controlPoint});
      }

      SECTION("starts a lasso that toggles the control points inside of it with CtrlCmd")
      {
        selectControlPoint({-16, -16, -16});
        selectControlPoint(controlPoint);

        auto tracker =
          controller.acceptMouseDrag(inputStateAt(lassoStart, ModifierKeys::CtrlCmd));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt(lassoEnd, ModifierKeys::CtrlCmd)));
        tracker->end(inputStateAt(lassoEnd, ModifierKeys::CtrlCmd));

        CHECK(selectedControlPoints() == std::vector<vm::vec3d>{{-16, -16, -16}});
      }

      SECTION("selects nothing when the lasso is cancelled")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt(lassoStart));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt(lassoEnd)));
        tracker->cancel();

        CHECK(selectedControlPoints().empty());
      }

      SECTION("does nothing with Alt or Shift")
      {
        const auto modifierKeys = GENERATE(ModifierKeys::Alt, ModifierKeys::Shift);
        CAPTURE(modifierKeys);

        CHECK(
          controller.acceptMouseDrag(inputStateAt(emptySpace, modifierKeys)) == nullptr);
      }
    }

    SECTION("does nothing with another mouse button")
    {
      CHECK(
        controller.acceptMouseDrag(
          inputStateAt({16, -16, -16}, ModifierKeys::None, MouseButtons::Right))
        == nullptr);
    }
  }

  SECTION("setRenderOptions")
  {
    auto testGl = gl::TestGl{};
    auto fontManager = gl::FontManager{[](const auto& path) { return path; }};
    auto shaderManager = gl::ShaderManager{[](const auto& path) { return path; }};
    auto renderContext = render::RenderContext{
      testGl, render::RenderMode::Render3D, camera, fontManager, shaderManager};

    SECTION("hides the selection guide")
    {
      renderContext.setShowSelectionGuide();
      REQUIRE(renderContext.showSelectionGuide());

      controller.setRenderOptions(inputStateAt(emptySpace), renderContext);
      CHECK(!renderContext.showSelectionGuide());
    }
  }

  SECTION("cancel")
  {
    SECTION("deselects all control points")
    {
      selectControlPoint({16, -16, -16});

      CHECK(controller.cancel());
      CHECK(selectedControlPoints().empty());
    }

    SECTION("does nothing if no control point is selected")
    {
      CHECK(!controller.cancel());
    }
  }
}

} // namespace tb::ui
