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
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/CatchConfig.h"
#include "mdl/EditorContext.h"
#include "mdl/HitFilter.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/Map_Geometry.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/NodeHandleManager.h"
#include "mdl/NodeHandles.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "render/RenderContext.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/ToolController.h"
#include "ui/VertexTool.h"
#include "ui/VertexToolController.h"

#include "kd/result.h"

#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

namespace tb::ui
{
using namespace Catch::Matchers;

TEST_CASE("VertexToolController")
{
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create();
  auto& map = document.map();

  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto* brushNode =
    new mdl::BrushNode{builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value()};
  mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});
  mdl::selectNodes(map, {brushNode});

  auto tool = VertexTool{document};
  auto controller = VertexToolController{tool};

  REQUIRE(tool.activate());

  // looks at the brush from the front right and from above, so that no two vertices
  // line up
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

  const auto selectVertex = [&](const vm::vec3d& position) {
    map.nodeHandles().selectHandle(mdl::VertexHandle{position});
  };

  const auto selectedVertices = [&]() {
    return mdl::VertexHandle::getPositions(
      map.nodeHandles().selectedHandles<mdl::VertexHandle>());
  };

  const auto vertexHits = [](const InputState& inputState) {
    return inputState.pickResult().all(
      mdl::HitFilters::type(mdl::VertexHandle::HandleHitType));
  };

  // above the brush, away from any handle
  const auto emptySpace = vm::vec3d{0, 0, 64};

  SECTION("tool")
  {
    CHECK(&controller.tool() == static_cast<Tool*>(&tool));
    CHECK(&std::as_const(controller).tool() == static_cast<Tool*>(&tool));
  }

  SECTION("pick")
  {
    SECTION("adds a hit for the vertex under the mouse")
    {
      const auto hits = vertexHits(inputStateAt({16, 16, 16}));
      REQUIRE(hits.size() == 1u);
      CHECK(hits.front().target<mdl::VertexHandle>().position == vm::vec3d{16, 16, 16});
    }

    SECTION("adds nothing in empty space")
    {
      CHECK(inputStateAt(emptySpace).pickResult().empty());
    }
  }

  SECTION("mouseClick")
  {
    SECTION("selects the vertex under the mouse")
    {
      CHECK(controller.mouseClick(inputStateAt({16, 16, 16})));
      CHECK(selectedVertices() == std::vector<vm::vec3d>{{16, 16, 16}});
    }

    SECTION("replaces the selection")
    {
      selectVertex({-16, 16, 16});

      CHECK(controller.mouseClick(inputStateAt({16, 16, 16})));
      CHECK(selectedVertices() == std::vector<vm::vec3d>{{16, 16, 16}});
    }

    SECTION("with CtrlCmd")
    {
      selectVertex({-16, 16, 16});

      SECTION("adds an unselected vertex to the selection")
      {
        CHECK(controller.mouseClick(inputStateAt({16, 16, 16}, ModifierKeys::CtrlCmd)));
        CHECK_THAT(
          selectedVertices(),
          UnorderedRangeEquals(std::vector<vm::vec3d>{{-16, 16, 16}, {16, 16, 16}}));
      }

      SECTION("removes a selected vertex from the selection")
      {
        CHECK(controller.mouseClick(inputStateAt({-16, 16, 16}, ModifierKeys::CtrlCmd)));
        CHECK(selectedVertices().empty());
      }
    }

    SECTION("in empty space")
    {
      SECTION("deselects all vertices")
      {
        selectVertex({16, 16, 16});

        CHECK(controller.mouseClick(inputStateAt(emptySpace)));
        CHECK(selectedVertices().empty());
      }

      SECTION("does nothing if no vertex is selected")
      {
        CHECK(!controller.mouseClick(inputStateAt(emptySpace)));
      }
    }

    SECTION("does nothing with another mouse button")
    {
      CHECK(!controller.mouseClick(
        inputStateAt({16, 16, 16}, ModifierKeys::None, MouseButtons::Right)));
      CHECK(selectedVertices().empty());
    }

    SECTION("does nothing with Alt or Shift")
    {
      const auto modifierKeys = GENERATE(
        ModifierKeys::Alt,
        ModifierKeys::Shift,
        ModifierKeys::Alt | ModifierKeys::CtrlCmd,
        ModifierKeys::Shift | ModifierKeys::CtrlCmd);
      CAPTURE(modifierKeys);

      CHECK(!controller.mouseClick(inputStateAt({16, 16, 16}, modifierKeys)));
      CHECK(selectedVertices().empty());
    }

    SECTION("with Shift+Alt")
    {
      const auto modifierKeys = ModifierKeys::Shift | ModifierKeys::Alt;

      // https://github.com/TrenchBroom/TrenchBroom/issues/5491
      SECTION("moves the selected vertex onto the vertex under the mouse")
      {
        selectVertex({16, 16, 16});

        CHECK(controller.mouseClick(inputStateAt({-16, 16, 16}, modifierKeys)));

        CHECK(brushNode->brush().vertexCount() == 7u);
        CHECK(!brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
        CHECK(selectedVertices() == std::vector<vm::vec3d>{{-16, 16, 16}});
      }

      SECTION("does nothing if no vertex is selected")
      {
        CHECK(!controller.mouseClick(inputStateAt({-16, 16, 16}, modifierKeys)));

        CHECK(selectedVertices().empty());
        CHECK(brushNode->brush().vertexCount() == 8u);
      }

      SECTION("does nothing if several vertices are selected")
      {
        selectVertex({16, 16, 16});
        selectVertex({16, -16, 16});

        CHECK(!controller.mouseClick(inputStateAt({-16, 16, 16}, modifierKeys)));

        CHECK_THAT(
          selectedVertices(),
          UnorderedRangeEquals(std::vector<vm::vec3d>{{16, 16, 16}, {16, -16, 16}}));
        CHECK(brushNode->brush().vertexCount() == 8u);
      }

      SECTION("does nothing if the mouse is not over a vertex")
      {
        selectVertex({16, 16, 16});

        // the center of the top face, where Shift offers to split the face
        CHECK(!controller.mouseClick(inputStateAt({0, 0, 16}, modifierKeys)));

        CHECK(selectedVertices() == std::vector<vm::vec3d>{{16, 16, 16}});
        CHECK(brushNode->brush().vertexCount() == 8u);
      }
    }
  }

  SECTION("acceptMouseDrag")
  {
    SECTION("on a vertex")
    {
      SECTION("starts a move with any combination of modifier keys")
      {
        const auto modifierKeys = GENERATE(
          ModifierKeys::None,
          ModifierKeys::Alt,
          ModifierKeys::CtrlCmd,
          ModifierKeys::CtrlCmd | ModifierKeys::Alt,
          ModifierKeys::Shift,
          ModifierKeys::Shift | ModifierKeys::Alt,
          ModifierKeys::Shift | ModifierKeys::CtrlCmd,
          ModifierKeys::Shift | ModifierKeys::CtrlCmd | ModifierKeys::Alt);
        CAPTURE(modifierKeys);

        auto tracker =
          controller.acceptMouseDrag(inputStateAt({16, 16, 16}, modifierKeys));
        REQUIRE(tracker != nullptr);
        CHECK(selectedVertices() == std::vector<vm::vec3d>{{16, 16, 16}});

        tracker->cancel();
      }

      SECTION("moves the vertex horizontally")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt({16, 16, 16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({32, 16, 16})));
        tracker->end(inputStateAt({32, 16, 16}));

        CHECK(brushNode->brush().hasVertex(vm::vec3d{32, 16, 16}));
        CHECK(!brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
        CHECK(selectedVertices() == std::vector<vm::vec3d>{{32, 16, 16}});
      }

      SECTION("moves the vertex vertically with Alt")
      {
        auto tracker =
          controller.acceptMouseDrag(inputStateAt({16, 16, 16}, ModifierKeys::Alt));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({16, 16, 32}, ModifierKeys::Alt)));
        tracker->end(inputStateAt({16, 16, 32}, ModifierKeys::Alt));

        CHECK(brushNode->brush().hasVertex(vm::vec3d{16, 16, 32}));
        CHECK(!brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
      }

      SECTION("snapping")
      {
        // move the vertex at 16 16 16 off the grid
        REQUIRE(mdl::translateSelection(map, vm::vec3d{4, 0, 0}));
        REQUIRE(brushNode->brush().hasVertex(vm::vec3d{20, 16, 16}));

        SECTION("snaps the distance moved to the grid")
        {
          auto tracker = controller.acceptMouseDrag(inputStateAt({20, 16, 16}));
          REQUIRE(tracker != nullptr);

          CHECK(tracker->update(inputStateAt({42, 16, 16})));
          tracker->end(inputStateAt({42, 16, 16}));

          CHECK(brushNode->brush().hasVertex(vm::vec3d{36, 16, 16}));
        }

        SECTION("snaps the vertex to the grid with CtrlCmd")
        {
          auto tracker =
            controller.acceptMouseDrag(inputStateAt({20, 16, 16}, ModifierKeys::CtrlCmd));
          REQUIRE(tracker != nullptr);

          CHECK(tracker->update(inputStateAt({42, 16, 16}, ModifierKeys::CtrlCmd)));
          tracker->end(inputStateAt({42, 16, 16}, ModifierKeys::CtrlCmd));

          CHECK(brushNode->brush().hasVertex(vm::vec3d{48, 16, 16}));
        }
      }

      SECTION("moves all selected vertices if the vertex is selected")
      {
        selectVertex({16, 16, 16});
        selectVertex({-16, 16, 16});

        auto tracker = controller.acceptMouseDrag(inputStateAt({16, 16, 16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({16, 32, 16})));
        tracker->end(inputStateAt({16, 32, 16}));

        CHECK(brushNode->brush().hasVertex(vm::vec3d{16, 32, 16}));
        CHECK(brushNode->brush().hasVertex(vm::vec3d{-16, 32, 16}));
        CHECK_THAT(
          selectedVertices(),
          UnorderedRangeEquals(std::vector<vm::vec3d>{{16, 32, 16}, {-16, 32, 16}}));
      }

      SECTION("moves only the vertex if it is not selected")
      {
        selectVertex({-16, 16, 16});

        auto tracker = controller.acceptMouseDrag(inputStateAt({16, 16, 16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({16, 32, 16})));
        tracker->end(inputStateAt({16, 32, 16}));

        CHECK(brushNode->brush().hasVertex(vm::vec3d{16, 32, 16}));
        CHECK(brushNode->brush().hasVertex(vm::vec3d{-16, 16, 16}));
        CHECK(selectedVertices() == std::vector<vm::vec3d>{{16, 32, 16}});
      }

      SECTION("does not move the vertex out of the world bounds")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt({16, 16, 16}));
        REQUIRE(tracker != nullptr);

        const auto outside = vm::vec3d{2.0 * map.worldBounds().max.x(), 16, 16};
        CHECK(tracker->update(inputStateAt(outside)));
        tracker->end(inputStateAt(outside));

        CHECK(brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
        CHECK(brushNode->brush().vertexCount() == 8u);
      }

      SECTION("ends the move when the vertex is absorbed by the brush")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt({16, 16, 16}));
        REQUIRE(tracker != nullptr);

        // on the line between two other vertices of the top face
        CHECK(!tracker->update(inputStateAt({0, 0, 16})));
        tracker->end(inputStateAt({0, 0, 16}));

        CHECK(brushNode->brush().vertexCount() == 7u);
        CHECK(!brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
        CHECK(selectedVertices().empty());
      }

      SECTION("restores the brush when the move is cancelled")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt({16, 16, 16}));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt({32, 16, 16})));
        REQUIRE(brushNode->brush().hasVertex(vm::vec3d{32, 16, 16}));

        tracker->cancel();

        CHECK(brushNode->brush().hasVertex(vm::vec3d{16, 16, 16}));
        CHECK(!brushNode->brush().hasVertex(vm::vec3d{32, 16, 16}));
      }
    }

    SECTION("with Shift")
    {
      SECTION("on an edge, splits the edge")
      {
        // just beside the edge, so that the pick ray misses the adjacent faces
        auto tracker = controller.acceptMouseDrag(
          inputStateAt({16.25, 16.25, 0}, ModifierKeys::Shift));
        REQUIRE(tracker != nullptr);
        CHECK(map.nodeHandles().selectedHandleCount<mdl::EdgeHandle>() == 1u);

        CHECK(tracker->update(inputStateAt({32, 16, 0}, ModifierKeys::Shift)));
        tracker->end(inputStateAt({32, 16, 0}, ModifierKeys::Shift));

        CHECK(brushNode->brush().vertexCount() == 9u);
        CHECK(brushNode->brush().hasVertex(vm::vec3d{32, 16, 0}));
        CHECK(selectedVertices() == std::vector<vm::vec3d>{{32, 16, 0}});
      }

      SECTION("on a face, splits the face")
      {
        const auto modifierKeys = ModifierKeys::Shift | ModifierKeys::Alt;

        auto tracker = controller.acceptMouseDrag(inputStateAt({0, 0, 16}, modifierKeys));
        REQUIRE(tracker != nullptr);
        CHECK(map.nodeHandles().selectedHandleCount<mdl::FaceHandle>() == 1u);

        CHECK(tracker->update(inputStateAt({0, 0, 32}, modifierKeys)));
        tracker->end(inputStateAt({0, 0, 32}, modifierKeys));

        CHECK(brushNode->brush().vertexCount() == 9u);
        CHECK(brushNode->brush().hasVertex(vm::vec3d{0, 0, 32}));
        CHECK(selectedVertices() == std::vector<vm::vec3d>{{0, 0, 32}});
      }
    }

    SECTION("in empty space")
    {
      // opposite corners of a box around the front top right vertex of the brush
      const auto vertex = vm::vec3d{16, -16, 16};
      const auto diagonal = vm::vec3d{4.0f * (camera.right() + camera.up())};
      const auto lassoStart = vertex - diagonal;
      const auto lassoEnd = vertex + diagonal;

      SECTION("starts a lasso that selects only the vertices inside of it")
      {
        selectVertex({-16, 16, 16});

        auto tracker = controller.acceptMouseDrag(inputStateAt(lassoStart));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt(lassoEnd)));
        tracker->end(inputStateAt(lassoEnd));

        CHECK(selectedVertices() == std::vector<vm::vec3d>{vertex});
      }

      SECTION("starts a lasso that toggles the vertices inside of it with CtrlCmd")
      {
        selectVertex({-16, 16, 16});
        selectVertex(vertex);

        auto tracker =
          controller.acceptMouseDrag(inputStateAt(lassoStart, ModifierKeys::CtrlCmd));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt(lassoEnd, ModifierKeys::CtrlCmd)));
        tracker->end(inputStateAt(lassoEnd, ModifierKeys::CtrlCmd));

        CHECK(selectedVertices() == std::vector<vm::vec3d>{{-16, 16, 16}});
      }

      SECTION("selects nothing when the lasso is cancelled")
      {
        auto tracker = controller.acceptMouseDrag(inputStateAt(lassoStart));
        REQUIRE(tracker != nullptr);

        CHECK(tracker->update(inputStateAt(lassoEnd)));
        tracker->cancel();

        CHECK(selectedVertices().empty());
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
          inputStateAt({16, 16, 16}, ModifierKeys::None, MouseButtons::Right))
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
    SECTION("deselects all vertices")
    {
      selectVertex({16, 16, 16});

      CHECK(controller.cancel());
      CHECK(selectedVertices().empty());
    }

    SECTION("does nothing if no vertex is selected")
    {
      CHECK(!controller.cancel());
    }
  }
}

} // namespace tb::ui
